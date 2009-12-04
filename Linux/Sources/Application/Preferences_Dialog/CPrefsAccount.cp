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


// Source for CPrefsAccount class

#include "CPrefsAccount.h"

#include "CAdminLock.h"
#include "CCreateAccountDialog.h"
#include "CErrorHandler.h"
#include "CGetStringDialog.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsAccountPanel.h"
#include "CPrefsMailAccount.h"
#include "CPrefsPOP3Account.h"
#include "CPrefsLocalAccount.h"
#include "CPrefsSMTPAccount.h"
#include "CPrefsRemoteAccount.h"
#include "CPrefsRemoteCalendarAccount.h"
#include "CPrefsAdbkAccount.h"
#include "CPrefsLDAPAccount.h"
#include "CPrefsSIEVEAccount.h"
#include "CTextField.h"
#include "CXStringResources.h"

#include "TPopupMenu.h"

#include <JXCardFile.h>
#include <JXColormap.h>
#include <JXEngravedRect.h>
#include <JXImage.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>

#include <cassert>
#include <typeinfo>

enum
{
	eMailAccountPopup_New = 1,
	eMailAccountPopup_Rename,
	eMailAccountPopup_Delete,
	eMailAccountPopup
};

enum
{
	eIMAPAccountPanel = 1,
	ePOP3AccountPanel,
	eLocalAccountPanel,
	eSMTPAccountPanel,
	eRemoteAccountPanel,
	eAdbkAccountPanel,
	eAddrSearchAccountPanel,
	eManageSIEVEAccountPanel,
	eCalendarAccountPanel
};

enum
{
	eAccountIMAPMailbox = 1,
	eAccountPOP3Mailbox,
	eAccountLocalMailbox,
	eAccountSMTPMailbox,
	eAccountWebDAVOptions,
	eAccountIMSPOptions,
	eAccountACAPOptions,
	eAccountCardDAVAdbk,
	eAccountIMSPAdbk,
	eAccountACAPAdbk,
	eAccountLDAPSearch,
	eAccountManageSIEVE,
	eAccountCalDAVCalendar,
	eAccountWebDAVCalendar
};

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S A D D R E S S
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAccount::CPrefsAccount(JXContainer* enclosure,
								const HSizingOption hSizing, 
								const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) 
	: CPrefsPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
	mIsSMTP = false;
}

CPrefsAccount::~CPrefsAccount() 
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CPrefsAccount::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Accounts Preferences", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 140,20);
    assert( obj1 != NULL );
    const JFontStyle obj1_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj1->SetFontStyle(obj1_style);

    JXEngravedRect* obj2 =
        new JXEngravedRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,30, 380,345);
    assert( obj2 != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Account:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,23, 60,20);
    assert( obj3 != NULL );

    mAccountPopup =
        new HPopupMenu("",this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,20, 190,20);
    assert( mAccountPopup != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Type:", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,20, 45,20);
    assert( obj4 != NULL );

    mServerType =
        new JXStaticText("", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 70,20, 255,20);
    assert( mServerType != NULL );
    mServerType->SetBorderWidth(kJXDefaultBorderWidth);

    mServerIPTitle =
        new JXStaticText("Server:", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,45, 50,20);
    assert( mServerIPTitle != NULL );

    mServerIP =
        new CTextInputField(obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 70,45, 255,20);
    assert( mServerIP != NULL );

    mMatchUIDPswd =
        new JXTextCheckbox("Match User ID and Password", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,375, 195,20);
    assert( mMatchUIDPswd != NULL );

    mPanels =
        new JXCardFile( obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,75, 360,260);
    assert( mPanels != NULL );

// end JXLayout1

	// Create tab panels
	AddPanel(new CPrefsMailAccount(mPanels, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 360, 260));
	AddPanel(new CPrefsPOP3Account(mPanels, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 360, 260));
	AddPanel(new CPrefsLocalAccount(mPanels, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 360, 260));
	AddPanel(new CPrefsSMTPAccount(mPanels, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 360, 260));
	AddPanel(new CPrefsRemoteAccount(mPanels, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 360, 260));
	AddPanel(new CPrefsAdbkAccount(mPanels, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 360, 260));
	AddPanel(new CPrefsLDAPAccount(mPanels, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 360, 260));
	AddPanel(new CPrefsSIEVEAccount(mPanels, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 360, 260));
	AddPanel(new CPrefsRemoteCalendarAccount(mPanels, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 360, 260));

	// Set to intial panel
	mPanels->ShowCard(1);

	// Lock out fields
	if (CAdminLock::sAdminLock.mLockServerAddress)
		mServerIP->Deactivate();

	// Start listening
	ListenTo(mAccountPopup);
}

void CPrefsAccount::AddPanel(CPrefsAccountPanel* panel)
{
	panel->OnCreate();
	mPanels->AppendCard(panel);
}

