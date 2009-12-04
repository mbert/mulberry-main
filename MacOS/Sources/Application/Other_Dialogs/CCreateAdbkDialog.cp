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


// Source for CCreateAdbkDialog class

#include "CCreateAdbkDialog.h"

#include "CAdminLock.h"
#include "CAdbkProtocol.h"
#include "CAddressBookManager.h"
#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CStaticText.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>
#include <LPopupButton.h>
#include <LRadioButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CCreateAdbkDialog::CCreateAdbkDialog()
{
}

// Constructor from stream
CCreateAdbkDialog::CCreateAdbkDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CCreateAdbkDialog::~CCreateAdbkDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CCreateAdbkDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get name
	mMkAdbk = dynamic_cast<LRadioButton*>(FindPaneByID(paneid_CrMkAdbk));
	mMkCol = dynamic_cast<LRadioButton*>(FindPaneByID(paneid_CrMkCol));
	mAdbkName = (CTextFieldX*) FindPaneByID(paneid_CrAdbk);

	// Get type items
	mFullPath = dynamic_cast<LRadioButton*>(FindPaneByID(paneid_CrAdbkFullPath));
	mUseDirectory = dynamic_cast<LRadioButton*>(FindPaneByID(paneid_CrAdbkUseDirectory));
	mHierarchy = dynamic_cast<CStaticText*>(FindPaneByID(paneid_CrAdbkHierarchy));

	mMover = (LView*) FindPaneByID(paneid_CrAdbkMover);

	// Get account bits
	mAccountBox = (LView*) FindPaneByID(paneid_CrAdbkAccountBox);
	mAccount = (CStaticText*) FindPaneByID(paneid_CrAdbkAccount);
	mAccountPopup = (LPopupButton*) FindPaneByID(paneid_CrAdbkAccountPopup);
	InitAccountMenu();

	// Get checkbox
	mOpenOnStartup = (LCheckBox*) FindPaneByID(paneid_CrAdbkOpenOnStartup);
	mUseNicknames = (LCheckBox*) FindPaneByID(paneid_CrAdbkUseNicknames);
	mUseSearch = (LCheckBox*) FindPaneByID(paneid_CrAdbkUseSearch);

	// Make text edit field active
	SetLatentSub(mAdbkName);
	mAdbkName->SelectAll();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CCreateAdbkDialogBtns);

}

// Handle OK button
void CCreateAdbkDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_CrAdbkMkAdbk:
		if (*((long*) ioParam))
		{
			mOpenOnStartup->Enable();
			mUseNicknames->Enable();
			mUseSearch->Enable();
		}
		break;

	case msg_CrAdbkMkCol:
		if (*((long*) ioParam))
		{
			mOpenOnStartup->Disable();
			mUseNicknames->Disable();
			mUseSearch->Disable();
		}
		break;

	case msg_CrAdbkSetFullPath:
		if (*((long*) ioParam))
			mHierarchy->Disable();
		break;

	case msg_CrAdbkSetUseDirectory:
		if (*((long*) ioParam))
			mHierarchy->Enable();
		break;

	default:
		LDialogBox::ListenToMessage(inMessage, ioParam);
		break;
	}
}

// Set the details
void CCreateAdbkDialog::SetDetails(SCreateAdbk* create)
{
	// If no account use the popup
	if (create->account.empty())
	{
		mAccountBox->Hide();
	}
	else
	{
		mAccountPopup->Hide();
		mAccount->SetText(create->account);
	}

	mHierarchy->SetText(create->parent);

	mOpenOnStartup->SetValue(create->open_on_startup);
	mUseNicknames->SetValue(create->use_nicknames);
	mUseSearch->SetValue(create->use_search);
	
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
void CCreateAdbkDialog::GetDetails(SCreateAdbk* result)
{
	result->directory = (mMkCol->GetValue() == 1);
	result->name = mAdbkName->GetText();

	result->use_wd = (mFullPath->GetValue() != 1);

	result->open_on_startup = (mOpenOnStartup->GetValue() == 1);
	result->use_nicknames = (mUseNicknames->GetValue() == 1);
	result->use_search = (mUseSearch->GetValue() == 1);

	// Get account if not specified
	if (result->account.empty())
	{
		result->account = ::GetPopupMenuItemTextUTF8(mAccountPopup);
	}
}

// Called during idle
void CCreateAdbkDialog::InitAccountMenu(void)
{
	// Delete previous items
	MenuHandle menuH = mAccountPopup->GetMacMenuH();
	for(short i = ::CountMenuItems(menuH); i >= 1; i--)
		::DeleteMenuItem(menuH, i);

	// Add to menu
	short menu_pos = 1;
	if (!CAdminLock::sAdminLock.mNoLocalAdbks)
	{
		if (UEnvironment::GetOSVersion() >= 0x01020)
			::AppendItemToMenu(menuH, menu_pos++, CPreferences::sPrefs->mOSAdbkAccount.GetValue().GetName());
		::AppendItemToMenu(menuH, menu_pos++, CPreferences::sPrefs->mLocalAdbkAccount.GetValue().GetName());
	}
	
	// Add each adbk account (only IMSP/ACAP)
	for(CAddressAccountList::const_iterator iter = CPreferences::sPrefs->mAddressAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mAddressAccounts.GetValue().end(); iter++)
	{
		// Only if IMSP/ACAP
		if (((*iter)->GetServerType() != CINETAccount::eIMSP) &&
			((*iter)->GetServerType() != CINETAccount::eACAP) &&
			((*iter)->GetServerType() != CINETAccount::eCardDAVAdbk))
			continue;

		// Add to menu
		::AppendItemToMenu(menuH, menu_pos, (*iter)->GetName());
		
		// Disable if not logged in
		if (!CAddressBookManager::sAddressBookManager->GetProtocol((*iter)->GetName())->IsLoggedOn())
			::DisableItem(menuH, menu_pos);
		
		// Update menu id here after we have added an actual item
		menu_pos++;
	}

	// Force max/min update
	mAccountPopup->SetMenuMinMax();
}

// Do the dialog
bool CCreateAdbkDialog::PoseDialog(SCreateAdbk* details)
{
	bool result = false;

	{
		// Create the dialog
		CBalloonDialog	theHandler(paneid_CreateAdbkDialog, CMulberryApp::sApp);
		CCreateAdbkDialog* dlog = static_cast<CCreateAdbkDialog*>(theHandler.GetDialog());
		dlog->SetDetails(details);

		theHandler.StartDialog();

		// Let DialogHandler process events
		MessageT hitMessage;
		while (true)
		{					
			hitMessage = theHandler.DoDialog();
			
			if (hitMessage == msg_OK)
			{
				dlog->GetDetails(details);
				result = true;
				break;
			}
			else if (hitMessage == msg_Cancel)
				break;
		}
	}

	return result;
}
