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


// cdthread : pure virtual base class that implements a thread

#ifndef __CDTHREAD__MULBERRY__
#define __CDTHREAD__MULBERRY__

#include "cdmutex.h"
#include <vector>

#if __dest_os == __mac_os || __dest_os == __mac_os_x
class LThread;
#endif

class cdthread
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
	cdthread();
	virtual ~cdthread();

	static int current_tid();					// Get current thread id

	void SetRepeating(bool repeating)
		{ mRepeating = repeating; }
	bool GetRepeating() const
		{ return mRepeating; }

	void SetRunning(bool running)
		{ mRunning = running; }
	bool GetRunning() const
		{ return mRunning; }

	void SetFailed(bool failed)
		{ mFailed = failed; }
	bool GetFailed() const
		{ return mFailed; }

	void SetSleep(long sleep)
		{ mSleep = sleep; }
	long GetSleep() const
		{ return mSleep; }

	void SetExit()
		{ mExit = true; }
	bool GetExit() const
		{ return mExit; }

	cdmutex& GetExitLock()
		{ return _can_exit; }

protected:
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	static void ThreadRun(cthread& thread, void* arg);
#elif __dest_os == __win32_os
	static UINT ThreadRun(LPVOID pParam);
#elif __dest_os == __linux_os
	static void* ThreadRun(void* param);
#else
#error __dest_os
#endif
	void StartExecution();
	void StopExecution();
	virtual void Execute() = 0;					// Pure virtual: must override in derived classes

private:
#if __dest_os == __linux_os
	cthread mThread;
#else
	cthread* mThread;
#endif
	bool mRepeating;
	bool mRunning;
	bool mFailed;
	long mSleep;
	bool mExit;
	cdmutex _can_exit;

	void ClearThread()
		{ mThread = 0L; }
};

typedef std::vector<cdthread*> cdthreadvect;

#endif
