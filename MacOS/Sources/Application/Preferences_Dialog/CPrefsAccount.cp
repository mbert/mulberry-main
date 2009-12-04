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
#include "CBalloonDialog.h"
#include "CCreateAccountDialog.h"
#include "CErrorHandler.h"
#include "CGetStringDialog.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsAccountPanel.h"
#include "CPrefsAdbkAccount.h"
#include "CPrefsLDAPAccount.h"
#include "CPrefsLocalAccount.h"
#include "CPrefsMailAccount.h"
#include "CPrefsPOP3Account.h"
#include "CPrefsRemoteAccount.h"
#include "CPrefsRemoteCalendarAccount.h"
#include "CPrefsSIEVEAccount.h"
#include "CPrefsSMTPAccount.h"
#include "CMulberryCommon.h"
#include "CStaticText.h"
#include "CTextDisplay.h"
#include "CTextFieldX.h"
#include "CXStringResources.h"

#include <LCheckBox.h>
#include <LPopupButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S A D D R E S S
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAccount::CPrefsAccount()
{
	mCurrentPanel = NULL;
	mCurrentPanelNum = 0;
	mIsSMTP = false;
}

// Constructor from stream
CPrefsAccount::CPrefsAccount(LStream *inStream)
		: CPrefsPanel(inStream)
{
	mCurrentPanel = NULL;
	mCurrentPanelNum = 0;
	mIsSMTP = false;
}

// Default destructor
CPrefsAccount::~CPrefsAccount()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAccount::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsPanel::FinishCreateSelf();

	mAccountPopup = (LPopupButton*) FindPaneByID(paneid_AccountPopup);
	mServerType = (CStaticText*) FindPaneByID(paneid_AccountType);
	mServerIP = (CTextFieldX*) FindPaneByID(paneid_AccountServerIP);
	mPanel = (LView*) FindPaneByID(paneid_AccountPanel);
	mMatchUIDPswd = (LCheckBox*) FindPaneByID(paneid_AccountMatchUIDPswd);

	mAccountValue = -1;

	// Lock out fields
	if (CAdminLock::sAdminLock.mLockServerAddress)
	{
		mServerIP->Disable();

		::DisableItem(mAccountPopup->GetMacMenuH(), eMailAccountPopup_New);
		::DisableItem(mAccountPopup->GetMacMenuH(), eMailAccountPopup_Delete);
	}

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CPrefsAccountBtns);
}

// Handle buttons
void CPrefsAccount::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_MailAccountPopup:
			switch(*(long*) ioParam)
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
				if ((mAccountValue >= 0) && (mAccountValue != *(long*) ioParam - eMailAccountPopup))
					UpdateAccount();

				// Set new value
				mAccountValue = *(long*) ioParam - eMailAccountPopup;
				if (mAccountValue >= 0)
				{
					CINETAccount* acct = NULL;
					long acct_index;
					CINETAccountList* acct_list = NULL;
					GetCurrentAccount(acct, acct_index, acct_list);
					SetAccount(acct);
				}
				break;
			}
			break;

		default:
			break;
	}
}

// Toggle display of IC items
void CPrefsAccount::ToggleICDisplay(void)
{
	// Determine srever IP locks
	if (!CAdminLock::sAdminLock.mLockServerAddress ||
		mIsSMTP && CAdminLock::sAdminLock.mNoLockSMTP)
	{
		mServerIP->Enable();
		Refresh();
	}
	else
	{
		mServerIP->Disable();
		Refresh();
	}

	if (mCopyPrefs->mUse_IC.GetValue())
	{
		// Find out if fist mail or SMTP account
		CINETAccount* acct = NULL;
		long acct_index = 0;
		CINETAccountList* acct_list = NULL;
		GetCurrentAccount(acct, acct_index, acct_list);
		bool lock_it = false;

		switch(acct->GetServerType())
		{
		case CINETAccount::eIMAP:
		case CINETAccount::ePOP3:
		case CINETAccount::eSMTP:
			lock_it = (acct_index == 0);
			break;
		default:;
		}

		if (lock_it)
		{
			if (!CAdminLock::sAdminLock.mLockServerAddress)
			{
				mServerIP->Disable();
				Refresh();
			}

			if (mCurrentPanel)
				mCurrentPanel->ToggleICDisplay(true);

			return;
		}
	}

	if (mCurrentPanel)
		mCurrentPanel->ToggleICDisplay(false);
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
		StopListening();
		mAccountPopup->SetValue(eMailAccountPopup_Off);
		StartListening();
		mAccountPopup->SetValue(mAccountValue + eMailAccountPopup);
	}
	else
	{
		mAccountValue = -1;
		mAccountPopup->SetValue(eMailAccountPopup_Off);
		UpdateItems(false);
	}

	mMatchUIDPswd->SetValue(local_prefs->mRemoteCachePswd.GetValue());

	// Toggle IC display - do after account is first set
	ToggleICDisplay();
}

