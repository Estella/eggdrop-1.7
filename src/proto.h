/*
 * proto.h
 *   prototypes for every function used outside its own module
 *
 * (i guess i'm not very modular, cuz there are a LOT of these.)
 * with full prototyping, some have been moved to other .h files
 * because they use structures in those
 * (saves including those .h files EVERY time) - Beldin
 *
 * $Id: proto.h,v 1.9 2004/08/31 01:48:21 wcc Exp $
 */
/*
 * Copyright (C) 1997 Robey Pointer
 * Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004 Eggheads Development Team
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

#ifndef _EGG_PROTO_H
#define _EGG_PROTO_H

#include "lush.h"
#include "misc_file.h"

struct chanset_t;               /* keeps the compiler warnings down :) */
struct userrec;
struct maskrec;
struct igrec;
struct flag_record;
struct list_type;

#ifndef MAKING_MODS
extern void (*encrypt_pass) (char *, char *);
extern char *(*encrypt_string) (char *, char *);
extern char *(*decrypt_string) (char *, char *);
extern int (*match_noterej) (struct userrec *, char *);
#endif

/* chanprog.c */
void tell_verbose_uptime(int);
void tell_verbose_status(int);
void tell_settings(int);
int isowner(char *);
void reaffirm_owners();
void rehash();
void reload();
void chanprog();
void check_timers();
void check_utimers();
void rmspace(char *s);
void check_timers();
void set_chanlist(const char *host, struct userrec *rec);
void clear_chanlist(void);
void clear_chanlist_member(const char *nick);

/* mem.c */
void *n_malloc(int, const char *, int);
void *n_realloc(void *, int, const char *, int);
void n_free(void *, const char *, int);
void tell_mem_status(char *);
void tell_mem_status_dcc(int);
void debug_mem_to_dcc(int);

/* misc.c */
int egg_strcatn(char *, const char *, size_t);
int my_strcpy(char *, char *);
char *stristr(char *, char *);
void splitc(char *, char *, char);
void splitcn(char *, char *, char, size_t);
void remove_crlf(char **);
char *newsplit(char **);
char *splitnick(char **);
void stridx(char *, char *, int);
void dumplots(int, const char *, char *);
void daysago(time_t, time_t, char *);
void days(time_t, time_t, char *);
void daysdur(time_t, time_t, char *);
void help_subst(char *, char *, struct flag_record *, int, char *);
void sub_lang(int, char *);
void show_motd(int);
void tellhelp(int, char *, struct flag_record *, int);
void tellwildhelp(int, char *, struct flag_record *);
void tellallhelp(int, char *, struct flag_record *);
void showhelp(char *, char *, struct flag_record *, int);
void rem_help_reference(char *);
void add_help_reference(char *);
void debug_help(int);
void reload_help_data(void);
char *extracthostname(char *);
void show_banner(int i);
void make_rand_str(char *, int);
int oatoi(const char *);
int is_file(const char *);
char *str_escape(const char *, const char, const char);
char *strchr_unescape(char *, const char, register const char);
void str_unescape(char *, register const char);
int str_isdigit(const char *);
void kill_bot(char *, char *);

void _maskhost(const char *, char *, int);
#define maskhost(a,b) _maskhost((a),(b),1)
#define maskban(a,b)  _maskhost((a),(b),0)

/* tcl.c */
void protect_tcl();
void unprotect_tcl();
void do_tcl(char *, char *);
int readtclprog(char *fname);
int findidx(int);
int findanyidx(int);

/* users.c */
void addignore(char *, char *, char *, time_t);
int delignore(char *);
void tell_ignores(int, char *);
int match_ignore(char *);
void check_expired_ignores();
void autolink_cycle(char *);
void tell_file_stats(int, char *);
void tell_user_ident(int, char *, int);
void tell_users_match(int, char *, int, int, int, char *);
int readuserfile(char *, struct userrec **);

#endif /* _EGG_PROTO_H */
