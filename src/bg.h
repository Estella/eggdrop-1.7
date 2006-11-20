/* bg.h
 *
 * Copyright (C) 2000 - 2006 Eggheads Development Team
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
 * $Id: bg.h,v 1.5 2006/11/20 13:53:32 tothwolf Exp $
 */

#ifndef _EGG_BG_H
#define _EGG_BG_H

typedef enum {
  BG_QUIT = 1,
  BG_ABORT
} bg_quit_t;

/* Format of messages sent from the newly forked process to the original
 * process, connected to the terminal.
 */
typedef struct {
  enum {
    BG_COMM_QUIT,      /* Quit original process. Write PID file, detach. */
    BG_COMM_ABORT,     /* Quit original process.                         */
    BG_COMM_TRANSFERPF /* Sending pid_file.                              */
  } comm_type;
  union {
    struct {           /* Data for BG_COMM_TRANSFERPF.                   */
      int len;         /* Length of the file name.                       */
    } transferpf;
  } comm_data;
} bg_comm_t;

typedef enum {
  BG_NONE = 0, /* No forking has taken place yet. */
  BG_SPLIT,    /* I'm the newly forked process.   */
  BG_PARENT    /* I'm the original process.       */
} bg_state_t;

typedef struct {
  int comm_recv;    /* Receives messages from the child process.   */
  int comm_send;    /* Sends messages to the parent process.       */
  bg_state_t state; /* Current state, see above enum descriptions. */
  pid_t child_pid;  /* PID of split process.                       */
} bg_t;

#ifndef MAKING_MODS
void bg_prepare_split();
void bg_send_quit(bg_quit_t q);
void bg_do_split();
#endif

#endif /* _EGG_BG_H */
