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


// Header for CAddressFieldMultiLine class

#ifndef __CADDRESSFIELDMULTILINE__MULBERRY__
#define __CADDRESSFIELDMULTILINE__MULBERRY__

#include "CAddressFieldBase.h"

// Constants

// Panes
const	PaneIDT		paneid_AddressFieldMultiLine = 1742;

// Mesages

// Resources
const	ResIDT		RidL_CAddressFieldMultiLineBtns = 1742;

// Classes
class CTextDisplay;

class CAddressFieldMultiLine : public CAddressFieldBase
{
protected:
	CTextDisplay*    mData;

public:
	enum { class_ID = 'Aflt' };
    
					CAddressFieldMultiLine();
					CAddressFieldMultiLine(LStream *inStream);
	virtual 		~CAddressFieldMultiLine();

    void            SetDetails(const cdstring& title, int newtype, const cdstring& newdata);
    bool            GetDetails(int& type, cdstring& data);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends
};

#endif
