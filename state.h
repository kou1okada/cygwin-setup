/*
 * Copyright (c) 2000, Red Hat, Inc.
 *
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     A copy of the GNU General Public License can be found at
 *     http://www.gnu.org/
 *
 * Written by DJ Delorie <dj@cygnus.com>
 *
 */

/* The purpose of this file is to contain all the global variables
   that define the "state" of the install, that is, all the
   information that the user has provided so far.  These are set by
   the various dialogs and used by the various actions. */

extern int	source;

extern char *	root_dir;
extern int	root_text;
extern int	root_scope;

extern int	net_method;
extern char *	net_proxy_host;
extern int	net_proxy_port;
extern char *	net_proxy_user;
extern char *	net_proxy_passwd;

extern char *	mirror_site;
extern char *	other_url;

extern int	trust_level;

#define MIRROR_SITE (mirror_site ? mirror_site : other_url)
