/*
 * fake KClient routines
 * by Aaron Wohl / n3liw+@cmu.edu
 * $Header: /users/dev/CVS/Plug-ins/Kerberos/CommonKClient/win32_kclient/KClientKrbGlue.c,v 1.2 2000/12/08 01:25:11 daboo Exp $
 */

#include <stdio.h>
#include <wchar_t.h>
#include "KClientPublic.h"

#ifndef FALSE
#define FALSE (0==1)
#endif

/*
 * create a new session
 */
int KClientNewSession(KClientSessionInfo *session, unsigned long lAddr,unsigned short lPort,unsigned long fAddr,unsigned short fPort)
{
  return 0;
}

/*
 * destroy a previously created session
 */ 
int KClientDisposeSession(KClientSessionInfo  *session)
{
  return 0;
}

/*
 * undo packing in g_tkt_svc.c
 */
static long fetch_net_long_from_ptr(void *src)
{
  long result;
  (void)memcpy(&result,src,sizeof(result));
  result=ntohl(result);
  return result;
}

/*
 * call into des ecb_encrypt
 */
/* created by n3liw+@cmu.edu to support SASL, need to be able to specify checksum */
int KClient_des_ecb_encrypt(KClientSessionInfo  *session,des_cblock v1,des_cblock v2,int do_encrypt)
{
	Key_schedule schedule;
	memcpy(schedule,&session->schedule,sizeof(schedule));
	des_ecb_encrypt((void *)v1,(void *)v1,schedule,do_encrypt);
	return 0;
}

/*
 * call into des pcbc_encrypt
 */
/* created by n3liw+@cmu.edu to support SASL, need to be able to specify checksum */
int KClient_des_pcbc_encrypt(KClientSessionInfo  *session,des_cblock v1,des_cblock v2,long len,int do_encrypt)
{
	Key_schedule schedule;
	memcpy(schedule,&session->schedule,sizeof(schedule));
	des_pcbc_encrypt((void *)v1,(void *)v1,len,schedule,&session->sessionKey,do_encrypt);
	return 0;
}

/* created by n3liw+@cmu.edu to support SASL, need to be able to specify checksum */
int KClientGetTicketForServiceFull(KClientSessionInfo *session, char *service,void *buf,unsigned long *buflen,long cks)
{
  return xxx_get_ticket_for_service(service,buf,buflen,cks,(void*)&session->sessionKey,(void*)&session->schedule,"",FALSE);
}

/*
 * get a ticket for service (principal.instance@realm)
 */
int KClientGetTicketForService(KClientSessionInfo *session, char *service,void *buf,unsigned long *buflen)
{
  return KClientGetTicketForServiceFull(session,service,buf,buflen,0);
}

/*
 * incrypt buf input into encryptBuf output
 * will be up to twelve bytes longer due to authenticator stuff
 */
int KClientEncrypt(KClientSessionInfo *session, void *buf,unsigned long buflen,void *encryptBuf,unsigned long *encryptLength)
{
  struct sockaddr_in send_addr;
  struct sockaddr_in recv_addr;
  memset(&send_addr,0,sizeof(send_addr));
  memset(&recv_addr,0,sizeof(recv_addr));
  *encryptLength=krb_mk_priv(buf,encryptBuf,buflen,(void*)&session->schedule,(void*)&session->sessionKey,&send_addr,&recv_addr);
  return 0;
}
