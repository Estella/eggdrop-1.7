/* module.h
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
 * $Id: module.h,v 1.13 2005/08/25 00:25:29 wcc Exp $
 */

#ifndef _EGG_MOD_MODULE_H
#define _EGG_MOD_MODULE_H

#include "src/main.h"
#include "modvals.h"

#include "src/types.h"
#include "src/botmsg.h"
#include "src/dcc.h"
#include "src/dccutil.h"
#include "src/dns.h"
#include "src/logfile.h"
#include "src/misc.h"
#include "src/net.h"

/*
 * This file contains all the horrible stuff required to do the lookup
 * table for symbols, rather than getting the OS to do it, since most
 * OS's require all symbols resolved, this can cause a problem with
 * some modules.
 *
 * This is intimately related to the table in `modules.c'. Don't change
 * the files unless you have flamable underwear.
 *
 * Do not read this file whilst unless heavily sedated, I will not be
 * held responsible for mental break-downs caused by this file <G>
 */

#undef nmalloc
#undef nfree
#undef nrealloc
#undef feof
#undef user_malloc
#undef dprintf
#undef get_data_ptr
#undef wild_match
#undef wild_match_per
#undef user_realloc
#undef Assert

/* Compability functions. */
#ifdef egg_inet_aton
#  undef egg_inet_aton
#endif
#ifdef egg_vsnprintf
#  undef egg_vsnprintf
#endif
#ifdef egg_snprintf
#  undef egg_snprintf
#endif
#ifdef egg_memset
#  undef egg_memset
#endif
#ifdef egg_strcasecmp
#  undef egg_strcasecmp
#endif
#ifdef egg_strncasecmp
#  undef egg_strncasecmp
#endif

#if defined (__CYGWIN__) && !defined(STATIC)
#  define EXPORT_SCOPE  __declspec(dllexport)
#else
#  define EXPORT_SCOPE
#endif

/* Redefine for module-relevance. */

