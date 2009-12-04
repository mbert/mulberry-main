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


// cdmutex.cp - handles thread locking

#include "cdmutex.h"

#include "CGeneralException.h"
#include "CLog.h"

#include "CMailControl.h"

#include <algorithm>
#include <time.h>

const unsigned long aquire_timeout_s = 30;	// 30 second aquire timeout
const unsigned long aquire_timeout_ms = aquire_timeout_s * 1000;

#pragma mark ____________________________cdmutex

cdmutex::cdmutex()
#if __dest_os == __linux_os
	: _tid(0,0)
#endif
{
	_count = 0;
#if __dest_os != __linux_os
	_tid = 0;
#endif
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	_lock = 0;
#elif __dest_os == __win32_os
	_lock = ::CreateMutex(NULL, false, NULL);
#else
	// _lock & _tid constructed already
#endif
}

cdmutex::~cdmutex()
{
    remove();
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#elif __dest_os == __win32_os
	::CloseHandle(_lock);
#else
	// _lock object auto-destructs
#endif
}

// Get current thread id
cdmutex::cdthread_t cdmutex::current_tid()
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	cdthread_t this_tid = 0;
	::GetCurrentThread(&this_tid);
#elif __dest_os == __win32_os
	cdthread_t this_tid = 0;
    this_tid = ::GetCurrentThreadId();
#elif __dest_os == __linux_os
	cdthread_t this_tid(0,0);
	ACE_thread_t this_t = ACE_Thread::self();
	ACE_hthread_t this_th;
	ACE_Thread::self(this_th);
	this_tid = ACE_Thread_ID(this_t, this_th);
#endif

	return this_tid;
}

int cdmutex::remove()
{
	_count = 0;
#if __dest_os != __linux_os
	_tid = 0;
#else
	_tid = ACE_Thread_ID(0,0);
#endif
    return 0;
}

#if __dest_os == __win32_os
BOOL OnMessagePendingMutex();
BOOL OnMessagePendingMutex()
{
#if 0
	MSG msg;

	// eat all mouse messages in our queue
	while (::PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) {}

	// eat all keyboard messages in our queue
	while (::PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)) {}

	// Process all others
	if (::PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
		return FALSE;   // usually return TRUE, but OnIdle usually causes WM_PAINTs
	}
	return FALSE;
#else
	MSG msg;
	// Must allow socket messages through
#define	WM_TCP_ASYNCDNR	(WM_USER + 2)
	if (::PeekMessage(&msg, NULL, WM_TCP_ASYNCDNR, WM_TCP_ASYNCDNR, PM_REMOVE))
	{
		::DispatchMessage(&msg);
	}

#define	WM_TCP_ASYNCSELECT	(WM_USER + 3)
	else if (::PeekMessage(&msg, NULL, WM_TCP_ASYNCSELECT, WM_TCP_ASYNCSELECT, PM_REMOVE))
	{
		::DispatchMessage(&msg);
	}

	else if (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_REMOVE))
	{
		::DispatchMessage(&msg);
		return FALSE;   // usually return TRUE, but OnIdle usually causes WM_PAINTs
	}
	return FALSE;
#endif
}
#endif

// WARNING This routine can only be executed by one thread at a time
int cdmutex::acquire()
{
#if __dest_os == __win32_os

	// First try to get the lock without waiting
	if (!try_lock())
	{
		// Set wait cursor if main thread
		bool wait_cursor = false;
		if (::GetCurrentThreadId() == AfxGetApp()->m_nThreadID)
		{
			AfxGetApp()->BeginWaitCursor();
			wait_cursor = true;
		}

		// Single element array of handles
		HANDLE wait_for[1];
		wait_for[0] = _lock;
		
		// Start timer
		time_t time_start = ::time(NULL);

		// Wait on the handle while pumping WM_PAINT messages only
		while(true)
		{
			DWORD dwResult = ::MsgWaitForMultipleObjects(1, wait_for, false, aquire_timeout_ms, QS_PAINT | QS_POSTMESSAGE | QS_SENDMESSAGE);
			
			if ((dwResult == WAIT_OBJECT_0) || (dwResult == WAIT_ABANDONED_0))
			{
				// Got the lock
	    		_tid = ::GetCurrentThreadId();
				_count++;
				break;
			}
			else if (dwResult == WAIT_OBJECT_0 + 1)
			{
				// Got a message - process it
				if (OnMessagePendingMutex())
					// allow user-interface updates
					AfxGetThread()->OnIdle(-1);
			}
			else if (dwResult == WAIT_TIMEOUT)
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}
			
			// Check timeout and throw
			if (::time(NULL) > time_start + aquire_timeout_s)
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}
		}
		
		// Clear wait cursor
		if (wait_cursor)
			AfxGetApp()->EndWaitCursor();
	}

