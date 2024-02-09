#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <assert.h>
#include <ctype.h>

#include <ncurses.h>

static void on_error()
{
	perror("editor");
	exit(1);
}

#define runtime_assert(expression)  (((bool)(expression)) ? (void)0 : on_error())

inline static uintptr_t round_up_to_page_size(uintptr_t size)
{
	// TODO: use syscall to get page size
	return ((size - 1) & ~(4096 - 1)) + 4096;
}

typedef struct __line_t
{
	// TODO: Kinda wanna rewrite this in C++ with std::vector later
	size_t size;
	size_t capacity;
	char *buffer;
	struct __line_t *prev;
	struct __line_t *next;
} line_t;

typedef enum __state_t
{
	COMMAND,
	INSERT,
} state_t;

typedef struct __cursor_t
{
	size_t line;
	size_t column;
	line_t *line_ptr;
} cursor_t;

line_t head = { 0, 0, NULL, &head, &head };

inline bool is_empty(line_t *line) { return line == line->next; }
inline bool is_head(line_t *line) { return line == &head; }
inline bool is_last(line_t *line) { return !is_head(line) && line == head.prev; }
inline bool is_first(line_t *line) { return !is_head(line) && line == head.next; }

FILE *file = NULL;
state_t state = COMMAND;
cursor_t cursor = { 0, 0, NULL};
size_t starting_column = 0;
size_t starting_line_index = 0;
line_t *starting_line = NULL;
size_t viewport_width = 80;
size_t viewport_height = 24;
bool is_running = true;
size_t line_count = 0;
bool is_writing_command = false;
line_t *command_buffer;

void insert_after(line_t *line, line_t *element)
{
	assert(line && element);

	line->next->prev = element;
	element->next = line->next;
	line->next = element;
	element->prev = line;

	line_count++;
}

void insert_before(line_t *line, line_t *element)
{
	assert(line && element);

	line->prev->next = element;
	element->prev = line->prev;
	line->prev = element;
	element->next = line;

	line_count++;
}

line_t *create_empty_line()
{
	line_t *line = malloc(sizeof(line_t));
	runtime_assert(line);

	line->size = 1;
	line->capacity = 4;
	line->prev = line->next = line;

	line->buffer = malloc(line->capacity);
	runtime_assert(line->buffer);

	memset(line->buffer, 0, line->capacity);

	return line;
}

void ensure_capacity(line_t *line, size_t capacity)
{
	assert(line);

	if (capacity <= line->capacity)
		return;

	size_t new_capacity = line->capacity;

	while (new_capacity < capacity)
		new_capacity *= 2;

	char *new_buffer = realloc(line->buffer, new_capacity);
	runtime_assert(new_buffer);

	line->buffer = new_buffer;
	memset(line->buffer + line->capacity, 0, new_capacity - line->capacity);
	line->capacity = new_capacity;
}

void insert_char_at(line_t *line, char ch, size_t position)
{
	assert(line && position < line->size);

	ensure_capacity(line, line->size + 1);

	memmove(line->buffer + position + 1, line->buffer + position, line->size - position);
	line->buffer[position] = ch;
	line->size += 1;
}

void insert_char_at_end(line_t *line, char ch)
{
	insert_char_at(line, ch, line->size - 1);
}

void clear_line(line_t *line)
{
	memset(line->buffer, 0, line->size);
	line->size = 1;
}

void break_at(line_t *line, size_t position)
{
	assert(line && position < line->size - 1);

	line_t *next_line = create_empty_line();
	size_t next_line_size = line->size - position;

	ensure_capacity(next_line, next_line_size);
	memcpy(next_line->buffer, line->buffer + position, next_line_size);
	memset(line->buffer + position, 0, next_line_size);

	next_line->size = next_line_size;
	line->size -= next_line_size;

	insert_after(line, next_line);
}

