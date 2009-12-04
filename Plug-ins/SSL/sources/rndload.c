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

#include <openssl/rand.h>       /* SSLeay stuff */

#define BUILD_DLL

#if __dest_os == __mac_os
/* Mac OS stuff */
#include <CodeFragments.h>
#include <Errors.h>
#include <Resources.h>
#include <Types.h>

FSSpec thisDLL;
#endif

#if __dest_os == __win32_os
#include <windows.h>

HINSTANCE thisInst;

#ifdef BUILD_DLL
BOOL __stdcall DllMain(HINSTANCE inst, DWORD fdwReason, LPVOID lpvReserved);     /* hh 971207 Added prototype */
BOOL __stdcall DllMain(HINSTANCE inst, DWORD fdwReason, LPVOID lpvReserved)
{
	thisInst = inst;
    return 1;
}
#endif
#endif

#if __dest_os != __linux_os
int RAND_poll();

int RAND_poll()
{
}
#endif

#if __dest_os == __linux_os
int RAND_load_rsrc();

int RAND_load_rsrc()
{
#if defined(sun)
#include "rndout.h"
	
	/* Add handle bytes to RAND engine */
	RAND_add(rsrc, sizeof(rsrc), sizeof(rsrc));
#endif
	return 1;

}

#endif
