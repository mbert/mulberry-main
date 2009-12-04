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


// cdmutex : mutex to halt thread until yield

#ifndef __CDMUTEX__MULBERRY__
#define __CDMUTEX__MULBERRY__

#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __mac_os || __dest_os == __mac_os_x_x
#include <Threads.h>
#elif __dest_os == __linux_os
#include <ace/Synch.h>
#include <ace/OS.h>
#include <vector>
#endif

#include <set>

class CBusyContext;

class cdmutex
{
public:
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	typedef ThreadID	cdthread_t;
	typedef long		cdmutex_t;
#elif __dest_os == __win32_os
	typedef DWORD				cdthread_t;
	typedef HANDLE				cdmutex_t;
#elif __dest_os == __linux_os
	typedef ACE_Thread_ID cdthread_t;
	typedef ACE_Mutex cdmutex_t;
#endif

	// Stack class to do locks
	class lock_cdmutex
	{
	public:
		lock_cdmutex(cdmutex& lock_it)
			: mLockIt(lock_it)
			{ mLockIt.acquire(); }
		~lock_cdmutex()
			{ mLockIt.release(); }
	private:
		cdmutex& mLockIt;
	};

	class try_cdmutex
	{
	public:
		try_cdmutex(cdmutex& lock_it)
			: mLockIt(lock_it)
			{ have_lock = mLockIt.try_lock(); }
		~try_cdmutex()
			{ if (have_lock) mLockIt.release(); }
		
		bool GotLock() const
			{ return have_lock; }

	private:
		cdmutex& mLockIt;
		bool have_lock;
	};

	cdmutex();
	~cdmutex();
	
	int remove();				// explicit removal
	int acquire();				// acquire lock
	int release();				// release lock
	bool try_lock();			// try lock but don't wait
	bool is_locked() const		// is it locked
		{ return _count; }
	bool is_locked_by_current_thread() const;		// is it locked by the current thread

	static void yield(CBusyContext* busy);		// yield while waiting for mutex lock to be released by another thread
	static cdthread_t current_tid();	// Get current thread id

private:
	unsigned long _count;
	cdthread_t _tid;
	cdmutex_t _lock;
};

class cdrwmutex
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	typedef ThreadID	cdthread_t;
	typedef long		cdmutex_t;
#elif __dest_os == __win32_os
	typedef DWORD				cdthread_t;
	typedef CRITICAL_SECTION	cdmutex_t;
#elif __dest_os == __linux_os
	typedef ACE_Thread_ID cdthread_t;
	typedef ACE_Mutex cdmutex_t;
#endif
#if __dest_os == __linux_os
	typedef std::vector<cdthread_t> cdthreadset;
#else
	typedef std::multiset<cdthread_t> cdthreadset;
#endif
public:
	// Stack class to do locks
	class read_lock_cdrwmutex
	{
	public:
		read_lock_cdrwmutex(cdrwmutex& lock_it)
			: mLockIt(lock_it)
			{ mLockIt.read_acquire(); }
		~read_lock_cdrwmutex()
			{ mLockIt.read_release(); }
	private:
		cdrwmutex& mLockIt;
	};

	class write_lock_cdrwmutex
	{
	public:
		write_lock_cdrwmutex(cdrwmutex& lock_it)
			: mLockIt(lock_it)
			{ mLockIt.write_acquire(); }
		~write_lock_cdrwmutex()
			{ mLockIt.write_release(); }
	private:
		cdrwmutex& mLockIt;
	};

	cdrwmutex();
	~cdrwmutex();
	
	int remove();				// explicit removal
	int read_acquire();			// acquire read lock
	int write_acquire();		// acquire write lock
	int read_release();			// release read lock
	int write_release();		// release write lock
	bool try_read_lock();		// try read lock but don't wait
	bool try_write_lock();		// try write lock but don't wait
	bool is_read_locked() const		// is it locked
		{ return _read_tids.size(); }
	bool is_write_locked() const		// is it locked
		{ return _write_count; }

private:
	unsigned long _write_count;
	cdthread_t _write_tid;
	cdthreadset _read_tids;
	cdmutex_t _lock;
};

#endif
