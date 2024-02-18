#pragma once

#include "sys/types.h"
#include "stdint.h"

#define SIG_DFL 1
#define SIG_ERR 2
#define SIG_HOLD 3
#define SIG_IGN 4

#define SIGABRT 1
#define SIGALRM 2
#define SIGBUS 3
#define SIGCHLD 4
#define SIGCONT 5
#define SIGFPE 6
#define SIGHUP 7
#define SIGILL 8
#define SIGINT 9
#define SIGKILL 10
#define SIGPIPE 11
#define SIGQUIT 12
#define SIGSEGV 13
#define SIGSTOP 14
#define SIGTERM 15
#define SIGTSTP 16
#define SIGTTIN 17
#define SIGTTOU 18
#define SIGUSR1 19
#define SIGUSR2 20
#define SIGPOLL 21
#define SIGPROF 22
#define SIGSYS 23
#define SIGTRAP 24
#define SIGURG 25
#define SIGVTALRM 26
#define SIGXCPU 27
#define SIGXFSZ 28

#define SIG_BLOCK 1
#define SIG_UNBLOCK 2
#define SIG_SETMASK 3

#ifdef __cplusplus
extern "C"
{
#endif

	typedef volatile int sig_atomic_t;
	typedef uint32_t sigset_t;

	union sigval
	{
		int sival_int;
		void *sival_ptr;
	};

	typedef struct __siginfo_t
	{
		int si_signo;
		int si_code;
		int si_errno;
		pid_t si_pid;
		uid_t si_uid;
		void *si_addr;
		int si_status;
		long si_band;
		union sigval si_value;
	} siginfo_t;

	struct sigaction
	{
		union
		{
			void (*sa_handler)(int);
			void (*sa_sigaction)(int, siginfo_t *, void *);
		};
		sigset_t sa_mask;
		int sa_flags;
	};

	void (*signal(int sig, void (*func)(int)))(int);
	int sigaction(int sig, const struct sigaction *__restrict act, struct sigaction *__restrict oact);

	int sigemptyset(sigset_t *set);
	int sigaddset(sigset_t *set, int signo);
	int sigismember(const sigset_t *set, int signo);

	int kill(pid_t pid, int sig);

	int sigprocmask(int how, const sigset_t *__restrict set, sigset_t *__restrict oset);

#ifdef __cplusplus
}
#endif
