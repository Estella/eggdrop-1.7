/* help.c
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
 * $Id: help.c,v 1.4 2005/01/21 01:43:40 wcc Exp $
 */

#include "main.h"
#include <fcntl.h>

#ifdef HAVE_UNAME
#  include <sys/utsname.h>
#endif


#include "help.h"
#include "botmsg.h"  /* simple_sprintf */
#include "dccutil.h" /* dprintf */
#include "match.h"   /* wild_match */
#include "mem.h"     /* nmalloc, nrealloc, nfree */
#include "misc.h"    /* my_strcpy, strncpyz */


extern char version[], botname[], admin[], network[], helpdir[], textdir[],
            ver[], botnetnick[];
extern time_t now;
extern struct chanset_t *chanset;
extern struct dcc_t *dcc;


static struct help_ref *help_list = NULL;
static int cols = 0;
static int colsofar = 0;
static int subwidth = 70;
static char *colstr = NULL;
static int blind = 0;


static void subst_addcol(char *, char *);
static int display_tellhelp(int, char *, FILE *, struct flag_record *);
static FILE *resolve_help(int, char *);
static void scan_help_file(struct help_ref *, char *, int);
static void tellwildhelp(int, char *, struct flag_record *);
static void tellallhelp(int, char *, struct flag_record *);


int help_expmem()
{
  struct help_ref *current;
  struct help_list_t *item;
  int tot = 0;

  for (current = help_list; current; current = current->next) {
    tot += sizeof(struct help_ref) + strlen(current->name) + 1;

    for (item = current->first; item; item = item->next)
      tot += sizeof(struct help_list_t) + strlen(item->name) + 1;
  }

  return tot;
}

void debug_help(int idx)
{
  struct help_ref *current;
  struct help_list_t *item;

  for (current = help_list; current; current = current->next) {
    dprintf(idx, "HELP FILE(S): %s\n", current->name);
    for (item = current->first; item; item = item->next) {
      dprintf(idx, "   %s (%s)\n", item->name, (item->type == 0) ? "msg/" :
              (item->type == 1) ? "" : "set/");
    }
  }
}

void add_help_reference(char *file)
{
  char s[1024];
  struct help_ref *current;

  for (current = help_list; current; current = current->next) {
    if (!strcmp(current->name, file))
      return; /* Already exists, can't re-add :P */
  }

  current = nmalloc(sizeof *current);
  current->name = nmalloc(strlen(file) + 1);
  strcpy(current->name, file);
  current->next = help_list;
  current->first = NULL;
  help_list = current;
  egg_snprintf(s, sizeof s, "%smsg/%s", helpdir, file);
  scan_help_file(current, s, 0);
  egg_snprintf(s, sizeof s, "%s%s", helpdir, file);
  scan_help_file(current, s, 1);
  egg_snprintf(s, sizeof s, "%sset/%s", helpdir, file);
  scan_help_file(current, s, 2);
}

void rem_help_reference(char *file)
{
  struct help_ref *current, *last = NULL;
  struct help_list_t *item;

  for (current = help_list; current; last = current, current = current->next) {
    if (!strcmp(current->name, file)) {
      while ((item = current->first)) {
        current->first = item->next;
        nfree(item->name);
        nfree(item);
      }
      nfree(current->name);
      if (last)
        last->next = current->next;
      else
        help_list = current->next;
      nfree(current);
      return;
    }
  }
}

void reload_help_data()
{
  struct help_ref *current = help_list, *next;
  struct help_list_t *item;

  help_list = NULL;
  while (current) {
    while ((item = current->first)) {
      current->first = item->next;
      nfree(item->name);
      nfree(item);
    }
    add_help_reference(current->name);
    nfree(current->name);
    next = current->next;
    nfree(current);
    current = next;
  }
}