/* 0 - 9 */
#define nmalloc(x) (((void *(*)())global[0])((x),MODULE_NAME,__FILE__,__LINE__))
#define nfree(x) (global[1]((x),MODULE_NAME,__FILE__,__LINE__))
#define module_rename ((int (*)(char *, char *))global[2])
#define module_register ((int (*)(char *, Function *, int, int))global[3])
#define module_find ((module_entry * (*)(char *,int,int))global[4])
#define module_depend ((Function *(*)(char *,char *,int,int))global[5])
#define module_undepend ((int(*)(char *))global[6])
#define add_bind_table ((p_tcl_bind_list(*)(const char *,int,Function))global[7])
#define del_bind_table ((void (*) (p_tcl_bind_list))global[8])
#define find_bind_table ((p_tcl_bind_list(*)(const char *))global[9])
/* 10 - 19 */
#define check_tcl_bind ((int (*) (p_tcl_bind_list,const char *,struct flag_record *,const char *, int))global[10])
#define add_builtins ((int (*) (p_tcl_bind_list, cmd_t *))global[11])
#define rem_builtins ((int (*) (p_tcl_bind_list, cmd_t *))global[12])
#define add_tcl_commands ((void (*) (tcl_cmds *))global[13])
#define rem_tcl_commands ((void (*) (tcl_cmds *))global[14])
#define add_tcl_ints ((void (*) (tcl_ints *))global[15])
#define rem_tcl_ints ((void (*) (tcl_ints *))global[16])
#define add_tcl_strings ((void (*) (tcl_strings *))global[17])
#define rem_tcl_strings ((void (*) (tcl_strings *))global[18])
#define base64_to_int ((int (*) (char *))global[19])
/* 20 - 29 */
#define int_to_base64 ((char * (*) (int))global[20])
#define int_to_base10 ((char * (*) (int))global[21])
#define simple_sprintf ((int (*)())global[22])
#define botnet_send_zapf ((void (*)(int, char *, char *, char *))global[23])
#define botnet_send_zapf_broad ((void (*)(int, char *, char *, char *))global[24])
#define botnet_send_unlinked ((void (*)(int, char *, char *))global[25])
#define botnet_send_bye ((void(*)(void))global[26])
#define botnet_send_chat ((void(*)(int,char*,char*))global[27])
#define botnet_send_filereject ((void(*)(int,char*,char*,char*))global[28])
#define botnet_send_filesend ((void(*)(int,char*,char*,char*))global[29])
/* 30 - 39 */
#define botnet_send_filereq ((void(*)(int,char*,char*,char*))global[30])
#define botnet_send_join_idx ((void(*)(int,int))global[31])
#define botnet_send_part_idx ((void(*)(int,char *))global[32])
#define updatebot ((void(*)(int,char*,char,int))global[33])
#define nextbot ((int (*)(char *))global[34])
#define zapfbot ((void (*)(int))global[35])
#define n_free ((void (*)(void *,char *, int))global[36])
#define u_pass_match ((int (*)(struct userrec *,char *))global[37])
#define user_malloc(x) ((void *(*)(int,char *,int))global[38])(x,__FILE__,__LINE__)
#define get_user ((void *(*)(struct user_entry_type *,struct userrec *))global[39])
/* 40 - 49 */
#define set_user ((int(*)(struct user_entry_type *,struct userrec *,void *))global[40])
#define add_entry_type ((int (*) ( struct user_entry_type * ))global[41])
#define del_entry_type ((int (*) ( struct user_entry_type * ))global[42])
#define get_user_flagrec ((void (*)(struct userrec *, struct flag_record *, const char *))global[43])
#define set_user_flagrec ((void (*)(struct userrec *, struct flag_record *, const char *))global[44])
#define get_user_by_host ((struct userrec * (*)(char *))global[45])
#define get_user_by_handle ((struct userrec *(*)(struct userrec *,char *))global[46])
#define find_entry_type ((struct user_entry_type * (*) ( char * ))global[47])
#define find_user_entry ((struct user_entry * (*)( struct user_entry_type *, struct userrec *))global[48])
#define adduser ((struct userrec *(*)(struct userrec *,char*,char*,char*,int))global[49])
/* 50 - 59 */
#define deluser ((int (*)(char *))global[50])
#define addhost_by_handle ((void (*) (char *, char *))global[51])
#define delhost_by_handle ((int(*)(char *,char *))global[52])
#define readuserfile ((int (*)(char *,struct userrec **))global[53])
#define writeuserfile ((void(*)(int))global[54])
#define geticon ((char (*) (int))global[55])
#define clear_chanlist ((void (*)(void))global[56])
#define reaffirm_owners ((void (*)(void))global[57])
#define change_handle ((int(*)(struct userrec *,char*))global[58])
#define write_user ((int (*)(struct userrec *, FILE *,int))global[59])
/* 60 - 69 */
#define clear_userlist ((void (*)(struct userrec *))global[60])
#define count_users ((int(*)(struct userrec *))global[61])
#define sanity_check ((int(*)(int))global[62])
#define break_down_flags ((void (*)(const char *,struct flag_record *,struct flag_record *))global[63])
#define build_flags ((void (*)(char *, struct flag_record *, struct flag_record *))global[64])
#define flagrec_eq ((int(*)(struct flag_record*,struct flag_record *))global[65])
#define flagrec_ok ((int(*)(struct flag_record*,struct flag_record *))global[66])
#define shareout (*(Function *)(global[67]))
#define dprintf (global[68])
#define chatout (global[69])
/* 70 - 79 */
#define chanout_but ((void(*)())global[70])
#define check_validity ((int (*) (char *,Function))global[71])
#define list_delete ((int (*)( struct list_type **, struct list_type *))global[72])
#define list_append ((int (*) ( struct list_type **, struct list_type *))global[73])
#define list_contains ((int (*) (struct list_type *, struct list_type *))global[74])
#define answer ((int (*) (int,char *,unsigned long *,unsigned short *,int))global[75])
#define getmyip ((IP (*) (void))global[76])
#define neterror ((void (*) (char *))global[77])
#define tputs ((void (*) (int, char *,unsigned int))global[78])
#define new_dcc ((int (*) (struct dcc_table *, int))global[79])
/* 80 - 89 */
#define lostdcc ((void (*) (int))global[80])
#define getsock ((int (*) (int))global[81])
#define killsock ((void (*) (int))global[82])
#define open_listen ((int (*) (int *))global[83])
#define open_telnet_dcc ((int (*) (int,char *,char *))global[84])
#define get_data_ptr(x) ((void *(*)(int,char*,int))global[85])(x,__FILE__,__LINE__)
#define open_telnet ((int (*) (char *, int))global[86])
#define check_tcl_event ((void * (*) (const char *))global[87])
#define my_memcpy ((void * (*) (void *, const void *, size_t))global[88])
#define my_atoul ((IP(*)(char *))global[89])
/* 90 - 99 */
#define my_strcpy ((int (*)(char *, const char *))global[90])
#define dcc (*(struct dcc_t **)global[91])
#define chanset (*(struct chanset_t **)(global[92]))
#define userlist (*(struct userrec **)global[93])
#define lastuser (*(struct userrec **)(global[94]))
#define global_bans (*(maskrec **)(global[95]))
#define global_ign (*(struct igrec **)(global[96]))
#define password_timeout (*(int *)(global[97]))
#define share_greet (*(int *)global[98])
#define max_dcc (*(int *)global[99])
/* 100 - 109 */
#define ignore_time (*(int *)(global[101]))
#define reserved_port_min (*(int *)(global[102]))
#define reserved_port_max (*(int *)(global[103]))
#define raw_log (*(int *)(global[104]))
#define noshare (*(int *)(global[105]))
#define make_userfile (*(int*)global[106])
#define default_flags (*(int*)global[107])
#define dcc_total (*(int*)global[108])
#define tempdir ((char *)(global[109]))
/* 110 - 119 */
#define natip ((char *)(global[110]))
#define hostname ((char *)(global[111]))
#define origbotname ((char *)(global[112]))
#define botuser ((char *)(global[113]))
#define admin ((char *)(global[114]))
#define userfile ((char *)global[115])
#define ver ((char *)global[116])
#define notify_new ((char *)global[117])
#define helpdir ((char *)global[118])
#define Version ((char *)global[119])
/* 120 - 129 */
#define botnetnick ((char *)global[120])
#define DCC_CHAT_PASS (*(struct dcc_table *)(global[121]))
#define DCC_BOT (*(struct dcc_table *)(global[122]))
#define DCC_LOST (*(struct dcc_table *)(global[123]))
#define DCC_CHAT (*(struct dcc_table *)(global[124]))
#define interp (*(Tcl_Interp **)(global[125]))
#define now (*(time_t*)global[126])
#define findanyidx ((int (*)(int))global[127])
#define findchan ((struct chanset_t *(*)(char *))global[128])
#define cmd_die (global[129])
/* 130 - 139 */
#define days ((void (*)(time_t,time_t,char *,int))global[130])
#define ismember ((memberlist * (*) (struct chanset_t *, char *))global[131])
#define newsplit ((char *(*)(char **))global[132])
#define splitnick ((char *(*)(char **))global[133])
#define splitc ((void (*)(char *,char *,char))global[134])
#define addignore ((void (*) (char *, char *, char *,time_t))global[135])
#define match_ignore ((int (*)(char *))global[136])
#define delignore ((int (*)(char *))global[137])
#define fatal (global[138])
#define xtra_kill ((void (*)(struct user_entry *))global[139])
/* 140 - 149 */
#define xtra_unpack ((void (*)(struct userrec *, struct user_entry *))global[140])
#define movefile ((int (*) (char *, char *))global[141])
#define copyfile ((int (*) (char *, char *))global[142])
#define do_tcl ((void (*)(char *, char *))global[143])
#define readtclprog ((int (*)(const char *))global[144])
#define get_language ((char *(*)(int))global[145])
#define def_get ((void *(*)(struct userrec *, struct user_entry *))global[146])
#define makepass ((void (*) (char *))global[147])
#define wild_match ((int (*)(const char *, const char *))global[148])
#define maskhost ((void (*)(const char *, char *, int))global[149])
/* 150 - 159 */
#define show_motd ((void(*)(int))global[150])
#define tellhelp ((void(*)(int, char *, struct flag_record *, int))global[151])
#define showhelp ((void(*)(char *, char *, struct flag_record *, int))global[152])
#define add_help_reference ((void(*)(char *))global[153])
#define rem_help_reference ((void(*)(char *))global[154])
#define touch_laston ((void (*)(struct userrec *,char *,time_t))global[155])
#define add_mode ((void (*)(struct chanset_t *,char,char,char *))(*(Function**)(global[156])))
#define rmspace ((void (*)(char *))global[157])
#define in_chain ((int (*)(char *))global[158])
#define add_note ((int (*)(char *,char*,char*,int,int))global[159])
/* 160 - 169 */
#define del_lang_section ((int(*)(char *))global[160])
#define detect_dcc_flood ((int (*) (time_t *,struct chat_info *,int))global[161])
#define flush_lines ((void(*)(int,struct chat_info*))global[162])
#define expected_memory ((int(*)(void))global[163])
#define do_restart (*(int *)(global[164]))
#define add_hook(a,b) (((void (*) (int, Function))global[166])(a,b))
#define del_hook(a,b) (((void (*) (int, Function))global[167])(a,b))
#define H_event (*(p_tcl_bind_list *)(global[213]))
#define H_dcc (*(p_tcl_bind_list *)(global[168]))
#define H_filt (*(p_tcl_bind_list *)(global[169]))
/* 170 - 179 */
#define H_chon (*(p_tcl_bind_list *)(global[170]))
#define H_chof (*(p_tcl_bind_list *)(global[171]))
#define H_load (*(p_tcl_bind_list *)(global[172]))
#define H_unld (*(p_tcl_bind_list *)(global[173]))
#define H_chat (*(p_tcl_bind_list *)(global[174]))
#define H_act (*(p_tcl_bind_list *)(global[175]))
#define H_bcst (*(p_tcl_bind_list *)(global[176]))
#define H_bot (*(p_tcl_bind_list *)(global[177]))
#define H_link (*(p_tcl_bind_list *)(global[178]))
#define H_disc (*(p_tcl_bind_list *)(global[179]))
/* 180 - 189 */
#define H_away (*(p_tcl_bind_list *)(global[180]))
#define H_nkch (*(p_tcl_bind_list *)(global[181]))
#define USERENTRY_BOTADDR (*(struct user_entry_type *)(global[182]))
#define USERENTRY_BOTFL (*(struct user_entry_type *)(global[183]))
#define USERENTRY_HOSTS (*(struct user_entry_type *)(global[184]))
#define USERENTRY_PASS (*(struct user_entry_type *)(global[185]))
#define USERENTRY_XTRA (*(struct user_entry_type *)(global[186]))
#define USERENTRY_INFO (*(struct user_entry_type *)(global[187]))
#define USERENTRY_COMMENT (*(struct user_entry_type *)(global[188]))
#define USERENTRY_LASTON (*(struct user_entry_type *)(global[189]))
/* 190 - 199 */
#define user_del_chan ((void(*)(char *))(global[190]))
#define putlog (global[191])
#define botnet_send_chan ((void(*)(int,char*,char*,int,char*))global[192])
#define list_type_kill ((void(*)(struct list_type *))global[193])
#define logmodes ((int(*)(char *))global[194])
#define masktype ((const char *(*)(int))global[195])
#define stripmodes ((int(*)(char *))global[196])
#define stripmasktype ((const char *(*)(int))global[197])
#define sub_lang ((void(*)(int,char *))global[198])
#define online_since (*(int *)(global[199]))
/* 200 - 209 */
#define cmd_loadlanguage ((int (*)(struct userrec *,int,char *))global[200])
#define check_dcc_attrs ((int (*)(struct userrec *,int))global[201])
#define check_dcc_chanattrs ((int (*)(struct userrec *,char *,int,int))global[202])
#define add_tcl_coups ((void (*) (tcl_coups *))global[203])
#define rem_tcl_coups ((void (*) (tcl_coups *))global[204])
#define botname ((char *)(global[205]))
#define check_tcl_chjn ((void (*) (const char *,const char *,int,char,int,const char *))global[206])
#define sanitycheck_dcc ((int (*)(char *, char *, char *, char *))global[207])
#define isowner ((int (*)(char *))global[208])
#define rfc_casecmp ((int(*)(char *, char *))(*(Function**)(global[209])))
/* 210 - 219 */
#define rfc_ncasecmp ((int(*)(char *, char *, int *))(*(Function**)(global[210])))
#define global_exempts (*(maskrec **)(global[211]))
#define global_invites (*(maskrec **)(global[212]))
#define check_tcl_filt ((const char *(*)(int, const char *))global[213])
#define use_exempts (*(int *)(global[214]))
#define use_invites (*(int *)(global[215]))
#define force_expire (*(int *)(global[216]))
#define add_lang_section ((void(*)(char *))global[217])
#define user_realloc(x,y) ((void *(*)(void *,int,char *,int))global[218])((x),(y),__FILE__,__LINE__)
#define nrealloc(x,y) (((void *(*)())global[219])((x),(y),MODULE_NAME,__FILE__,__LINE__))
/* 220 - 229 */
#define xtra_set ((int(*)(struct userrec *,struct user_entry *, void *))global[220])