// Handle buttons
void CPrefsAccount::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXMenu::kItemSelected))
	{
		JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
    	if (sender == mAccountPopup)
    	{
			switch(index)
			{
			case eMailAccountPopup_New:
				DoNewAccount();
				break;
			case eMailAccountPopup_Rename:
				DoRenameAccount();
				break;
			case eMailAccountPopup_Delete:
				DoDeleteAccount();
				break;
			default:
				// Update existing account
				if ((mAccountValue >= 0) && (mAccountValue != index - eMailAccountPopup))
					UpdateAccount();
				
				// Set new value
				mAccountValue = index - eMailAccountPopup;
				if (mAccountValue >= 0)
				{
					CINETAccount* acct = nil;
					long acct_index;
					CINETAccountList* acct_list = nil;
					GetCurrentAccount(acct, acct_index, acct_list);
					SetAccount(acct);
				}
				break;
			}
			return;
		}
	}
	
	CPrefsPanel::Receive(sender, message);
}

// Set prefs
void CPrefsAccount::SetPrefs(CPreferences* copyPrefs)
{
	// Save ref to prefs
	mCopyPrefs = copyPrefs;
	CPreferences* local_prefs = copyPrefs;

	// Init the acount menu
	InitAccountMenu();

	// Set initial value
	if (mCopyPrefs->mMailAccounts.GetValue().size() ||
		mCopyPrefs->mSMTPAccounts.GetValue().size() ||
		mCopyPrefs->mRemoteAccounts.GetValue().size() ||
		mCopyPrefs->mAddressAccounts.GetValue().size() ||
		mCopyPrefs->mSIEVEAccounts.GetValue().size() ||
		mCopyPrefs->mCalendarAccounts.GetValue().size())
	{
		mAccountValue = 0;
		UpdateItems(true);
		SetListening(kFalse);
		mAccountPopup->SetValue(eMailAccountPopup);
		SetListening(kTrue);

		// Now setup initial panel
		CINETAccount* acct = nil;
		long acct_index;
		CINETAccountList* acct_list = NULL;
		GetCurrentAccount(acct, acct_index, acct_list);
		SetAccount(acct);
	}
	else
	{
		mAccountValue = -1;
		mAccountPopup->SetValue(eMailAccountPopup_New);
		UpdateItems(false);
	}

	mMatchUIDPswd->SetState(JBoolean(local_prefs->mRemoteCachePswd.GetValue()));
}

// Force update of prefs
void CPrefsAccount::UpdatePrefs(CPreferences* prefs)
{
	// Update current account
	if (mAccountValue >= 0)
		UpdateAccount();

	mCopyPrefs->mRemoteCachePswd.SetValue(mMatchUIDPswd->IsChecked());
}

// Set up account menu
void CPrefsAccount::InitAccountMenu(void)
{
	// Delete previous items
	mAccountPopup->RemoveAllItems();
	mAccountPopup->SetMenuItems("New... |"
								"Rename...|"
								"Delete... %l");

	// Add each mail account
	short menu_pos = eMailAccountPopup;
	for(CMailAccountList::const_iterator iter = mCopyPrefs->mMailAccounts.GetValue().begin();
			iter != mCopyPrefs->mMailAccounts.GetValue().end(); iter++, menu_pos++)
	{
		mAccountPopup->AppendItem((*iter)->GetName(), kFalse, kTrue);
		mAccountPopup->SetItemImageID(mAccountPopup->GetItemCount(), IDI_ACCOUNTMAILBOX);
	}

	// Add each SMTP account
	for(CSMTPAccountList::const_iterator iter = mCopyPrefs->mSMTPAccounts.GetValue().begin();
			iter != mCopyPrefs->mSMTPAccounts.GetValue().end(); iter++, menu_pos++)
	{
		mAccountPopup->AppendItem((*iter)->GetName(), kFalse, kTrue);
		mAccountPopup->SetItemImageID(mAccountPopup->GetItemCount(), IDI_ACCOUNTSMTP);
	}

	// Add each remote account
	for(COptionsAccountList::const_iterator iter = mCopyPrefs->mRemoteAccounts.GetValue().begin();
			iter != mCopyPrefs->mRemoteAccounts.GetValue().end(); iter++, menu_pos++)
	{
		mAccountPopup->AppendItem((*iter)->GetName(), kFalse, kTrue);
		mAccountPopup->SetItemImageID(mAccountPopup->GetItemCount(), IDI_ACCOUNTREMOTE);
	}

	// Add each address account
	for(CAddressAccountList::const_iterator iter = mCopyPrefs->mAddressAccounts.GetValue().begin();
			iter != mCopyPrefs->mAddressAccounts.GetValue().end(); iter++, menu_pos++)
	{
		// Insert item
		switch((*iter)->GetServerType())
		{
		case CINETAccount::eIMSP:
		case CINETAccount::eACAP:
		case CINETAccount::eCardDAVAdbk:
			mAccountPopup->AppendItem((*iter)->GetName(), kFalse, kTrue);
			mAccountPopup->SetItemImageID(mAccountPopup->GetItemCount(), IDI_ACCOUNTADBK);
			break;
		case CINETAccount::eLDAP:
			mAccountPopup->AppendItem((*iter)->GetName(), kFalse, kTrue);
			mAccountPopup->SetItemImageID(mAccountPopup->GetItemCount(), IDI_ACCOUNTADDRSEARCH);
			break;
		default:;
		}
	}

	// Add each SIEVE account
	for(CManageSIEVEAccountList::const_iterator iter = mCopyPrefs->mSIEVEAccounts.GetValue().begin();
			iter != mCopyPrefs->mSIEVEAccounts.GetValue().end(); iter++, menu_pos++)
	{
		mAccountPopup->AppendItem((*iter)->GetName(), kFalse, kTrue);
		mAccountPopup->SetItemImageID(mAccountPopup->GetItemCount(), IDI_ACCOUNTSIEVE);
	}

	// Add each calendar account
	for(CCalendarAccountList::const_iterator iter = mCopyPrefs->mCalendarAccounts.GetValue().begin();
			iter != mCopyPrefs->mCalendarAccounts.GetValue().end(); iter++, menu_pos++)
	{
		mAccountPopup->AppendItem((*iter)->GetName(), kFalse, kTrue);
		mAccountPopup->SetItemImageID(mAccountPopup->GetItemCount(), IDI_CALENDAR);
	}

	mAccountPopup->SetUpdateAction(JXMenu::kDisableNone);

	// Lock out fields
	if (CAdminLock::sAdminLock.mLockServerAddress)
	{
		mAccountPopup->DisableItem(eMailAccountPopup_New);
		mAccountPopup->DisableItem(eMailAccountPopup_Delete);
	}
}


