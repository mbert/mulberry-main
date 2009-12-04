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


// Source for CSMTPAccountPopup class

#include "CSMTPAccountPopup.h"

#include "CMulberryCommon.h"
#include "CSMTPAccountManager.h"
#include "CSMTPSender.h"

#include <LIconControl.h>
#include <LPopupButton.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CSMTPAccountPopup::CSMTPAccountPopup(
							const SPaneInfo& inPaneInfo,
							const SViewInfo& inViewInfo,
							MessageT		inValueMessage,
							ResIDT			inMENUid,
							ResIDT			inICONid,
							ConstStringPtr	inTitle)
	: LView(inPaneInfo, inViewInfo)
{
	InitControls(inPaneInfo, inValueMessage, inMENUid, inICONid, inTitle);
	InitAccountPopup();
}

// Default destructor
CSMTPAccountPopup::~CSMTPAccountPopup()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Set to default item
void CSMTPAccountPopup::InitControls(const SPaneInfo& inPaneInfo,
										MessageT		inValueMessage,
										ResIDT			inMENUid,
										ResIDT			inICONid,
										ConstStringPtr	inTitle)
{
	{
		SPaneInfo pane;
		pane.visible = true;
		pane.enabled = true;
		pane.userCon = 0L;
		pane.superView = this;
		pane.bindings.left = true;
		pane.bindings.right = true;
		pane.bindings.top = true;
		pane.bindings.bottom = false;
		pane.paneID = inPaneInfo.paneID;
		pane.width = inPaneInfo.width - 32;
		pane.height = inPaneInfo.height;
		pane.left = 0;
		pane.top = 0;
		mAccountsPopup = new LPopupButton(
							pane,
							inValueMessage,
							popupTitleLeftJust,
							inMENUid,
							-1,
							kControlPopupButtonProc + kControlPopupFixedWidthVariant,
							261,
							inTitle,
							0,
							1);
	}

	{
		SPaneInfo pane;
		pane.visible = true;
		pane.enabled = true;
		pane.userCon = 0L;
		pane.superView = this;
		pane.bindings.left = false;
		pane.bindings.right = true;
		pane.bindings.top = true;
		pane.bindings.bottom = false;
		pane.paneID = inPaneInfo.paneID + 1;
		pane.width = 16;
		pane.height = 16;
		pane.left = inPaneInfo.width - 24;
		pane.top = 2;
		mSecure = new LIconControl(
							pane,
							0,
							kControlIconSuiteNoTrackProc,
							3014);
		mSecure->SetIconAlignment(kAlignAbsoluteCenter);
	}
}

void CSMTPAccountPopup::InitAccountPopup()
{
	// Delete previous items
	MenuHandle menuH = mAccountsPopup->GetMacMenuH();
	for(short i = ::CountMenuItems(menuH); i >= 1; i--)
		::DeleteMenuItem(menuH, i);

	short menu_pos = 1;
	short first_enabled = 0;

	// Add each SMTP account
	CSMTPSenderList& senders = CSMTPAccountManager::sSMTPAccountManager->GetSMTPSenders();
	for(CSMTPSenderList::const_iterator iter = senders.begin(); iter != senders.end(); iter++, menu_pos++)
	{
		// Insert item
		::AppendItemToMenu(menuH, menu_pos, (*iter)->GetAccountName());
		
		// Disable if not queued
		if (!static_cast<CSMTPAccount*>((*iter)->GetAccount())->GetUseQueue())
			::DisableItem(menuH, menu_pos);
		else if (!first_enabled)
			first_enabled = menu_pos;
	}

	// Force max/min update
	mAccountsPopup->SetMenuMinMax();

	// Set initial value and fake hit
	mAccountsPopup->SetValue(first_enabled);
}
