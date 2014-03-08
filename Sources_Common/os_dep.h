/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// Header for common OS specific definitions

#ifndef __OS_DEP__MULBERRY__
#define __OS_DEP__MULBERRY__

// Trigger error if __dest_os is not defined
#if __dest_os != __mac_os && __dest_os != __mac_os_x && __dest_os != __win32_os && __dest_os != __linux_os
#error __dest_os
#endif

// Byte-order (endian-ness) determination
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#if defined(__GNUC__)
# if defined(__ppc__)
#  define big_endian
# elif defined(__i386__)
#  undef big_endian
# endif
#else
#  define big_endian
#endif
#elif __dest_os == __win32_os
# undef big_endian
#elif __dest_os == __linux_os
#  if defined (sparc) || defined(powerpc)
#   define big_endian
#  elif defined(__i386__)
#   undef big_endian
#  elif defined(__x86_64__)
#   undef big_endian
#  else
#   error unknown endian
#  endif

#else
	// Other processors must define one of the above
#error __dest_os
#endif

#if __dest_os == __linux_os
#define LINUX_NOTYET
#define nil NULL
#endif

// In MSL 7, errno on Mac OS comes back with posix Mac error code
// The actual OS error code is in a new variable which we want
#if __dest_os == __mac_os || __dest_os == __win32_os
	#if __MSL__ >= 0x7001
		#if __dest_os == __mac_os
			#define os_errno	__MacOSErrNo
		#elif __dest_os == __win32_os
			#define os_errno	_doserrno
		#endif
	#else
		#define os_errno	errno
	#endif
#else
	#define os_errno	errno
#endif

// How to include stat.h
#if __dest_os == __mac_os
#define OSSTAT stat
#define OSSTATSTRUCT stat
#define __stat_header <stat.h>
#elif __dest_os == __win32_os
#define OSSTAT _stat
#define OSSTATSTRUCT _stat
#define S_IRWXU	0x0e00
#define S_ISDIR(m)	(((m)&(S_IFMT)) == (S_IFDIR))
#define S_ISREG(m)	(((m)&(S_IFMT)) == (S_IFREG))
#define __stat_header <sys/stat.h>
#elif __dest_os == __linux_os || __dest_os == __mac_os_x
#define OSSTAT stat
#define OSSTATSTRUCT stat
#define __stat_header <sys/stat.h>
#else
#error __dest_os
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __win32_os
#define __use_speech
#endif

#if __dest_os == __mac_os
//	Not defined on Mac so need these here
#define htonl(hostlong)		((unsigned long)(hostlong))
#define htons(hostshort)	((unsigned short)(hostshort))
#define ntohl(netlong)		((unsigned long)(netlong))
#define ntohs(netshort)		((unsigned short)(netshort))
#endif

// unichar_t support
#if __dest_os == __mac_os || __dest_os == __mac_os_x
typedef unsigned short unichar_t;
#elif __dest_os == __win32_os
typedef wchar_t unichar_t;
#else
typedef unsigned short unichar_t;
#endif

// Frameworks
#include "framework.h"

// Line end enum
enum EEndl
{
	eEndl_Auto = 0,
	eEndl_CR,
	eEndl_LF,
	eEndl_CRLF,
	eEndl_Any
};

// Strings containing OS and Network line-end chars
extern const char* os_endl;
extern const unichar_t* os_uendl;
extern unsigned long os_endl_len;
extern const char* os_endl2;
extern const unichar_t* os_uendl2;
extern const char* net_endl;
extern unsigned long net_endl_len;
extern const char* net_endl2;

const char* get_endl(EEndl endl);
const unichar_t* get_uendl(EEndl endl);
unsigned long get_endl_len(EEndl endl);

// Macros for line end types: '\r', '\n', '\r\n'
#define __cr	1
#define __lf	2
#define __crlf	3

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#define __line_end	__cr

const char lendl1 = '\r';
const char lendl2 = '\r';
const char lendl3 = '\n';

const EEndl lendl = eEndl_CR;

#elif __dest_os == __win32_os
#define __line_end	__crlf

const char lendl1 = '\r';
const char lendl2 = '\n';

const EEndl lendl = eEndl_CRLF;

#else
#define __line_end	__lf

const char lendl1 = '\n';
const char lendl2 = '\n';
const char lendl3 = '\r';

const EEndl lendl = eEndl_LF;

#endif

#if __dest_os == __mac_os
const char os_dir_delim = ':';
#elif __dest_os == __win32_os
const char os_dir_delim = '\\';
#elif __dest_os == __linux_os || __dest_os == __mac_os_x
const char os_dir_delim = '/';
#endif
const char os_dir_escape = '%';

#endif
