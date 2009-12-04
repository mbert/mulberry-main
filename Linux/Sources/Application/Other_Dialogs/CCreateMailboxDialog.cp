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


// Source for CCreateMailboxDialog class

#include "CCreateMailboxDialog.h"

#include "CINETCommon.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTextField.h"

#include <JXWindow.h>
#include <JXUpRect.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextRadioButton.h>
#include <JXRadioGroup.h>
#include <JXTextCheckbox.h>
#include "TPopupMenu.h"

#include <jXGlobals.h>

#include <string.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CCreateMailboxDialog::CCreateMailboxDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}


// Default destructor
CCreateMailboxDialog::~CCreateMailboxDialog()
{
}

void CCreateMailboxDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 435,305, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 435,305);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Mailbox Name:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,65, 350,20);
    assert( obj2 != NULL );

    mMailboxName =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,85, 390,20);
    assert( mMailboxName != NULL );

    mWhichPath =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,115, 415,80);
    assert( mWhichPath != NULL );

    mFullPathnameBtn =
        new JXTextRadioButton(eFullPathname, "Full Pathname", mWhichPath,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 140,20);
    assert( mFullPathnameBtn != NULL );

    mInHierBtn =
        new JXTextRadioButton(eInHierarchy, "Create In Hierarchy:", mWhichPath,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,25, 140,20);
    assert( mInHierBtn != NULL );

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

    mAccountMenu =
        new HPopupMenu( "", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 100,210, 250,25);
    assert( mAccountMenu != NULL );

    mSubscribe =
        new JXTextCheckbox("Subscribe when Created", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,245, 190,20);
    assert( mSubscribe != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 345,270, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 255,270, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mDirectory =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 415,50);
    assert( mDirectory != NULL );

    mCreateMailbox =
        new JXTextRadioButton(1, "Create a Mailbox to Contain Messages", mDirectory,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 250,20);
    assert( mCreateMailbox != NULL );

    mCreateDirectory =
        new JXTextRadioButton(2, "Create a Mailbox to Contain Mailboxes", mDirectory,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,25, 250,20);
    assert( mCreateDirectory != NULL );

// end JXLayout
	window->SetTitle("Create Mailbox");
	SetButtons(mOKBtn, mCancelBtn);

	// The only way to make it read only
	mInHierarchy->Deactivate();
	mAccount->Deactivate();
	
	InitAccountMenu();
	ListenTo(mAccountMenu);
}

// Called during idle
void CCreateMailboxDialog::InitAccountMenu(void)
{
	// Delete previous items
	mAccountMenu->RemoveAllItems();

	// Add each mail account
	for(CMailAccountList::const_iterator iter = CPreferences::sPrefs->mMailAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mMailAccounts.GetValue().end(); iter++)
		//Append it as a radio button
		mAccountMenu->AppendItem((*iter)->GetName(), kFalse, kTrue);

	mAccountMenu->SetToPopupChoice(kTrue, 1);
}

// Account popup changed
void CCreateMailboxDialog::OnAccountPopup(JIndex index)
{
	// Get account
	CMailAccount* acct = CPreferences::sPrefs->mMailAccounts.GetValue()[index - 1];
	
	// Enable subscribe only for IMAP accounts
	if (acct->GetServerType() == CINETAccount::eIMAP)
	{
		mUseSubscribe = true;
		mSubscribe->Show();
	}
	else
	{
		mUseSubscribe = false;
		mSubscribe->Hide();
	}
}

// Set the details
void CCreateMailboxDialog::SetDetails(SCreateMailbox* create)
{
	// If no account use the popup
	if (create->account.empty())
	{
		mAccount->Hide();
	}
	else
	{
		mAccountMenu->Hide();
		mAccount->SetText(create->account);

		// Hide subscribed if not IMAP
		mUseSubscribe = (create->account_type == CINETAccount::eIMAP);
		if (!mUseSubscribe)
			mSubscribe->Hide();
	}

	mInHierarchy->SetText(create->parent);

	mWhichPath->SelectItem((create->use_wd && !create->parent.empty()) ? eInHierarchy : eFullPathname);
	
	if (create->parent.empty())
		mInHierBtn->Deactivate();
}

// Get the details
void CCreateMailboxDialog::GetDetails(SCreateMailbox* result)
{
	result->directory = (mDirectory->GetSelectedItem() == 2);

	result->new_name = mMailboxName->GetText();

	result->use_wd = (mWhichPath->GetSelectedItem() != eFullPathname);

	// No subscribed if not IMAP
	result->subscribe = mUseSubscribe && (mSubscribe->IsChecked() == 1);

	if (mAccount->IsEmpty())
	{
		short acct_num = mAccountMenu->GetValue() - 1;
		result->account = CPreferences::sPrefs->mMailAccounts.GetValue().at(acct_num)->GetName();
	}
}

void CCreateMailboxDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (sender == mWhichPath && message.Is(JXRadioGroup::kSelectionChanged))
	{
		const JXRadioGroup::SelectionChanged* info =
			dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message);
		assert(info != NULL);
		switch (info->GetID()) {
		case eFullPathname:
			mInHierarchy->Deactivate();
			return;
		case eInHierarchy:
			mInHierarchy->Activate();
			return;
		}				
	}
	else if (message.Is(JXMenu::kItemSelected) && sender == mAccountMenu)
	{
    	const JXMenu::ItemSelected* is = dynamic_cast<const JXMenu::ItemSelected*>(&message);
    	OnAccountPopup(is->GetIndex());
    	return;
	}

	CDialogDirector::Receive(sender, message);
	return;
}

bool CCreateMailboxDialog::PoseDialog(SCreateMailbox& details)
{
	bool result = false;

	CCreateMailboxDialog* dlog = new CCreateMailboxDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetDetails(&details);

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetDetails(&details);
		result = true;
		dlog->Close();
	}

	return result;
}
