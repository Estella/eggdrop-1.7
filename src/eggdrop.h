/*
 * eggdrop.h
 *   Eggdrop compile-time settings
 *
 *   IF YOU ALTER THIS FILE, YOU NEED TO RECOMPILE THE BOT.
 *
 * $Id: eggdrop.h,v 1.3 2004/08/26 10:36:51 wcc Exp $
 */
/*
 * Copyright (C) 1997 Robey Pointer
 * Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004 Eggheads Development Team
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
 */

#ifndef _EGG_EGGDROP_H
#define _EGG_EGGDROP_H

/*
 * If you're *only* going to link to new version bots (1.3.0 or higher)
 * then you can safely define this.
 */
#undef NO_OLD_BOTNET

/*
 * Undefine this to completely disable context debugging.
 * WARNING: DO NOT send in bug reports if you undefine this!
 */
#define DEBUG_CONTEXT

/*
 * Set the following to the timestamp for the logfile entries.
 * Popular times might be "[%H:%M]" (hour, min), or "[%H:%M:%S]" (hour, min, sec)
 * Read `man strftime' for more formatting options.  Keep it below 32 chars.
 */
#define LOG_TS "[%H:%M]"

/*
 * HANDLEN note:
 *       HANDLEN defines the maximum length a handle on the bot can be.
 *       Standard (and minimum) is 9 characters long.
 *
 *       Beware that using lengths over 9 chars is 'non-standard' and if
 *       you wish to link to other bots, they _must_ both have the same
 *       maximum handle length.
 *
 * NICKMAX note:
 *       You should leave this at 32 characters and modify nick-len in the
 *       configuration file instead.
 */
#define HANDLEN 9   /* valid values 9->NICKMAX  */
#define NICKMAX 32  /* valid values HANDLEN->32 */


/* Handy string lengths */
#define UHOSTMAX    291 + NICKMAX /* 32 (ident) + 3 (\0, !, @) + NICKMAX */
#define DIRMAX      512           /* paranoia                            */
#define LOGLINEMAX  767           /* for misc.c/putlog() <cybah>         */

/* Invalid characters */
#define BADNICKCHARS "-,+*=:!.@#;$%&"
#define BADHANDCHARS "-,+*=:!.@#;$%&"


/* Language stuff */
#define LANGDIR  "./language" /* language file directory                   */
#define BASELANG "english"    /* language which always gets loaded before
                                 all other languages. You do not want to
                                 change this.                              */


/* The 'configure' script should make this next part automatic, so you
 * shouldn't need to adjust anything below.
 */
#define NICKLEN      NICKMAX + 1
#define UHOSTLEN     UHOSTMAX + 1
#define DIRLEN       DIRMAX + 1
#define LOGLINELEN   LOGLINEMAX + 1
#define NOTENAMELEN  ((HANDLEN * 2) + 1)


/* We have to generate compiler errors in a weird way since not all compilers
 * support the #error preprocessor directive. */
#ifndef STDC_HEADERS
#  include "Error: Your system must have standard ANSI C headers."
#endif

#ifndef HAVE_VPRINTF
#  include "Error: You need vsprintf to compile eggdrop."
#endif

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

/* This allows us to make things a lot less messy in modules.c. */
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

#if (NICKMAX < 9) || (NICKMAX > 32)
#  include "Error: Invalid NICKMAX value."
#endif

#if (HANDLEN < 9) || (HANDLEN > 32)
#  include "Error: Invalid HANDLEN value."
#endif

#if HANDLEN > NICKMAX
#  include "Error: HANDLEN MUST BE <= NICKMAX."
#endif

#ifdef HAVE_SYS_PARAM_H
#  include <sys/param.h>
#endif

/* NAME_MAX is what POSIX defines, but BSD calls it MAXNAMLEN.
 * Use 255 if we can't find anything else.
 */
#ifndef NAME_MAX
#  ifdef MAXNAMLEN
#    define NAME_MAX    MAXNAMLEN
#  else
#    define NAME_MAX    255
#  endif
#endif

/* Almost every module needs some sort of time thingy, so... */
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

#ifndef HAVE_SRANDOM
#  define srandom(x) srand(x)
#endif

#ifndef HAVE_RANDOM
#  define random() (rand()/16)
#endif

#ifndef HAVE_SOCKLEN_T
typedef int socklen_t;
#endif

/*
 *    Handy aliases for memory tracking and core dumps
 */

#define nmalloc(x)    n_malloc((x),__FILE__,__LINE__)
#define nrealloc(x,y) n_realloc((x),(y),__FILE__,__LINE__)
#define nfree(x)      n_free((x),__FILE__,__LINE__)

#ifdef DEBUG_CONTEXT
#  define Context           eggContext(__FILE__, __LINE__, NULL)
#  define ContextNote(note) eggContextNote(__FILE__, __LINE__, NULL, note)
#else
#  define Context           {}
#  define ContextNote(note) {}
#endif

