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
 * $Id: misc.c,v 1.11 2004/08/31 22:56:12 wcc Exp $
 */

#include "main.h"

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include "chan.h"
#include "modules.h"
#include "stat.h"


#include "botmsg.h"  /* simple_sprintf, botnet_send_* */
#include "dcc.h"     /* DCC_*, STAT_*, struct dcc_t */
#include "dccutil.h" /* dprintf, chatout */
#include "help.h"    /* help_subst */
#include "match.h"   /* wild_match */
#include "rfc1459.h" /* rfc_casecmp */
#include "userrec.h" /* write_userfile */


extern struct dcc_t *dcc;
extern struct chanset_t *chanset;
extern char motdfile[], botnetnick[], bannerfile[];
extern int con_chan, strict_ident;
extern time_t now;


/*  This implementation wont overrun dst - 'max' is the max bytes that dst
 *  can be, including the null terminator. So if 'dst' is a 128 byte buffer,
 *  pass 128 as 'max'. The function will _always_ null-terminate 'dst'.
 *
 *  Returns: The number of characters appended to 'dst'.
 *
 *  Usage example:
 *
 *    char buf[128];
 *    size_t bufsize = sizeof(buf);
 *
 *    buf[0] = 0, bufsize--;
 *
 *    while (blah && bufsize) {
 *      bufsize -= egg_strcatn(buf, <some-long-string>, sizeof(buf));
 *    }
 *
 *  <Cybah>
 */
int egg_strcatn(char *dst, const char *src, size_t max)
{
  size_t tmpmax = 0;

  /* find end of 'dst' */
  while (*dst && max > 0) {
    dst++;
    max--;
  }

  /*    Store 'max', so we can use it to workout how many characters were
   *  written later on.
   */
  tmpmax = max;

  /* copy upto, but not including the null terminator */
  while (*src && max > 1) {
    *dst++ = *src++;
    max--;
  }

  /* null-terminate the buffer */
  *dst = 0;

  /*    Don't include the terminating null in our count, as it will cumulate
   *  in loops - causing a headache for the caller.
   */
  return tmpmax - max;
}

int my_strcpy(register char *a, register char *b)
{
  register char *c = b;

  while (*b)
    *a++ = *b++;
  *a = *b;
  return b - c;
}

/* Split first word off of rest and put it in first
 */
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
    /*    In most circumstances, strcpy with src and dst being the same buffer
     *  can produce undefined results. We're safe here, as the src is
     *  guaranteed to be at least 2 bytes higher in memory than dest. <Cybah>
     */
    strcpy(rest, p + 1);
}

/*    As above, but lets you specify the 'max' number of bytes (EXCLUDING the
 * terminating null).
 *
 * Example of use:
 *
 * char buf[HANDLEN + 1];
 *
 * splitcn(buf, input, "@", HANDLEN);
 *
 * <Cybah>
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
    /*    In most circumstances, strcpy with src and dst being the same buffer
     *  can produce undefined results. We're safe here, as the src is
     *  guaranteed to be at least 2 bytes higher in memory than dest. <Cybah>
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

void remove_crlf(char **line)
{
  char *p;

  p = strchr(*line, '\n');
  if (p != NULL)
    *p = 0;
  p = strchr(*line, '\r');
  if (p != NULL)
    *p = 0;
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

/* Convert "abc!user@a.b.host" into "*!user@*.b.host"
 * or "abc!user@1.2.3.4" into "*!user@1.2.3.*"
 * or "abc!user@0:0:0:0:0:ffff:1.2.3.4" into "*!user@0:0:0:0:0:ffff:1.2.3.*"
 * or "abc!user@3ffe:604:2:b02e:6174:7265:6964:6573" into
 *    "*!user@3ffe:604:2:b02e:6174:7265:6964:*"
 */
