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
 * $Id: logfile.c,v 1.6 2004/11/24 22:37:32 wcc Exp $
 */

#include "main.h"

#include <sys/stat.h>
#include "stat.h"

#include "logfile.h"
#include "dcc.h"     /* DCC_* struct dcc_t */
#include "dccutil.h" /* dprintf */
#include "misc.h"    /* my_strcpy, strncpyz */
#include "rfc1459.h" /* rfc_casecmp */


extern struct dcc_t *dcc;
extern char logfile_suffix[];
extern int backgrd, con_chan, term_z, use_stderr, keep_all_logs, quick_logs,
           dcc_total;
extern time_t now;
extern Tcl_Interp *interp;

log_t *logs = 0;      /* Logfiles. */
int max_logs = 5;     /* Current maximum log files. */
int max_logsize = 0;  /* Maximum logfile size; 0 for no limit. */
int raw_log = 0;      /* Allow logging of raw traffic? */

int conmask = LOG_TCLERROR | LOG_MODES | LOG_CMDS | LOG_MISC; /* Console mask. */


int logfile_expmem()
{
  int tot = 0, i;

  for (i = 0; i < max_logs; i++) {
    if (logs[i].filename != NULL) {
      tot += strlen(logs[i].filename) + 1;
      tot += strlen(logs[i].chname) + 1;
    }
  }
  tot += (max_logs * sizeof(log_t));

  return tot;
}

int logmodes(char *s)
{
  int i, res = 0;

  for (i = 0; i < strlen(s); i++) {
    switch (s[i]) {
      case 'm':
      case 'M':
        res |= LOG_MSGS;
        break;
      case 'p':
      case 'P':
        res |= LOG_PUBLIC;
        break;
      case 'j':
      case 'J':
        res |= LOG_JOIN;
        break;
      case 'k':
      case 'K':
        res |= LOG_MODES;
        break;
      case 'c':
      case 'C':
        res |= LOG_CMDS;
        break;
      case 'o':
      case 'O':
        res |= LOG_MISC;
        break;
      case 'b':
      case 'B':
        res |= LOG_BOTS;
        break;
      case 'w':
      case 'W':
        res |= LOG_WALL;
        break;
      case 'x':
      case 'X':
        res |= LOG_FILES;
        break;
      case 's':
      case 'S':
        res |= LOG_SERV;
        break;
      case 'd':
      case 'D':
        res |= LOG_DEBUG;
        break;
      case 'r':
      case 'R':
        res |= raw_log ? LOG_RAW : 0;
        break;
      case 'v':
      case 'V':
        res |= raw_log ? LOG_SRVOUT : 0;
        break;
      case 't':
      case 'T':
        res |= raw_log ? LOG_BOTNET : 0;
        break;
      case 'h':
      case 'H':
        res |= raw_log ? LOG_BOTSHARE : 0;
        break;
      case 'e':
      case 'E':
        res |= LOG_TCLERROR;
        break;
      case '1':
        res |= LOG_LEV1;
        break;
      case '2':
        res |= LOG_LEV2;
        break;
      case '3':
        res |= LOG_LEV3;
        break;
      case '4':
        res |= LOG_LEV4;
        break;
      case '5':
        res |= LOG_LEV5;
        break;
      case '6':
        res |= LOG_LEV6;
        break;
      case '7':
        res |= LOG_LEV7;
        break;
      case '8':
        res |= LOG_LEV8;
        break;
      case '*':
        res |= LOG_ALL;
        break;
    }
  }

  return res;
}

