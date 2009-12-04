/*

Copyright © 1996 by Project Mandarin, Inc.

*/

/* ******************* Our very own fake ticket ******************* */
typedef struct fake_ticket
	{
	char	szNetID[40]	;			// Net ID
	char	szTicket[256] ;		// NetID and password, separated by a ','
	long	lTimeAcquired ;		// time ticket was acquired
	long	lTimeExpires ;			// time ticket expires
	WORD	iLifeTime ;				// lifetime of ticket
	BOOL	bAuthenticated ;		// did we pass the authentication test?
	}	FAKETICKET ;

typedef  FAKETICKET FAR * LPFAKETICKET ;
	
/* ******************* Kerberos specific typdefs ******************* */
#define	RET_TKFIL		21       /* Can't read ticket file */
#define	ANAME_SZ			40
#define	REALM_SZ       40
#define	SNAME_SZ       40
#define	INST_SZ        40
#define	MAX_KTXT_LEN 1250
#define	MAX_K_NAME_SZ	(ANAME_SZ + INST_SZ + REALM_SZ + 2)

typedef unsigned char des_cblock[8];    /* crypto-block size */
#define C_Block des_cblock

typedef struct ktext
	{
	WORD	length ;								/* Length of the text */
	unsigned char dat[MAX_KTXT_LEN] ;	/* The data itself */
	unsigned long mbz ;						/* zero to catch runaway strings */
	} KTEXT_ST ;

typedef struct ktext FAR * KTEXT ;

typedef struct credentials
	{
   char	service[ANAME_SZ] ;	/* Service name */
   char	instance[INST_SZ] ;  /* Instance */
   char	realm[REALM_SZ] ;    /* Auth domain */
   C_Block session ;          /* Session key */
   WORD	lifetime ;           /* Lifetime */
   WORD		kvno ;               /* Key version number */
   KTEXT_ST ticket_st ;       /* The ticket itself */
   long	issue_date ;         /* The issue time */
   char	pname[ANAME_SZ] ;    /* Principal's name */
   char 	pinst[INST_SZ] ;     /* Principal's instance */
	} CREDENTIALS ;

typedef CREDENTIALS FAR * LPCREDENTIALS ;

typedef struct ticketinfo
	{
	char	principal[MAX_K_NAME_SZ] ;	/* Principal name/instance/realm */
	WORD	btickets ;				 		/* Do we have tickets? */
	long	lifetime ;						/* Lifetime -- needs to have room for 255 5-minute periods * 5 * 60 */
	long	issue_date ;					/* The issue time */	                        	
	} TICKETINFO ;  



#if defined(_WIN32)
/* unfortunately the 32-bit compiler doesn't allow a function */
/* to be declared as __stdcall AND __declspec(dllexport), so  */
/* I have to use a .def file for the export part */
#define KL_CALLTYPE __stdcall
#define KL_EXPORT 
#else
#define KL_CALLTYPE WINAPI
#define KL_EXPORT _export
#endif

#if defined(_WIN32)
#define _fstrcspn  strcspn
#define _fstrnicmp  strnicmp
#define _fstrlen  strlen
#define _fstrncat strncat
#define _fstricmp stricmp
#define _fstrcmp strcmp
#define _fstrcpy strcpy
#define _fstrcat strcat
#define _fstrchr strchr
#define _fmemmove memmove
#define _fmemset memset
#define _fmemcpy memcpy
#endif
