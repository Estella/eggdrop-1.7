/* dccutil.h: prototypes for functions in dccutil.c
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
 * $Id: dccutil.h,v 1.1 2004/08/25 07:41:36 wcc Exp $
 */

#ifndef _EGG_DCCUTIL_H
#define _EGG_DCCUTIL_H

#define get_data_ptr(x) _get_data_ptr(x,__FILE__,__LINE__)

void dprintf EGG_VARARGS(int, arg1);
void chatout EGG_VARARGS(char *, arg1);
extern void (*shareout) ();
extern void (*sharein) (int, char *);
void chanout_but EGG_VARARGS(int, arg1);
void dcc_chatter(int);
void lostdcc(int);
void killtransfer(int);
void removedcc(int);
void makepass(char *);
void tell_dcc(int);
void not_away(int);
void set_away(int, char *);
void *_get_data_ptr(int, char *, int);
void dcc_remove_lost(void);
void do_boot(int, char *, char *);
int detect_dcc_flood(time_t *, struct chat_info *, int);
void flush_lines(int, struct chat_info *);
int new_dcc(struct dcc_table *, int);
char *add_cr(char *);
void changeover_dcc(int, struct dcc_table *, int);

#endif /* !_EGG_DCCUTIL_H */
