/* main.c
 *
 * Copyright (C) 1997 Robey Pointer
 * Copyright (C) 1999 - 2006 Eggheads Development Team
 *
 * http://www.eggheads.org
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
 * $Id: main.c,v 1.25 2006/11/20 13:53:34 tothwolf Exp $
 */

#include "main.h"

#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <netdb.h>
#include <setjmp.h>

/* OSF/1 needs this. */
#ifdef STOP_UAC
#  include <sys/sysinfo.h>
#  define UAC_NOPRINT 0x00000001 /* Don't report unaligned fixups. */
#endif

#include "chan.h"
#include "modules.h"
#include "bg.h"

#include "botmsg.h"   /* botnet_send_* */
#include "botnet.h"   /* check_botnet_pings */
#include "cmds.h"     /* tell_verbose_status */
#include "chanprog.h" /* reaffirm_owners */
#include "dcc.h"      /* DCC_*, STRIP_*, STAT_*, struct chat_info, struct dcc_t */
#include "dccutil.h"  /* dprintf, dcc_chatter, lostdcc, tell_dcc, new_dcc,
                       * dcc_remove_lost */
#include "help.h"     /* add_help_reference */
#include "logfile.h"  /* LOG_*, putlog, logfile_init, flushlogs, check_expiring_logs,
                       * expire_all_logs, flush_and_check_logs, close_logs_at_midnight,
                       * switch_logs, LOGS_FIVEMINUTELY, LOGS_MINUTELY */
#include "mem.h"      /* nfree, tell_mem_status */
#include "misc.h"     /* strncpyz, newsplit, make_rand_str */
#include "net.h"      /* SOCK_*, getmyip, setsock, killsock, dequeue_sockets, sockgets */
#include "traffic.h"  /* traffic_update_out, traffic_reset, init_traffic */
#include "userfile.h" /* readuserfile, writeuserfile, backupuserfile */
#include "userrec.h"  /* adduser, count_users */

#ifdef STATIC
void check_static(char *, char *(*)());
#  include "mod/static.h"
#endif

#ifndef ENABLE_STRIP
#  include <sys/resource.h>
#endif

#ifdef CYGWIN_HACKS
#  include <windows.h>
#endif

/* Solaris needs this. */
#ifndef _POSIX_SOURCE
#  define _POSIX_SOURCE 1
#endif


extern char userfile[], botnetnick[];
extern int dcc_total, conmask, cache_hit, cache_miss, protect_readonly, noshare,
           switch_logfiles_at;
extern struct dcc_t *dcc;
extern struct userrec *userlist;
extern struct chanset_t *chanset;
extern module_entry *module_list;
extern Tcl_Interp *interp;
extern tcl_timer_t *timer, *utimer;
extern sigjmp_buf alarmret;

/* Please use patch.h instead of directly altering the version string. Also
 * please read the README file regarding your rights to distribute modified
 * versions of this bot.
 */
char egg_version[1024] = "1.7.0"; /* Text version.         */
int egg_numver = 1070000;         /* Numeric version.      */
char ver[41];                     /* Version info (short). */
char version[81];                 /* Version info (long).  */
char egg_xtra[2048];              /* Patch info.           */

char configfile[121] = "eggdrop.conf"; /* Default config file name. */

int backgrd = 1;    /* Run in the background?                        */
int con_chan = 0;   /* Foreground: constantly display channel stats? */
int term_z = 0;     /* Foreground: use the terminal as a partyline?  */
int use_stderr = 1; /* Send stuff to stderr instead of logfiles?     */

