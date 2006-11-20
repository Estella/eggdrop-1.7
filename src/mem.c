/* mem.c: memory allocation, tracking, debugging, etc.
 *
 * Copyright (C) 1997 Robey Pointer
 * Copyright (C) 1999 - 2006 Eggheads Development Team
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
 * $Id: mem.c,v 1.11 2006/11/20 13:53:34 tothwolf Exp $
 */

#define COMPILING_MEM

#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "mod/modvals.h"

#include "mem.h"
#include "dccutil.h" /* dprintf */
#include "help.h"    /* help_expmem */
#include "logfile.h" /* logfile_expmem */

extern module_entry *module_list;


#ifdef DEBUG_MEM
unsigned long memused = 0;
static int lastused = 0;
struct memory_table memtbl[MEMTBLSIZE];
#endif


/* Initialize the memory structure. */
void init_mem()
{
#ifdef DEBUG_MEM
  int i;

  for (i = 0; i < MEMTBLSIZE; i++)
    memtbl[i].ptr = NULL;
#endif
}

/* Return expected memoray usage. */
int expected_memory()
{
  return (expmem_users() + expmem_dccutil() + expmem_botnet() + expmem_tcl() +
    expmem_tclhash() + expmem_net() + expmem_modules(0) + expmem_language() +
    expmem_tcldcc() + logfile_expmem() + help_expmem());
}

/* Tell memory table status to an IDX. */
void tell_mem_status(int idx)
{
#ifdef DEBUG_MEM
  int expected;
  float percent;

  expected = expected_memory();
  percent = ((lastused * 1.0) / (MEMTBLSIZE * 1.0)) * 100.0;
  dprintf(idx, "Memory table: %d/%d (%.1f%% full).\n", lastused, MEMTBLSIZE,
          percent);
  percent = ((expected * 1.0) / (memused * 1.0)) * 100.0;
  if (percent != 100.0)
    dprintf(idx, "Memory fault: only accounting for %d/%ld (%.1f%%).\n",
            expected, memused, percent);
  dprintf(idx, "Memory table itself occupies an additional %dk static.\n",
          (int) (sizeof(memtbl) / 1024));
#endif
}

/* Tell memory debugging information to an IDX. */
void debug_mem_to_dcc(int idx)
{
#ifdef DEBUG_MEM
  module_entry *me;
  unsigned long expected[MAX_MEM], using[MAX_MEM];
  unsigned long l;
  char filename[20], sofar[81], *p;
  int i, j;

  /* Initialize the expected memory array. */
  expected[0]  = expmem_language();
  expected[1]  = expmem_users();
  expected[2]  = expmem_net();
  expected[3]  = expmem_dccutil();
  expected[4]  = expmem_botnet();
  expected[5]  = expmem_tcl();
  expected[6]  = expmem_tclhash();
  expected[7]  = expmem_modules(1);
  expected[8]  = expmem_tcldcc();
  expected[9]  = expmem_dns();
  expected[10] = help_expmem();
  expected[11] = logfile_expmem();

  for (me = module_list; me; me = me->next)
    me->mem_work = 0;

  for (i = 0; i < MAX_MEM; i++)
    using[i] = 0;

  for (i = 0; i < lastused; i++) {
    strcpy(filename, memtbl[i].file);
    p = strchr(filename, ':');
    if (p)
      *p = 0;
    l = memtbl[i].size;
    if (!strcmp(filename, "language.c"))
      using[0] += l;
    else if (!strcmp(filename, "userrec.c"))
      using[1] += l;
    else if (!strcmp(filename, "net.c"))
      using[0] += l;
    else if (!strcmp(filename, "dccutil.c"))
      using[3] += l;
    else if (!strcmp(filename, "botnet.c"))
      using[4] += l;
    else if (!strcmp(filename, "tcl.c"))
      using[5] += l;
    else if (!strcmp(filename, "tclhash.c"))
      using[6] += l;
    else if (!strcmp(filename, "modules.c"))
      using[7] += l;
    else if (!strcmp(filename, "tcldcc.c"))
      using[8] += l;
    else if (!strcmp(filename, "dns.c"))
      using[9] += l;
    else if (!strcmp(filename, "help.c"))
      using[10] += l;
    else if (!strcmp(filename, "logfile.c"))
      using[11] += l;
    else if (p) {
      for (me = module_list; me; me = me->next) {
        if (!strcmp(filename, me->name))
          me->mem_work += l;
      }
    }
    else
      dprintf(idx, "Not logging file '%s'!\n", filename);
  }

  for (i = 0; i < MAX_MEM; i++) {
    switch (i) {
      case 0:
        strcpy(filename, "language.c");
        break;
      case 1:
        strcpy(filename, "userrec.c");
        break;
      case 2:
        strcpy(filename, "net.c");
        break;
      case 3:
        strcpy(filename, "dccutil.c");
        break;
      case 4:
        strcpy(filename, "botnet.c");
        break;
      case 5:
        strcpy(filename, "tcl.c");
        break;
      case 6:
        strcpy(filename, "tclhash.c");
        break;
      case 7:
        strcpy(filename, "modules.c");
        break;
      case 8:
        strcpy(filename, "tcldcc.c");
        break;
      case 9:
        strcpy(filename, "dns.c");
        break;
      case 10:
        strcpy(filename, "help.c");
        break;
      case 11:
        strcpy(filename, "logfile.c");
        break;
    }

    if (using[i] == expected[i])
      dprintf(idx, "File '%-10s': accounted for %lu/%lu (ok).\n",
              filename, expected[i], using[i]);
    else {
      dprintf(idx, "File '%-10s': accounted for %lu/%lu (debug follows):\n",
              filename, expected[i], using[i]);
      strcpy(sofar, "   ");
      for (j = 0; j < lastused; j++) {
        p = strchr(memtbl[j].file, ':');
        if (p != NULL)
          *p = 0;
        if (!egg_strcasecmp(memtbl[j].file, filename)) {
          if (p != NULL)
            sprintf(&sofar[strlen(sofar)], "%-10s/%-4d:(%04d) ",
                    p + 1, memtbl[j].line, memtbl[j].size);
          else
            sprintf(&sofar[strlen(sofar)], "%-4d:(%04d) ",
                    memtbl[j].line, memtbl[j].size);

          if (strlen(sofar) > 60) {
            sofar[strlen(sofar) - 1] = 0;
            dprintf(idx, "%s\n", sofar);
            strcpy(sofar, "   ");
          }
        }
        if (p != NULL)
          *p = ':';
      }
      if (sofar[0]) {
        sofar[strlen(sofar) - 1] = 0;
        dprintf(idx, "%s\n", sofar);
      }
    }
  }

  for (me = module_list; me; me = me->next) {
    Function *f = me->funcs;
    int expt = 0;

    if (f != NULL && f[MODCALL_EXPMEM] != NULL)
      expt = f[MODCALL_EXPMEM] ();

    if (me->mem_work == expt)
      dprintf(idx, "Module '%-10s': accounted for %lu/%lu (ok).\n", me->name,
              expt, me->mem_work);
    else {
      dprintf(idx, "Module '%-10s': accounted for %lu/%lu (debug follows):\n",
              me->name, expt, me->mem_work);
      strcpy(sofar, "   ");
      for (j = 0; j < lastused; j++) {
        strcpy(filename, memtbl[j].file);
        p = strchr(filename, ':');
        if (p != NULL) {
          *p = 0;
          if (!egg_strcasecmp(filename, me->name)) {
            sprintf(&sofar[strlen(sofar)], "%-10s/%-4d:(%04X) ", p + 1,
                    memtbl[j].line, memtbl[j].size);
            if (strlen(sofar) > 60) {
              sofar[strlen(sofar) - 1] = 0;
              dprintf(idx, "%s\n", sofar);
              strcpy(sofar, "   ");
            }
            *p = ':';
          }
        }
      }
      if (sofar[0]) {
        sofar[strlen(sofar) - 1] = 0;
        dprintf(idx, "%s\n", sofar);
      }
    }
  }
#else
  dprintf(idx, "Compiled without extensive memory debugging (sorry).\n");
#endif
}