#elif __dest_os == __mac_os || __dest_os == __mac_os_x

	cdthread_t this_tid = 0;

	::GetCurrentThread(&this_tid);

	// Is it the same thread
	if (_tid == this_tid)
		// Just bump count
		_count++;
	else
	{
		// Begin a busy operation
		CBusyContext busy;
		busy.SetCancelOthers(true);
		cdstring desc;
		StMailBusy busy_lock(&busy, &desc);

		// Start timer
		time_t time_start = ::time(NULL);

		// Must block while count
		while(true)
		{
			// Must lock criticial section here
			// This will prevent more than one thread gaining access to the mutex
			// when any existing thread relinquishes it

			// _count goes to zero when some other thread is finished
			if (!_count)
			{
				// This thread can acquire
				_tid = this_tid;
				_count = 1;
				// Can now leave the critical section as this thread is firmly established as the owner of the mutex
				break;
			}

			// Must unlock criticial section to allow message pumping

			// Yield to other threads
			yield(&busy);
			
			// Check timeout and throw
			if (::time(NULL) > time_start + aquire_timeout_s)
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}
		}
	}
#elif __dest_os == __linux_os

	cdthread_t this_tid(0,0);

	ACE_thread_t this_t = ACE_Thread::self();
	ACE_hthread_t this_th;
	ACE_Thread::self(this_th);
	this_tid = ACE_Thread_ID(this_t, this_th);

	// Is it the same thread
	if (_tid == this_tid)
		// Just bump count
		_count++;
	else
	{
		// Begin a busy operation
		CBusyContext busy;
		busy.SetCancelOthers(true);
		cdstring desc;
		StMailBusy busy_lock(&busy, &desc);

		// Start timer
		time_t time_start = ::time(NULL);

		// Must block while count
		while(true)
		{
			// Must lock criticial section here
			// This will prevent more than one thread gaining access to the mutex
			// when any existing thread relinquishes it
			_lock.acquire();

			// _count goes to zero when some other thread is finished
			if (!_count)
			{
				// This thread can acquire
				_tid = this_tid;
				_count = 1;
				// Can now leave the critical section as this thread is firmly established as the owner of the mutex
				_lock.release();
				break;
			}

			// Must unlock criticial section to allow message pumping
			_lock.release();

			// Yield to other threads
			yield(&busy);
			
			// Check timeout and throw
			if (::time(NULL) > time_start + aquire_timeout_s)
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}
		}
	}
#endif

    return 0;
}

void cdmutex::yield(CBusyContext* busy)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Ensure that events are pumped for main thread
	CMailControl::ProcessBusy(busy);

#elif __dest_os == __win32_os
	// Dispatch messages to avoid deadlock with the thread that has
	// the lock on the mutex. The locking thread may send a message to
	// this thread and block waiting for the message to be processed.
	MSG msg;

#if 1
	// Paint UI on idle
	if (!::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) || !OnMessagePendingMutex())
	{
		CWinThread* pThread = AfxGetThread();
		// allow user-interface updates
		pThread->OnIdle(-1);
	}
#else
	// New code to block current thread until message arrives

	// Paint UI on idle
	if (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) && OnMessagePendingMutex())
	{
		CWinThread* pThread = AfxGetThread();
		// allow user-interface updates
		pThread->OnIdle(-1);
	}
	else
		// Put to sleep
		::WaitMessage();
#endif
#elif __dest_os == __linux_os
	// Ensure that events are pumped for main thread
	CMailControl::ProcessBusy(busy);
#else
#error __dest_os
#endif
}

int cdmutex::release()
{
#if __dest_os == __win32_os

	::ReleaseMutex(_lock);
	_count--;

#else

#if __dest_os != __linux_os
	cdthread_t this_tid = 0;
#else
	cdthread_t this_tid(0,0);
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	::GetCurrentThread(&this_tid);
#elif __dest_os == __linux_os
	ACE_thread_t this_t = ACE_Thread::self();
	ACE_hthread_t this_th;
	ACE_Thread::self(this_th);
	this_tid = ACE_Thread_ID(this_t, this_th);
#endif

	// If threads the same decrement count
	if (_tid == this_tid)
	{

		// Changing _count must only be done in a critical section
#if __dest_os == __linux_os
		_lock.acquire();
#endif
		_count--;

		if (!_count)
#if __dest_os != __linux_os		
			_tid = 0;
#else
		_tid = ACE_Thread_ID(0,0);
#endif


		// Must unlock criticial section
#if __dest_os == __linux_os
		_lock.release();
#endif


	}
#endif

    return 0;
}

