/* misc.c
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
 * $Id: misc.c,v 1.12 2004/09/10 01:10:50 wcc Exp $
 */

#include "main.h"

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include "chan.h"
#include "modules.h"
#include "stat.h"

#include "misc.h"
#include "botmsg.h"  /* simple_sprintf, botnet_send_* */
#include "dcc.h"     /* DCC_*, STAT_*, struct dcc_t */
#include "dccutil.h" /* dprintf, chatout */
#include "help.h"    /* help_subst */
#include "match.h"   /* wild_match */
#include "rfc1459.h" /* rfc_casecmp */
#include "userrec.h" /* write_userfile */


extern struct dcc_t *dcc;
extern struct chanset_t *chanset;
extern char botnetnick[];
extern int con_chan, strict_ident;
extern time_t now;


int my_strcpy(register char *a, register char *b)
{
  register char *c = b;

  while (*b)
    *a++ = *b++;
  *a = *b;
  return b - c;
}

/* Split first word off of 'rest' and put it in 'first'. */
void splitc(char *first, char *rest, char divider)
{
  char *p = strchr(rest, divider);

  if (p == NULL) {
    if (first != rest && first)
      first[0] = 0;
    return;
  }

  *p = 0;

  if (first != NULL)
    strcpy(first, rest);

  if (first != rest)
    /* In most circumstances, strcpy with src and dst being the same buffer can
     * produce undefined results. We're safe here, as the src is guaranteed to
     * be at least 2 bytes higher in memory than dest. <Cybah>
     *
     * FIXME: It's still hack-ish and probably needs to be changed. It makes
     * valgrind bitch alot =P -Wcc
     */
    strcpy(rest, p + 1);
}

/* Identical to splitc(), but lets you specify the 'max' number of bytes
 * (EXCLUDING the terminating NUL).
 *
 * Example of use:
 *   char buf[HANDLEN + 1];
 *   splitcn(buf, input, "@", HANDLEN);
 */
void splitcn(char *first, char *rest, char divider, size_t max)
{
  char *p = strchr(rest, divider);

  if (p == NULL) {
    if (first != rest && first)
      first[0] = 0;
    return;
  }

  *p = 0;
  if (first != NULL)
    strncpyz(first, rest, max);

  if (first != rest)
    /* In most circumstances, strcpy with src and dst being the same buffer can
     * produce undefined results. We're safe here, as the src is guaranteed to
     * be at least 2 bytes higher in memory than dest. <Cybah>
     *
     * FIXME: It's still hack-ish and probably needs to be changed. It makes
     * valgrind bitch alot =P -Wcc
     */
    strcpy(rest, p + 1);
}

char *splitnick(char **blah)
{
  char *p = strchr(*blah, '!'), *q = *blah;

  if (p) {
    *p = 0;
    *blah = p + 1;
    return q;
  }

  return "";
}

char *newsplit(char **rest)
{
  register char *o, *r;

  if (!rest)
    return *rest = "";

  o = *rest;
  while (*o == ' ')
    o++;
  r = o;
  while (*o && (*o != ' '))
    o++;
  if (*o)
    *o++ = 0;
  *rest = o;

  return r;
}

/* Creates a mask from a nick!user@host.
 * Examples:
 *   abc!user@a.b.host -> *!user@*.b.host
 *   abc!user@1.2.3.4  -> *!user@1.2.3.*
 *   abc!user@0:0:0:0:0:ffff:1.2.3.4 -> *!user@0:0:0:0:0:ffff:1.2.3.*
 *   abc!user@3ffe:604:2:b02e:6174:7265:6964:6573 -> *!user@3ffe:604:2:b02e:6174:7265:6964:*
 *   MASKHOST_BAN: abc!~user@1.2.3.4 -> *!*user@1.2.3.*
 *   MASKHOST_HOST (with strict-ident): abc!~user@1.2.3.4 -> *!~user@1.2.3.*
 *   MASKHOST_HOST (without strict-ident): abc!~user@1.2.3.4 -> *!?user@1.2.3.*
 */
