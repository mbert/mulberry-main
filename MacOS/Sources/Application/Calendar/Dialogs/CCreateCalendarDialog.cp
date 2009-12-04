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


// Source for CCreateCalendarDialog class

#include "CCreateCalendarDialog.h"

#include "CAdminLock.h"
#include "CBalloonDialog.h"
#include "CINETCommon.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CStaticText.h"
#include "CTextFieldX.h"

#include "CCalendarAccount.h"
#include "CCalendarProtocol.h"
#include "CCalendarStoreManager.h"

#include <LCheckBox.h>
#include <LPopupButton.h>
#include <LRadioButton.h>

#include "MyCFString.h"

#include <string.h>

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CCreateCalendarDialog::CCreateCalendarDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CCreateCalendarDialog::~CCreateCalendarDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CCreateCalendarDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get name
	mCalendar = dynamic_cast<LRadioButton*>(FindPaneByID(eCalendar_ID));
	mDirectory = dynamic_cast<LRadioButton*>(FindPaneByID(eDirectory_ID));
	mCalendarName = dynamic_cast<CTextFieldX*>(FindPaneByID(eName_ID));

	// Get checkbox
	mSubscribe = dynamic_cast<LCheckBox*>(FindPaneByID(eSubscribe_ID));
	mUseSubscribe = true;

	// Get radio buttons
	mFullPath = dynamic_cast<LRadioButton*>(FindPaneByID(eFullPath_ID));
	mUseDirectory = dynamic_cast<LRadioButton*>(FindPaneByID(eUseDirectory_ID));

	// Get captions
	mHierarchy = dynamic_cast<CStaticText*>(FindPaneByID(eHierarchy_ID));
	mAccountBox = dynamic_cast<LView*>(FindPaneByID(eAccountBox_ID));
	mAccount = dynamic_cast<CStaticText*>(FindPaneByID(eAccount_ID));
	mAccountPopup = dynamic_cast<LPopupButton*>(FindPaneByID(eAccountPopup_ID));
	InitAccountMenu();

	// Make text edit field active
	SetLatentSub(mCalendarName);
	mCalendarName->SelectAll();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, pane_ID);

}

// Handle OK button
void CCreateCalendarDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_Calendar:
		if (*((long*) ioParam))
		{
			mSubscribe->Enable();
			mUseSubscribe = true;
		}
		break;

	case msg_Directory:
		if (*((long*) ioParam))
		{
			mSubscribe->Disable();
			mUseSubscribe = false;
		}
		break;

	case msg_SetFullPath:
		if (*((long*) ioParam))
			mHierarchy->Disable();
		break;

	case msg_SetUseDirectory:
		if (*((long*) ioParam))
			mHierarchy->Enable();
		break;

	default:
		LDialogBox::ListenToMessage(inMessage, ioParam);
		break;
	}
}

// Set the details
void CCreateCalendarDialog::SetDetails(SCreateCalendar* create)
{
	// If no account use the popup
	if (create->account.empty())
	{
		mAccountBox->Hide();
	}
	else
	{
		mAccountPopup->Hide();
		MyCFString text(create->account.c_str(), kCFStringEncodingUTF8);
		mAccount->SetCFDescriptor(text);
	}

	MyCFString text(create->parent.c_str(), kCFStringEncodingUTF8);
	mHierarchy->SetCFDescriptor(text);

	if (create->use_wd && !create->parent.empty())
		mUseDirectory->SetValue(1);
	else
		mFullPath->SetValue(1);

	if (create->parent.empty())
	{
		mUseDirectory->Disable();
		mHierarchy->Disable();
	}
}

// Get the details
void CCreateCalendarDialog::GetDetails(SCreateCalendar* result)
{
	result->directory = (mDirectory->GetValue() == 1);
	result->new_name = mCalendarName->GetText();

	result->use_wd = (mFullPath->GetValue() != 1);

	result->subscribe = mUseSubscribe && (mSubscribe->GetValue() == 1);

	// Get account if not specified
	if (result->account.empty())
	{
		result->account = ::GetPopupMenuItemTextUTF8(mAccountPopup);
	}
}

// Called during idle
void CCreateCalendarDialog::InitAccountMenu(void)
{
	// Delete previous items
	MenuHandle menuH = mAccountPopup->GetMacMenuH();
	for(short i = ::CountMenuItems(menuH); i >= 1; i--)
		::DeleteMenuItem(menuH, i);

	// Add to menu
	short menu_pos = 1;
	if (!CAdminLock::sAdminLock.mNoLocalCalendars)
	{
		MyCFString menu_title(CPreferences::sPrefs->mLocalCalendarAccount.GetValue().GetName(), kCFStringEncodingUTF8);
		::AppendMenuItemTextWithCFString(menuH, menu_title, 0, 0, NULL);
		mHasLocal = true;
	}
	
	// Add each mail account
	for(CCalendarAccountList::const_iterator iter = CPreferences::sPrefs->mCalendarAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mCalendarAccounts.GetValue().end(); iter++, menu_pos++)
	{
		// Add to menu
		MyCFString menu_title((*iter)->GetName(), kCFStringEncodingUTF8);
		::AppendMenuItemTextWithCFString(menuH, menu_title, 0, 0, NULL);
		mHasRemote = true;
		
		// Disable if not logged in
		if (!calstore::CCalendarStoreManager::sCalendarStoreManager->GetProtocol((*iter)->GetName())->IsLoggedOn())
			::DisableItem(menuH, menu_pos);
	}

	// Force max/min update
	mAccountPopup->SetMenuMinMax();
}

bool CCreateCalendarDialog::PoseDialog(SCreateCalendar& create)
{
	bool result = false;

	// Create the dialog
	CBalloonDialog	theHandler(pane_ID, CMulberryApp::sApp);
	static_cast<CCreateCalendarDialog*>(theHandler.GetDialog())->SetDetails(&create);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			static_cast<CCreateCalendarDialog*>(theHandler.GetDialog())->GetDetails(&create);
			result = !create.new_name.empty();
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}

	return result;
}
