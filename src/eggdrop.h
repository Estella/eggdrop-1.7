/* eggdrop.h: compile-time settings
 *
 * IF YOU ALTER THIS FILE, YOU NEED TO RECOMPILE THE BOT.
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
 * $Id: eggdrop.h,v 1.5 2004/08/27 05:34:18 wcc Exp $
 */

#ifndef _EGG_EGGDROP_H
#define _EGG_EGGDROP_H

/* If you're *only* going to link to new version bots (1.3.0 or higher), then
 * then you can safely define this.
 */
#undef NO_OLD_BOTNET

/* Set the following to the timestamp for the logfile entries. Popular times
 * might be "[%H:%M]" (hour, min), or "[%H:%M:%S]" (hour, min, sec). Read
 * 'man strftime' for more formatting options. Keep it below 32 characters.
 */
#define LOG_TS  "[%H:%M]"

/* HANDLEN defines the maximum length a handle on the bot can be. Standard
 * (and minimum) is 9 characters long.
 *
 * Beware that using lengths over 9 chars is 'non-standard', and if you wish
 * to link to other bots, they _must_ both have the same maximum handle length.
 */
#define HANDLEN  9 /* Valid values are 9 to NICKMAX. */

/* You should leave this at 32 characters and modify nick-len in the
 * configuration file instead.
 */
#define NICKMAX  32  /* Valid values are HANDLEN to 32. */



/* Handy string lengths. */
#define UHOSTMAX    291 + NICKMAX /* 32 (ident) + 3 (\0, !, @) + NICKMAX */
#define DIRMAX      512
#define LOGLINEMAX  767           /* For putlog(). */

/* Invalid characters. */
#define BADNICKCHARS "-,+*=:!.@#;$%&"
#define BADHANDCHARS "-,+*=:!.@#;$%&"


#define NICKLEN     NICKMAX + 1
#define UHOSTLEN    UHOSTMAX + 1
#define DIRLEN      DIRMAX + 1
#define LOGLINELEN  LOGLINEMAX + 1
#define NOTENAMELEN ((HANDLEN * 2) + 1)


#if (NICKMAX < 9) || (NICKMAX > 32)
#  include "Error: Invalid NICKMAX value."
#endif

#if (HANDLEN < 9) || (HANDLEN > 32)
#  include "Error: Invalid HANDLEN value."
#endif

#if HANDLEN > NICKMAX
#  include "Error: HANDLEN MUST BE <= NICKMAX."
#endif

#define nmalloc(x)    n_malloc((x),__FILE__,__LINE__)
#define nrealloc(x,y) n_realloc((x),(y),__FILE__,__LINE__)
#define nfree(x)      n_free((x),__FILE__,__LINE__)

#ifndef COMPILING_MEM
#  undef malloc
#  define malloc(x) dont_use_old_malloc(x)
#  undef free
#  define free(x)   dont_use_old_free(x)
#endif /* !COMPILING_MEM */

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


/* Number of global partyline channels. */
#define GLOBAL_CHANS 100000

#define STR_PROTECT     2
#define STR_DIR         1

#define HELP_DCC        1
#define HELP_TEXT       2
#define HELP_IRC        16

#endif /* _EGG_EGGDROP_H */
