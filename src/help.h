/* help.h
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
 * $Id: help.h,v 1.3 2006/11/20 13:53:34 tothwolf Exp $
 */

#ifndef _EGG_HELP_H
#define _EGG_HELP_H

#include "flags.h" /* flag_record */

struct help_list_t {
  struct help_list_t *next;
  char *name;
  int type;
};

struct help_ref {
  struct help_list_t *first;
  struct help_ref *next;
  char *name;
};

#define HELP_BUF_LEN 256

/* Help display flags. */
#define HELP_BOLD  0x01
#define HELP_REV   0x02
#define HELP_UNDER 0x04
#define HELP_FLASH 0x08

#ifndef MAKING_MODS
int help_expmem();
void showhelp(char *, char *, struct flag_record *, int);
void tellhelp(int, char *, struct flag_record *, int);
void help_subst(char *, char *, struct flag_record *, int, char *);
void add_help_reference(char *);
void rem_help_reference(char *);
void debug_help(int);
void reload_help_data();
void help_init();
#endif

#endif /* !_EGG_HELP_H */
