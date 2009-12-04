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


// CPrefsSimple.cpp : implementation file
//


#include "CPrefsSimple.h"

#include "CAddressList.h"
#include "CAdminLock.h"
#include "CAuthenticator.h"
#include "CIconLoader.h"
#include "CMailboxPopup.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CPrefsEditHeadFoot.h"
#include "CTextField.h"

#include "JXSecondaryRadioGroup.h"

#include <JXUpRect.h>
#include <JXTextButton.h>
#include <JXStaticText.h>
#include <JXInputField.h>
#include <JXColormap.h>
#include <JXDownRect.h>
#include <JXTextRadioButton.h>
#include <JXTextCheckbox.h>
#include <JXIntegerInput.h>
#include <JXImageButton.h>
#include <JXEngravedRect.h>
#include "CInputField.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsSimple property page


CPrefsSimple::CPrefsSimple( JXContainer* enclosure,
														const HSizingOption hSizing, 
														const VSizingOption vSizing,
														const JCoordinate x, const JCoordinate y,
														const JCoordinate w, const JCoordinate h) 
	:CPrefsPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
}

void CPrefsSimple::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Preferences", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 85,20);
    assert( obj1 != NULL );
    const JFontStyle obj1_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj1->SetFontStyle(obj1_style);

    JXStaticText* obj2 =
        new JXStaticText("Real Name:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 20,20, 80,20);
    assert( obj2 != NULL );

    mRealName =
        new CTextInputField(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 120,20, 230,20);
    assert( mRealName != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Email Address:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 20,45, 100,20);
    assert( obj3 != NULL );

    mEmailAddress =
        new CTextInputField(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 120,45, 230,20);
    assert( mEmailAddress != NULL );

    JXStaticText* obj4 =
        new JXStaticText("User ID:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 20,70, 100,20);
    assert( obj4 != NULL );

    mIMAPuid =
        new CTextInputField(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 120,70, 230,20);
    assert( mIMAPuid != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Email Server:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 20,95, 100,20);
    assert( obj5 != NULL );

    mIMAPip =
        new CTextInputField(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 120,95, 230,20);
    assert( mIMAPip != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Server Type:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,120, 80,20);
    assert( obj6 != NULL );

    mServerTypeGroup =
        new JXRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 120,118, 230,24);
    assert( mServerTypeGroup != NULL );
    mServerTypeGroup->SetBorderWidth(0);

    JXTextRadioButton* obj7 =
        new JXTextRadioButton(1, "IMAP", mServerTypeGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,2, 60,20);
    assert( obj7 != NULL );

    JXTextRadioButton* obj8 =
        new JXTextRadioButton(2, "POP3", mServerTypeGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,2, 60,20);
    assert( obj8 != NULL );

    JXTextRadioButton* obj9 =
        new JXTextRadioButton(3, "Local", mServerTypeGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 160,2, 60,20);
    assert( obj9 != NULL );

    JXStaticText* obj10 =
        new JXStaticText("SMTP Server:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 20,145, 90,20);
    assert( obj10 != NULL );

    mSMTPip =
        new CTextInputField(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 120,145, 230,20);
    assert( mSMTPip != NULL );

    mCheckMailGroup =
        new JXSecondaryRadioGroup(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 25,185, 175,84);
    assert( mCheckMailGroup != NULL );

    JXStaticText* obj11 =
        new JXStaticText("Check for New Email:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 30,175, 130,20);
    assert( obj11 != NULL );

    JXTextRadioButton* obj12 =
        new JXTextRadioButton(1, "Never", mCheckMailGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,10, 70,20);
    assert( obj12 != NULL );

    JXTextRadioButton* obj13 =
        new JXTextRadioButton(2, "Every:", mCheckMailGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,30, 65,20);
    assert( obj13 != NULL );

    mCheckInterval =
        new CInputField<JXIntegerInput>(mCheckMailGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 80,31, 45,20);
    assert( mCheckInterval != NULL );

    JXStaticText* obj14 =
        new JXStaticText("mins.", mCheckMailGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 130,33, 35,20);
    assert( obj14 != NULL );

    mNewMailAlert =
        new JXTextCheckbox("Use Alert", mCheckMailGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,55, 95,20);
    assert( mNewMailAlert != NULL );

    mSaveUser =
        new JXTextCheckbox("Save User ID", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 210,175, 115,20);
    assert( mSaveUser != NULL );

    mSavePswd =
        new JXTextCheckbox("Save Password", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 210,195, 115,21);
    assert( mSavePswd != NULL );

    mLogonStartup =
        new JXTextCheckbox("Login at Startup", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 210,215, 155,20);
    assert( mLogonStartup != NULL );

    mLeaveOnServer =
        new JXTextCheckbox("Leave Email on Server", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 210,215, 160,20);
    assert( mLeaveOnServer != NULL );

    mDirectorySeparatorTitle =
        new JXStaticText("Hierarchy\nSeparator:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 215,235, 65,35);
    assert( mDirectorySeparatorTitle != NULL );

    mDirectorySeparator =
        new CTextInputField(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 290,247, 30,20);
    assert( mDirectorySeparator != NULL );

    JXStaticText* obj15 =
        new JXStaticText("Signature", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 21,307, 60,20);
    assert( obj15 != NULL );

    mSignatureBtn =
        new JXImageButton(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 35,274, 32,32);
    assert( mSignatureBtn != NULL );

    JXDownRect* obj16 =
        new JXDownRect(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 20,335, 340,55);
    assert( obj16 != NULL );

    mDoCopyTo =
        new JXTextCheckbox("Copy Outgoing Messages", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 35,325, 180,20);
    assert( mDoCopyTo != NULL );

    JXStaticText* obj17 =
        new JXStaticText("Default:", obj16,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,20, 50,20);
    assert( obj17 != NULL );

    mCopyToMailbox =
        new CTextInputField(obj16,
                    JXWidget::kHElastic, JXWidget::kVElastic, 60,20, 230,20);
    assert( mCopyToMailbox != NULL );

    mCopyToMailboxPopup =
        new CMailboxPopupButton(false, obj16,
                    JXWidget::kHElastic, JXWidget::kVElastic, 300,22, 30,16);
    assert( mCopyToMailboxPopup != NULL );

// end JXLayout1

	// Do locks
	if (CAdminLock::sAdminLock.mLockReturnAddress)
		mEmailAddress->Deactivate();
	if (CAdminLock::sAdminLock.mLockServerAddress)
	{
		mIMAPip->Deactivate();
		if (!CAdminLock::sAdminLock.mNoLockSMTP)
			mSMTPip->Deactivate();
		mServerTypeGroup->Deactivate();
	}
	if (CAdminLock::sAdminLock.mLockSavePswd)
		mSavePswd->Deactivate();

	mSignatureBtn->SetImage(CIconLoader::GetIcon(IDI_SIGNATURE, mSignatureBtn, 32, 0x00CCCCCC), kFalse);
	if (!CMulberryApp::sApp->LoadedPrefs())
		mCopyToMailboxPopup->Deactivate();

	ListenTo(mServerTypeGroup);
	ListenTo(mCheckMailGroup);
	ListenTo(mSignatureBtn);
	ListenTo(mDoCopyTo);
	ListenTo(mCopyToMailboxPopup);
}

void CPrefsSimple::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		if (sender == mServerTypeGroup)
		{
			switch(index)
			{
			case 1:
				SetType(CINETAccount::eIMAP);
				break;
			case 2:
				SetType(CINETAccount::ePOP3);
				break;
			case 3:
				SetType(CINETAccount::eLocal);
				break;
			}
			return;
		}
		else if (sender == mCheckMailGroup)
		{
			switch(index)
			{
			case 1:
				mCheckInterval->Deactivate();
				break;
			case 2:
				mCheckInterval->Activate();
				mCheckInterval->Focus();
				mCheckInterval->SelectAll();
				break;
			}
			return;
		}
	}
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mSignatureBtn)
		{
			OnSignatureBtn();
			return;
		}
	}
	else if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mDoCopyTo)
		{
			OnCopyTo();
			return;
		}
	}
	else if(message.Is(JXMenu::kItemSelected))
	{
    	if (sender == mCopyToMailboxPopup)
    	{
			const JXMenu::ItemSelected* is = 
				dynamic_cast<const JXMenu::ItemSelected*>(&message);			
			OnChangeCopyTo(is->GetIndex());
			return;
		} 
		
	}
	CPrefsPanel::Receive(sender, message);
}

