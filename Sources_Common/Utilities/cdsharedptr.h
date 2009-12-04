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


// cdsharedptr.h

#ifndef __cdsharedptr__MULBERRY__
#define __cdsharedptr__MULBERRY__

#include <stdlib.h>

template<typename T> class cdsharedptr
{
public:
	explicit cdsharedptr(T* ptr = NULL) :
		_ptr(ptr), _ref(NULL)
	{
		try 
		{
			_ref = new unsigned long(1);
		}
		catch(...)
		{
			delete _ptr;
			throw;
		}
	}
	
#if defined(__GNUC__)
	template<typename P> cdsharedptr(const cdsharedptr<P>& copy) :
		_ptr(copy._ptr), _ref(copy._ref)
	{
		++*_ref;
	}
	cdsharedptr(const cdsharedptr& copy) :
		_ptr(copy._ptr), _ref(copy._ref)
	{
		++*_ref;
	}
#else
	template<typename P> explicit cdsharedptr(const cdsharedptr<P>& copy) :
		_ptr(copy._ptr), _ref(copy._ref)
	{
		++*_ref;
	}
	explicit cdsharedptr(const cdsharedptr& copy) :
		_ptr(copy._ptr), _ref(copy._ref)
	{
		++*_ref;
	}
#endif

	~cdsharedptr()
		{ dispose(); }

	template<typename P> cdsharedptr& operator=(const cdsharedptr<P>& copy)
	{
		share(copy._ptr, copy._ref);
		return *this;
	}
	cdsharedptr& operator=(const cdsharedptr& copy)
	{
		share(copy._ptr, copy._ref);
		return *this;
	}

	void reset(T* ptr = NULL)
	{
		if (_ptr == ptr) return;
		if (--*_ref == 0)
		{
			delete _ptr;
			ptr = NULL;
		}
		else
		{
			try 
			{
				_ref = new unsigned long;
			}
			catch(...)
			{
				++*_ref;
				delete ptr;
				throw;
			}
		}
		*_ref = 1;
		_ptr = ptr;
	}

	T& operator*() const
	{
		return *_ptr;
	}
	T* operator->() const
	{
		return _ptr;
	}
	T* get() const
	{
		return _ptr;
	}

	unsigned long use_count() const
	{
		return *_ref;
	}
	bool unique() const
	{
		return *_ref == 1;
	}

private:
	T*	_ptr;
	unsigned long* _ref;
	
	void dispose()
	{
		if (--*_ref == 0)
		{
			delete _ptr;
			_ptr = NULL;
			delete _ref;
			_ref = NULL;
		}
	}
	
	void share(T* ptr, unsigned long* ref)
	{
		if (ref != _ref)
		{
			++*ref;
			dispose();
			_ptr = ptr;
			_ref = ref;
		}
	}
};

template<class T, class U> inline bool operator==(cdsharedptr<T> const & a, cdsharedptr<U> const & b)
{
    return a.get() == b.get();
}

template<class T, class U> inline bool operator!=(cdsharedptr<T> const & a, cdsharedptr<U> const & b)
{
    return a.get() != b.get();
}

template<class T, class U> inline bool operator<(cdsharedptr<T> const & a, cdsharedptr<U> const & b)
{
    return a.get() < b.get();
}
#endif
