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


// Header for CCJKBase class

#ifndef __CCJKBASE__MULBERRY__
#define __CCJKBASE__MULBERRY__

#include "CConverterBase.h"

namespace i18n 
{

class CCJKBase : public CConverterBase
{
public:
	CCJKBase() {}
	virtual ~CCJKBase() {}

	virtual wchar_t c_2_w(const unsigned char*& c);
	virtual int w_2_c(wchar_t wc, char* out);

protected:
	size_t mOffset1;
	size_t mOffset2;
	size_t mSize1;
	size_t mSize2;
	size_t mMax1;
	size_t mMax2;
	size_t mNumPages;

	void init_tables();
	
	// c_2_w table data
	virtual wchar_t tabledata(size_t i, size_t j) const = 0;
	
	// w_2_c page maps
	virtual wchar_t get_page_bounds(int i, int j) const = 0;
	virtual void set_page_map(int i, char* pm) = 0;
	virtual const char* get_page_map(int i) const = 0;
};

}
#endif
