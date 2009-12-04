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


// cdptr.h

#ifndef __CDPTR__MULBERRY__
#define __CDPTR__MULBERRY__

template class cdptr<class T>
{
public:
	cdptr()
		{ _ptr = nil; _ref_ctr = 0; }
	cdptr(T* ptr)
		{ _ptr = ptr; _ref_ctr++; }
	~cdptr()
		{ _ref_ctr--; if (!_ref_ctr) delete _ptr; _ptr = nil; }
private:
	T*	_ptr;
	unsigned long _ref_ctr;
};

#endif
