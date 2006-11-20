/* modules.c
 *
 * Originally by Darrin Smith (beldin@light.iinet.net.au)
 *
 * Copyright (C) 1997 Robey Pointer
 * Copyright (C) 1999 - 2006 Eggheads Development Team
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
 * $Id: modules.c,v 1.20 2006/11/20 13:53:34 tothwolf Exp $
 */

#include <ctype.h>
#include "main.h"
#include "modules.h"
#include "md5/md5.h"
#include "users.h"

#include "botcmd.h"
#include "botmsg.h"
#include "botnet.h"
#include "chanprog.h"
#include "cmds.h"
#include "dcc.h"
#include "dccutil.h"
#include "dns.h"
#include "help.h"
#include "language.h"
#include "match.h"
#include "mem.h"
#include "misc.h"
#include "logfile.h"
#include "net.h"
#include "rfc1459.h"
#include "userfile.h"
#include "userent.h"
#include "userrec.h"


#ifndef STATIC
#  ifdef MOD_USE_SHL
#    include <dl.h>
#  endif
#  ifdef MOD_USE_DYLD
#    include <mach-o/dyld.h>
#    define DYLDFLAGS NSLINKMODULE_OPTION_BINDNOW|NSLINKMODULE_OPTION_PRIVATE|NSLINKMODULE_OPTION_RETURN_ON_ERROR
#  endif
#  ifdef MOD_USE_RLD
#    ifdef HAVE_MACH_O_RLD_H
#      include <mach-o/rld.h>
#    else
#      ifdef HAVE_RLD_H
#        indluce <rld.h>
#      endif
#    endif
#  endif
#  ifdef MOD_USE_LOADER
#    include <loader.h>
#  endif

#  ifdef MOD_USE_DL
#    ifdef DLOPEN_1
char *dlerror();
void *dlopen(const char *, int);
int dlclose(void *);
void *dlsym(void *, char *);
#      define DLFLAGS 1
#    else /* DLOPEN_1 */
#      include <dlfcn.h>

#      ifndef RTLD_GLOBAL
#        define RTLD_GLOBAL 0
#      endif
#      ifndef RTLD_NOW
#        define RTLD_NOW 1
#      endif
#      ifdef RTLD_LAZY
#        define DLFLAGS RTLD_LAZY|RTLD_GLOBAL
#      else
#        define DLFLAGS RTLD_NOW|RTLD_GLOBAL
#      endif
#    endif /* DLOPEN_1 */
#  endif /* MOD_USE_DL */
#endif /* !STATIC */



extern struct dcc_t *dcc;
extern struct userrec *userlist, *lastuser;
extern struct chanset_t *chanset;

extern char tempdir[], botnetnick[], botname[], natip[], hostname[],
            origbotname[], botuser[], admin[], userfile[], ver[], notify_new[],
            helpdir[], version[], quit_msg[];

extern int parties, noshare, dcc_total, egg_numver, userfile_perm, do_restart,
           ignore_time, must_be_owner, raw_log, max_dcc, make_userfile,
           default_flags, share_greet, use_invites, use_exempts, force_expire,
           password_timeout, protect_readonly, reserved_port_min, copy_to_tmp,
           reserved_port_max, quiet_reject;

extern party_t *party;
extern time_t now, online_since;
extern tand_t *tandbot;
extern Tcl_Interp *interp;
extern sock_list *socklist;

int cmd_die();
int xtra_kill();
int xtra_unpack();
static int module_rename(char *name, char *newname);

#ifndef STATIC
char moddir[121] = "modules/";
#endif

#ifdef STATIC
struct static_list {
  struct static_list *next;
  char *name;
  char *(*func) ();
} *static_modules = NULL;

void check_static(char *name, char *(*func) ())
{
  struct static_list *p = nmalloc(sizeof(struct static_list));

  p->name = nmalloc(strlen(name) + 1);
  strcpy(p->name, name);
  p->func = func;
  p->next = static_modules;
  static_modules = p;
}
#endif /* STATIC */


/* The null functions */
void null_func()
{
}

char *charp_func()
{
  return NULL;
}

int minus_func()
{
  return -1;
}

int false_func()
{
  return 0;
}


