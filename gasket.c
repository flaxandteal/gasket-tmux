/* $Id$ */

/*
 * Copyright (c) 2007 Nicholas Marriott <nicm@users.sourceforge.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <event.h>
#include <fcntl.h>
#include <locale.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tmux.h"

char		 gasket_socket_path[MAXPATHLEN];

char 		*gasket_makesocketpath(void);

char *
gasket_makesocketpath(void)
{
	char		base[MAXPATHLEN], realbase[MAXPATHLEN], *path, *s;
	struct stat	sb;
	u_int		uid;
        char            label[20];

        xsnprintf(label, sizeof label, "%s", "_gasket");

	uid = getuid();
	if ((s = getenv("TMUX_TMPDIR")) != NULL && *s != '\0')
		xsnprintf(base, sizeof base, "%s/", s);
	else if ((s = getenv("TMPDIR")) != NULL && *s != '\0')
		xsnprintf(base, sizeof base, "%s/tmux-%u", s, uid);
	else
		xsnprintf(base, sizeof base, "%s/tmux-%u", _PATH_TMP, uid);

	if (mkdir(base, S_IRWXU) != 0 && errno != EEXIST)
		return (NULL);

	if (lstat(base, &sb) != 0)
		return (NULL);
	if (!S_ISDIR(sb.st_mode)) {
		errno = ENOTDIR;
		return (NULL);
	}
	if (sb.st_uid != uid || (sb.st_mode & (S_IRWXG|S_IRWXO)) != 0) {
		errno = EACCES;
		return (NULL);
	}

	if (realpath(base, realbase) == NULL)
		strlcpy(realbase, base, sizeof realbase);

	xasprintf(&path, "%s/%s", realbase, label);
	return (path);
}

int
gasket_init(void)
{
        char* path = gasket_makesocketpath();
	strlcpy(gasket_socket_path, path, sizeof gasket_socket_path);
	free(path);
}
