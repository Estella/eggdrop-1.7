/* users.c
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
 * $Id: users.c,v 1.11 2004/10/27 23:54:54 wcc Exp $
 */

#include "main.h"
#include "users.h"
#include "chan.h"
#include "modules.h"

#include <netinet/in.h>
#include <arpa/inet.h>


#include "users.h"
#include "botmsg.h"   /* botnet_send_* */
#include "botnet.h"   /* nextbot, in_chain, botlink, rembot */
#include "dcc.h"      /* DCC_*, struct dcc_t */
#include "dccutil.h"  /* dprintf, chatout, shareout, lostdcc */
#include "chanprog.h" /* findchan_by_dname */
#include "logfile.h"  /* putlog, LOG_* */
#include "match.h"    /* wild_match */
#include "misc.h"     /* DAY_*, splitc, newsplit, strncpyz, days, str_escape,
                       * strchr_unescape, rmspace */
#include "net.h"      /* killsock */
#include "rfc1459.h"  /* rfc_casecmp */
#include "userrec.h"  /* adduser, clear_masks, clear_chanlist */


extern struct dcc_t *dcc;
extern struct userrec *userlist, *lastuser;
extern struct chanset_t *chanset;
extern int dcc_total, noshare;
extern char botnetnick[];
extern Tcl_Interp *interp;
extern time_t now;



int ignore_time = 10;           /* how many minutes will ignores last? */
int noxtra = 0;


/* Is this nick!user@host being ignored? */
int match_ignore(char *uhost)
{
  struct igrec *ir;

  for (ir = global_ign; ir; ir = ir->next) {
    if (wild_match(ir->igmask, uhost))
      return 1;
  }
  return 0;
}

/* Remove a mask from the ignore list. */
int delignore(char *ign)
{
  int i, j;
  struct igrec **u, *t;
  char temp[256];

  i = 0;
  if (!strchr(ign, '!') && (j = atoi(ign))) {
    for (u = &global_ign, j--; *u && j; u = &((*u)->next), j--);
    if (*u) {
      strncpyz(temp, (*u)->igmask, sizeof temp);
      i = 1;
    }
  } else {
    /* Find the matching host, if there is one. */
    for (u = &global_ign; *u && !i; u = &((*u)->next)) {
      if (!rfc_casecmp(ign, (*u)->igmask)) {
        strncpyz(temp, ign, sizeof temp);
        i = 1;
        break;
      }
    }
  }

  if (i) {
    if (!noshare) {
      char *mask = str_escape(temp, ':', '\\');

      if (mask) {
        shareout(NULL, "-i %s\n", mask);
        nfree(mask);
      }
    }
    nfree((*u)->igmask);
    if ((*u)->msg)
      nfree((*u)->msg);
    if ((*u)->user)
      nfree((*u)->user);
    t = *u;
    *u = (*u)->next;
    nfree(t);
  }
  return i;
}

/* Add a mask to the ignore list. */
void addignore(char *ign, char *from, char *mnote, time_t expire_time)
{
  struct igrec *p = NULL, *l;

  for (l = global_ign; l; l = l->next) {
    if (!rfc_casecmp(l->igmask, ign)) {
      p = l;
      break;
    }
  }

  if (p == NULL) {
    p = user_malloc(sizeof(struct igrec));
    p->next = global_ign;
    global_ign = p;
  } else {
    nfree(p->igmask);
    nfree(p->user);
    nfree(p->msg);
  }

  p->expire = expire_time;
  p->added = now;
  p->flags = expire_time ? 0 : IGREC_PERM;
  p->igmask = user_malloc(strlen(ign) + 1);
  strcpy(p->igmask, ign);
  p->user = user_malloc(strlen(from) + 1);
  strcpy(p->user, from);
  p->msg = user_malloc(strlen(mnote) + 1);
  strcpy(p->msg, mnote);

  if (!noshare) {
    char *mask = str_escape(ign, ':', '\\');

    if (mask) {
      shareout(NULL, "+i %s %li %c %s %s\n", mask, expire_time - now,
               (p->flags & IGREC_PERM) ? 'p' : '-', from, mnote);
      nfree(mask);
    }
  }
}