static void read_lines(char *buffer)
{
	size_t index = 0;

	size_t current_line_start = 0;

	do
	{
		// TODO: this assumes NL format (not CR or CRNL)
		if (buffer[index] == '\n' || buffer[index] == '\0')
		{
			line_t *line = create_empty_line();
			size_t size = index - current_line_start + 1;
			ensure_capacity(line, size);
			memcpy(line->buffer, buffer + current_line_start, size);
			line->size = size;
			insert_before(&head, line);
			current_line_start = index + 1;
		}
	} while (buffer[index++] != '\0');

	assert(!is_empty(&head));
}

static void initialize(int argc, char **argv)
{
	if (argc != 2)
	{
		puts("Usage: editor <file>");
		exit(1);
	}

	file = fopen(argv[1], "r+");

	if (!file)
	{
		perror("fopen");
		exit(1);
	}

	struct stat st;
	fstat(fileno(file), &st);

	// TODO: Use private file mapping when implemented
	uintptr_t mapping_size = round_up_to_page_size(st.st_size);
	char *buffer = mmap(NULL, mapping_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if ((int)buffer == MAP_FAILED)
	{
		perror("mmap");
		exit(1);
	}

	size_t bytes_read = read(fileno(file), buffer, st.st_size);
	memset(buffer + st.st_size, 0, mapping_size - st.st_size);

	if (bytes_read != st.st_size)
	{
		perror("read");
		exit(1);
	}

	read_lines(buffer);
	munmap(buffer, mapping_size);

	cursor.line_ptr = head.next;
	starting_line = head.next;
	starting_line_index = 0;
	command_buffer = create_empty_line();
}

static void finalize()
{
	fflush(file);
	fclose(file);

	line_t *current = head.next;

	while (current != &head)
	{
		free(current->buffer);
		line_t *next = current->next;
		free(current);
		current = next;
	}
}

static void draw_status_bar()
{
	move((int)viewport_height - 1, 0);
	clrtobot();

	const char *state_strings[] = { "COMMAND", "INSERT" };

	if (state == COMMAND && is_writing_command)
		mvaddstr((int)viewport_height - 1, 0, command_buffer->buffer);
	else
		mvprintw((int)viewport_height - 1, 0, "-- %s --", state_strings[state]);

	mvprintw((int)viewport_height - 1, (int)viewport_width - 10, "%lu,%lu", cursor.line, cursor.column);
}

static void update_screen(size_t start_position, line_t *first_line)
{
	clear();
	assert(first_line);

	line_t *line = first_line;

	for (size_t row = 0; row < viewport_height - 1; row++)
	{
		size_t print_count = line->size > start_position ? line->size - start_position : 0;
		if (print_count > viewport_width)
			print_count = viewport_width;

		mvaddnstr(row, 0, line->buffer + start_position, print_count);
		refresh(); // TODO: Remove this line when Terminal implements scroll regions

		line = line->next;

		if (is_head(line))
			break;
	}

	clrtobot();
	draw_status_bar();
	refresh();
}

static void update_cursor()
{
	update_screen(starting_column, starting_line);
	move(cursor.line - starting_line_index, cursor.column - starting_column);
}

static void scroll_up()
{
	if (starting_line_index == 0)
		return;

	starting_line_index--;
	starting_line = starting_line->prev;
	update_screen(starting_column, starting_line);
}

static void scroll_down()
{
	if (starting_line_index + viewport_height - 1 >= line_count)
		return;

	starting_line_index++;
	starting_line = starting_line->next;
	update_screen(starting_column, starting_line);
}

static void scroll_right()
{
	starting_column += viewport_width;
	update_screen(starting_column, starting_line);
}

static void scroll_left()
{
	starting_column -= viewport_width;
	update_screen(starting_column, starting_line);
}

static void move_cursor_up()
{
	if (!is_first(cursor.line_ptr))
	{
		cursor.line_ptr = cursor.line_ptr->prev;
		cursor.line--;

		if (cursor.column >= cursor.line_ptr->size)
		{
			cursor.column = cursor.line_ptr->size - 1;

			if (cursor.column < starting_column)
				scroll_left();
		}

		if (cursor.line < starting_line_index)
			scroll_up();

		update_cursor();
	}
}

static void move_cursor_down()
{
	if (!is_last(cursor.line_ptr))
	{
		cursor.line_ptr = cursor.line_ptr->next;
		cursor.line++;

		if (cursor.column >= cursor.line_ptr->size)
		{
			cursor.column = cursor.line_ptr->size - 1;

			if (cursor.column < starting_column)
				scroll_left();
		}

		if (cursor.line >= starting_line_index + viewport_height - 1)
			scroll_down();

		update_cursor();
	}
}

static void move_cursor_left()
{
	if (cursor.column > 0)
	{
		cursor.column--;

		if (cursor.column < starting_column)
			scroll_left();

		update_cursor();
	}
}

static void move_cursor_right()
{
	if (cursor.column < cursor.line_ptr->size - 1)
	{
		cursor.column++;

		if (cursor.column >= starting_column + viewport_width)
			scroll_right();

		update_cursor();
	}
}

static void execute_command(char *cmd)
{
	// TODO: vim-like command parsing

	bool do_save = false;
	bool do_exit = false;
	bool force = false;

	char *current = cmd + 1;
	while (*current != '\0')
	{
		switch (*current++)
		{
		case 'w':
			do_save = true;
			break;
		case 'q':
			do_exit = true;
			break;
		case '!':
			if (*current != '\0')
			{
				mvaddstr((int)viewport_height - 1, 0, "Invalid command");
				refresh();
				return;
			}
			force = true;
			break;
		default:
			mvaddstr((int)viewport_height - 1, 0, "Invalid command");
			refresh();
			return;
		}
	}

	if (do_save)
	{
		// TODO: save
	}

	if (do_exit && (do_save || force))
	{
		is_running = false;
	}

	draw_status_bar();
	refresh();
}

static void command(int ch)
{
	if (is_writing_command)
	{
		switch (ch)
		{
		case '\n':
			execute_command(command_buffer->buffer);
			__attribute__((fallthrough));
		case '\e':
			is_writing_command = false;
			clear_line(command_buffer);
			return;
		default:
			break;
		}

		insert_char_at_end(command_buffer, ch);
		draw_status_bar();
		refresh();

		return;
	}

	switch (ch)
	{
	case 'h':
		move_cursor_left();
		break;
	case 'j':
		move_cursor_down();
		break;
	case 'k':
		move_cursor_up();
		break;
	case 'l':
		move_cursor_right();
		break;
	case 'i':
		state = INSERT;
		draw_status_bar();
		refresh();
		break;
	case ':':
		is_writing_command = true;
		insert_char_at_end(command_buffer, ':');
		draw_status_bar();
		refresh();
	default:
		break;
	}
}

static void insert(int ch)
{
	// TODO: We can't yet differentiate a standalone escape from an escape sequence
	//       For that we need access to timers (poll(), select() and time() i guess)
	if (ch == '\e')
	{
		state = COMMAND;
		draw_status_bar();
		refresh();
		return;
	}

	if (ch == '\n')
	{
		break_at(cursor.line_ptr, cursor.column);
		move_cursor_down();
		cursor.column = 0;
		if (starting_column)
			starting_column = 0;
		update_screen(starting_column, starting_line);
		update_cursor();
		return;
	}

	if (!isascii(ch))
		return;

	insert_char_at(cursor.line_ptr, (char)ch, cursor.column);
	cursor.column++;

	if (cursor.column >= starting_column + viewport_width)
		scroll_right();

	update_screen(starting_column, starting_line);
	update_cursor();
}

int main(int argc, char **argv)
{
	initialize(argc, argv);

	initscr();
	cbreak();
	noecho();
	clear();

	getmaxyx(stdscr, viewport_height, viewport_width);

	update_screen(0, head.next);
	update_cursor();

	while (is_running)
	{
		int ch = getch();

		switch (state)
		{
		case COMMAND:
			command(ch);
			break;
		case INSERT:
			insert(ch);
			break;
		default:
			assert(false);
			break;
		}
	}

	endwin();

	finalize();

	return 0;
}