char notify_new[121] = ""; /* Person to send a note to for new users. */
int default_flags = 0;     /* Default user flags.                     */
int default_uflags = 0;    /* Default user-definied flags.            */
char pid_file[120];          /* Name of the pid file.     */
char helpdir[121] = "help/"; /* Directory of help files.  */
char textdir[121] = "text/"; /* Directory for text files. */
char tempdir[121] = "";      /* Directory for temp files. */
char admin[121] = "";        /* Admin info.               */
int make_userfile = 0;    /* Using bot in userfile-creation mode?            */
char owner[121] = "";     /* Permanent owner(s) of the bot                   */
int save_users_at = 0;    /* Minutes past the hour to save the userfile?     */
int notify_users_at = 0;  /* Minutes past the hour to notify users of notes? */
int die_on_sighup = 0;    /* Die if bot receives SIGHUP                      */
int die_on_sigterm = 1;   /* Die if bot receives SIGTERM                     */
int resolve_timeout = 15; /* Hostname/address lookup timeout                 */
char origbotname[NICKLEN + 1]; /* Bot's nick.      */
char botname[NICKLEN + 1];     /* Primary botname. */
int do_restart = 0;  /* .restart has been called; restart ASAP. */
char quit_msg[1024]; /* Quit message. */

time_t now;
static time_t then;
static int lastmin = 99;
static struct tm nowtm;
time_t online_since; /* Time that the bot was started. */

int init_mem();
int init_dcc_max();
int init_userent();
int init_bots();
int init_net();
int init_modules();
int init_tcl(int, char **);
int init_language(int);
void kill_tcl();
void restart_chons();


void fatal(const char *s, int recoverable)
{
  int i;

  putlog(LOG_MISC, "*", "* %s", s);
  flushlogs();

  for (i = 0; i < dcc_total; i++) {
    if (dcc[i].sock >= 0)
      killsock(dcc[i].sock);
  }

  unlink(pid_file);

  if (!recoverable) {
    bg_send_quit(BG_ABORT);
    exit(1);
  }
}

static void check_expired_dcc()
{
  int i;

  for (i = 0; i < dcc_total; i++)
    if (dcc[i].type && dcc[i].type->timeout_val &&
        ((now - dcc[i].timeval) > *(dcc[i].type->timeout_val))) {
      if (dcc[i].type->timeout)
        dcc[i].type->timeout(i);
      else if (dcc[i].type->eof)
        dcc[i].type->eof(i);
      else
        continue;
      /* Only timeout 1 socket per cycle, too risky for more. */
      return;
    }
}

static void got_bus(int z)
{
  write_debug();
  fatal("BUS ERROR -- CRASHING!", 1);
#ifdef SA_RESETHAND
  kill(getpid(), SIGBUS);
#else
  bg_send_quit(BG_ABORT);
  exit(1);
#endif
}

static void got_segv(int z)
{
  write_debug();
  fatal("SEGMENT VIOLATION -- CRASHING!", 1);
#ifdef SA_RESETHAND
  kill(getpid(), SIGSEGV);
#else
  bg_send_quit(BG_ABORT);
  exit(1);
#endif
}

static void got_fpe(int z)
{
  write_debug();
  fatal("FLOATING POINT ERROR -- CRASHING!", 0);
}

static void got_term(int z)
{
  writeuserfile(-1);
  check_tcl_event("sigterm");
  if (die_on_sigterm) {
    botnet_send_chat(-1, botnetnick, "ACK, I've been terminated!");
    fatal("RECEIVED TERMINATE SIGNAL -- SIGNING OFF!", 0);
  }
  else
    putlog(LOG_MISC, "*", "Received TERM signal (ignoring).");
}

static void got_quit(int z)
{
  check_tcl_event("sigquit");
  putlog(LOG_MISC, "*", "Received QUIT signal (ignoring).");
  return;
}

static void got_hup(int z)
{
  writeuserfile(-1);
  check_tcl_event("sighup");
  if (die_on_sighup) {
    fatal("HANGUP SIGNAL -- SIGNING OFF", 0);
  }
  else
    putlog(LOG_MISC, "*", "Received HUP signal: rehashing...");
  do_restart = -2;
  return;
}

/* A call to resolver (gethostbyname, etc) timed out. */
static void got_alarm(int z)
{
  siglongjmp(alarmret, 1);
  /* Never reached. */
}

/* Got ILL signal. */
static void got_ill(int z)
{
  check_tcl_event("sigill");
}

