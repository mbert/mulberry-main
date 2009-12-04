/* This file contains definitions for use by the Hesiod name service and
 * applications.
 *
 * @doc
 *
 * @module hesiod.h |
 * For copying and distribution information, see the file <mit-copyright.h>.
 *
 * Original version by Steve Dyer, IBM/Project Athena.
 *
 *      $Author: daboo $
 *      $Athena: hesiod.h,v 1.3 88/08/07 21:52:39 treese Locked $
 *      $Header: /users/dev/CVS/Plug-ins/Kerberos/CommonKClient/win32_kclient/krb/HESIOD.H,v 1.1.1.1 2000/03/25 00:18:31 daboo Exp $
 *      $Source: /users/dev/CVS/Plug-ins/Kerberos/CommonKClient/win32_kclient/krb/HESIOD.H,v $
 *      $Log: HESIOD.H,v $
 *      Revision 1.1.1.1  2000/03/25 00:18:31  daboo
 *      First Imported.
 *
 * Revision 1.6  90/07/20  13:09:16  probe
 * Incorrect declaration of hes_getpwnam()
 * 
 * Revision 1.5  90/07/11  16:49:12  probe
 * Patches from <mar>
 * Added missing declarations
 * 
 * Revision 1.5  90/07/09  18:44:30  mar
 * mention hes_getservbyname(), hes_getpwent()
 * 
 * Revision 1.4  88/08/07  23:18:00  treese
 * Second-public-distribution
 * 
 * Revision 1.3  88/08/07  21:52:39  treese
 * First public distribution
 * 
 * Revision 1.2  88/06/05  19:51:32  treese
 * Cleaned up for public distribution
 * 
 */

/* Configuration information. */

#ifndef _HESIOD_
#define _HESIOD_

#ifdef WINDOWS
#include <windows.h>
#endif

#include <decldll.h>

#if !defined(WINDOWS) && !defined(_WINDOWS) && !defined(_WIN32)
#define HESIOD_CONF     "/etc/hesiod.conf"      /* Configuration file. */
#else
#define HESIOD_CONF     "c:\\net\\tcp\\hesiod.cfg"
#endif

#define DEF_RHS         ".Athena.MIT.EDU"       /* Defaults if HESIOD_CONF */
#define DEF_LHS         ".ns"                   /*    file is not present. */

/* @doc ERROR
/* Error codes. */
/*

@type HES_ER_UNINIT   | -1      uninitialized 
@type HES_ER_OK       | 0       no error 
@type HES_ER_NOTFOUND | 1       Hesiod name not found by server 
@type HES_ER_CONFIG   | 2       local problem (no config file?) 
@type HES_ER_NET      | 3        network problem

 */

#define HES_ER_UNINIT   -1      /* uninitialized */
#define HES_ER_OK       0       /* no error */
#define HES_ER_NOTFOUND 1       /* Hesiod name not found by server */
#define HES_ER_CONFIG   2       /* local problem (no config file?) */
#define HES_ER_NET      3       /* network problem */

/* Declaration of routines */

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(WINDOWS) && !defined(_WINDOWS) && !defined(_WIN32)
char *hes_to_bind();
char **hes_resolve();
int hes_error();
#else 
#ifndef _WSHELPER_
EXPORT32 LPSTR EXPORT WINAPI hes_to_bind(LPSTR HesiodName, LPSTR HesiodNameType);
EXPORT32 LPSTR * EXPORT WINAPI hes_resolve(LPSTR HesiodName, LPSTR HesiodNameType);
EXPORT32 int EXPORT WINAPI hes_error(void);
#endif
#endif


/*
 * @doc
 *
 * @struct hes_postoffice | For use in getting post-office information.
 *
 * @field 	LPSTR   | po_type | The post office type, e.g. POP, IMAP 
 * @field 	LPSTR   | po_host | The post office host, e.g. PO10.MIT.EDU
 * @field 	LPSTR   | po_name | The account name on the post office, e.g. tom
 *
 *
 */
#if !defined(WINDOWS) && !defined(_WINDOWS)
struct hes_postoffice {
	char    *po_type;
	char    *po_host;
	char    *po_name;
};
#else
struct hes_postoffice {
	LPSTR   po_type;
	LPSTR   po_host;
	LPSTR   po_name;
};
#endif

/* Other routines */

#if !defined(WINDOWS) && !defined(_WINDOWS)
struct hes_postoffice *hes_getmailhost();
struct servent *hes_getservbyname();
struct passwd *hes_getpwnam();
struct passwd *hes_getpwuid();
#else
struct hes_postoffice FAR * WINAPI hes_getmailhost(LPSTR user);
struct servent FAR * WINAPI hes_getservbyname(LPSTR name, LPSTR proto);
struct passwd FAR * WINAPI hes_getpwnam(LPSTR nam);
struct passwd FAR * WINAPI hes_getpwuid(int uid);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _HESIOD_ */
