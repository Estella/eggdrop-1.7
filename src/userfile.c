/* userfile.c
 *
 * Handles reading and writing of the userfile.
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
 * $Id: userfile.c,v 1.2 2004/11/26 05:35:27 wcc Exp $
 */


#include "main.h"
#include "modules.h"

#include <sys/stat.h> /* chmod */

#include "userfile.h"
#include "chanprog.h" /* findchan_by_dname */
#include "dcc.h"      /* DCC_*, struct dcc_t */
#include "mem.h"      /* nmalloc, nfree */
#include "misc.h"     /* splitc, newsplit, rmspace */
#include "rfc1459.h"  /* rfc_casecmp */
#include "userrec.h"  /* clear_masks */


extern struct dcc_t *dcc;
extern struct userrec *userlist, *lastuser;
extern char ver[], botnetnick[];
extern int quiet_save, sort_users, noshare, noxtra, dcc_total, share_greet;
extern time_t now;


char userfile[121] = "";
int sort_users = 0;       /* Sort the userlist when saving? */
int userfile_perm = 0600; /* Userfile permissions on disk (default rw-------). */


static void sort_userlist();
static int sort_compare(struct userrec *, struct userrec *);
static void restore_chanban(struct chanset_t *, char *);
static void restore_chanexempt(struct chanset_t *, char *);
static void restore_chaninvite(struct chanset_t *, char *);
static void restore_ignore(char *);
static void real_restore_chanmask(struct chanset_t *, maskrec **, maskrec **,
                                  char *, char *, char *, int, time_t, time_t,
                                  time_t);

/* Rewrite the entire user file. Call USERFILE hook as well, probably causing
 * the channel file to be rewritten as well.
 */
void writeuserfile(int idx)
{
  struct userrec *u;
  time_t tt;
  FILE *f;
  char *new_userfile, s[81];
  int ok;

  if (userlist == NULL)
    return; /* No point in saving the userfile. */

  new_userfile = nmalloc(strlen(userfile) + 5);
  sprintf(new_userfile, "%s~new", userfile);
  f = fopen(new_userfile, "w");
  if (f == NULL) {
    putlog(LOG_MISC, "*", USERF_ERRWRITE);
    nfree(new_userfile);
    return;
  }
  chmod(new_userfile, userfile_perm);

  if (!quiet_save)
    putlog(LOG_MISC, "*", USERF_WRITING);

  if (sort_users)
    sort_userlist();

  tt = now;
  strcpy(s, ctime(&tt));
  fprintf(f, "#4v: %s -- %s -- written %s", ver, botnetnick, s);
  ok = 1;
  for (u = userlist; u && ok; u = u->next) {
    if (!write_user(u, f, idx))
      ok = 0;
  }
  if (!ok || !write_ignores(f, -1) || fflush(f)) {
    putlog(LOG_MISC, "*", "%s (%s)", USERF_ERRWRITE, strerror(ferror(f)));
    fclose(f);
    nfree(new_userfile);
    return;
  }
  fclose(f);

  call_hook(HOOK_USERFILE);
  movefile(new_userfile, userfile);
  nfree(new_userfile);
}

/* Reload the user file from disk. */
void reloaduserfile()
{
  if (!file_readable(userfile)) {
    putlog(LOG_MISC, "*", MISC_CANTRELOADUSER);
    return;
  }

  noshare = 1;
  clear_userlist(userlist);
  noshare = 0;

  userlist = NULL;
  if (!readuserfile(userfile, &userlist))
    fatal(MISC_MISSINGUSERF, 0);

  reaffirm_owners();
  check_tcl_event("userfile-loaded");
  call_hook(HOOK_READ_USERFILE);
}

void backupuserfile(void)
{
  char s[125];

  putlog(LOG_MISC, "*", USERF_BACKUP);
  egg_snprintf(s, sizeof s, "%s~bak", userfile);
  copyfile(userfile, s);
}

/* Tagged lines in the user file:
 *
 * FIXME: Document me better.
 *
 *   #  (comment)
 *   ;  (comment)
 *   -  hostmask(s)
 *   +  email
 *   *  dcc directory
 *   =  comment
 *   :  info line
 *   .  xtra (Tcl)
 *   !  channel-specific
 *   !! global laston
 *   :: channel-specific bans
 *   *ban    global bans
 *   *ignore global ignores
 *   ::#chan channel bans
 *   - entries in each
 *   <handle> begin user entry
 *   --KEY INFO - info on each
 *   % exemptmask(s)
 *   @ Invitemask(s)
 *   *exempt global exempts
 *   *Invite global Invites
 *   && channel-specific exempts
 *   &&#chan channel exempts
 *   $$ channel-specific Invites
 *   $$#chan channel Invites
 */
