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


// CSMTPSendThread : class to do threaded mail checking

#ifndef __CSMTPSENDTHREAD__MULBERRY__
#define __CSMTPSENDTHREAD__MULBERRY__

#include "cdmutex.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
class LThread;
#elif __dest_os == __linux_os
#include "CThreadPauser.h"
#endif


class CSMTPSender;

class CSMTPSendThread
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	typedef	LThread cthread;
#elif __dest_os == __win32_os
	typedef CWinThread cthread;
#elif __dest_os == __linux_os
	typedef ACE_hthread_t cthread;
#else
#error __dest_os
#endif

public:
	CSMTPSendThread(CSMTPSender* sender);
	~CSMTPSendThread();

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	static void Thread(cthread& thread, void* arg);
#elif __dest_os == __win32_os
	static UINT Thread(LPVOID pParam);
#elif __dest_os == __linux_os
	static void* Thread(void* param);
#else
#error __dest_os
#endif

	void RunThread();

	bool IsRunning()
		{ return mRunning; }

	void Suspend();
	void Resume();
	bool IsSuspended() const;
	cdmutex& GetRunLock()
		{ return _can_run; }

private:
#if __dest_os == __linux_os
	cthread mThread;
	CThreadPauser suspend_;
#else
	cthread* mThread;
#endif
	CSMTPSender* mSMTPSender;
	bool mRunning;
	bool mPause;
	bool mExit;
	cdmutex _can_run;
	cdmutex _can_exit;
};

#endif
