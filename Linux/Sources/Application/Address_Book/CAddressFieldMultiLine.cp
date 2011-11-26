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


// Source for CAddressFieldMultiLine class

#include "CAddressFieldMultiLine.h"

#include "CStaticText.h"
#include "CTextDisplay.h"
#include "CToolbarButton.h"

#include "TPopupMenu.h"

#include <JXScrollbarSet.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressFieldMultiLine::CAddressFieldMultiLine(JXContainer* enclosure,
		const HSizingOption hSizing, const VSizingOption vSizing,
		const JCoordinate x, const JCoordinate y,
		const JCoordinate w, const JCoordinate h) :
		CAddressFieldBase(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Default destructor
CAddressFieldMultiLine::~CAddressFieldMultiLine()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAddressFieldMultiLine::OnCreate()
{
// begin JXLayout1

    CToolbarButton* obj1 =
        new CToolbarButton("", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 3,0, 20,20);
    assert( obj1 != NULL );

    CToolbarButton* obj2 =
        new CToolbarButton("", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 3,1, 20,20);
    assert( obj2 != NULL );

    CStaticText* obj3 =
        new CStaticText("", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 25,0, 80,20);
    assert( obj3 != NULL );

    HPopupMenu* obj4 =
        new HPopupMenu("", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 106,0, 102,20);
    assert( obj4 != NULL );

    mData =
        new CTextInputDisplay(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 210,0, 280,60);
    assert( mData != NULL );

// end JXLayout1

    mAdd = obj1;
    mRemove = obj2;
    mTitle = obj3;
    mType = obj4;
	mDataMove = static_cast<JXWidget*>(mData->GetScroller());

	// Do inherited
	CAddressFieldBase::OnCreate();
}

void CAddressFieldMultiLine::SetDetails(const cdstring& title, int type, const cdstring& data)
{
    // Cache this to check for changes later
    mOriginalType = type;
    mOriginalData = data;

    mTitle->SetText(title);
    if (type != 0)
        mType->SetValue(type);
    mData->SetText(data);
}

bool CAddressFieldMultiLine::GetDetails(int& newtype, cdstring& newdata)
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