int readuserfile(char *file, struct userrec **ret)
{
  struct userrec *bu, *u = NULL;
  struct chanset_t *cst = NULL;
  struct flag_record fr;
  struct chanuserrec *cr;
  FILE *f;
  char *p, buf[512], lasthand[512], *attr, *pass, *code, *s, s1[512], ignored[512];
  int i;

  bu = (*ret);
  ignored[0] = 0;
  if (bu == userlist) {
    clear_chanlist();
    lastuser    = NULL;
    global_ign  = NULL;
    global_bans = global_exempts = global_invites = NULL;
  }
  lasthand[0] = 0;

  f = fopen(file, "r");
  if (f == NULL)
    return 0;

  noshare = noxtra = 1;

  /* Read opening comment. */
  s = buf;
  fgets(s, 180, f);
  if (s[1] < '4')
    fatal(USERF_OLDFMT, 0);
  if (s[1] > '4')
    fatal(USERF_INVALID, 0);

  while (!feof(f)) {
    s = buf;
    fgets(s, 511, f);
    if (feof(f))
      continue;
    if (s[0] == '#' || s[0] == ';' || !s[0])
      continue;
    code = newsplit(&s);
    rmspace(s);
    if (!strcmp(code, "-")) {
      if (!lasthand[0])
        continue; /* Skip. */
      if (u) { /* Only break it down if they're a real user. */
        p = strchr(s, ',');
        while (p != NULL) {
          splitc(s1, s, ',');
          rmspace(s1);
          if (s1[0])
            set_user(&USERENTRY_HOSTS, u, s1);
          p = strchr(s, ',');
        }
      }
      /* Channel bans are never stacked with ','. */
      if (s[0]) {
        if (lasthand[0] && strchr(CHANMETA, lasthand[0]) != NULL)
          restore_chanban(cst, s);
        else if (lasthand[0] == '*') {
          if (lasthand[1] == 'i')
            restore_ignore(s);
          else
            restore_chanban(NULL, s);
        } else if (lasthand[0])
          set_user(&USERENTRY_HOSTS, u, s);
      }
    } else if (!strcmp(code, "%")) { /* exemptmasks */
      if (!lasthand[0])
        continue; /* Skip. */
      if (s[0]) {
        if (lasthand[0] == '#' || lasthand[0] == '+')
          restore_chanexempt(cst, s);
        else if (lasthand[0] == '*' && lasthand[1] == 'e')
          restore_chanexempt(NULL, s);
      }
    } else if (!strcmp(code, "@")) { /* Invitemasks */
      if (!lasthand[0])
        continue; /* Skip. */
      if (s[0]) {
        if (lasthand[0] == '#' || lasthand[0] == '+')
          restore_chaninvite(cst, s);
        else if (lasthand[0] == '*' && lasthand[1] == 'I')
          restore_chaninvite(NULL, s);
      }
    } else if (!strcmp(code, "!")) {
      /* ! #chan laston flags [info]. */
      char *chname, *st, *fl;

      if (u) {
        chname = newsplit(&s);
        st = newsplit(&s);
        fl = newsplit(&s);
        rmspace(s);
        fr.match = FR_CHAN;
        break_down_flags(fl, &fr, 0);
        if (findchan_by_dname(chname)) {
          for (cr = u->chanrec; cr; cr = cr->next) {
            if (!rfc_casecmp(cr->channel, chname))
              break;
          }
          if (!cr) {
            cr = (struct chanuserrec *)
              user_malloc(sizeof(struct chanuserrec));

            cr->next = u->chanrec;
            u->chanrec = cr;
            strncpyz(cr->channel, chname, 80);
            cr->laston = atoi(st);
            cr->flags = fr.chan;
            cr->flags_udef = fr.udef_chan;
            if (s[0]) {
              cr->info = (char *) user_malloc(strlen(s) + 1);
              strcpy(cr->info, s);
            } else
              cr->info = NULL;
          }
        }
      }
    } else if (!strncmp(code, "::", 2)) {
      /* Channel-specific bans. */
      strcpy(lasthand, &code[2]);
      u = NULL;
      if (!findchan_by_dname(lasthand)) {
        strcpy(s1, lasthand);
        strcat(s1, " ");
        if (strstr(ignored, s1) == NULL) {
          strcat(ignored, lasthand);
          strcat(ignored, " ");
        }
        lasthand[0] = 0;
      } else {
        /* Remove all bans for this channel to avoid dupes.
         * NOTE: only remove bans for when getting a userfile
         * from another bot & that channel is shared.
         */
        cst = findchan_by_dname(lasthand);
        if ((*ret == userlist) || channel_shared(cst)) {
          clear_masks(cst->bans);
          cst->bans = NULL;
        } else {
          /* Otherwise, ignore any bans for this channel. */
          cst = NULL;
          lasthand[0] = 0;
        }
      }
    } else if (!strncmp(code, "&&", 2)) {
      /* Channel-specific exempts. */
      strcpy(lasthand, &code[2]);
      u = NULL;
      if (!findchan_by_dname(lasthand)) {
        strcpy(s1, lasthand);
        strcat(s1, " ");
        if (strstr(ignored, s1) == NULL) {
          strcat(ignored, lasthand);
          strcat(ignored, " ");
        }
        lasthand[0] = 0;
      } else {
        /* Remove all exempts for this channel to avoid dupes.
         * NOTE: only remove exempts for when getting a userfile
         * from another bot & that channel is shared.
         */
        cst = findchan_by_dname(lasthand);
        if ((*ret == userlist) || channel_shared(cst)) {
          clear_masks(cst->exempts);
          cst->exempts = NULL;
        } else {
          /* Otherwise, ignore any exempts for this channel. */
          cst = NULL;
          lasthand[0] = 0;
        }
      }
    } else if (!strncmp(code, "$$", 2)) {
      /* Channel-specific invites. */
      strcpy(lasthand, &code[2]);
      u = NULL;
      if (!findchan_by_dname(lasthand)) {
        strcpy(s1, lasthand);
        strcat(s1, " ");
        if (strstr(ignored, s1) == NULL) {
          strcat(ignored, lasthand);
          strcat(ignored, " ");
        }
        lasthand[0] = 0;
      } else {
        /* Remove all invites for this channel to avoid dupes.
         * NOTE: only remove invites for when getting a userfile
         * from another bot & that channel is shared.
         */
        cst = findchan_by_dname(lasthand);
        if ((*ret == userlist) || channel_shared(cst)) {
          clear_masks(cst->invites);
          cst->invites = NULL;
        } else {
          /* Otherwise, ignore any invites for this channel. */
          cst = NULL;
          lasthand[0] = 0;
        }
      }
    } else if (!strncmp(code, "--", 2)) {
      if (u) {
        struct user_entry *ue;
        int ok = 0;

        for (ue = u->entries; ue && !ok; ue = ue->next) {
          if (ue->name && !egg_strcasecmp(code + 2, ue->name)) {
            struct list_type *list;

            list = user_malloc(sizeof(struct list_type));

            list->next = NULL;
            list->extra = user_malloc(strlen(s) + 1);
            strcpy(list->extra, s);
            list_append((&ue->u.list), list);
            ok = 1;
          }
        }
        if (!ok) {
          ue = user_malloc(sizeof(struct user_entry));
          ue->name = user_malloc(strlen(code + 1));
          ue->type = NULL;
          strcpy(ue->name, code + 2);
          ue->u.list = user_malloc(sizeof(struct list_type));
          ue->u.list->next = NULL;
          ue->u.list->extra = user_malloc(strlen(s) + 1);
          strcpy(ue->u.list->extra, s);
          list_insert((&u->entries), ue);
        }
      }
    } else if (!rfc_casecmp(code, BAN_NAME)) {
      strcpy(lasthand, code);
      u = NULL;
    } else if (!rfc_casecmp(code, IGNORE_NAME)) {
      strcpy(lasthand, code);
      u = NULL;
    } else if (!rfc_casecmp(code, EXEMPT_NAME)) {
      strcpy(lasthand, code);
      u = NULL;
    } else if (!rfc_casecmp(code, INVITE_NAME)) {
      strcpy(lasthand, code);
      u = NULL;
    } else if (code[0] == '*') {
      lasthand[0] = 0;
      u = NULL;
    } else {
      pass = newsplit(&s);
      attr = newsplit(&s);
      rmspace(s);
      if (!attr[0] || !pass[0]) {
        putlog(LOG_MISC, "*", "* %s '%s'!", USERF_CORRUPT, code);
        lasthand[0] = 0;
      } else {
        u = get_user_by_handle(bu, code);
        if (u && !(u->flags & USER_UNSHARED)) {
          putlog(LOG_MISC, "*", "* %s '%s'!", USERF_DUPE, code);
          lasthand[0] = 0;
          u = NULL;
        } else if (u) {
          lasthand[0] = 0;
          u = NULL;
        } else {
          fr.match = FR_GLOBAL;
          break_down_flags(attr, &fr, 0);
          strcpy(lasthand, code);
          cst = NULL;
          if (strlen(code) > HANDLEN)
            code[HANDLEN] = 0;
          if (strlen(pass) > 20) {
            putlog(LOG_MISC, "*", "* %s '%s'", USERF_BROKEPASS, code);
            strcpy(pass, "-");
          }
          bu = adduser(bu, code, 0, pass, sanity_check(fr.global &USER_VALID));
          u = get_user_by_handle(bu, code);
          for (i = 0; i < dcc_total; i++) {
            if (!egg_strcasecmp(code, dcc[i].nick))
              dcc[i].user = u;
          }
          u->flags_udef = fr.udef_global;
          /* If s starts with '/', it's got file info. */
        }
      }
    }
  }
  fclose(f);
  (*ret) = bu;

  if (ignored[0])
    putlog(LOG_MISC, "*", "%s %s", USERF_IGNBANS, ignored);

  putlog(LOG_MISC, "*", "Userfile loaded; unpacking...");
  for (u = bu; u; u = u->next) {
    struct user_entry *e;

    if (!(u->flags & USER_BOT) && !egg_strcasecmp(u->handle, botnetnick))
      putlog(LOG_MISC, "*", "(!) I have an user record, but without +b.");

    for (e = u->entries; e; e = e->next) {
      if (e->name) {
        struct user_entry_type *uet = find_entry_type(e->name);

        if (!uet)
          continue;
        e->type = uet;
        uet->unpack(u, e);
        nfree(e->name);
        e->name = NULL;
      }
    }
  }

  noshare = noxtra = 0;
  return 1;
}

