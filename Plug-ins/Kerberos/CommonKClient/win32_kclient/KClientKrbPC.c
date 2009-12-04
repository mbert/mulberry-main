/* 
 * KClientKrbPC.c
 * krb_get_ticket_for_service that matches unix interface for use on a pc
 */

// CD Mods:

// 09-Sep-1998: Added pname static variable which is filled in from CREDENTIALS structure after getting a ticket.
//				This is then returned as the user id when requested.

// 09-Sep-1998:	Moved code for getting ticket with checksum into new routine. Now test for failure of this
//				and do default leash implementation of get ticket to force login dialog to be displayed.
//				The checksum version is then called again.

#include <stdio.h>
#include <string.h>
#define	DEFINE_SOCKADDR
#include "syskrb.h"
#include "KClientPublic.h"
#include "KClientKrbPC.h"
#include "win32KClient.h"

// #include "KClient.h"

// CD's hack for user name
static char    pname[ANAME_SZ];    /* Principal's name */

/*
 * return the logged in userid if logged in
 */
int KClientGetUserName(char *dst)
{
	// CD's hack for user name
	strcpy(dst, pname);
	return 0;
#if 0
	// cache this ???
	return tf_get_pname(dst);
#endif
}


/* FIXME -- this should probably be calling mk_auth nowadays.  */
#define	KRB_SENDAUTH_VERS "AUTHV0.1" 	/* MUST be KRB_SENDAUTH_VLEN chars */


static int
ParseFullName(name, instance, realm, fname)
	char *name;
	char *instance;
	char *realm;
	char *fname;
{
	int err;
	
	if (!*fname) return KNAME_FMT;					/* null names are not OK */
	*instance = '\0';
	err = kname_parse(name,instance,realm,fname);
	if (err) return err;
	if (!*name) return KNAME_FMT;					/* null names are not OK */
	if (!*realm) { 
		if (err = krb_get_lrealm (realm, 1))
			return err;
		if (!*realm) return KNAME_FMT;		/* FIXME -- should give better error */
	}
	return KSUCCESS;
}

// CD's bit to ensure that login dialog is used
int cd_xxx_get_ticket_for_service(
	char *serviceName,
	char *buf,
	unsigned KRB_INT32 *buflen,
	int checksum,
	des_cblock sessionKey,
	Key_schedule schedule,
	char *version,
	int includeVersion);
int cd_xxx_get_ticket_for_service(
	char *serviceName,
	char *buf,
	unsigned KRB_INT32 *buflen,
	int checksum,
	des_cblock sessionKey,
	Key_schedule schedule,
	char *version,
	int includeVersion)
{
	char service[SNAME_SZ];
	char instance[INST_SZ];
	char realm[REALM_SZ];
	int err;
	CREDENTIALS cr;
	
	/*set dll debug*/
#ifdef RUBBISH
	SetKrbdllMode(1);
#endif

    /*
     * get a ticket in the ticket cache and return it to the user
	 */
	unsigned KRB_INT32 temp_buflen= *buflen;
   	KTEXT_ST ticket;
#ifdef RUBBISH
	// let the leash get ticket do the work of getting the tgt and cred
	if (!GetTicketForService(serviceName,buf,&temp_buflen)) {
		// int kcerr=KClientKerberosErrno();
		// int kberr=KClientErrno();
		return -1;
	}
#endif
	service[0] = '\0';
	instance[0] = '\0';
	realm[0] = '\0';

	// CD's hack for user name
	*pname = 0;

	/*
	 * now need to fetch the cred from the ticket cache to get the session key
	 */
	/* parse out service name */
	err = ParseFullName(service, instance, realm, serviceName);
	if (err)
		return err;

	// sigh, if the checksum is non zero as it is for SASL we need to redo some of the work
	// that GetTicketForService did.  This is because the version of GetTicketForService exported
	// by leash doesnt take a checksum argument.  Fortunetly the credential isn't changed by the checksum
	// only the ticket created from it.
	if((err = krb_mk_req(&ticket, service, instance, realm,checksum)) != KSUCCESS)
	  	return err;		//shouldnt happen since GetTicketForService just succeded
    if(*buflen<ticket.length)
      	return -1;
    *buflen = ticket.length + 4;
    memcpy(buf,&ticket.length,4);			//??? check byte ordering of this, not that it used...
	memcpy(buf+4,&ticket.dat,ticket.length);

	/* get the session key for later use in deciphering the server response */
 	memset(&cr,0,sizeof(cr));
	err = krb_get_cred(service,instance,realm,&cr);
	if (err)
		return err;

	// CD's hack for user name
	strncpy(pname, cr.pname, ANAME_SZ);
	pname[ANAME_SZ - 1] = 0;

	memcpy((char *)sessionKey, (char *)cr.session, sizeof(C_Block));
  	err = key_sched(sessionKey, schedule);
	if (err)
		return KFAILURE;		/* Bad DES key for some reason (FIXME better error) */
	return KSUCCESS;
}

// CD's version to try password dialog on first fail
int xxx_get_ticket_for_service(
	char *serviceName,
	char *buf,
	unsigned KRB_INT32 *buflen,
	int checksum,
	des_cblock sessionKey,
	Key_schedule schedule,
	char *version,
	int includeVersion)
{

	// Try checksum implementation
	int result = cd_xxx_get_ticket_for_service(serviceName, buf, buflen, checksum, sessionKey, schedule, version, includeVersion);
	
	// If failed try leash version
	if (result != 0)
	{
	    /*
	     * get a ticket in the ticket cache and return it to the user
		 */
		unsigned KRB_INT32 temp_buflen= *buflen;
	   	KTEXT_ST ticket;
		// let the leash get ticket do the work of getting the tgt and cred
		if (!GetTicketForService(serviceName, buf, &temp_buflen)) {
			// int kcerr=KClientKerberosErrno();
			// int kberr=KClientErrno();
			return -1;
		}
	}
	
	// Now do checksum version again, hopefully after successful login dialog
	return cd_xxx_get_ticket_for_service(serviceName, buf, buflen, checksum, sessionKey, schedule, version, includeVersion);
}