// Add new account
void CPrefsAccount::DoNewAccount(void)
{
	// Update current account
	if (mAccountValue >= 0)
		UpdateAccount();

	while(true)
	{
		// Let Dialog process events
		cdstring new_name;
		short acct_type;
		if (CCreateAccountDialog::PoseDialog(new_name, acct_type))
		{
			// Empty name
			if (new_name.empty())
			{
				CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::EmptyAccountName");
				continue;
			}

			// Check duplicate name
			const CINETAccountList* acct_list = NULL;
			switch(acct_type)
			{
			case eAccountIMAPMailbox:
			case eAccountPOP3Mailbox:
			case eAccountLocalMailbox:
				acct_list = (CINETAccountList*) &mCopyPrefs->mMailAccounts.GetValue();
				break;

			case eAccountSMTPMailbox:
				acct_list = (CINETAccountList*) &mCopyPrefs->mSMTPAccounts.GetValue();
				break;

			case eAccountWebDAVOptions:
			case eAccountIMSPOptions:
			case eAccountACAPOptions:
				acct_list = (CINETAccountList*) &mCopyPrefs->mRemoteAccounts.GetValue();
				break;

			case eAccountCardDAVAdbk:
			case eAccountIMSPAdbk:
			case eAccountACAPAdbk:
			case eAccountLDAPSearch:
				acct_list = (CINETAccountList*) &mCopyPrefs->mAddressAccounts.GetValue();
				break;
			case eAccountManageSIEVE:
				acct_list = (CINETAccountList*) &mCopyPrefs->mSIEVEAccounts.GetValue();
				break;
			case eAccountCalDAVCalendar:
			case eAccountWebDAVCalendar:
				acct_list = (CINETAccountList*) &mCopyPrefs->mCalendarAccounts.GetValue();
				break;
			default:;
			}
			bool fail = false;
			for(CINETAccountList::const_iterator iter = acct_list->begin(); iter != acct_list->end(); iter++)
			{
				if (new_name == (*iter)->GetName())
				{
					CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::DuplicateAccountName");
					fail = true;
					break;
				}
			}
			if (fail)
				continue;

			// Create new account and insert into menu with icon
			CINETAccount* acct = NULL;
			short insert_pos = 0;
			switch(acct_type)
			{
			case eAccountIMAPMailbox:
			case eAccountPOP3Mailbox:
			case eAccountLocalMailbox:
				acct = new CMailAccount;
				switch(acct_type)
				{
				case eAccountIMAPMailbox:
					acct->SetServerType(CINETAccount::eIMAP);
					break;
				case eAccountPOP3Mailbox:
					acct->SetServerType(CINETAccount::ePOP3);
					break;
				case eAccountLocalMailbox:
					acct->SetServerType(CINETAccount::eLocal);
					break;
				}
				acct->SetName(new_name);
				mCopyPrefs->mMailAccounts.Value().push_back((CMailAccount*) acct);
				mCopyPrefs->mMailAccounts.SetDirty();
				insert_pos = mCopyPrefs->mMailAccounts.GetValue().size() - 1;
				break;

			case eAccountSMTPMailbox:
				acct = new CSMTPAccount;
				acct->SetServerType(CINETAccount::eSMTP);
				acct->SetName(new_name);
				mCopyPrefs->mSMTPAccounts.Value().push_back((CSMTPAccount*) acct);
				mCopyPrefs->mSMTPAccounts.SetDirty();
				insert_pos = mCopyPrefs->mMailAccounts.GetValue().size() +
								mCopyPrefs->mSMTPAccounts.GetValue().size() - 1;
				break;

			case eAccountWebDAVOptions:
			case eAccountIMSPOptions:
			case eAccountACAPOptions:
				acct = new COptionsAccount;
				switch(acct_type)
				{
				case eAccountWebDAVOptions:
					acct->SetServerType(CINETAccount::eWebDAVPrefs);
					break;
				case eAccountIMSPOptions:
					acct->SetServerType(CINETAccount::eIMSP);
					break;
				case eAccountACAPOptions:
					acct->SetServerType(CINETAccount::eACAP);
					break;
				}
				acct->SetName(new_name);
				mCopyPrefs->mRemoteAccounts.Value().push_back((COptionsAccount*)acct);
				mCopyPrefs->mRemoteAccounts.SetDirty();
				insert_pos = mCopyPrefs->mMailAccounts.GetValue().size() +
								mCopyPrefs->mSMTPAccounts.GetValue().size() +
								mCopyPrefs->mRemoteAccounts.GetValue().size() - 1;
				break;

			case eAccountCardDAVAdbk:
			case eAccountIMSPAdbk:
			case eAccountACAPAdbk:
			case eAccountLDAPSearch:
				acct = new CAddressAccount;
				switch(acct_type)
				{
				case eAccountCardDAVAdbk:
					acct->SetServerType(CINETAccount::eCardDAVAdbk);
					break;
				case eAccountIMSPAdbk:
					acct->SetServerType(CINETAccount::eIMSP);
					break;
				case eAccountACAPAdbk:
					acct->SetServerType(CINETAccount::eACAP);
					break;
				case eAccountLDAPSearch:
					acct->SetServerType(CINETAccount::eLDAP);
					break;
				}
				acct->SetName(new_name);
				mCopyPrefs->mAddressAccounts.Value().push_back((CAddressAccount*) acct);
				mCopyPrefs->mAddressAccounts.SetDirty();
				insert_pos = mCopyPrefs->mMailAccounts.GetValue().size() +
								mCopyPrefs->mSMTPAccounts.GetValue().size() +
								mCopyPrefs->mRemoteAccounts.GetValue().size() +
								mCopyPrefs->mAddressAccounts.GetValue().size() - 1;
				break;

			case eAccountManageSIEVE:
				acct = new CManageSIEVEAccount;
				acct->SetServerType(CINETAccount::eManageSIEVE);
				acct->SetName(new_name);
				mCopyPrefs->mSIEVEAccounts.Value().push_back((CManageSIEVEAccount*) acct);
				mCopyPrefs->mSIEVEAccounts.SetDirty();
				insert_pos = mCopyPrefs->mMailAccounts.GetValue().size() +
								mCopyPrefs->mSMTPAccounts.GetValue().size() +
								mCopyPrefs->mRemoteAccounts.GetValue().size() +
								mCopyPrefs->mAddressAccounts.GetValue().size() +
								mCopyPrefs->mSIEVEAccounts.GetValue().size() - 1;
				break;

			case eAccountCalDAVCalendar:
			case eAccountWebDAVCalendar:
				acct = new CCalendarAccount;
				switch(acct_type)
				{
				case eAccountCalDAVCalendar:
					acct->SetServerType(CINETAccount::eCalDAVCalendar);
					break;
				case eAccountWebDAVCalendar:
					acct->SetServerType(CINETAccount::eWebDAVCalendar);
					break;
				}
				acct->SetName(new_name);
				mCopyPrefs->mCalendarAccounts.Value().push_back((CCalendarAccount*) acct);
				mCopyPrefs->mCalendarAccounts.SetDirty();
				insert_pos = mCopyPrefs->mMailAccounts.GetValue().size() +
								mCopyPrefs->mSMTPAccounts.GetValue().size() +
								mCopyPrefs->mRemoteAccounts.GetValue().size() +
								mCopyPrefs->mAddressAccounts.GetValue().size() +
								mCopyPrefs->mSIEVEAccounts.GetValue().size() +
									mCopyPrefs->mCalendarAccounts.GetValue().size() - 1;
				break;
			default:;
			}

			// Always initialise the account
			acct->NewAccount();

			// Reset menu
			InitAccountMenu();

			// Enable items if disabled
			if (mAccountValue < 0)
				UpdateItems(true);

			// Set new values
			mAccountValue = insert_pos;
			mAccountPopup->SetValue(mAccountValue + eMailAccountPopup);
			SetAccount(acct);

			// Add associated preference items
			mCopyPrefs->NewAccount(acct);
			CPreferencesDialog::sPrefsDlog->SetAccountNew(true);
			return;
		}
		else
		{
			mAccountPopup->SetValue(mAccountValue + eMailAccountPopup);
			return;
		}
	}
}

