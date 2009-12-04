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


// Source for CEditIdentities class

#include "CEditIdentities.h"

#include "CAdminLock.h"
#include "CEditIdentityAddress.h"
#include "CEditIdentityOptions.h"
#include "CEditIdentityOutgoing.h"
#include "CEditIdentitySecurity.h"
#include "CMulberryCommon.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CPrefsSubTab.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>
#include <LPopupButton.h>


// __________________________________________________________________________________________________
// C L A S S __ C E D I T I D E N T I T I E S D I A L O G
// __________________________________________________________________________________________________

enum
{
	tabt_From = 1,
	tabt_ReplyTo,
	tabt_Sender,
	tabt_Options,
	tabt_Outgoing,
	tabt_Security,
	tabt_Calendar
};

enum
{
	eInherit_Default = 1,
	eInherit_First = eInherit_Default + 2
};

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CEditIdentities::CEditIdentities()
{
}

// Constructor from stream
CEditIdentities::CEditIdentities(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CEditIdentities::~CEditIdentities()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CEditIdentities::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get fields
	mName = (CTextFieldX*) FindPaneByID(paneid_IdentityName);
	mInherit = (LPopupButton*) FindPaneByID(paneid_IdentityInherit);
	mServer = (LPopupButton*) FindPaneByID(paneid_IdentityServer);
	mServerInherit = (LCheckBox*) FindPaneByID(paneid_IdentityServerInherit);

	mTabs = (CPrefsSubTab*) FindPaneByID(paneid_IdentityTabs);

	// Disable locked out items
	if (CAdminLock::sAdminLock.mLockServerAddress)
	{
		mServer->Disable();
		mServerInherit->Disable();
	}

	// Set default commander to tabber
	LCommander* defCommander;
	GetSubCommanders().FetchItemAt(1, defCommander);
	SetDefaultCommander(defCommander);

	StopListening();

	short removed = 0;
	if (CAdminLock::sAdminLock.mLockIdentityFrom)
	{
		mTabs->RemoveTabButtonAt(tabt_From - removed++);
		mFromEnabled = false;
	}
	else
	{
		mTabs->AddPanel(paneid_EditIdentityAddress);
		CEditIdentityAddress* panel = (CEditIdentityAddress*) mTabs->GetPanel(tabt_From - 1);
		panel->SetAddressType(true, false, false, false, false, false, false);
		mFromEnabled = true;
	}
	if (CAdminLock::sAdminLock.mLockIdentityReplyTo)
	{
		mTabs->RemoveTabButtonAt(tabt_ReplyTo - removed++);
		mReplyToEnabled = false;
	}
	else
	{
		mTabs->AddPanel(paneid_EditIdentityAddress);
		CEditIdentityAddress* panel = (CEditIdentityAddress*) mTabs->GetPanel(tabt_ReplyTo - removed - 1);
		panel->SetAddressType(false, true, false, false, false, false, false);
		mReplyToEnabled = true;
	}
	if (CAdminLock::sAdminLock.mLockIdentitySender)
	{
		mTabs->RemoveTabButtonAt(tabt_Sender - removed++);
		mSenderEnabled = false;
	}
	else
	{
		mTabs->AddPanel(paneid_EditIdentityAddress);
		CEditIdentityAddress* panel = (CEditIdentityAddress*) mTabs->GetPanel(tabt_Sender - removed - 1);
		panel->SetAddressType(false, false, true, false, false, false, false);
		mSenderEnabled = true;
	}
	mTabs->AddPanel(paneid_EditIdentityOptions);
	mTabs->AddPanel(paneid_EditIdentityOutgoing);
	if (CPluginManager::sPluginManager.HasSecurity())
		mTabs->AddPanel(paneid_EditIdentitySecurity);
	else
		mTabs->RemoveTabButtonAt(tabt_Security - removed++);
	if (CAdminLock::sAdminLock.mLockIdentityFrom)
	{
		mTabs->RemoveTabButtonAt(tabt_Calendar - removed++);
		mCalendarEnabled = false;
	}
	else
	{
		mTabs->AddPanel(paneid_EditIdentityAddress);
		CEditIdentityAddress* panel = (CEditIdentityAddress*) mTabs->GetPanel(tabt_Calendar - removed - 1);
		panel->SetAddressType(false, false, false, false, false, false, true);
		mCalendarEnabled = true;
	}

	StartListening();

	mTabs->SetDisplayPanel(1);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CEditIdentitiesBtns);

	// Set target
	SetLatentSub(mName);
}