void *n_malloc(int size, const char *file, int line)
{
  void *x;
#ifdef DEBUG_MEM
  char *p;
  int i = 0;
#endif

  x = (void *) malloc(size);
  if (x == NULL) {
    putlog(LOG_MISC, "*", "*** FAILED MALLOC %s (%d) (%d): %s.", file, line,
           size, strerror(errno));
    fatal("Memory allocation failed.", 0);
  }

#ifdef DEBUG_MEM
  if (lastused == MEMTBLSIZE) {
    putlog(LOG_MISC, "*", "*** MEMORY TABLE FULL: %s (%d).", file, line);
    fatal("Memory table full.", 0);
  }
  i = lastused;
  memtbl[i].ptr = x;
  memtbl[i].line = line;
  memtbl[i].size = size;
  p = strrchr(file, '/');
  strncpy(memtbl[i].file, p ? p + 1 : file, 19);
  memtbl[i].file[19] = 0;
  memused += size;
  lastused++;
#endif

  return x;
}

void *n_realloc(void *ptr, int size, const char *file, int line)
{
  void *x;
#ifdef DEBUG_MEM
  char *p;
#endif
  int i = 0;

  /* ptr == NULL is valid. Avoiding duplicate code further down. */
  if (!ptr)
    return n_malloc(size, file, line);

  x = (void *) realloc(ptr, size);
  if (x == NULL && size > 0) {
    i = i;
    putlog(LOG_MISC, "*", "*** FAILED REALLOC %s (%d).", file, line);
    return NULL;
  }

#ifdef DEBUG_MEM
  for (i = 0; i < lastused && memtbl[i].ptr != ptr; i++);
  if (i == lastused) {
    putlog(LOG_MISC, "*", "*** ATTEMPTING TO REALLOC NON-MALLOC'D POINTER:"
           " %s (%d).", file, line);
    return NULL;
  }
  memused -= memtbl[i].size;
  memtbl[i].ptr = x;
  memtbl[i].line = line;
  memtbl[i].size = size;
  p = strrchr(file, '/');
  strncpy(memtbl[i].file, p ? p + 1 : file, 19);
  memtbl[i].file[19] = 0;
  memused += size;
#endif

  return x;
}

void n_free(void *ptr, const char *file, int line)
{
  int i = 0;

  if (ptr == NULL) {
    putlog(LOG_MISC, "*", "*** ATTEMPTING TO FREE NULL POINTER: %s (%d).",
           file, line);
    i = i; /* WTF? */
    return;
  }

#ifdef DEBUG_MEM
  /* Give tcl builtins an escape mechanism */
  if (line) {
    for (i = 0; i < lastused && memtbl[i].ptr != ptr; i++);
    if (i == lastused) {
      putlog(LOG_MISC, "*", "*** ATTEMPTING TO FREE NON-MALLOC'D POINTER: %s (%d).",
             file, line);
      return;
    }
    memused -= memtbl[i].size;
    lastused--;
    /* We don't want any holes, so if this wasn't the last entry, swap it. */
    if (i != lastused) {
      memtbl[i].ptr = memtbl[lastused].ptr;
      memtbl[i].size = memtbl[lastused].size;
      memtbl[i].line = memtbl[lastused].line;
      strcpy(memtbl[i].file, memtbl[lastused].file);
    }
  }

#endif
  free(ptr);
}
