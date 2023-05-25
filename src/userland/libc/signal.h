#pragma once

#include "sys/types.h"

#define SIGINT 1
#define SIGTERM 2
#define SIG_IGN 3
#define SIG_DFL 4

#ifdef __cplusplus
extern "C"
{
#endif

	typedef volatile int sig_atomic_t;
	typedef int sigset_t;

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
		void (*sa_handler)(int);
		sigset_t sa_mask;
		int sa_flags;
		void (*sa_sigaction)(int, siginfo_t *, void *);
	};

	void (*signal(int sig, void (*func)(int)))(int);
	int sigaction(int sig, const struct sigaction *__restrict act, struct sigaction *__restrict oact);
	int sigemptyset(sigset_t *set);

#ifdef __cplusplus
}
#endif