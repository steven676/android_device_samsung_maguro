/**
 * maguro-rild.h -- chroot jail configuration for rild on maguro
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

#ifndef _CHROOT_SANDBOX_MAGURO_RILD_H
#define _CHROOT_SANDBOX_MAGURO_RILD_H

#include "sandbox.h"

#define SANDBOX_PATH "/sandbox/rild"

char *const maguro_rild_args[] = { "/system/bin/rild", NULL };

#define SANDBOX_COMMAND "/rild"
#define SANDBOX_COMMAND_ARGS maguro_rild_args

/* Who knows what proprietary crap needs for file descriptors ... */
#undef USE_FEXECVE

struct dir newdirs[] = {
	{ .name = "data", .mode = 0755 },
	{ .name = "data/data", .mode = 0755 },
	{ NULL, 0 }
};

struct file copies[] = {
	{ .src = "/default.prop", .dst = "default.prop" },
	{ NULL, NULL }
};

struct file symlinks[] = {
	{ .src = "system/vendor", .dst = "vendor" },
	{ NULL, NULL }
};

struct bindmount mounts[] = {
	{ .src = "/proc", .dst = "proc", .flags = MS_NODEV|MS_NOSUID|MS_NOEXEC },
	{ .src = "/sys", .dst = "sys", .flags = MS_NODEV|MS_NOSUID|MS_NOEXEC },
	{ .src = "/acct", .dst = "acct", .flags = MS_NODEV|MS_NOSUID|MS_NOEXEC },
	{ .src = "/dev", .dst = "dev" },
	{ .src = "/system", .dst = "system", .flags = MS_RDONLY|MS_NODEV|MS_NOSUID },
	{ .src = "/factory", .dst = "factory", .flags = MS_RDONLY|MS_NODEV|MS_NOSUID|MS_NOEXEC },
	{ .src = "/cache", .dst = "cache" },
	{ .src = "/data/misc", .dst = "data/misc" },
	{ .src = "/data/radio", .dst = "data/radio" },
	{ .src = "/data/data/com.android.providers.telephony", .dst = "data/data/com.android.providers.telephony" },
	{ NULL, NULL, 0 }
};

#endif	/* _CHROOT_SANDBOX_MAGURO_RILD_H */