// Force update of prefs
void CPrefsAccount::UpdatePrefs(void)
{
	// Update current account
	if (mAccountValue >= 0)
		UpdateAccount();

	mCopyPrefs->mRemoteCachePswd.SetValue((mMatchUIDPswd->GetValue()==1));
}

// Set up account menu
void CPrefsAccount::InitAccountMenu(void)
{
	// Delete previous items
	MenuHandle menuH = mAccountPopup->GetMacMenuH();
	for(short i = ::CountMenuItems(menuH); i >= eMailAccountPopup; i--)
		::DeleteMenuItem(menuH, i);

	// Add each mail account
	short menu_pos = eMailAccountPopup;
	for(CMailAccountList::const_iterator iter = mCopyPrefs->mMailAccounts.GetValue().begin();
			iter != mCopyPrefs->mMailAccounts.GetValue().end(); iter++, menu_pos++)
		::AppendItemToMenu(menuH, menu_pos, (*iter)->GetName(), false, cicn_MailAccount);

	// Add each SMTP account
	for(CSMTPAccountList::const_iterator iter = mCopyPrefs->mSMTPAccounts.GetValue().begin();
			iter != mCopyPrefs->mSMTPAccounts.GetValue().end(); iter++, menu_pos++)
		::AppendItemToMenu(menuH, menu_pos, (*iter)->GetName(), false, cicn_SMTPAccount);

	// Add each remote account
	for(COptionsAccountList::const_iterator iter = mCopyPrefs->mRemoteAccounts.GetValue().begin();
			iter != mCopyPrefs->mRemoteAccounts.GetValue().end(); iter++, menu_pos++)
		::AppendItemToMenu(menuH, menu_pos, (*iter)->GetName(), false, cicn_RemoteAccount);

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
		default:
			::AppendItemToMenu(menuH, menu_pos, (*iter)->GetName(), false, cicn_AdbkAccount);
			break;
		case CINETAccount::eLDAP:
			::AppendItemToMenu(menuH, menu_pos, (*iter)->GetName(), false, cicn_AddrSearchAccount);
			break;
		}
	}

	// Add each sieve account
	for(CManageSIEVEAccountList::const_iterator iter = mCopyPrefs->mSIEVEAccounts.GetValue().begin();
			iter != mCopyPrefs->mSIEVEAccounts.GetValue().end(); iter++, menu_pos++)
		::AppendItemToMenu(menuH, menu_pos, (*iter)->GetName(), false, cicn_ManageSIEVEAccount);

	// Add each calendar account
	for(CCalendarAccountList::const_iterator iter = mCopyPrefs->mCalendarAccounts.GetValue().begin();
			iter != mCopyPrefs->mCalendarAccounts.GetValue().end(); iter++, menu_pos++)
		::AppendItemToMenu(menuH, menu_pos, (*iter)->GetName(), false, cicn_CalendarAccount);

	// Force max/min update
	mAccountPopup->SetMenuMinMax();
}


