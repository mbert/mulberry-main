/*
    Copyright (c) 2007 Cyrus Daboo. All rights reserved.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/


#ifndef _MULBERRY_PREFIX_H
#define _MULBERRY_PREFIX_H

//The various OS's
#define __win32_os 95
#define __mac_os 2
#define __linux_os 1
#define __dest_os __linux_os


#if __dest_os == __linux_os
#define MULUNIX
#endif


#ifdef MULUNIX

#include "os_dep.h"

/* Numeric version part of 'vers' resource */
#ifdef big_endian
struct NumVersion {
	/* Numeric version part of 'vers' resource */
	unsigned char      majorRev;               /*1st part of version number in BCD*/
	unsigned char      minorAndBugRev;         /*2nd & 3rd part of version number share a byte*/
	unsigned char      stage;                  /*stage code: dev, alpha, beta, final*/
	unsigned char      nonRelRev;              /*revision level of non-released version*/
};
typedef struct NumVersion               NumVersion;
#else
struct NumVersion {
	/* Numeric version part of 'vers' resource accessable in little endian format */
	unsigned char      nonRelRev;              /*revision level of non-released version*/
	unsigned char      stage;                  /*stage code: dev, alpha, beta, final*/
	unsigned char      minorAndBugRev;         /*2nd & 3rd part of version number share a byte*/
	unsigned char      majorRev;               /*1st part of version number in BCD*/
};
typedef struct NumVersion               NumVersion;
#endif

union NumVersionVariant {
																/* NumVersionVariant is a wrapper so NumVersion can be accessed as a 32-bit value */
	NumVersion 						parts;
	unsigned long 					whole;
};

class JRect;
typedef JRect Rect;

#define kOK_Btn		CErrorHandler::Ok
#define IDOK CErrorHandler::Ok
#define IDCANCEL CErrorHandler::Cancel
//#include <ace/OS.h>

#define UNIX_SOCKETS
#define UNIX_ERRNO
#define USE_JX
#endif

	// Define all debugging symbols

#define Debug_Throw
#define Debug_Signal

#define __MULBERRY
#define __MULBERRY_V2

// Pull these from linux build to help cut down on excessive
// object code prior to link. When we finally start using gcc with
// a precompile option then we can add these back it.

//#include "cdstring.h"
//#include "templs.h"

//#include "ptrvector.h"
//#include "prefsvector.h"
//#include "CGeneralException.h"

typedef		char			SInt8;
typedef		short			SInt16;
typedef		long			SInt32;

typedef		unsigned char	UInt8;
typedef		unsigned short	UInt16;
typedef		unsigned long	UInt32;

typedef		UInt16			Char16;

typedef long ExceptionCode;

typedef short OSErr;
typedef unsigned long OSType;
class JRGB;
typedef JRGB RGBColor;


/* Date and time conversion */
struct DateTimeRec {
	short							year;
	short							month;
	short							day;
	short							hour;
	short							minute;
	short							second;
	short							dayOfWeek;
};
typedef struct DateTimeRec DateTimeRec;

// Constants

enum {	// Error codes
	noErr						= 0,
	readErr						= -19,							/*I/O System Errors*/
	writErr						= -20,							/*I/O System Errors*/
	eofErr						= -39							/*End of file*/
};

enum {	// FONT styles
	normal						= 0,
	bold						= 1,
	italic						= 2,
	underline					= 4,
	outline						= 8,
	shadow						= 0x10,
	condense					= 0x20,
	extend						= 0x40,
	strike            = 0x80
};

enum {	// Version Release Stage Codes
	developStage				= 0x20,
	alphaStage					= 0x40,
	betaStage					= 0x60,
	finalStage					= 0x80
};

typedef unsigned long ResIDT;

#define msg_OK true

#include "resource.h"
#include "resource_unix.h"
#endif