// Set up params for DDX
void CPrefsSimple::SetPrefs(CPreferences* prefs)
{
	// Save ref to prefs
	mCopyPrefs = prefs;
	CPreferences* local_prefs = prefs;
	cdstring copyStr;

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
		copyStr = addr_list.front()->GetName();
		mRealName->SetText(copyStr);

		// Always display complete address to avoid confusion
		//copyStr = (CAdminLock::sAdminLock.mLockReturnAddress ? CAdminLock::sAdminLock.mLockedReturnDomain : addr_list.front()->GetMailAddress());
		copyStr = addr_list.front()->GetMailAddress();
		mEmailAddress->SetText(copyStr);
	}

	if (auth)
	{
		copyStr = auth->GetUID();
		mIMAPuid->SetText(copyStr);
	}
	else
		mIMAPuid->Deactivate();

	copyStr = imap->GetServerIP();
	mIMAPip->SetText(copyStr);

	SetType(imap->GetServerType());
	switch(imap->GetServerType())
	{
	case CINETAccount::eIMAP:
	default:
		mServerTypeGroup->SelectItem(1);
		break;

	case CINETAccount::ePOP3:
		mServerTypeGroup->SelectItem(2);
		break;

	case CINETAccount::eLocal:
		mServerTypeGroup->SelectItem(3);
		break;
	}

	copyStr = smtp->GetServerIP();
	mSMTPip->SetText(copyStr);

	if (auth)
	{
		mSaveUser->SetState(JBoolean(auth->GetSaveUID()));
		mSavePswd->SetState(JBoolean(auth->GetSavePswd() && !CAdminLock::sAdminLock.mLockSavePswd));
	}

	mLogonStartup->SetState(JBoolean(imap->GetLogonAtStart()));
	mLeaveOnServer->SetState(JBoolean(imap->GetLeaveOnServer()));

	mCheckMailGroup->SelectItem(notify.IsEnabled() ? 2 : 1);

	copyStr = (long) notify.GetCheckInterval();
	mCheckInterval->SetText(copyStr);
	if (notify.IsEnabled())
		mCheckInterval->Activate();
	else
		mCheckInterval->Deactivate();

	mNewMailAlert->SetState(JBoolean(notify.DoShowAlertForeground()));

	if (imap->GetServerType() == CINETAccount::eIMAP)
	{
		if (imap->GetDirDelim())
			copyStr = imap->GetDirDelim();
		else
			copyStr = cdstring::null_str;
		mDirectorySeparator->SetText(copyStr);
	}

	mSignature = id->GetSignature();

	mDoCopyTo->SetState(JBoolean(id->UseCopyTo() && !id->GetCopyToNone()));

	// Strip account name
	cdstring copy_to = id->GetCopyTo();
	if (::strchr(copy_to.c_str(), cMailAccountSeparator))
		copyStr = ::strchr(copy_to.c_str(), cMailAccountSeparator) + 1;
	else
		copyStr = copy_to;
	mCopyToMailbox->SetText(copyStr);

	if (!mDoCopyTo->IsChecked())
	{
		mCopyToMailbox->Deactivate();
		mCopyToMailboxPopup->Deactivate();
	}
}

