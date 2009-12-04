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


// Header for CSSLUtls.h

#ifndef __CSSLUTILS__MULBERRY__
#define __CSSLUTILS__MULBERRY__

#include <stdlib.h>

class cdstring;
struct PW_CB_DATA;

namespace NSSL
{
	
	// Handle auto-delete of SSL objects
	template<class T, class U = T> class StSSLObject
	{
	public:
		explicit StSSLObject(T* obj = 0)
			: mObj(obj) {}
		inline StSSLObject(const StSSLObject<T>& a)
			: mObj(a.release()) {}
		inline StSSLObject& operator=(const StSSLObject<T>& a)
			{ reset(a.release()); }
		inline ~StSSLObject()
			{ if (mObj) delete_obj(); }

		inline T& operator*() const
			{ return &mObj; }
		inline T* operator->() const
			{ return mObj; }
		inline T* get() const
			{ return mObj; }

		inline T* release()
			{ T* temp = mObj; mObj = NULL; return temp; }
		inline void reset(T* p = 0)
			{ if (p != mObj) { delete_obj(); mObj = p; } }

	private:
		T*	mObj;
		
		void delete_obj();
	};

	// Better error strings	
	void ERR_better_errors(cdstring& shorterrs, cdstring& longerrs);

	extern "C" int PassphraseCallback(char* buf, int bufsiz, int verify, char* out);

}

#endif