/* Take host entry from ignore list and display it ignore-style. */
void display_ignore(int idx, int number, struct igrec *ignore)
{
  char dates[81], s[41];

  if (ignore->added) {
    days(now, ignore->added, s, DAYS_AGO);
    sprintf(dates, "Started %s", s);
  } else {
    dates[0] = 0;
  }

  if (ignore->flags & IGREC_PERM)
    strcpy(s, "(perm)");
  else {
    char s1[41];

    days(ignore->expire, now, s1, DAYS_IN);
    sprintf(s, "(expires %s)", s1);
  }

  if (number >= 0)
    dprintf(idx, "  [%3d] %s %s\n", number, ignore->igmask, s);
  else
    dprintf(idx, "IGNORE: %s %s\n", ignore->igmask, s);

  if (ignore->msg && ignore->msg[0])
    dprintf(idx, "        %s: %s\n", ignore->user, ignore->msg);
  else
    dprintf(idx, "        %s %s\n", MODES_PLACEDBY, ignore->user);

  if (dates[0])
    dprintf(idx, "        %s\n", dates);
}

/* list the ignores and how long they've been active */
void tell_ignores(int idx, char *match)
{
  struct igrec *u = global_ign;
  int k = 1;

  if (u == NULL) {
    dprintf(idx, "No ignores.\n");
    return;
  }
  dprintf(idx, "%s:\n", IGN_CURRENT);
  for (; u; u = u->next) {
    if (match[0]) {
      if (wild_match(match, u->igmask) ||
          wild_match(match, u->msg) || wild_match(match, u->user))
        display_ignore(idx, k, u);
      k++;
    } else
      display_ignore(idx, k++, u);
  }
}

/* Check for expired timed-ignores. */
void check_expired_ignores()
{
  struct igrec **u = &global_ign;

  if (!*u)
    return;

  while (*u) {
    if (!((*u)->flags & IGREC_PERM) && (now >= (*u)->expire)) {
      putlog(LOG_MISC, "*", "%s %s (%s)", IGN_NOLONGER, (*u)->igmask,
             MISC_EXPIRED);
      delignore((*u)->igmask);
    } else
      u = &((*u)->next);
  }
}

void tell_user(int idx, struct userrec *u, int master)
{
  char s[81], s1[81], format[81];
  int n = 0;
  time_t now2;
  struct chanuserrec *ch;
  struct user_entry *ue;
  struct laston_info *li;
  struct flag_record fr = { FR_GLOBAL, 0, 0, 0, 0, 0 };

  fr.global = u->flags;

  fr.udef_global = u->flags_udef;
  build_flags(s, &fr, NULL);
  if (module_find("notes", 0, 0)) {
    Tcl_SetVar(interp, "user", u->handle, 0);
    if (Tcl_VarEval(interp, "notes ", "$user", NULL) == TCL_OK)
      n = atoi(interp->result);
  }
  li = get_user(&USERENTRY_LASTON, u);
  if (!li || !li->laston)
    strcpy(s1, "never");
  else {
    now2 = now - li->laston;
    if (now2 > 86400)
      egg_strftime(s1, 7, "%d %b", localtime(&li->laston));
    else
      egg_strftime(s1, 6, "%H:%M", localtime(&li->laston));
  }
  egg_snprintf(format, sizeof format, "%%-%us %%-5s%%5d %%-15s %%s (%%s)\n",
               HANDLEN);
  dprintf(idx, format, u->handle,
          get_user(&USERENTRY_PASS, u) ? "yes" : "no", n, s, s1,
          (li && li->lastonplace) ? li->lastonplace : "nowhere");
  /* channel flags? */
  for (ch = u->chanrec; ch; ch = ch->next) {
    fr.match = FR_CHAN | FR_GLOBAL;
    get_user_flagrec(dcc[idx].user, &fr, ch->channel);
    if (!glob_op(fr) && !chan_op(fr))
      continue;
    if (ch->laston == 0L)
      strcpy(s1, "never");
    else {
      now2 = now - (ch->laston);
      if (now2 > 86400)
        egg_strftime(s1, 7, "%d %b", localtime(&ch->laston));
      else
        egg_strftime(s1, 6, "%H:%M", localtime(&ch->laston));
    }
    fr.match = FR_CHAN;
    fr.chan = ch->flags;
    fr.udef_chan = ch->flags_udef;
    build_flags(s, &fr, NULL);
    egg_snprintf(format, sizeof format, "%%%us  %%-18s %%-15s %%s\n",
                  HANDLEN - 9);
    dprintf(idx, format, " ", ch->channel, s, s1);
    if (ch->info != NULL)
      dprintf(idx, "    INFO: %s\n", ch->info);
  }
  /* user-defined extra fields */
  for (ue = u->entries; ue; ue = ue->next) {
    if (!ue->name && ue->type->display)
      ue->type->display(idx, ue);
  }
}

