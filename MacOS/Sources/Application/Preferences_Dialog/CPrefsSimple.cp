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


// Source for PrefsSimple class

#include "CPrefsSimple.h"

#include "CAddressList.h"
#include "CAdminLock.h"
#include "CMailboxPopup.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsEditHeadFoot.h"
#include "CStaticText.h"
#include "CTextFieldX.h"

#include <LBevelButton.h>
#include <LCheckBox.h>
#include <LCheckBoxGroupBox.h>
#include <LRadioButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S S I M P L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsSimple::CPrefsSimple()
{
}

// Constructor from stream
CPrefsSimple::CPrefsSimple(LStream *inStream)
		: CPrefsPanel(inStream)
{
}

// Default destructor
CPrefsSimple::~CPrefsSimple()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsSimple::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsPanel::FinishCreateSelf();

	// Set backgrounds
	mRealName = (CTextFieldX*) FindPaneByID(paneid_SimpleRealName);
	mEmailAddress = (CTextFieldX*) FindPaneByID(paneid_SimpleEmailAddress);
	if (CAdminLock::sAdminLock.mLockReturnAddress)
		mEmailAddress->Disable();
	mIMAPuid = (CTextFieldX*) FindPaneByID(paneid_SimpleIMAPuid);
	mIMAPip = (CTextFieldX*) FindPaneByID(paneid_SimpleIMAPip);
	mServerIMAP = (LRadioButton*) FindPaneByID(paneid_SimpleServerIMAP);
	mServerPOP3 = (LRadioButton*) FindPaneByID(paneid_SimpleServerPOP3);
	mServerLocal = (LRadioButton*) FindPaneByID(paneid_SimpleServerLocal);
	mSMTPip = (CTextFieldX*) FindPaneByID(paneid_SimpleSMTPip);
	if (CAdminLock::sAdminLock.mLockServerAddress)
	{
		mIMAPip->Disable();
		if (!CAdminLock::sAdminLock.mNoLockSMTP)
			mSMTPip->Disable();
		mServerIMAP->Disable();
		mServerPOP3->Disable();
		mServerLocal->Disable();
	}

	mSaveUser = (LCheckBox*) FindPaneByID(paneid_SimpleSaveUser);
	mSavePswd = (LCheckBox*) FindPaneByID(paneid_SimpleSavePswd);
	if (CAdminLock::sAdminLock.mLockSavePswd)
		mSavePswd->Disable();

	mLogonStartup = (LCheckBox*) FindPaneByID(paneid_SimpleLogonStartup);
	mLeaveOnServer = (LCheckBox*) FindPaneByID(paneid_SimpleLeaveOnServer);

	mCheckNever = (LRadioButton*) FindPaneByID(paneid_SimpleCheckNever);
	mCheckEvery = (LRadioButton*) FindPaneByID(paneid_SimpleCheckEvery);
	mCheckInterval = (CTextFieldX*) FindPaneByID(paneid_SimpleCheckInterval);

	mNewMailAlert = (LCheckBox*) FindPaneByID(paneid_SimpleNewMailAlert);

	mDirectorySeparatorTitle = (CStaticText*) FindPaneByID(paneid_SimpleWDSeparatorTitle);
	mDirectorySeparator = (CTextFieldX*) FindPaneByID(paneid_SimpleWDSeparator);

	mSignatureBtn = (LBevelButton*) FindPaneByID(paneid_SimpleSetSignatureBtn);

	mDoCopyTo = (LCheckBoxGroupBox*) FindPaneByID(paneid_SimpleDoCopyTo);
	mCopyToMailbox = (CTextFieldX*) FindPaneByID(paneid_SimpleCopyToMailbox);
	mCopyToMailboxPopup = (CMailboxPopup*) FindPaneByID(paneid_SimpleCopyToMailboxPopup);
	mCopyToMailboxPopup->SetCopyTo(false);
	if (!CMulberryApp::sApp->LoadedPrefs())
		mCopyToMailboxPopup->Disable();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsSimpleBtns);
}