/* The REAL hooks. When these are called, a return of 0 indicates unhandled;
 * 1 indicates handled. */
struct hook_entry *hook_list[REAL_HOOKS];

static void null_share(int idx, char *x)
{
  if ((x[0] == 'u') && (x[1] == 'n')) {
    putlog(LOG_BOTS, "*", "User file rejected by %s: %s", dcc[idx].nick, x + 3);
    dcc[idx].status &= ~BSTAT_OFFERED;
    if (!(dcc[idx].status & BSTAT_GETTING)) {
      dcc[idx].status &= ~BSTAT_SHARE;
    }
  } else if ((x[0] != 'v') && (x[0] != 'e')) {
    dprintf(idx, "s un Not sharing userfile.\n");
  }
}

void (*encrypt_pass) (char *, char *) = 0;
char *(*encrypt_string) (char *, char *) = 0;
char *(*decrypt_string) (char *, char *) = 0;
void (*shareout) () = null_func;
void (*sharein) (int, char *) = null_share;
void (*qserver) (int, char *, int) = (void (*)(int, char *, int)) null_func;
void (*add_mode) () = null_func;
int (*match_noterej) (struct userrec *, char *) =
    (int (*)(struct userrec *, char *)) false_func;
int (*rfc_casecmp) (const char *, const char *) = _rfc_casecmp;
int (*rfc_ncasecmp) (const char *, const char *, int) = _rfc_ncasecmp;
int (*rfc_toupper) (int) = _rfc_toupper;
int (*rfc_tolower) (int) = _rfc_tolower;
void (*dns_hostbyip) (IP) = block_dns_hostbyip;
void (*dns_ipbyhost) (char *) = block_dns_ipbyhost;

module_entry *module_list;
dependancy *dependancy_list = NULL;

/* The horrible global lookup table for functions
 * BUT it makes the whole thing *much* more portable than letting each
 * OS screw up the symbols their own special way :/
 */