static void do_arg(char *s)
{
  char x[512], *z = x;
  int i;

  if (s[0] == '-') {
    for (i = 1; i < strlen(s); i++) {
      switch (s[i]) {
      case 'n':
        backgrd = 0;
        break;
      case 'c':
        con_chan = 1;
        term_z = 0;
        break;
      case 't':
        con_chan = 0;
        term_z = 1;
        break;
      case 'm':
        make_userfile = 1;
        break;
      case 'v':
        strncpyz(x, egg_version, sizeof x);
        newsplit(&z);
        newsplit(&z);
        printf("%s\n", version);
        if (z[0])
          printf("  (patches: %s)\n", z);
        bg_send_quit(BG_ABORT);
        exit(0);
        break; /* This should never be reached. */
      case 'h':
        printf("\n%s\n\n", version);
        printf(EGG_USAGE);
        printf("\n");
        bg_send_quit(BG_ABORT);
        exit(0);
        break; /* This should never be reached. */
      }
    }
  }
  else {
    strncpyz(configfile, s, sizeof configfile);
  }
}

static void readconfig()
{
  char s[161], rands[8];
  FILE *f;

  admin[0] = 0;
  helpdir[0] = 0;
  tempdir[0] = 0;

  expire_all_logs();

  /* Turn off read-only variables (make them write-able) for rehash. */
  protect_readonly = 0;

  /* Read the config file. */
  if (!readtclprog(configfile))
    fatal(MISC_NOCONFIGFILE, 0);

  /* Check logfiles which are expiring. */
  check_expiring_logs();

  /* We should be safe now. */
  call_hook(HOOK_REHASH);
  protect_readonly = 1;

  if (!botnetnick[0])
    strncpyz(botnetnick, origbotname, HANDLEN + 1);

  if (!botnetnick[0])
    fatal("I don't have a botnet nick!\n", 0);

  if (!userfile[0])
    fatal(MISC_NOUSERFILE2, 0);

  if (!readuserfile(userfile, &userlist)) {
    if (!make_userfile) {
      char tmp[178];

      egg_snprintf(tmp, sizeof tmp, MISC_NOUSERFILE, configfile);
      fatal(tmp, 0);
    }
    printf("\n\n%s\n", MISC_NOUSERFILE2);
    if (module_find("server", 0, 0))
      printf(MISC_USERFCREATE1, origbotname);
    printf("%s\n\n", MISC_USERFCREATE2);
  }
  else if (make_userfile) {
    make_userfile = 0;
    printf("%s\n", MISC_USERFEXISTS);
  }

  if (helpdir[0] && helpdir[strlen(helpdir) - 1] != '/')
    strcat(helpdir, "/");

  if (tempdir[0] && tempdir[strlen(tempdir) - 1] != '/')
    strcat(tempdir, "/");

  /* Test tempdir: it's vital.
   *
   * Possible file race condition solved by using a random string
   * and the process id in the filename.
   * FIXME: This race is only partitially fixed. We could still be
   *        overwriting an existing file / following a malicious
   *        link.
   */
  make_rand_str(rands, 7); /* create random string */
  sprintf(s, "%s.test-%u-%s", tempdir, getpid(), rands);
  f = fopen(s, "w");
  if (f == NULL)
    fatal(MISC_CANTWRITETEMP, 0);
  fclose(f);
  unlink(s);
  reaffirm_owners();
  check_tcl_event("userfile-loaded");
}

static void rehash()
{
  call_hook(HOOK_PRE_REHASH);

  noshare = 1;
  clear_userlist(userlist);
  noshare = 0;

  userlist = NULL;

  readconfig();
  /* FIXME: We should really call post rehash here as well, instead of in readconfig(). */
}