void tell_user_ident(int idx, char *id, int master)
{
  char format[81];
  struct userrec *u;

  u = get_user_by_handle(userlist, id);
  if (!u)
    u = get_user_by_host(id);
  if (!u) {
    dprintf(idx, "%s.\n", USERF_NOMATCH);
    return;
  }
  egg_snprintf(format, sizeof format,
               "%%-%us PASS NOTES FLAGS           LAST\n", HANDLEN);
  dprintf(idx, format, "HANDLE");
  tell_user(idx, u, master);
}

void tell_users_match(int idx, char *mtch, int start, int limit, int master,
                      char *chname)
{
  char format[81];
  int fnd = 0, cnt, nomns = 0, flags = 0;
  struct userrec *u;
  struct list_type *q;
  struct flag_record user, pls, mns;

  dprintf(idx, "*** %s '%s':\n", MISC_MATCHING, mtch);
  cnt = 0;
  egg_snprintf(format, sizeof format,
               "%%-%us PASS NOTES FLAGS           LAST\n", HANDLEN);
  dprintf(idx, format, "HANDLE");
  if (start > 1)
    dprintf(idx, "(%s %d)\n", MISC_SKIPPING, start - 1);
  if (strchr("+-&|", *mtch)) {
    user.match = pls.match = FR_GLOBAL | FR_BOT | FR_CHAN;
    break_down_flags(mtch, &pls, &mns);
    mns.match = pls.match ^ (FR_AND | FR_OR);
    if (!mns.global &&!mns.udef_global && !mns.chan && !mns.udef_chan &&
        !mns.bot) {
      nomns = 1;
      if (!pls.global &&!pls.udef_global && !pls.chan && !pls.udef_chan &&
          !pls.bot) {
        dprintf(idx, "Unknown flag specified for matching!\n");
        return;
      }
    }
    if (!chname || !chname[0])
      chname = dcc[idx].u.chat->con_chan;
    flags = 1;
  }

  for (u = userlist; u; u = u->next) {
    if (flags) {
      get_user_flagrec(u, &user, chname);
      if (flagrec_eq(&pls, &user) && (nomns || !flagrec_eq(&mns, &user))) {
        cnt++;
        if (cnt <= limit && cnt >= start)
          tell_user(idx, u, master);
        if (cnt == limit + 1)
          dprintf(idx, MISC_TRUNCATED, limit);
      }
    } else if (wild_match(mtch, u->handle)) {
      cnt++;
      if (cnt <= limit && cnt >= start)
        tell_user(idx, u, master);
      if (cnt == limit + 1)
        dprintf(idx, MISC_TRUNCATED, limit);
    } else {
      fnd = 0;
      for (q = get_user(&USERENTRY_HOSTS, u); q; q = q->next) {
        if (!wild_match(mtch, q->extra) || fnd)
          continue;
        cnt++;
        fnd = 1;
        if (cnt <= limit && cnt >= start)
          tell_user(idx, u, master);
        if (cnt == limit + 1)
          dprintf(idx, MISC_TRUNCATED, limit);
      }
    }
  }

  dprintf(idx, MISC_FOUNDMATCH, cnt, cnt == 1 ? "" : MISC_MATCH_PLURAL);
}

/* New methodology - cycle through list 3 times
 * 1st time scan for +sh bots and link if none connected
 * 2nd time scan for +h bots
 * 3rd time scan for +a/+h bots */
