/* chanprog.h
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
 * $Id: chanprog.h,v 1.2 2004/10/06 00:04:32 wcc Exp $
 */

#ifndef _EGG_CHANPROG_H
#define _EGG_CHANPROG_H

#ifndef MAKING_MODS
memberlist *ismember(struct chanset_t *, char *);
struct chanset_t *findchan(const char *name);
struct chanset_t *findchan_by_dname(const char *name);
int isowner(char *);
void reaffirm_owners();
#endif

#endif /* !_EGG_CHANPROG_H */