// Handle buttons
void CPrefsSimple::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage) {

		case msg_SimpleServerIMAP:
			if (*((long*) ioParam))
				SetType(CINETAccount::eIMAP);
			break;

		case msg_SimpleServerPOP3:
			if (*((long*) ioParam))
				SetType(CINETAccount::ePOP3);
			break;

		case msg_SimpleServerLocal:
			if (*((long*) ioParam))
				SetType(CINETAccount::eLocal);
			break;

		case msg_SimpleCheckNever:
			if (*((long*) ioParam))
			{
				mCheckInterval->Disable();
				Refresh();
			}
			break;

		case msg_SimpleCheckEvery:
			if (*((long*) ioParam))
			{
				mCheckInterval->Enable();
				LCommander::SwitchTarget(mCheckInterval);
				mCheckInterval->SelectAll();
				Refresh();
			}
			break;

		case msg_SimpleSetSignature:
		{
			EditSignature(&mSignature);
			break;
		}

		case msg_SimpleDoCopyTo:
			if (*((long*) ioParam))
			{
				mCopyToMailbox->Enable();
				if (CMulberryApp::sApp->LoadedPrefs())
					mCopyToMailboxPopup->Enable();
				LCommander::SwitchTarget(mCopyToMailbox);
				mCopyToMailbox->SelectAll();
			}
			else
			{
				mCopyToMailbox->Disable();
				mCopyToMailboxPopup->Disable();
			}
			Refresh();
			break;

		case msg_SimpleCopyToMailboxPopup:
			{
				cdstring mbox_name;
				if (mCopyToMailboxPopup->GetSelectedMboxName(mbox_name))
				{
					if (mbox_name.empty())
					{
						mCopyToMailbox->SetText(cdstring::null_str);
					}
					else if (*mbox_name.c_str() != 0x01)
					{
						cdstring copyStr;
						if (::strchr(mbox_name.c_str(), cMailAccountSeparator))
							copyStr = ::strchr(mbox_name.c_str(), cMailAccountSeparator) + 1;
						else
							copyStr = mbox_name;
						mCopyToMailbox->SetText(copyStr);
					}
				}
			}
			break;
	}
}

// Toggle display of IC items
void CPrefsSimple::ToggleICDisplay(void)
{
	if (mCopyPrefs->mUse_IC.GetValue())
	{
		mIMAPuid->Disable();
		if (!CAdminLock::sAdminLock.mLockReturnAddress)
			mEmailAddress->Disable();
		mRealName->Disable();
		if (!CAdminLock::sAdminLock.mLockServerAddress)
		{
			mIMAPip->Disable();
			mSMTPip->Disable();
		}
		mSignatureBtn->Disable();
	}
	else
	{
		if (!mServerLocal->GetValue())
			mIMAPuid->Enable();
		if (!CAdminLock::sAdminLock.mLockReturnAddress)
			mEmailAddress->Enable();
		mRealName->Enable();
		if (!CAdminLock::sAdminLock.mLockServerAddress)
		{
			if (!mServerLocal->GetValue())
				mIMAPip->Enable();
		}
		if (!CAdminLock::sAdminLock.mLockServerAddress || CAdminLock::sAdminLock.mNoLockSMTP)
			mSMTPip->Enable();
		mSignatureBtn->Enable();
	}
}