bool cdmutex::try_lock()
{
#if __dest_os == __win32_os

	// Wait on the object with 0 timeout
	if (::WaitForSingleObject(_lock, 0) == WAIT_OBJECT_0)
	{
		// Got the lock
    	_tid = ::GetCurrentThreadId();
		_count++;
		return TRUE;
	}
	else
		// Did not get a lock
		return FALSE;

#else

#if __dest_os != __linux_os
	cdthread_t this_tid = 0;
#else
	cdthread_t this_tid(0,0);
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	::GetCurrentThread(&this_tid);
#elif __dest_os == __linux_os
		ACE_thread_t this_t = ACE_Thread::self();
		ACE_hthread_t this_th;
		ACE_Thread::self(this_th);
		this_tid = ACE_Thread_ID(this_t, this_th);
#endif

	// Cannot lock if already locked by another thread
	if (_count && (_tid != this_tid))
		return false;
	else
	{
		// Acquire now
		acquire();
		return true;
	}
#endif
}

// is it locked by the current thread
bool cdmutex::is_locked_by_current_thread() const
{
#if __dest_os != __linux_os
	cdthread_t this_tid = 0;
#else
	cdthread_t this_tid(0,0);
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	::GetCurrentThread(&this_tid);
#elif __dest_os == __win32_os
    this_tid = ::GetCurrentThreadId();
#elif __dest_os == __linux_os
	ACE_thread_t this_t = ACE_Thread::self();
	ACE_hthread_t this_th;
	ACE_Thread::self(this_th);
	this_tid = ACE_Thread_ID(this_t, this_th);
#endif

	return is_locked() && (_tid == this_tid);
}

#pragma mark ____________________________cdrwmutex

cdrwmutex::cdrwmutex()
#if __dest_os == __linux_os
 : _write_tid(0,0)
#endif
{
	_write_count = 0;
#if __dest_os != __linux_os
	_write_tid = 0;
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	_lock = 0;
#elif __dest_os == __win32_os
	::InitializeCriticalSection(&_lock);
#endif
}

cdrwmutex::~cdrwmutex()
{
    remove();
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#elif __dest_os == __win32_os
	::DeleteCriticalSection(&_lock);
#else
	// _lock auto-destructsa
#endif
}

int cdrwmutex::remove()
{
	_write_count = 0;
#if __dest_os != __linux_os
	_write_tid = 0;
#else
	_write_tid = ACE_Thread_ID(0,0);
#endif
	_read_tids.clear();

    return 0;
}

// WARNING This routine can only be executed by one thread at a time
int cdrwmutex::read_acquire()
{
#if __dest_os != __linux_os
	cdthread_t this_tid = 0;
#else
	cdthread_t this_tid(0,0);
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	::GetCurrentThread(&this_tid);
#elif __dest_os == __win32_os
    this_tid = ::GetCurrentThreadId();
#elif __dest_os == __linux_os
		ACE_thread_t this_t = ACE_Thread::self();
		ACE_hthread_t this_th;
		ACE_Thread::self(this_th);
		this_tid = ACE_Thread_ID(this_t, this_th);
#endif

	// Is it the same thread
	if (_write_tid == this_tid)
	{
#if __dest_os == __win32_os
		::EnterCriticalSection(&_lock);
#elif __dest_os == __linux_os
		_lock.acquire();
#endif

		// Add tid to read set
#if __dest_os != __linux_os
		_read_tids.insert(this_tid);
#else
		_read_tids.push_back(this_tid);
#endif
		
#if __dest_os == __win32_os
		::LeaveCriticalSection(&_lock);
#elif __dest_os == __linux_os
		_lock.release();
#endif
	}
	else
	{
		// Must block while count
		while(true)
		{
			// Must lock criticial section here
			// This will prevent more than one thread gaining access to the mutex
			// when any existing thread relinquishes it
#if __dest_os == __win32_os
			::EnterCriticalSection(&_lock);
#elif __dest_os == __linux_os
		_lock.acquire();
#endif

			// _count goes to zero when write lock is finished by other thread
			if (!_write_count)
			{
#if __dest_os != __linux_os
				// Add tid to read set
				_read_tids.insert(this_tid);
#else
				_read_tids.push_back(this_tid);
#endif
				// Can now leave the critical section as this thread is firmly established as the owner of the mutex
#if __dest_os == __win32_os
				::LeaveCriticalSection(&_lock);
#elif __dest_os == __linux_os
				_lock.release();
#endif
				break;
			}
			
			// Must unlock criticial section to allow message pumping
#if __dest_os == __win32_os
			::LeaveCriticalSection(&_lock);
#elif __dest_os == __linux_os
		_lock.release();
#endif

			cdmutex::yield(NULL);
		}
	}

    return 0;
}

