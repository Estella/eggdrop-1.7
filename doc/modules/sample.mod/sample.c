/* sample.c
 * 
 * Originally written by ButchBub         15 July     1997
 * Comments by Fabian Knittel             29 December 1999
 *
 * Copyright (C) 1999-2004 Eggheads Development Team
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
 * $Id: sample.c,v 1.1 2004/12/02 22:15:52 wcc Exp $
 */

#define MODULE_NAME "sample"
#define MAKING_SAMPLE

/* This file needs to be included by all modules. */
#include "src/mod/module.h"

/* This is a pointer to the Eggdrop global function table. It gets initialized
 * below in sample_start(). */
#undef global
static Function *global = NULL;

/* Prototype for sample_start() below. */
EXPORT_SCOPE char *sample_start();


/* Calculate the memory we keep allocated. This module doesn't allocate any
 * memory, but since an expmem function needs to exist for each module, we
 * just return 0 here. */
static int sample_expmem()
{
  return 0;
}

/* This is a sample partyline command. 'u' is the userrec of the user using
 * the command, 'idx' is that user's idx, and 'par' contains the arguments
 * issued after the command. */
static int cmd_sample(struct userrec *u, int idx, char *par)
{
  /* Log the command as soon as you're sure all parameters are valid. */
  putlog(LOG_CMDS, "*", "#%s# sample", dcc[idx].nick);

  /* This prints "I am only a sample. I don't really do anything :(." to
   * the console of the user issuing the command. */
  dprintf(idx, "I am only a sample. I don't really do anything :(.\n");

  return 0;
}

/* This shows the module's status when the '.status' partyline command is
 * called. 'details' will be 1 if '.status all' is used, or 0 if '.status'
 * was used. */
static void sample_report(int idx, int details)
{
  if (details) {
    int size = sample_expmem();

    dprintf(idx, "    Using %d byte%s of memory.\n", size,
            (size != 1) ? "s" : "");
  }
}

/* NOTE:
 *   The tcl-name is automatically created if you set it to NULL. In the
 *   example below, it would be "*dcc:sample". If you specify "sample:sample"
 *   it would be "*dcc:sample:sample" instead. */
static cmd_t mydcc[] = {
  /* command name  required flags    function to call     tcl-name */
  {"sample",       "",               cmd_sample,          NULL},

  /* Always end command tables with a NULL entry. */
  {NULL,      NULL,  NULL,        NULL}
};


/* This function is called when the module is unloaded. */
static char *sample_close()
{
  /* Remove the partyline commands we added when the module is unloaded. */
  rem_builtins(H_dcc, mydcc);

  module_undepend(MODULE_NAME);
  return NULL;
}

/* This function table is exported and may be used by other modules and
 * the core.
 *
 * The first four have to be defined (you may define them as 0 or NULL),
 * as they are used by eggdrop core.
 */
static Function sample_table[] = {
  (Function) sample_start,
  (Function) sample_close,
  (Function) sample_expmem,
  (Function) sample_report,
};

/* This function is called as the module is loaded. */
char *sample_start(Function *global_funcs)
{
  /* Assign the global function table. After this point, you can use all
   * normal functions defined in src/mod/modules.h. */
  global = global_funcs;

  /* Register the module. */
  module_register(MODULE_NAME, sample_table, 2, 0);
  /*                                            ^--- minor module version
   *                                         ^------ major module version
   *                           ^-------------------- module function table
   *              ^--------------------------------- module name
   */

  /* Depend on the Eggdrop core, version 1.7.0 or later. */
  if (!module_depend(MODULE_NAME, "eggdrop", 107, 0)) {
    module_undepend(MODULE_NAME);
    return "This module requires Eggdrop 1.7.0 or later.";
  }

  /* Add command table to bind list H_dcc, responsible for DCC/partyline
   * commands. Currently we only add one command, 'sample'. */
  add_builtins(H_dcc, mydcc);

  /* Return NULL unless we're passing an error message. */
  return NULL;
}
