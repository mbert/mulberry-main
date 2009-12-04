/*
 * fake KClient.h for unix for kweb project
 * by Aaron Wohl / n3liw+@cmu.edu
 * $Header: /users/dev/CVS/Plug-ins/Kerberos/CommonKClient/win32_kclient/win32KClientPublic.h,v 1.1.1.1 2000/03/25 00:18:31 daboo Exp $
 */

#ifndef KCLIENT_H
#define KCLIENT_H

#include "syskrb.h"
#include <des.h>

struct KClientSessionInfo_R {
  des_cblock sessionKey;
  Key_schedule schedule;
};
typedef struct KClientSessionInfo_R KClientSessionInfo;

/*
 * return the logged in userid if logged in
 */
int KClientGetUserName(char *dst);

/*
 * call into des ecb_encrypt
 */
/* created by n3liw+@cmu.edu to support SASL, need to be able to specify checksum */
int KClient_des_ecb_encrypt(KClientSessionInfo  *session,des_cblock v1,des_cblock v2,int do_encrypt);

/*
 * call into des pcbc_encrypt
 */
/* created by n3liw+@cmu.edu to support SASL, need to be able to specify checksum */
int KClient_des_pcbc_encrypt(KClientSessionInfo  *session,des_cblock v1,des_cblock v2,long len,int do_encrypt);

/* created by n3liw+@cmu.edu to support SASL, need to be able to specify checksum */
int KClientGetTicketForServiceFull(KClientSessionInfo *session, char *service,void *buf,unsigned long *buflen,long cks);

/*
 * create a new session
 */
int KClientNewSession(KClientSessionInfo *session, unsigned long lAddr,unsigned short lPort,unsigned long fAddr,unsigned short fPort);

/*
 * destroy a previously created session
 */ 
int KClientDisposeSession(KClientSessionInfo  *session);

/*
 * get a ticket for service (principal.instance@realm)
 */
int KClientGetTicketForService(KClientSessionInfo *session, char *service,void *buf,unsigned long *buflen);

/*
 * incrypt buf input into encryptBuf output
 * will be up to twelve bytes longer due to authenticator stuff
 */
int KClientEncrypt(KClientSessionInfo *session, void *buf,unsigned long buflen,void *encryptBuf,unsigned long *encryptLength);

#endif
