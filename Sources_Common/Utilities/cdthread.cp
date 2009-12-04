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


// cdthread : class to do threaded mail checking

#include "cdthread.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include <LSimpleThread.h>
#endif

#if __dest_os == __linux_os
#include <ace/Thread.h>
#endif

cdthread::cdthread()
{
	mThread = 0L;
	mRepeating = false;
	mRunning = false;
	mFailed = false;
	mSleep = 1;
	mExit = false;
}

cdthread::~cdthread()
{
	// Set flag to force thread loop to terminate
	SetExit();

	// Try to acquire exit lock which is released only when thread exits
	// Use of mutex to allow messages to be pumped while waiting for exit
	GetExitLock().acquire();

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// No need to delete: thread deletes itself on exit from thread proc
#elif __dest_os == __win32_os
	// No need to delete: thread deletes itself on exit from thread proc (MFC 4.2.1)
	//delete sThread;
#elif __dest_os == __linux_os
	// No need to delete: thread deletes itself on exit from thread proc
#else
#error __dest_os
#endif
	mThread = 0L;
}

// Get current thread id
int cdthread::current_tid()
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	unsigned long this_tid = 0;
	::GetCurrentThread(&this_tid);
#elif __dest_os == __win32_os
	int this_tid = ::GetCurrentThreadId();
#elif __dest_os == __linux_os
	int this_tid = ACE_Thread::self();
#endif

	return this_tid;
}

// Thread entry point
#if __dest_os == __mac_os || __dest_os == __mac_os_x
void cdthread::ThreadRun(cthread& thread, void* pParam)
#elif __dest_os == __win32_os
UINT cdthread::ThreadRun(LPVOID pParam)
#elif __dest_os == __linux_os
void* cdthread::ThreadRun(void* pParam)
#else
#error __dest_os
#endif
{
	// Get pointer to owner
	cdthread* owner = static_cast<cdthread*>(pParam);

	// Grab exit lock
	cdmutex::lock_cdmutex _lock(owner->GetExitLock());

	// Must catch all errors - cannot throw out of thread!
	try
	{
		if (owner->GetRepeating())
		{
			// Loop while waiting for exit
			while(!owner->GetExit())
			{
				// See if it wants time
				if (owner->GetRunning())
					// Execute it
					owner->Execute();
				else
					// Wait for it to wake up
#if __dest_os == __mac_os || __dest_os == __mac_os_x
					thread.Sleep(owner->GetSleep());
#elif __dest_os == __win32_os
					::Sleep(owner->GetSleep());
#elif __dest_os == __linux_os
					usleep(owner->GetSleep());
#else
#error __dest_os
#endif
			}
		}
		else
		{
			// Execute it once
			owner->Execute();

			// Make sure running flag is reset
			owner->StopExecution();
		}
	}
	catch(...)
	{
	}

	// Thread always deletes itself therefore set its pointer to NULL in the owner
	// to allow owner to recreate thread if required to run again
	owner->ClearThread();

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#elif __dest_os == __win32_os
	return 0;
#elif __dest_os == __linux_os
	return 0;
#else
#error __dest_os
#endif
}

// NB This will be called from outside the thread
void cdthread::StartExecution()
{
	// WARNING: This must not be called while a thread is terminating since mThread
	// may appear to be valid but it will be deleted when the thread completes
	// Therefore do not start execution if thread is about to exit
	if (GetExit())
		return;

	// Create the thread if not already done
	if (!mThread)
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Start suspended so that mThread is allocated before we call the entry point
		mThread = new LSimpleThread(ThreadRun, this);
		if (mThread != NULL)
			mThread->Resume();
#elif __dest_os == __win32_os
		// Start suspended so that mThread is allocated before we call the entry point
		mThread = ::AfxBeginThread(ThreadRun, this, THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED);
		if (mThread != NULL)
			mThread->ResumeThread();
#elif __dest_os == __linux_os
		ACE_Thread::spawn((ACE_THR_FUNC)ThreadRun, this,
//											THR_NEW_LWP | THR_JOINABLE,
											THR_NEW_LWP | THR_DETACHED,
											NULL, &mThread);
#else
#error __dest_os
#endif
	}

	// Initialise state
	SetFailed(false);

	// Now start the thread by setting it running
	SetRunning(true);
}

// NB This will generally be called from within the thread itself
void cdthread::StopExecution()
{
	// Stop it
	SetRunning(false);
}
