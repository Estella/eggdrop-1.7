/* logfile.c
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
 * $Id: logfile.c,v 1.1 2004/08/30 23:58:23 wcc Exp $
 */

#include "main.h"

#include <sys/stat.h>
#include "stat.h"

#include "logfile.h"
#include "dcc.h"     /* DCC_* struct dcc_t */
#include "dccutil.h" /* dprintf */
#include "rfc1459.h" /* rfc_casecmp */


extern struct dcc_t *dcc;
extern char logfile_suffix[];
extern int backgrd, con_chan, term_z, use_stderr, keep_all_logs, quick_logs,
           dcc_total;
extern time_t now;

log_t *logs = 0;      /* Logfiles */
int shtime = 1;       /* Display the time with console output? */
int max_logs = 5;     /* Current maximum log files. */
int max_logsize = 0;  /* Maximum logfile size; 0 for no limit. */
int raw_log = 0;      /* Allow logging of raw traffic? */

void logfile_init()
{
  static int last = 0;

  if (max_logs < 1)
    max_logs = 1;

  if (logs)
    logs = nrealloc(logs, max_logs * sizeof(log_t));
  else
    logs = nmalloc(max_logs * sizeof(log_t));

  for (; last < max_logs; last++) {
    logs[last].filename  = logs[last].chname = NULL;
    logs[last].mask      = 0;
    logs[last].f         = NULL;
    logs[last].szlast[0] = 0;
    logs[last].repeats   = 0;
    logs[last].flags     = 0;
  }
}

int logfile_expmem()
{
  return (max_logs * sizeof(log_t));
}

void putlog EGG_VARARGS_DEF(int, arg1)
{
  int i, type, tsl = 0;
  char *format, *chname, s[LOGLINELEN], s1[256], *out, ct[81], *s2, stamp[34];
  va_list va;
  time_t now2 = time(NULL);
  struct tm *t = localtime(&now2);

  type = EGG_VARARGS_START(int, arg1, va);
  chname = va_arg(va, char *);
  format = va_arg(va, char *);

  /* Create the timestamp. */
  t = localtime(&now2);
  if (shtime) {
    egg_strftime(stamp, sizeof(stamp) - 2, LOG_TS, t);
    strcat(stamp, " ");
    tsl = strlen(stamp);
  }
  else
    *stamp = '\0';

  /* Format log entry at offset 'tsl,' then i can prepend the timestamp. */
  out = s + tsl;
  /* No need to check if out should be null-terminated here, just do it! <cybah> */
  egg_vsnprintf(out, LOGLINEMAX - tsl, format, va);
  out[LOGLINEMAX - tsl] = 0;
  if (keep_all_logs) {
    if (!logfile_suffix[0])
      egg_strftime(ct, 12, ".%d%b%Y", t);
    else {
      egg_strftime(ct, 80, logfile_suffix, t);
      ct[80] = 0;
      s2 = ct;
      /* replace spaces by underscores */
      while (s2[0]) {
        if (s2[0] == ' ')
          s2[0] = '_';
        s2++;
      }
    }
  }
  /* Place the timestamp in the string to be printed. */
  if (out[0] && shtime) {
    strncpy(s, stamp, tsl);
    out = s;
  }
  strcat(out, "\n");
  if (!use_stderr) {
    for (i = 0; i < max_logs; i++) {
      if ((logs[i].filename != NULL) && (logs[i].mask & type) &&
          ((chname[0] == '*') || (logs[i].chname[0] == '*') ||
           (!rfc_casecmp(chname, logs[i].chname)))) {
        if (logs[i].f == NULL) {
          /* Open this logfile. */
          if (keep_all_logs) {
            egg_snprintf(s1, 256, "%s%s", logs[i].filename, ct);
            logs[i].f = fopen(s1, "a+");
          } else
            logs[i].f = fopen(logs[i].filename, "a+");
        }
        if (logs[i].f != NULL) {
          /* Check if this is the same as the last line added to the log. */
          if (!egg_strcasecmp(out + tsl, logs[i].szlast))
            /* It is a repeat, so increment repeats. */
            logs[i].repeats++;
          else {
            /* Not a repeat; check if there were any repeat lines previously. */
            if (logs[i].repeats > 0) {
              /* Yep.. so display 'last message repeated x times'
               * then reset repeats. We want the current time here,
               * so put that in the file first.
               */
              fprintf(logs[i].f, stamp);
              fprintf(logs[i].f, MISC_LOGREPEAT, logs[i].repeats);
              logs[i].repeats = 0;
              /* No need to reset logs[i].szlast here because we update it later on. */
            }
            fputs(out, logs[i].f);
            strncpyz(logs[i].szlast, out + tsl, LOGLINEMAX);
          }
        }
      }
    }
  }
  for (i = 0; i < dcc_total; i++) {
    if ((dcc[i].type == &DCC_CHAT) && (dcc[i].u.chat->con_flags & type)) {
      if ((chname[0] == '*') || (dcc[i].u.chat->con_chan[0] == '*') ||
          !rfc_casecmp(chname, dcc[i].u.chat->con_chan)) {
        dprintf(i, "%s", out);
      }
    }
  }
  if (!backgrd && !con_chan && !term_z)
    dprintf(DP_STDOUT, "%s", out);
  else if ((type & LOG_MISC) && use_stderr) {
    if (shtime)
      out += tsl;
    dprintf(DP_STDERR, "%s", s);
  }
  va_end(va);
}

