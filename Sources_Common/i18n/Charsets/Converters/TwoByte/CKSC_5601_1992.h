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


// Header for CKSC_5601_1992 class

#ifndef __CKSC_5601_1992__MULBERRY__
#define __CKSC_5601_1992__MULBERRY__

#include "CCJKBase.h"

namespace i18n 
{

class CKSC_5601_1992 : public CCJKBase
{
public:
	CKSC_5601_1992(bool shift = true);
	virtual ~CKSC_5601_1992() {}

	virtual wchar_t c_2_w(const unsigned char*& c);
	virtual int w_2_c(wchar_t wc, char* out);

protected:
	bool mASCIIShift;

	// c_2_w table data
	virtual wchar_t tabledata(int i, int j) const;
	
	// w_2_c page maps
	virtual wchar_t get_page_bounds(int i, int j) const;
	virtual void set_page_map(int i, char* pm);
	virtual const char* get_page_map(int i) const;
};

}
#endif