Function global_table[] = {
  /* 0 - 9 */
  (Function) mod_malloc,
  (Function) mod_free,
  (Function) module_rename,
  (Function) module_register,
  (Function) module_find,
  (Function) module_depend,
  (Function) module_undepend,
  (Function) add_bind_table,
  (Function) del_bind_table,
  (Function) find_bind_table,
  /* 10 - 19 */
  (Function) check_tcl_bind,
  (Function) add_builtins,
  (Function) rem_builtins,
  (Function) add_tcl_commands,
  (Function) rem_tcl_commands,
  (Function) add_tcl_ints,
  (Function) rem_tcl_ints,
  (Function) add_tcl_strings,
  (Function) rem_tcl_strings,
  (Function) base64_to_int,
  /* 20 - 29 */
  (Function) int_to_base64,
  (Function) int_to_base10,
  (Function) simple_sprintf,
  (Function) botnet_send_zapf,
  (Function) botnet_send_zapf_broad,
  (Function) botnet_send_unlinked,
  (Function) botnet_send_bye,
  (Function) botnet_send_chat,
  (Function) botnet_send_filereject,
  (Function) botnet_send_filesend,
  /* 30 - 39 */
  (Function) botnet_send_filereq,
  (Function) botnet_send_join_idx,
  (Function) botnet_send_part_idx,
  (Function) updatebot,
  (Function) nextbot,
  (Function) zapfbot,
  (Function) n_free,
  (Function) u_pass_match,
  (Function) _user_malloc,
  (Function) get_user,
  /* 40 - 49 */
  (Function) set_user,
  (Function) add_entry_type,
  (Function) del_entry_type,
  (Function) get_user_flagrec,
  (Function) set_user_flagrec,
  (Function) get_user_by_host,
  (Function) get_user_by_handle,
  (Function) find_entry_type,
  (Function) find_user_entry,
  (Function) adduser,
  /* 50 - 59 */
  (Function) deluser,
  (Function) addhost_by_handle,
  (Function) delhost_by_handle,
  (Function) readuserfile,
  (Function) writeuserfile,
  (Function) geticon,
  (Function) clear_chanlist,
  (Function) reaffirm_owners,
  (Function) change_handle,
  (Function) write_user,
  /* 60 - 69 */
  (Function) clear_userlist,
  (Function) count_users,
  (Function) sanity_check,
  (Function) break_down_flags,
  (Function) build_flags,
  (Function) flagrec_eq,
  (Function) flagrec_ok,
  (Function) & shareout,
  (Function) dprintf,
  (Function) chatout,
  /* 70 - 79 */
  (Function) chanout_but,
  (Function) check_validity,
  (Function) list_delete,
  (Function) list_append,
  (Function) list_contains,
  (Function) answer,
  (Function) getmyip,
  (Function) neterror,
  (Function) tputs,
  (Function) new_dcc,
  /* 80 - 89 */
  (Function) lostdcc,
  (Function) getsock,
  (Function) killsock,
  (Function) open_listen,
  (Function) open_telnet_dcc,
  (Function) _get_data_ptr,
  (Function) open_telnet,
  (Function) check_tcl_event,
  (Function) egg_memcpy,
  (Function) my_atoul,
  /* 90 - 99 */
  (Function) my_strcpy,
  (Function) & dcc,
  (Function) & chanset,
  (Function) & userlist,
  (Function) & lastuser,
  (Function) & global_bans,
  (Function) & global_ign,
  (Function) & password_timeout,
  (Function) & share_greet,
  (Function) & max_dcc,
  /* 100 - 109 */
  (Function) & ignore_time,
  (Function) & reserved_port_min,
  (Function) & reserved_port_max,
  (Function) & raw_log,
  (Function) & noshare,
  (Function) & make_userfile,
  (Function) & default_flags,
  (Function) & dcc_total,
  (Function) tempdir,
  (Function) natip,
  /* 110 - 119 */
  (Function) hostname,
  (Function) origbotname,
  (Function) botuser,
  (Function) admin,
  (Function) userfile,
  (Function) ver,
  (Function) notify_new,
  (Function) helpdir,
  (Function) version,
  (Function) botnetnick,
  /* 120 - 129 */
  (Function) & DCC_CHAT_PASS,
  (Function) & DCC_BOT,
  (Function) & DCC_LOST,
  (Function) & DCC_CHAT,
  (Function) & interp,
  (Function) & now,
  (Function) findanyidx,
  (Function) findchan,
  (Function) cmd_die,
  (Function) days,
  /* 130 - 139 */
  (Function) ismember,
  (Function) newsplit,
  (Function) splitnick,
  (Function) splitc,
  (Function) addignore,
  (Function) match_ignore,
  (Function) delignore,
  (Function) fatal,
  (Function) xtra_kill,
  (Function) xtra_unpack,
  /* 140 - 149 */
  (Function) movefile,
  (Function) copyfile,
  (Function) do_tcl,
  (Function) readtclprog,
  (Function) get_language,
  (Function) def_get,
  (Function) makepass,
  (Function) _wild_match,
  (Function) maskhost,
  (Function) show_motd,
  /* 150 - 159 */
  (Function) tellhelp,
  (Function) showhelp,
  (Function) add_help_reference,
  (Function) rem_help_reference,
  (Function) touch_laston,
  (Function) & add_mode,
  (Function) rmspace,
  (Function) in_chain,
  (Function) add_note,
  (Function) del_lang_section,
  /* 160 - 169 */
  (Function) detect_dcc_flood,
  (Function) flush_lines,
  (Function) expected_memory,
  (Function) & do_restart,
  (Function) add_hook,
  (Function) del_hook,
  (Function) & H_event,
  (Function) & H_dcc,
  (Function) & H_filt,
  (Function) & H_chon,
  /* 170 - 179 */
  (Function) & H_chof,
  (Function) & H_load,
  (Function) & H_unld,
  (Function) & H_chat,
  (Function) & H_act,
  (Function) & H_bcst,
  (Function) & H_bot,
  (Function) & H_link,
  (Function) & H_disc,
  (Function) & H_away,
  /* 180 - 189 */
  (Function) & H_nkch,
  (Function) & USERENTRY_BOTADDR,
  (Function) & USERENTRY_BOTFL,
  (Function) & USERENTRY_HOSTS,
  (Function) & USERENTRY_PASS,
  (Function) & USERENTRY_XTRA,
  (Function) & USERENTRY_INFO,
  (Function) & USERENTRY_COMMENT,
  (Function) & USERENTRY_LASTON,
  (Function) user_del_chan,
  /* 190 - 199 */
  (Function) putlog,
  (Function) botnet_send_chan,
  (Function) list_type_kill,
  (Function) logmodes,
  (Function) masktype,
  (Function) stripmodes,
  (Function) stripmasktype,
  (Function) sub_lang,
  (Function) & online_since,
  (Function) cmd_loadlanguage,
  /* 200 - 209 */
  (Function) check_dcc_attrs,
  (Function) check_dcc_chanattrs,
  (Function) add_tcl_coups,
  (Function) rem_tcl_coups,
  (Function) botname,
  (Function) check_tcl_chjn,
  (Function) sanitycheck_dcc,
  (Function) isowner,
  (Function) & rfc_casecmp,
  (Function) & rfc_ncasecmp,
  /* 210 - 219 */
  (Function) & global_exempts,
  (Function) & global_invites,
  (Function) check_tcl_filt,
  (Function) & use_exempts,
  (Function) & use_invites,
  (Function) & force_expire,
  (Function) add_lang_section,
  (Function) _user_realloc,
  (Function) mod_realloc,
  (Function) xtra_set,
  /* 220 - 229 */
#ifdef DEBUG_ASSERT
  (Function) eggAssert,
#else
  (Function) 0,
#endif
  (Function) allocsock,
  (Function) call_hostbyip,
  (Function) call_ipbyhost,
  (Function) iptostr,
  (Function) & DCC_DNSWAIT,
  (Function) hostsanitycheck_dcc,
  (Function) dcc_dnsipbyhost,
  (Function) dcc_dnshostbyip,
  (Function) changeover_dcc,
  /* 230 - 239 */
  (Function) make_rand_str,
  (Function) & protect_readonly,
  (Function) findchan_by_dname,
  (Function) removedcc,
  (Function) & userfile_perm,
  (Function) sock_has_data,
  (Function) bots_in_subtree,
  (Function) users_in_subtree,
  (Function) egg_inet_aton,
  (Function) egg_snprintf,
  /* 240 - 249 */
  (Function) egg_vsnprintf,
  (Function) egg_memset,
  (Function) egg_strcasecmp,
  (Function) egg_strncasecmp,
  (Function) is_file,
  (Function) & must_be_owner,
  (Function) & tandbot,
  (Function) & party,
  (Function) open_address_listen,
  (Function) str_escape,
  /* 250 - 259 */
  (Function) strchr_unescape,
  (Function) str_unescape,
  (Function) clear_chanlist_member,
  (Function) fixfrom,
  (Function) & socklist,
  (Function) sockoptions,
  (Function) kill_bot,
  (Function) quit_msg,
  (Function) module_load,
  (Function) module_unload,
  /* 260 - 269 */
  (Function) & parties,
  (Function) tell_bottree,
  (Function) MD5_Init,
  (Function) MD5_Update,
  (Function) MD5_Final,
  (Function) _wild_match_per,
  (Function) killtransfer,
  (Function) write_ignores,
  (Function) & copy_to_tmp,
  (Function) & quiet_reject,
  /* 270 - */
  (Function) file_readable,
  (Function) strip_mirc_codes,
  (Function) check_ansi,
  (Function) oatoi,
  (Function) str_isdigit,
  (Function) remove_crlf
};

