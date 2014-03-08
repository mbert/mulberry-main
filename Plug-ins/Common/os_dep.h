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

// OS common information

#ifndef __OS_DEP__PLUGIN__
#define __OS_DEP__PLUGIN__

#if defined(__cplusplus)

// Generic type of OS in use (this rolls Mac OS Classic and Mac OS X into one)
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#define __build_os __mac_os
#elif __dest_os == __win32_os
#define __build_os __win32_os
#elif __dest_os == __linux_os
#define __build_os __linux_os
#else
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
#undef big_endian
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

// unichar_t support
#if __dest_os == __mac_os || __dest_os == __mac_os_x
typedef unsigned short unichar_t;
#elif __dest_os == __win32_os
typedef wchar_t unichar_t;
#else
typedef unsigned short unichar_t;
#endif

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

#if __dest_os == __mac_os
const char os_dir_delim = ':';
#elif __dest_os == __win32_os
const char os_dir_delim = '\\';
#elif __dest_os == __linux_os || __dest_os == __mac_os_x
const char os_dir_delim = '/';
#endif

#endif

#endif