// Get params from controls
void CPrefsSimple::UpdatePrefs(CPreferences* prefs)
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
	switch(mServerTypeGroup->GetSelectedItem())
	{
	case 1:
		type = CINETAccount::eIMAP;
		break;
	case 2:
		type = CINETAccount::ePOP3;
		break;
	case 3:
		type = CINETAccount::eLocal;
		break;
	}

	if (type != imap->GetServerType())
	{
		// May need to change name
		bool change_name = false;
		switch(imap->GetServerType())
		{
		case CINETAccount::eIMAP:
		default:;
			change_name = (imap->GetName() == "IMAP");
			break;
		case CINETAccount::ePOP3:
			change_name = (imap->GetName() == "POP3");
			break;
		case CINETAccount::eLocal:
			change_name = (imap->GetName() == "Local");
			break;
		}
		imap->SetServerType(type);

		// Change name only if a single account exists
		if (change_name && (local_prefs->mMailAccounts.GetValue().size() == 1))
		{
			switch(imap->GetServerType())
			{
			case CINETAccount::eIMAP:
			default:;
				local_prefs->RenameAccount(imap, "IMAP");
				break;
			case CINETAccount::ePOP3:
				local_prefs->RenameAccount(imap, "POP3");
				break;
			case CINETAccount::eLocal:
				local_prefs->RenameAccount(imap, "Local");
				break;
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
		cdstring temp = mIMAPuid->GetText();
		auth->SetUID(temp);
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
		auth->SetSaveUID(mSaveUser->IsChecked());

	if ((type != CINETAccount::eLocal) && auth && !CAdminLock::sAdminLock.mLockSavePswd)
		auth->SetSavePswd(mSavePswd->IsChecked());

	if (type == CINETAccount::eIMAP)
		imap->SetLoginAtStart(mLogonStartup->IsChecked());

	if (type == CINETAccount::ePOP3)
		imap->SetLeaveOnServer(mLeaveOnServer->IsChecked());

	notify.Enable(mCheckMailGroup->GetSelectedItem() == 2);
	{
		cdstring temp = mCheckInterval->GetText().GetCString();
		notify.SetCheckInterval(::atoi((char*) temp));
	}
	notify.SetShowAlertForeground(mNewMailAlert->IsChecked());

	if (type == CINETAccount::eIMAP)
	{
		cdstring temp = mDirectorySeparator->GetText();
		if (temp.length())
			imap->SetDirDelim(temp[0UL]);
		else
			imap->SetDirDelim(0);
	}

	id->SetSignature(mSignature, mSignature.length());

	// Add account prefix
	cdstring temp = mCopyToMailbox->GetText();
	cdstring copy_to = (temp.length() ? imap->GetName() + cMailAccountSeparator : cdstring::null_str);
	copy_to += temp;

	// Always activate copy to, do copy to none if no copy requested, do copy choose if mailbox name empty
	id->SetCopyTo(copy_to, true);
	id->SetCopyToNone(!mDoCopyTo->IsChecked());
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

/////////////////////////////////////////////////////////////////////////////
// CPrefsSimple message handlers

void CPrefsSimple::SetType(CINETAccount::EINETServerType type)
{
	switch(type)
	{
	case CINETAccount::eIMAP:
	default:
		mIMAPuid->Activate();
		if (!CAdminLock::sAdminLock.mLockServerAddress)
			mIMAPip->Activate();
		mSaveUser->Show();
		mSavePswd->Show();
		mLogonStartup->Show();
		mLeaveOnServer->Hide();
		mDirectorySeparatorTitle->Show();
		mDirectorySeparator->Show();
		break;

	case CINETAccount::ePOP3:
		mIMAPuid->Activate();
		if (!CAdminLock::sAdminLock.mLockServerAddress)
			mIMAPip->Activate();
		mSaveUser->Show();
		mSavePswd->Show();
		mLogonStartup->Hide();
		mLeaveOnServer->Show();
		mDirectorySeparatorTitle->Hide();
		mDirectorySeparator->Hide();
		break;

	case CINETAccount::eLocal:
		mIMAPuid->Deactivate();
		mIMAPip->Deactivate();
		mSaveUser->Hide();
		mSavePswd->Hide();
		mLogonStartup->Hide();
		mLeaveOnServer->Hide();
		mDirectorySeparatorTitle->Hide();
		mDirectorySeparator->Hide();
		break;
	}
}

void CPrefsSimple::OnSignatureBtn()
{
	cdstring title = "Set Signature";
	CPrefsEditHeadFoot::PoseDialog(mSignature, title, mCopyPrefs, true);
}

void CPrefsSimple::OnCopyTo()
{
	// TODO: Add your control notification handler code here
	if (mDoCopyTo->IsChecked())
	{
		mCopyToMailbox->Activate();
		mCopyToMailbox->Focus();
		mCopyToMailbox->SelectAll();
		if (CMulberryApp::sApp->LoadedPrefs())
			mCopyToMailboxPopup->Activate();
	}
	else
	{
		mCopyToMailbox->Deactivate();
		mCopyToMailboxPopup->Deactivate();
	}
}

// Change copy to
void CPrefsSimple::OnChangeCopyTo(JIndex nID)
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
				copyStr = mbox_name.c_str();
			mCopyToMailbox->SetText(copyStr);
		}
	}
}