/* Write a user record to the userfile. */
int write_user(struct userrec *u, FILE *f, int idx)
{
  struct chanuserrec *ch;
  struct chanset_t *cst;
  struct user_entry *ue;
  struct flag_record fr = { FR_GLOBAL, 0, 0, 0, 0, 0 };
  char s[181];

  fr.global = u->flags;
  fr.udef_global = u->flags_udef;
  build_flags(s, &fr, NULL);
  if (fprintf(f, "%-10s - %-24s\n", u->handle, s) == EOF)
    return 0;

  for (ch = u->chanrec; ch; ch = ch->next) {
    cst = findchan_by_dname(ch->channel);
    if (cst && (idx < 0 || channel_shared(cst))) {
      if (idx >= 0) {
        fr.match = (FR_CHAN | FR_BOT);
        get_user_flagrec(dcc[idx].user, &fr, ch->channel);
      } else
        fr.chan = BOT_SHARE;
      if ((fr.chan & BOT_SHARE) || (fr.bot & BOT_GLOBAL)) {
        fr.match = FR_CHAN;
        fr.chan = ch->flags;
        fr.udef_chan = ch->flags_udef;
        build_flags(s, &fr, NULL);
        if (fprintf(f, "! %-20s %lu %-10s %s\n", ch->channel, ch->laston, s,
            ((idx < 0 || share_greet) && ch->info) ? ch->info : "") == EOF)
          return 0;
      }
    }
  }
  for (ue = u->entries; ue; ue = ue->next) {
    if (ue->name) {
      struct list_type *lt;

      for (lt = ue->u.list; lt; lt = lt->next)
        if (fprintf(f, "--%s %s\n", ue->name, lt->extra) == EOF)
          return 0;
    }
    else if (!ue->type->write_userfile(f, u, ue))
      return 0;
  }
  return 1;
}

