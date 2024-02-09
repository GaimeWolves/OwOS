#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#define INPUT_BUFFER_LEN 1024
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

extern char **environ;

char **path = NULL;

char cwd[PATH_MAX];

char input_buffer[INPUT_BUFFER_LEN] = {0};
size_t input_buffer_index = 0;
size_t input_buffer_length = 0;

char *command = NULL;
char **arguments = NULL;
char file_path[PATH_MAX];

bool is_running = true;

struct termios new_tty_state;
struct termios original_tty_state;

void print_error()
{
	printf("\e[91m");
	perror("sh");
	printf("\e[39m");
}

void builtin_echo()
{
	char **current = arguments + 1;

	// TODO: parse env variable references

	while (*current)
	{
		printf("%s ", *current);
		current++;
	}

	putchar('\n');
}

void builtin_cd()
{
	if (arguments[1] == NULL)
		return;

	if (chdir(arguments[1]))
	{
		print_error();
		return;
	}

	getcwd(cwd, PATH_MAX);
}

void builtin_exit()
{
	is_running = false;
}

typedef struct __builtin_t
{
	const char *name;
	void (*function)();
} builtin_t;

const builtin_t builtins[] = {
    {"echo", builtin_echo},
    {"cd", builtin_cd},
    {"exit", builtin_exit},
};

void setup_tty()
{
	tcgetattr(STDIN_FILENO, &new_tty_state);

	original_tty_state = new_tty_state;

	// non-canonical mode; blocking single character read; convert NL to CRNL
	new_tty_state.c_iflag = 0;
	new_tty_state.c_oflag = ONLCR | OPOST;
	new_tty_state.c_lflag = 0;
	new_tty_state.c_cflag = CS8 | B19200;
	new_tty_state.c_cc[VMIN] = 1;
	new_tty_state.c_cc[VTIME] = 0;

	tcsetattr(STDIN_FILENO, TCSANOW, &new_tty_state);

	printf("\e[H\e[m\e[2J");
}

void parse_path()
{
	char **current_env_var = environ;

	char *path_env_var = NULL;

	while (*current_env_var)
	{
		if (strncmp(*current_env_var, "PATH=", 5) == 0)
		{
			path_env_var = strdup((*current_env_var) + 5);
			break;
		}

		current_env_var++;
	}

	if (*path_env_var == '\0')
	{
		path = calloc(1, sizeof(char *));
		free(path_env_var);
		return;
	}

	size_t count = 2;
	size_t len = strlen(path_env_var);

	for (size_t i = 0; i < len; i++)
	{
		if (path_env_var[i] == ';')
			count++;
	}

	path = calloc(count, sizeof(char *));

	size_t i = 0;
	path[i++] = strtok(path_env_var, ";");
	while ((path[i++] = strtok(NULL, ";")))
		;
}

void cleanup()
{
	free(path[0]);
	free(path);

	tcsetattr(STDIN_FILENO, TCSANOW, &original_tty_state);
}

void init_prompt()
{
	memset(input_buffer, 0, INPUT_BUFFER_LEN);
	input_buffer_index = 0;
	input_buffer_length = 0;
	command = NULL;
	file_path[0] = '\0';
	printf("\n%s\e[92m$\e[39m ", cwd);
}

void read_loop()
{
	static int current_ch = 0;

	// TODO: handle control sequences for line editing
	while (true)
	{
		current_ch = fgetc(stdin);

		if (current_ch == '\n')
		{
			putchar('\n');
			break;
		}

		if (current_ch == 0x7f)
		{
			if (input_buffer_index > 0)
			{
				// TODO: xterm backspace doesn't wrap afaik. so we should really manipulate the cursor with CSI sequences instead of backspace.
				printf("\b \b");
				memmove(input_buffer + input_buffer_index - 1, input_buffer + input_buffer_index, input_buffer_length - input_buffer_index + 1);
				input_buffer_index--;
				input_buffer_length--;
			}

			continue;
		}

		if (input_buffer_index < INPUT_BUFFER_LEN - 1)
		{
			input_buffer[input_buffer_index++] = (char)current_ch;
			input_buffer_length++;
			putchar(current_ch);
		}
	}
}

void parse_command()
{
	if (input_buffer_length == 0)
		return;

	size_t first_non_space = input_buffer_length;

	for (size_t i = 0; i < input_buffer_length; i++)
	{
		if (isspace(input_buffer[i]))
			input_buffer[i] = '\0';
		else if (first_non_space == input_buffer_length)
			first_non_space = i;
	}

	if (first_non_space == input_buffer_length)
		return;

	command = input_buffer + first_non_space;

	size_t arg_count = 1;

	for (size_t i = first_non_space; i < input_buffer_length; i++)
	{
		if (input_buffer[i] == '\0' && input_buffer[i + 1] != '\0')
			arg_count++;
	}

	size_t size = (arg_count + 1) * sizeof(char *);
	arguments = realloc(arguments, size);

	if (!arguments)
	{
		print_error();
		abort();
	}

	memset(arguments, 0, size);

	char **current = arguments + 1;

	arguments[0] = command;

	for (size_t i = first_non_space; i < input_buffer_length; i++)
	{
		if (input_buffer[i] == '\0' && input_buffer[i + 1] != '\0')
			*current++ = &input_buffer[i + 1];
	}
}

bool check_file_exists()
{
	struct stat st;

	errno = 0;
	stat(file_path, &st);

	return errno == 0;
}

bool check_builtin()
{
	for (size_t i = 0; i < ARRAY_SIZE(builtins); i++)
	{
		if (strcmp(builtins[i].name, command) == 0)
		{
			tcsetattr(STDIN_FILENO, TCSANOW, &original_tty_state);
			builtins[i].function();
			tcsetattr(STDIN_FILENO, TCSANOW, &new_tty_state);
			return true;
		}
	}

	return false;
}

void find_file()
{
	if (!command)
		return;

	if (command[0] == '/')
	{
		strncpy(file_path, command, PATH_MAX);

		if (check_file_exists())
			return;
	}
	else
	{
		char **current_path_prefix = path;

		while (*current_path_prefix)
		{
			size_t len = strlen(*current_path_prefix);

			if (len >= PATH_MAX)
				continue;

			strncpy(file_path, *current_path_prefix, PATH_MAX);

			if ((*current_path_prefix)[len - 1] != '/')
			{
				file_path[len] = '/';
				len++;
			}

			strncpy(file_path + len, command, PATH_MAX - len);

			if (check_file_exists())
				return;

			current_path_prefix++;
		}
	}

	file_path[0] = '\0';
	print_error();
}

void execute_command()
{
	if (file_path[0] == '\0')
		return;

	tcsetattr(STDIN_FILENO, TCSANOW, &original_tty_state);

	pid_t pid = fork();

	if (pid == -1)
	{
		print_error();
		tcsetattr(STDIN_FILENO, TCSANOW, &new_tty_state);
		return;
	}

	if (pid == 0)
	{
		execv(file_path, arguments);
		print_error();
		exit(errno);
	}

	int ret;
	do
	{
		ret = waitpid(pid, NULL, 0);
	} while (ret == -1 && errno == EINTR);

	tcsetattr(STDIN_FILENO, TCSANOW, &new_tty_state);
}

int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	parse_path();
	setup_tty();
	arguments = malloc(sizeof(char *));
	getcwd(cwd, PATH_MAX);

	printf("OwOS shell ver. 0.1\n");

	while (is_running)
	{
		init_prompt();
		read_loop();
		parse_command();

		if (!command)
			continue;

		if (!check_builtin())
		{
			find_file();
			execute_command();
		}
	}

	cleanup();

	return 0;
}