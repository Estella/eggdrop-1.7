/* userfile.h
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
 * $Id: userfile.h,v 1.3 2006/11/20 13:53:36 tothwolf Exp $
 */

#ifndef _EGG_USERFILE_H
#define _EGG_USERFILE_H

#ifndef MAKING_MODS
void writeuserfile(int);
void reloaduserfile();
void backupuserfile();
int readuserfile(char *, struct userrec **);
int write_user(struct userrec *, FILE *, int);
int write_ignores(FILE *, int);
#endif

#endif /* !_EGG_USERFILE_H */
