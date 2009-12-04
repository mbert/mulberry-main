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

#include "CIconLoader.h"
#include "CMulberryApp.h"
#include "CSMTPAccountManager.h"
#include "CSMTPSender.h"

#include "TPopupMenu.h"

#include <JXImageWidget.h>
#include <JXStaticText.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S


// Constructor from stream
CSMTPAccountPopup::CSMTPAccountPopup(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h)
	: JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Default destructor
CSMTPAccountPopup::~CSMTPAccountPopup()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

void CSMTPAccountPopup::OnCreate()
{
// begin JXLayout1

    mAccountsPopup =
        new HPopupMenu("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 55,0, 140,20);
    assert( mAccountsPopup != NULL );

    mTitle =
        new JXStaticText("Accounts:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,3, 55,15);
    assert( mTitle != NULL );

    mSecure =
        new JXImageWidget(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 200,2, 16,16);
    assert( mSecure != NULL );

// end JXLayout1

	// Popup
	InitAccountPopup();

	// Icon
	mSecure->SetImage(CIconLoader::GetIcon(IDI_FLAG_ENCRYPTED, mSecure, 16, 0x00CCCCCC), kFalse);
}

void CSMTPAccountPopup::InitAccountPopup()
{
	// Delete previous items
	mAccountsPopup->RemoveAllItems();

	// Add each SMTP account
	int menu_pos = 1;
	int first_enabled = 1;
	CSMTPSenderList& senders = CSMTPAccountManager::sSMTPAccountManager->GetSMTPSenders();
	for(CSMTPSenderList::const_iterator iter = senders.begin(); iter != senders.end(); iter++, menu_pos++)
	{
		// Insert item
		mAccountsPopup->AppendItem((*iter)->GetAccountName(), kTrue, kFalse);
		
		// Disable if not queued
		if (!static_cast<CSMTPAccount*>((*iter)->GetAccount())->GetUseQueue())
			mAccountsPopup->SetItemEnable(menu_pos, JConvertToBoolean(false));
		else if (!first_enabled)
			first_enabled = menu_pos;
	}

	// Don't do dynamic updates of menu items
	mAccountsPopup->SetUpdateAction(JXMenu::kDisableNone);

	// Set initial value and fake hit
	mAccountsPopup->SetToPopupChoice(kTrue, first_enabled);
}

