/* misc.h
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
 * $Id: misc.h,v 1.5 2004/10/06 00:04:33 wcc Exp $
 */

#ifndef _EGG_MISC_H
#define _EGG_MISC_H

/* Flags for days(). */
#define DAYS_IN  1
#define DAYS_AGO 2
#define DAYS_FOR 3

/* Flags for maskhost(). */
#define MASKHOST_BAN  0
#define MASKHOST_HOST 1

/* Removes a leading ':' from '_x' if it exists; otherwise assign '_x' to
 * newsplit(&_x);
 */
#define fixcolon(_x) do {         \
        if ((_x)[0] == ':')       \
          (_x)++;                 \
        else                      \
          (_x) = newsplit(&(_x)); \
} while (0)

/* This macro copies (_len - 1) bytes from _source to _target. The
 * target string is NUL-terminated.
 */
#define strncpyz(_target, _source, _len) do {      \
        strncpy((_target), (_source), (_len) - 1); \
        (_target)[(_len) - 1] = 0;                 \
} while (0)


/* Use high-order bits for getting the random integer. With random()
 * modulo would probably be sufficient but on systems lacking random(),
 * the function will be just renamed rand().
 */
#define randint(n) (unsigned long) (random() / (RAND_MAX + 1.0) * ((n) < 0 ? (-(n)) : (n)))

#ifndef MAKING_MODS
int my_strcpy(char *, char *);
void splitc(char *, char *, char);
void splitcn(char *, char *, char, size_t);
char *newsplit(char **);
void rmspace(char *);
void maskhost(const char *, char *, int);
char *splitnick(char **);
void dumplots(int, const char *, char *);
void days(time_t, time_t, char *, int);
char *extracthostname(char *);
void make_rand_str(char *, int);
int oatoi(const char *);
char *str_escape(const char *, const char, const char);
char *strchr_unescape(char *, const char, register const char);
void str_unescape(char *, register const char);
int str_isdigit(const char *);
void kill_bot(char *, char *);
#endif

#endif /* !_EGG_MISC_H */