/* Write ignores to the userfile. */
int write_ignores(FILE *f, int idx)
{
  struct igrec *i;
  char *mask;

  if (global_ign && fprintf(f, IGNORE_NAME " - -\n") == EOF)
    return 0;

  for (i = global_ign; i; i = i->next) {
    mask = str_escape(i->igmask, ':', '\\');
    if (!mask || fprintf(f, "- %s:%s%lu:%s:%lu:%s\n", mask,
        (i->flags & IGREC_PERM) ? "+" : "", i->expire,
        i->user ? i->user : botnetnick, i->added,
        i->msg ? i->msg : "") == EOF) {
      if (mask)
        nfree(mask);
      return 0;
    }
    nfree(mask);
  }
  return 1;
}

/* Sorting function used if sort_users = 1. */
static void sort_userlist()
{
  int again = 1;
  struct userrec *last = NULL, *p, *c, *n;

  while (userlist != last && again) {
    p = NULL;
    c = userlist;
    n = c->next;
    again = 0;
    while (n != last) {
      if (sort_compare(c, n)) {
        again = 1;
        c->next = n->next;
        n->next = c;
        if (p == NULL)
          userlist = n;
        else
          p->next = n;
      }
      p = c;
      c = n;
      n = n->next;
    }
    last = c;
  }
}