// Rename account
void CPrefsAccount::DoRenameAccount(void)
{
	// Update current account
	if (mAccountValue >= 0)
		UpdateAccount();

	// Create the dialog
	{
		CINETAccount* acct = NULL;
		long acct_index;
		CINETAccountList* acct_list = NULL;
		GetCurrentAccount(acct, acct_index, acct_list);

		while (true)
		{					
			cdstring new_name = acct->GetName();
			
			if (CGetStringDialog::PoseDialog("Alerts::Preferences::RenameAccount", new_name))
			{
				// Empty name
				if (new_name.empty())
				{
					CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::EmptyAccountName");
					continue;
				}

				// Check duplicate name
				const CINETAccountList* acct_list = NULL;
				if (typeid(*acct) == typeid(CMailAccount))
					acct_list = (CINETAccountList*) &mCopyPrefs->mMailAccounts.GetValue();
				else if (typeid(*acct) == typeid(CSMTPAccount))
					acct_list = (CINETAccountList*) &mCopyPrefs->mSMTPAccounts.GetValue();
				else if (typeid(*acct) == typeid(COptionsAccount))
					acct_list = (CINETAccountList*) &mCopyPrefs->mRemoteAccounts.GetValue();
				else if (typeid(*acct) == typeid(CAddressAccount))
					acct_list = (CINETAccountList*) &mCopyPrefs->mAddressAccounts.GetValue();
				else if (typeid(*acct) == typeid(CManageSIEVEAccount))
					acct_list = (CINETAccountList*) &mCopyPrefs->mSIEVEAccounts.GetValue();
				else if (typeid(*acct) == typeid(CCalendarAccount))
					acct_list = (CINETAccountList*) &mCopyPrefs->mCalendarAccounts.GetValue();

				bool fail = false;
				for(CINETAccountList::const_iterator iter = acct_list->begin(); iter != acct_list->end(); iter++)
				{
					if (new_name == (*iter)->GetName())
					{
						CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::DuplicateAccountName");
						fail = true;
						break;
					}
				}
				if (fail)
					continue;

				// Do account rename
				
				// Get old account name
				cdstring old_name = acct->GetName();
				
				// Dirty first while object still exists
				if (typeid(*acct) == typeid(CMailAccount))
					mCopyPrefs->mMailAccounts.SetDirty();
				else if (typeid(*acct) == typeid(CSMTPAccount))
					mCopyPrefs->mSMTPAccounts.SetDirty();
				else if (typeid(*acct) == typeid(COptionsAccount))
					mCopyPrefs->mRemoteAccounts.SetDirty();
				else if (typeid(*acct) == typeid(CAddressAccount))
					mCopyPrefs->mAddressAccounts.SetDirty();
				else if (typeid(*acct) == typeid(CManageSIEVEAccount))
					mCopyPrefs->mSIEVEAccounts.SetDirty();
				else if (typeid(*acct) == typeid(CCalendarAccount))
					mCopyPrefs->mCalendarAccounts.SetDirty();

				// Rename account and associated preference items
				mCopyPrefs->RenameAccount(acct, new_name);

				// Mail account => need to change mbox ref objects after prefs change
				if (typeid(*acct) == typeid(CMailAccount))
				{
					// Flag rename in dialog
					CPreferencesDialog::sPrefsDlog->SetAccountRename(true);
					CPreferencesDialog::sPrefsDlog->GetRenames().push_back(cdstrpair(old_name, new_name));
				}
				
				// Reset menu
				InitAccountMenu();

				break;
			}
			else
				break;
		}
	}

	// Reset to previous value - will update display
	mAccountPopup->SetValue(mAccountValue + eMailAccountPopup);
}

