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


// CMailCheckThread : class to do threaded mail checking

#ifndef __CMAILCHECKTHREAD__MULBERRY__
#define __CMAILCHECKTHREAD__MULBERRY__

#include "cdmutex.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
class LThread;
#endif

class CMailCheckThread
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	typedef	LThread cthread;
#elif __dest_os == __win32_os
	typedef CWinThread cthread;
#elif __dest_os == __linux_os
	//We don't really use sThread in the Linux version, so we'll just
	//typedef this to void*
	typedef void* cthread;
#endif

public:
	static void BeginMailCheck();
	static void EndMailCheck();
	
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	static void MailCheckThread(cthread& thread, void* arg);
#elif __dest_os == __win32_os
	static UINT MailCheckThread(LPVOID pParam);
	static void BoostPriority();
#elif __dest_os == __linux_os
	static void* MailCheckThread(void* param);
#endif

	static bool IsRunning()
		{ return sRunning; }
	static cdmutex& RunLock()
		{ return _can_run; }

	static bool Pause(bool pause)
		{ bool temp = sPause; sPause = pause; return temp; }

	static void DoCheck()
		{ sDoCheck = true && !sRunning; }

private:
	static cthread* sThread;
	static bool sRunning;
	static bool sDoCheck;
	static bool sPause;
	static bool sExit;
	static cdmutex _can_run;
	static cdmutex _can_exit;

	// Always use statics
	CMailCheckThread() {}
	~CMailCheckThread() {}
};

#endif
