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


// Source for CEditIdentityOutgoing class

#include "CEditIdentityOutgoing.h"

#include "CPrefsSubTab.h"

#include "CEditIdentityAddress.h"
#include "CEditIdentityDSN.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CEditIdentityOutgoing::CEditIdentityOutgoing()
{
}

// Constructor from stream
CEditIdentityOutgoing::CEditIdentityOutgoing(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CEditIdentityOutgoing::~CEditIdentityOutgoing()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CEditIdentityOutgoing::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mTabs = (CPrefsSubTab*) FindPaneByID(paneid_EditIdentityOutgoingTabs);

	// Create tab sub-panels
	{
		mTabs->AddPanel(paneid_EditIdentityAddressOutgoing);
		CEditIdentityAddress* panel = (CEditIdentityAddress*) mTabs->GetPanel(0);
		panel->SetAddressType(false, false, false, true, false, false, false);
	}
	{
		mTabs->AddPanel(paneid_EditIdentityAddressOutgoing);
		CEditIdentityAddress* panel = (CEditIdentityAddress*) mTabs->GetPanel(1);
		panel->SetAddressType(false, false, false, false, true, false, false);
	}
	{
		mTabs->AddPanel(paneid_EditIdentityAddressOutgoing);
		CEditIdentityAddress* panel = (CEditIdentityAddress*) mTabs->GetPanel(2);
		panel->SetAddressType(false, false, false, false, false, true, false);
	}
	mTabs->AddPanel(paneid_EditIdentityDSN);
	mTabs->SetDisplayPanel(1);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CEditIdentityOutgoingBtns);

}

// Handle buttons
void CEditIdentityOutgoing::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_EditIdentityOutgoingTabs:
			mTabs->SetDisplayPanel(*(unsigned long*) ioParam);
			break;
	}
}

// Set prefs
void CEditIdentityOutgoing::SetData(void* data)
{
	// Set existing panel
	mTabs->SetData(data);
}

// Force update of prefs
void CEditIdentityOutgoing::UpdateData(void* data)
{
	// Update existing panel
	mTabs->UpdateData(data);
}
