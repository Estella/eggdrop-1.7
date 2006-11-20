/* tclhash.h
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
 * $Id: tclhash.h,v 1.5 2006/11/20 13:53:35 tothwolf Exp $
 */

#ifndef _EGG_TCLHASH_H
#define _EGG_TCLHASH_H

#include "types.h" /* Function */

#define TC_DELETED   0x0001     /* This command/trigger was deleted. */

typedef struct tcl_cmd_b {
  struct tcl_cmd_b *next;

  struct flag_record flags;
  char *func_name;              /* Proc name. */
  /* FIXME: 'hits' could overflow if a bind is triggered enough. */
  int hits;                     /* Number of times this proc was triggered. */
  u_8bit_t attributes;          /* Flags for this entry. TC_* */
} tcl_cmd_t;


#define TBM_DELETED  0x0001     /* This mask was deleted. */

typedef struct tcl_bind_mask_b {
  struct tcl_bind_mask_b *next;

  tcl_cmd_t *first;             /* List of commands registered for this bind. */
  char *mask;
  u_8bit_t flags;               /* Flags for this entry. TBM_* */
} tcl_bind_mask_t;


#define HT_STACKABLE 0x0001 /* Triggers in this bind list may be stacked. */
#define HT_DELETED   0x0002 /* This bind list was already deleted. Don't use it anymore. */

typedef struct tcl_bind_list_b {
  struct tcl_bind_list_b *next;

  tcl_bind_mask_t *first;       /* Pointer to registered binds for this list. */
  char name[5];                 /* Name of the bind. */
  u_8bit_t flags;               /* Flags for this element. HT_* */
  Function func;                /* Function used as the Tcl calling interface
                                 * for procs actually representing C functions. */
} tcl_bind_list_t, *p_tcl_bind_list;


#define CHECKVALIDITY(a)        do {                                    \
        if (!check_validity(argv[0], (a))) {                            \
                Tcl_AppendResult(irp, "bad builtin command call!",      \
                                 NULL);                                 \
                return TCL_ERROR;                                       \
        }                                                               \
} while (0)


#ifndef MAKING_MODS
#  define check_tcl_chat(a, b, c) check_tcl_chatactbcst(a ,b, c, H_chat)
#  define check_tcl_act(a, b, c) check_tcl_chatactbcst(a, b, c, H_act)
#  define check_tcl_bcst(a, b, c) check_tcl_chatactbcst(a, b, c, H_bcst)
#  define check_tcl_chon(a, b) check_tcl_chonof(a, b, H_chon)
#  define check_tcl_chof(a, b) check_tcl_chonof(a, b, H_chof)
#  define check_tcl_load(a) check_tcl_loadunld(a, H_load)
#  define check_tcl_unld(a) check_tcl_loadunld(a, H_unld)
inline void garbage_collect_tclhash(void);
void init_bind(void);
void kill_bind(void);
int expmem_tclhash(void);
tcl_bind_list_t *add_bind_table(const char *nme, int flg, Function func);
void del_bind_table(tcl_bind_list_t *tl_which);
tcl_bind_list_t *find_bind_table(const char *nme);
int check_tcl_bind(tcl_bind_list_t *, const char *, struct flag_record *,
                   const char *, int);
int check_tcl_dcc(const char *, int, const char *);
void check_tcl_chjn(const char *, const char *, int, char, int, const char *);
void check_tcl_chpt(const char *, const char *, int, int);
void check_tcl_bot(const char *, const char *, const char *);
void check_tcl_link(const char *, const char *);
void check_tcl_disc(const char *);
const char *check_tcl_filt(int, const char *);
int check_tcl_note(const char *, const char *, const char *);
void check_tcl_listen(const char *, int);
void check_tcl_time(struct tm *);
void tell_binds(int, char *);
void check_tcl_nkch(const char *, const char *);
void check_tcl_away(const char *, int, const char *);
void check_tcl_chatactbcst(const char *, int, const char *, tcl_bind_list_t *);
void check_tcl_event(const char *);
void check_tcl_chonof(char *, int, tcl_bind_list_t *);
void check_tcl_loadunld(const char *, tcl_bind_list_t *);
void rem_builtins(tcl_bind_list_t *, cmd_t *);
void add_builtins(tcl_bind_list_t *, cmd_t *);
int check_validity(char *, Function);
extern p_tcl_bind_list H_chat, H_act, H_bcst, H_chon, H_chof;
extern p_tcl_bind_list H_load, H_unld, H_dcc, H_bot, H_link;
extern p_tcl_bind_list H_away, H_nkch, H_filt, H_disc, H_event;
#endif

#endif /* _EGG_TCLHASH_H */
