/* debug.h
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
 * $Id: debug.h,v 1.4 2005/08/22 03:32:33 wcc Exp $
 */

#ifndef _EGG_DEBUG_H
#define _EGG_DEBUG_H

/* Debug log macros. */
#define debug0(_x)                 putlog(LOG_DEBUG, "*", (_x))
#define debug1(_x,_a1)             putlog(LOG_DEBUG, "*", (_x), (_a1))
#define debug2(_x,_a1,_a2)         putlog(LOG_DEBUG, "*", (_x), (_a1), (_a2))
#define debug3(_x,_a1,_a2,_a3)     putlog(LOG_DEBUG, "*", (_x), (_a1), (_a2), (_a3))
#define debug4(_x,_a1,_a2,_a3,_a4) putlog(LOG_DEBUG, "*", (_x), (_a1), (_a2), (_a3), (_a4))

#ifdef DEBUG_ASSERT
#  define Assert(_expr) do {                      \
          if (!(_expr))                           \
            eggAssert(__FILE__, __LINE__, NULL);  \
} while (0)
#else
#  define Assert(_expr) do {                      \
} while (0)
#endif

#ifndef MAKING_MODS
void write_debug();
#  ifdef DEBUG_ASSERT
void eggAssert(const char *, int, const char *);
#  endif
#endif

#endif /* !_EGG_DEBUG_H */