// Add new account
void CPrefsAccount::DoNewAccount(void)
{
	// Get a new name for the mailbox (use old name as starter)
	CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) mSuperView;
	while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
		prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();

	// Update current account
	if (mAccountValue >= 0)
		UpdateAccount();

	// Create the dialog
	{
		CBalloonDialog	theHandler(paneid_CreateAccountDialog, prefs_dlog);
		CCreateAccountDialog* dlog = (CCreateAccountDialog*) theHandler.GetDialog();
		theHandler.StartDialog();

		// Let DialogHandler process events
		MessageT hitMessage;
		while (true)
		{
			hitMessage = theHandler.DoDialog();

			if (hitMessage == msg_OK)
			{
				cdstring new_name;
				short acct_type;

				// Get dialog details
				dlog->GetDetails(new_name, acct_type);

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
				case menu_AccountIMAPMailbox:
				case menu_AccountPOPMailbox:
				case menu_AccountLocalMailbox:
					acct_list = (CINETAccountList*) &mCopyPrefs->mMailAccounts.GetValue();
					break;

				case menu_AccountSMTPMailbox:
					acct_list = (CINETAccountList*) &mCopyPrefs->mSMTPAccounts.GetValue();
					break;

				case menu_AccountWebDAVOptions:
				case menu_AccountIMSPOptions:
				case menu_AccountACAPOptions:
					acct_list = (CINETAccountList*) &mCopyPrefs->mRemoteAccounts.GetValue();
					break;

				case menu_AccountCardDAVAdbk:
				case menu_AccountIMSPAdbk:
				case menu_AccountACAPAdbk:
				case menu_AccountLDAPSearch:
					acct_list = (CINETAccountList*) &mCopyPrefs->mAddressAccounts.GetValue();
					break;
				case menu_AccountManageSIEVE:
					acct_list = (CINETAccountList*) &mCopyPrefs->mSIEVEAccounts.GetValue();
					break;
				case menu_AccountCalDAVCalendar:
				case menu_AccountWebDAVCalendar:
					acct_list = (CINETAccountList*) &mCopyPrefs->mCalendarAccounts.GetValue();
					break;
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
				short insert_icon = 0;
				switch(acct_type)
				{
				case menu_AccountIMAPMailbox:
				case menu_AccountPOPMailbox:
				case menu_AccountLocalMailbox:
					acct = new CMailAccount;
					switch(acct_type)
					{
					case menu_AccountIMAPMailbox:
						acct->SetServerType(CINETAccount::eIMAP);
						break;
					case menu_AccountPOPMailbox:
						acct->SetServerType(CINETAccount::ePOP3);
						break;
					case menu_AccountLocalMailbox:
						acct->SetServerType(CINETAccount::eLocal);
						break;
					}
					acct->SetName(new_name);
					mCopyPrefs->mMailAccounts.Value().push_back((CMailAccount*) acct);
					mCopyPrefs->mMailAccounts.SetDirty();
					insert_pos = mCopyPrefs->mMailAccounts.GetValue().size() - 1;
					insert_icon = cicn_MailAccount;
					break;

				case menu_AccountSMTPMailbox:
					acct = new CSMTPAccount;
					acct->SetServerType(CINETAccount::eSMTP);
					acct->SetName(new_name);
					mCopyPrefs->mSMTPAccounts.Value().push_back((CSMTPAccount*) acct);
					mCopyPrefs->mSMTPAccounts.SetDirty();
					insert_pos = mCopyPrefs->mMailAccounts.GetValue().size() +
									mCopyPrefs->mSMTPAccounts.GetValue().size() - 1;
					insert_icon = cicn_SMTPAccount;
					break;

				case menu_AccountWebDAVOptions:
				case menu_AccountIMSPOptions:
				case menu_AccountACAPOptions:
					acct = new COptionsAccount;
					switch(acct_type)
					{
					case menu_AccountWebDAVOptions:
						acct->SetServerType(CINETAccount::eWebDAVPrefs);
						break;
					case menu_AccountIMSPOptions:
						acct->SetServerType(CINETAccount::eIMSP);
						break;
					case menu_AccountACAPOptions:
						acct->SetServerType(CINETAccount::eACAP);
						break;
					}
					acct->SetName(new_name);
					mCopyPrefs->mRemoteAccounts.Value().push_back((COptionsAccount*)acct);
					mCopyPrefs->mRemoteAccounts.SetDirty();
					insert_pos = mCopyPrefs->mMailAccounts.GetValue().size() +
									mCopyPrefs->mSMTPAccounts.GetValue().size() +
									mCopyPrefs->mRemoteAccounts.GetValue().size() - 1;
					insert_icon = cicn_RemoteAccount;
					break;

				case menu_AccountCardDAVAdbk:
				case menu_AccountIMSPAdbk:
				case menu_AccountACAPAdbk:
				case menu_AccountLDAPSearch:
					acct = new CAddressAccount;
					switch(acct_type)
					{
					case menu_AccountCardDAVAdbk:
						acct->SetServerType(CINETAccount::eCardDAVAdbk);
						insert_icon = cicn_AdbkAccount;
						break;
					case menu_AccountIMSPAdbk:
						acct->SetServerType(CINETAccount::eIMSP);
						insert_icon = cicn_AdbkAccount;
						break;
					case menu_AccountACAPAdbk:
						acct->SetServerType(CINETAccount::eACAP);
						insert_icon = cicn_AdbkAccount;
						break;
					case menu_AccountLDAPSearch:
						acct->SetServerType(CINETAccount::eLDAP);
						insert_icon = cicn_AddrSearchAccount;
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

				case menu_AccountManageSIEVE:
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
					insert_icon = cicn_ManageSIEVEAccount;
					break;

				case menu_AccountCalDAVCalendar:
				case menu_AccountWebDAVCalendar:
					acct = new CCalendarAccount;
					switch(acct_type)
					{
					case menu_AccountCalDAVCalendar:
						acct->SetServerType(CINETAccount::eCalDAVCalendar);
						break;
					case menu_AccountWebDAVCalendar:
						acct->SetServerType(CINETAccount::eWebDAVCalendar);
						break;
					}
					insert_icon = cicn_CalendarAccount;
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

				}

				// Always initialise the account
				acct->NewAccount();

				// Insert item
				MenuHandle menuH = mAccountPopup->GetMacMenuH();
				::InsertMenuItem(menuH, "\p?", insert_pos++ + eMailAccountPopup_Off);
				if (!new_name.empty())
				{
					::SetMenuItemTextUTF8(menuH, insert_pos + eMailAccountPopup_Off, new_name);
				}
				::SetItemIcon(menuH, insert_pos + eMailAccountPopup_Off, insert_icon);

				// Force max/min update
				mAccountPopup->SetMenuMinMax();

				// Enable items if disabled
				if (mAccountValue < 0)
					UpdateItems(true);

				// Set new values
				mAccountValue = insert_pos - 1;
				mAccountPopup->SetValue(mAccountValue + eMailAccountPopup);

				// Add associated preference items
				mCopyPrefs->NewAccount(acct);
				prefs_dlog->SetAccountNew(true);

				// Fix weird appearance manager re-draw bug by forcing redraw here
				// Don't rely on update events which seem to be eaten when controls are
				// actrivated after dialog.
				prefs_dlog->Draw(NULL);
				break;
			}
			else if (hitMessage == msg_Cancel)
			{
				mAccountPopup->SetValue(mAccountValue + eMailAccountPopup);
				break;
			}
		}
	}
}

