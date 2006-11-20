/* chanprog.c
 *
 * Copyright (C) 1997 Robey Pointer
 * Copyright (C) 1999 - 2006 Eggheads Development Team
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
 * $Id: chanprog.c,v 1.10 2006/11/20 13:53:33 tothwolf Exp $
 */

#include "main.h"

#include "chanprog.h"
#include "misc.h"    /* strncpyz, splitnick, rmspace */
#include "rfc1459.h" /* rfc_casecmp */
#include "userrec.h" /* clear_userlist */


extern struct userrec *userlist;
extern char owner[];
extern int noshare;


struct chanset_t *chanset = NULL; /* Channel list. */


/* Returns memberfields if the nick is in the member list. */
memberlist *ismember(struct chanset_t *chan, char *nick)
{
  register memberlist *x;

  for (x = chan->channel.member; x && x->nick[0]; x = x->next) {
    if (!rfc_casecmp(x->nick, nick))
      return x;
  }
  return NULL;
}

/* Find a chanset by channel name as the server knows it (ie !ABCDEchannel) */
struct chanset_t *findchan(const char *name)
{
  register struct chanset_t *chan;

  for (chan = chanset; chan; chan = chan->next) {
    if (!rfc_casecmp(chan->name, name))
      return chan;
  }
  return NULL;
}

/* Find a chanset by display name (ie !channel) */
struct chanset_t *findchan_by_dname(const char *name)
{
  register struct chanset_t *chan;

  for (chan = chanset; chan; chan = chan->next) {
    if (!rfc_casecmp(chan->dname, name))
      return chan;
  }
  return NULL;
}

/* Oddly enough, written by Sup (former(?) Eggdrop coder) */
int isowner(char *name)
{
  register char *ptr = NULL, *s = NULL, *n = NULL;

  if (!owner || !name)
    return 0;

  ptr = owner - 1;

  do {
    ptr++;
    if (*ptr && !egg_isspace(*ptr) && *ptr != ',') {
      if (!s)
        s = ptr;
    } else if (s) {
      for (n = name; *n && *s && s < ptr && tolower(*n) == tolower(*s); n++, s++);

      if (s == ptr && !*n)
        return 1;

      s = NULL;
    }
  } while (*ptr);

  return 0;
}

void reaffirm_owners()
{
  char *p, *q, s[121];
  struct userrec *u;

  /* Please stop breaking this function. */
  if (!owner[0])
    return;

  q = owner;
  p = strchr(q, ',');
  while (p) {
    strncpyz(s, q, (p - q) + 1);
    rmspace(s);
    u = get_user_by_handle(userlist, s);
    if (u)
      u->flags = sanity_check(u->flags | USER_OWNER);
    q = p + 1;
    p = strchr(q, ',');
  }
  strcpy(s, q);
  rmspace(s);
  u = get_user_by_handle(userlist, s);
  if (u)
    u->flags = sanity_check(u->flags | USER_OWNER);
}
