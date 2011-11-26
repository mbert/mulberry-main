/*
    Copyright (c) 2007-2011 Cyrus Daboo. All rights reserved.
    
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


// Header for CAddressFieldText class

#ifndef __CADDRESSFIELDTEXT__MULBERRY__
#define __CADDRESSFIELDTEXT__MULBERRY__

#include "CAddressFieldBase.h"

// Constants

// Classes
class CTextInputField;

class CAddressFieldText : public CAddressFieldBase
{
public:
					CAddressFieldText(JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h);
	virtual 		~CAddressFieldText();

    void            SetDetails(const cdstring& title, int type, const cdstring& data);
    bool            GetDetails(int& newtype, cdstring& newdata);

protected:
// begin JXLayout1

    CTextInputField* mData;

// end JXLayout1
	virtual void	OnCreate();					// Do odds & ends
};

#endif