/* Called as soon as the logfile suffix changes. All logs are closed
 * and the new suffix is stored in `logfile_suffix'.
 */
void logsuffix_change(char *s)
{
  int i;
  char *s2 = logfile_suffix;

  /* If the suffix didn't really change, ignore. It's probably a rehash. */
  if (s && s2 && !strcmp(s, s2))
    return;

  debug0("Logfile suffix changed. Closing all open logs.");
  strcpy(logfile_suffix, s);
  while (s2[0]) {
    if (s2[0] == ' ')
      s2[0] = '_';
    s2++;
  }
  for (i = 0; i < max_logs; i++) {
    if (logs[i].f) {
      fflush(logs[i].f);
      fclose(logs[i].f);
      logs[i].f = NULL;
    }
  }
}

void check_logsize()
{
  struct stat ss;
  int i;
  char buf[1024];

  if (keep_all_logs || max_logsize <= 0)
    return;

  for (i = 0; i < max_logs; i++) {
    if (!logs[i].filename)
      continue;
    if (stat(logs[i].filename, &ss) != 0)
      break;
    if ((ss.st_size >> 10) > max_logsize) {
      if (logs[i].f) {
        /* write to the log before closing it huh.. */
        putlog(LOG_MISC, "*", MISC_CLOGS, logs[i].filename, ss.st_size);
        fflush(logs[i].f);
        fclose(logs[i].f);
        logs[i].f = NULL;
      }
      egg_snprintf(buf, sizeof buf, "%s.yesterday", logs[i].filename);
      buf[1023] = 0;
      unlink(buf);
      movefile(logs[i].filename, buf);
    }
  }
}

/* Flush the logfiles to disk */
void flushlogs()
{
  int i;

  /* Logs may not be initialised yet. */
  if (!logs)
    return;

  /* Now also checks to see if there's a repeat message and
   * displays the 'last message repeated...' stuff too <cybah>
   */
  for (i = 0; i < max_logs; i++) {
    if (logs[i].f == NULL)
      continue;
    if ((logs[i].repeats > 0) && quick_logs) {
      /* Repeat.. if quicklogs used then display 'last message
        * repeated x times' and reset repeats.
        */
      char stamp[33];

      egg_strftime(stamp, sizeof(stamp) - 1, LOG_TS, localtime(&now));
      fprintf(logs[i].f, "%s ", stamp);
      fprintf(logs[i].f, MISC_LOGREPEAT, logs[i].repeats);
      /* Reset repeats. */
      logs[i].repeats = 0;
    }
    fflush(logs[i].f);
  }
}


