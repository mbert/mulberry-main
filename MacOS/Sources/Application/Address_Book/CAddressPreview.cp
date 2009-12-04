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


// Source for CAddressPreview class

#include "CAddressPreview.h"

#include "CAdbkAddress.h"
#include "CAddressBook.h"
#include "CAddressContactPanel.h"
#include "CAddressNotesPanel.h"
#include "CAddressPersonalPanel.h"
#include "CCommands.h"
#include "CStaticText.h"
#include "CTabController.h"
#include "CToolbarButton.h"
#include "C3PaneWindow.h"

#include "MyCFString.h"

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressPreview::CAddressPreview()
{
	mAddress = NULL;
}

// Constructor from stream
CAddressPreview::CAddressPreview(LStream *inStream)
		: CAddressPreviewBase(inStream)
{
	mAddress = NULL;
}

// Default destructor
CAddressPreview::~CAddressPreview()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAddressPreview::FinishCreateSelf(void)
{
	// Do inherited
	CAddressPreviewBase::FinishCreateSelf();

	// See if we are a child of a 3-pane
	LView* super = GetSuperView();
	while(super && !dynamic_cast<C3PaneWindow*>(super))
		super = super->GetSuperView();
	m3PaneWindow = dynamic_cast<C3PaneWindow*>(super);

	mTabs = (CTabController*) FindPaneByID(paneid_TabController);
	mDescriptor = (CStaticText*) FindPaneByID(paneid_AddressPreviewDesc);

	// Hide close/zoom if not 3pane
	if (!m3PaneWindow)
	{
		FindPaneByID(paneid_AddressPreviewZoom)->Hide();
	}
	else
	{
		CToolbarButton* btn = (CToolbarButton*) FindPaneByID(paneid_AddressPreviewZoom);
		btn->AddListener(this);
		btn->SetSmallIcon(true);
		btn->SetShowIcon(true);
		btn->SetShowCaption(false);
	}

	// Get tabs pane
	InitTabs();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, GetPaneID());
}

// Respond to clicks in the icon buttons
void CAddressPreview::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch(inMessage)
	{
	case msg_AddressPreviewZoom:
		OnZoom();
		break;
	}
}

void CAddressPreview::OnZoom()
{
	m3PaneWindow->ObeyCommand(cmd_ToolbarZoomPreview, NULL);
}

void CAddressPreview::InitTabs(void)
{
	mTabs->AddPanel(paneid_AddressPersonal);
	mTabs->AddPanel(paneid_AddressContact);
	mTabs->AddPanel(paneid_AddressNotes);
}

void CAddressPreview::Close(void)
{
	// If it exists and its changed, update it
	if (mAdbk && mAddress)
	{
		// Need copy of original address
		CAdbkAddress old_addr(*mAddress);
		if (mTabs->UpdateData(mAddress))
		{
			// Need to set address to NULL bfore the update as the update
			// will cause a refresh and make this close again
			CAdbkAddress* temp = mAddress;
			mAddress = NULL;
			mAdbk->UpdateAddress(temp, true);
		}
	}
}

void CAddressPreview::Focus()
{
	static_cast<CAddressPanelBase*>(mTabs->GetCurrentPanel())->Focus();
}

void CAddressPreview::SetAddress(CAdbkAddress* addr)
{
	// Add to each panel
	mAddress = addr;
	cdstring title;
	if (addr)
		title = addr->GetName();
	MyCFString temp(title, kCFStringEncodingUTF8);
	mDescriptor->SetCFDescriptor(temp);
	mTabs->SetData(addr);
}

// Address removed by someone else
void CAddressPreview::ClearAddress()
{
	mAddress = NULL;
	SetAddress(NULL);
}
