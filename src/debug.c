/* debug.c
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
 * $Id: debug.c,v 1.7 2005/08/22 03:32:33 wcc Exp $
 */

#include "main.h"
#include "bg.h"

#include <sys/stat.h>
#if HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <fcntl.h>
#include "stat.h"

#include "debug.h"
#include "dcc.h"     /* SOCK_* */
#include "dccutil.h" /* dprintf, tell_dcc */
#include "logfile.h" /* putlog, LOG_* */
#include "mem.h"     /* debug_mem_to_dcc */
#include "misc.h"    /* strncpyz */
#include "net.h"     /* setsock, killsock */


/* Did the bot crash IN write_debug()? */
static int nested_debug = 0;

extern char ver[], egg_xtra[];
extern time_t now;
extern Tcl_Interp *interp;


void write_debug()
{
  int x;
  char s[25];

  if (nested_debug) {
    /* We had a crash inside write_debug()... ouch. */
    x = creat("DEBUG.DEBUG", 0644);
    setsock(x, SOCK_NONSOCK);
    if (x >= 0) {
      strncpyz(s, ctime(&now), sizeof s);
      dprintf(-x, "Debug (%s) written %s\n", ver, s);
      dprintf(-x, "Please report problem to bugs@eggheads.org\n");
      dprintf(-x, "after a visit to http://www.eggheads.org/bugzilla/\n");
      dprintf(-x, "Full Patch List: %s\n", egg_xtra);
      killsock(x);
      close(x);
    }
    bg_send_quit(BG_ABORT);
    exit(1); /* Dont even tell people about; it may have caused the fault last time. */
  } else {
    nested_debug = 1;
  }

  putlog(LOG_MISC, "*", "* Please REPORT this BUG!");
  putlog(LOG_MISC, "*", "* See doc/BUG-REPORT for how to do so.");

  x = creat("DEBUG", 0644);
  setsock(x, SOCK_NONSOCK);
  if (x < 0) {
    putlog(LOG_MISC, "*", "* Failed to write DEBUG.");
  } else {
    strncpyz(s, ctime(&now), sizeof s);
    dprintf(-x, "Debug (%s) written %s\n", ver, s);
    dprintf(-x, "Full Patch List: %s\n", egg_xtra);
#ifdef STATIC
    dprintf(-x, "STATICALLY LINKED\n");
#endif
    dprintf(-x, "Tcl library: %s\n",
            (interp && (Tcl_Eval(interp, "info library") == TCL_OK)) ?
            interp->result : "*unknown*");

    dprintf(-x, "Tcl version: %s (header version %s)\n",
            (interp && (Tcl_Eval(interp, "info patchlevel") == TCL_OK)) ?
            interp->result : (Tcl_Eval(interp, "info tclversion") == TCL_OK) ?
            interp->result : "*unknown*", TCL_PATCH_LEVEL ? TCL_PATCH_LEVEL :
            "*unknown*");
#ifdef HAVE_TCL_THREADS
    dprintf(-x, "Tcl is threaded.\n");
#endif
#ifdef CCFLAGS
    dprintf(-x, "Compile flags: %s\n", CCFLAGS);
#endif
#ifdef LDFLAGS
    dprintf(-x, "Link flags: %s\n", LDFLAGS);
#endif
#ifdef STRIPFLAGS
    dprintf(-x, "Strip flags: %s\n", STRIPFLAGS);
#endif
    tell_dcc(-x);
    dprintf(-x, "\n");
    debug_mem_to_dcc(-x);
    killsock(x);
    close(x);
    putlog(LOG_MISC, "*", "* Wrote DEBUG.");
  }
}

#ifdef DEBUG_ASSERT
/* Called from the Assert macro. */
void eggAssert(const char *file, int line, const char *module)
{
  write_debug();

  if (!module) {
    putlog(LOG_MISC, "*", "* In file %s, line %u", file, line);
  }
  else {
    putlog(LOG_MISC, "*", "* In file %s:%s, line %u", module, file, line);
  }

  fatal("ASSERT FAILED -- CRASHING!", 1);
}
#endif /* DEBUG_ASSERT */