void showhelp(char *who, char *file, struct flag_record *flags, int fl)
{
  int lines = 0;
  char s[HELP_BUF_LEN + 1];
  FILE *f = resolve_help(fl, file);

  if (f) {
    help_subst(NULL, NULL, 0, HELP_IRC, NULL);  /* Clear flags. */
    while (!feof(f)) {
      fgets(s, HELP_BUF_LEN, f);
      if (!feof(f)) {
        if (s[strlen(s) - 1] == '\n')
          s[strlen(s) - 1] = 0;
        if (!s[0])
          strcpy(s, " ");
        help_subst(s, who, flags, 0, file);
        if (s[0] && strlen(s) > 1) {
          dprintf(DP_HELP, "NOTICE %s :%s\n", who, s);
          lines++;
        }
      }
    }
    fclose(f);
  }

  if (!lines && !(fl & HELP_TEXT))
    dprintf(DP_HELP, "NOTICE %s :%s\n", who, IRC_NOHELP2);
}

void tellhelp(int idx, char *file, struct flag_record *flags, int fl)
{
  int lines = 0;
  FILE *f = resolve_help(HELP_DCC | fl, file);

  if (f)
    lines = display_tellhelp(idx, file, f, flags);

  if (!lines && !(fl & HELP_TEXT))
    dprintf(idx, "%s\n", IRC_NOHELP2);
}

