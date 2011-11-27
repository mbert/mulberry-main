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

#ifndef __CAddressFieldMultiLine__MULBERRY__
#define __CAddressFieldMultiLine__MULBERRY__

#include "CAddressFieldBase.h"

#include "CCmdEdit.h"

// Constants

// Classes

class CAddressFieldMultiLine : public CAddressFieldBase
{
public:
					CAddressFieldMultiLine();
	virtual 		~CAddressFieldMultiLine();

    virtual void    SetDetails(const cdstring& title, int type, const cdstring& data);
    virtual bool    GetDetails(int& newtype, cdstring& newdata);
	
    virtual bool	SetInitialFocus();

protected:
	CCmdEdit			mData;

	afx_msg int 	OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

#endif
