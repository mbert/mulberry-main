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


// Header for CSJIS class

#ifndef __CSJIS__MULBERRY__
#define __CSJIS__MULBERRY__

#include "CConverterBase.h"

#include "CJIS_0201.h"
#include "CJIS_0208.h"

namespace i18n 
{

class CSJIS : public CConverterBase
{
public:
	CSJIS() {}
	virtual ~CSJIS() {}

	virtual wchar_t c_2_w(const unsigned char*& c);
	virtual int w_2_c(wchar_t wc, char* out);

private:
	CJIS_0201	mJIS0201_Converter;
	CJIS_0208	mJIS0208_Converter;
};

}
#endif
