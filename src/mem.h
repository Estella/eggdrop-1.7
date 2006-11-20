/* mem.h
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
 * $Id: mem.h,v 1.3 2006/11/20 13:53:34 tothwolf Exp $
 */

#ifndef _EGG_MEM_H
#define _EGG_MEM_H


#define MAX_MEM    12    /* Max number of expmem functions to collect from. */
#define MEMTBLSIZE 25000 /* Size of memory table. */

struct memory_table {
  void *ptr;
  int size;
  short line;
  char file[20];
};

#ifndef MAKING_MODS
void tell_mem_status(int);
void debug_mem_to_dcc(int);
int expected_memory();

void *n_malloc(int, const char *, int);
void *n_realloc(void *, int, const char *, int);
void n_free(void *, const char *, int);

#  define nmalloc(x)    n_malloc((x),__FILE__,__LINE__)
#  define nrealloc(x,y) n_realloc((x),(y),__FILE__,__LINE__)
#  define nfree(x)      n_free((x),__FILE__,__LINE__)

int expmem_chanprog();
int expmem_fileq();
int expmem_users();
int expmem_dccutil();
int expmem_botnet();
int expmem_tcl();
int expmem_tclhash();
int expmem_net();
int expmem_modules();
int expmem_language();
int expmem_tcldcc();
int expmem_dns();
#endif

#endif /* !_EGG_MEM_H */
