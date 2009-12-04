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


// cdomutex.cp - handles thread locking

#include "cdomutex.h"

#include "CMbox.h"

#pragma mark ____________________________cdomutex

template <class T> void cdomutex<T>::acquire(const T* obj)
{
	do
	{
		bool acquired = false;

		// Lock the mutex
		_mutex.acquire();
		
		// See if object is in the set
		typename objinfoset::iterator found = _objects.find(obj);
		
		// If not found, insert it
		if (found == _objects.end())
		{
			// Insert - now acquired
			_objects.insert(obj);
			acquired = true;
		}

		// Check whether thread id's match
		else if ((*found).mTid == cdmutex::current_tid())
		{
			// Bump up reference count - still acquired
			const_cast<objinfo&>(*found).mRefCount++;
			acquired = true;
		}

		// Unlock the mutex
		_mutex.release();

		// If acquired then exit
		if (acquired)
			break;
		
		// Yield and wait
		cdmutex::yield(NULL);

	} while(true);
}

template <class T> void cdomutex<T>::release(const T* obj)
{
	// Lock the mutex
	_mutex.acquire();
	
	// See if object is in the set
	typename objinfoset::iterator found = _objects.find(obj);
		
	// If found, remove reference count
	if (found != _objects.end())
	{
		// Bump down reference count and remove it if count is now zero
		if (!--const_cast<objinfo&>(*found).mRefCount)
			_objects.erase(found);
	}
	
	// Unlock the mutex
	_mutex.release();
}

template <class T> bool cdomutex<T>::is_locked(const T* obj)
{
	// Lock the mutex
	_mutex.acquire();
	
	// See if mbox is in the set
	typename objinfoset::size_type result = _objects.count(obj);
	
	// Unlock the mutex
	_mutex.release();
	
	return result;
}

template <class T> bool cdomutex<T>::try_lock(const T* obj)
{
	bool acquired = false;

	// Lock the mutex
	_mutex.acquire();
	
	// See if object is in the set
	typename objinfoset::iterator found = _objects.find(obj);
	
	// If not found, insert it
	if (found == _objects.end())
	{
		// Insert - now acquired
		_objects.insert(obj);
		acquired = true;
	}

	// Check whether thread id's match
	else if ((*found).mTid == cdmutex::current_tid())
	{
		// Bump up reference count - still acquired
		const_cast<objinfo&>(*found).mRefCount++;
		acquired = true;
	}

	// Unlock the mutex
	_mutex.release();

	// Return acquired state
	return acquired;

}

template class cdomutex<CMbox>;

