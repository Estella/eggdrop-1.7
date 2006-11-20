/* traffic.c: traffic accounting
 *
 * Copyright (C) 2004 - 2005 Eggheads Development Team
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
 * $Id: traffic.c,v 1.6 2006/11/20 13:26:01 tothwolf Exp $
 */

#include "main.h"

#include "traffic.h"
#include "dcc.h"     /* struct dcc_table, dcc */
#include "dccutil.h" /* dprintf */
#include "logfile.h" /* putlog, LOG_* */


extern struct dcc_t *dcc;


egg_traffic_t traffic;
static char trafficbuf[48];


static char *btos(unsigned long bytes)
{
  char unit[10];
  float xbytes;

  sprintf(unit, "Bytes");
  xbytes = bytes;
  if (xbytes > 1024.0) {
    sprintf(unit, "KBytes");
    xbytes = xbytes / 1024.0;
  }
  if (xbytes > 1024.0) {
    sprintf(unit, "MBytes");
    xbytes = xbytes / 1024.0;
  }
  if (xbytes > 1024.0) {
    sprintf(unit, "GBytes");
    xbytes = xbytes / 1024.0;
  }
  if (xbytes > 1024.0) {
    sprintf(unit, "TBytes");
    xbytes = xbytes / 1024.0;
  }

  if (bytes > 1024)
    egg_snprintf(trafficbuf, sizeof(trafficbuf), "%.2f %s", xbytes, unit);
  else
    egg_snprintf(trafficbuf, sizeof(trafficbuf), "%lu Bytes", bytes);

  return trafficbuf;
}

void traffic_update_in(struct dcc_table *type, int size)
{
  if (!type->name) /* Sanity check. */
    return;

  if (!strncmp(type->name, "BOT", 3))
    traffic.in_today.botnet += size;
  else if (!strcmp(type->name, "SERVER"))
    traffic.in_today.irc += size;
  else if (!strncmp(type->name, "CHAT", 4))
    traffic.in_today.partyline += size;
  else if (!strncmp(type->name, "FILES", 5))
    traffic.in_today.filesys += size;
  else if (!strcmp(type->name, "SEND"))
    traffic.in_today.transfer += size;
  else if (!strncmp(type->name, "GET", 3))
    traffic.in_today.transfer += size;
  else
    traffic.in_today.unknown += size;
}

void traffic_update_out(struct dcc_table *type, int size)
{
  if (!type->name) /* Sanity check. */
    return;

  if (!strncmp(type->name, "BOT", 3))
    traffic.out_today.botnet += size;
  else if (!strcmp(type->name, "SERVER"))
    traffic.out_today.irc += size;
  else if (!strncmp(type->name, "CHAT", 4))
    traffic.out_today.partyline += size;
  else if (!strncmp(type->name, "FILES", 5))
    traffic.out_today.filesys += size;
  else if (!strcmp(type->name, "SEND"))
    traffic.out_today.transfer += size;
  else if (!strncmp(type->name, "GET", 3))
    traffic.out_today.transfer += size;
  else
    traffic.out_today.unknown += size;
}

void traffic_reset()
{
  traffic.out_total.irc       += traffic.out_today.irc;
  traffic.out_total.botnet    += traffic.out_today.botnet;
  traffic.out_total.filesys   += traffic.out_today.filesys;
  traffic.out_total.partyline += traffic.out_today.partyline;
  traffic.out_total.transfer  += traffic.out_today.transfer;
  traffic.out_total.unknown   += traffic.out_today.unknown;

  traffic.in_total.irc       += traffic.in_today.irc;
  traffic.in_total.botnet    += traffic.in_today.botnet;
  traffic.in_total.filesys   += traffic.in_today.filesys;
  traffic.in_total.partyline += traffic.in_today.partyline;
  traffic.in_total.transfer  += traffic.in_today.transfer;
  traffic.in_total.unknown   += traffic.in_today.unknown;

  traffic.in_today.irc       = traffic.out_today.irc       = 0;
  traffic.in_today.botnet    = traffic.out_today.botnet    = 0;
  traffic.in_today.filesys   = traffic.out_today.filesys   = 0;
  traffic.in_today.partyline = traffic.out_today.partyline = 0;
  traffic.in_today.transfer  = traffic.out_today.transfer  = 0;
  traffic.in_today.unknown   = traffic.out_today.unknown   = 0;

  egg_memset(&traffic.out_today, 0, sizeof(traffic.out_today));
  egg_memset(&traffic.in_today,  0, sizeof(traffic.in_today));
}

