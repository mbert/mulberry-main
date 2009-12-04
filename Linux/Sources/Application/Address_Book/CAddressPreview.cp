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
#include "CIconLoader.h"
#include "CStaticText.h"
#include "CTabController.h"
#include "CToolbarButton.h"
#include "C3PaneWindow.h"

#include <JXColormap.h>
#include <JXImageWidget.h>
#include <JXUpRect.h>

#include <cassert>

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressPreview::CAddressPreview(JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h)
		: CAddressPreviewBase(enclosure, hSizing, vSizing, x, y, w, h)
{
	mAddress = NULL;
}

// Default destructor
CAddressPreview::~CAddressPreview()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAddressPreview::OnCreate()
{
// begin JXLayout1

    JXUpRect* obj1 =
        new JXUpRect(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,0, 475,24);
    assert( obj1 != NULL );

    JXImageWidget* icon =
        new JXImageWidget(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,2, 16,16);
    assert( icon != NULL );

    mDescriptor =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 25,2, 445,20);
    assert( mDescriptor != NULL );

    mZoomBtn =
        new CToolbarButton("",this,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 477,2, 20,20);
    assert( mZoomBtn != NULL );

    mTabs =
        new CTabController(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,24, 500,276);
    assert( mTabs != NULL );

// end JXLayout1

	icon->SetImage(CIconLoader::GetIcon(IDI_3PANEPREVIEWADDRESS, icon, 16, 0x00CCCCCC), kFalse);

	// See if we are a child of a 3-pane
	m3PaneWindow = dynamic_cast<C3PaneWindow*>(GetWindow()->GetDirector());

	// Hide close/zoom if not 3pane
	if (!m3PaneWindow)
	{
		mZoomBtn->Hide();
	}
	else
	{
		ListenTo(mZoomBtn);
		mZoomBtn->SetImage(IDI_3PANE_ZOOM, 0);
		mZoomBtn->SetSmallIcon(true);
		mZoomBtn->SetShowIcon(true);
		mZoomBtn->SetShowCaption(false);
	}

	// Get tabs pane
	InitTabs();
}

// Respond to clicks in the icon buttons
void CAddressPreview::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mZoomBtn)
		{
			OnZoom();
			return;
		}
	}
	
	CAddressPreviewBase::Receive(sender, message);
}

void CAddressPreview::OnZoom()
{
	m3PaneWindow->ObeyCommand(CCommand::eToolbarZoomPreview, NULL);
}

void CAddressPreview::InitTabs(void)
{
	CTabPanel* card = new CAddressPersonalPanel(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 500, 250);
	mTabs->AppendCard(card, "Personal");

	card = new CAddressContactPanel(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 500, 250);
	mTabs->AppendCard(card, "Contact");

	card = new CAddressNotesPanel(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 500, 250);
	mTabs->AppendCard(card, "Notes");
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
	static_cast<CAddressPanelBase*>(mTabs->GetCurrentCard())->Focus();
}

void CAddressPreview::SetAddress(CAdbkAddress* addr)
{
	// Add to each panel
	mAddress = addr;
	cdstring title;
	if (addr)
		title = addr->GetName();
	mDescriptor->SetText(title);
	mTabs->SetData(addr);
}

// Address removed by someone else
void CAddressPreview::ClearAddress()
{
	mAddress = NULL;
	SetAddress(NULL);
}
