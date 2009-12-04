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


// Header for CISO2022_KR class

#ifndef __CISO2022_KR__MULBERRY__
#define __CISO2022_KR__MULBERRY__

#include "CConverterBase.h"

#include "CASCII.h"
#include "CKSC_5601_1987.h"

namespace i18n 
{

class CISO2022_KR : public CConverterBase
{
public:
	enum EShiftState1
	{
		eASCII = 0,
		eTwoByte
	};
	enum EShiftState2
	{
		eNone = 0,
		eKSC_5601_1987
	};

	CISO2022_KR() :
		mKSC5601_1987_Converter(false)	// No ascii shift
		{ mState1 = eASCII; mState2 = eNone; }
	virtual ~CISO2022_KR() {}

	virtual void init_w_2_c(std::ostream& out);
	virtual void finish_w_2_c(std::ostream& out);

	virtual wchar_t c_2_w(const unsigned char*& c);
	virtual int w_2_c(wchar_t wc, char* out);

protected:
	EShiftState1 mState1;
	EShiftState2 mState2;

	CASCII			mASCII_Converter;
	CKSC_5601_1987	mKSC5601_1987_Converter;

	int changestate1(char*& out);
	int changestate2(char*& out);
};

}
#endif
