/* bg.c
 *
 * Originally by Darrin Smith (beldin@light.iinet.net.au)
 *
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
 *
 *
 * When threads are started during eggdrop's init phase, we can't simply
 * fork() later on, because that only copies the VM space over and doesn't
 * actually duplicate the threads.
 *
 * To work around this, we fork() very early and let the parent process wait
 * in an event loop. As soon as the init phase is completed and we would
 * normally move to the background, the child process simply messages it's
 * parent that it may now quit. This allows us to control the terminal long
 * enough to, e.g. properly feed error messages to cron scripts and let the
 * user abort the loading process by hitting CTRL+C.
 *
 * [ Parent process                  ] [ Child process                   ]
 *
 *   main()
 *     ...
 *     bg_prepare_split()
 *       fork()                              - created -
 *       waiting in event loop.            continuing execution in main()
 *                                         ...
 *                                         completed init.
 *                                         bg_do_detach()
 *                                           message parent new PID file-
 *                                            name.
 *       receives new PID filename
 *         message.
 *                                           message parent to quit.
 *       receives quit message.            continues to main loop.
 *       writes PID to file.               ...
 *       exits.
 *
 *
 * $Id: bg.c,v 1.3 2005/01/21 01:43:39 wcc Exp $
 */

#include "main.h"
#include <signal.h>

#include "bg.h"


#ifdef HAVE_TCL_THREADS
#  define SUPPORT_THREADS
#endif


extern char pid_file[];


#ifdef SUPPORT_THREADS
static bg_t bg = { 0 };
#endif


/* Do everything we normally do after we have split off a new process to
 * the background. This includes writing a PID file and informing the user
 * of the split.
 */
static void bg_do_detach(pid_t p)
{
  FILE *fp;

  /* Need to attempt to write PID now, not later. */
  unlink(pid_file);
  fp = fopen(pid_file, "w");
  if (fp == NULL)
    printf("Cannot not write to '%s' (PID file). Check for correct file "
           "permissions.", pid_file);
  else {
    fprintf(fp, "%u\n", p);
    if (fflush(fp)) {
      /* Kill bot incase a botchk is run from crond. */
      printf("Cannot not write to '%s' (PID file). Make sure you have "
             "sufficient disk space.\n", pid_file);
      fclose(fp);
      unlink(pid_file);
      exit(1);
    }
    fclose(fp);
  }
  printf("Launched into the background (PID: %d)\n\n", p);
#ifdef HAVE_SETPGID
  setpgid(p, p);
#endif
  exit(0);
}

void bg_prepare_split()
{
#ifdef SUPPORT_THREADS
  /* Create a pipe between parent and split process, fork to create a
   * parent and a split process and wait for messages on the pipe. */
  pid_t p;
  bg_comm_t message;
  int comm_pair[2];

  if (pipe(comm_pair) < 0)
    fatal("CANNOT OPEN PIPE.", 0);

  bg.comm_recv = comm_pair[0];
  bg.comm_send = comm_pair[1];

  p = fork();
  if (p == -1)
    fatal("CANNOT FORK PROCESS.", 0);
  if (p == 0) {
    bg.state = BG_SPLIT;
    return;
  }
  else {
    bg.child_pid = p;
    bg.state = BG_PARENT;
  }

  while (read(bg.comm_recv, &message, sizeof(message)) > 0) {
    switch (message.comm_type) {
    case BG_COMM_QUIT:
      bg_do_detach(p);
      break;
    case BG_COMM_ABORT:
      exit(1);
      break;
    case BG_COMM_TRANSFERPF:
      /* Now transferring file from split process. */
      if (message.comm_data.transferpf.len >= 40)
        message.comm_data.transferpf.len = 40 - 1;
      /* Next message contains data. */
      if (read(bg.comm_recv, pid_file, message.comm_data.transferpf.len) <= 0)
        goto error;
      pid_file[message.comm_data.transferpf.len] = 0;
      break;
    }
  }

error:
  /* We only reach this point in case of an error. */
  fatal("COMMUNICATION THROUGH PIPE BROKE.", 0);
#endif /* SUPPORT_THREADS */
}

#ifdef SUPPORT_THREADS
/* Transfer contents of pid_file to parent process. This is necessary,
 * as the pid_file[] buffer has changed in this fork by now, but the
 * parent needs an up-to-date version.
 */
static void bg_send_pidfile()
{
  bg_comm_t message;

  message.comm_type = BG_COMM_TRANSFERPF;
  message.comm_data.transferpf.len = strlen(pid_file);

  /* Send type message. */
  if (write(bg.comm_send, &message, sizeof(message)) < 0)
    goto error;
  /* Send data. */
  if (write(bg.comm_send, pid_file, message.comm_data.transferpf.len) < 0)
    goto error;
  return;
error:
  fatal("COMMUNICATION THROUGH PIPE BROKE.", 0);
}
#endif /* SUPPORT_THREADS */

void bg_send_quit(bg_quit_t q)
{
#ifdef SUPPORT_THREADS
  if (bg.state == BG_PARENT)
    kill(bg.child_pid, SIGKILL);
  else if (bg.state == BG_SPLIT) {
    bg_comm_t message;

    if (q == BG_QUIT) {
      bg_send_pidfile();
      message.comm_type = BG_COMM_QUIT;
    }
    else
      message.comm_type = BG_COMM_ABORT;
    /* Send message. */
    if (write(bg.comm_send, &message, sizeof(message)) < 0)
      fatal("COMMUNICATION THROUGH PIPE BROKE.", 0);
  }
#endif /* SUPPORT_THREADS */
}

void bg_do_split()
{
#ifdef SUPPORT_THREADS
  /* Tell our parent process to go away now, as we don't need it anymore. */
  bg_send_quit(BG_QUIT);
#else
  /* Split off a new process. */
  int x;

  x = fork();
  if (x == -1)
    fatal("CANNOT FORK PROCESS.", 0);
  if (x != 0)
    bg_do_detach(x);
#endif /* SUPPORT_THREADS */
}
