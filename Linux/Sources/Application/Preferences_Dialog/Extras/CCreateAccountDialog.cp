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


// Source for CCreateAccountDialog class

#include "CCreateAccountDialog.h"

#include "CAdminLock.h"
#include "CTextField.h"

#include "TPopupMenu.h"

#include "cdstring.h"

#include <JXImage.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CCreateAccountDialog::CCreateAccountDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor, kTrue)
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CCreateAccountDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 350,125, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 350,125);
    assert( obj1 != NULL );

    mOkBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 260,90, 70,25);
    assert( mOkBtn != NULL );
    mOkBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 170,90, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    JXStaticText* obj2 =
        new JXStaticText("Account Name:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,22, 95,20);
    assert( obj2 != NULL );

    mText =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 105,20, 230,20);
    assert( mText != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Account Type:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,58, 90,20);
    assert( obj3 != NULL );

    mType =
        new HPopupMenu("",obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 105,55, 210,20);
    assert( mType != NULL );

// end JXLayout

	InitAccountTypePopup();

	window->SetTitle("Create New Account");
	SetButtons(mOkBtn, mCancelBtn);
}

void CCreateAccountDialog::GetDetails(cdstring& change, short& type)
{
	change = cdstring(mText->GetText());
	type = mType->GetValue();
}

void CCreateAccountDialog::InitAccountTypePopup()
{
	mType->SetMenuItems("IMAP Mailbox %r |"
						"POP3 Mailbox %r |"
						"Local Mailbox %r %l |"
						"SMTP Send Email %r %l |"
						"WebDAV Options %r |"
						"IMSP Options %r |"
						"ACAP Options %r %l |"
						"CardDAV Address Books %r |"
						"IMSP Address Books %r |"
						"ACAP Address Books %r %l |"
						"LDAP Address Search %r %l |"
						"SIEVE Filtering - Manage SIEVE %r %l |"
						"CalDAV Calendar %r |"
						"WebDAV Calendar %r");
	mType->SetItemImageID(1, IDI_ACCOUNTMAILBOX);
	mType->SetItemImageID(2, IDI_ACCOUNTMAILBOX);
	mType->SetItemImageID(3, IDI_ACCOUNTMAILBOX);
	mType->SetItemImageID(4, IDI_ACCOUNTSMTP);
	mType->SetItemImageID(5, IDI_ACCOUNTREMOTE);
	mType->SetItemImageID(6, IDI_ACCOUNTREMOTE);
	mType->SetItemImageID(7, IDI_ACCOUNTREMOTE);
	mType->SetItemImageID(8, IDI_ACCOUNTADBK);
	mType->SetItemImageID(9, IDI_ACCOUNTADBK);
	mType->SetItemImageID(10, IDI_ACCOUNTADBK);
	mType->SetItemImageID(11, IDI_ACCOUNTADDRSEARCH);
	mType->SetItemImageID(12, IDI_ACCOUNTSIEVE);
	mType->SetItemImageID(13, IDI_CALENDAR);
	mType->SetItemImageID(14, IDI_CALENDAR);

	// Disable certain types
	if (CAdminLock::sAdminLock.mNoLocalMbox)
	{
		mType->DisableItem(2);
		mType->DisableItem(3);
	}
	if (CAdminLock::sAdminLock.mNoRemotePrefs)
	{
		mType->DisableItem(5);
		mType->DisableItem(6);
	}
	if (CAdminLock::sAdminLock.mPreventCalendars)
	{
		mType->DisableItem(11);
		mType->DisableItem(12);
	}

	mType->SetToPopupChoice(kTrue, 1);
}

bool CCreateAccountDialog::PoseDialog(cdstring& change, short& type)
{
	bool result = false;

	CCreateAccountDialog* dlog = new CCreateAccountDialog(JXGetApplication());

	if (dlog->DoModal() == kDialogClosed_OK)
	{
		dlog->GetDetails(change, type);
		result = true;
		dlog->Close();
	}

	return result;
}