void help_subst(char *s, char *nick, struct flag_record *flags,
                int isdcc, char *topic)
{
  char xx[HELP_BUF_LEN + 1], sub[161], *current, *q, chr, *writeidx,
       *readidx, *towrite;
  struct chanset_t *chan;
  int i, j, center = 0;
  static int help_flags;
#ifdef HAVE_UNAME
  struct utsname uname_info;
#endif

  if (s == NULL) {
    /* Used to reset substitutions. */
    blind = 0;
    cols = 0;
    subwidth = 70;
    if (colstr != NULL) {
      nfree(colstr);
      colstr = NULL;
    }
    help_flags = isdcc;
    return;
  }

  strncpyz(xx, s, sizeof xx);
  readidx = xx;
  writeidx = s;
  current = strchr(readidx, '%');
  while (current) {
    /* Are we about to copy a chuck to the end of the buffer? If so, return. */
    if ((writeidx + (current - readidx)) >= (s + HELP_BUF_LEN)) {
      strncpy(writeidx, readidx, (s + HELP_BUF_LEN) - writeidx);
      s[HELP_BUF_LEN] = 0;
      return;
    }

    chr = *(current + 1);
    *current = 0;
    if (!blind)
      writeidx += my_strcpy(writeidx, readidx);
    towrite = NULL;
    switch (chr) {
    case 'b':
      if (glob_hilite(*flags)) {
        if (help_flags & HELP_IRC) {
          towrite = "\002";
        } else if (help_flags & HELP_BOLD) {
          help_flags &= ~HELP_BOLD;
          towrite = "\033[0m";
        } else {
          help_flags |= HELP_BOLD;
          towrite = "\033[1m";
        }
      }
      break;
    case 'v':
      if (glob_hilite(*flags)) {
        if (help_flags & HELP_IRC) {
          towrite = "\026";
        } else if (help_flags & HELP_REV) {
          help_flags &= ~HELP_REV;
          towrite = "\033[0m";
        } else {
          help_flags |= HELP_REV;
          towrite = "\033[7m";
        }
      }
      break;
    case '_':
      if (glob_hilite(*flags)) {
        if (help_flags & HELP_IRC) {
          towrite = "\037";
        } else if (help_flags & HELP_UNDER) {
          help_flags &= ~HELP_UNDER;
          towrite = "\033[0m";
        } else {
          help_flags |= HELP_UNDER;
          towrite = "\033[4m";
        }
      }
      break;
    case 'f':
      if (glob_hilite(*flags)) {
        if (help_flags & HELP_FLASH) {
          if (help_flags & HELP_IRC)
            towrite = "\002\037";
          else
            towrite = "\033[0m";
          help_flags &= ~HELP_FLASH;
        } else {
          help_flags |= HELP_FLASH;
          if (help_flags & HELP_IRC)
            towrite = "\037\002";
          else
            towrite = "\033[5m";
        }
      }
      break;
    case 'U':
#ifdef HAVE_UNAME
      if (uname(&uname_info) >= 0) {
        egg_snprintf(sub, sizeof sub, "%s %s", uname_info.sysname,
                     uname_info.release);
        towrite = sub;
      } else
#endif
        towrite = "*UNKNOWN*";
      break;
    case 'B':
      towrite = (isdcc ? botnetnick : botname);
      break;
    case 'V':
      towrite = ver;
      break;
    case 'E':
      towrite = version;
      break;
    case 'A':
      towrite = admin;
      break;
    case 'n':
      towrite = network;
      break;
    case 'T':
      egg_strftime(sub, 6, "%H:%M", localtime(&now));
      towrite = sub;
      break;
    case 'N':
      towrite = strchr(nick, ':');
      if (towrite)
        towrite++;
      else
        towrite = nick;
      break;
    case 'C':
      if (!blind)
        for (chan = chanset; chan; chan = chan->next) {
          if ((strlen(chan->dname) + writeidx + 2) >= (s + HELP_BUF_LEN)) {
            strncpy(writeidx, chan->dname, (s + HELP_BUF_LEN) - writeidx);
            s[HELP_BUF_LEN] = 0;
            return;
          }
          writeidx += my_strcpy(writeidx, chan->dname);
          if (chan->next) {
            *writeidx++ = ',';
            *writeidx++ = ' ';
          }
        }
      break;
    case '{':
      q = current;
      current++;
      while (*current != '}' && *current)
        current++;
      if (*current) {
        *current = 0;
        current--;
        q += 2;
        /* Now q is the string and p is where the rest of the fcn expects */
        if (!strncmp(q, "help=", 5)) {
          if (topic && egg_strcasecmp(q + 5, topic))
            blind |= 2;
          else
            blind &= ~2;
        } else if (!(blind & 2)) {
          if (q[0] == '+') {
            struct flag_record fr = { FR_GLOBAL | FR_CHAN, 0, 0, 0, 0, 0 };

            break_down_flags(q + 1, &fr, NULL);
            if (!flagrec_ok(&fr, flags))
              blind |= 1;
            else
              blind &= ~1;
          } else if (q[0] == '-')
            blind &= ~1;
          else if (!egg_strcasecmp(q, "end")) {
            blind &= ~1;
            subwidth = 70;
            if (cols) {
              sub[0] = 0;
              subst_addcol(sub, "\377");
              nfree(colstr);
              colstr = NULL;
              cols = 0;
              towrite = sub;
            }
          } else if (!egg_strcasecmp(q, "center"))
            center = 1;
          else if (!strncmp(q, "cols=", 5)) {
            char *r;

            cols = atoi(q + 5);
            colsofar = 0;
            colstr = nmalloc(1);
            colstr[0] = 0;
            r = strchr(q + 5, '/');
            if (r != NULL)
              subwidth = atoi(r + 1);
          }
        }
      } else
        current = q;            /* no } so ignore */
      break;
    default:
      if (!blind) {
        *writeidx++ = chr;
        if (writeidx >= (s + HELP_BUF_LEN)) {
          *writeidx = 0;
          return;
        }
      }
    }
    if (towrite && !blind) {
      if ((writeidx + strlen(towrite)) >= (s + HELP_BUF_LEN)) {
        strncpy(writeidx, towrite, (s + HELP_BUF_LEN) - writeidx);
        s[HELP_BUF_LEN] = 0;
        return;
      }
      writeidx += my_strcpy(writeidx, towrite);
    }
    if (chr) {
      readidx = current + 2;
      current = strchr(readidx, '%');
    } else {
      readidx = current + 1;
      current = NULL;
    }
  }
  if (!blind) {
    i = strlen(readidx);
    if (i && ((writeidx + i) >= (s + HELP_BUF_LEN))) {
      strncpy(writeidx, readidx, (s + HELP_BUF_LEN) - writeidx);
      s[HELP_BUF_LEN] = 0;
      return;
    }
    strcpy(writeidx, readidx);
  } else
    *writeidx = 0;
  if (center) {
    strcpy(xx, s);
    i = 35 - (strlen(xx) / 2);
    if (i > 0) {
      s[0] = 0;
      for (j = 0; j < i; j++)
        s[j] = ' ';
      strcpy(s + i, xx);
    }
  }
  if (cols) {
    strcpy(xx, s);
    s[0] = 0;
    subst_addcol(s, xx);
  }
}