char *masktype(int x)
{
  static char s[26]; /* Change this if you change the levels. */
  char *p = s;

  if (x & LOG_MSGS)
    *p++ = 'm';
  if (x & LOG_PUBLIC)
    *p++ = 'p';
  if (x & LOG_JOIN)
    *p++ = 'j';
  if (x & LOG_MODES)
    *p++ = 'k';
  if (x & LOG_CMDS)
    *p++ = 'c';
  if (x & LOG_MISC)
    *p++ = 'o';
  if (x & LOG_BOTS)
    *p++ = 'b';
  if (x & LOG_FILES)
    *p++ = 'x';
  if (x & LOG_SERV)
    *p++ = 's';
  if (x & LOG_DEBUG)
    *p++ = 'd';
  if (x & LOG_WALL)
    *p++ = 'w';
  if ((x & LOG_RAW) && raw_log)
    *p++ = 'r';
  if ((x & LOG_SRVOUT) && raw_log)
    *p++ = 'v';
  if ((x & LOG_BOTNET) && raw_log)
    *p++ = 't';
  if ((x & LOG_BOTSHARE) && raw_log)
    *p++ = 'h';
  if ((x & LOG_TCLERROR))
    *p++ = 'e';
  if (x & LOG_LEV1)
    *p++ = '1';
  if (x & LOG_LEV2)
    *p++ = '2';
  if (x & LOG_LEV3)
    *p++ = '3';
  if (x & LOG_LEV4)
    *p++ = '4';
  if (x & LOG_LEV5)
    *p++ = '5';
  if (x & LOG_LEV6)
    *p++ = '6';
  if (x & LOG_LEV7)
    *p++ = '7';
  if (x & LOG_LEV8)
    *p++ = '8';
  if (p == s)
    *p++ = '-';
  *p = 0;

  return s;
}

