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


// CPrefsAccounts.cpp : implementation file
//

#include "CPrefsAccounts.h"

#include "CAdminLock.h"
#include "CCreateAccountDialog.h"
#include "CErrorHandler.h"
#include "CGetStringDialog.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsMailAccount.h"
#include "CPrefsPOP3Account.h"
#include "CPrefsLocalAccount.h"
#include "CPrefsSMTPAccount.h"
#include "CPrefsRemoteAccount.h"
#include "CPrefsRemoteCalendarAccount.h"
#include "CPrefsAdbkAccount.h"
#include "CPrefsLDAPAccount.h"
#include "CPrefsSIEVEAccount.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"
#include "CXStringResources.h"

enum
{
	eIMAPAccountPanel = 0,
	ePOP3AccountPanel,
	eLocalAccountPanel,
	eSMTPAccountPanel,
	eRemoteAccountPanel,
	eAdbkAccountPanel,
	eAddrSearchAccountPanel,
	eSIEVEAccountPanel,
	eRemoteCalendarAccountPanel
};

/////////////////////////////////////////////////////////////////////////////
// CPrefsAccounts dialog

IMPLEMENT_DYNCREATE(CPrefsAccounts, CPrefsPanel)

CPrefsAccounts::CPrefsAccounts()
	: CPrefsPanel(CPrefsAccounts::IDD)
{
	//{{AFX_DATA_INIT(CPrefsAccounts)
	mMatchUIDPswd = FALSE;
	mAccountValue = -1;
	//}}AFX_DATA_INIT
	mIsSMTP = false;
}

