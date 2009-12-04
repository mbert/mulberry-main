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
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTextField.h"

#include "TPopupMenu.h"

#include <JXWindow.h>
#include <JXUpRect.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextRadioButton.h>
#include <JXRadioGroup.h>
#include <JXTextCheckbox.h>

#include <jXGlobals.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CCreateAdbkDialog::CCreateAdbkDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

// Default destructor
CCreateAdbkDialog::~CCreateAdbkDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CCreateAdbkDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 435,345, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 435,345);
    assert( obj1 != NULL );

    mDirectoryGroup =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 415,50);
    assert( mDirectoryGroup != NULL );

    mMkAdbk =
        new JXTextRadioButton(1, "Create an Address Book", mDirectoryGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 250,20);
    assert( mMkAdbk != NULL );

    mMkCol =
        new JXTextRadioButton(2, "Create an Address Book Hierarchy", mDirectoryGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,25, 250,20);
    assert( mMkCol != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Name:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,65, 350,20);
    assert( obj2 != NULL );

    mAdbkName =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,85, 390,20);
    assert( mAdbkName != NULL );

    mWhichPath =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,115, 415,80);
    assert( mWhichPath != NULL );

    mFullPath =
        new JXTextRadioButton(eFullPathname, "Full Pathname", mWhichPath,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 140,20);
    assert( mFullPath != NULL );

    mUseDirectory =
        new JXTextRadioButton(eInHierarchy, "Create In Hierarchy:", mWhichPath,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,25, 140,20);
    assert( mUseDirectory != NULL );

    mInHierarchy =
        new CTextInputField(mWhichPath,
                    JXWidget::kHElastic, JXWidget::kVElastic, 30,50, 370,20);
    assert( mInHierarchy != NULL );

    JXStaticText* obj3 =
        new JXStaticText("In Account:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,212, 80,20);
    assert( obj3 != NULL );

    mAccount =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 100,210, 310,20);
    assert( mAccount != NULL );

    mAccountPopup =
        new HPopupMenu("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 100,210, 250,25);
    assert( mAccountPopup != NULL );

    mOpenOnStartup =
        new JXTextCheckbox("Open on Startup", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,245, 190,20);
    assert( mOpenOnStartup != NULL );

    mUseNicknames =
        new JXTextCheckbox("Use for Nicknames", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,270, 190,20);
    assert( mUseNicknames != NULL );

    mUseSearch =
        new JXTextCheckbox("Use for Searching", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,295, 190,20);
    assert( mUseSearch != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 345,310, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 255,310, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

// end JXLayout

	window->SetTitle("Create Address Book");
	SetButtons(mOKBtn, mCancelBtn);

	mInHierarchy->Deactivate();
	mAccount->Deactivate();

	InitAccountMenu();
	ListenTo(mAccountPopup);

}

void CCreateAdbkDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		const JXRadioGroup::SelectionChanged* info =
			dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message);
		assert(info != NULL);
		if (sender == mWhichPath)
		{
			mInHierarchy->SetActive(info->GetID() == eInHierarchy);
		}
		else if (sender == mDirectoryGroup)
		{
			mOpenOnStartup->SetActive(info->GetID() == 1);
			mUseNicknames->SetActive(info->GetID() == 1);
			mUseSearch->SetActive(info->GetID() == 1);
		}
		return;
	}

	CDialogDirector::Receive(sender, message);
}

// Called during idle
void CCreateAdbkDialog::InitAccountMenu(void)
{
	// Delete previous items
	mAccountPopup->RemoveAllItems();

	JIndex menu_pos = 1;
	if (!CAdminLock::sAdminLock.mNoLocalAdbks)
	{
		mAccountPopup->AppendItem(CPreferences::sPrefs->mLocalAdbkAccount.GetValue().GetName(), kFalse, kTrue);
		menu_pos++;
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
		mAccountPopup->AppendItem((*iter)->GetName(), kFalse, kTrue);
		
		// Disable if not logged in
		if (!CAddressBookManager::sAddressBookManager->GetProtocol((*iter)->GetName())->IsLoggedOn())
			mAccountPopup->DisableItem(menu_pos);
		
		// Update menu id here after we have added an actual item
		menu_pos++;
	}

	mAccountPopup->SetToPopupChoice(kTrue, 1);
}

// Set the details
void CCreateAdbkDialog::SetDetails(SCreateAdbk* create)
{
	// If no account use the popup
	if (create->account.empty())
	{
		mAccount->Hide();
	}
	else
	{
		mAccountPopup->Hide();
		mAccount->SetText(create->account);
	}

	mInHierarchy->SetText(create->parent);

	mOpenOnStartup->SetState(JBoolean(create->open_on_startup));
	mUseNicknames->SetState(JBoolean(create->use_nicknames));
	mUseSearch->SetState(JBoolean(create->use_search));
	
	mWhichPath->SelectItem((create->use_wd && !create->parent.empty()) ? eInHierarchy : eFullPathname);

	if (create->parent.empty())
	{
		mUseDirectory->Deactivate();
		mInHierarchy->Deactivate();
	}
}

// Get the details
void CCreateAdbkDialog::GetDetails(SCreateAdbk* result)
{
	result->directory = (mDirectoryGroup->GetSelectedItem() == 2);
	result->name = mAdbkName->GetText();

	result->use_wd = (mWhichPath->GetSelectedItem() != eFullPathname);

	result->open_on_startup = mOpenOnStartup->IsChecked();
	result->use_nicknames = mUseNicknames->IsChecked();
	result->use_search = mUseSearch->IsChecked();

	// Get account if not specified
	if (result->account.empty())
		result->account = mAccountPopup->GetItemText(mAccountPopup->GetValue());
}

// Do the dialog
bool CCreateAdbkDialog::PoseDialog(SCreateAdbk* details)
{
	bool result = false;

	CCreateAdbkDialog* dlog = new CCreateAdbkDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetDetails(details);

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetDetails(details);
		result = true;
		dlog->Close();
	}

	return result;
}
