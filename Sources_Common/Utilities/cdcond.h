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


// cdcond : condition variable to wait on

#ifndef __CDCOND__MULBERRY__
#define __CDCOND__MULBERRY__

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CEventSemaphore.h"
#elif __dest_os == __linux_os
#include <ace/Synch.h>
#endif

class cdcond
{
public:
	cdcond();
	~cdcond();
	
	enum
	{
		eCondOK = 0,
		eCondFailed = 1,
		eCondTimeout = 2
	};

	enum
	{
		eTimeoutNone = 0,
		eTimeoutInfinite = -1L
	};

	int wait(long timeout_ms);			// Wait for condition or timeout
	int signal();						// Signal the condition as done
	int clear();						// Clear condition

private:
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	CEventSemaphore	mEvent;
#elif __dest_os == __win32_os
	HANDLE			mEvent;
#elif __dest_os == __linux_os
	ACE_Event		mEvent;
#endif
};

#endif