char *maskname(int x)
{
  static char s[256]; /* Change this if you change the levels. */
  int i = 0;

  s[0] = 0;
  if (x & LOG_MSGS)
    i += my_strcpy(s, "msgs, ");
  if (x & LOG_PUBLIC)
    i += my_strcpy(s + i, "public, ");
  if (x & LOG_JOIN)
    i += my_strcpy(s + i, "joins, ");
  if (x & LOG_MODES)
    i += my_strcpy(s + i, "kicks/modes, ");
  if (x & LOG_CMDS)
    i += my_strcpy(s + i, "cmds, ");
  if (x & LOG_MISC)
    i += my_strcpy(s + i, "misc, ");
  if (x & LOG_BOTS)
    i += my_strcpy(s + i, "bots, ");
  if (x & LOG_FILES)
    i += my_strcpy(s + i, "files, ");
  if (x & LOG_SERV)
    i += my_strcpy(s + i, "server, ");
  if (x & LOG_DEBUG)
    i += my_strcpy(s + i, "debug, ");
  if (x & LOG_WALL)
    i += my_strcpy(s + i, "wallops, ");
  if ((x & LOG_RAW) && raw_log)
    i += my_strcpy(s + i, "server raw (incoming), ");
  if ((x & LOG_SRVOUT) && raw_log)
    i += my_strcpy(s + i, "server raw (outgoing), ");
  if ((x & LOG_BOTNET) && raw_log)
    i += my_strcpy(s + i, "botnet traffic, ");
  if ((x & LOG_BOTSHARE) && raw_log)
    i += my_strcpy(s + i, "share traffic, ");
  if ((x & LOG_TCLERROR))
    i += my_strcpy(s + i, "Tcl errors, ");
  if (x & LOG_LEV1)
    i += my_strcpy(s + i, "level 1, ");
  if (x & LOG_LEV2)
    i += my_strcpy(s + i, "level 2, ");
  if (x & LOG_LEV3)
    i += my_strcpy(s + i, "level 3, ");
  if (x & LOG_LEV4)
    i += my_strcpy(s + i, "level 4, ");
  if (x & LOG_LEV5)
    i += my_strcpy(s + i, "level 5, ");
  if (x & LOG_LEV6)
    i += my_strcpy(s + i, "level 6, ");
  if (x & LOG_LEV7)
    i += my_strcpy(s + i, "level 7, ");
  if (x & LOG_LEV8)
    i += my_strcpy(s + i, "level 8, ");

  if (i)
    s[i - 2] = 0;
  else
    strcpy(s, "none");

  return s;
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
  egg_strftime(stamp, sizeof(stamp) - 2, LOG_TS, t);
  strcat(stamp, " ");
  tsl = strlen(stamp);

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
  if (out[0]) {
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

static void cmd_addlog(struct userrec *u, int idx, char *par)
{
  if (!par[0]) {
    dprintf(idx, "Usage: addlog <message>\n");
    return;
  }

  dprintf(idx, "Placed entry in the log file.\n");
  putlog(LOG_MISC, "*", "%s: %s", dcc[idx].nick, par);
}

static int tcl_logfile STDVAR
{
  int i;
  char s[151];

  BADARGS(1, 4, " ?logmodes channel logfile?");

  if (argc == 1) {
    /* They just want a list of the logfiles and modes. */
    for (i = 0; i < max_logs; i++) {
      if (logs[i].filename != NULL) {
        strcpy(s, masktype(logs[i].mask));
        strcat(s, " ");
        strcat(s, logs[i].chname);
        strcat(s, " ");
        strcat(s, logs[i].filename);
        Tcl_AppendElement(interp, s);
      }

      return TCL_OK;
    }
  }

  BADARGS(4, 4, " ?logmodes channel logfile?");

  for (i = 0; i < max_logs; i++) {
    if (logs[i].filename != NULL && !strcmp(logs[i].filename, argv[3])) {
      logs[i].flags &= ~LF_EXPIRING;
      logs[i].mask = logmodes(argv[1]);
      nfree(logs[i].chname);
      logs[i].chname = NULL;
      if (!logs[i].mask) {
        /* Ending logfile. */
        nfree(logs[i].filename);
        logs[i].filename = NULL;
        if (logs[i].f != NULL) {
          fclose(logs[i].f);
          logs[i].f = NULL;
        }
        logs[i].flags = 0;
      } else {
        logs[i].chname = nmalloc(strlen(argv[2]) + 1);
        strcpy(logs[i].chname, argv[2]);
      }
      Tcl_AppendResult(interp, argv[3], NULL);

      return TCL_OK;
    }
  }

  /* Do not add logfiles without any flags to log. */
  if (!logmodes(argv[1])) {
    Tcl_AppendResult(interp, "can't remove \"", argv[3],
                     "\" from list: no such logfile", NULL);
    return TCL_ERROR;
  }

  for (i = 0; i < max_logs; i++) {
    if (logs[i].filename == NULL) {
      logs[i].flags = 0;
      logs[i].mask = logmodes(argv[1]);
      logs[i].filename = nmalloc(strlen(argv[3]) + 1);
      strcpy(logs[i].filename, argv[3]);
      logs[i].chname = nmalloc(strlen(argv[2]) + 1);
      strcpy(logs[i].chname, argv[2]);
      Tcl_AppendResult(interp, argv[3], NULL);

      return TCL_OK;
    }
  }

  Tcl_AppendResult(interp, "reached max number of logfiles", NULL);
  return TCL_ERROR;
}

static int tcl_putlog STDVAR
{
  char logtext[501];

  BADARGS(2, 2, " text");

  strncpyz(logtext, argv[1], sizeof logtext);
  putlog(LOG_MISC, "*", "%s", logtext);

  return TCL_OK;
}

static int tcl_putcmdlog STDVAR
{
  char logtext[501];

  BADARGS(2, 2, " text");

  strncpyz(logtext, argv[1], sizeof logtext);
  putlog(LOG_CMDS, "*", "%s", logtext);

  return TCL_OK;
}

static int tcl_putxferlog STDVAR
{
  char logtext[501];

  BADARGS(2, 2, " text");

  strncpyz(logtext, argv[1], sizeof logtext);
  putlog(LOG_FILES, "*", "%s", logtext);

  return TCL_OK;
}

static int tcl_putloglev STDVAR
{
  int lev = 0;
  char logtext[501];

  BADARGS(4, 4, " level channel text");

  lev = logmodes(argv[1]);
  if (!lev) {
    Tcl_AppendResult(irp, "no valid log-level given", NULL);
    return TCL_ERROR;
  }

  strncpyz(logtext, argv[3], sizeof logtext);
  putlog(lev, argv[2], "%s", logtext);

  return TCL_OK;
}

cmd_t logfile_dcc[] = {
  {"addlog", "to|o", (Function) cmd_addlog, NULL},
  {NULL,     NULL,   NULL,                  NULL}
};

tcl_cmds logfile_tcl[] = {
  {"logfile",    tcl_logfile},
  {"putlog",     tcl_putlog},
  {"putcmdlog",  tcl_putcmdlog},
  {"putxferlog", tcl_putxferlog},
  {"putloglev",  tcl_putloglev},
  {NULL,         NULL}
};

void logfile_init(int dotcl)
{
  if (dotcl)
    add_tcl_commands(logfile_tcl);
  else {
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
}