void init_modules()
{
  int i;

  module_list = nmalloc(sizeof(module_entry));
  module_list->name = nmalloc(8);
  strcpy(module_list->name, "eggdrop");
  module_list->major = (egg_numver) / 10000;
  module_list->minor = (egg_numver / 100) % 100;
#ifndef STATIC
  module_list->hand = NULL;
#endif
  module_list->next = NULL;
  module_list->funcs = NULL;

  for (i = 0; i < REAL_HOOKS; i++) {
    hook_list[i] = NULL;
  }
}

int expmem_modules(int y)
{
  int c = 0, i;
  module_entry *p;
  dependancy *d;
  struct hook_entry *q;
  Function *f;
#ifdef STATIC
  struct static_list *s;

  for (s = static_modules; s; s = s->next) {
    c += sizeof(struct static_list) + strlen(s->name) + 1;
  }
#endif

  for (i = 0; i < REAL_HOOKS; i++) {
    for (q = hook_list[i]; q; q = q->next) {
      c += sizeof(struct hook_entry);
    }
  }

  for (d = dependancy_list; d; d = d->next) {
    c += sizeof(dependancy);
  }

  for (p = module_list; p; p = p->next) {
    c += sizeof(module_entry);
    c += strlen(p->name) + 1;
    f = p->funcs;
    if (f && f[MODCALL_EXPMEM] && !y)
      c += (int) (f[MODCALL_EXPMEM] ());
  }

  return c;
}

