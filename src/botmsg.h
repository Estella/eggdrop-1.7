/* botmsg.h: prototypes for functions in botmsg.c
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
 * $Id: botmsg.h,v 1.1 2004/08/25 06:39:38 wcc Exp $
 */

#ifndef _EGG_BOTMSG_H
#define _EGG_BOTMSG_H

int add_note(char *, char *, char *, int, int);
int simple_sprintf EGG_VARARGS(char *, arg1);
void tandout_but EGG_VARARGS(int, arg1);
char *int_to_base10(int);
char *unsigned_int_to_base10(unsigned int);
char *int_to_base64(unsigned int);
int base64_to_int(char *);

#endif /* !_EGG_BOTMSG_H */
