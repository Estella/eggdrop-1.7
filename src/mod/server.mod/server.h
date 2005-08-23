/*
 * server.h -- part of server.mod
 *
 * $Id: server.h,v 1.6 2005/08/23 14:52:57 guppy Exp $
 */
/*
 * Copyright (C) 1997 Robey Pointer
 * Copyright (C) 1999 - 2005 Eggheads Development Team
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
 */

#ifndef _EGG_MOD_SERVER_SERVER_H
#define _EGG_MOD_SERVER_SERVER_H

#define check_tcl_ctcp(a,b,c,d,e,f) check_tcl_ctcpr(a,b,c,d,e,f,H_ctcp)
#define check_tcl_ctcr(a,b,c,d,e,f) check_tcl_ctcpr(a,b,c,d,e,f,H_ctcr)

#ifndef MAKING_SERVER

/* 4 - 7 */
/* Empty (4) */
#define botuserhost ((char *)(server_funcs[5]))
#define serv (*(int *)(server_funcs[6]))
#define flud_thr (*(int*)(server_funcs[7]))
/* 8 - 11 */
#define flud_time (*(int*)(server_funcs[8]))
#define flud_ctcp_thr (*(int*)(server_funcs[9]))
#define flud_ctcp_time (*(int*)(server_funcs[10]))
#define match_my_nick ((int(*)(char *))server_funcs[11])
/* 12 - 15 */
#define check_tcl_flud ((int (*)(char *,char *,struct userrec *,char *,char *))server_funcs[12])
#define answer_ctcp (*(int *)(server_funcs[13]))
#define trigger_on_ignore (*(int *)(server_funcs[14]))
#define check_tcl_ctcpr ((int(*)(char*,char*,struct userrec*,char*,char*,char*,p_tcl_bind_list))server_funcs[15])
/* 16 - 19 */
/* was detect_avalanche - 16 */
#define nuke_server ((void(*)(char *))server_funcs[17])
#define newserver ((char *)(server_funcs[18]))
#define newserverport (*(int *)(server_funcs[19]))
/* 20 - 23 */
#define newserverpass ((char *)(server_funcs[20]))
#define cycle_time (*(int *)(server_funcs[21]))
#define default_port (*(int *)(server_funcs[22]))
#define server_online (*(int *)(server_funcs[23]))
/* 24 - 27 */
/* was min_servs - 24 */
#define H_raw (*(p_tcl_bind_list *)(server_funcs[25]))
#define H_wall (*(p_tcl_bind_list *)(server_funcs[26]))
#define H_msg (*(p_tcl_bind_list *)(server_funcs[27]))
/* 28 - 31 */
#define H_msgm (*(p_tcl_bind_list *)(server_funcs[38]))
#define H_notc (*(p_tcl_bind_list *)(server_funcs[29]))
#define H_flud (*(p_tcl_bind_list *)(server_funcs[30]))
#define H_ctcp (*(p_tcl_bind_list *)(server_funcs[31]))
/* 32 - 35 */
#define H_ctcr (*(p_tcl_bind_list *)(server_funcs[32]))
#define ctcp_reply ((char *)(server_funcs[33]))
#define get_altbotnick ((char *(*)(void))(server_funcs[34]))
#define nick_len (*(int *)(server_funcs[35]))
/* 36 - 37 */
#define check_tcl_notc ((int (*)(char *,char *,struct userrec *,char *,char *))server_funcs[36])
#define exclusive_binds (*(int *)(server_funcs[37]))

#else /* MAKING_SERVER */

/* Macros for commonly used commands. */
#define free_null(ptr)  do {                            \
        nfree(ptr);                                     \
        ptr = NULL;                                     \
} while (0)

#define write_to_server(x,y) do {                       \
        tputs(serv, (x), (y));                          \
        tputs(serv, "\r\n", 2);                         \
} while (0)

#endif /* MAKING_SERVER */

struct server_list {
  struct server_list *next;

  char *name;
  int port;
  char *pass;
  char *realname;
};

/* Available net types.  */
enum {
  NETT_EFNET        = 0, /* EFnet                    */
  NETT_IRCNET       = 1, /* IRCnet                   */
  NETT_UNDERNET     = 2, /* UnderNet                 */
  NETT_DALNET       = 3, /* DALnet                   */
  NETT_HYBRID_EFNET = 4  /* +e/+I/max-bans 20 Hybrid */
} nett_t;

#endif /* _EGG_MOD_SERVER_SERVER_H */
