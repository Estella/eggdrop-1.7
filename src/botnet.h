/* botnet.h
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
 * $Id: botnet.h,v 1.5 2005/01/21 01:43:39 wcc Exp $
 */

#ifndef _EGG_BOTNET_H
#define _EGG_BOTNET_H

#include "types.h" /* tand_t */

#ifndef MAKING_MODS
void addbot(char *, char *, char *, char, int);
void updatebot(int, char *, char, int);
tand_t *findbot(char *);
int partysock(char *, char *);
int addparty(char *, char *, int, char, int, char *, int *);
void partystat(char *, int, int, int);
void partysetidle(char *, int, int);
int getparty(char *, int);
int partyidle(char *, char *);
int partynick(char *, int, char *);
void partyaway(char *, int, char *);
void rembot(char *);
void remparty(char *, int);
void unvia(int, tand_t *);
int nextbot(char *);
char *lastbot(char *);
void answer_local_whom(int, int);
void tell_bots(int);
void tell_bottree(int, int);
void dump_links(int);
int in_chain(char *);
int bots_in_subtree(tand_t *);
int users_in_subtree(tand_t *);
int botunlink(int, char *, char *, char *);
int botlink(char *, int, char *);
void check_botnet_pings();
void tandem_relay(int, char *, int);
void zapfbot(int);
#endif

#endif /* !_EGG_BOTNET_H */