int cdrwmutex::read_release()
{
#if __dest_os != __linux_os
	cdthread_t this_tid = 0;
#else
	cdthread_t this_tid(0,0);
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	::GetCurrentThread(&this_tid);
#elif __dest_os == __win32_os
    this_tid = ::GetCurrentThreadId();
#elif __dest_os == __linux_os
		ACE_thread_t this_t = ACE_Thread::self();
		ACE_hthread_t this_th;
		ACE_Thread::self(this_th);
		this_tid = ACE_Thread_ID(this_t, this_th);
#endif

	// Changing _count must only be done in a critical section
#if __dest_os == __win32_os
	::EnterCriticalSection(&_lock);
#elif __dest_os == __linux_os
	_lock.acquire();
#endif
#if __dest_os != __linux_os
	cdthreadset::iterator found = _read_tids.find(this_tid);
#else
	cdthreadset::iterator found = _read_tids.begin();
	for (; found!=_read_tids.end(); ++found) if (*found == this_tid) break;
#endif
	if (found != _read_tids.end())
		_read_tids.erase(found);

	// Must unlock criticial section
#if __dest_os == __win32_os
	::LeaveCriticalSection(&_lock);
#elif __dest_os == __linux_os
	_lock.release();
#endif

    return 0;
}

// WARNING This routine can only be executed by one thread at a time
int cdrwmutex::write_acquire()
{
#if __dest_os != __linux_os
	cdthread_t this_tid = 0;
#else
	cdthread_t this_tid(0,0);
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	::GetCurrentThread(&this_tid);
#elif __dest_os == __win32_os
    this_tid = ::GetCurrentThreadId();
#elif __dest_os == __linux_os
		ACE_thread_t this_t = ACE_Thread::self();
		ACE_hthread_t this_th;
		ACE_Thread::self(this_th);
		this_tid = ACE_Thread_ID(this_t, this_th);
#endif

	// Is it the same thread
	if (_write_tid == this_tid)
		// Just bump count
		_write_count++;
	else
	{
		// Must block while count
		while(true)
		{
			// Must lock criticial section here
			// This will prevent more than one thread gaining access to the mutex
			// when any existing thread relinquishes it
#if __dest_os == __win32_os
			::EnterCriticalSection(&_lock);
#elif __dest_os == __linux_os
			_lock.acquire();
#endif

			// _count goes to zero when some other thread is finished
			if (!_write_count)
			{
#if __dest_os != __linux_os
				// Check that all read-tids match this one
				std::pair<cdthreadset::iterator, cdthreadset::iterator> eq = std::equal_range(_read_tids.begin(), _read_tids.end(), this_tid);
				if ((eq.first == _read_tids.begin()) &&
					(eq.second == _read_tids.end()))
#else
				bool all = true;
				for (cdthreadset::iterator i = _read_tids.begin();
						 i!= _read_tids.end();
						 ++i) {
					if (*i != this_tid) {
						all = false;
						break;
					}
				}
				if (all) 
#endif
				{
					// This thread can acquire
					_write_tid = this_tid;
					_write_count = 1;

					// Can now leave the critical section as this thread is firmly established as the owner of the mutex
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#elif __dest_os == __win32_os
					::LeaveCriticalSection(&_lock);
#elif __dest_os == __linux_os
					_lock.release();
#endif
					break;
				}
			}
			
			// Must unlock criticial section to allow message pumping
#if __dest_os == __win32_os
			::LeaveCriticalSection(&_lock);
#elif __dest_os == __linux_os
			_lock.release();
#endif
			cdmutex::yield(NULL);
		}
	}

    return 0;
}

int cdrwmutex::write_release()
{
#if __dest_os != __linux_os
	cdthread_t this_tid = 0;
#else
	cdthread_t this_tid(0,0);
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	::GetCurrentThread(&this_tid);
#elif __dest_os == __win32_os
    this_tid = ::GetCurrentThreadId();
#elif __dest_os == __linux_os
		ACE_thread_t this_t = ACE_Thread::self();
		ACE_hthread_t this_th;
		ACE_Thread::self(this_th);
		this_tid = ACE_Thread_ID(this_t, this_th);
#endif

	// If threads the same decrement count
	if (_write_tid == this_tid)
	{
		// Changing _count must only be done in a critical section
#if __dest_os == __win32_os
		::EnterCriticalSection(&_lock);
#elif __dest_os == __linux_os
		_lock.acquire();
#endif
		_write_count--;

		if (!_write_count)
#if __dest_os != __linux_os
			_write_tid = 0;
#else
		_write_tid = ACE_Thread_ID(0,0);
#endif

		// Must unlock criticial section
#if __dest_os == __win32_os
		::LeaveCriticalSection(&_lock);
#elif __dest_os == __linux_os
		_lock.release();
#endif

	}
    return 0;
}
