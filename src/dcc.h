/* dcc.h
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
 * $Id: dcc.h,v 1.2 2004/08/26 03:21:13 wcc Exp $
 */

#ifndef _EGG_DCC_H
#define _EGG_DCC_H

#include "types.h" /* u_8bit_t */

/* Telnet codes. See RFC 854 RFC 875 for details. */
#define TLN_AYT     246     /* Are You There        */
#define TLN_WILL    251     /* Will                 */
#define TLN_WILL_C  "\373"
#define TLN_WONT    252     /* Won't                */
#define TLN_WONT_C  "\374"
#define TLN_DO      253     /* Do                   */
#define TLN_DO_C    "\375"
#define TLN_DONT    254     /* Don't                */
#define TLN_DONT_C  "\376"
#define TLN_IAC     255     /* Interpret As Command */
#define TLN_IAC_C   "\377"
#define TLN_ECHO    1       /* Echo                 */
#define TLN_ECHO_C  "\001"

/* For stripping out mIRC codes. */
#define STRIP_COLOR 0x00001  /* remove all color codes         */
#define STRIP_BOLD  0x00002  /* remove all boldface codes      */
#define STRIP_REV   0x00004  /* remove all reverse video codes */
#define STRIP_UNDER 0x00008  /* remove underline codes         */
#define STRIP_ANSI  0x00010  /* remove all ANSI codes          */
#define STRIP_BELLS 0x00020  /* remove all ctrl-g (bell) codes */
#define STRIP_ALL   0x00040  /* remove all of the above        */

struct dcc_table {
  char *name;
  int flags;
  void (*eof) (int);
  void (*activity) (int, char *, int);
  int *timeout_val;
  void (*timeout) ();
  void (*display) (int, char *);
  int (*expmem) (void *);
  void (*kill) (int, void *);
  void (*output) (int, char *, void *);
  void (*outdone) (int);
};

struct bot_info {
  char version[121];            /* channel/version info    */
  char linker[NOTENAMELEN + 1]; /* who requested this link */
  int numver;                   /* numversion              */
  int port;                     /* base port               */
  int uff_flags;                /* user file feature flags */
};

struct chat_info {
  char *away;          /* non-NULL if user is away             */
  int msgs_per_sec;    /* used to stop flooding                */
  int con_flags;       /* with console: what to show           */
  int strip_flags;     /* what codes to strip (b,r,u,c,a,g,*)  */
  char con_chan[81];   /* with console: what channel to view   */
  int channel;         /* 0=party line, -1=off                 */
  struct msgq *buffer; /* a buffer of outgoing lines (for .page cmd) */
  int max_line;        /* maximum lines at once                */
  int line_count;      /* number of lines sent since last page */
  int current_lines;   /* number of lines total stored         */
  char *su_nick;       /* non-NULL if user is su'd             */
};

struct relay_info {
  struct chat_info *chat;
  int sock;
  int port;
  int old_status;
};

struct file_info {
  struct chat_info *chat; /* chat info */
  char dir[161];          /* CWD */
};

struct script_info {
  struct dcc_table *type;
  union {
    struct chat_info *chat;
    struct file_info *file;
    void *other;
  } u;
  char command[121];
};

struct dupwait_info {
  int atr;                /* Attributes */
  struct chat_info *chat; /* chat info */
};

struct dns_info {
  void (*dns_success) (int);    /* is called if the dns request succeeds     */
  void (*dns_failure) (int);    /* is called if it fails                     */
  char *host;                   /* hostname                                  */
  char *cbuf;                   /* temporary buffer. free'd when dns_info is */
  char *cptr;                   /* temporary pointer                         */
  IP ip;                        /* IP address                                */
  int ibuf;                     /* temporary buffer for one integer          */
  u_8bit_t dns_type;            /* lookup type: RES_IPBYHOST or RES_HOSTBYIP */
  struct dcc_table *type;       /* type we are doing the lookup for          */
};

struct xfer_info {
  char *filename;
  char *origname;
  char dir[DIRLEN];            /* used when uploads go to the current dir        */
  unsigned long length;
  unsigned long acked;
  char buf[4];
  unsigned char sofar;         /* how much of the byte count received            */
  char from[NICKLEN];          /* [GET] user who offered the file                */
  FILE *f;                     /* pointer to file being sent/received            */
  unsigned int type;           /* xfer connection type, see enum below           */
  unsigned short ack_type;     /* type of ack                                    */
  unsigned long offset;        /* offset from beginning of file during resend    */
  unsigned long block_pending; /* bytes of this DCC block which weren't sent yet */
  time_t start_time;           /* Time when a xfer was started.                  */
};

enum {
  XFER_SEND,
  XFER_RESEND,
  XFER_RESEND_PEND,
  XFER_RESUME,
  XFER_RESUME_PEND,
  XFER_GET
};

enum {
  XFER_ACK_UNKNOWN,       /* We don't know how blocks are acked.       */
  XFER_ACK_WITH_OFFSET,   /* Skipped data is also counted as received. */
  XFER_ACK_WITHOUT_OFFSET /* Skipped data is NOT counted in ack.       */
};


struct userrec;

struct dcc_t {
  long sock;            /* This should be a long to keep 64-bit machines sane.           */
  IP addr;              /* IP address in host byte order.                                */
  unsigned int port;
  struct userrec *user;
  char nick[NICKLEN];
  char host[UHOSTLEN];
  struct dcc_table *type;
  time_t timeval;       /* This is used for timeout checking.                            */
  unsigned long status; /* A LOT of dcc types have status things; makes it more avaliabe */
  union {
    struct chat_info *chat;
    struct file_info *file;
    struct edit_info *edit;
    struct xfer_info *xfer;
    struct bot_info *bot;
    struct relay_info *relay;
    struct script_info *script;
    struct dns_info *dns;
    struct dupwait_info *dupwait;
    int ident_sock;
    void *other;
  } u;                  /* Special use depending on type.                                */
};



#ifndef MAKING_MODS
extern struct dcc_table DCC_CHAT, DCC_BOT, DCC_LOST, DCC_SCRIPT, DCC_BOT_NEW,
                        DCC_RELAY, DCC_RELAYING, DCC_FORK_RELAY, DCC_PRE_RELAY,
                        DCC_CHAT_PASS, DCC_FORK_BOT, DCC_SOCKET, DCC_TELNET_ID,
                        DCC_TELNET_NEW, DCC_TELNET_PW, DCC_TELNET, DCC_IDENT,
                        DCC_IDENTWAIT, DCC_DNSWAIT;
#endif

#ifndef MAKING_MODS
void failed_link(int);
void strip_mirc_codes(int, char *);
int check_ansi(char *);
void dupwait_notify(char *);
#endif

#endif /* !_EGG_DCC_H */
