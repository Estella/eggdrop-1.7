/* eggdrop.h: compile-time settings
 *
 * IF YOU ALTER THIS FILE, YOU NEED TO RECOMPILE THE BOT.
 *
 * Copyright (C) 1997 Robey Pointer
 * Copyright (C) 1999-2004 Eggheads Development Team
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
 * $Id: eggdrop.h,v 1.7 2004/11/26 05:35:27 wcc Exp $
 */

#ifndef _EGG_EGGDROP_H
#define _EGG_EGGDROP_H

/* If you're *only* going to link to new version bots (1.3.0 or higher), then
 * then you can safely define this.
 */
#undef NO_OLD_BOTNET

/* Set the following to the timestamp for the logfile entries. Popular times
 * might be "[%H:%M]" (hour, min), or "[%H:%M:%S]" (hour, min, sec). Read
 * 'man strftime' for more formatting options. Keep it below 32 characters.
 */
#define LOG_TS  "[%H:%M]"

/* HANDLEN defines the maximum length a handle on the bot can be. Standard
 * (and minimum) is 9 characters long.
 *
 * Beware that using lengths over 9 chars is 'non-standard', and if you wish
 * to link to other bots, they _must_ both have the same maximum handle length.
 */
#define HANDLEN  9 /* Valid values are 9 to NICKMAX. */

/* You should leave this at 32 characters and modify nick-len in the
 * configuration file instead.
 */
#define NICKMAX  32  /* Valid values are HANDLEN to 32. */

/* This is the maximum length for a log line.
 */
#define LOGLINEMAX  767

/* Handy string lengths. */
#define UHOSTMAX    291 + NICKMAX /* 32 (ident) + 3 (\0, !, @) + NICKMAX */
#define DIRMAX      512

/* Invalid characters. */
#define BADNICKCHARS "-,+*=:!.@#;$%&"
#define BADHANDCHARS "-,+*=:!.@#;$%&"


#define NICKLEN     NICKMAX + 1
#define UHOSTLEN    UHOSTMAX + 1
#define DIRLEN      DIRMAX + 1
#define NOTENAMELEN ((HANDLEN * 2) + 1)


#if (NICKMAX < 9) || (NICKMAX > 32)
#  include "Error: Invalid NICKMAX value."
#endif

#if (HANDLEN < 9) || (HANDLEN > 32)
#  include "Error: Invalid HANDLEN value."
#endif

#if HANDLEN > NICKMAX
#  include "Error: HANDLEN MUST BE <= NICKMAX."
#endif

#ifndef COMPILING_MEM
#  undef malloc
#  define malloc(x) dont_use_old_malloc(x)
#  undef free
#  define free(x)   dont_use_old_free(x)
#endif /* !COMPILING_MEM */

/* These apparently are unsafe without recasting. */
#define egg_isdigit(x)  isdigit((int)  (unsigned char) (x))
#define egg_isxdigit(x) isxdigit((int) (unsigned char) (x))
#define egg_isascii(x)  isascii((int)  (unsigned char) (x))
#define egg_isspace(x)  isspace((int)  (unsigned char) (x))
#define egg_islower(x)  islower((int)  (unsigned char) (x))

/***********************************************************************/

/* chan & global */
#define FLOOD_PRIVMSG    0
#define FLOOD_NOTICE     1
#define FLOOD_CTCP       2
#define FLOOD_NICK       3
#define FLOOD_JOIN       4
#define FLOOD_KICK       5
#define FLOOD_DEOP       6

#define FLOOD_CHAN_MAX   7
#define FLOOD_GLOBAL_MAX 3

/* For local console */
#define STDIN  0
#define STDOUT 1
#define STDERR 2


/* Number of global partyline channels. */
#define GLOBAL_CHANS 100000

#define STR_PROTECT     2
#define STR_DIR         1

#define HELP_DCC        1
#define HELP_TEXT       2
#define HELP_IRC        16

#endif /* _EGG_EGGDROP_H */
