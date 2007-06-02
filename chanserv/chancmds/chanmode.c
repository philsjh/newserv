/* Automatically generated by refactor.pl.
 *
 *
 * CMDNAME: chanmode
 * CMDLEVEL: QCMD_AUTHED
 * CMDARGS: 4
 * CMDDESC: Shows which modes are forced or denied on a channel.
 * CMDFUNC: csc_dochanmode
 * CMDPROTO: int csc_dochanmode(void *source, int cargc, char **cargv);
 */

#include "../chanserv.h"
#include "../../nick/nick.h"
#include "../../lib/flags.h"
#include "../../lib/irc_string.h"
#include "../../channel/channel.h"
#include "../../parser/parser.h"
#include "../../irc/irc.h"
#include "../../localuser/localuserchannel.h"
#include <string.h>
#include <stdio.h>

char *getchanmode(regchan *rcp) {
  static char buf1[50];
  char buf2[30];

  if (rcp->forcemodes) {
    strcpy(buf1,printflags(rcp->forcemodes, cmodeflags));
  } else {
    buf1[0]='\0';
  }

  strcpy(buf2,printflagdiff(CHANMODE_ALL, ~(rcp->denymodes), cmodeflags));
  strcat(buf1, buf2);

  if (rcp->forcemodes & CHANMODE_LIMIT) {
    sprintf(buf2, " %d",rcp->limit);
    strcat(buf1, buf2);
  }

  if (rcp->forcemodes & CHANMODE_KEY) {
    sprintf(buf2, " %s",rcp->key->content);
    strcat(buf1, buf2);
  }

  if (*buf1=='\0') {
    strcpy(buf1,"(none)");
  }

  return buf1;
}

int csc_dochanmode(void *source, int cargc, char **cargv) {
  regchan *rcp;
  nick *sender=source;
  chanindex *cip;
  flag_t forceflags,denyflags;
  char buf1[60];
  int carg=2,limdone=0;
  sstring *newkey=NULL;
  unsigned int newlim=0;

  if (cargc<1) {
    chanservstdmessage(sender,QM_NOTENOUGHPARAMS,"chanmode");
    return CMD_ERROR;
  }

  if (!(cip=cs_checkaccess(sender, cargv[0], CA_OPPRIV, 
			   NULL, "chanmode", QPRIV_VIEWCHANMODES, 0)))
    return CMD_ERROR;
  
  rcp=cip->exts[chanservext];

  if (cargc>1) {
    if (!cs_checkaccess(sender, NULL, CA_MASTERPRIV,
			cip, "chanmode", QPRIV_CHANGECHANMODES, 0))
      return CMD_ERROR;

    /* Save the current modes.. */
    strcpy(buf1,getchanmode(rcp));

    /* Pick out the + flags: start from 0 */
    forceflags=0;
    setflags(&forceflags, CHANMODE_ALL, cargv[1], cmodeflags, REJECT_NONE);    

    /* Pick out the - flags: start from everything and invert afterwards.. */
    denyflags=CHANMODE_ALL;
    setflags(&denyflags, CHANMODE_ALL, cargv[1], cmodeflags, REJECT_NONE);
    denyflags = (~denyflags) & CHANMODE_ALL;

    forceflags &= ~denyflags; /* Can't force and deny the same mode (shouldn't be possible anyway) */
    if (forceflags & CHANMODE_SECRET) {
      forceflags &= ~CHANMODE_PRIVATE;
      denyflags |= CHANMODE_PRIVATE;
    }
    if (forceflags & CHANMODE_PRIVATE) {
      forceflags &= ~CHANMODE_SECRET;
      denyflags |= CHANMODE_SECRET;
    }

    if ((forceflags & CHANMODE_LIMIT) && 
	(!(forceflags & CHANMODE_KEY) || strrchr(cargv[1],'l') < strrchr(cargv[1],'k'))) {
      if (cargc<=carg) {
	chanservstdmessage(sender,QM_NOTENOUGHPARAMS,"chanmode");
	return CMD_ERROR;
      }
      newlim=strtol(cargv[carg++],NULL,10);
      limdone=1;
    }

    if (forceflags & CHANMODE_KEY) {
      if (cargc<=carg) {
	chanservstdmessage(sender,QM_NOTENOUGHPARAMS,"chanmode");
	return CMD_ERROR;
      }
      newkey=getsstring(cargv[carg++], KEYLEN);
    }

    if ((forceflags & CHANMODE_LIMIT) && !limdone) {
      if (cargc<=carg) {
	chanservstdmessage(sender,QM_NOTENOUGHPARAMS,"chanmode");
	return CMD_ERROR;
      }
      newlim=strtol(cargv[carg++],NULL,10);
      limdone=1;
    }

    if (CIsAutoLimit(rcp)) {
      forceflags |= CHANMODE_LIMIT;
      denyflags &= ~CHANMODE_LIMIT;
      newlim=rcp->limit;
    }

    /* It parsed OK, so update the structure.. */
    rcp->forcemodes=forceflags;
    rcp->denymodes=denyflags;      
    if (rcp->key)
      freesstring(rcp->key);
    rcp->key=newkey;
    rcp->limit=newlim;
    
    chanservstdmessage(sender, QM_DONE);
    cs_log(sender,"CHANMODE %s %s (%s -> %s)",cip->name->content,cargv[1],buf1,getchanmode(rcp));
    csdb_updatechannel(rcp);
    cs_checkchanmodes(cip->channel);    
  }
  
  chanservstdmessage(sender,QM_CURFORCEMODES,cip->name->content,getchanmode(rcp));

  return CMD_OK;
}