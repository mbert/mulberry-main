/*
 * including <krb.h> on various cpu types doesn't work
 * krb.h needs some things defined first depending on the system
 * include this to get a krb.h but not have to worry about the local
 * system magic
 */

#ifndef SYSKRB_H
#define SYSKRB_H

//#include <sys/types.h>
#ifdef UNIX
#include <netinet/in.h>
#endif

#define _INT32

#ifdef _WINDOWS
#define int32 long
#include <windows.h>
#include "conf-pc.h"
#endif

#ifdef RUBBISHX
#define ZCONST

extern int krbONE;
#define		HOST_BYTE_ORDER	(* (char *) &krbONE)
#define		MSBFIRST		0	/* 68000, IBM RT/PC */
#define		LSBFIRST		1	/* Vax, PC8086 */
#define		BITS32			1
#endif

#include <krb.h>
#endif

