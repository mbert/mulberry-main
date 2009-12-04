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

#include "CTaskQueue.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include <LThread.h>
#endif

#pragma mark ____________________________CTaskQueue

CTaskQueue CTaskQueue::sTaskQueue;

bool CTaskQueue::DoNextWork()
{
	// Only if not paused
	if (mPause)
		return false;

	// Look for a task item
	CTaskBase* task = Dequeue();
	if (task)
	{		
		// Run the task now
		task->DoWork();

		// task will get deleted as a result of the call to DoWork

		return true;
	}
	else 
		return false;
}

void CTaskQueue::Enqueue(CTaskBase* task)
{
	// Lock access to queue
	cdmutex::lock_cdmutex _lock(_mutex);

	// Add item to queue
	mTaskQueue.push(task);
}

CTaskBase* CTaskQueue::Dequeue()
{
	// Lock access to queue
	cdmutex::lock_cdmutex _lock(_mutex);

	// See if anything in the queue
	if (mTaskQueue.empty())
		return NULL;
	else
	{
		// Get and pop the first item in the queue
		CTaskBase* task = mTaskQueue.front();
		mTaskQueue.pop();
		return task;
	}
}

bool CTaskQueue::InMainThread() const
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	return LThread::InMainThread();
#elif __dest_os == __win32_os
	return (::GetCurrentThreadId() == AfxGetApp()->m_nThreadID);
#elif __dest_os == __linux_os
	return (ACE_Thread::self() == mMainThreadID);
#endif
}

#pragma mark ____________________________CTaskBase

unsigned long CTaskBase::Go()
{
	unsigned long result = 0;
	
	// Check whether to run it now or not
	if (CTaskQueue::sTaskQueue.InMainThread())
	{
		// In main thread so run it now
		Work();
		result = mResult;
		
		// Once we're done we delete ourselves
		delete this;
	}
	else
		// Queue it to run at idle time in main thread
		CTaskQueue::sTaskQueue.Enqueue(this);

	return result;
}

void CTaskBase::DoWork()
{
	// Do work when run from queue
	Work();
	
	// Once we're done we delete ourselves
	delete this;
}

#pragma mark ____________________________CTaskAlwaysQueued

unsigned long CTaskAlwaysQueued::Go()
{
	// Always add to queue so that it is done at idle time in main thread
	// even if we are already in the main thread
	CTaskQueue::sTaskQueue.Enqueue(this);
	return 0;
}

#pragma mark ____________________________CTaskSync

unsigned long CTaskSync::Go()
{
	unsigned long result = 0;

	// Check whether to run it now or not
	if (CTaskQueue::sTaskQueue.InMainThread())
		// In main thread so run it now
		Work();
	else
	{
		// Safe to clear condition since no other thread knows about us yet
		_done.clear();
		
		// Queue it to run at idle time in main thread
		CTaskQueue::sTaskQueue.Enqueue(this);

		// Block waiting on condition to be signalled
		_done.wait(cdcond::eTimeoutInfinite);
	}

	result = mResult;
	
	// Once we're done we delete ourselves
	delete this;

	return result;
}

void CTaskSync::DoWork()
{
	// Signal that the queue is running us
	_working = true;

	// Do work when run from queue
	Work();

	// Signal that we are done - will wake child thread
	_done.signal();
}

