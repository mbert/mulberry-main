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


// CEditIdentities.cpp : implementation file
//

#include "CEditIdentities.h"

#include "CAdminLock.h"
#include "CEditIdentityAddress.h"
#include "CEditIdentityOptions.h"
#include "CEditIdentityOutgoing.h"
#include "CEditIdentitySecurity.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CTabController.h"
#include "CTextField.h"

#include "TPopupMenu.h"

#include <JXWindow.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXUpRect.h>

#include <jXGlobals.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CEditIdentities dialog


CEditIdentities::CEditIdentities(JXDirector* supervisor)
	: CDialogDirector(supervisor, kTrue)
{
}

void CEditIdentities::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 470,375, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 470,375);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Identity:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,12, 80,20);
    assert( obj2 != NULL );

    mName =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 110,10, 220,20);
    assert( mName != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Inherit From:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,42, 95,20);
    assert( obj3 != NULL );

    mInheritPopup =
        new HPopupMenu("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 110,40, 220,25);
    assert( mInheritPopup != NULL );

    JXStaticText* obj4 =
        new JXStaticText("SMTP Account:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,72, 100,20);
    assert( obj4 != NULL );

    mAccountPopup =
        new HPopupMenu("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 110,70, 220,25);
    assert( mAccountPopup != NULL );

    mUseSMTPAccount =
        new JXTextCheckbox("Inherit", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 340,72, 75,20);
    assert( mUseSMTPAccount != NULL );

    mTabs =
        new CTabController(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,100, 450,225);
    assert( mTabs != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 380,340, 70,25);
    assert( mOKBtn != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 290,340, 70,25);
    assert( mCancelBtn != NULL );

// end JXLayout

	window->SetTitle("Edit Identity");
	SetButtons(mOKBtn, mCancelBtn);

	// Init controls
	mName->SetText(mCopyIdentity.GetIdentity());
	InitServerPopup();
	InitInheritPopup();

	// Disable locked out items
	if (CAdminLock::sAdminLock.mLockServerAddress)
	{
		mAccountPopup->Deactivate();
		mUseSMTPAccount->Deactivate();
	}
	else
	{
		mUseSMTPAccount->SetState(JBoolean(!mCopyIdentity.UseSMTPAccount()));
		if (!mCopyIdentity.UseSMTPAccount())
			mAccountPopup->Deactivate();
	}
	ListenTo(mUseSMTPAccount);

	// Create tab panels
	int index = 0;
	mFromEnabled = !CAdminLock::sAdminLock.mLockIdentityFrom;
	if (mFromEnabled)
	{
		CEditIdentityAddress* panel_from = new CEditIdentityAddress(false, mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450, 150);
		panel_from->SetAddressType(true, false, false, false, false, false, false);
		cdstring title;
		title.FromResource(IDS_IDENTITY_PANEL_FROM);
		mTabs->AppendCard(panel_from, title);
	}

	mReplyToEnabled = !CAdminLock::sAdminLock.mLockIdentityReplyTo;
	if (mReplyToEnabled)
	{
		CEditIdentityAddress* panel_reply_to = new CEditIdentityAddress(false, mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450, 150);
		panel_reply_to->SetAddressType(false, true, false, false, false, false, false);
		cdstring title;
		title.FromResource(IDS_IDENTITY_PANEL_REPLYTO);
		mTabs->AppendCard(panel_reply_to, title);
	}

	mSenderEnabled = !CAdminLock::sAdminLock.mLockIdentitySender;
	if (mSenderEnabled)
	{
		CEditIdentityAddress* panel_sender = new CEditIdentityAddress(false, mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450, 150);
		panel_sender->SetAddressType(false, false, true, false, false, false, false);
		cdstring title;
		title.FromResource(IDS_IDENTITY_PANEL_SENDER);
		mTabs->AppendCard(panel_sender, title);
	}

	CEditIdentityOptions* panel_options = new CEditIdentityOptions(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450, 150);
	panel_options->SetPrefs(mCopyPrefs);
	mTabs->AppendCard(panel_options, "Options");

	CEditIdentityOutgoing* panel_outgoing = new CEditIdentityOutgoing(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450, 150);
	mTabs->AppendCard(panel_outgoing, "Outgoing");

	if (CPluginManager::sPluginManager.HasSecurity())
	{
		CEditIdentitySecurity* panel_security = new CEditIdentitySecurity(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450, 150);
		mTabs->AppendCard(panel_security, "Security");
	}

	mCalendarEnabled = !CAdminLock::sAdminLock.mLockIdentityFrom;
	if (mFromEnabled)
	{
		CEditIdentityAddress* panel_calendar = new CEditIdentityAddress(false, mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450, 150);
		panel_calendar->SetAddressType(false, false, false, false, false, false, true);
		mTabs->AppendCard(panel_calendar, "Calendar");
	}

	// Give data to tab panels
	mTabs->SetData(&mCopyIdentity);

	// Set to intial panel
	mTabs->ShowCard(1);
}

// Handle buttons
void CEditIdentities::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mUseSMTPAccount)
		{
			if (mUseSMTPAccount->IsChecked())
				mAccountPopup->Deactivate();
			else
				mAccountPopup->Activate();
			return;
		}
	}
	
	CDialogDirector::Receive(sender, message);
}