void CPrefsAccounts::DoDataExchange(CDataExchange* pDX)
{
	CPrefsPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsAccounts)
	DDX_Control(pDX, IDC_PREFSACCOUNTS_SERVERIP, mServerIP);
	DDX_Control(pDX, IDC_PREFSACCOUNTS_SERVERIP_TITLE, mServerIPTitle);
	DDX_Control(pDX, IDC_PREFSACCOUNTS_TYPE, mServerType);
	DDX_Check(pDX, IDC_MATCHUID, mMatchUIDPswd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsAccounts, CPrefsPanel)
	//{{AFX_MSG_MAP(CPrefsAccounts)
	ON_COMMAND_RANGE(IDM_ACCOUNT_NEW, IDM_ACCOUNTEnd, OnAccountPopup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsAccounts message handlers

void CPrefsAccounts::InitControls(void)
{
	// Subclass buttons
	mAccountPopup.SubclassDlgItem(IDC_PREFSACCOUNTS_POPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mAccountPopup.SetMenu(IDR_POPUP_ACCOUNTS);

	CMenu* pPopup = mAccountPopup.GetPopupMenu();
	if (CAdminLock::sAdminLock.mLockServerAddress)
	{
		pPopup->EnableMenuItem(IDM_ACCOUNT_NEW, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(IDM_ACCOUNT_DELETE, MF_GRAYED | MF_BYCOMMAND);
	}

	mPanels.SubclassDlgItem(IDC_PREFSACCOUNTS_PANELS, this);

	// Create tab panels
	CPrefsMailAccount* imap_account = new CPrefsMailAccount;
	mPanels.AddPanel(imap_account);

	CPrefsPOP3Account* pop3_account = new CPrefsPOP3Account;
	mPanels.AddPanel(pop3_account);

	CPrefsLocalAccount* local_account = new CPrefsLocalAccount;
	mPanels.AddPanel(local_account);

	CPrefsSMTPAccount* smtp_account = new CPrefsSMTPAccount;
	mPanels.AddPanel(smtp_account);

	CPrefsRemoteAccount* remote_account = new CPrefsRemoteAccount;
	mPanels.AddPanel(remote_account);

	CPrefsAdbkAccount* adbk_account = new CPrefsAdbkAccount;
	mPanels.AddPanel(adbk_account);

	CPrefsLDAPAccount* ldap_account = new CPrefsLDAPAccount;
	mPanels.AddPanel(ldap_account);

	CPrefsSIEVEAccount* sieve_account = new CPrefsSIEVEAccount;
	mPanels.AddPanel(sieve_account);

	CPrefsRemoteCalendarAccount* remotecal_account = new CPrefsRemoteCalendarAccount;
	mPanels.AddPanel(remotecal_account);

}

void CPrefsAccounts::SetControls(void)
{
	// Init the acount menu
	InitAccountPopup();

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
		mAccountPopup.SetValue(mAccountValue + IDM_ACCOUNTStart);
		CINETAccount* acct = NULL;
		long acct_index;
		CINETAccountList* acct_list = NULL;
		GetCurrentAccount(acct, acct_index, acct_list);
		SetAccount(acct);
	}
	else
	{
		mAccountValue = -1;
		mAccountPopup.SetValue(mAccountValue);
		SetAccount(NULL);
		UpdateItems(false);
	}
}

// Set prefs
void CPrefsAccounts::SetPrefs(CPreferences* copyPrefs)
{
	// Save ref to prefs
	mCopyPrefs = copyPrefs;

	mMatchUIDPswd = copyPrefs->mRemoteCachePswd.GetValue();
}

// Force update of prefs
void CPrefsAccounts::UpdatePrefs(CPreferences* prefs)
{
	// Update current account
	if (mAccountValue >= 0)
		UpdateAccount();

	prefs->mRemoteCachePswd.SetValue(mMatchUIDPswd);
}

// Set up account menu
void CPrefsAccounts::InitAccountPopup(void)
{
	CMenu* pPopup = mAccountPopup.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = 4; i < num_menu; i++)
		pPopup->RemoveMenu(4, MF_BYPOSITION);

	// Add each mail account
	// Now add current items
	int menu_id = IDM_ACCOUNTStart;
	for(CMailAccountList::const_iterator iter = mCopyPrefs->mMailAccounts.GetValue().begin();
			iter != mCopyPrefs->mMailAccounts.GetValue().end(); iter++, menu_id++)
	{
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter)->GetName());
	}

	// Add each SMTP account
	for(CSMTPAccountList::const_iterator iter = mCopyPrefs->mSMTPAccounts.GetValue().begin();
			iter != mCopyPrefs->mSMTPAccounts.GetValue().end(); iter++, menu_id++)
	{
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter)->GetName());
	}

	// Add each remote account
	for(CINETAccountList::const_iterator iter = mCopyPrefs->mRemoteAccounts.GetValue().begin();
			iter != mCopyPrefs->mRemoteAccounts.GetValue().end(); iter++, menu_id++)
	{
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter)->GetName());
	}

	// Add each address account
	for(CAddressAccountList::const_iterator iter = mCopyPrefs->mAddressAccounts.GetValue().begin();
			iter != mCopyPrefs->mAddressAccounts.GetValue().end(); iter++, menu_id++)
	{
		// Insert item
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter)->GetName());
	}
	
	// Add each SIEVE account
	for(CManageSIEVEAccountList::const_iterator iter = mCopyPrefs->mSIEVEAccounts.GetValue().begin();
			iter != mCopyPrefs->mSIEVEAccounts.GetValue().end(); iter++, menu_id++)
	{
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter)->GetName());
	}

	// Add each calendar account
	for(CCalendarAccountList::const_iterator iter = mCopyPrefs->mCalendarAccounts.GetValue().begin();
			iter != mCopyPrefs->mCalendarAccounts.GetValue().end(); iter++, menu_id++)
	{
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter)->GetName());
	}

	// Refresh its display
	mAccountPopup.RefreshValue();
}

void CPrefsAccounts::OnAccountPopup(UINT nID)
{
	switch(nID)
	{
	case IDM_ACCOUNT_NEW:
		DoNewAccount();
		break;
	case IDM_ACCOUNT_RENAME:
		DoRenameAccount();
		break;
	case IDM_ACCOUNT_DELETE:
		DoDeleteAccount();
		break;
	default:
		// Update existing account
		if ((mAccountValue >= 0) && (mAccountValue != nID - IDM_ACCOUNTStart))
			UpdateAccount();
		
		// Set new value
		mAccountValue = nID - IDM_ACCOUNTStart;
		if (mAccountValue >= 0)
		{
			CINETAccount* acct = NULL;
			long acct_index;
			CINETAccountList* acct_list = NULL;
			GetCurrentAccount(acct, acct_index, acct_list);
			SetAccount(acct);
		}

		// Update menu
		mAccountPopup.SetValue(nID);
	}
}