int module_register(char *name, Function *funcs, int major, int minor)
{
  module_entry *p;

  for (p = module_list; p && p->name; p = p->next) {
    if (!egg_strcasecmp(name, p->name)) {
      p->major = major;
      p->minor = minor;
      p->funcs = funcs;
      return 1;
    }
  }

  return 0;
}

const char *module_load(char *name)
{
  module_entry *p;
  char *e;
  Function f;
#ifdef STATIC
  struct static_list *sl;
#endif

#ifndef STATIC
  char workbuf[1024];
#  ifdef MOD_USE_SHL
  shl_t hand;
#  endif
#  ifdef MOD_USE_DYLD
  NSObjectFileImage file;
  NSObjectFileImageReturnCode ret;
  NSModule hand;
  NSSymbol sym;
#  endif
#  ifdef MOD_USE_RLD
  long ret;
#  endif
#  ifdef MOD_USE_LOADER
  ldr_module_t hand;
#  endif
#  ifdef MOD_USE_DL
  void *hand;
#  endif
#endif /* !STATIC */

  if (module_find(name, 0, 0) != NULL)
    return MOD_ALREADYLOAD;

#ifndef STATIC
  if (moddir[0] != '/') {
    if (getcwd(workbuf, 1024) == NULL)
      return MOD_BADCWD;
    sprintf(&(workbuf[strlen(workbuf)]), "/%s%s." EGG_MOD_EXT, moddir, name);
  } else {
    sprintf(workbuf, "%s%s." EGG_MOD_EXT, moddir, name);
  }

#  ifdef MOD_USE_SHL
  hand = shl_load(workbuf, BIND_IMMEDIATE, 0L);
  if (!hand)
    return "Can't load module.";
  sprintf(workbuf, "%s_start", name);
  if (shl_findsym(&hand, workbuf, (short) TYPE_PROCEDURE, (void *) &f))
    f = NULL;
  if (f == NULL) {
    /* Some OS's require a _ to be prepended to the symbol name (Darwin, etc). */
    sprintf(workbuf, "_%s_start", name);
    if (shl_findsym(&hand, workbuf, (short) TYPE_PROCEDURE, (void *) &f))
      f = NULL;
  }
  if (f == NULL) {
    shl_unload(hand);
    return MOD_NOSTARTDEF;
  }
#  endif /* MOD_USE_SHL */

#  ifdef MOD_USE_DYLD
  ret = NSCreateObjectFileImageFromFile(workbuf, &file);
  if (ret != NSObjectFileImageSuccess)
    return "Can't load module.";
  hand = NSLinkModule(file, workbuf, DYLDFLAGS);
  sprintf(workbuf, "_%s_start", name);
  sym = NSLookupSymbolInModule(hand, workbuf);
  if (sym)
    f = (Function) NSAddressOfSymbol(sym);
  else
    f = NULL;
  if (f == NULL) {
    NSUnLinkModule(hand, NSUNLINKMODULE_OPTION_NONE);
    return MOD_NOSTARTDEF;
  }
#  endif /* MOD_USE_DYLD */

#  ifdef MOD_USE_RLD
  ret = rld_load(NULL, (struct mach_header **) 0, workbuf, (const char *) 0);
  if (!ret)
    return "Can't load module.";
  sprintf(workbuf, "_%s_start", name);
  ret = rld_lookup(NULL, workbuf, &f)
  if (!ret || f == NULL)
    return MOD_NOSTARTDEF;
  /* There isn't a reliable way to unload at this point... just keep it loaded. */
#  endif /* MOD_USE_DYLD */

#  ifdef MOD_USE_LOADER
  hand = load(workbuf, LDR_NOFLAGS);
  if (hand == LDR_NULL_MODULE)
    return "Can't load module.";
  sprintf(workbuf, "%s_start", name);
  f = (Function) ldr_lookup_package(hand, workbuf);
  if (f == NULL) {
    sprintf(workbuf, "_%s_start", name);
    f = (Function) ldr_lookup_package(hand, workbuf);
  }
  if (f == NULL) {
    unload(hand);
    return MOD_NOSTARTDEF;
  }
#  endif /* MOD_USE_LOADER */

#  ifdef MOD_USE_DL
  hand = dlopen(workbuf, DLFLAGS);
  if (!hand)
    return dlerror();
  sprintf(workbuf, "%s_start", name);
  f = (Function) dlsym(hand, workbuf);
  if (f == NULL) {
    sprintf(workbuf, "_%s_start", name);
    f = (Function) dlsym(hand, workbuf);
  }
  if (f == NULL) {
    dlclose(hand);
    return MOD_NOSTARTDEF;
  }
#  endif /* MOD_USE_DL */
#endif /* !STATIC */

#ifdef STATIC
  for (sl = static_modules; sl && egg_strcasecmp(sl->name, name); sl = sl->next);
  if (!sl)
    return "Unknown module.";
  f = (Function) sl->func;
#endif /* STATIC */

  p = nmalloc(sizeof(module_entry));
  if (p == NULL)
    return "Malloc error";
  p->name = nmalloc(strlen(name) + 1);
  strcpy(p->name, name);
  p->major = 0;
  p->minor = 0;
#ifndef STATIC
  p->hand = hand;
#endif
  p->funcs = 0;
  p->next = module_list;
  module_list = p;
  e = (((char *(*)()) f) (global_table));
  if (e) {
    module_list = module_list->next;
    nfree(p->name);
    nfree(p);
    return e;
  }
  check_tcl_load(name);

  if (exist_lang_section(name))
    putlog(LOG_MISC, "*", MOD_LOADED_WITH_LANG, name);
  else
    putlog(LOG_MISC, "*", MOD_LOADED, name);

  return NULL;
}

