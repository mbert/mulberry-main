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


// Header for CConverterBase class

#ifndef __CCONVERTERBASE__MULBERRY__
#define __CCONVERTERBASE__MULBERRY__

#include <ostream>

namespace i18n 
{

class CConverterBase
{
public:
	CConverterBase() {}
	virtual ~CConverterBase() {}

	static char undefined_charmap;
	static wchar_t undefined_wcharmap;

	virtual void ToUnicode(const char* str, size_t len, std::ostream& out);
	virtual void FromUnicode(const wchar_t* wstr, size_t wlen, std::ostream& out);

	virtual void ToUTF16(const char* str, size_t len, std::ostream& out);
	virtual void FromUTF16(const unsigned short* str, size_t ulen, std::ostream& out);

	virtual void ToUTF8(const char* str, size_t len, std::ostream& out);
	virtual void FromUTF8(const char* str, size_t len, std::ostream& out);

	virtual void init_w_2_c(std::ostream& out);
	virtual void finish_w_2_c(std::ostream& out);

	virtual wchar_t c_2_w(const unsigned char*& c) = 0;
	virtual int w_2_c(wchar_t wc, char* out) = 0;
};

}

#endif
