/* logfile.h
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
 * $Id: logfile.h,v 1.1 2004/08/30 23:58:23 wcc Exp $
 */

#ifndef _EGG_LOGFILE_H
#define _EGG_LOGFILE_H

/* Logfile mask flags. */
#define LOG_MSGS      0x000001   /* m   msgs/notice/ctcps                */
#define LOG_PUBLIC    0x000002   /* p   public msg/notice/ctcps          */
#define LOG_JOIN      0x000004   /* j   channel joins/parts/etc          */
#define LOG_MODES     0x000008   /* k   mode changes/kicks/bans          */
#define LOG_CMDS      0x000010   /* c   user dcc or msg commands         */
#define LOG_MISC      0x000020   /* o   other misc bot things            */
#define LOG_BOTS      0x000040   /* b   bot notices                      */
#define LOG_RAW       0x000080   /* r   raw server stuff coming in       */
#define LOG_FILES     0x000100   /* x   file transfer commands and stats */
#define LOG_LEV1      0x000200   /* 1   user log level                   */
#define LOG_LEV2      0x000400   /* 2   user log level                   */
#define LOG_LEV3      0x000800   /* 3   user log level                   */
#define LOG_LEV4      0x001000   /* 4   user log level                   */
#define LOG_LEV5      0x002000   /* 5   user log level                   */
#define LOG_LEV6      0x004000   /* 6   user log level                   */
#define LOG_LEV7      0x008000   /* 7   user log level                   */
#define LOG_LEV8      0x010000   /* 8   user log level                   */
#define LOG_SERV      0x020000   /* s   server information               */
#define LOG_DEBUG     0x040000   /* d   debug                            */
#define LOG_WALL      0x080000   /* w   wallops                          */
#define LOG_SRVOUT    0x100000   /* v   server output                    */
#define LOG_BOTNET    0x200000   /* t   botnet traffic                   */
#define LOG_BOTSHARE  0x400000   /* h   share traffic                    */
#define LOG_ALL       0x7fffff   /* (dump to all logfiles)               */

/* Internal logfile flags. */
#define LF_EXPIRING  0x000001 /* Logfile will be closed soon. */

/* Max length of log lines. */
#define LOGLINELEN  LOGLINEMAX + 1

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
void logfile_init();
int logfile_expmem();
void putlog EGG_VARARGS(int, arg1);
void flushlogs();
void check_logsize();
void logsuffix_change(char *);
#endif

#endif /* !_EGG_LOGFILE_H */