char *module_unload(char *name, char *user)
{
  module_entry *p = module_list, *o = NULL;
  char *e;
  Function *f;

  while (p) {
    if ((p->name != NULL) && !strcmp(name, p->name)) {
      dependancy *d;

      for (d = dependancy_list; d; d = d->next)
        if (d->needed == p)
          return MOD_NEEDED;

      f = p->funcs;
      if (f && !f[MODCALL_CLOSE])
        return MOD_NOCLOSEDEF;
      if (f) {
        check_tcl_unld(name);
        e = (((char *(*)()) f[MODCALL_CLOSE]) (user));
        if (e != NULL)
          return e;
#ifndef STATIC
#  ifdef MOD_USE_SHL
        shl_unload(p->hand);
#  endif
#  ifdef MOD_USE_DYLD
        NSUnLinkModule(p->hand, NSUNLINKMODULE_OPTION_NONE);
#  endif
#  ifdef MOD_USE_LOADER
        unload(p->hand);
#  endif
#  ifdef MOD_USE_DL
        dlclose(p->hand);
#  endif
#endif /* !STATIC */
      }
      nfree(p->name);
      if (o == NULL)
        module_list = p->next;
      else
        o->next = p->next;
      nfree(p);
      putlog(LOG_MISC, "*", "%s %s", MOD_UNLOADED, name);
      return NULL;
    }
    o = p;
    p = p->next;
  }

  return MOD_NOSUCH;
}

module_entry *module_find(char *name, int major, int minor)
{
  module_entry *p;

  for (p = module_list; p && p->name; p = p->next) {
    if ((major == p->major || !major) && minor <= p->minor &&
        !egg_strcasecmp(name, p->name))
      return p;
  }
  return NULL;
}

static int module_rename(char *name, char *newname)
{
  module_entry *p;

  for (p = module_list; p; p = p->next)
    if (!egg_strcasecmp(newname, p->name))
      return 0;

  for (p = module_list; p && p->name; p = p->next) {
    if (!egg_strcasecmp(name, p->name)) {
      nfree(p->name);
      p->name = nmalloc(strlen(newname) + 1);
      strcpy(p->name, newname);
      return 1;
    }
  }
  return 0;
}