// Rename account
void CPrefsAccount::DoRenameAccount(void)
{
	// Get a new name for the mailbox (use old name as starter)
	CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) mSuperView;
	while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
		prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();

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

				// Rename account and associated preference items
				mCopyPrefs->RenameAccount(acct, new_name);

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

				// Mail account => need to change mbox ref objects after prefs change
				if (typeid(*acct) == typeid(CMailAccount))
				{
					// Flag rename in dialog
					prefs_dlog->SetAccountRename(true);
					prefs_dlog->GetRenames().push_back(cdstrpair(old_name, new_name));
				}

				// Reset menu
				InitAccountMenu();

				break;
			}
			else
				break;
		}

		// Reset to previous value - will update display
		mAccountPopup->SetValue(mAccountValue + eMailAccountPopup);
	}
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
		// Get the dialog
		CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) mSuperView;
		while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
			prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();

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
			if ((mCopyPrefs->mRemoteAccounts.GetValue().size() == 1) && !prefs_dlog->IsLocal())
			{
				CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::NoDeleteRemoteAccount");
				mAccountPopup->SetValue(mAccountValue + eMailAccountPopup);
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

		prefs_dlog->SetAccountNew(true);

		// Erase from prefs
		acct_list->erase(acct_list->begin() + acct_index);

		// Remove from menu
		MenuHandle menuH = mAccountPopup->GetMacMenuH();
		::DeleteMenuItem(menuH, mAccountValue + eMailAccountPopup);

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
			mAccountPopup->SetValue(mAccountValue + eMailAccountPopup);
		else
		{
			mAccountPopup->SetValue(eMailAccountPopup_Off);
			UpdateItems(false);
		}

		// Fix weird appearance manager re-draw bug by forcing redraw here
		// Don't rely on update events which seem to be eaten when controls are
		// actrivated after dialog.
		prefs_dlog->Draw(NULL);

		// Remove references to this account
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
			mIsSMTP && CAdminLock::sAdminLock.mNoLockSMTP)
			mServerIP->Enable();

		if (mCurrentPanel)
			mCurrentPanel->Enable();

		// Enable menu rename/delete
		::EnableItem(mAccountPopup->GetMacMenuH(), eMailAccountPopup_Rename);
		if (!CAdminLock::sAdminLock.mLockServerAddress)
			::EnableItem(mAccountPopup->GetMacMenuH(), eMailAccountPopup_Delete);
	}
	else
	{
		SetAccount(NULL);
		mServerIP->Disable();
		if (mCurrentPanel)
			mCurrentPanel->Disable();

		// Disable menu rename/delete
		::DisableItem(mAccountPopup->GetMacMenuH(), eMailAccountPopup_Rename);
		::DisableItem(mAccountPopup->GetMacMenuH(), eMailAccountPopup_Delete);
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
			case CINETAccount::eIMSP:
				copyStr.FromResource("UI::Preferences::AccountIMSPAddressBooks");
				break;
			case CINETAccount::eACAP:
				copyStr.FromResource("UI::Preferences::AccountACAPAddressBooks");
				break;
			case CINETAccount::eLDAP:
				copyStr.FromResource("UI::Preferences::AccountLDAPAddressSearch");
				break;
			case CINETAccount::eCardDAVAdbk:
				copyStr.FromResource("UI::Preferences::AccountCardDAVAddressBooks");
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
		mServerType->SetText(copyStr);

		// Copy text to edit fields
		if (account->GetServerType() == CINETAccount::eLocal)
		{
			mServerIP->SetText(cdstring::null_str);
			mServerIP->Hide();
		}
		else
		{
			mServerIP->SetText(account->GetServerIP());
			mServerIP->Show();
		}

		SetPanel(account);
		if (mCurrentPanel)
		{
			mCurrentPanel->Show();
			mCurrentPanel->SetPrefs(mCopyPrefs);
			mCurrentPanel->SetData((void*) account);
		}

		// Toggle IC display - do after account is first set
		ToggleICDisplay();
	}
	else
	{
		// Hide all panels
		if (mCurrentPanel)
			mCurrentPanel->Hide();
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
		cdstring temp = mServerIP->GetText();
		temp.trimspace();
		account->SetServerIP(temp);
	}

	if (mCurrentPanel)
		mCurrentPanel->UpdateData(account);

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
	long acct_index = mAccountValue;
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
	ResIDT panel;

	if (typeid(*account) == typeid(CMailAccount))
	{
		if (typeid(*account) == typeid(CMailAccount))
		{
			switch(account->GetServerType())
			{
			case CINETAccount::eIMAP:
				panel = paneid_PrefsMailAccount;
				break;
			case CINETAccount::ePOP3:
				panel = paneid_PrefsPOP3Account;
				break;
			case CINETAccount::eLocal:
				panel = paneid_PrefsLocalAccount;
				break;
			default:;
			}
		}
	}
	else if (typeid(*account) == typeid(CSMTPAccount))
		panel = paneid_PrefsSMTPAccount;
	else if (typeid(*account) == typeid(COptionsAccount))
		panel = paneid_PrefsRemoteAccount;
	else if (typeid(*account) == typeid(CAddressAccount))
	{
		switch(account->GetServerType())
		{
		case CINETAccount::eIMSP:
		case CINETAccount::eACAP:
		case CINETAccount::eCardDAVAdbk:
			panel = paneid_PrefsAdbkAccount;
			break;
		case CINETAccount::eLDAP:
			panel = paneid_PrefsLDAPAccount;
			break;
		default:
			panel = 0;
		}
	}
	else if (typeid(*account) == typeid(CManageSIEVEAccount))
		panel = paneid_PrefsSIEVEAccount;
	else if (typeid(*account) == typeid(CCalendarAccount))
	{
		switch(account->GetServerType())
		{
		case CINETAccount::eWebDAVCalendar:
		case CINETAccount::eCalDAVCalendar:
			panel = paneid_PrefsRemoteCalendarAccount;
			break;
		default:
			panel = 0;
		}
	}

	// Only if different
	if (mCurrentPanelNum == panel)
		return;

	// First remove and update any existing panel
	mPanel->DeleteAllSubPanes();

	// Update to new panel id
	mCurrentPanelNum = panel;

	// Make panel area default so new panel is automatically added to it
	SetDefaultView(mPanel);
	if (panel)
	{
		mPanel->Hide();
		CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) mSuperView;
		while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
			prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();
		LCommander* defCommander;
		prefs_dlog->GetSubCommanders().FetchItemAt(1, defCommander);
		prefs_dlog->SetDefaultCommander(defCommander);
		mCurrentPanel = (CPrefsAccountPanel*) UReanimator::ReadObjects('PPob', panel);
		mCurrentPanel->FinishCreate();
		mPanel->Show();
	}
	else
	{
		mCurrentPanel = NULL;
		mPanel->Refresh();
	}
}