// Delete accoount
void CPrefsAccount::DoDeleteAccount(void)
{
	// Update current account
	if (mAccountValue >= 0)
		UpdateAccount();

	// Check that this is what we want to do
	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Preferences::DeleteAccount") == CErrorHandler::Ok)
	{
		CINETAccount* acct = NULL;
		long acct_index;
		CINETAccountList* acct_list = NULL;
		GetCurrentAccount(acct, acct_index, acct_list);
		
		// Dirty first while object still exists
		if (typeid(*acct) == typeid(CMailAccount))
			mCopyPrefs->mMailAccounts.SetDirty();
		else if (typeid(*acct) == typeid(CSMTPAccount))
			mCopyPrefs->mSMTPAccounts.SetDirty();
		else if (typeid(*acct) == typeid(COptionsAccount))
		{
			// Prevent delete of last remote account if set to remote
			if ((mCopyPrefs->mRemoteAccounts.GetValue().size() == 1) && !CPreferencesDialog::sPrefsDlog->IsLocal())
			{
				CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::NoDeleteRemoteAccount");
				return;
			}

			mCopyPrefs->mRemoteAccounts.SetDirty();
		}
		else if (typeid(*acct) == typeid(CAddressAccount))
			mCopyPrefs->mAddressAccounts.SetDirty();
		else if (typeid(*acct) == typeid(CManageSIEVEAccount))
			mCopyPrefs->mSIEVEAccounts.SetDirty();
		else if (typeid(*acct) == typeid(CCalendarAccount))
			mCopyPrefs->mCalendarAccounts.SetDirty();

		// Delete associated preference items and flag cabinets for update
		mCopyPrefs->DeleteAccount(acct);

		CPreferencesDialog::sPrefsDlog->SetAccountNew(true);

		// Erase from prefs
		acct_list->erase(acct_list->begin() + acct_index);
		
		// Reset menu
		InitAccountMenu();
		
		// Adjust to new value
		mAccountValue--;
		if ((mAccountValue < 0) &&
			(mCopyPrefs->mMailAccounts.GetValue().size() ||
			 mCopyPrefs->mSMTPAccounts.GetValue().size() ||
			 mCopyPrefs->mRemoteAccounts.GetValue().size() ||
			 mCopyPrefs->mAddressAccounts.GetValue().size() ||
			 mCopyPrefs->mSIEVEAccounts.GetValue().size() ||
			 mCopyPrefs->mCalendarAccounts.GetValue().size()))
			mAccountValue = 0;
		
		// Update items
		if (mAccountValue >= 0)
		{
			mAccountPopup->SetValue(mAccountValue + eMailAccountPopup);
			long acct_index;
			CINETAccountList* acct_list = NULL;
			GetCurrentAccount(acct, acct_index, acct_list);
			SetAccount(acct);
		}
		else
		{
			mAccountPopup->SetValue(eMailAccountPopup);
			SetAccount(NULL);
			UpdateItems(false);
		}
	}
	else
		mAccountPopup->SetValue(mAccountValue + eMailAccountPopup);
}

