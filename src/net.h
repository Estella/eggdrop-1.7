/* net.h
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
 * $Id: net.h,v 1.2 2004/08/27 10:01:17 wcc Exp $
 */

#ifndef _EGG_NET_H
#define _EGG_NET_H

#include "types.h" /* IP */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef NEXT_HACKS
#  define O_NONBLOCK 00000004 /* POSIX non-blocking I/O. */
#endif /* NEXT_HACKS */

#define iptolong(a) (0xffffffff & (long) (htonl((unsigned long) a)))

#define SOCK_UNUSED     0x0001  /* empty/unused socket                         */
#define SOCK_BINARY     0x0002  /* do not buffer input                  */
#define SOCK_LISTEN     0x0004  /* listening port                       */
#define SOCK_CONNECT    0x0008  /* connection attempt                   */
#define SOCK_NONSOCK    0x0010  /* used for file i/o on debug           */
#define SOCK_STRONGCONN 0x0020  /* don't report success until sure      */
#define SOCK_EOFD       0x0040  /* it EOF'd recently during a write     */
#define SOCK_PROXYWAIT  0x0080  /* waiting for SOCKS traversal          */
#define SOCK_PASS       0x0100  /* passed on; only notify in case of traffic                           */
#define SOCK_VIRTUAL    0x0200  /* not-connected socket (dont read it!) */
#define SOCK_BUFFER     0x0400  /* buffer data; don't notify dcc funcs  */

/* Flags for sock_has_data. */
enum {
  SOCK_DATA_OUTGOING, /* Data in out-queue? */
  SOCK_DATA_INCOMING  /* Data in in-queue?  */
};

/* Flags for sockoptions(). */
enum {
  EGG_OPTION_SET   = 1, /* Set option(s).  */
  EGG_OPTION_UNSET = 2  /* Unset option(s). */
};

typedef struct {
  int sock;
  short flags;
  char *inbuf;
  char *outbuf;
  unsigned long outbuflen; /* Outbuf could be binary data */
  unsigned long inbuflen;  /* Inbuf could be binary data  */
} sock_list;

#ifndef MAKING_MODS
IP my_atoul(char *);
IP getmyip();
void neterror(char *);
void setsock(int, int);
int allocsock(int, int);
int getsock(int);
void killsock(int);
int answer(int, char *, unsigned long *, unsigned short *, int);
inline int open_listen(int *);
int open_address_listen(IP, int *);
int open_telnet(char *, int);
int open_telnet_dcc(int, char *, char *);
int open_telnet_raw(int, char *, int);
void tputs(int, char *, unsigned int);
void dequeue_sockets();
int sockgets(char *, int *);
void tell_netdebug(int);
int sanitycheck_dcc(char *, char *, char *, char *);
int hostsanitycheck_dcc(char *, char *, IP, char *, char *);
char *iptostr(IP);
int sock_has_data(int, int);
int sockoptions(int, int, int);
#endif

#endif /* !_EGG_NET_H */
