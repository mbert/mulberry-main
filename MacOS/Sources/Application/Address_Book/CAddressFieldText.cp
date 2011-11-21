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


// Source for CAddressFieldText class

#include "CAddressFieldText.h"

#include "CStaticText.h"
#include "CTextFieldX.h"

#include <LPopupButton.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressFieldText::CAddressFieldText()
{
}

// Constructor from stream
CAddressFieldText::CAddressFieldText(LStream *inStream)
		: CAddressFieldBase(inStream)
{
}

// Default destructor
CAddressFieldText::~CAddressFieldText()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAddressFieldText::FinishCreateSelf(void)
{
	// Do inherited
	CAddressFieldBase::FinishCreateSelf();

	// Get controls
	mData = (CTextFieldX*) mDataMove;
    
	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CAddressFieldTextBtns);
}

void CAddressFieldText::SetDetails(const cdstring& title, int type, const cdstring& data)
{
    // Cache this to check for changes later
    mOriginalType = type;
    mOriginalData = data;

    mTitle->SetText(title);
    if (type != 0)
        mType->SetValue(type);
    mData->SetText(data);
}

bool CAddressFieldText::GetDetails(int& newtype, cdstring& newdata)
{
    bool changed = false;
    
    if (mUsesType)
    {
        newtype = mType->GetValue();
        if (newtype != mOriginalType)
            changed = true;
    }
    
    mData->GetText(newdata);
    if (newdata != mOriginalData)
        changed = true;
    
    return changed;
}
