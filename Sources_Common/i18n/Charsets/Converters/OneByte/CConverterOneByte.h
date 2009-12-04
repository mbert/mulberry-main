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


// Header for CConverterOneByte class

#ifndef __CCONVERTERONEBYTE__MULBERRY__
#define __CCONVERTERONEBYTE__MULBERRY__

#include "CConverterBase.h"

namespace i18n 
{

class CConverterOneByte : public CConverterBase
{
public:
	CConverterOneByte(const wchar_t* map);
	virtual ~CConverterOneByte();

	virtual wchar_t c_2_w(const unsigned char*& c);
	virtual int w_2_c(wchar_t wc, char* out);

protected:
	const wchar_t* m_c_2_w_map;
	const unsigned char* m_w_2_c_map;
	wchar_t max_w_2_c;

	void generate_map();
};

}
#endif