#ifdef DEBUG_ASSERT
#  define Assert(expr)          do {                                    \
        if (!(expr))                                                    \
                (global[221](__FILE__, __LINE__, MODULE_NAME));         \
} while (0)
#else
#  define Assert(expr)  do {    } while (0)
#endif

#define allocsock ((int(*)(int sock,int options))global[222])
#define call_hostbyip ((void(*)(IP, char *, int))global[223])
#define call_ipbyhost ((void(*)(char *, IP, int))global[224])
#define iptostr ((char *(*)(IP))global[225])
#define DCC_DNSWAIT (*(struct dcc_table *)(global[226]))
#define hostsanitycheck_dcc ((int(*)(char *, char *, IP, char *, char *))global[227])
#define dcc_dnsipbyhost ((void (*)(char *))global[228])
#define dcc_dnshostbyip ((void (*)(IP))global[229])
/* 230 - 239 */
#define changeover_dcc ((void (*)(int, struct dcc_table *, int))global[230])
#define make_rand_str ((void (*) (char *, int))global[231])
#define protect_readonly (*(int *)(global[232]))
#define findchan_by_dname ((struct chanset_t *(*)(char *))global[233])
#define removedcc ((void (*) (int))global[234])
#define userfile_perm (*(int *)global[235])
#define sock_has_data ((int(*)(int, int))global[236])
#define bots_in_subtree ((int (*)(tand_t *))global[237])
#define users_in_subtree ((int (*)(tand_t *))global[238])
#define egg_inet_aton ((int (*)(const char *cp, struct in_addr *addr))global[239])
/* 240 - 249 */
#define egg_snprintf (global[240])
#define egg_vsnprintf ((int (*)(char *, size_t, const char *, va_list))global[241])
#define egg_memset ((void *(*)(void *, int, size_t))global[242])
#define egg_strcasecmp ((int (*)(const char *, const char *))global[243])
#define egg_strncasecmp ((int (*)(const char *, const char *, size_t))global[244])
#define is_file ((int (*)(const char *))global[245])
#define must_be_owner (*(int *)(global[246]))
#define tandbot (*(tand_t **)(global[247]))
#define party (*(party_t **)(global[248]))
#define open_address_listen ((int (*)(IP addr, int *port))global[249])
/* 250 - 259 */
#define str_escape ((char *(*)(const char *, const char, const char))global[250])
#define strchr_unescape ((char *(*)(char *, const char, register const char))global[251])
#define str_unescape ((void (*)(char *, register const char))global[252])
#define clear_chanlist_member ((void (*)(const char *nick))global[253])
#define fixfrom ((char *(*)(char *))global[254])
#define socklist (*(struct sock_list **)global[255])
#define sockoptions ((int (*)(int, int, int))global[256])
#define kill_bot ((void (*)(char *, char *))global[257])
#define quit_msg ((char *)(global[258]))
#define module_load ((char *(*)(char *))global[259])
/* 260 - 269 */
#define module_unload ((char *(*)(char *, char *))global[260])
#define parties (*(int *)global[261])
#define tell_bottree ((void (*)(int, int))global[262])
#define MD5_Init ((void (*)(MD5_CTX *))global[263])
#define MD5_Update ((void (*)(MD5_CTX *, void *, unsigned long))global[264])
#define MD5_Final ((void (*)(unsigned char *, MD5_CTX *))global[265])
#define wild_match_per ((int (*)(const char *, const char *))global[266])
#define killtransfer ((void(*)(int))global[267])
#define write_ignores ((int (*)(FILE *, int))global[268])
#define copy_to_tmp (*(int *)(global[269]))
/* 270 - */
#define quiet_reject (*(int *)(global[270]))
#define file_readable ((int (*) (char *))global[271])
#define strip_mirc_codes ((void (*)(int, char *))global[272])
#define check_ansi ((int (*) (char *))global[273])
#define oatoi ((int (*) (const char *))global[274])
#define str_isdigit ((int (*) (const char *))global[275])
#define remove_crlf ((void (*)(char **))global[276])

/* This is for blowfish module, couldnt be bothered making a whole new .h
 * file for it ;)
 */
#ifndef MAKING_ENCRYPTION
# define encrypt_string(a, b)                                          \
        (((char *(*)(char *,char*))encryption_funcs[4])(a,b))
# define decrypt_string(a, b)                                          \
        (((char *(*)(char *,char*))encryption_funcs[5])(a,b))
#endif

#endif /* _EGG_MOD_MODULE_H */
