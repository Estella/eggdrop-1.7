/* dcc.h
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
 * $Id: dcc.h,v 1.7 2005/01/21 01:43:40 wcc Exp $
 */

#ifndef _EGG_DCC_H
#define _EGG_DCC_H

#include "types.h" /* u_8bit_t */

/* Telnet codes. See RFC 854 RFC 875 for details. */
#define TLN_AYT  246  /* Are You There        */
#define TLN_WILL 251  /* Will                 */
#define TLN_WONT 252  /* Won't                */
#define TLN_DO   253  /* Do                   */
#define TLN_DONT 254  /* Don't                */
#define TLN_IAC  255  /* Interpret As Command */
#define TLN_ECHO 1    /* Echo                 */

#define TLN_WILL_C "\373"
#define TLN_WONT_C "\374"
#define TLN_DO_C   "\375"
#define TLN_DONT_C "\376"
#define TLN_IAC_C  "\377"
#define TLN_ECHO_C "\001"

/* For stripping mIRC/ANSI codes. */
#define STRIP_COLOR 0x00001  /* Remove all color codes.         */
#define STRIP_BOLD  0x00002  /* Remove all boldface codes.      */
#define STRIP_REV   0x00004  /* Remove all reverse video codes. */
#define STRIP_UNDER 0x00008  /* Remove underline codes.         */
#define STRIP_ANSI  0x00010  /* Remove all ANSI codes.          */
#define STRIP_BELLS 0x00020  /* Remove all ctrl-g (bell) codes. */
#define STRIP_ALL   0x00040  /* Remove all of the above.        */

/* Flags for DCC types. */
#define DCT_CHAT      0x00000001  /* Can receive chat?                       */
#define DCT_MASTER    0x00000002  /* Can receive master-only chat?           */
#define DCT_SHOWWHO   0x00000004  /* Show in who?                            */
#define DCT_REMOTEWHO 0x00000008  /* Show in remote who?                     */
#define DCT_VALIDIDX  0x00000010  /* Report IDX as valid to tcl_valididx?    */
#define DCT_SIMUL     0x00000020  /* Can be tcl_simul/.simul'd?              */
#define DCT_CANBOOT   0x00000040  /* Can be booted?                          */
#define DCT_FILES     0x00000080  /* In the file ares?                       */
#define DCT_BOT       0x00000100  /* Bot connection?                         */
#define DCT_FORKTYPE  0x00000200  /* DCT_FILESEND from outside transfer.mod. */
#define DCT_FILETRAN  0x00000400  /* A file transfer of some sort.           */
#define DCT_FILESEND  0x00000800  /* A send *TO* the bot (FIXME!).           */
#define DCT_LISTEN    0x00001000  /* A listening port of some sort.          */
#define DCT_GETNOTES  DCT_CHAT    /* Can receive notes?                      */

/* Flags for listening sockets. */
#define LSTN_PUBLIC 0x000001  /* No access restrictions. */

/* Status flags for DCC/file area connections. */
#define STAT_ECHO    0x00001 /* Echo commands back?                           */
#define STAT_DENY    0x00002 /* Bad username (ignore pass & deny access)      */
#define STAT_CHAT    0x00004 /* Can return to partyline if in file area.      */
#define STAT_TELNET  0x00008 /* Connected via telnet (or CTCP chat (FIXME!)). */
#define STAT_PARTY   0x00010 /* Has the 'p' flag but isn't an op.             */
#define STAT_BOTONLY 0x00020 /* Bot-only listen port.                         */
#define STAT_USRONLY 0x00040 /* User-only port.                               */
#define STAT_PAGE    0x00080 /* Page output to the user.                      */

/* Status flags for bot links. */
#define BSTAT_PINGED      0x00001 /* Sent a pink, waiting for a pong.         */
#define BSTAT_SHARE       0x00002 /* This is a share-bot.                     */
#define BSTAT_CALLED      0x00004 /* This remote bot initiated the link.      */
#define BSTAT_OFFERED     0x00008 /* A userfile transfer has been offered.    */
#define BSTAT_SENDING     0x00010 /* In the process of sending a user list.   */
#define BSTAT_GETTING     0x00020 /* In the process of getting a user list.   */
#define BSTAT_WARNED      0x00040 /* Warned about unleaflike behavior.        */
#define BSTAT_LEAF        0x00080 /* This bot cannot act as a hub; leaf only. */
#define BSTAT_LINKING     0x00100 /* Bot is in the process of linking.        */
#define BSTAT_AGGRESSIVE  0x00200 /* Aggressively sharing with this bot.      */

/* Status flags for partyline users. */
#define PLSTAT_AWAY  0x001

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
  long sock;            /* This should be a long to keep 64-bit machines sane.  */
  IP addr;              /* IP address in network byte order.                    */
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
