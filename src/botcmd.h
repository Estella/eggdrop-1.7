/* botcmd.h
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
 * $Id: botcmd.h,v 1.4 2005/01/21 01:43:39 wcc Exp $
 */

#ifndef _EGG_BOTCMD_H
#define _EGG_BOTCMD_H

#include "types.h" /* Function */

typedef struct {
  char *name;
  Function func;
} botcmd_t;


#ifndef MAKING_MODS
void bot_share(int, char *);
#endif

#endif /* !_EGG_BOTCMD_H */