Function *module_depend(char *name1, char *name2, int major, int minor)
{
  module_entry *p = module_find(name2, major, minor);
  module_entry *o = module_find(name1, 0, 0);
  dependancy *d;

  if (!p) {
    if (module_load(name2))
      return 0;
    p = module_find(name2, major, minor);
  }
  if (!p || !o)
    return 0;
  d = nmalloc(sizeof(dependancy));

  d->needed = p;
  d->needing = o;
  d->next = dependancy_list;
  d->major = major;
  d->minor = minor;
  dependancy_list = d;
  return p->funcs ? p->funcs : (Function *) 1;
}

int module_undepend(char *name1)
{
  int ok = 0;
  module_entry *p = module_find(name1, 0, 0);
  dependancy *d = dependancy_list, *o = NULL;

  if (p == NULL)
    return 0;
  while (d != NULL) {
    if (d->needing == p) {
      if (o == NULL) {
        dependancy_list = d->next;
      } else {
        o->next = d->next;
      }
      nfree(d);
      if (o == NULL)
        d = dependancy_list;
      else
        d = o->next;
      ok++;
    } else {
      o = d;
      d = d->next;
    }
  }
  return ok;
}

void *mod_malloc(int size, const char *modname, const char *filename, int line)
{
#ifdef DEBUG_MEM
  char x[100], *p;

  p = strrchr(filename, '/');
  egg_snprintf(x, sizeof x, "%s:%s", modname, p ? p + 1 : filename);
  x[19] = 0;
  return n_malloc(size, x, line);
#else
  return nmalloc(size);
#endif
}

void *mod_realloc(void *ptr, int size, const char *modname,
                  const char *filename, int line)
{
#ifdef DEBUG_MEM
  char x[100], *p;

  p = strrchr(filename, '/');
  egg_snprintf(x, sizeof x, "%s:%s", modname, p ? p + 1 : filename);
  x[19] = 0;
  return n_realloc(ptr, size, x, line);
#else
  return nrealloc(ptr, size);
#endif
}

void mod_free(void *ptr, const char *modname, const char *filename, int line)
{
  char x[100], *p;

  p = strrchr(filename, '/');
  egg_snprintf(x, sizeof x, "%s:%s", modname, p ? p + 1 : filename);
  x[19] = 0;
  n_free(ptr, x, line);
}

/* Hooks, various tables of functions to call on ceratin events
 */
void add_hook(int hook_num, Function func)
{
  if (hook_num < REAL_HOOKS) {
    struct hook_entry *p;

    for (p = hook_list[hook_num]; p; p = p->next)
      if (p->func == func)
        return;                 /* Don't add it if it's already there */
    p = nmalloc(sizeof(struct hook_entry));

    p->next = hook_list[hook_num];
    hook_list[hook_num] = p;
    p->func = func;
  } else
    switch (hook_num) {
    case HOOK_ENCRYPT_PASS:
      encrypt_pass = (void (*)(char *, char *)) func;
      break;
    case HOOK_ENCRYPT_STRING:
      encrypt_string = (char *(*)(char *, char *)) func;
      break;
    case HOOK_DECRYPT_STRING:
      decrypt_string = (char *(*)(char *, char *)) func;
      break;
    case HOOK_SHAREOUT:
      shareout = (void (*)()) func;
      break;
    case HOOK_SHAREIN:
      sharein = (void (*)(int, char *)) func;
      break;
    case HOOK_QSERV:
      if (qserver == (void (*)(int, char *, int)) null_func)
        qserver = (void (*)(int, char *, int)) func;
      break;
    case HOOK_ADD_MODE:
      if (add_mode == (void (*)()) null_func)
        add_mode = (void (*)()) func;
      break;
      /* special hook <drummer> */
    case HOOK_RFC_CASECMP:
      if (func == NULL) {
        rfc_casecmp = egg_strcasecmp;
        rfc_ncasecmp =
          (int (*)(const char *, const char *, int)) egg_strncasecmp;
        rfc_tolower = tolower;
        rfc_toupper = toupper;
      } else {
        rfc_casecmp = _rfc_casecmp;
        rfc_ncasecmp = _rfc_ncasecmp;
        rfc_tolower = _rfc_tolower;
        rfc_toupper = _rfc_toupper;
      }
      break;
    case HOOK_MATCH_NOTEREJ:
      if (match_noterej == (int (*)(struct userrec *, char *)) false_func)
        match_noterej = func;
      break;
    case HOOK_DNS_HOSTBYIP:
      if (dns_hostbyip == block_dns_hostbyip)
        dns_hostbyip = (void (*)(IP)) func;
      break;
    case HOOK_DNS_IPBYHOST:
      if (dns_ipbyhost == block_dns_ipbyhost)
        dns_ipbyhost = (void (*)(char *)) func;
      break;
    }
}

