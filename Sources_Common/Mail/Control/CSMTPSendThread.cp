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

#include "CSMTPSendThread.h"

#include "CSMTPSender.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include <LSimpleThread.h>
#endif

CSMTPSendThread::CSMTPSendThread(CSMTPSender* sender) 
{
	mSMTPSender = sender;
	mRunning = false;
	mPause = false;
	mExit = false;

	// Create and allow to run immediately
	// Thread will suspend itself
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Start suspended so that mThread is allocated before we call the entry point
	mThread = new LSimpleThread(Thread, this);
	mThread->Resume();
#elif __dest_os == __win32_os
	// Start suspended so that mThread is allocated before we call the entry point
	mThread = ::AfxBeginThread(Thread, this, THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED);
	mThread->ResumeThread();
#elif __dest_os == __linux_os
	ACE_Thread::spawn((ACE_THR_FUNC)Thread, this,
										THR_NEW_LWP | THR_JOINABLE,
										NULL, &mThread);
#else
#error __dest_os
#endif
}

CSMTPSendThread::~CSMTPSendThread()
{
	// Set flag to force check loop to terminate
	mExit = true;

	// Must resume to allow it to exit
	Resume();

	// Try to acquire exit lock which is released only when mail check exits
	// Use of mutex to allow messages to be pumped while waiting for mail check to exit
	_can_exit.acquire();

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// No need to delete: thread deletes itself on exit from thread proc
	mThread = NULL;
#elif __dest_os == __win32_os
	// No need to delete: thread deletes itself on exit from thread proc (MFC 4.2.1)
	//delete sThread;
	mThread = NULL;
#elif __dest_os == __linux_os
	// No need to delete: thread never really had an "object" to go with it
	// and it will clean up itself on exit.
	mThread = 0;
#else
#error __dest_os
#endif

	mSMTPSender = NULL;
}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
void CSMTPSendThread::Thread(cthread& thread, void* pParam)
#elif __dest_os == __win32_os
UINT CSMTPSendThread::Thread(LPVOID pParam)
#elif __dest_os == __linux_os
void* CSMTPSendThread::Thread(void* pParam)
#else
#error __dest_os
#endif
{
	reinterpret_cast<CSMTPSendThread*>(pParam)->RunThread();

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#elif __dest_os == __win32_os || __dest_os == __linux_os
	return 0;
#else
#error __dest_os
#endif
}

void CSMTPSendThread::RunThread()
{
	// Grab exit lock
	cdmutex::lock_cdmutex _lock(_can_exit);

	// Start suspended
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mThread->Suspend();
#elif __dest_os == __win32_os
	mThread->SuspendThread();
#elif __dest_os == __linux_os
	suspend_.pause();
#else
#error __dest_os
#endif

	// Loop looking for exit
	while(!mExit)
	{
		{
			// Grab run lock
			cdmutex::lock_cdmutex _lock(_can_run);

			// Must catch all errors - cannot throw out of thread!
			try
			{
				mRunning = !mPause;

				// Try to start SMTP async operation
				if (!mPause && mSMTPSender->SMTPStartAsync())
				{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
					// Mac OS needs to yield
					::YieldToAnyThread();
#endif
					// Loop over more
					bool first = true;
					while(!mPause && !mExit && mSMTPSender->SMTPNextAsync(!first))
					{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
						// Mac OS needs to yield
						::YieldToAnyThread();
#endif
						first = false;
					}

					// Done with async ops
					mSMTPSender->SMTPStopAsync();
				}

				mRunning = false;
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Catch all errors to prevent throwing out of thread

				mRunning = false;
			}
		}

		// Suspend this thread until woken by external source
		if (!mExit)
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			mThread->Suspend();
#elif __dest_os == __win32_os
			mThread->SuspendThread();
#elif __dest_os == __linux_os
			suspend_.pause();
#else
#error __dest_os
#endif
	}
}

void CSMTPSendThread::Suspend()
{
	// Just set the flag and let it suspend itself
	mPause = true;
}

void CSMTPSendThread::Resume()
{
	// Set flag
	mPause = false;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	if (mThread->GetState() & LThread::threadState_Suspended)
		mThread->Resume();
#elif __dest_os == __win32_os
	mThread->ResumeThread();
#elif __dest_os == __linux_os
	suspend_.unpause();
#else
#error __dest_os
#endif
}

bool CSMTPSendThread::IsSuspended() const
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	return (mThread->GetState() & LThread::threadState_Suspended);
#elif __dest_os == __win32_os 
	return false;
#elif __dest_os == __linux_os
	return (suspend_.paused());
#else
#error __dest_os
#endif
}
