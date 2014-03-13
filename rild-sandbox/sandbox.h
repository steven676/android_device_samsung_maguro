/**
 * sandbox.h -- definitions for structures used to configure the sandbox
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

#ifndef _CHROOT_SANDBOX_H
#define _CHROOT_SANDBOX_H 1

/* Struct describing a new directory to create */
struct dir {
	char *name;	/* relative to sandbox root */
	int mode;
};

/* Struct describing a file to be copied or linked into the sandbox */
struct file {
	char *src;	/* absolute path */
	char *dst;	/* relative to sandbox root */
};

/* Struct describing a bind mount to be made in the sandbox */
struct bindmount {
	char *src;	/* absolute path */
	char *dst;	/* relative to sandbox root */
	int flags;	/* flags to be passed to mount(2) with MS_REMOUNT */
};

#endif	/* _CHROOT_SANDBOX_H */
