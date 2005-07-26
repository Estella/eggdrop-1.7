/* logfile.h
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
 * $Id: logfile.h,v 1.5 2005/07/26 03:31:29 wcc Exp $
 */

#ifndef _EGG_LOGFILE_H
#define _EGG_LOGFILE_H

/* Logfile mask flags. */
#define LOG_MSGS      0x000001   /* m   msgs/notice/ctcps                 */
#define LOG_PUBLIC    0x000002   /* p   public msg/notice/ctcps           */
#define LOG_JOIN      0x000004   /* j   joins/parts/etc                   */
#define LOG_MODES     0x000008   /* k   mode changes/kicks/bans           */
#define LOG_CMDS      0x000010   /* c   User partyline or msg commands.   */
#define LOG_MISC      0x000020   /* o   Misc stuff.                       */
#define LOG_BOTS      0x000040   /* b   Bot/botnet related.               */
#define LOG_FILES     0x000080   /* x   File transfer commands and stats. */
#define LOG_LEV1      0x000100   /* 1   User log level.                   */
#define LOG_LEV2      0x000200   /* 2   User log level.                   */
#define LOG_LEV3      0x000400   /* 3   User log level.                   */
#define LOG_LEV4      0x000800   /* 4   User log level.                   */
#define LOG_LEV5      0x001000   /* 5   User log level.                   */
#define LOG_LEV6      0x002000   /* 6   User log level.                   */
#define LOG_LEV7      0x004000   /* 7   User log level.                   */
#define LOG_LEV8      0x008000   /* 8   User log level.                   */
#define LOG_SERV      0x010000   /* s   Server information.               */
#define LOG_DEBUG     0x020000   /* d   Debug.                            */
#define LOG_WALL      0x040000   /* w   Wallops.                          */
#define LOG_RAW       0x080000   /* r   Raw server (incoming).            */
#define LOG_SRVOUT    0x100000   /* v   Raw server (outgoing).            */
#define LOG_BOTNET    0x200000   /* t   Botnet traffic.                   */
#define LOG_BOTSHARE  0x400000   /* h   Shareing traffic.                 */
#define LOG_TCLERROR  0x800000   /* e   Tcl errors.                       */
#define LOG_ALL       0xffffff   /* Write to all logs 'levels'.           */

/* Internal logfile flags. */
#define LF_EXPIRING  0x000001 /* Logfile will be closed soon. */

/* Max length of log lines. */
#define LOGLINELEN  LOGLINEMAX + 1

/* Log event time flags. */
#define LOGS_FIVEMINUTELY 0x000001 /* Once every five minutes. */
#define LOGS_MINUTELY     0x000002 /* Once every minute.       */

typedef struct {
  char *filename;
  unsigned int mask;        /* What to send to this log.                          */
  char *chname;             /* Channel name.                                      */
  char szlast[LOGLINELEN];  /* 'Last message repeated n times' stuff in putlog(). */
  int repeats;              /* Number of times szlast has been repeated.          */
  unsigned int flags;       /* Other flags.                                       */
  FILE *f;                  /* File descriptor.                                   */
} log_t;

#ifndef MAKING_MODS
void logfile_init(int);
int logfile_expmem();
void check_expiring_logs();
void expire_all_logs();
void flush_and_check_logs(int timeflag);
void close_logs_at_midnight();
void switch_logs();
int logmodes(char *);
char *masktype(int);
char *maskname(int);
void putlog EGG_VARARGS(int, arg1);
void flushlogs();
void check_logsize();
void logsuffix_change(char *);
#endif

#endif /* !_EGG_LOGFILE_H */



