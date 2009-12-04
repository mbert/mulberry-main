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


#ifndef __CTASKQUEUE__MULBERRY__
#define __CTASKQUEUE__MULBERRY__

#include "cdmutex.h"
#include "cdcond.h"

#include <queue>

#if __dest_os == __linux_os
#include <ace/Thread.h>
#endif

// Queue to hold tasks and run at idle time
// Application must call DoNextWork at idle time
class CTaskBase;
class CTaskQueue
{
public:
	static CTaskQueue sTaskQueue;

#if __dest_os == __linux_os
	CTaskQueue(): mPause(false), mMainThreadID(ACE_Thread::self()) {}
#else
	CTaskQueue() {}
#endif
	void Enqueue(CTaskBase* task);
	bool DoNextWork();
	
	bool InMainThread() const;

	void Pause(bool pause)
		{ mPause = pause; }

protected:
	std::queue<CTaskBase*> mTaskQueue;
	bool mPause;
	cdmutex _mutex;
#if __dest_os == __linux_os
	ACE_thread_t mMainThreadID;
#endif

	CTaskBase* Dequeue();
};

class CTaskBase
{
public:
	friend class CTaskQueue;

	// These must be created off the heap, using new, as they will be deleted
	// elsewhere
	CTaskBase() : mResult(0) {}

	// If in the main thread will do work, otherwise will put it on CTaskQueue
	// In either case, the creator no longer owns the memory
	virtual unsigned long Go();

protected:
	unsigned long mResult;

	// Make this protected so we cannot create one on the stack
	// Tasks must always be created on the heap with operator new
	virtual ~CTaskBase() {}

	// Does the actual work.  Should not be overridden unless you want to change
	// the semantics of how work is called.  This is used for Sync vs Async for
	// example
	virtual void DoWork();

	// Override with whatever work needs to be done
	virtual void Work() = 0;

};

// Does a task asynchronously (at main thread idle time)
class CTaskAsync : public CTaskBase
{
public:
	CTaskAsync() {}
};

class CTaskAlwaysQueued : public CTaskBase
{
public:
	CTaskAlwaysQueued() {}
	virtual unsigned long Go();
};

// Does a task synchronously (at main thread idle time)
class CTaskSync : public CTaskAsync
{
public:
	CTaskSync() : _working(false) {}
	virtual unsigned long Go();

protected:
	bool	_working;
	cdcond	_done;

	virtual void DoWork();
};

#endif