void autolink_cycle(char *start)
{
  struct userrec *u = userlist, *autc = NULL;
  static int cycle = 0;
  int got_hub = 0, got_alt = 0, got_shared = 0, linked, ready = 0, i, bfl;

  /* Don't start a new cycle if some links are still pending. */
  if (!start) {
    for (i = 0; i < dcc_total; i++) {
      if (dcc[i].type == &DCC_BOT_NEW)
        return;
      if (dcc[i].type == &DCC_FORK_BOT)
        return;
      if ((dcc[i].type == &DCC_DNSWAIT) && dcc[i].u.dns &&
          (dcc[i].u.dns->type == &DCC_FORK_BOT))
        return;
    }
  }
  if (!start) {
    ready = 1;
    cycle = 0;
  }
  while (u && !autc) {
    while (u && !autc) {
      if (u->flags & USER_BOT && strcmp(u->handle, botnetnick)) { /* Ignore our own user record. */
        bfl = bot_flags(u);
        if (bfl & (BOT_HUB | BOT_ALT)) {
          linked = 0;
          for (i = 0; i < dcc_total; i++) {
            if (dcc[i].user == u) {
              if (dcc[i].type == &DCC_BOT)
                linked = 1;
              if (dcc[i].type == &DCC_BOT_NEW)
                linked = 1;
              if (dcc[i].type == &DCC_FORK_BOT)
                linked = 1;
            }
          }
          if ((bfl & BOT_HUB) && (bfl & BOT_SHARE)) {
            if (linked)
              got_shared = 1;
            else if (!cycle && ready && !autc)
              autc = u;
          } else if ((bfl & BOT_HUB) && cycle > 0) {
            if (linked)
              got_hub = 1;
            else if ((cycle == 1) && ready && !autc)
              autc = u;
          } else if ((bfl & BOT_ALT) && (cycle == 2)) {
            if (linked)
              got_alt = 1;
            else if (!in_chain(u->handle) && ready && !autc)
              autc = u;
          }
          if (!ready && !egg_strcasecmp(u->handle, start)) {
            ready = 1;
            autc = NULL;
            /* If starting point is a +h bot, must be in 2nd cycle */
            if ((bfl & BOT_HUB) && !(bfl & BOT_SHARE))
              cycle = 1;
            /* If starting point is a +a bot, must be in 3rd cycle */
            if (bfl & BOT_ALT)
              cycle = 2;
          }
        }
        if (!cycle && (bfl & BOT_REJECT) && in_chain(u->handle)) {
          /* Get rid of nasty reject bot. */
          int i;

          i = nextbot(u->handle);
          if (i >= 0 && !egg_strcasecmp(dcc[i].nick, u->handle)) {
            char *p = MISC_REJECTED;

            /* We're directly connected to the offending bot?! (shudder!) */
            putlog(LOG_BOTS, "*", "%s %s", BOT_REJECTING, dcc[i].nick);
            chatout("*** %s bot %s\n", p, dcc[i].nick);
            botnet_send_unlinked(i, dcc[i].nick, p);
            dprintf(i, "bye %s\n", BOT_REJECTING);
            killsock(dcc[i].sock);
            lostdcc(i);
          } else if (i < 0 && egg_strcasecmp(botnetnick, u->handle)) {
            /* The bot is not connected, but listed in our tandem list! */
            putlog(LOG_BOTS, "*", "(!) BUG: rejecting not connected bot %s!",
                   u->handle);
            rembot(u->handle);
          }
        }
      }
      u = u->next;
    }
    if (!autc) {
      if (!cycle && !got_shared) {
        cycle++;
        u = userlist;
      } else if ((cycle == 1) && !(got_shared || got_hub)) {
        cycle++;
        u = userlist;
      }
    }
  }
  if (got_shared && !cycle)
    autc = NULL;
  else if ((got_shared || got_hub) && (cycle == 1))
    autc = NULL;
  else if ((got_shared || got_hub || got_alt) && (cycle == 2))
    autc = NULL;
  if (autc)
    botlink("", -3, autc->handle); /* Try autoconnect. */
}
