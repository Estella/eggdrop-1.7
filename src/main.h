/* main.h
 *
 * Copyright (C) 1997 Robey Pointer
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
 * $Id: main.h,v 1.6 2004/08/27 05:34:18 wcc Exp $
 */

#ifndef _EGG_MAIN_H
#define _EGG_MAIN_H

/* FIXME: *A LOT* of this stuff still needs to be moved out of main.h. */

#include "config.h"

#ifndef STDC_HEADERS
#  include "Error: Your system must have standard ANSI C headers."
#endif

#ifndef HAVE_VPRINTF
#  include "Error: You need vsprintf() to compile eggdrop."
#endif

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#  include <sys/param.h>
#endif

#ifdef TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
#else
#  ifdef HAVE_SYS_TIME_H
#    include <sys/time.h>
#  else
#    include <time.h>
#  endif
#endif

/* NAME_MAX is what POSIX defines, but BSD calls it MAXNAMLEN.
 * Use 255 if we can't find anything else.
 */
#ifndef NAME_MAX
#  ifdef MAXNAMLEN
#    define NAME_MAX MAXNAMLEN
#  else
#    define NAME_MAX 255
#  endif
#endif

/* This allows us to make things a lot less messy in modules.c. */
/* FIXME: move me to a better place. */
#ifndef STATIC
#  if !defined(MODULES_OK) || (!defined(MOD_USE_DL) && !defined(MOD_USE_SHL) && !defined(MOD_USE_DYLD) && !defined(MOD_USE_RLD) && !defined(MOD_USE_LOADER))
#    include "Error: You can't compile with module support on this system (try make static)."
#  else
#    ifdef MOD_USE_DL
#      ifndef HAVE_DLOPEN
#        include "Error: We have detected that dlopen() should be used to load modules on this OS; but it was not found. Please use 'make static'."
#      endif
#      undef MOD_USE_SHL
#      undef MOD_USE_DYLD
#      undef MOD_USE_RLD
#      undef MOD_USE_LOADER
#    endif
#    ifdef MOD_USE_SHL
#      ifndef HAVE_SHL_LOAD
#        include "Error: We have detected that shl_load() should be used to load modules on this OS; but it was not found. Please use 'make static'."
#      endif
#      undef MOD_USE_DL
#      undef MOD_USE_DYLD
#      undef MOD_USE_RLD
#      undef MOD_USE_LOADER
#    endif
#    ifdef MOD_USE_DYLD
#      ifndef HAVE_NSLINKMODULE
#        include "Error: We have detected that NSLinkModule() should be used to load modules on this OS; but it was not found. Please use 'make static'."
#      endif
#      undef MOD_USE_DL
#      undef MOD_USE_SHL
#      undef MOD_USE_RLD
#      undef MOD_USE_LOADER
#    endif
#    ifdef MOD_USE_RLD
#      ifndef HAVE_RLD_LOAD
#        include "Error: We have detected that rld_load() should be used to load modules on this OS; but it was not found. Please use 'make static'."
#      endif
#      undef MOD_USE_DL
#      undef MOD_USE_SHL
#      undef MOD_USE_DYLD
#      undef MOD_USE_LOADER
#    endif
#    ifdef MOD_USE_LOADER
#      ifndef HAVE_LOAD
#        include "Error: We have detected that load() should be used to load modules on this OS; but it was not found. Please use 'make static'."
#      endif
#      undef MOD_USE_DL
#      undef MOD_USE_SHL
#      undef MOD_USE_DYLD
#      undef MOD_USE_RLD
#    endif
#  endif
#endif

#include "lush.h"

#if ((TCL_MAJOR_VERSION > 7) || ((TCL_MAJOR_VERSION == 7) && (TCL_MINOR_VERSION >= 5)))
#  define USE_TCL_EVENTS
#  define USE_TCL_FINDEXEC
#  define USE_TCL_PACKAGE
#  define USE_TCL_VARARGS
#endif