void _maskhost(const char *s, char *nw, int host)
{
  register const char *p, *q, *e, *f;
  int i;
  char *newmask = nw;

  *nw++ = '*';
  *nw++ = '!';
  p = (q = strchr(s, '!')) ? q + 1 : s;
  /* Strip of any nick, if a username is found, use last 8 chars */
  if ((q = strchr(p, '@'))) {
    int fl = 0;

    if ((q - p) > 9) {
      nw[0] = '*';
      p = q - 7;
      i = 1;
    } else
      i = 0;
    while (*p != '@') {
      if (!fl && strchr("~+-^=", *p) && !host) {
        /* depends on ban, we change prefix to '?' or '*'
         * but since if it's ban isn't better just to put '*'? (takeda)
         */
        if (strict_ident)
          nw[i] = '?';
        else
          nw[i] = '*';
      } else
        nw[i] = *p;
      fl++;
      p++;
      i++;
    }
    nw[i++] = '@';
    q++;
  } else {
    nw[0] = '*';
    nw[1] = '@';
    i = 2;
    q = s;
  }
  nw += i;
  e = NULL;
  /* Now q points to the hostname, i point to where to put the mask */
  if ((!(p = strchr(q, '.')) || !(e = strchr(p + 1, '.'))) && !strchr(q, ':'))
    /* TLD or 2 part host */
    strcpy(nw, q);
  else {
    if (e == NULL) {            /* IPv6 address?                */
      const char *mask_str;

      f = strrchr(q, ':');
      if (strchr(f, '.')) {     /* IPv4 wrapped in an IPv6?     */
        f = strrchr(f, '.');
        mask_str = ".*";
      } else                      /* ... no, true IPv6.               */
        mask_str = ":*";
      strncpy(nw, q, f - q);
      /* No need to nw[f-q] = 0 here, as the strcpy below will
       * terminate the string for us.
       */
      nw += (f - q);
      strcpy(nw, mask_str);
    } else {
      for (f = e; *f; f++);
      f--;
      if (*f >= '0' && *f <= '9') {     /* Numeric IP address */
        while (*f != '.')
          f--;
        strncpy(nw, q, f - q);
        /* No need to nw[f-q] = 0 here, as the strcpy below will
         * terminate the string for us.
         */
        nw += (f - q);
        strcpy(nw, ".*");
      } else {                    /* Normal host >= 3 parts */
        /*    a.b.c  -> *.b.c
         *    a.b.c.d ->  *.b.c.d if tld is a country (2 chars)
         *             OR   *.c.d if tld is com/edu/etc (3 chars)
         *    a.b.c.d.e -> *.c.d.e   etc
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
        sprintf(nw, "*%s", x);
      }
    }
  }
  if (host && !strict_ident)
    fixfrom(newmask);
}

/* Dump a potentially super-long string of text.
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
    /* Search for embedded linefeed first */
    n = strchr(p, '\n');
    if (n && n < q) {
      /* Great! dump that first line then start over */
      *n = 0;
      dprintf(idx, "%s%s\n", prefix, p);
      *n = '\n';
      p = n + 1;
    } else {
      /* Search backwards for the last space */
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
  /* Last trailing bit: split by linefeeds if possible */
  n = strchr(p, '\n');
  while (n) {
    *n = 0;
    dprintf(idx, "%s%s\n", prefix, p);
    *n = '\n';
    p = n + 1;
    n = strchr(p, '\n');
  }
  if (*p)
    dprintf(idx, "%s%s\n", prefix, p);  /* Last trailing bit */
}

/* Convert an interval (in seconds) to one of:
 * "19 days ago", "1 day ago", "18:12"
 */
void daysago(time_t now, time_t then, char *out)
{
  if (now - then > 86400) {
    int days = (now - then) / 86400;

    sprintf(out, "%d day%s ago", days, (days == 1) ? "" : "s");
    return;
  }
  egg_strftime(out, 6, "%H:%M", localtime(&then));
}

/* Convert an interval (in seconds) to one of:
 * "in 19 days", "in 1 day", "at 18:12"
 */
void days(time_t now, time_t then, char *out)
{
  if (now - then > 86400) {
    int days = (now - then) / 86400;

    sprintf(out, "in %d day%s", days, (days == 1) ? "" : "s");
    return;
  }
  egg_strftime(out, 9, "at %H:%M", localtime(&now));
}

/* Convert an interval (in seconds) to one of:
 * "for 19 days", "for 1 day", "for 09:10"
 */
void daysdur(time_t now, time_t then, char *out)
{
  char s[81];
  int hrs, mins;

  if (now - then > 86400) {
    int days = (now - then) / 86400;

    sprintf(out, "for %d day%s", days, (days == 1) ? "" : "s");
    return;
  }
  strcpy(out, "for ");
  now -= then;
  hrs = (int) (now / 3600);
  mins = (int) ((now - (hrs * 3600)) / 60);
  sprintf(s, "%02d:%02d", hrs, mins);
  strcat(out, s);
}



/* Substitute %x codes in help files
 *
 * %B = bot nickname
 * %V = version
 * %C = list of channels i monitor
 * %E = eggdrop banner
 * %A = admin line
 * %n = network name
 * %T = current time ("14:15")
 * %N = user's nickname
 * %U = display system name if possible
 * %{+xy}     require flags to read this section
 * %{-}       turn of required flag matching only
 * %{center}  center this line
 * %{cols=N}  start of columnated section (indented)
 * %{help=TOPIC} start a section for a particular command
 * %{end}     end of section
 */

/* Substitute vars in a lang text to dcc chatter. */
void sub_lang(int idx, char *text)
{
  char s[1024];
  struct flag_record fr = { FR_GLOBAL | FR_CHAN, 0, 0, 0, 0, 0 };

  get_user_flagrec(dcc[idx].user, &fr, dcc[idx].u.chat->con_chan);
  help_subst(NULL, NULL, 0,
             (dcc[idx].status & STAT_TELNET) ? 0 : HELP_IRC, NULL);
  strncpyz(s, text, sizeof s);
  if (s[strlen(s) - 1] == '\n')
    s[strlen(s) - 1] = 0;
  if (!s[0])
    strcpy(s, " ");
  help_subst(s, dcc[idx].nick, &fr, 1, botnetnick);
  if (s[0])
    dprintf(idx, "%s\n", s);
}

/* This will return a pointer to the first character after the @ in the
 * string given it.  Possibly it's time to think about a regexp library
 * for eggdrop...
 */
char *extracthostname(char *hostmask)
{
  char *p = strrchr(hostmask, '@');

  return p ? p + 1 : "";
}

/* Show motd to dcc chatter
 */
void show_motd(int idx)
{
  FILE *vv;
  char s[1024];
  struct flag_record fr = { FR_GLOBAL | FR_CHAN, 0, 0, 0, 0, 0 };

  if (!is_file(motdfile))
    return;

  vv = fopen(motdfile, "r");
  if (!vv)
    return;

  get_user_flagrec(dcc[idx].user, &fr, dcc[idx].u.chat->con_chan);
  dprintf(idx, "\n");
  /* reset the help_subst variables to their defaults */
  help_subst(NULL, NULL, 0,
             (dcc[idx].status & STAT_TELNET) ? 0 : HELP_IRC, NULL);
  while (!feof(vv)) {
    fgets(s, 120, vv);
    if (!feof(vv)) {
      if (s[strlen(s) - 1] == '\n')
        s[strlen(s) - 1] = 0;
      if (!s[0])
        strcpy(s, " ");
      help_subst(s, dcc[idx].nick, &fr, 1, botnetnick);
      if (s[0])
        dprintf(idx, "%s\n", s);
    }
  }
  fclose(vv);
  dprintf(idx, "\n");
}

/* Show banner to telnet user
 */
void show_banner(int idx)
{
  FILE *vv;
  char s[1024];
  struct flag_record fr = { FR_GLOBAL | FR_CHAN, 0, 0, 0, 0, 0 };

  if (!is_file(bannerfile))
    return;

  vv = fopen(bannerfile, "r");
  if (!vv)
    return;

  get_user_flagrec(dcc[idx].user, &fr, dcc[idx].u.chat->con_chan);
  /* reset the help_subst variables to their defaults */
  help_subst(NULL, NULL, 0, 0, NULL);
  while (!feof(vv)) {
    fgets(s, 120, vv);
    if (!feof(vv)) {
      if (!s[0])
        strcpy(s, " \n");
      help_subst(s, dcc[idx].nick, &fr, 0, botnetnick);
      dprintf(idx, "%s", s);
    }
  }
  fclose(vv);
}

/* Create a string with random letters and digits
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

/* Convert an octal string into a decimal integer value.  If the string
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

/* Return an allocated buffer which contains a copy of the string
 * 'str', with all 'div' characters escaped by 'mask'. 'mask'
 * characters are escaped too.
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

/* Search for a certain character 'div' in the string 'str', while
 * ignoring escaped characters prefixed with 'mask'.
 *
 * The string
 *
 *   "\\3a\\5c i am funny \\3a):further text\\5c):oink"
 *
 * as str, '\\' as mask and ':' as div would change the str buffer
 * to
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
    if (*s == esc_char) {       /* Found escape character.              */
      /* Convert code to character. */
      buf[0] = s[1], buf[1] = s[2];
      *p = (unsigned char) strtol(buf, NULL, 16);
      s += 2;
    } else if (*s == div) {
      *p = *s = 0;
      return (s + 1);           /* Found searched for character.        */
    } else
      *p = *s;
  }
  *p = 0;
  return NULL;
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

/* As strchr_unescape(), but converts the complete string, without
 * searching for a specific delimiter character.
 */
void str_unescape(char *str, register const char esc_char)
{
  (void) strchr_unescape(str, 0, esc_char);
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
