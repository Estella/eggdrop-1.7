/* userrec.h
 *
 * Copyright (C) 2004 - 2006 Eggheads Development Team
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
 * $Id: userrec.h,v 1.6 2006/11/20 13:53:36 tothwolf Exp $
 */

#ifndef _EGG_USERREC_H
#define _EGG_USERREC_H

#ifndef MAKING_MODS
void set_chanlist(const char *, struct userrec *);
void clear_chanlist(void);
void clear_chanlist_member(const char *);
struct userrec *adduser(struct userrec *, char *, char *, char *, int);
void addhost_by_handle(char *, char *);
void clear_masks(struct maskrec *);
void clear_userlist(struct userrec *);
int u_pass_match(struct userrec *, char *);
int delhost_by_handle(char *, char *);
int count_users(struct userrec *);
int deluser(char *);
void freeuser(struct userrec *);
int change_handle(struct userrec *, char *);
void correct_handle(char *);
struct userrec *check_dcclist_hand(char *);
void touch_laston(struct userrec *, char *, time_t);
void user_del_chan(char *);
char *fixfrom(char *);
#endif

#endif /* !_EGG_USERREC_H */
