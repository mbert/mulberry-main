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

#include <LBevelButton.h>
#include <LPopupButton.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressFieldBase::CAddressFieldBase()
{
    mUsesType = true;
}

// Constructor from stream
CAddressFieldBase::CAddressFieldBase(LStream *inStream)
		: LView(inStream)
{
    mUsesType = true;
}

// Default destructor
CAddressFieldBase::~CAddressFieldBase()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAddressFieldBase::FinishCreateSelf(void)
{
	// Do inherited
	LView::FinishCreateSelf();

	// Get controls
	mAdd = (LBevelButton*) FindPaneByID(paneid_AddressFieldBaseAdd);
	mRemove = (LBevelButton*) FindPaneByID(paneid_AddressFieldBaseRemove);
	mTitle = (CStaticText*) FindPaneByID(paneid_AddressFieldBaseTitle);
	mType = (LPopupButton*) FindPaneByID(paneid_AddressFieldBaseType);
	mDataMove = (LView*) FindPaneByID(paneid_AddressFieldBaseData);
}

// Handle buttons
void CAddressFieldBase::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_AddressFieldBaseAdd:
			OnAdd();
			break;

		case msg_AddressFieldBaseRemove:
			OnRemove();
			break;

		default:
			break;
	}
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
    mTitle->MoveBy(-cButtonHOffset, 0, false);
    SDimension16 size;
    mType->GetFrameSize(size);
    mDataMove->MoveBy(-size.width - cButtonHOffset, 0, false);
    mDataMove->ResizeFrameBy(size.width + cButtonHOffset, 0, false);
}

void CAddressFieldBase::HideTitle()
{
    mTitle->Hide();
    
    // Move and resize type/data fields
    SDimension16 size;
    mTitle->GetFrameSize(size);
    mType->MoveBy(-size.width, 0, false);
    mDataMove->MoveBy(-size.width, 0, false);
    mDataMove->ResizeFrameBy(size.width, 0, false);
}

void CAddressFieldBase::SetMenu(ResIDT typePopup)
{
    mType->SetMenuID(typePopup);
}


void CAddressFieldBase::OnAdd()
{
    CAddressFieldSubContainer* parent = static_cast<CAddressFieldSubContainer*>(GetSuperView());
    parent->AppendField();
}

void CAddressFieldBase::OnRemove()
{
    CAddressFieldSubContainer* parent = static_cast<CAddressFieldSubContainer*>(GetSuperView());
    parent->RemoveField(this);
}

