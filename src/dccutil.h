/* dccutil.h
 *
 * Copyright (C) 2004 Eggheads Development Team
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id: dccutil.h,v 1.6 2004/10/06 00:04:32 wcc Exp $
 */

#ifndef _EGG_DCCUTIL_H
#define _EGG_DCCUTIL_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "dcc.h" /* struct dcc_table, struct chat_info */

#ifdef HAVE_DPRINTF
#  define dprintf eggdrop_dprintf
#endif

/* Fake idx's for dprintf - these should be ridiculously large +ve nums. */
#define DP_STDOUT       0x7FF1
#define DP_LOG          0x7FF2
#define DP_SERVER       0x7FF3
#define DP_HELP         0x7FF4
#define DP_STDERR       0x7FF5
#define DP_MODE         0x7FF6
#define DP_MODE_NEXT    0x7FF7
#define DP_SERVER_NEXT  0x7FF8
#define DP_HELP_NEXT    0x7FF9

#ifndef MAKING_MODS
#  define get_data_ptr(x) _get_data_ptr(x,__FILE__,__LINE__)
#endif

#ifndef MAKING_MODS
int findidx(int);
int findanyidx(int);
char *add_cr(char *);
void remove_crlf(char **);
void show_banner(int);
void show_motd(int);
void dprintf EGG_VARARGS(int, arg1);
void chatout EGG_VARARGS(char *, arg1);
extern void (*shareout) ();
extern void (*sharein) (int, char *);
void chanout_but EGG_VARARGS(int, arg1);
void dcc_chatter(int);
void killtransfer(int);
void lostdcc(int);
void removedcc(int);
void dcc_remove_lost(void);
void tell_dcc(int);
void not_away(int);
void set_away(int, char *);
void *_get_data_ptr(int, char *, int);
void makepass(char *);
void flush_lines(int, struct chat_info *);
int new_dcc(struct dcc_table *, int);
void changeover_dcc(int, struct dcc_table *, int);
int detect_dcc_flood(time_t *, struct chat_info *, int);
void do_boot(int, char *, char *);
#endif

#endif /* !_EGG_DCCUTIL_H */
