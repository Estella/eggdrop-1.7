/* language.h: prototypes for functions in language.c
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
 * $Id: language.h,v 1.1 2004/08/25 07:41:36 wcc Exp $
 */

#ifndef _EGG_LANGUAGE_H
#define _EGG_LANGUAGE_H

char *get_language(int);
int cmd_loadlanguage(struct userrec *, int, char *);
void add_lang_section(char *);
int del_lang_section(char *);
int exist_lang_section(char *);

#endif /* !_EGG_LANGUAGE_H */