// Set prefs
void CPrefsSimple::SetPrefs(CPreferences* copyPrefs)
{
	// Save ref to prefs
	mCopyPrefs = copyPrefs;
	CPreferences* local_prefs = copyPrefs;

	// Get defaults
	const CMailAccount* imap = local_prefs->mMailAccounts.GetValue().front();
	const CAuthenticator* top_auth = &imap->GetAuthenticator();
	const CAuthenticatorUserPswd* auth = top_auth->RequiresUserPswd() ? static_cast<const CAuthenticatorUserPswd*>(top_auth->GetAuthenticator()) : nil;
	const CSMTPAccount* smtp = local_prefs->mSMTPAccounts.GetValue().front();
	const CIdentity* id = &local_prefs->mIdentities.GetValue().front();
	// Determine address list
	CAddressList addr_list(id->GetFrom().c_str(), id->GetFrom().length());
	if (!local_prefs->mMailNotification.GetValue().size())
	{
		CMailNotification notify;
		notify.SetName("Default");
		local_prefs->mMailNotification.Value().push_back(notify);
		local_prefs->mMailNotification.SetDirty();
	}
	const CMailNotification& notify = local_prefs->mMailNotification.GetValue().front();

	// Copy text to edit fields
	if (addr_list.size())
	{
		mRealName->SetText(addr_list.front()->GetName());

		// Always display complete address to avoid confusion
		//copyStr = (CAdminLock::sAdminLock.mLockReturnAddress ? CAdminLock::sAdminLock.mLockedReturnDomain : addr_list.front()->GetMailAddress());
		mEmailAddress->SetText(addr_list.front()->GetMailAddress());
	}

	if (auth)
	{
		mIMAPuid->SetText(auth->GetUID());
	}
	else
		mIMAPuid->Disable();

	mIMAPip->SetText(imap->GetServerIP());

	SetType(imap->GetServerType());
	switch(imap->GetServerType())
	{
	case CINETAccount::eIMAP:
	default:
		mServerIMAP->SetValue(1);
		break;

	case CINETAccount::ePOP3:
		mServerPOP3->SetValue(1);
		break;

	case CINETAccount::eLocal:
		mServerLocal->SetValue(1);
		break;
	}

	mSMTPip->SetText(smtp->GetServerIP());

	if (auth)
	{
		mSaveUser->SetValue(auth->GetSaveUID() ? 1 : 0);
		mSavePswd->SetValue((auth->GetSavePswd() && !CAdminLock::sAdminLock.mLockSavePswd) ? 1 : 0);
	}

	mLogonStartup->SetValue(imap->GetLogonAtStart());
	mLeaveOnServer->SetValue(imap->GetLeaveOnServer());

	if (notify.IsEnabled())
		mCheckEvery->SetValue(1);
	else
		mCheckNever->SetValue(1);

	mCheckInterval->SetNumber(notify.GetCheckInterval());
	if (notify.IsEnabled())
		mCheckInterval->Enable();
	else
		mCheckInterval->Disable();

	mNewMailAlert->SetValue(notify.DoShowAlertForeground() ? 1 : 0);

	if (imap->GetServerType() == CINETAccount::eIMAP)
	{
		cdstring temp(imap->GetDirDelim());
		mDirectorySeparator->SetText(temp);
	}

	mSignature = id->GetSignature();

	mDoCopyTo->SetValue(id->UseCopyTo() && !id->GetCopyToNone());

	// Strip account name
	cdstring copy_to = id->GetCopyTo();
	cdstring copyStr;
	if (::strchr(copy_to.c_str(), cMailAccountSeparator))
		copyStr = ::strchr(copy_to.c_str(), cMailAccountSeparator) + 1;
	else
		copyStr = copy_to.c_str();
	mCopyToMailbox->SetText(copyStr);

	if (!mDoCopyTo->GetValue())
	{
		mCopyToMailbox->Disable();
		mCopyToMailboxPopup->Disable();
	}

	// Toggle IC display - do after account is first set
	ToggleICDisplay();

	// Make the first suitable TextEdit the Target and select all its text
	CTextFieldX* select = (!local_prefs->mUse_IC.GetValue() ? mRealName : mCheckInterval);
	select->GetSuperCommander()->SetLatentSub(select);
	LCommander::SwitchTarget(select);
	select->SelectAll();
}

