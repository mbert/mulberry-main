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


// cdmutexprotect: Template class for protecting access to shared resource

#ifndef __CDMUTEXPROTECT__MULBERRY__
#define __CDMUTEXPROTECT__MULBERRY__

#include "cdmutex.h"

#ifdef __MULBERRY
#include "CGeneralException.h"
#include "CLog.h"
#endif

template <class T> class cdmutexprotect
{
public:
	// Stack class to do locks
	class lock
	{
	public:
		lock(cdmutexprotect& lock_it)
			: mLockIt(lock_it._mutex)
			{ mLockIt.acquire(); }
		~lock()
			{ mLockIt.release(); }
	private:
		cdmutex& mLockIt;
	};
	friend class lock;

	class try_lock
	{
	public:
		try_lock(cdmutexprotect& lock_it)
			: mLockIt(lock_it._mutex)
			{ have_lock = mLockIt.try_lock(); }
		~try_lock()
			{ if (have_lock) mLockIt.release(); }
		
		bool GotLock() const
			{ return have_lock; }

	private:
		cdmutex& mLockIt;
		bool 	have_lock;
	};
	friend class try_lock;


	cdmutexprotect() {}
	~cdmutexprotect() {}
	
	T* operator->();
	const T* operator->() const;

	T& get();					// This will throw if the object is not locked!
	const T& get() const;		// This will throw if the object is not locked!
	
	bool is_locked() const
		{ return _mutex.is_locked(); }

private:
	T			_obj;
	cdmutex		_mutex;
};

template <class T> T* cdmutexprotect<T>::operator->()
{
	if (!_mutex.is_locked_by_current_thread())
	{
#ifdef __MULBERRY
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
#else
		throw -1;
#endif
	}
	
	return &_obj;
}

template <class T> const T* cdmutexprotect<T>::operator->() const
{
	if (!_mutex.is_locked_by_current_thread())
	{
#ifdef __MULBERRY
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
#else
		throw -1;
#endif
	}
	
	return &_obj;
}

template <class T> T& cdmutexprotect<T>::get()
{
	if (!_mutex.is_locked_by_current_thread())
	{
#ifdef __MULBERRY
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
#else
		throw -1;
#endif
	}
	
	return _obj;
}

template <class T> const T& cdmutexprotect<T>::get() const
{
	if (!_mutex.is_locked_by_current_thread())
	{
#ifdef __MULBERRY
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
#else
		throw -1;
#endif
	}
	
	return _obj;
}
#endif
