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


// cdomutex : mutex to manage multiple objct locks

#ifndef __CDOMUTEX__MULBERRY__
#define __CDOMUTEX__MULBERRY__

#include "cdmutex.h"

#include <set>

template <class T> class cdomutex
{
public:
	class lock_cdomutex
	{
	public:
		lock_cdomutex(cdomutex& lock_it, const T* obj)
			: mLockIt(lock_it)
			{ mLockIt.acquire(obj); mObj = obj; }
		~lock_cdomutex()
			{ mLockIt.release(mObj); }
	private:
		cdomutex& 		mLockIt;
		const T*		mObj;
	};

	class trylock_cdomutex
	{
	public:
		trylock_cdomutex(cdomutex& lock_it, const T* obj)
			: mLockIt(lock_it)
			{ mLocked = mLockIt.try_lock(obj); mObj = obj; }
		~trylock_cdomutex()
			{ if (mLocked) mLockIt.release(mObj); }
		
		bool is_locked() const
			{ return mLocked; }

	private:
		cdomutex& 		mLockIt;
		bool			mLocked;
		const T*		mObj;
	};

	cdomutex() {}
	~cdomutex() {}
	
	void acquire(const T* obj);
	void release(const T* obj);
	bool is_locked(const T* obj);
	bool try_lock(const T* obj);

private:
	class objinfo
	{
		friend class cdomutex;

	public:
		objinfo(const T* obj) :
			mTid(cdmutex::current_tid())
			{ mObj = obj; mRefCount = 1; }
		~objinfo() {}

	int operator==(const objinfo& comp) const						// Compare with same
		{ return mObj == comp.mObj; }								// Just compare object pointers
	int operator<(const objinfo& comp) const						// Compare with same
		{ return mObj < comp.mObj; }								// Just compare object pointers

	private:
		const T* 				mObj;
		cdmutex::cdthread_t		mTid;
		unsigned long			mRefCount;
	};

	typedef std::set<objinfo>	objinfoset;
	cdmutex						_mutex;
	objinfoset					_objects;
	
};

#endif