// Force update of prefs
void CPrefsSimple::UpdatePrefs(void)
{
	CPreferences* local_prefs = mCopyPrefs;

	CMailAccount* imap = local_prefs->mMailAccounts.GetValue().front();
	CAuthenticator* top_auth = &imap->GetAuthenticator();
	CAuthenticatorUserPswd* auth = top_auth->RequiresUserPswd() ? static_cast<CAuthenticatorUserPswd*>(top_auth->GetAuthenticator()) : nil;
	CSMTPAccount* smtp = local_prefs->mSMTPAccounts.GetValue().front();
	CIdentity* id = const_cast<CIdentity*>(&local_prefs->mIdentities.GetValue().front());

	// Determine address list
	CAddressList addr_list(id->GetFrom().c_str(), id->GetFrom().length());
	if (!local_prefs->mMailNotification.GetValue().size())
	{
		CMailNotification notify;
		notify.SetName("Default");
		local_prefs->mMailNotification.Value().push_back(notify);
		local_prefs->mMailNotification.SetDirty();
	}
	CMailNotification& notify = (CMailNotification&) local_prefs->mMailNotification.GetValue().front();

	// Make copies to look for changes
	CMailAccount copy_imap(*imap);
	CAuthenticatorUserPswd temp_auth;
	CAuthenticatorUserPswd copy_auth(auth ? *auth : temp_auth);
	CSMTPAccount copy_smtp(*smtp);
	CIdentity copy_id(*id);
	CMailNotification copy_notify(notify);

	// Reset account type
	CINETAccount::EINETServerType type = CINETAccount::eIMAP;
	if (mServerIMAP->GetValue())
		type = CINETAccount::eIMAP;
	else if (mServerPOP3->GetValue())
		type = CINETAccount::ePOP3;
	else if (mServerLocal->GetValue())
		type = CINETAccount::eLocal;
	if (type != imap->GetServerType())
	{
		// May need to change name
		bool change_name = false;
		switch(imap->GetServerType())
		{
		case CINETAccount::eIMAP:
			change_name = (imap->GetName() == "IMAP");
			break;
		case CINETAccount::ePOP3:
			change_name = (imap->GetName() == "POP3");
			break;
		case CINETAccount::eLocal:
			change_name = (imap->GetName() == "Local");
			break;
		default:;
		}
		imap->SetServerType(type);

		// Change name only if a single account exists
		if (change_name && (local_prefs->mMailAccounts.GetValue().size() == 1))
		{
			switch(imap->GetServerType())
			{
			case CINETAccount::eIMAP:
				local_prefs->RenameAccount(imap, "IMAP");
				break;
			case CINETAccount::ePOP3:
				local_prefs->RenameAccount(imap, "POP3");
				break;
			case CINETAccount::eLocal:
				local_prefs->RenameAccount(imap, "Local");
				break;
			default:;
			}
		}

		// Clear out WDs and reinit account
		imap->GetWDs().clear();
		imap->NewAccount();
	}

	// Copy info from panel into prefs
	{
		// Determine address list
		CAddressList addr_list(id->GetFrom().c_str(), id->GetFrom().length());

		cdstring name = mRealName->GetText();
		cdstring email = mEmailAddress->GetText();
		if (CAdminLock::sAdminLock.mLockReturnAddress && addr_list.size())
			email = addr_list.front()->GetMailAddress();
		CAddress addr(email, name);
		id->SetFrom(addr.GetFullAddress(), true);
	}

	if ((type != CINETAccount::eLocal) && auth)
	{
		auth->SetUID(mIMAPuid->GetText());
	}

	if (!CAdminLock::sAdminLock.mLockServerAddress)
	{
		cdstring temp = mIMAPip->GetText();
		temp.trimspace();
		imap->SetServerIP(temp);
	}

	// SMTP may be unlocked even though other servers are locked
	if (!CAdminLock::sAdminLock.mLockServerAddress || CAdminLock::sAdminLock.mNoLockSMTP)
	{
		cdstring temp = mSMTPip->GetText();
		temp.trimspace();
		smtp->SetServerIP(temp);
	}

	if ((type != CINETAccount::eLocal) && auth)
		auth->SetSaveUID(mSaveUser->GetValue()==1);

	if ((type != CINETAccount::eLocal) && auth && !CAdminLock::sAdminLock.mLockSavePswd)
		auth->SetSavePswd(mSavePswd->GetValue()==1);

	if (type == CINETAccount::eIMAP)
		imap->SetLoginAtStart(mLogonStartup->GetValue()==1);

	if (type == CINETAccount::ePOP3)
		imap->SetLeaveOnServer(mLeaveOnServer->GetValue()==1);

	notify.Enable(mCheckEvery->GetValue() == 1);
	notify.SetCheckInterval(mCheckInterval->GetNumber());
	notify.SetShowAlertForeground(mNewMailAlert->GetValue()==1);

	if (type == CINETAccount::eIMAP)
	{
		cdstring temp = mDirectorySeparator->GetText();
		if (!temp.empty())
			imap->SetDirDelim(temp[1UL]);
		else
			imap->SetDirDelim(0);
	}

	id->SetSignature(mSignature, mSignature.length());

	// Add account prefix
	cdstring temp = mCopyToMailbox->GetText();
	cdstring copy_to = (!temp.empty() ? imap->GetName() + cMailAccountSeparator : cdstring::null_str);
	copy_to += temp;

	// Always active copy to, do copy to none if no copy requested, do copy choose if mailbox name is empty
	id->SetCopyTo(copy_to, true);
	id->SetCopyToNone(mDoCopyTo->GetValue() == 0);
	id->SetCopyToChoose(copy_to.empty());

	// Set dirty if required
	if (!(copy_imap == *imap))
		local_prefs->mMailAccounts.SetDirty();
	if ((type != CINETAccount::eLocal) && auth && !(copy_auth == *auth))
		local_prefs->mMailAccounts.SetDirty();
	if (!(copy_smtp == *smtp))
		local_prefs->mSMTPAccounts.SetDirty();
	if (!(copy_id == *id))
		local_prefs->mIdentities.SetDirty();
	if (!(copy_notify == notify))
		local_prefs->mMailNotification.SetDirty();
}