void maskhost(const char *s, char *new, int host)
{
  register const char *p, *q, *e, *f;
  int i, j = 0;

  *new++ = '*';
  *new++ = '!';

  q = strchr(s, '!');
  if (q)
    p = q + 1;
  else
    p = s;

  /* Strip of any nick. If a username is found, use last 8 chars. */
  q = strchr(p, '@');
  if (q) {
    if ((q - p) > 9) {
      new[0] = '*';
      p = q - 7;
      i = 1;
    }
    else
      i = 0;
    while (*p != '@') {
      if (!j && strchr("~+-^=", *p)) {
        if (!host)
          new[i] = '*';
        else if (!strict_ident)
          new[i] = '?';
        else
          new[i] = *p;
      }
      else
        new[i] = *p;
      p++;
      i++;
      j++;
    }
    new[i++] = '@';
    q++;
  }
  else {
    new[0] = '*';
    new[1] = '@';
    i = 2;
    q = s;
  }
  new += i;
  e = NULL;
  /* Now q points to the hostname, i points to where to put the mask. */
  if (!strchr(q, ':') && (!(p = strchr(q, '.')) || !(e = strchr(p + 1, '.')))) { /* TLD or 2 part host. */
    strcpy(new, q);
    return;
  }

  if (e == NULL) {        /* IPv6 address? */
    const char *mask_str;

    f = strrchr(q, ':');
    if (strchr(f, '.')) { /* IPv4 wrapped in an IPv6?     */
      f = strrchr(f, '.');
      mask_str = ".*";
    }
    else                  /* True IPv6. */
      mask_str = ":*";
    strncpy(new, q, f - q);
    /* No need to new[f-q] = 0 here, as the strcpy below will terminate the
     * string for us. */
    new += (f - q);
    strcpy(new, mask_str);
    return;
  }

  for (f = e; *f; f++);
  f--;
  if (*f >= '0' && *f <= '9') { /* Numeric IP address */
    while (*f != '.')
      f--;
    strncpy(new, q, f - q);
    /* No need to new[f-q] = 0 here, as the strcpy below will terminate the
     * string for us. */
    new += (f - q);
    strcpy(new, ".*");
  }
  else {
   /* a.b.c     -> *.b.c
    * a.b.c.d   -> *.b.c.d if tld is a country (2 chars)
    *           OR *.c.d if tld is com/edu/etc (3 chars)
    * a.b.c.d.e -> *.c.d.e,
    * etc
    */
    const char *x = strchr(e + 1, '.');

    if (!x)
      x = p;
    else if (strchr(x + 1, '.'))
      x = e;
    else if (strlen(x) == 3)
      x = p;
    else
      x = e;
    sprintf(new, "*%s", x);
  }
}

/* Dump a potentially super-long string of text ('data') to 'idx', prefixed
 * by 'prefix'.
 *
 * Example of use:
 *   dumplots(idx, "Tcl error: ", result);
 */
void dumplots(int idx, const char *prefix, char *data)
{
  char *p = data, *q, *n, c;
  const int max_data_len = 500 - strlen(prefix);

  if (!*data) {
    dprintf(idx, "%s\n", prefix);
    return;
  }

  while (strlen(p) > max_data_len) {
    q = p + max_data_len;
    /* Search for embedded linefeed first. */
    n = strchr(p, '\n');
    if (n && n < q) {
      /* Great! Dump that first line then start over. */
      *n = 0;
      dprintf(idx, "%s%s\n", prefix, p);
      *n = '\n';
      p = n + 1;
    } else {
      /* Search backwards for the last space. */
      while (*q != ' ' && q != p)
        q--;
      if (q == p)
        q = p + max_data_len;
      c = *q;
      *q = 0;
      dprintf(idx, "%s%s\n", prefix, p);
      *q = c;
      p = q;
      if (c == ' ')
        p++;
    }
  }

  /* Last trailing bit: split by linefeeds if possible. */
  n = strchr(p, '\n');
  while (n) {
    *n = 0;
    dprintf(idx, "%s%s\n", prefix, p);
    *n = '\n';
    p = n + 1;
    n = strchr(p, '\n');
  }

  if (*p)
    dprintf(idx, "%s%s\n", prefix, p);  /* Last trailing bit. */
}

/* Convert an interval (in seconds ('now' - 'then')) to one of the following
 * (depending on 'flag'), and copy it to 'out'.
 *
 *   DAYS_IN:  "in 19 days", "in 1 day", "at 18:12"
 *   DAYS_FOR: "for 19 days", "for 1 day", "for 09:10"
 *   DAYS_AGO: "19 days ago", "1 day ago", "18:12"
 */