/////////////////////////////////////////////////////////////////////////////
// CEditIdentities message handlers

// Set initial identity fields
void CEditIdentities::SetIdentity(const CPreferences* prefs, const CIdentity* identity)
{
	mCopyIdentity = *identity;
	mCopyPrefs = prefs;

	// Make sure SMTP account is valid irrespective of server locked status
	if (mCopyIdentity.GetSMTPAccount(false).empty() && prefs->mSMTPAccounts.GetValue().size())
		mCopyIdentity.SetSMTPAccount(prefs->mSMTPAccounts.GetValue().front()->GetName(), false);

	// Other items will get inited in OnCreate
}

// Get final identity fields
void CEditIdentities::GetIdentity(CIdentity* identity)
{
	identity->SetIdentity(cdstring(mName->GetText()));

	identity->SetInherit(cdstring(mInheritPopup->GetCurrentItemText()));

	if (CAdminLock::sAdminLock.mLockServerAddress)
		identity->SetSMTPAccount(mCopyIdentity.GetSMTPAccount(false), false);
	else
	{
		identity->SetSMTPAccount(cdstring(mAccountPopup->GetCurrentItemText()), !mUseSMTPAccount->IsChecked());
	}

	// Update existing panel
	mTabs->UpdateData(identity);

	if (CAdminLock::sAdminLock.mLockIdentityFrom)
		identity->SetFrom(mCopyIdentity.GetFrom(), mCopyIdentity.UseFrom());

	if (CAdminLock::sAdminLock.mLockIdentityReplyTo)
		identity->SetReplyTo(mCopyIdentity.GetReplyTo(), mCopyIdentity.UseReplyTo());

	if (CAdminLock::sAdminLock.mLockIdentitySender)
		identity->SetSender(mCopyIdentity.GetSender(), mCopyIdentity.UseSender());

	if (!CAdminLock::sAdminLock.mAllowXHeaders)
		identity->SetHeader(mCopyIdentity.GetHeader(), mCopyIdentity.UseHeader());
}

void CEditIdentities::InitServerPopup()
{
	// Remove any existing items
	mAccountPopup->RemoveAllItems();

	// Now add current items
	JIndex menu_id = 1;
	JIndex found_match = 0;
	for(CSMTPAccountList::const_iterator iter = mCopyPrefs->mSMTPAccounts.GetValue().begin();
			iter != mCopyPrefs->mSMTPAccounts.GetValue().end(); iter++, menu_id++)
	{
		mAccountPopup->AppendItem((*iter)->GetName(), kFalse, kTrue);
		
		// Check for match
		if ((*iter)->GetName() == mCopyIdentity.GetSMTPAccount(false))
			found_match = menu_id;
	}

	// Set initial value
	if (!found_match && mCopyPrefs->mSMTPAccounts.GetValue().size())
		found_match = 1;
	if (found_match)
		mAccountPopup->SetValue(found_match);
}

void CEditIdentities::InitInheritPopup()
{
	// Remove any existing items
	mInheritPopup->RemoveAllItems();

	// Now add current items
	mInheritPopup->SetMenuItems("Default Identity%r %l");
	JIndex menu_id = 2;
	JIndex found_match = 0;

	// Add each identity except this one account
	for(CIdentityList::const_iterator iter = mCopyPrefs->mIdentities.GetValue().begin();
			iter != mCopyPrefs->mIdentities.GetValue().end(); iter++, menu_id++)
	{
		// Not if this one!
		if ((*iter).GetIdentity() == mCopyIdentity.GetIdentity())
		{
			menu_id--;
			continue;
		}

		// Insert item
		mInheritPopup->AppendItem((*iter).GetIdentity(), kFalse, kTrue);

		// Check for match
		if ((*iter).GetIdentity() == mCopyIdentity.GetInherit())
			found_match = menu_id;
	}

	// Set initial value
	if (!found_match && mCopyPrefs->mIdentities.GetValue().size())
		found_match = 1;
	if (found_match)
		mInheritPopup->SetValue(found_match);
}

bool CEditIdentities::PoseDialog(const CPreferences* prefs, CIdentity* identity)
{
	bool result = false;

	CEditIdentities* dlog = new CEditIdentities(JXGetApplication());
	dlog->SetIdentity(prefs, identity);
	dlog->OnCreate();

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetIdentity(identity);
		result = true;
		dlog->Close();
	}

	return result;
}
