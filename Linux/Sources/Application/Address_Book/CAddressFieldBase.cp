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


// Source for CAddressFieldBase class

#include "CAddressFieldBase.h"

#include "CAddressFieldSubContainer.h"
#include "CStaticText.h"
#include "CToolbarButton.h"

#include "TPopupMenu.h"

const char* cGenericTypeField =
	"Home %r| Work %r| Other %r";

const char* cTelephoneTypeField =
	"Home %r| Work %r| Mobile %r| Fax %r| Home Fax %r| Work Fax %r| Pager %r| Other %r";

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressFieldBase::CAddressFieldBase(JXContainer* enclosure,
		const HSizingOption hSizing, const VSizingOption vSizing,
		const JCoordinate x, const JCoordinate y,
		const JCoordinate w, const JCoordinate h) :
		JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Default destructor
CAddressFieldBase::~CAddressFieldBase()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAddressFieldBase::OnCreate()
{
	// Assume layout has been generated already

	// Get controls
	mAdd->SetImage(IDI_ACL_CREATE, IDI_ACL_CREATE);
    mAdd->SetSmallIcon(true);
    mAdd->SetShowIcon(true);
    mAdd->SetShowCaption(false);
    mAdd->SetShowFrame(false);
    ListenTo(mAdd);

    mRemove->SetImage(IDI_ACL_DELETE, IDI_ACL_DELETE);
    mRemove->SetSmallIcon(true);
    mRemove->SetShowIcon(true);
    mRemove->SetShowCaption(false);
    mRemove->SetShowFrame(false);
    ListenTo(mRemove);

	mType->SetMenuItems(cGenericTypeField);
	mType->SetValue(1);

}

// Handle buttons
void CAddressFieldBase::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mAdd)
		{
			OnAdd();
			return;
		}
		else if (sender == mRemove)
		{
			OnRemove();
			return;
		}
	}

	JXWidgetSet::Receive(sender, message);
}

void CAddressFieldBase::SetSingleInstance()
{
    mAdd->Hide();
    mRemove->Hide();
    HideType();
}

void CAddressFieldBase::SetMultipleInstance()
{
    mAdd->Hide();
    HideTitle();
}

void CAddressFieldBase::SetBottom()
{
    mAdd->Show();
    mRemove->Hide();
    mTitle->Hide();
    mType->Hide();
    mDataMove->Hide();
}

const int cButtonHOffset = 22;

void CAddressFieldBase::HideType()
{
    mType->Hide();
    mUsesType = false;
    
    // Move and resize data field
    mTitle->Move(-cButtonHOffset, 0);
    JCoordinate size = mType->GetFrameWidth();
    mDataMove->Move(-size - cButtonHOffset, 0);
    mDataMove->AdjustSize(size + cButtonHOffset, 0);
}

void CAddressFieldBase::HideTitle()
{
    mTitle->Hide();
    
    // Move and resize type/data fields
    JCoordinate size = mTitle->GetFrameWidth();
    mType->Move(-size, 0);
    mDataMove->Move(-size, 0);
    mDataMove->AdjustSize(size, 0);
}

void CAddressFieldBase::SetMenu(bool typePopup)
{
	mType->SetMenuItems(cTelephoneTypeField);
}


void CAddressFieldBase::OnAdd()
{
    CAddressFieldSubContainer* parent = static_cast<CAddressFieldSubContainer*>(GetEnclosure());
    parent->AppendField();
}

void CAddressFieldBase::OnRemove()
{
    CAddressFieldSubContainer* parent = static_cast<CAddressFieldSubContainer*>(GetEnclosure());
    parent->RemoveField(this);
}

