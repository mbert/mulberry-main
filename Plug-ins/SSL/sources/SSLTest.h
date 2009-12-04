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

#pragma once

#include <ansi_parms.h>

#if __dest_os == __mac_os
#define USE_SOCKETS
#define USE_WINSOCK
#define MACOS
#define NO_SYSLOG
#elif __dest_os == __win32_os
#define WINDOWS
#define NOCRYPT
#endif
#define MONOLITH

#if __dest_os == __mac_os
/*	Macintosh host ordering is the same as network ordering	*/
#define htonl(hostlong)		((u_long)(hostlong))
#define htons(hostshort)	((u_short)(hostshort))
#define ntohl(netlong)		((u_long)(netlong))
#define ntohs(netshort)		((u_short)(netshort))
#endif