/* Comparison function for sort_userlist().
 *
 * Orders by flags, then alphabetically.
 *   first bots: +h, +a, +l, other bots
 *   then users: +n, +m, +o, +h, other users
 *
 * return 1 if (a > b)
 */
static int sort_compare(struct userrec *a, struct userrec *b)
{
  if (a->flags & b->flags & USER_BOT) {
    if (~bot_flags(a) & bot_flags(b) & BOT_HUB)
      return 1;
    if (bot_flags(a) & ~bot_flags(b) & BOT_HUB)
      return 0;
    if (~bot_flags(a) & bot_flags(b) & BOT_ALT)
      return 1;
    if (bot_flags(a) & ~bot_flags(b) & BOT_ALT)
      return 0;
    if (~bot_flags(a) & bot_flags(b) & BOT_LEAF)
      return 1;
    if (bot_flags(a) & ~bot_flags(b) & BOT_LEAF)
      return 0;
  } else {
    if (~a->flags & b->flags & USER_BOT)
      return 1;
    if (a->flags & ~b->flags & USER_BOT)
      return 0;
    if (~a->flags & b->flags & USER_OWNER)
      return 1;
    if (a->flags & ~b->flags & USER_OWNER)
      return 0;
    if (~a->flags & b->flags & USER_MASTER)
      return 1;
    if (a->flags & ~b->flags & USER_MASTER)
      return 0;
    if (~a->flags & b->flags & USER_OP)
      return 1;
    if (a->flags & ~b->flags & USER_OP)
      return 0;
    if (~a->flags & b->flags & USER_HALFOP)
      return 1;
    if (a->flags & ~b->flags & USER_HALFOP)
      return 0;
  }
  return (egg_strcasecmp(a->handle, b->handle) > 0);
}

/* Helper function for readuserfile(). */
static void restore_chanban(struct chanset_t *chan, char *host)
{
  char *expi, *add, *last, *user, *desc;
  int flags = 0;

  expi = strchr_unescape(host, ':', '\\');
  if (expi) {
    if (*expi == '+') {
      flags |= MASKREC_PERM;
      expi++;
    }
    add = strchr(expi, ':');
    if (add) {
      if (add[-1] == '*') {
        flags |= MASKREC_STICKY;
        add[-1] = 0;
      } else
        *add = 0;
      add++;
      if (*add == '+') {
        last = strchr(add, ':');
        if (last) {
          *last = 0;
          last++;
          user = strchr(last, ':');
          if (user) {
            *user = 0;
            user++;
            desc = strchr(user, ':');
            if (desc) {
              *desc = 0;
              desc++;
              real_restore_chanmask(chan, &chan->bans, &global_bans, host,
                                    user, desc, flags, atoi(expi), atoi(add),
                                    atoi(last));
              return;
            }
          }
        }
      } else {
        desc = strchr(add, ':');
        if (desc) {
          *desc = 0;
          desc++;
          real_restore_chanmask(chan, &chan->bans, &global_bans, host, add,
                                desc, flags, atoi(expi), now, 0);
          return;
        }
      }
    }
  }
  putlog(LOG_MISC, "*", "*** Malformed banline for %s.",
         chan ? chan->dname : "global_bans");
}

