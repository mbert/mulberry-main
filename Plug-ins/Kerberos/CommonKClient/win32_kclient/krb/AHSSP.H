/*

Copyright © 1996 by Project Mandarin, Inc.

*/

/* ******************* symbolic constants ******************* */
#define  MAXTRACELEN    128
#define 	MAXTYPE         12
#define  MAXPORT          8
#define  MAXBUFF        256
#define 	MAXSEND   		256			// maximum bytes to send() for strings
#define 	MAXFILESEND    512			// maximum bytes to send() files
#define  MAXMSG        4096			// maximum size of a ahssp msg

// for Trace()ing to an application window
#define YYZ_TRACE		WM_USER+25

/* ******************* typdefs ******************* */
typedef struct TransActionContext
	{
	char szType[MAXTYPE] ;			// protocol associated with this TC
	SOCKET socket ;					// actual socket descriptor
	DWORD dwBytes ;					// number of bytes received (progress meter)
	SOCKADDR_IN r_asso ;				// remote association for this connection
	BOOL connect ;						// TRUE if connected, FALSE otherwise
	char szAlias[MAXBUFF] ;			// alias
	char szService[MAXBUFF] ;		// service
	char szHost[MAXBUFF] ;			// IP or hostname
	WORD iPort ;						// port
	char ticket[2048] ;				// authenticator?
	DWORD iTicketLen ;				// length of ticket
	} TC ;

typedef TC FAR * LPTC ;				// TransActionContext Pointer
