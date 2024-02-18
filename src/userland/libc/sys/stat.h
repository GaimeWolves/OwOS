#pragma once

#include <bits/guards.h>
#include <sys/types.h>
#include <time.h>

__LIBC_HEADER_BEGIN

#define S_IFMT     0170000
#define S_IFSOCK   0140000
#define S_IFLNK    0120000
#define S_IFREG    0100000
#define S_IFBLK    0060000
#define S_IFDIR    0040000
#define S_IFCHR    0020000
#define S_IFIFO    0010000

#define S_ISDIR(m) (m & S_IFDIR)

#define st_atime st_atim.tv_sec
#define st_ctime st_ctim.tv_sec
#define st_mtime st_mtim.tv_sec

struct stat
{
	dev_t st_dev;
	ino_t st_ino;
	mode_t st_mode;
	nlink_t st_nlink;
	uid_t st_uid;
	gid_t st_gid;
	dev_t st_rdev;
	off_t st_size;
	struct timespec st_atim;
	struct timespec st_mtim;
	struct timespec st_ctim;
	blksize_t st_blksize;
	blkcnt_t st_blocks;
};

int stat(const char *__restrict path, struct stat *__restrict buf);
int fstat(int, struct stat *);

int mkdir(const char *path, mode_t mode);

__LIBC_HEADER_END
