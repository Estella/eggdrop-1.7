/* userent.h
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
 * $Id: userent.h,v 1.2 2004/08/27 10:01:17 wcc Exp $
 */

#ifndef _EGG_USERENT_H
#define _EGG_USERENT_H

#ifndef MAKING_MODS
void list_type_kill(struct list_type *);
int list_type_expmem(struct list_type *);
int xtra_set();
#endif

#endif /* !_EGG_USERENT_H */