static void subst_addcol(char *s, char *newcol)
{
  char *p, *q;
  int i, colwidth;

  if ((newcol[0]) && (newcol[0] != '\377'))
    colsofar++;
  colstr = nrealloc(colstr, strlen(colstr) + strlen(newcol) +
                    (colstr[0] ? 2 : 1));
  if ((newcol[0]) && (newcol[0] != '\377')) {
    if (colstr[0])
      strcat(colstr, "\377");
    strcat(colstr, newcol);
  }
  if ((colsofar == cols) || ((newcol[0] == '\377') && (colstr[0]))) {
    colsofar = 0;
    strcpy(s, "     ");
    colwidth = (subwidth - 5) / cols;
    q = colstr;
    p = strchr(colstr, '\377');
    while (p != NULL) {
      *p = 0;
      strcat(s, q);
      for (i = strlen(q); i < colwidth; i++)
        strcat(s, " ");
      q = p + 1;
      p = strchr(q, '\377');
    }
    strcat(s, q);
    nfree(colstr);
    colstr = nmalloc(1);
    colstr[0] = 0;
  }
}

static void tellwildhelp(int idx, char *match, struct flag_record *flags)
{
  struct help_ref *current;
  struct help_list_t *item;
  FILE *f;
  char s[1024];

  s[0] = '\0';
  for (current = help_list; current; current = current->next) {
    for (item = current->first; item; item = item->next) {
      if (wild_match(match, item->name) && item->type) {
        if (item->type == 1)
          egg_snprintf(s, sizeof s, "%s%s", helpdir, current->name);
        else
          egg_snprintf(s, sizeof s, "%sset/%s", helpdir, current->name);
        f = fopen(s, "r");
        if (f)
          display_tellhelp(idx, item->name, f, flags);
      }
    }
  }

  if (!s[0])
    dprintf(idx, "%s\n", IRC_NOHELP2);
}

static void tellallhelp(int idx, char *match, struct flag_record *flags)
{
  struct help_ref *current;
  struct help_list_t *item;
  FILE *f;
  char s[1024];

  s[0] = '\0';
  for (current = help_list; current; current = current->next) {
    for (item = current->first; item; item = item->next) {
      if (!strcmp(match, item->name) && item->type) {
        if (item->type == 1)
          egg_snprintf(s, sizeof s, "%s%s", helpdir, current->name);
        else
          egg_snprintf(s, sizeof s, "%sset/%s", helpdir, current->name);
        f = fopen(s, "r");
        if (f)
          display_tellhelp(idx, item->name, f, flags);
      }
    }
  }

  if (!s[0])
    dprintf(idx, "%s\n", IRC_NOHELP2);
}

static int display_tellhelp(int idx, char *file, FILE *f,
                            struct flag_record *flags)
{
  char s[HELP_BUF_LEN + 1];
  int lines = 0;

  if (f) {
    help_subst(NULL, NULL, 0, (dcc[idx].status & STAT_TELNET) ? 0 :
               HELP_IRC, NULL);
    while (!feof(f)) {
      fgets(s, HELP_BUF_LEN, f);
      if (!feof(f)) {
        if (s[strlen(s) - 1] == '\n')
          s[strlen(s) - 1] = 0;
        if (!s[0])
          strcpy(s, " ");
        help_subst(s, dcc[idx].nick, flags, 1, file);
        if (s[0]) {
          dprintf(idx, "%s\n", s);
          lines++;
        }
      }
    }
    fclose(f);
  }

  return lines;
}