// Delete accoount
void CPrefsAccount::UpdateItems(bool enable)
{
	if (enable)
	{
		
		if (!CAdminLock::sAdminLock.mLockServerAddress ||
			(mIsSMTP && CAdminLock::sAdminLock.mNoLockSMTP))
			mServerIP->Activate();

		mPanels->Activate();

		// Enable menu rename & delete
		mAccountPopup->EnableItem(eMailAccountPopup_Rename);
		if (!CAdminLock::sAdminLock.mLockServerAddress)
			mAccountPopup->EnableItem(eMailAccountPopup_Delete);
	}
	else
	{
		SetAccount(NULL);
		mServerIP->Deactivate();
		mPanels->Deactivate();
		
		// Disable menu rename & delete
		mAccountPopup->DisableItem(eMailAccountPopup_Rename);
		mAccountPopup->DisableItem(eMailAccountPopup_Delete);
	}
}

// Set account details
void CPrefsAccount::SetAccount(const CINETAccount* account)
{
	cdstring copyStr;
	mIsSMTP = false;
	if (account)
	{
		// Do type caption
		if (typeid(*account) == typeid(CMailAccount))
		{
			switch(account->GetServerType())
			{
			case CINETAccount::eIMAP:
				copyStr.FromResource("UI::Preferences::AccountIMAPMailbox");
				break;
			case CINETAccount::ePOP3:
				copyStr.FromResource("UI::Preferences::AccountPOP3Mailbox");
				break;
			case CINETAccount::eLocal:
				copyStr.FromResource("UI::Preferences::AccountLocalMailbox");
				break;
			default:;
			}
		}
		else if (typeid(*account) == typeid(CSMTPAccount))
		{
			copyStr.FromResource("UI::Preferences::AccountSMTPSendMail");
			mIsSMTP = true;
		}
		else if (typeid(*account) == typeid(COptionsAccount))
		{
			switch(account->GetServerType())
			{
			case CINETAccount::eWebDAVPrefs:
				copyStr.FromResource("UI::Preferences::AccountWebDAVOptions");
				break;
			case CINETAccount::eIMSP:
				copyStr.FromResource("UI::Preferences::AccountIMSPOptions");
				break;
			case CINETAccount::eACAP:
				copyStr.FromResource("UI::Preferences::AccountACAPOptions");
				break;
			default:;
			}
		}
		else if (typeid(*account) == typeid(CAddressAccount))
		{
			switch(account->GetServerType())
			{
			case CINETAccount::eCardDAVAdbk:
				copyStr.FromResource("UI::Preferences::AccountCardDAVAddressBooks");
				break;
			case CINETAccount::eIMSP:
				copyStr.FromResource("UI::Preferences::AccountIMSPAddressBooks");
				break;
			case CINETAccount::eACAP:
				copyStr.FromResource("UI::Preferences::AccountACAPAddressBooks");
				break;
			case CINETAccount::eLDAP:
				copyStr.FromResource("UI::Preferences::AccountLDAPAddressSearch");
				break;
			default:;
			}
		}
		else if (typeid(*account) == typeid(CManageSIEVEAccount))
		{
			copyStr.FromResource("UI::Preferences::AccountManageSIEVE");
		}
		else if (typeid(*account) == typeid(CCalendarAccount))
		{
			switch(account->GetServerType())
			{
			case CINETAccount::eCalDAVCalendar:
				copyStr.FromResource("UI::Preferences::AccountCalDAVCalendar");
				break;
			case CINETAccount::eWebDAVCalendar:
				copyStr.FromResource("UI::Preferences::AccountWebDAVCalendar");
				break;
			default:;
			}
		}

		// Copy text to edit fields
		mServerType->SetText(copyStr);
		if (account->GetServerType() == CINETAccount::eLocal)
		{
			mServerIP->SetText(cdstring::null_str);
			mServerIP->Hide();
			mServerIPTitle->Hide();
		}
		else
		{
			mServerIP->SetText(account->GetServerIP());
			mServerIP->Show();
			mServerIPTitle->Show();
		}

		SetPanel(account);
		mPanels->Show();

		// Enable/disable server ip based on admin locks
		if (!CAdminLock::sAdminLock.mLockServerAddress ||
			(mIsSMTP && CAdminLock::sAdminLock.mNoLockSMTP))
			mServerIP->Activate();
		else
			mServerIP->Deactivate();
	}
	else
	{
		// Hide all panels
		mServerType->SetText(copyStr);
		mServerIP->SetText(copyStr);
		mPanels->Hide();
	}
}

