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


// Source for CAddressPreviewAdvanced class

#include "CAddressPreviewAdvanced.h"

#include "CAdbkAddress.h"
#include "CAddressBook.h"
#include "CAddressFieldContainer.h"
#include "CCommands.h"
#include "CStaticText.h"
#include "CStringUtils.h"
#include "CToolbarButton.h"
#include "C3PaneWindow.h"

#include "CVCardAddressBook.h"
#include "CVCardVCard.h"
#include "CVCardMapper.h"

#include "cdstring.h"

#include "MyCFString.h"

#include <memory>
#include <strstream>

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressPreviewAdvanced::CAddressPreviewAdvanced()
{
}

// Constructor from stream
CAddressPreviewAdvanced::CAddressPreviewAdvanced(LStream *inStream)
    : CAddressPreviewBase(inStream)
{
}

// Default destructor
CAddressPreviewAdvanced::~CAddressPreviewAdvanced()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAddressPreviewAdvanced::FinishCreateSelf(void)
{
	// Do inherited
	CAddressPreviewBase::FinishCreateSelf();
    
	// See if we are a child of a 3-pane
	LView* super = GetSuperView();
	while(super && !dynamic_cast<C3PaneWindow*>(super))
		super = super->GetSuperView();
	m3PaneWindow = dynamic_cast<C3PaneWindow*>(super);
    
	// Get items
	mDescriptor = (CStaticText*) FindPaneByID(paneid_AddressPreviewAdvancedDesc);
	mFields = (CAddressFieldContainer*) FindPaneByID(paneid_AddressPreviewAdvancedFields);
    
	// Hide close/zoom if not 3pane
	if (!m3PaneWindow)
	{
		FindPaneByID(paneid_AddressPreviewAdvancedZoom)->Hide();
	}
	else
	{
		CToolbarButton* btn = (CToolbarButton*) FindPaneByID(paneid_AddressPreviewAdvancedZoom);
		btn->AddListener(this);
		btn->SetSmallIcon(true);
		btn->SetShowIcon(true);
		btn->SetShowCaption(false);
	}
    
	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, GetPaneID());
}

// Respond to clicks in the icon buttons
void CAddressPreviewAdvanced::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch(inMessage)
	{
        case msg_AddressPreviewAdvancedZoom:
            OnZoom();
            break;
	}
}

void CAddressPreviewAdvanced::OnZoom()
{
	m3PaneWindow->ObeyCommand(cmd_ToolbarZoomPreview, NULL);
}

void CAddressPreviewAdvanced::Close(void)
{
	// If it exists and its changed, update it
	// If it exits and its changed, update it
	if (mAdbk && mAddress && GetFields(mAddress))
	{
        // Need to set address to NULL bfore the update as the update
        // will cause a refresh and make this close again
        CAdbkAddress* temp = mAddress;
        mAddress = NULL;
        mAdbk->UpdateAddress(temp, true);
	}
}

void CAddressPreviewAdvanced::Focus()
{
	//LCommander::SwitchTarget(mFields);
}

void CAddressPreviewAdvanced::SetAddress(CAdbkAddress* addr)
{
	// Add to each panel
	mAddress = addr;
	cdstring title;
	if (addr)
		title = addr->GetName();
	MyCFString temp(title, kCFStringEncodingUTF8);
	mDescriptor->SetCFDescriptor(temp);
	SetFields(addr);
}

// Address removed by someone else
void CAddressPreviewAdvanced::ClearAddress()
{
	mAddress = NULL;
	SetAddress(NULL);
}

// Set fields in dialog
void CAddressPreviewAdvanced::SetFields(const CAdbkAddress* addr)
{
    mFields->SetAddress(addr);
}

// Get fields from dialog
bool CAddressPreviewAdvanced::GetFields(CAdbkAddress* addr)
{
	// Nothing to do right now
    return mFields->GetAddress(addr);
}