static FILE *resolve_help(int dcc, char *file)
{

  char s[1024];
  FILE *f;
  struct help_ref *current;
  struct help_list_t *item;

  /* Somewhere here goes the eventual substituation. */
  if (!(dcc & HELP_TEXT)) {
    for (current = help_list; current; current = current->next) {
      for (item = current->first; item; item = item->next) {
        if (!strcmp(item->name, file)) {
          if (!item->type && !dcc) {
            egg_snprintf(s, sizeof s, "%smsg/%s", helpdir, current->name);
            f = fopen(s, "r");
            if (f)
              return f;
          } else if (dcc && item->type) {
            if (item->type == 1)
              egg_snprintf(s, sizeof s, "%s%s", helpdir, current->name);
            else
              egg_snprintf(s, sizeof s, "%sset/%s", helpdir, current->name);
            f = fopen(s, "r");
            if (f)
              return f;
          }
        }
      }
    }

    /* No match was found, so we better return NULL. */
    return NULL;
  }

  /* Since we're not dealing with help files, we should just prepend the filename with textdir. */
  simple_sprintf(s, "%s%s", textdir, file);
  if (is_file(s))
    return fopen(s, "r");
  else
    return NULL;
}


static void scan_help_file(struct help_ref *current, char *filename, int type)
{
  FILE *f;
  char s[HELP_BUF_LEN + 1], *p, *q;
  struct help_list_t *list;

  if (!is_file(filename) || !(f = fopen(filename, "r")))
    return;

  while (!feof(f)) {
    fgets(s, HELP_BUF_LEN, f);
    if (feof(f))
      continue;
    p = s;
    while ((q = strstr(p, "%{help="))) {
      q += 7;
      p = strchr(q, '}');
      if (p) {
        *p = 0;
        list = nmalloc(sizeof *list);
        list->name = nmalloc(p - q + 1);
        strcpy(list->name, q);
        list->next = current->first;
        list->type = type;
        current->first = list;
        p++;
      } else {
        p = "";
      }
    }
  }
  fclose(f);
}

static void cmd_help(struct userrec *u, int idx, char *par)
{
  struct flag_record fr = { FR_GLOBAL | FR_CHAN, 0, 0, 0, 0, 0 };

  get_user_flagrec(u, &fr, dcc[idx].u.chat->con_chan);
  if (par[0]) {
    putlog(LOG_CMDS, "*", "#%s# help %s", dcc[idx].nick, par);
    if (!strcmp(par, "all"))
      tellallhelp(idx, "all", &fr);
    else if (strchr(par, '*') || strchr(par, '?')) {
      char *p = par;

      /* Check if the search pattern only consists of '*' and/or '?'
       * If it does, show help for "all" instead of listing all help
       * entries.
       */
      for (p = par; *p && (*p == '*' || *p == '?'); p++);
      if (*p)
        tellwildhelp(idx, par, &fr);
      else
        tellallhelp(idx, "all", &fr);
    } else
      tellhelp(idx, par, &fr, 0);
  } else {
    putlog(LOG_CMDS, "*", "#%s# help", dcc[idx].nick);
    if (glob_op(fr) || glob_botmast(fr) || chan_op(fr))
      tellhelp(idx, "help", &fr, 0);
    else
      tellhelp(idx, "partyline", &fr, 0);
  }
}

static void cmd_rehelp(struct userrec *u, int idx, char *par)
{
  putlog(LOG_CMDS, "*", "#%s# rehelp", dcc[idx].nick);
  dprintf(idx, "Reload help cache...\n");
  reload_help_data();
}

static int tcl_loadhelp STDVAR
{
  BADARGS(2, 2, " helpfile");

  add_help_reference(argv[1]);
  return TCL_OK;
}

static int tcl_unloadhelp STDVAR
{
  BADARGS(2, 2, " helpfile");

  rem_help_reference(argv[1]);
  return TCL_OK;
}

static int tcl_reloadhelp STDVAR
{
  BADARGS(1, 1, "");

  reload_help_data();
  return TCL_OK;
}

cmd_t help_dcc[] = {
  {"help",   "",  (Function) cmd_help,   NULL},
  {"rehelp", "n", (Function) cmd_rehelp, NULL},
  {NULL,      NULL,  NULL,               NULL}
};

tcl_cmds help_tcl[] = {
  {"loadhelp",   tcl_loadhelp},
  {"unloadhelp", tcl_unloadhelp},
  {"reloadhelp", tcl_reloadhelp},
  {NULL,         NULL}
};

void help_init()
{
    add_tcl_commands(help_tcl);
}