static void cmd_traffic(struct userrec *u, int idx, char *par)
{
  unsigned long today, total;

  dprintf(idx, "Traffic Since Last Restart\n");
  dprintf(idx, "==========================\n");
  dprintf(idx, "IRC:\n");
  dprintf(idx, "  Out: %s (%s today)\n",
          btos(traffic.out_total.irc + traffic.out_today.irc),
          btos(traffic.out_today.irc));
  dprintf(idx, "   In: %s (%s today)\n",
          btos(traffic.in_total.irc + traffic.in_today.irc),
          btos(traffic.in_today.irc));
  dprintf(idx, "Botnet:\n");
  dprintf(idx, "  Out: %s (%s today)\n",
          btos(traffic.out_total.botnet + traffic.out_today.botnet),
          btos(traffic.out_today.botnet));
  dprintf(idx, "   In: %s (%s today)\n",
          btos(traffic.in_total.botnet + traffic.in_today.botnet),
          btos(traffic.in_today.botnet));
  dprintf(idx, "Partyline:\n");
  dprintf(idx, "  Out: %s (%s today)\n",
          btos(traffic.out_total.partyline + traffic.out_today.partyline),
          btos(traffic.out_today.partyline));
  dprintf(idx, "   In: %s (%s today)\n",
          btos(traffic.in_total.partyline + traffic.in_today.partyline),
          btos(traffic.in_today.partyline));
  dprintf(idx, "Filesys Module:\n");
  dprintf(idx, "  Out: %s (%s today)\n",
          btos(traffic.out_total.filesys + traffic.out_today.filesys),
          btos(traffic.out_today.filesys));
  dprintf(idx, "   In: %s (%s today)\n",
          btos(traffic.in_total.filesys + traffic.in_today.filesys),
          btos(traffic.in_today.filesys));
  dprintf(idx, "Transfer Module:\n");
  dprintf(idx, "  Out: %s (%s today)\n",
          btos(traffic.out_total.transfer + traffic.out_today.transfer),
          btos(traffic.out_today.transfer));
  dprintf(idx, "   In: %s (%s today)\n",
          btos(traffic.in_total.transfer + traffic.in_today.transfer),
          btos(traffic.in_today.transfer));
  dprintf(idx, "Unclassified:\n");
  dprintf(idx, "  Out: %s (%s today)\n",
          btos(traffic.out_total.unknown + traffic.out_today.unknown),
          btos(traffic.out_today.unknown));
  dprintf(idx, "   In: %s (%s today)\n",
          btos(traffic.in_total.unknown + traffic.in_today.unknown),
          btos(traffic.in_today.unknown));
  dprintf(idx, "---\n");
  dprintf(idx, "Total:\n");

  today = traffic.out_today.irc + traffic.out_today.botnet +
          traffic.out_today.partyline + traffic.out_today.transfer +
          traffic.out_today.filesys + traffic.out_today.unknown;
  total = today + traffic.out_total.irc + traffic.out_total.botnet +
          traffic.out_total.partyline + traffic.out_total.transfer +
          traffic.out_total.filesys + traffic.out_total.unknown;
  dprintf(idx, "  Out: %s (%s today)\n", btos(total), btos(today));


  today = traffic.in_today.irc + traffic.in_today.botnet +
          traffic.in_today.partyline + traffic.in_today.transfer +
          traffic.in_today.filesys + traffic.in_today.unknown;
  total = today + traffic.in_total.irc + traffic.in_total.botnet +
          traffic.in_total.partyline + traffic.in_total.transfer +
          traffic.in_total.filesys + traffic.in_total.unknown;
  dprintf(idx, "   In: %s (%s today)\n", btos(total), btos(today));

  putlog(LOG_CMDS, "*", "#%s# traffic", dcc[idx].nick);
}

