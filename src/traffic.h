/* traffic.h
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
 * $Id: traffic.h,v 1.4 2005/01/21 01:43:40 wcc Exp $
 */

#ifndef _EGG_TRAFFIC_H
#define _EGG_TRAFFIC_H

#include "dcc.h" /* struxt dcc_table */

typedef struct {
  struct {
    unsigned long irc;
    unsigned long botnet;
    unsigned long partyline;
    unsigned long filesys;
    unsigned long transfer;
    unsigned long unknown;
  } in_total, in_today, out_total, out_today;
} egg_traffic_t;


#ifndef MAKING_MODS
void traffic_update_in(struct dcc_table *, int);
void traffic_update_out(struct dcc_table *, int);
void traffic_reset();
void traffic_init();
#endif

#endif /* !__EGG_TRAFFIC_H */