/* Helper function for readuserfile(). */
static void restore_chanexempt(struct chanset_t *chan, char *host)
{
  char *expi, *add, *last, *user, *desc;
  int flags = 0;

  expi = strchr_unescape(host, ':', '\\');
  if (expi) {
    if (*expi == '+') {
      flags |= MASKREC_PERM;
      expi++;
    }
    add = strchr(expi, ':');
    if (add) {
      if (add[-1] == '*') {
        flags |= MASKREC_STICKY;
        add[-1] = 0;
      } else
        *add = 0;
      add++;
      if (*add == '+') {
        last = strchr(add, ':');
        if (last) {
          *last = 0;
          last++;
          user = strchr(last, ':');
          if (user) {
            *user = 0;
            user++;
            desc = strchr(user, ':');
            if (desc) {
              *desc = 0;
              desc++;
              real_restore_chanmask(chan, &chan->exempts, &global_exempts,
                                    host, user, desc, flags, atoi(expi),
                                    atoi(add), atoi(last));
              return;
            }
          }
        }
      } else {
        desc = strchr(add, ':');
        if (desc) {
          *desc = 0;
          desc++;
          real_restore_chanmask(chan, &chan->exempts, &global_exempts, host,
                                add, desc, flags, atoi(expi), now, 0);
          return;
        }
      }
    }
  }
  putlog(LOG_MISC, "*", "*** Malformed exemptline for %s.",
         chan ? chan->dname : "global_exempts");
}

/* Helper function for readuserfile(). */
static void restore_chaninvite(struct chanset_t *chan, char *host)
{
  char *expi, *add, *last, *user, *desc;
  int flags = 0;

  expi = strchr_unescape(host, ':', '\\');
  if (expi) {
    if (*expi == '+') {
      flags |= MASKREC_PERM;
      expi++;
    }
    add = strchr(expi, ':');
    if (add) {
      if (add[-1] == '*') {
        flags |= MASKREC_STICKY;
        add[-1] = 0;
      } else
        *add = 0;
      add++;
      if (*add == '+') {
        last = strchr(add, ':');
        if (last) {
          *last = 0;
          last++;
          user = strchr(last, ':');
          if (user) {
            *user = 0;
            user++;
            desc = strchr(user, ':');
            if (desc) {
              *desc = 0;
              desc++;
              real_restore_chanmask(chan, &chan->invites, &global_invites,
                                    host, user, desc, flags, atoi(expi),
                                    atoi(add), atoi(last));
              return;
            }
          }
        }
      } else {
        desc = strchr(add, ':');
        if (desc) {
          *desc = 0;
          desc++;
          real_restore_chanmask(chan, &chan->invites, &global_invites, host,
                                add, desc, flags, atoi(expi), now, 0);
          return;
        }
      }
    }
  }
  putlog(LOG_MISC, "*", "*** Malformed inviteline for %s.",
         chan ? chan->dname : "global_invites");
}

/* Helper function for restore_chan*() functions. */
static void real_restore_chanmask(struct chanset_t *chan, maskrec **m,
                                  maskrec **global, char *mask, char *from,
                                  char *note, int flags, time_t expire_time,
                                  time_t added, time_t last)
{
  maskrec *p = user_malloc(sizeof(maskrec));
  maskrec **u = (chan) ? m : global;

  p->next = *u;
  *u = p;
  p->expire = expire_time;
  p->added = added;
  p->lastactive = last;
  p->flags = flags;
  p->mask = user_malloc(strlen(mask) + 1);
  strcpy(p->mask, mask);
  p->user = user_malloc(strlen(from) + 1);
  strcpy(p->user, from);
  p->desc = user_malloc(strlen(note) + 1);
  strcpy(p->desc, note);
}

/* Helper function for readuserfile(). */
static void restore_ignore(char *host)
{
  struct igrec *p;
  char *expi, *user, *added, *desc;
  int flags = 0;

  expi = strchr_unescape(host, ':', '\\');
  if (expi) {
    if (*expi == '+') {
      flags |= IGREC_PERM;
      expi++;
    }
    user = strchr(expi, ':');
    if (user) {
      *user = 0;
      user++;
      added = strchr(user, ':');
      if (added) {
        *added = 0;
        added++;
        desc = strchr(added, ':');
        if (desc) {
          *desc = 0;
          desc++;
        } else
          desc = NULL;
      } else {
        added = "0";
        desc = NULL;
      }

      p = user_malloc(sizeof(struct igrec));
      p->next = global_ign;
      global_ign = p;
      p->expire = atoi(expi);
      p->added = atoi(added);
      p->flags = flags;
      p->igmask = user_malloc(strlen(host) + 1);
      strcpy(p->igmask, host);
      p->user = user_malloc(strlen(user) + 1);
      strcpy(p->user, user);
      if (desc) {
        p->msg = user_malloc(strlen(desc) + 1);
        strcpy(p->msg, desc);
      } else
        p->msg = NULL;
      return;
    }
  }
  putlog(LOG_MISC, "*", "*** Malformed ignore line.");
}

