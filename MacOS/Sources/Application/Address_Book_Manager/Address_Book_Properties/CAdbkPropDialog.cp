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


// Source for CAdbkPropDialog class

#include "CAdbkPropDialog.h"

#include "CAddressBook.h"
#include "CAdbkProtocol.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPropAdbkACL.h"
#include "CPropAdbkGeneral.h"

#include "MyCFString.h"

// __________________________________________________________________________________________________
// C L A S S __ C A D B K P R O P D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAdbkPropDialog::CAdbkPropDialog()
{
}

// Constructor from stream
CAdbkPropDialog::CAdbkPropDialog(LStream *inStream)
		: CPropDialog(inStream)
{
}

// Default destructor
CAdbkPropDialog::~CAdbkPropDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAdbkPropDialog::FinishCreateSelf(void)
{
	// Do inherited
	CPropDialog::FinishCreateSelf();
}

// Set up panels
void CAdbkPropDialog::SetUpPanels(void)
{
	// Panel list
	mPanelList.push_back(paneid_PropAdbkGeneral);
	mPanelList.push_back(paneid_PropAdbkACL);

	// Message list
	mMsgPanelList.push_back(msg_SetPropAdbkGeneral);
	mMsgPanelList.push_back(msg_SetPropAdbkACL);

	// Ridl for buttons
	mRidl = RidL_CAdbkPropDialogBtns;
}

// About to show panel
void CAdbkPropDialog::DoPanelInit(void)
{
	((CAdbkPropPanel*) mCurrentPanel)->SetAdbkList(mAdbkList);
}

// Set input panel
void CAdbkPropDialog::SetAdbkList(CAddressBookList* adbk_list)
{
	mAdbkList = adbk_list;

	// Load string title
	cdstring title;
	title.FromResource((mAdbkList->size() == 1 ? "UI::AdbkProps::Title" : "UI::AdbkProps::MultiTitle"));

	// Add name if single mailbox
	if (mAdbkList->size() == 1)
	{
		title += mAdbkList->front()->GetName();
	}

	// Now set window title
	MyCFString temp(title, kCFStringEncodingUTF8);
	SetCFDescriptor(temp);

	// Disable unwanted prefs panels
	CAddressBook* adbk = mAdbkList->front();
	bool logged_in = adbk->GetProtocol()->IsLoggedOn();
	if (!logged_in || !adbk->GetProtocol()->UsingACLs())
	{
		LPane* btn = FindPaneByID(paneid_AdbkPropACLBtn);
		btn->Disable();
	}
}
