/* botmsg.h
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
 * $Id: botmsg.h,v 1.4 2004/08/27 10:01:17 wcc Exp $
 */

#ifndef _EGG_BOTMSG_H
#define _EGG_BOTMSG_H

#include "types.h" /* tand_t */

/* Return codes for add_note(). */
#define NOTE_ERROR   0  /* Error                           */
#define NOTE_OK      1  /* Success                         */
#define NOTE_STORED  2  /* User not online; note stored.   */
#define NOTE_FULL    3  /* Too many notes stored for user. */
#define NOTE_TCL     4  /* Tcl binding caught it.          */
#define NOTE_AWAY    5  /* User is away; note stored.      */
#define NOTE_FWD     6  /* User is away; note forwarded.   */
#define NOTE_REJECT  7  /* Note matched an ignore.         */

/* Minimum version that uses tokens & base64 ints for channel msgs. */
#define NEAT_BOTNET 1029900

#ifndef MAKING_MODS
void tandout_but EGG_VARARGS(int, arg1);
int base64_to_int(char *);
char *int_to_base64(unsigned int);
char *int_to_base10(int);
char *unsigned_int_to_base10(unsigned int);
int simple_sprintf EGG_VARARGS(char *, arg1);
void send_tand_but(int, char *, int);
void botnet_send_chan(int, char *, char *, int, char *);
void botnet_send_chat(int, char *, char *);
void botnet_send_act(int, char *, char *, int, char *);
void botnet_send_ping(int);
void botnet_send_pong(int);
void botnet_send_priv EGG_VARARGS(int, arg1);
void botnet_send_who(int, char *, char *, int);
void botnet_send_infoq(int, char *);
void botnet_send_unlinked(int, char *, char *);
void botnet_send_traced(int, char *, char *);
void botnet_send_trace(int, char *, char *, char *);
void botnet_send_unlink(int, char *, char *, char *, char *);
void botnet_send_link(int, char *, char *, char *);
void botnet_send_update(int, tand_t *);
void botnet_send_nlinked(int, char *, char *, char, int);
void botnet_send_reject(int, char *, char *, char *, char *, char *);
void botnet_send_zapf(int, char *, char *, char *);
void botnet_send_zapf_broad(int, char *, char *, char *);
void botnet_send_motd(int, char *, char *);
void botnet_send_filereq(int, char *, char *, char *);
void botnet_send_filereject(int, char *, char *, char *);
void botnet_send_filesend(int, char *, char *, char *);
void botnet_send_away(int, char *, int, char *, int);
void botnet_send_idle(int, char *, int, int, char *);
void botnet_send_join_idx(int, int);
void botnet_send_join_party(int, int, int, int);
void botnet_send_part_idx(int, char *);
void botnet_send_part_party(int, int, char *, int);
void botnet_send_bye();
void botnet_send_nkch_part(int, int, char *);
void botnet_send_nkch(int, char *);
int add_note(char *, char *, char *, int, int);
#endif

#endif /* !_EGG_BOTMSG_H */
