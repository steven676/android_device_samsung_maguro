/**
 * chroot-sandbox.c -- creates a chroot jail and invokes a program inside it
 *
 * Copyright (C) 2014 Steven Luo
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 */

#define _BSD_SOURCE
#define _XOPEN_SOURCE 700
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <fcntl.h>

#include "sandbox.h"
#include "config.h"

#define SANDBOX_CREATED ".sandbox_created"

static int create_dir(const char *name, int mode) {
	mode_t saved_umask = umask(0);
	int ret;
	int saved_errno = 0;

	if ((ret = mkdir(name, mode)) == -1)
		saved_errno = errno;

	umask(saved_umask);
	if (saved_errno)
		errno = saved_errno;

	return ret;
}

#define COPY_BUFSIZE 4096
static int copy_file(const char *src, const char *dst) {
	int in, out;
	struct stat st;
	mode_t saved_umask = umask(0);
	int saved_errno = 0;
	char buf[COPY_BUFSIZE];
	size_t len;

	if ((in = open(src, O_RDONLY)) == -1) {
		fprintf(stderr, "open(%s) for read failed: %s\n",
				src, strerror(errno));
		return -1;
	}

	if (fstat(in, &st) == -1) {
		saved_errno = errno;
		fprintf(stderr, "stat(%s) failed: %s\n",
				src, strerror(saved_errno));
		goto close_in;
	}

	if ((out = open(dst, O_WRONLY|O_CREAT|O_TRUNC, st.st_mode)) == -1) {
		saved_errno = errno;
		fprintf(stderr, "open(%s) for write failed: %s\n",
				dst, strerror(saved_errno));
		goto close_in;
	}

	while ((len = read(in, buf, COPY_BUFSIZE)) > 0) {
		if (write(out, buf, len) < len) {
			saved_errno = errno;
			fprintf(stderr, "write to %s failed: %s\n",
					dst, strerror(saved_errno));
			goto out;
		}
	}

	if (len == -1) {
		/* The last read failed */
		saved_errno = errno;
		fprintf(stderr, "read from %s failed: %s\n",
				src, strerror(saved_errno));
	}

out:
	close(out);
close_in:
	close(in);

	umask(saved_umask);

	if (saved_errno) {
		errno = saved_errno;
		return -1;
	}
	return 0;
}

static int bind_mount(const char *src, const char *dst, int flags) {
	int ret;

	if ((ret = create_dir(dst, 0555)) == -1)
		return ret;

	if ((ret = mount(src, dst, "bind", MS_BIND, NULL)) == -1)
		return ret;

	if (flags)
		ret = mount(src, dst, "bind", MS_BIND|MS_REMOUNT|flags, NULL);

	return ret;
}

#if defined(USE_FEXECVE) && !defined(HAVE_FEXECVE)
int fexecve(int fd, char *const argv[], char *const envp[]) {
	char buf[20];	/* strlen("/proc/self/fd/") + 5 digits + '\0*/
	snprintf(buf, 20, "/proc/self/fd/%d", fd);
	return execve(buf, argv, envp);
}
#endif

static int construct_sandbox(void) {
	struct dir *newdir;
	struct file *newfile;
	struct bindmount *mount;
	int fd;

	/* If the sandbox was created already, skip this step */
	if (access(SANDBOX_CREATED, F_OK) == 0)
		return 0;

	for (newdir = newdirs; newdir->name; ++newdir) {
		if (create_dir(newdir->name, newdir->mode) == -1) {
			fprintf(stderr, "mkdir(%s) failed: %s\n", 
					newdir->name, strerror(errno));
			return 255;
		}
	}

	for (newfile = copies; newfile->src && newfile->dst; ++newfile) {
		if (copy_file(newfile->src, newfile->dst) == -1) {
			fprintf(stderr, "copy of %s to %s failed: %s\n",
					newfile->src, newfile->dst,
					strerror(errno));
			return 255;
		}
	}

	for (newfile = symlinks; newfile->src && newfile->dst; ++newfile) {
		if (symlink(newfile->src, newfile->dst) == -1) {
			fprintf(stderr, "symlink(%s,%s) failed: %s\n",
					newfile->src, newfile->dst,
					strerror(errno));
			return 255;
		}
	}

	for (mount = mounts; mount->src && mount->dst; ++mount) {
		if (bind_mount(mount->src, mount->dst, mount->flags)) {
			fprintf(stderr, "bind mount of %s onto %s failed: %s\n",
					mount->src, mount->dst,
					strerror(errno));
			return 255;
		}
	}

	/* Set a flag to indicate sandbox creation complete */
	if ((fd = creat(SANDBOX_CREATED, 0444)) == -1)
		// XXX what to do here?
		return 0;
	close(fd);

	return 0;
}

int main(int argc, char **argv, char **envp) {
#ifdef USE_FEXECVE
	int execfd;
#endif

#ifndef SANDBOX_COMMAND
	if (argc < 2) {
		printf("Usage: %s [prog] [args...]\n", argv[0]);
		return 0;
	}
#endif

	if (chdir(SANDBOX_PATH) == -1) {
		fprintf(stderr, "cd to " SANDBOX_PATH " failed: %s\n",
				strerror(errno));
		return 255;
	}

	/* Construct the chroot */
	if (construct_sandbox())
		return 255;

#ifdef USE_FEXECVE
	/* Open the file to execute */
	if ((execfd = open(argv[1], O_RDONLY)) == -1) {
		fprintf(stderr, "couldn't open %s to execute: %s\n",
				argv[1], strerror(errno));
		return 255;
	}
#endif

	/* Do the chroot */
	if (chroot(".") == -1) {
		perror("chroot() failed");
		return 255;
	}

#ifdef _SETUP_ONLY
	/* for debugging purposes */
	return 0;
#endif

	/* Invoke the program to execute */
#if defined(USE_FEXECVE)
	fexecve(execfd, argv+1, envp);
#elif defined(SANDBOX_COMMAND) && defined(SANDBOX_COMMAND_ARGS)
	execve(SANDBOX_COMMAND, SANDBOX_COMMAND_ARGS, envp);
#else
	execve(argv[1], argv+1, envp);
#endif

	/* We only get here if the call to execve() fails */
	perror("exec() failed");
	return 255;
}