// Add new account
void CPrefsAccounts::DoNewAccount(void)
{
	// Update current account
	if (mAccountValue >= 0)
		UpdateAccount();

	while(true)
	{
		// Create the dialog
		CCreateAccountDialog dlog(CSDIFrame::GetAppTopWindow());

		// Let Dialog process events
		if (dlog.DoModal() == IDOK)
		{
			cdstring new_name;
			short acct_type;
			
			// Get dialog details
			dlog.GetDetails(new_name, acct_type);

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
			case IDM_ACCOUNT_IMAP_MAILBOX:
			case IDM_ACCOUNT_POP3_MAILBOX:
			case IDM_ACCOUNT_LOCAL_MAILBOX:
				acct_list = (CINETAccountList*) &mCopyPrefs->mMailAccounts.GetValue();
				break;

			case IDM_ACCOUNT_SMTP:
				acct_list = (CINETAccountList*) &mCopyPrefs->mSMTPAccounts.GetValue();
				break;

			case IDM_ACCOUNT_IMSP_OPTIONS:
			case IDM_ACCOUNT_ACAP_OPTIONS:
				acct_list = (CINETAccountList*) &mCopyPrefs->mRemoteAccounts.GetValue();
				break;

			case IDM_ACCOUNT_IMSP_ADBK:
			case IDM_ACCOUNT_ACAP_ADBK:
			case IDM_ACCOUNT_LDAP:
				acct_list = (CINETAccountList*) &mCopyPrefs->mAddressAccounts.GetValue();
				break;

			case IDM_ACCOUNT_SIEVE:
				acct_list = (CINETAccountList*) &mCopyPrefs->mSIEVEAccounts.GetValue();
				break;

			case IDM_ACCOUNT_CALDAVCALENDAR:
			case IDM_ACCOUNT_WEBDAVCALENDAR:
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
			switch(acct_type)
			{
			case IDM_ACCOUNT_IMAP_MAILBOX:
			case IDM_ACCOUNT_POP3_MAILBOX:
			case IDM_ACCOUNT_LOCAL_MAILBOX:
				acct = new CMailAccount;
				switch(acct_type)
				{
				case IDM_ACCOUNT_IMAP_MAILBOX:
					acct->SetServerType(CINETAccount::eIMAP);
					break;
				case IDM_ACCOUNT_POP3_MAILBOX:
					acct->SetServerType(CINETAccount::ePOP3);
					break;
				case IDM_ACCOUNT_LOCAL_MAILBOX:
					acct->SetServerType(CINETAccount::eLocal);
					break;
				}
				acct->SetName(new_name);
				((CMailAccountList&) mCopyPrefs->mMailAccounts.GetValue()).push_back((CMailAccount*) acct);
				mCopyPrefs->mMailAccounts.SetDirty();
				insert_pos = mCopyPrefs->mMailAccounts.GetValue().size() - 1;
				break;

			case IDM_ACCOUNT_SMTP:
				acct = new CSMTPAccount;
				acct->SetServerType(CINETAccount::eSMTP);
				acct->SetName(new_name);
				((CSMTPAccountList&) mCopyPrefs->mSMTPAccounts.GetValue()).push_back((CSMTPAccount*) acct);
				mCopyPrefs->mSMTPAccounts.SetDirty();
				insert_pos = mCopyPrefs->mMailAccounts.GetValue().size() +
								mCopyPrefs->mSMTPAccounts.GetValue().size() - 1;
				break;

			case IDM_ACCOUNT_IMSP_OPTIONS:
			case IDM_ACCOUNT_ACAP_OPTIONS:
				acct = new CINETAccount;
				switch(acct_type)
				{
				case IDM_ACCOUNT_IMSP_OPTIONS:
					acct->SetServerType(CINETAccount::eIMSP);
					break;
				case IDM_ACCOUNT_ACAP_OPTIONS:
					acct->SetServerType(CINETAccount::eACAP);
					break;
				}
				acct->SetName(new_name);
				((CINETAccountList&) mCopyPrefs->mRemoteAccounts.GetValue()).push_back(acct);
				mCopyPrefs->mRemoteAccounts.SetDirty();
				insert_pos = mCopyPrefs->mMailAccounts.GetValue().size() +
								mCopyPrefs->mSMTPAccounts.GetValue().size() +
								mCopyPrefs->mRemoteAccounts.GetValue().size() - 1;
				break;

			case IDM_ACCOUNT_IMSP_ADBK:
			case IDM_ACCOUNT_ACAP_ADBK:
			case IDM_ACCOUNT_LDAP:
				acct = new CAddressAccount;
				switch(acct_type)
				{
				case IDM_ACCOUNT_IMSP_ADBK:
					acct->SetServerType(CINETAccount::eIMSP);
					break;
				case IDM_ACCOUNT_ACAP_ADBK:
					acct->SetServerType(CINETAccount::eACAP);
					break;
				case IDM_ACCOUNT_LDAP:
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

			case IDM_ACCOUNT_SIEVE:
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

			case IDM_ACCOUNT_CALDAVCALENDAR:
			case IDM_ACCOUNT_WEBDAVCALENDAR:
				acct = new CCalendarAccount;
				switch(acct_type)
				{
				case IDM_ACCOUNT_CALDAVCALENDAR:
					acct->SetServerType(CINETAccount::eCalDAVCalendar);
					break;
				case IDM_ACCOUNT_WEBDAVCALENDAR:
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

			}

			// Always initialise the account
			acct->NewAccount();

			// Reset menu
			InitAccountPopup();

			// Enable items if disabled
			if (mAccountValue < 0)
				UpdateItems(true);

			// Set new values
			mAccountValue = insert_pos;
			mAccountPopup.SetValue(mAccountValue + IDM_ACCOUNTStart);
			mAccountPopup.RedrawWindow();
			SetAccount(acct);

			// Add associated preference items
			mCopyPrefs->NewAccount(acct);
			CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetParentOwner();
			prefs_dlog->SetAccountNew(true);
			break;
		}
		else
			break;
	}
}

// Rename account
void CPrefsAccounts::DoRenameAccount(void)
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
			
			if (CGetStringDialog::PoseDialog("Alerts::Preferences::RenameAccount", "Alerts::Preferences::RenameAccountTitle", new_name))
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
				else if (typeid(*acct) == typeid(CINETAccount))
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
				else if (typeid(*acct) == typeid(CINETAccount))
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
					CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetParentOwner();

					prefs_dlog->SetAccountRename(true);
					prefs_dlog->GetRenames().push_back(cdstrpair(old_name, new_name));
				}
				
				// Reset menu
				InitAccountPopup();

				break;
			}
			else
				break;
		}
	}
}

