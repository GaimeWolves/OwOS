#pragma once

// https://github.com/SerenityOS/serenity/blob/671712cae67d7b8782afa94c09e980a77450efd6/Kernel/API/POSIX/sys/types.h
#define unsigned signed
typedef __SIZE_TYPE__ ssize_t;
#undef unsigned

typedef __SIZE_TYPE__ size_t;

typedef unsigned int pid_t;
typedef int off_t;