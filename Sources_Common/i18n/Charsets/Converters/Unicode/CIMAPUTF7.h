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


// Header for CIMAPUTF7 class

#ifndef __CIMAPUTF7__MULBERRY__
#define __CIMAPUTF7__MULBERRY__

#include "CConverterBase.h"

namespace i18n 
{

class CIMAPUTF7 : public CConverterBase
{
public:
	CIMAPUTF7()
	{
		mEncoding = false;
	}
	virtual ~CIMAPUTF7() {}

	virtual void init_w_2_c(std::ostream& out);
	virtual void finish_w_2_c(std::ostream& out);

	virtual wchar_t c_2_w(const unsigned char*& c);
	virtual int w_2_c(wchar_t wc, char* out);

private:
	struct TBase64
	{
	#ifdef big_endian
		unsigned b0 : 6;
		unsigned b1 : 6;
		unsigned b2 : 6;
		unsigned b3 : 6;
	#else
		unsigned b3 : 6;
		unsigned b2 : 6;
		unsigned b1 : 6;
		unsigned b0 : 6;
	#endif
	};

	union TAtom
	{
		TBase64			base64;
		unsigned char	base256[3];
	};

	bool			mEncoding;
	TAtom			mEncoder;
	unsigned long	mAtomPos;
	
			int DoneEncoding(char*& out);
};

}
#endif