// Delete accoount
void CPrefsAccounts::DoDeleteAccount(void)
{
	// Update current account
	if (mAccountValue >= 0)
		UpdateAccount();

	// Check that this is what we want to do
	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Preferences::DeleteAccount") == CErrorHandler::Ok)
	{
		CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetParentOwner();

		CINETAccount* acct = NULL;
		long acct_index;
		CINETAccountList* acct_list = NULL;
		GetCurrentAccount(acct, acct_index, acct_list);
		
		// Dirty first while object still exists
		if (typeid(*acct) == typeid(CMailAccount))
			mCopyPrefs->mMailAccounts.SetDirty();
		else if (typeid(*acct) == typeid(CSMTPAccount))
			mCopyPrefs->mSMTPAccounts.SetDirty();
		else if (typeid(*acct) == typeid(CINETAccount))
		{
			// Prevent delete of last remote account if set to remote
			if ((mCopyPrefs->mRemoteAccounts.GetValue().size() == 1) && !prefs_dlog->IsLocal())
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

		prefs_dlog->SetAccountNew(true);

		// Erase from prefs
		acct_list->erase(acct_list->begin() + acct_index);
		
		// Reset menu
		InitAccountPopup();
		
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
			mAccountPopup.SetValue(mAccountValue + IDM_ACCOUNTStart);
			long acct_index;
			CINETAccountList* acct_list = NULL;
			GetCurrentAccount(acct, acct_index, acct_list);
			SetAccount(acct);
		}
		else
		{
			mAccountPopup.SetValue(-1);
			SetAccount(NULL);
			UpdateItems(false);
		}
	}
}

