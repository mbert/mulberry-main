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


// Header for CISO2022_JP class

#ifndef __CISO2022_JP__MULBERRY__
#define __CISO2022_JP__MULBERRY__

#include "CConverterBase.h"

#include "CASCII.h"
#include "CJIS_0201.h"
#include "CJIS_0208.h"

namespace i18n 
{

class CISO2022_JP : public CConverterBase
{
public:
	enum EShiftState
	{
		eASCII = 0,
		eJIS_0201,
		eJIS_0208
	};

	CISO2022_JP()
		{ mState = eASCII; }
	virtual ~CISO2022_JP() {}

	virtual void init_w_2_c(std::ostream& out);
	virtual void finish_w_2_c(std::ostream& out);

	virtual wchar_t c_2_w(const unsigned char*& c);
	virtual int w_2_c(wchar_t wc, char* out);

protected:
	EShiftState mState;

	CASCII		mASCII_Converter;
	CJIS_0201	mJIS0201_Converter;
	CJIS_0208	mJIS0208_Converter;

	int changestate(char*& out);
};

}
#endif