#ifdef DEBUG_ASSERT
#  define Assert(expr) do {                                             \
          if (!(expr))                                                  \
            eggAssert(__FILE__, __LINE__, NULL);                        \
} while (0)
#else
#  define Assert(expr) do {                                             \
} while (0)
#endif

#ifndef COMPILING_MEM
#  undef malloc
#  define malloc(x) dont_use_old_malloc(x)
#  undef free
#  define free(x)   dont_use_old_free(x)
#endif /* !COMPILING_MEM */

#define debug0(x)             putlog(LOG_DEBUG,"*",x)
#define debug1(x,a1)          putlog(LOG_DEBUG,"*",x,a1)
#define debug2(x,a1,a2)       putlog(LOG_DEBUG,"*",x,a1,a2)
#define debug3(x,a1,a2,a3)    putlog(LOG_DEBUG,"*",x,a1,a2,a3)
#define debug4(x,a1,a2,a3,a4) putlog(LOG_DEBUG,"*",x,a1,a2,a3,a4)

/* These apparently are unsafe without recasting. */
#define egg_isdigit(x)  isdigit((int)  (unsigned char) (x))
#define egg_isxdigit(x) isxdigit((int) (unsigned char) (x))
#define egg_isascii(x)  isascii((int)  (unsigned char) (x))
#define egg_isspace(x)  isspace((int)  (unsigned char) (x))
#define egg_islower(x)  islower((int)  (unsigned char) (x))

/***********************************************************************/

/* chan & global */
#define FLOOD_PRIVMSG    0
#define FLOOD_NOTICE     1
#define FLOOD_CTCP       2
#define FLOOD_NICK       3
#define FLOOD_JOIN       4
#define FLOOD_KICK       5
#define FLOOD_DEOP       6

#define FLOOD_CHAN_MAX   7
#define FLOOD_GLOBAL_MAX 3

/* Structure for internal logs */
typedef struct {
  char *filename;
  unsigned int mask;            /* what to send to this log                 */
  char *chname;                 /* which channel                            */
  char szlast[LOGLINELEN];      /* for 'Last message repeated n times'
                                 * stuff in misc.c/putlog() <cybah>         */
  int repeats;                  /* number of times szLast has been repeated */
  unsigned int flags;           /* other flags <rtc>                        */
  FILE *f;                      /* existing file                            */
} log_t;

/* Logfile display flags
 */
#define LOG_MSGS     0x000001   /* m   msgs/notice/ctcps                */
#define LOG_PUBLIC   0x000002   /* p   public msg/notice/ctcps          */
#define LOG_JOIN     0x000004   /* j   channel joins/parts/etc          */
#define LOG_MODES    0x000008   /* k   mode changes/kicks/bans          */
#define LOG_CMDS     0x000010   /* c   user dcc or msg commands         */
#define LOG_MISC     0x000020   /* o   other misc bot things            */
#define LOG_BOTS     0x000040   /* b   bot notices                      */
#define LOG_RAW      0x000080   /* r   raw server stuff coming in       */
#define LOG_FILES    0x000100   /* x   file transfer commands and stats */
#define LOG_LEV1     0x000200   /* 1   user log level                   */
#define LOG_LEV2     0x000400   /* 2   user log level                   */
#define LOG_LEV3     0x000800   /* 3   user log level                   */
#define LOG_LEV4     0x001000   /* 4   user log level                   */
#define LOG_LEV5     0x002000   /* 5   user log level                   */
#define LOG_LEV6     0x004000   /* 6   user log level                   */
#define LOG_LEV7     0x008000   /* 7   user log level                   */
#define LOG_LEV8     0x010000   /* 8   user log level                   */
#define LOG_SERV     0x020000   /* s   server information               */
#define LOG_DEBUG    0x040000   /* d   debug                            */
#define LOG_WALL     0x080000   /* w   wallops                          */
#define LOG_SRVOUT   0x100000   /* v   server output                    */
#define LOG_BOTNET   0x200000   /* t   botnet traffic                   */
#define LOG_BOTSHARE 0x400000   /* h   share traffic                    */
#define LOG_ALL      0x7fffff   /* (dump to all logfiles)               */

/* Internal logfile flags
 */
#define LF_EXPIRING 0x000001    /* Logfile will be closed soon          */


/* For local console */
#define STDIN  0
#define STDOUT 1
#define STDERR 2


/* Return codes for add_note */
#define NOTE_ERROR      0       /* error                        */
#define NOTE_OK         1       /* success                      */
#define NOTE_STORED     2       /* not online; stored           */
#define NOTE_FULL       3       /* too many notes stored        */
#define NOTE_TCL        4       /* tcl binding caught it        */
#define NOTE_AWAY       5       /* away; stored                 */
#define NOTE_FWD        6       /* away; forwarded              */
#define NOTE_REJECT     7       /* ignore mask matched          */

#define STR_PROTECT     2
#define STR_DIR         1

#define HELP_DCC        1
#define HELP_TEXT       2
#define HELP_IRC        16

#endif /* _EGG_EGGDROP_H */