#if (TCL_MAJOR_VERSION >= 8)
#  define USE_TCL_OBJ
#endif

#if (((TCL_MAJOR_VERSION == 8) && (TCL_MINOR_VERSION >= 1)) || (TCL_MAJOR_VERSION > 8))
#  define USE_TCL_BYTE_ARRAYS
#  define USE_TCL_ENCODING
#endif

#if (((TCL_MAJOR_VERSION == 8) && (TCL_MINOR_VERSION >= 4)) || (TCL_MAJOR_VERSION > 8))
#  ifdef CONST
#    define EGG_CONST CONST
#  else
#    define EGG_CONST
#  endif
#else
#  define EGG_CONST
#endif

/* UGH! Why couldn't Tcl pick a standard? */
#if defined(USE_TCL_VARARGS) && (defined(__STDC__) || defined(HAS_STDARG))
#  ifdef HAVE_STDARG_H
#    include <stdarg.h>
#  else
#    ifdef HAVE_STD_ARGS_H
#      include <std_args.h>
#    endif
#  endif
#  define EGG_VARARGS(type, name) (type name, ...)
#  define EGG_VARARGS_DEF(type, name) (type name, ...)
#  define EGG_VARARGS_START(type, name, list) (va_start(list, name), name)
#else
#  include <varargs.h>
#  define EGG_VARARGS(type, name) ()
#  define EGG_VARARGS_DEF(type, name) (va_alist) va_dcl
#  define EGG_VARARGS_START(type, name, list) (va_start(list), va_arg(list,type))
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#  include <strings.h>
#endif

#include <sys/types.h>

#include "lush.h"
#include "eggdrop.h"
#include "types.h" /* We need this basically everywhere. */
#include "debug.h" /* Allow debugging everywhere. */
#include "lang.h"
#include "flags.h"

#ifndef MAKING_MODS
#  include "proto.h"
#endif

#include "language.h"

#include "tclegg.h"
#include "tclhash.h"
#include "chan.h"
#include "users.h"

#include "compat/compat.h"

/* For pre Tcl7.5p1 versions */
#ifndef HAVE_TCL_FREE
#  define Tcl_Free(x) n_free(x, "", 0)
#endif

/* For pre7.6 Tcl versions */
#ifndef TCL_PATCH_LEVEL
#  define TCL_PATCH_LEVEL Tcl_GetVar(interp, "tcl_patchLevel", TCL_GLOBAL_ONLY)
#endif

/* FIXME: next 3 macros need to be in misc.h */
#ifndef HAVE_SRANDOM
#  define srandom(x) srand(x)
#endif

#ifndef HAVE_RANDOM
#  define random() (rand()/16)
#endif

#define fixcolon(x) do {                                                \
        if ((x)[0] == ':')                                              \
          (x)++;                                                        \
        else                                                            \
          (x) = newsplit(&(x));                                         \
} while (0)

/* This macro copies (_len - 1) bytes from _source to _target. The
 * target string is NULL-terminated.
 */
#define strncpyz(_target, _source, _len) do {                           \
        strncpy((_target), (_source), (_len) - 1);                      \
        (_target)[(_len) - 1] = 0;                                      \
} while (0)

#ifndef HAVE_SIGACTION
#  define sigaction sigvec
#  ifndef sa_handler
#    define sa_handler sv_handler
#    define sa_mask sv_mask
#    define sa_flags sv_flags
#  endif
#endif

#ifndef HAVE_SIGEMPTYSET
#  define sigemptyset(x) ((*(int *)(x))=0)
#endif

/* Use high-order bits for getting the random integer. With random()
 * modulo would probably be sufficient but on systems lacking random(),
 * the function will be just renamed rand().
 */
#define randint(n) (unsigned long) (random() / (RAND_MAX + 1.0) * ((n) < 0 ? (-(n)) : (n)))

#ifndef MAKING_MODS
void fatal(const char *, int);
int expected_memory(void);
void patch(const char *);
void backup_userfile(void);
#endif

#endif /* _EGG_MAIN_H */