// Edit signature
void CPrefsSimple::EditSignature(cdstring* text)
{
	// Allow signature to be set via a file
	CPrefsEditHeadFoot::PoseDialog("Set Signature", *text, true, mCopyPrefs->spaces_per_tab.GetValue(), mCopyPrefs->wrap_length.GetValue());
}

void CPrefsSimple::SetType(CINETAccount::EINETServerType type)
{
	switch(type)
	{
	case CINETAccount::eIMAP:
	default:
		if (!mCopyPrefs->mUse_IC.GetValue())
		{
			mIMAPuid->Enable();
			if (!CAdminLock::sAdminLock.mLockServerAddress)
				mIMAPip->Enable();
		}
		mSaveUser->Show();
		mSavePswd->Show();
		mLogonStartup->Show();
		mLeaveOnServer->Hide();
		mDirectorySeparatorTitle->Show();
		mDirectorySeparator->Show();
		break;

	case CINETAccount::ePOP3:
		if (!mCopyPrefs->mUse_IC.GetValue())
		{
			mIMAPuid->Enable();
			if (!CAdminLock::sAdminLock.mLockServerAddress)
				mIMAPip->Enable();
		}
		mSaveUser->Show();
		mSavePswd->Show();
		mLogonStartup->Hide();
		mLeaveOnServer->Show();
		mDirectorySeparatorTitle->Hide();
		mDirectorySeparator->Hide();
		break;

	case CINETAccount::eLocal:
		mIMAPuid->Disable();
		mIMAPip->Disable();
		mSaveUser->Hide();
		mSavePswd->Hide();
		mLogonStartup->Hide();
		mLeaveOnServer->Hide();
		mDirectorySeparatorTitle->Hide();
		mDirectorySeparator->Hide();
		break;
	}
}
