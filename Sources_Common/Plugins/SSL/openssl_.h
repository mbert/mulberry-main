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


// Header for openssl defs

#ifndef __OPENSSL__MULBERRY__
#define __OPENSSL__MULBERRY__

#if __dest_os == __mac_os

#define USE_SOCKETS
#define USE_WINSOCK
#define MACOS
#define NO_SYSLOG
#define MONOLITH

#elif __dest_os == __mac_os_x

#define USE_SOCKETS
#define MACOS
#define NO_SYSLOG
#define MONOLITH

#elif __dest_os == __win32_os

#ifndef WIN32
#define WIN32
#endif
#ifndef WINDOWS
#define WINDOWS
#endif
#define NOCRYPT
#define NO_SYS_TYPES_H

// Need this for intrinsic rotate
#define _lrotl(a,n)	(((a)<<(n))|(((a)&0xffffffff)>>(32-(n))))
#define _lrotr(a,n)	(((a)>>(n))+((a)<<(32-(n))))

#endif

#undef export
#include <openssl/pem.h>
#include <openssl/ssl.h>

#endif