/* Called once a second. */
static void core_secondly()
{
  static int cnt = 0;
  int miltime;

  check_timers(&utimer); /* Secondly timers */

  cnt++;
  if (cnt >= 10) { /* Every 10 seconds */
    cnt = 0;
    check_expired_dcc();
    if (con_chan && !backgrd) {
      dprintf(DP_STDOUT, "\033[2J\033[1;1H");
      tell_verbose_status(DP_STDOUT);
      do_module_report(DP_STDOUT, 0, "server");
      do_module_report(DP_STDOUT, 0, "channels");
      tell_mem_status(DP_STDOUT);
    }
  }

  egg_memcpy(&nowtm, localtime(&now), sizeof(struct tm));
  if (nowtm.tm_min != lastmin) {
    int i = 0;

    /* Once a minute */
    lastmin = (lastmin + 1) % 60;
    call_hook(HOOK_MINUTELY);
    check_expired_ignores();
    autolink_cycle(NULL); /* Attempt autolinks. */

    /* In case for some reason more than 1 min has passed: */
    while (nowtm.tm_min != lastmin) {
      /* Timer drift, dammit */
      debug2("timer: drift (lastmin=%d, now=%d)", lastmin, nowtm.tm_min);
      i++;
      lastmin = (lastmin + 1) % 60;
      call_hook(HOOK_MINUTELY);
    }
    if (i > 1)
      putlog(LOG_MISC, "*", "(!) timer drift -- spun %d minutes", i);

    miltime = (nowtm.tm_hour * 100) + nowtm.tm_min;
    if (((int) (nowtm.tm_min / 5) * 5) == nowtm.tm_min) {
      /* 5-minutely. */
      call_hook(HOOK_5MINUTELY);
      check_botnet_pings();
      flush_and_check_logs(LOGS_FIVEMINUTELY);

      /* Is it midnight? */
      if (miltime == MILTIME_MIDNIGHT) {
        char s[25];

        strncpyz(s, ctime(&now), sizeof s);
        putlog(LOG_ALL, "*", "--- %.11s%s", s, s + 20);
        call_hook(HOOK_BACKUP);
        close_logs_at_midnight();
      }
    }

    if (nowtm.tm_min == notify_users_at)
      call_hook(HOOK_HOURLY);

    if (miltime == switch_logfiles_at) {
      call_hook(HOOK_DAILY);
      switch_logs();
    }
  }
}

/* Called once a minute. */
static void core_minutely()
{
  check_tcl_time(&nowtm);
  check_timers(&timer);
  flush_and_check_logs(LOGS_MINUTELY);
}

/* Called once an hour. */
static void core_hourly()
{
  writeuserfile(-1);
}


static void event_rehash()
{
  check_tcl_event("rehash");
}

static void event_prerehash()
{
  check_tcl_event("prerehash");
}

static void event_save()
{
  check_tcl_event("save");
}

static void event_logfile()
{
  check_tcl_event("logfile");
}

static void event_loaded()
{
  check_tcl_event("loaded");
}


void patch(const char *str)
{
  char *p = strchr(egg_version, '+');

  if (!p)
    p = &egg_version[strlen(egg_version)];
  sprintf(p, "+%s", str);
  egg_numver++;
  sprintf(&egg_xtra[strlen(egg_xtra)], " %s", str);
}

static inline void garbage_collect()
{
  static u_8bit_t run_cnt = 0;

  if (run_cnt == 3)
    garbage_collect_tclhash();
  else
    run_cnt++;
}

void setup_signal_traps()
{
  struct sigaction sv;

  sv.sa_handler = got_bus;
  sigemptyset(&sv.sa_mask);
#ifdef SA_RESETHAND
  sv.sa_flags = SA_RESETHAND;
#else
  sv.sa_flags = 0;
#endif
  sigaction(SIGBUS, &sv, NULL);

  sv.sa_handler = got_segv;
  sigaction(SIGSEGV, &sv, NULL);
#ifdef SA_RESETHAND
  sv.sa_flags = 0;
#endif

  sv.sa_handler = got_fpe;
  sigaction(SIGFPE, &sv, NULL);

  sv.sa_handler = got_term;
  sigaction(SIGTERM, &sv, NULL);

  sv.sa_handler = got_hup;
  sigaction(SIGHUP, &sv, NULL);

  sv.sa_handler = got_quit;
  sigaction(SIGQUIT, &sv, NULL);

  sv.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &sv, NULL);

  sv.sa_handler = got_ill;
  sigaction(SIGILL, &sv, NULL);

  sv.sa_handler = got_alarm;
  sigaction(SIGALRM, &sv, NULL);
}