// Update current account
void CPrefsAccount::UpdateAccount(void)
{
	// Get existing
	CINETAccount* current = NULL;
	long acct_index;
	CINETAccountList* acct_list = NULL;
	GetCurrentAccount(current, acct_index, acct_list);

	// Create new account to fill in from panel
	CINETAccount* account = NULL;
	if (typeid(*current) == typeid(CMailAccount))
		account = new CMailAccount(*(CMailAccount*) current);
	else if (typeid(*current) == typeid(CSMTPAccount))
		account = new CSMTPAccount(*(CSMTPAccount*) current);
	else if (typeid(*current) == typeid(COptionsAccount))
		account = new COptionsAccount(*(COptionsAccount*) current);
	else if (typeid(*current) == typeid(CAddressAccount))
		account = new CAddressAccount(*(CAddressAccount*) current);
	else if (typeid(*current) == typeid(CManageSIEVEAccount))
		account = new CManageSIEVEAccount(*(CManageSIEVEAccount*) current);
	else if (typeid(*current) == typeid(CCalendarAccount))
		account = new CCalendarAccount(*(CCalendarAccount*) current);
	else
		return;

	// Copy info from panel into prefs
	if (account->GetServerType() != CINETAccount::eLocal)
	{
		cdstring temp(mServerIP->GetText());
		temp.trimspace();
		account->SetServerIP(temp);
	}
	
	static_cast<CPrefsAccountPanel*>(mPanels->GetCurrentCard())->UpdateData(account);
		
	// Now check similarity
	bool same = false;
	if (typeid(*current) == typeid(CMailAccount))
		same = (*((CMailAccount*) current) == *((CMailAccount*) account));
	else if (typeid(*current) == typeid(CSMTPAccount))
		same = (*((CSMTPAccount*) current) == *((CSMTPAccount*) account));
	else if (typeid(*current) == typeid(COptionsAccount))
		same = (*((COptionsAccount*) current) == *((COptionsAccount*) account));
	else if (typeid(*current) == typeid(CAddressAccount))
		same = (*((CAddressAccount*) current) == *((CAddressAccount*) account));
	else if (typeid(*current) == typeid(CManageSIEVEAccount))
		same = (*((CManageSIEVEAccount*) current) == *((CManageSIEVEAccount*) account));
	else if (typeid(*current) == typeid(CCalendarAccount))
		same = (*((CCalendarAccount*) current) == *((CCalendarAccount*) account));
	if (!same)
	{
		delete acct_list->at(acct_index);
		acct_list->at(acct_index) = account;
		if (typeid(*account) == typeid(CMailAccount))
			mCopyPrefs->mMailAccounts.SetDirty();
		else if (typeid(*account) == typeid(CSMTPAccount))
			mCopyPrefs->mSMTPAccounts.SetDirty();
		else if (typeid(*account) == typeid(COptionsAccount))
			mCopyPrefs->mRemoteAccounts.SetDirty();
		else if (typeid(*account) == typeid(CAddressAccount))
			mCopyPrefs->mAddressAccounts.SetDirty();
		else if (typeid(*account) == typeid(CManageSIEVEAccount))
			mCopyPrefs->mSIEVEAccounts.SetDirty();
		else if (typeid(*account) == typeid(CCalendarAccount))
			mCopyPrefs->mCalendarAccounts.SetDirty();
	}
	else
		delete account;
}

