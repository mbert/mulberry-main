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


#ifndef __MAC2WIN__MULBERRY__
#define __MAC2WIN__MULBERRY__


// Types
typedef short OSErr;
typedef unsigned long OSType;
typedef char* Ptr;
typedef Ptr* Handle;

typedef unsigned char Str255[256], Str63[64], Str32[33], Str31[32], Str27[28], Str15[16];

typedef unsigned char *StringPtr, **StringHandle;

typedef const unsigned char *ConstStr255Param;

typedef ConstStr255Param ConstStr63Param, ConstStr32Param, ConstStr31Param, ConstStr27Param, ConstStr15Param;

typedef COLORREF RGBColor;

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
typedef union NumVersionVariant			NumVersionVariant;

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
	extend						= 0x40
};

enum {	// Version Release Stage Codes
	developStage				= 0x20,
	alphaStage					= 0x40,
	betaStage					= 0x60,
	finalStage					= 0x80
};

// Define

#define nil			0L
#define Boolean		BOOL
#define Rect		RECT
#define	true		1
#define false		0

#define kOK_Btn		IDOK

#endif
