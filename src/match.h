/* match.h
 *
 * Copyright (C) 2004 - 2005 Eggheads Development Team
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
 * $Id: match.h,v 1.3 2005/01/21 01:43:40 wcc Exp $
 */

#ifndef _EGG_MATCH_H
#define _EGG_MATCH_H

/* Quoting character (overrides wildcards). */
#define WILDQUOTE '\\'

#define WILDS '*'  /* Matches 0 or more characters (including spaces). */
#define WILDP '%'  /* Matches 0 or more non-space characters.          */
#define WILDQ '?'  /* Matches ecactly one character.                   */
#define WILDT '~'  /* Matches 1 or more spaces.                        */

#define NOMATCH  0
#define MATCH    (match+sofar)
#define PERMATCH (match+saved+sofar)

#ifndef MAKING_MODS
/* FIXME: this is ugly. */
#  define wild_match(a,b) _wild_match((unsigned char *)(a),(unsigned char *)(b))
#  define wild_match_per(a,b) _wild_match_per((unsigned char *)(a),(unsigned char *)(b))
int _wild_match(register unsigned char *, register unsigned char *);
int _wild_match_per(register unsigned char *, register unsigned char *);
#endif

#endif /* !_EGG_MATCH_H */