// Handle buttons
void CEditIdentities::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_ServerInherit:
		if (*((long*) ioParam))
			mServer->Disable();
		else
			mServer->Enable();
		break;
	case msg_IdentityTabs:
		mTabs->SetDisplayPanel(*(unsigned long*) ioParam);
		break;

	default:
		break;
	}
}

// Set initial identity fields
void CEditIdentities::SetIdentity(const CPreferences* prefs, const CIdentity* identity)
{
	mCopyIdentity = *identity;

	// Make sure SMTP account is valid irrespective of server locked status
	if (mCopyIdentity.GetSMTPAccount(false).empty() && prefs->mSMTPAccounts.GetValue().size())
		mCopyIdentity.SetSMTPAccount(prefs->mSMTPAccounts.GetValue().front()->GetName(), true);

	// Copy text to edit fields
	mName->SetText(identity->GetIdentity());

	InitInheritPopup(prefs, identity);

	InitServerPopup(prefs);
	mServerInherit->SetValue(!identity->UseSMTPAccount());
	if (!identity->UseSMTPAccount())
		mServer->Disable();

	// Set existing panel
	mTabs->SetData(&mCopyIdentity);

	// Look at IC options
	if (prefs->mUse_IC.GetValue())
	{
		// Is it the first identity
		if (&prefs->mIdentities.GetValue().front() == identity)
		{
			if (mFromEnabled)
				mTabs->GetPanel(0)->ToggleICDisplay(true);
			mTabs->GetPanel((mFromEnabled ? 1 : 0) +
							(mReplyToEnabled ? 1 : 0) +
							(mSenderEnabled ? 1 : 0))->ToggleICDisplay(true);
		}
	}
}

// Get final identity fields
void CEditIdentities::GetIdentity(CIdentity* identity)
{
	identity->SetIdentity(mName->GetText());

	identity->SetInherit(::GetPopupMenuItemTextUTF8(mInherit));

	if (CAdminLock::sAdminLock.mLockServerAddress)
		identity->SetSMTPAccount(mCopyIdentity.GetSMTPAccount(false), true);
	else
	{
		identity->SetSMTPAccount(::GetPopupMenuItemTextUTF8(mServer), mServerInherit->GetValue() == 0);
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

void CEditIdentities::InitServerPopup(const CPreferences* prefs)
{
	// Delete previous items
	MenuHandle menuH = mServer->GetMacMenuH();
	for(short i = ::CountMenuItems(menuH); i >= 1; i--)
		::DeleteMenuItem(menuH, i);

	short menu_pos = 1;
	short found_match = 0;

	// Add each SMTP account
	for(CSMTPAccountList::const_iterator iter = prefs->mSMTPAccounts.GetValue().begin();
			iter != prefs->mSMTPAccounts.GetValue().end(); iter++, menu_pos++)
	{
		// Insert item
		::AppendItemToMenu(menuH, menu_pos, (*iter)->GetName());

		// Check for match
		if ((*iter)->GetName() == mCopyIdentity.GetSMTPAccount(false))
			found_match = menu_pos;
	}

	// Force max/min update
	mServer->SetMenuMinMax();

	// Set initial value
	if (!found_match && prefs->mSMTPAccounts.GetValue().size())
		found_match = 1;
	if (found_match)
		mServer->SetValue(found_match);
}

void CEditIdentities::InitInheritPopup(const CPreferences* prefs, const CIdentity* identity)
{
	// Delete previous items
	MenuHandle menuH = mInherit->GetMacMenuH();
	for(short i = ::CountMenuItems(menuH); i >= eInherit_First; i--)
		::DeleteMenuItem(menuH, i);

	short menu_pos = eInherit_First;
	short found_match = 0;

	// Add each identity except this one account
	for(CIdentityList::const_iterator iter = prefs->mIdentities.GetValue().begin();
			iter != prefs->mIdentities.GetValue().end(); iter++, menu_pos++)
	{
		// Not if this one!
		if (&(*iter) == identity)
		{
			menu_pos--;
			continue;
		}

		// Insert item
		::AppendItemToMenu(menuH, menu_pos, (*iter).GetIdentity());

		// Check for match
		if ((*iter).GetIdentity() == mCopyIdentity.GetInherit())
			found_match = menu_pos;
	}

	// Force max/min update
	mInherit->SetMenuMinMax();

	// Set initial value
	if (!found_match && prefs->mIdentities.GetValue().size())
		found_match = eInherit_Default;
	if (found_match)
		mInherit->SetValue(found_match);
}
