/* types.h
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
 * $Id: types.h,v 1.1 2004/08/26 03:21:14 wcc Exp $
 */

#ifndef _EGG_TYPES_H
#define _EGG_TYPES_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef NULL
#  define NULL 0
#endif

typedef int (*Function) ();

typedef struct {
  char *name;
  char *flags;
  Function func;
  char *funcname;
} cmd_t;

/* 32 bit type */
#if (SIZEOF_INT == 4)
typedef unsigned int u_32bit_t;
#else
#  if (SIZEOF_LONG == 4)
typedef unsigned long u_32bit_t;
#  else
#    include "Error: Can't find 32bit type."
#  endif
#endif

typedef unsigned short int u_16bit_t; /* 16 bit type */
typedef unsigned char u_8bit_t;       /* 8 bit type  */

typedef u_32bit_t IP; /* IP type */

#endif /* _EGG_TYPES_H */
