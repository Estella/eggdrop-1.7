/* debug.c
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
 * $Id: debug.c,v 1.1 2004/08/27 05:34:18 wcc Exp $
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
#include "net.h"     /* setsock, killsock */


#ifdef DEBUG_CONTEXT
/* Did the bot crash IN write_debug()? */
static int nested_debug = 0;

/* Context storage for fatal crashes */
char cx_file[16][30], cx_note[16][256];
int cx_line[16], cx_ptr = 0;


extern char ver[], egg_xtra[];
extern time_t now;
extern Tcl_Interp *interp;


void write_debug()
{
  int x, y;
  char s[25];

  if (nested_debug) {
    /* Yoicks, if we have this there's serious trouble!
     * All of these are pretty reliable, so we'll try these.
     *
     * NOTE: dont try and display context-notes in here, it's
     *       _not_ safe <cybah>
     */
    x = creat("DEBUG.DEBUG", 0644);
    setsock(x, SOCK_NONSOCK);
    if (x >= 0) {
      strncpyz(s, ctime(&now), sizeof s);
      dprintf(-x, "Debug (%s) written %s\n", ver, s);
      dprintf(-x, "Please report problem to bugs@eggheads.org\n");
      dprintf(-x, "after a visit to http://www.eggheads.org/bugzilla/\n");
      dprintf(-x, "Full Patch List: %s\n", egg_xtra);
      dprintf(-x, "Context: ");
      cx_ptr = cx_ptr & 15;
      for (y = ((cx_ptr + 1) & 15); y != cx_ptr; y = ((y + 1) & 15))
        dprintf(-x, "%s/%d,\n         ", cx_file[y], cx_line[y]);
      dprintf(-x, "%s/%d\n\n", cx_file[y], cx_line[y]);
      killsock(x);
      close(x);
    }
    bg_send_quit(BG_ABORT);
    exit(1); /* Dont even tell people about; it may have caused the fault last time. */
  } else {
    nested_debug = 1;
  }

  putlog(LOG_MISC, "*", "* Last context: %s/%d [%s]", cx_file[cx_ptr],
         cx_line[cx_ptr], cx_note[cx_ptr][0] ? cx_note[cx_ptr] : "");
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
    dprintf(-x, "Context: ");
    cx_ptr = cx_ptr & 15;
    for (y = ((cx_ptr + 1) & 15); y != cx_ptr; y = ((y + 1) & 15))
      dprintf(-x, "%s/%d, [%s]\n         ", cx_file[y], cx_line[y],
              (cx_note[y][0]) ? cx_note[y] : "");
    dprintf(-x, "%s/%d [%s]\n\n", cx_file[cx_ptr], cx_line[cx_ptr],
            (cx_note[cx_ptr][0]) ? cx_note[cx_ptr] : "");
    tell_dcc(-x);
    dprintf(-x, "\n");
    debug_mem_to_dcc(-x);
    killsock(x);
    close(x);
    putlog(LOG_MISC, "*", "* Wrote DEBUG.");
  }
}

/* Called from the Context macro. */
void eggContext(const char *file, int line, const char *module)
{
  char x[31], *p;

  p = strrchr(file, '/');
  if (!module)
    strncpyz(x, p ? p + 1 : file, sizeof x);
  else
    egg_snprintf(x, 31, "%s:%s", module, p ? p + 1 : file);
  cx_ptr = ((cx_ptr + 1) & 15);
  strcpy(cx_file[cx_ptr], x);
  cx_line[cx_ptr] = line;
  cx_note[cx_ptr][0] = 0;
}

/* Called from the ContextNote macro. */
void eggContextNote(const char *file, int line, const char *module,
                    const char *note)
{
  char x[31], *p;

  p = strrchr(file, '/');
  if (!module)
    strncpyz(x, p ? p + 1 : file, sizeof x);
  else
    egg_snprintf(x, 31, "%s:%s", module, p ? p + 1 : file);
  cx_ptr = ((cx_ptr + 1) & 15);
  strcpy(cx_file[cx_ptr], x);
  cx_line[cx_ptr] = line;
  strncpyz(cx_note[cx_ptr], note, sizeof cx_note[cx_ptr]);
}
#endif /* DEBUG_CONTEXT */


#ifdef DEBUG_ASSERT
/* Called from the Assert macro. */
void eggAssert(const char *file, int line, const char *module)
{
#  ifdef DEBUG_CONTEXT
  write_debug();
#  endif
  if (!module)
    putlog(LOG_MISC, "*", "* In file %s, line %u", file, line);
  else
    putlog(LOG_MISC, "*", "* In file %s:%s, line %u", module, file, line);
  fatal("ASSERT FAILED -- CRASHING!", 1);
}
#endif /* DEBUG_ASSERT */