void days(time_t now, time_t then, char *out, int flag)
{
  int i;

  if (now - then > 86400) {
    int days = (now - then) / 86400;

    switch (flag) {
      case DAYS_IN:
        sprintf(out, "in %d day%s", days, (days == 1) ? "" : "s");
        break;
      case DAYS_FOR:
        sprintf(out, "for %d day%s", days, (days == 1) ? "" : "s");
        break;
      case DAYS_AGO:
        sprintf(out, "%d day%s ago", days, (days == 1) ? "" : "s");
        break;
    }
    return;
  }

  switch (flag) {
    case DAYS_IN:
      egg_strftime(out, 9, "at %H:%M", localtime(&now));
      break;
    case DAYS_FOR:
      now -= then;
      i = (int) (now / 3600);
      sprintf(out, "for %02d:%02d", i, (int) ((i - (i * 3600)) / 60));
      break;
    case DAYS_AGO:
      egg_strftime(out, 6, "%H:%M", localtime(&then));
      break;
  }
}

/* This will return a pointer to the first character after the @ in the string
 * given it.
 */
char *extracthostname(char *hostmask)
{
  char *p = strrchr(hostmask, '@');

  return p ? p + 1 : "";
}

/* This fills string 's' with random letters and digits, up to 'len' and then
 * NUL-terminates 's' at 'len'.
 */
void make_rand_str(char *s, int len)
{
  int j;

  for (j = 0; j < len; j++) {
    if (!randint(3))
      s[j] = '0' + randint(10);
    else
      s[j] = 'a' + randint(26);
  }

  s[len] = 0;
}

/* This converts an octal string into a decimal integer value. If the string
 * is empty or contains non-octal characters, -1 is returned.
 */
int oatoi(const char *octal)
{
  register int i;

  if (!*octal)
    return -1;

  for (i = 0; ((*octal >= '0') && (*octal <= '7')); octal++)
    i = (i * 8) + (*octal - '0');

  if (*octal)
    return -1;

  return i;
}

/* Return an allocated buffer which contains a copy of the string 'str', with
 * all 'div' characters escaped by 'mask'. 'mask' characters are escaped too.
 *
 * Remember to free the returned memory block.
 */
char *str_escape(const char *str, const char div, const char mask)
{
  const int len = strlen(str);
  int buflen = (2 * len), blen = 0;
  char *buf = nmalloc(buflen + 1), *b = buf;
  const char *s;

  if (!buf)
    return NULL;

  for (s = str; *s; s++) {
    /* Resize buffer. */
    if ((buflen - blen) <= 3) {
      buflen = (buflen * 2);
      buf = nrealloc(buf, buflen + 1);
      if (!buf)
        return NULL;
      b = buf + blen;
    }

    if (*s == div || *s == mask) {
      sprintf(b, "%c%02x", mask, *s);
      b += 3;
      blen += 3;
    } else {
      *(b++) = *s;
      blen++;
    }
  }

  *b = 0;
  return buf;
}

/* Search for a certain character 'div' in the string 'str', while ignoring
 * escaped characters prefixed with 'mask'.
 *
 * The string
 *
 *   "\\3a\\5c i am funny \\3a):further text\\5c):oink"
 *
 * as str, '\\' as mask and ':' as div would change the str buffer to
 *
 *   ":\\ i am funny :)"
 *
 * and return a pointer to "further text\\5c):oink".
 *
 * NOTE: If you look carefully, you'll notice that strchr_unescape()
 *       behaves differently than strchr().
 */
char *strchr_unescape(char *str, const char div, register const char esc_char)
{
  char buf[3];
  register char *s, *p;

  buf[2] = 0;
  for (s = p = str; *s; s++, p++) {
    if (*s == esc_char) {
      /* Convert code to character. */
      buf[0] = s[1];
      buf[1] = s[2];
      *p = (unsigned char) strtol(buf, NULL, 16);
      s += 2;
    } else if (*s == div) {
      *p = *s = 0;
      return (s + 1); /* Found searched-for character. */
    } else
      *p = *s;
  }

  *p = 0;
  return NULL;
}

/* As strchr_unescape(), but converts the complete string, without searching
 * for a specific delimiter character.
 */
void str_unescape(char *str, register const char esc_char)
{
  (void) strchr_unescape(str, 0, esc_char);
}

/* Is every character in a string a digit? */
int str_isdigit(const char *str)
{
  if (!*str)
    return 0;

  for (; *str; ++str) {
    if (!egg_isdigit(*str))
      return 0;
  }

  return 1;
}

/* Kills the bot. s1 is the reason shown to other bots,
 * s2 the reason shown on the partyline. (Sup 25Jul2001)
 */
void kill_bot(char *s1, char *s2)
{
  call_hook(HOOK_DIE);
  chatout("*** %s\n", s1);
  botnet_send_chat(-1, botnetnick, s1);
  botnet_send_bye();
  write_userfile(-1);
  fatal(s2, 0);
}