/* FIXME: Move me somewhere out of main.c. */
int count_channels()
{
  int i = 0;
  struct chanset_t *chan;

  for (chan = chanset; chan; chan = chan->next) {
    i++;
  }

  return i;
}

void process_args(int argc, char **argv)
{
  int i;

  if (argc > 1) {
    for (i = 1; i < argc; i++)
      do_arg(argv[i]);
  }
}

int main(int argc, char **argv)
{
  int xx, i;
#ifdef STOP_UAC
  int nvpair[2];
#endif
  char buf[520], s[25];
  FILE *f;
#ifndef ENABLE_STRIP
  struct rlimit cdlim;
#endif

  /* Don't allow Eggdrop to run as root. */
  if (((int) getuid() == 0) || ((int) geteuid() == 0))
    fatal("ERROR: Eggdrop will not run as root!", 0);

#ifndef ENABLE_STRIP
  cdlim.rlim_cur = RLIM_INFINITY;
  cdlim.rlim_max = RLIM_INFINITY;
  setrlimit(RLIMIT_CORE, &cdlim);
#endif

#include "patch.h"
  /* Version info! */
  egg_snprintf(ver, sizeof ver, "eggdrop v%s", egg_version);
  egg_snprintf(version, sizeof version,
               "Eggdrop v%s (C) 1997 Robey Pointer (C) 2005 Eggheads",
               egg_version);
  /* Now add on the patchlevel (for Tcl) */
  sprintf(&egg_version[strlen(egg_version)], " %u", egg_numver);
  strcat(egg_version, egg_xtra);

#ifdef STOP_UAC
  nvpair[0] = SSIN_UACPROC;
  nvpair[1] = UAC_NOPRINT;
  setsysinfo(SSI_NVPAIRS, (char *) nvpair, 1, NULL, 0);
#endif

  /* Set up error / signal traps. */
  setup_signal_traps();

  /* Initialize a few variables before main loop. */
  cache_miss = 0;
  cache_hit  = 0;
  chanset    = NULL;
  now        = time(NULL);

  egg_memcpy(&nowtm, localtime(&now), sizeof(struct tm));
  lastmin = nowtm.tm_min;

  /* Initialize random number generator. */
  srandom((unsigned int) (now % (getpid() + getppid())));

  init_mem();
  init_language(1);

  /* Process command line arguments. */
  process_args(argc, argv);

  printf("\n%s\n", version);

  init_dcc_max();
  init_userent();
  logfile_init(0);
  init_bots();
  init_net();
  init_modules();

  if (backgrd)
    bg_prepare_split();

  init_tcl(argc, argv);
  init_language(0);
  help_init();
  traffic_init();
  logfile_init(1);

#ifdef STATIC
  link_statics();
#endif

  strncpyz(s, ctime(&now), sizeof s);
  strcpy(&s[11], &s[20]);
  putlog(LOG_ALL, "*", "--- Loading %s (%s)", ver, s);

  /* Read configuration data. */
  readconfig();

  /* Check for encryption module. */
  if (!encrypt_pass) {
    printf(MOD_NOCRYPT);
    bg_send_quit(BG_ABORT);
    exit(1);
  }

  putlog(LOG_MISC, "*", "=== %s: %d channels, %d users.", botnetnick,
         count_channels(), count_users(userlist));

  if (!pid_file[0])
    egg_snprintf(pid_file, sizeof pid_file, "pid.%s", botnetnick);

  /* Check for pre-existing eggdrop! */
  f = fopen(pid_file, "r");
  if (f != NULL) {
    fgets(s, 10, f);
    xx = atoi(s);
    kill(xx, SIGCHLD); /* Meaningless kill to determine if PID is used. */
    if (errno != ESRCH) {
      printf(EGG_RUNNING1, botnetnick);
      printf(EGG_RUNNING2, pid_file);
      bg_send_quit(BG_ABORT);
      exit(1);
    }
  }

  /* Move into background? */
  if (backgrd) {
#ifndef CYGWIN_HACKS
    bg_do_split();
  }
  else {
#endif
    xx = getpid();
    if (xx != 0) {
      FILE *fp;

      /* Write PID to file. */
      unlink(pid_file);
      fp = fopen(pid_file, "w");
      if (fp != NULL) {
        fprintf(fp, "%u\n", xx);
        if (fflush(fp)) {
          /* Let the bot live since this doesn't appear to be a botchk. */
          printf("Cannot not write to '%s' (PID file).\n", pid_file);
          fclose(fp);
          unlink(pid_file);
        }
        else
          fclose(fp);
      }
      else
        printf("Cannot not write to '%s' (PID file).\n", pid_file);
#ifdef CYGWIN_HACKS
      printf("Launched into the background (PID: %d)\n\n", xx);
#endif
    }
  }

  use_stderr = 0;               /* Stop writing to stderr now */
  if (backgrd) {
    /* Ok, try to disassociate from controlling terminal (finger cross) */
#if defined(HAVE_SETPGID) && !defined(CYGWIN_HACKS)
    setpgid(0, 0);
#endif

    /* Tcl wants the stdin, stdout and stderr file handles kept open. */
    freopen("/dev/null", "r", stdin);
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);

#ifdef CYGWIN_HACKS
    FreeConsole();
#endif
  }

  /* Terminal emulating dcc chat */
  if (!backgrd && term_z) {
    int n = new_dcc(&DCC_CHAT, sizeof(struct chat_info));

    dcc[n].addr = iptolong(getmyip());
    dcc[n].sock = STDOUT;
    dcc[n].timeval = now;
    dcc[n].u.chat->con_flags = conmask;
    dcc[n].u.chat->strip_flags = STRIP_ALL;
    dcc[n].status = STAT_ECHO;
    strcpy(dcc[n].nick, "HQ");
    strcpy(dcc[n].host, "llama@console");
    /* HACK: Workaround not to pass literal "HQ" as a non-const arg */
    dcc[n].user = get_user_by_handle(userlist, dcc[n].nick);
    /* Make sure there's an innocuous HQ user if needed */
    if (!dcc[n].user) {
      userlist = adduser(userlist, dcc[n].nick, "none", "-", USER_PARTY);
      dcc[n].user = get_user_by_handle(userlist, dcc[n].nick);
    }
    setsock(STDOUT, 0); /* Entry in net table */
    dprintf(n, "\n### ENTERING DCC CHAT SIMULATION ###\n\n");
    dcc_chatter(n);
  }

  then = now;
  online_since = now;
  autolink_cycle(NULL); /* Hurry and connect to tandem bots. */
  add_help_reference("cmds1.help");
  add_help_reference("cmds2.help");
  add_help_reference("core.help");

  /* Create hooks. */
  add_hook(HOOK_SECONDLY, (Function) core_secondly);
  add_hook(HOOK_MINUTELY, (Function) core_minutely);
  add_hook(HOOK_HOURLY, (Function) core_hourly);
  add_hook(HOOK_REHASH, (Function) event_rehash);
  add_hook(HOOK_PRE_REHASH, (Function) event_prerehash);
  add_hook(HOOK_USERFILE, (Function) event_save);
  add_hook(HOOK_BACKUP, (Function) backupuserfile);
  add_hook(HOOK_DAILY, (Function) event_logfile);
  add_hook(HOOK_DAILY, (Function) traffic_reset);
  add_hook(HOOK_LOADED, (Function) event_loaded);

  call_hook(HOOK_LOADED);

  debug0("main: entering loop");
  while (1) {
    int socket_cleanup = 0;

#ifdef USE_TCL_EVENTS
    /* Process a single Tcl event. */
    Tcl_DoOneEvent(TCL_ALL_EVENTS | TCL_DONT_WAIT);
#endif

    now = time(NULL);
    random();

    /* Every second... */
    if (now != then) {
      call_hook(HOOK_SECONDLY);
      then = now;
    }

    /* Only do this every so often. */
    if (!socket_cleanup) {
      socket_cleanup = 5;

      /* Remove dead dcc entries. */
      dcc_remove_lost();

      /* Check for server or dcc activity. */
      dequeue_sockets();
    }
    else {
      socket_cleanup--;
    }

    /* Free unused structures. */
    garbage_collect();

    xx = sockgets(buf, &i);
    if (xx >= 0) { /* Non-error */
      int idx;

      for (idx = 0; idx < dcc_total; idx++) {
        if (dcc[idx].sock != xx)
          continue;

        if (dcc[idx].type && dcc[idx].type->activity) {
          traffic_update_in(dcc[idx].type, (strlen(buf) + 1)); /* Traffic stats. */
          dcc[idx].type->activity(idx, buf, i);
        }
        else {
          putlog(LOG_MISC, "*", "!!! untrapped dcc activity: type %s, sock %d",
                 dcc[idx].type->name, dcc[idx].sock);
        }

        break;
      }
    }
    else if (xx == -1) { /* EOF */
      int idx;

      if (i == STDOUT && !backgrd)
        fatal("END OF FILE ON TERMINAL", 0);

      for (idx = 0; idx < dcc_total; idx++) {
        if (dcc[idx].sock != i)
          continue;

        if (dcc[idx].type && dcc[idx].type->eof) {
          dcc[idx].type->eof(idx);
        }
        else {
          putlog(LOG_MISC, "*",
                 "*** ATTENTION: DEAD SOCKET (%d) OF TYPE %s UNTRAPPED",
                 i, dcc[idx].type ? dcc[idx].type->name : "*UNKNOWN*");
          killsock(i);
          lostdcc(idx);
        }

        idx = dcc_total + 1;
      }

      if (idx == dcc_total) {
        putlog(LOG_MISC, "*",
               "(@) EOF socket %d, not a dcc socket, not anything.", i);
        close(i);
        killsock(i);
      }
    }
    else if (xx == -2 && errno != EINTR) { /* select() error */
      putlog(LOG_MISC, "*", "* Socket error #%d; recovering.", errno);
      for (i = 0; i < dcc_total; i++) {
        if ((fcntl(dcc[i].sock, F_GETFD, 0) == -1) && (errno == EBADF)) {
          putlog(LOG_MISC, "*",
                 "DCC socket %d (type %d, name '%s') expired -- pfft",
                 dcc[i].sock, dcc[i].type, dcc[i].nick);
          killsock(dcc[i].sock);
          lostdcc(i);
          i--;
        }
      }
    }
    else if (xx == -3) {
      call_hook(HOOK_IDLE);
      socket_cleanup = 0;       /* If we've been idle, cleanup & flush */
    }

    if (do_restart) {
      if (do_restart == -2) {
        rehash();
      }
      else {
        int f = 1;
        module_entry *p;
        Function startfunc;
        char name[256];


        check_tcl_event("prerestart");

        /* Unload as many modules as possible */
        while (f) {
          f = 0;

          for (p = module_list; p != NULL; p = p->next) {
            dependancy *d = dependancy_list;
            int ok = 1;

            while (ok && d) {
              if (d->needed == p)
                ok = 0;

              d = d->next;
            }
            if (ok) {
              strcpy(name, p->name);

              if (module_unload(name, botnetnick) == NULL) {
                f = 1;
                break;
              }
            }
          }
        }

        /* Make sure we don't have any modules left hanging around other than
         * "eggdrop" and the two that are supposed to be.
         */
        for (f = 0, p = module_list; p; p = p->next) {
          if (strcmp(p->name, "eggdrop") && strcmp(p->name, "encryption") &&
              strcmp(p->name, "uptime")) {
            f++;
          }
        }
        if (f != 0) {
          putlog(LOG_MISC, "*", MOD_STAGNANT);
        }

        /* Flush log files to disk. */
        flushlogs();

        /* Clean up Tcl stuff. */
        kill_tcl();

        /* Initialize stuff again. */
        init_tcl(argc, argv);
        init_language(0);
        help_init();
        traffic_init();
        logfile_init(1);

        /* This resets our modules which we didn't unload (encryption and uptime). */
        for (p = module_list; p; p = p->next) {
          if (p->funcs) {
            startfunc = p->funcs[MODCALL_START];
            startfunc(NULL);
          }
        }

        rehash();
        restart_chons();
        call_hook(HOOK_LOADED);
      }

      do_restart = 0;
    }
  }
}
