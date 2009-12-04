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


// Source for COpenMailboxDialog class

#include "COpenMailboxDialog.h"

#include "CBrowseMailboxDialog.h"
#include "CMboxRef.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTextField.h"

#include <JXWindow.h>
#include <JXUpRect.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include "TPopupMenu.h"

#include <jXGlobals.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
COpenMailboxDialog::COpenMailboxDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
	mBrowsed = nil;
}

// Default destructor
COpenMailboxDialog::~COpenMailboxDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void COpenMailboxDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 360,140, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 360,140);
    assert( obj1 != NULL );

    mAccountMenu =
        new HPopupMenu( "", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 75,15, 255,25);
    assert( mAccountMenu != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Name:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,57, 55,20);
    assert( obj2 != NULL );

    mMailboxName =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 75,55, 260,20);
    assert( mMailboxName != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Account:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,20, 60,20);
    assert( obj3 != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 270,105, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 180,105, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mBrowseBtn =
        new JXTextButton("Browse...", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 70,105, 70,25);
    assert( mBrowseBtn != NULL );

// end JXLayout
		window->SetTitle("Open Mailbox");
		SetButtons(mOKBtn, mCancelBtn);

		mMailboxName->ShouldBroadcastAllTextChanged(kTrue);
		ListenTo(mMailboxName);
		mOKBtn->Deactivate();

		ListenTo(mBrowseBtn);

		InitAccountMenu();
}

// Set the dialogs return info
CMbox* COpenMailboxDialog::GetSelectedMbox(void)
{
	// If browse result, use that
	if (mBrowsed)
		return mBrowsed;
	
	// Get account name for mailbox
	cdstring acct_name;
	
	short acct_num = mAccountMenu->GetValue() - 1;
	acct_name = CPreferences::sPrefs->mMailAccounts.GetValue().at(acct_num)->GetName();
	acct_name += cMailAccountSeparator;
	
	acct_name += mMailboxName->GetText();
	
	// Get diur delim for account
	char dir_delim = CPreferences::sPrefs->mMailAccounts.GetValue().at(acct_num)->GetDirDelim();

	// Try to resolve reference - force creation
	CMboxRef ref(acct_name, dir_delim);
	
	return ref.ResolveMbox(true);
}

// Called during idle
void COpenMailboxDialog::InitAccountMenu(void)
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

void COpenMailboxDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (sender == mMailboxName && message.Is(JTextEditor16::kTextChanged))
	{
		if (mMailboxName->GetTextLength())
			mOKBtn->Activate();
		else
			mOKBtn->Deactivate();
		return;
	}
	else if ((sender == mBrowseBtn) && message.Is(JXButton::kPushed))
	{
		// Do browse
		bool ignore = false;
		if (CBrowseMailboxDialog::PoseDialog(true, false, mBrowsed, ignore))
			EndDialog(kDialogClosed_OK);

		return;
	}
		
	CDialogDirector::Receive(sender, message);
	return;
}

bool COpenMailboxDialog::PoseDialog(CMbox*& mbox)
{
	bool result = false;

	COpenMailboxDialog* dlog = new COpenMailboxDialog(JXGetApplication());

	// Test for OK
	if (dlog->DoModal() == kDialogClosed_OK)
	{
		mbox = dlog->GetSelectedMbox();
		result = true;
		dlog->Close();
	}

	return result;
}
