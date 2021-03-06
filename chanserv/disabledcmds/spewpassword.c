/* Automatically generated by refactor.pl.
 *
 *
 * CMDNAME: spewpassword
 * CMDALIASES: spewpass
 * CMDLEVEL: QCMD_OPER
 * CMDARGS: 2
 * CMDDESC: Search for a password in the database.
 * CMDFUNC: csu_dospewpass
 * CMDPROTO: int csu_dospewpass(void *source, int cargc, char **cargv);
 * CMDHELP: Usage: @UCOMMAND@ <pattern> <reason>
 * CMDHELP: Displays all users with the specified password.
 */

#include "../chanserv.h"
#include "../../lib/irc_string.h"
#include <stdio.h>
#include <string.h>

int csu_dospewpass(void *source, int cargc, char **cargv) {
  nick *sender=source;
  reguser *rup=getreguserfromnick(sender);
  reguser *dbrup;
  int i;
  unsigned int count=0;
  char *reason;

  if (!rup)
    return CMD_ERROR;
  
  if (cargc < 2) {
    chanservstdmessage(sender, QM_NOTENOUGHPARAMS, "spewpassword");
    return CMD_ERROR;
  }

  reason = cargv[1];
  if(!checkreason(sender, reason))
    return CMD_ERROR;

  cs_log(sender, "SPEWPASSWORD %s (reason: %s)", cargv[0], reason);
  chanservwallmessage("%s (%s) using SPEWPASSWORD (see log for password), reason: %s", sender->nick, rup->username, reason);
  
  chanservstdmessage(sender, QM_SPEWHEADER);
  for (i=0;i<REGUSERHASHSIZE;i++) {
    for (dbrup=regusernicktable[i]; dbrup; dbrup=dbrup->nextbyname) {
      if (!UHasStaffPriv(dbrup) && !strcmp(cargv[0], dbrup->password)) {
        chanservsendmessage(sender, "%-15s %-10s %-30s %s", dbrup->username, UHasSuspension(dbrup)?"yes":"no", dbrup->email?dbrup->email->content:"none set", dbrup->lastuserhost?dbrup->lastuserhost->content:"none");
        count++;
        if (count >= 2000) {
          chanservstdmessage(sender, QM_TOOMANYRESULTS, 2000, "users");
          return CMD_ERROR;
        }
      }
    }
  }
  chanservstdmessage(sender, QM_RESULTCOUNT, count, "user", (count==1)?"":"s");
  
  return CMD_OK;
}