void del_hook(int hook_num, Function func)
{
  if (hook_num < REAL_HOOKS) {
    struct hook_entry *p = hook_list[hook_num], *o = NULL;

    while (p) {
      if (p->func == func) {
        if (o == NULL)
          hook_list[hook_num] = p->next;
        else
          o->next = p->next;
        nfree(p);
        break;
      }
      o = p;
      p = p->next;
    }
  } else
    switch (hook_num) {
    case HOOK_ENCRYPT_PASS:
      if (encrypt_pass == (void (*)(char *, char *)) func)
        encrypt_pass = (void (*)(char *, char *)) null_func;
      break;
    case HOOK_ENCRYPT_STRING:
      if (encrypt_string == (char *(*)(char *, char *)) func)
        encrypt_string = (char *(*)(char *, char *)) null_func;
      break;
    case HOOK_DECRYPT_STRING:
      if (decrypt_string == (char *(*)(char *, char *)) func)
        decrypt_string = (char *(*)(char *, char *)) null_func;
      break;
    case HOOK_SHAREOUT:
      if (shareout == (void (*)()) func)
        shareout = null_func;
      break;
    case HOOK_SHAREIN:
      if (sharein == (void (*)(int, char *)) func)
        sharein = null_share;
      break;
    case HOOK_QSERV:
      if (qserver == (void (*)(int, char *, int)) func)
        qserver = null_func;
      break;
    case HOOK_ADD_MODE:
      if (add_mode == (void (*)()) func)
        add_mode = null_func;
      break;
    case HOOK_MATCH_NOTEREJ:
      if (match_noterej == (int (*)(struct userrec *, char *)) func)
        match_noterej = false_func;
      break;
    case HOOK_DNS_HOSTBYIP:
      if (dns_hostbyip == (void (*)(IP)) func)
        dns_hostbyip = block_dns_hostbyip;
      break;
    case HOOK_DNS_IPBYHOST:
      if (dns_ipbyhost == (void (*)(char *)) func)
        dns_ipbyhost = block_dns_ipbyhost;
      break;
    }
}

int call_hook_cccc(int hooknum, char *a, char *b, char *c, char *d)
{
  struct hook_entry *p, *pn;
  int f = 0;

  if (hooknum >= REAL_HOOKS)
    return 0;
  p = hook_list[hooknum];
  for (p = hook_list[hooknum]; p && !f; p = pn) {
    pn = p->next;
    f = p->func(a, b, c, d);
  }
  return f;
}

void do_module_report(int idx, int details, char *which)
{
  module_entry *p = module_list;

  if (p && !which)
    dprintf(idx, "Loaded module information:\n");
  for (; p; p = p->next) {
    if (!which || !egg_strcasecmp(which, p->name)) {
      dependancy *d;

      if (details)
        dprintf(idx, "  Module: %s, v %d.%d\n", p->name ? p->name : "CORE",
                p->major, p->minor);
      if (details > 1) {
        for (d = dependancy_list; d; d = d->next)
          if (d->needing == p)
            dprintf(idx, "    requires: %s, v %d.%d\n", d->needed->name,
                    d->major, d->minor);
      }
      if (p->funcs) {
        Function f = p->funcs[MODCALL_REPORT];

        if (f != NULL)
          f(idx, details);
      }
      if (which)
        return;
    }
  }
  if (which)
    dprintf(idx, "No such module.\n");
}