static int tcl_traffic(ClientData cd, Tcl_Interp *irp,
                       int argc, char *argv[])
{
  unsigned long in_today, in_total, out_today, out_total;
  char buf[1024];

  /* IRC traffic */
  sprintf(buf, "irc %ld %ld %ld %ld", traffic.in_today.irc,
          (traffic.in_total.irc + traffic.in_today.irc), traffic.out_today.irc,
          (traffic.out_total.irc + traffic.out_today.irc));
  Tcl_AppendElement(irp, buf);

  /* Botnet traffic */
  sprintf(buf, "botnet %ld %ld %ld %ld", traffic.in_today.botnet,
          (traffic.in_total.botnet + traffic.in_today.botnet),
          traffic.out_today.botnet, (traffic.out_total.botnet +
          traffic.out_today.botnet));
  Tcl_AppendElement(irp, buf);

  /* Partyline traffic */
  sprintf(buf, "partyline %ld %ld %ld %ld", traffic.in_today.partyline,
          (traffic.in_total.partyline + traffic.in_today.partyline),
          traffic.out_today.partyline, (traffic.out_total.partyline +
          traffic.out_today.partyline));
  Tcl_AppendElement(irp, buf);

  /* Filesys.mod traffic */
  sprintf(buf, "filesys %ld %ld %ld %ld", traffic.in_today.filesys,
          (traffic.in_total.filesys + traffic.in_today.filesys),
          traffic.out_today.filesys, (traffic.out_total.filesys +
          traffic.out_today.filesys));
  Tcl_AppendElement(irp, buf);


  /* Misc traffic */
  sprintf(buf, "misc %ld %ld %ld %ld", traffic.in_today.unknown,
          (traffic.in_total.unknown + traffic.in_today.unknown),
          traffic.out_today.unknown, (traffic.out_total.unknown +
          traffic.out_today.unknown));
  Tcl_AppendElement(irp, buf);


  /* Totals */
  in_today  = traffic.in_today.irc + traffic.in_today.botnet +
              traffic.in_today.partyline + traffic.in_today.transfer +
              traffic.in_today.filesys + traffic.in_today.unknown;
  in_total  = in_today + traffic.in_total.irc + traffic.in_total.botnet +
              traffic.in_total.partyline + traffic.in_total.transfer +
              traffic.in_total.filesys + traffic.in_total.unknown;
  out_today = traffic.out_today.irc + traffic.out_today.botnet +
              traffic.out_today.partyline + traffic.out_today.transfer +
              traffic.out_today.filesys + traffic.out_today.unknown;
  out_total = out_today + traffic.out_total.irc + traffic.out_total.botnet +
              traffic.out_total.partyline + traffic.out_total.transfer +
              traffic.out_total.filesys + traffic.out_total.unknown;
  sprintf(buf, "total %ld %ld %ld %ld", in_today, in_total,
          out_today, out_total);
  Tcl_AppendElement(irp, buf);

  return TCL_OK;
}

cmd_t traffic_dcc[] = {
  {"traffic", "m|m", (Function) cmd_traffic, NULL},
  {NULL,      NULL,  NULL,                   NULL}
};

tcl_cmds traffic_tcl[] = {
  {"traffic", tcl_traffic},
  {NULL,      NULL}
};

void traffic_init()
{
    add_tcl_commands(traffic_tcl);
}