// Get current account
void CPrefsAccount::GetCurrentAccount(CINETAccount*& acct, 
												long& index,
												CINETAccountList*& list)
{
	unsigned long acct_index = mAccountValue;
	CINETAccountList* acct_list = NULL;

	// Adjust index for actual account
	acct_list = (CINETAccountList*) &mCopyPrefs->mMailAccounts.GetValue();
	if (acct_index >= acct_list->size())
	{
		acct_index -= acct_list->size();
		acct_list = (CINETAccountList*) &mCopyPrefs->mSMTPAccounts.GetValue();
		if (acct_index >= acct_list->size())
		{
			acct_index -= acct_list->size();
			acct_list = (CINETAccountList*) &mCopyPrefs->mRemoteAccounts.GetValue();
			if (acct_index >= acct_list->size())
			{
				acct_index -= acct_list->size();
				acct_list = (CINETAccountList*) &mCopyPrefs->mAddressAccounts.GetValue();
				if (acct_index >= acct_list->size())
				{
					acct_index -= acct_list->size();
					acct_list = (CINETAccountList*) &mCopyPrefs->mSIEVEAccounts.GetValue();
					if (acct_index >= acct_list->size())
					{
						acct_index -= acct_list->size();
						acct_list = (CINETAccountList*) &mCopyPrefs->mCalendarAccounts.GetValue();
						acct = acct_list->at(acct_index);
						index = acct_index;
						list = acct_list;
					}
					else
					{
						acct = acct_list->at(acct_index);
						index = acct_index;
						list = acct_list;
					}
				}
				else
				{
					acct = acct_list->at(acct_index);
					index = acct_index;
					list = acct_list;
				}
			}
			else
			{
				acct = acct_list->at(acct_index);
				index = acct_index;
				list = acct_list;
			}
		}
		else
		{
			acct = acct_list->at(acct_index);
			index = acct_index;
			list = acct_list;
		}
	}
	else
	{
		acct = acct_list->at(acct_index);
		index = acct_index;
		list = acct_list;
	}
}

// Set panel
void CPrefsAccount::SetPanel(const CINETAccount* account)
{
	int panel = -1;

	if (typeid(*account) == typeid(CMailAccount))
	{
		if (typeid(*account) == typeid(CMailAccount))
		{
			switch(account->GetServerType())
			{
			case CINETAccount::eIMAP:
				panel = eIMAPAccountPanel;
				break;
			case CINETAccount::ePOP3:
				panel = ePOP3AccountPanel;
				break;
			case CINETAccount::eLocal:
				panel = eLocalAccountPanel;
				break;
			default:;
			}
		}
	}
	else if (typeid(*account) == typeid(CSMTPAccount))
		panel = eSMTPAccountPanel;
	else if (typeid(*account) == typeid(COptionsAccount))
		panel = eRemoteAccountPanel;
	else if (typeid(*account) == typeid(CAddressAccount))
	{
		switch(account->GetServerType())
		{
		case CINETAccount::eCardDAVAdbk:
		case CINETAccount::eIMSP:
		case CINETAccount::eACAP:
			panel = eAdbkAccountPanel;
			break;
		case CINETAccount::eLDAP:
			panel = eAddrSearchAccountPanel;
			break;
		default:
			panel = -1;
		}
	}
	else if (typeid(*account) == typeid(CManageSIEVEAccount))
		panel = eManageSIEVEAccountPanel;
	else if (typeid(*account) == typeid(CCalendarAccount))
	{
		switch(account->GetServerType())
		{
		case CINETAccount::eWebDAVCalendar:
		case CINETAccount::eCalDAVCalendar:
			panel = eCalendarAccountPanel;
			break;
		default:
			panel = 0;
		}
	}

	mPanels->ShowCard(panel);
	static_cast<CPrefsAccountPanel*>(mPanels->GetCurrentCard())->SetPrefs(mCopyPrefs);
	static_cast<CPrefsAccountPanel*>(mPanels->GetCurrentCard())->SetData((void*) account);
}