// Delete accoount
void CPrefsAccounts::UpdateItems(bool enable)
{
	CMenu* pPopup = mAccountPopup.GetPopupMenu();

	if (enable)
	{
		
		mServerIP.EnableWindow(!CAdminLock::sAdminLock.mLockServerAddress ||
								mIsSMTP && CAdminLock::sAdminLock.mNoLockSMTP);

		mPanels.EnableWindow(true);

		// Enable menus rename & delete
		pPopup->EnableMenuItem(IDM_ACCOUNT_RENAME, MF_BYCOMMAND);
		if (!CAdminLock::sAdminLock.mLockServerAddress)
			pPopup->EnableMenuItem(IDM_ACCOUNT_DELETE, MF_BYCOMMAND);
	}
	else
	{
		SetAccount(NULL);
		mServerIP.EnableWindow(false);
		mPanels.EnableWindow(false);
		
		// Disable menus rename & delete
		pPopup->EnableMenuItem(IDM_ACCOUNT_RENAME, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(IDM_ACCOUNT_DELETE, MF_GRAYED | MF_BYCOMMAND);
	}
}

// Set account details
void CPrefsAccounts::SetAccount(const CINETAccount* account)
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
			}
		}
		else if (typeid(*account) == typeid(CSMTPAccount))
		{
			copyStr.FromResource("UI::Preferences::AccountSMTPSendMail");
			mIsSMTP = true;
		}
		else if (typeid(*account) == typeid(CINETAccount))
		{
			switch(account->GetServerType())
			{
			case CINETAccount::eIMSP:
				copyStr.FromResource("UI::Preferences::AccountIMSPOptions");
				break;
			case CINETAccount::eACAP:
				copyStr.FromResource("UI::Preferences::AccountACAPOptions");
				break;
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
			}
		}

		// Copy text to edit fields
		CUnicodeUtils::SetWindowTextUTF8(&mServerType, copyStr);
		CUnicodeUtils::SetWindowTextUTF8(&mServerIP, account->GetServerIP());
		if (account->GetServerType() == CINETAccount::eLocal)
		{
			CUnicodeUtils::SetWindowTextUTF8(&mServerIP, cdstring::null_str);
			mServerIP.ShowWindow(SW_HIDE);
			mServerIPTitle.ShowWindow(SW_HIDE);
		}
		else
		{
			CUnicodeUtils::SetWindowTextUTF8(&mServerIP, account->GetServerIP());
			mServerIP.ShowWindow(SW_SHOW);
			mServerIPTitle.ShowWindow(SW_SHOW);
		}

		SetPanel(account);
		mPanels.ShowWindow(SW_HIDE);

		// Toggle server locks based on type
		mServerIP.EnableWindow(!CAdminLock::sAdminLock.mLockServerAddress ||
								mIsSMTP && CAdminLock::sAdminLock.mNoLockSMTP);
	}
	else
	{
		// Hide all panels
		CUnicodeUtils::SetWindowTextUTF8(&mServerType, copyStr);
		CUnicodeUtils::SetWindowTextUTF8(&mServerIP, copyStr);
		mPanels.ShowWindow(SW_HIDE);
	}
}

// Update current account
void CPrefsAccounts::UpdateAccount(void)
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
	else if (typeid(*current) == typeid(CINETAccount))
		account = new CINETAccount(*(CINETAccount*) current);
	else if (typeid(*current) == typeid(CAddressAccount))
		account = new CAddressAccount(*(CAddressAccount*) current);
	else if (typeid(*current) == typeid(CManageSIEVEAccount))
		account = new CManageSIEVEAccount(*(CManageSIEVEAccount*) current);
	else if (typeid(*current) == typeid(CCalendarAccount))
		account = new CCalendarAccount(*(CCalendarAccount*) current);
	else
		return;

	CString copyStr;

	// Copy info from panel into prefs
	cdstring temp = CUnicodeUtils::GetWindowTextUTF8(&mServerIP);
	temp.trimspace();
	account->SetServerIP(temp);
	
	mPanels.UpdatePanelContent(account);
		
	// Now check similarity
	bool same = false;
	if (typeid(*current) == typeid(CMailAccount))
		same = (*((CMailAccount*) current) == *((CMailAccount*) account));
	else if (typeid(*current) == typeid(CSMTPAccount))
		same = (*((CSMTPAccount*) current) == *((CSMTPAccount*) account));
	else if (typeid(*current) == typeid(CINETAccount))
		same = (*((CINETAccount*) current) == *((CINETAccount*) account));
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
		else if (typeid(*account) == typeid(CINETAccount))
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
void CPrefsAccounts::GetCurrentAccount(CINETAccount*& acct, 
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
void CPrefsAccounts::SetPanel(const CINETAccount* account)
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
			}
		}
	}
	else if (typeid(*account) == typeid(CSMTPAccount))
		panel = eSMTPAccountPanel;
	else if (typeid(*account) == typeid(CINETAccount))
		panel = eRemoteAccountPanel;
	else if (typeid(*account) == typeid(CAddressAccount))
	{
		switch(account->GetServerType())
		{
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
		panel = eSIEVEAccountPanel;
	else if (typeid(*account) == typeid(CCalendarAccount))
	{
		switch(account->GetServerType())
		{
		case CINETAccount::eWebDAVCalendar:
		case CINETAccount::eCalDAVCalendar:
			panel = eRemoteCalendarAccountPanel;
			break;
		default:
			panel = -1;
		}
	}

	mPanels.SetPanel(panel);
	CPrefsAccountPanel* pnl = static_cast<CPrefsAccountPanel*>(mPanels.GetCurrentPanel());
	pnl->SetPrefs(mCopyPrefs);
	mPanels.SetPanelContent((void*) account);
}

