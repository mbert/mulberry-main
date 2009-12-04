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
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CPrefsEditHeadFoot.h"
#include "CRFC822.h"
#include "CTextEngine.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsSimple property page

IMPLEMENT_DYNCREATE(CPrefsSimple, CPrefsPanel)

CPrefsSimple::CPrefsSimple() : CPrefsPanel(CPrefsSimple::IDD)
{
	//{{AFX_DATA_INIT(CPrefsSimple)
	mServerType = 0;
	mSaveUser = FALSE;
	mSavePswd = FALSE;
	mLogonStartup = FALSE;
	mDoCheck = -1;
	mCheckInterval = 0;
	mNewMailAlert = FALSE;
	mDoCopyTo = FALSE;
	//}}AFX_DATA_INIT
}

CPrefsSimple::~CPrefsSimple()
{
}

void CPrefsSimple::DoDataExchange(CDataExchange* pDX)
{
	CPrefsPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsSimple)
	DDX_UTF8Text(pDX, IDC_SIMPLE_REALNAME, mRealName);
	DDX_UTF8Text(pDX, IDC_SIMPLE_EMAILADDR, mEmailAddress);
	DDX_Control(pDX, IDC_SIMPLE_EMAILADDR, mEmailAddressCtrl);
	DDX_UTF8Text(pDX, IDC_SIMPLE_IMAPUID, mIMAPuid);
	DDX_Control(pDX, IDC_SIMPLE_IMAPUID, mIMAPuidCtrl);
	DDX_UTF8Text(pDX, IDC_SIMPLE_IMAPIP, mIMAPip);
	DDX_Control(pDX, IDC_SIMPLE_IMAPIP, mIMAPipCtrl);
	DDX_Radio(pDX, IDC_SIMPLE_SERVERIMAP, mServerType);
	DDX_UTF8Text(pDX, IDC_SIMPLE_SMTPIP, mSMTPip);
	DDX_Control(pDX, IDC_SIMPLE_SMTPIP, mSMTPipCtrl);
	DDX_Check(pDX, IDC_SIMPLE_REMEMBERUID, mSaveUser);
	DDX_Control(pDX, IDC_SIMPLE_REMEMBERUID, mSaveUserCtrl);
	DDX_Check(pDX, IDC_SIMPLE_REMEMBERPSWD, mSavePswd);
	DDX_Control(pDX, IDC_SIMPLE_REMEMBERPSWD, mSavePswdCtrl);
	DDX_Check(pDX, IDC_SIMPLE_LOGINSTART, mLogonStartup);
	DDX_Control(pDX, IDC_SIMPLE_LOGINSTART, mLogonStartupCtrl);
	DDX_Check(pDX, IDC_SIMPLE_LEAVEONSERVER, mLeaveOnServer);
	DDX_Control(pDX, IDC_SIMPLE_LEAVEONSERVER, mLeaveOnServerCtrl);
	DDX_Radio(pDX, IDC_SIMPLE_CHECKNEVER, mDoCheck);
	DDX_UTF8Text(pDX, IDC_SIMPLE_CHECKINTERVAL, mCheckInterval);
	DDV_MinMaxInt(pDX, mCheckInterval, 0, 32767);
	DDX_Control(pDX, IDC_SIMPLE_CHECKINTERVAL, mCheckIntervalCtrl);
	DDX_Check(pDX, IDC_SIMPLE_USEALERT, mNewMailAlert);
	DDX_Control(pDX, IDC_SIMPLE_SEPARATORTITLE, mDirectorySeparatorTitle);
	DDX_UTF8Text(pDX, IDC_SIMPLE_SEPARATOR, mDirectorySeparator);
	DDV_UTF8MaxChars(pDX, mDirectorySeparator, 1);
	DDX_Control(pDX, IDC_SIMPLE_SEPARATOR, mDirectorySeparatorCtrl);
	DDX_Check(pDX, IDC_SIMPLE_COPYTO, mDoCopyTo);
	DDX_Control(pDX, IDC_SIMPLE_COPYTO, mDoCopyToCtrl);
	DDX_UTF8Text(pDX, IDC_SIMPLE_COPYMAILBOX, mCopyToMailbox);
	DDX_Control(pDX, IDC_SIMPLE_COPYMAILBOX, mCopyToMailboxCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsSimple, CPrefsPanel)
	//{{AFX_MSG_MAP(CPrefsSimple)
	ON_BN_CLICKED(IDC_SIMPLE_SERVERIMAP, OnServerIMAP)
	ON_BN_CLICKED(IDC_SIMPLE_SERVERPOP3, OnServerPOP3)
	ON_BN_CLICKED(IDC_SIMPLE_SERVERLOCAL, OnServerLocal)
	ON_BN_CLICKED(IDC_SIMPLE_CHECKEVERY, OnCheckEvery)
	ON_BN_CLICKED(IDC_SIMPLE_CHECKNEVER, OnCheckNever)
	ON_BN_CLICKED(IDC_SIMPLE_SIGNATURE, OnSignatureBtn)
	ON_BN_CLICKED(IDC_SIMPLE_COPYTO, OnCopyTo)
	ON_COMMAND_RANGE(IDM_AppendToPopupNone, IDM_AppendToPopupEnd, OnChangeCopyTo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Set up params for DDX
void CPrefsSimple::SetPrefs(CPreferences* prefs)
{
	// Save ref to prefs
	mCopyPrefs = prefs;

	// Get defaults
	const CMailAccount* imap = prefs->mMailAccounts.GetValue().front();
	const CAuthenticator* top_auth = &imap->GetAuthenticator();
	const CAuthenticatorUserPswd* auth = top_auth->RequiresUserPswd() ? static_cast<const CAuthenticatorUserPswd*>(top_auth->GetAuthenticator()) : nil;
	mDoesUIDPswd = top_auth->RequiresUserPswd();
	const CSMTPAccount* smtp = prefs->mSMTPAccounts.GetValue().front();
	const CIdentity* id = &prefs->mIdentities.GetValue().front();
	// Determine address list
	CAddressList addr_list(id->GetFrom().c_str(), id->GetFrom().length());
	if (!prefs->mMailNotification.GetValue().size())
	{
		CMailNotification notify;
		notify.SetName("Default");
		prefs->mMailNotification.Value().push_back(notify);
		prefs->mMailNotification.SetDirty();
	}
	const CMailNotification& notify = prefs->mMailNotification.GetValue().front();

	// Copy text to edit fields
	if (addr_list.size())
	{
		mRealName = addr_list.front()->GetName();

		// Always display complete address to avoid confusion
		//mEmailAddress = (CAdminLock::sAdminLock.mLockReturnAddress ? CAdminLock::sAdminLock.mLockedReturnDomain : addr_list.front()->GetMailAddress());
		mEmailAddress = addr_list.front()->GetMailAddress();
	}

	if (auth)
		mIMAPuid = auth->GetUID();
	else
		mIMAPuidCtrl.EnableWindow(false);

	mIMAPip = imap->GetServerIP();

	switch(imap->GetServerType())
	{
	case CINETAccount::eIMAP:
	default:
		mServerType = 0;
		break;

	case CINETAccount::ePOP3:
		mServerType = 1;
		break;

	case CINETAccount::eLocal:
		mServerType = 2;
		break;
	}

	mSMTPip = smtp->GetServerIP();

	if (auth)
	{
		mSaveUser = auth->GetSaveUID();
		mSavePswd = auth->GetSavePswd() && !CAdminLock::sAdminLock.mLockSavePswd;
	}

	mLogonStartup = imap->GetLogonAtStart();
	mLeaveOnServer = imap->GetLeaveOnServer();

	mDoCheck = notify.IsEnabled();

	mCheckInterval = notify.GetCheckInterval();
	
	mNewMailAlert = notify.DoShowAlertForeground();

	if (imap->GetServerType() == CINETAccount::eIMAP)
		mDirectorySeparator = imap->GetDirDelim();

	mSignature = id->GetSignature();

	mDoCopyTo = id->UseCopyTo() && !id->GetCopyToNone();

	// Strip account name
	cdstring copy_to = id->GetCopyTo();
	if (::strchr(copy_to.c_str(), cMailAccountSeparator))
		mCopyToMailbox = ::strchr(copy_to.c_str(), cMailAccountSeparator) + 1;
	else
		mCopyToMailbox = copy_to.c_str();
}

// Get params from DDX
void CPrefsSimple::UpdatePrefs(CPreferences* prefs)
{

	// Reset account type BEFORE doing anything else
	CMailAccount* imap = prefs->mMailAccounts.GetValue().front();
	CINETAccount::EINETServerType type = CINETAccount::eIMAP;
	if (mServerType == 0)
		type = CINETAccount::eIMAP;
	else if (mServerType == 1)
		type = CINETAccount::ePOP3;
	else if (mServerType == 2)
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
		}
		imap->SetServerType(type);

		// Change name only if a single account exists
		if (change_name && (prefs->mMailAccounts.GetValue().size() == 1))
		{
			switch(imap->GetServerType())
			{
			case CINETAccount::eIMAP:
				prefs->RenameAccount(imap, "IMAP");
				break;
			case CINETAccount::ePOP3:
				prefs->RenameAccount(imap, "POP3");
				break;
			case CINETAccount::eLocal:
				prefs->RenameAccount(imap, "Local");
				break;
			}
		}

		// Clear out WDs and reinit account
		imap->GetWDs().clear();
		imap->NewAccount();
	}

	// Now get other bits for comparison
	CAuthenticator* top_auth = &imap->GetAuthenticator();
	CAuthenticatorUserPswd* auth = top_auth->RequiresUserPswd() ? static_cast<CAuthenticatorUserPswd*>(top_auth->GetAuthenticator()) : nil;
	CSMTPAccount* smtp = prefs->mSMTPAccounts.GetValue().front();
	CIdentity* id = const_cast<CIdentity*>(&prefs->mIdentities.GetValue().front());

	// Determine address list
	CAddressList addr_list(id->GetFrom().c_str(), id->GetFrom().length());
	if (!prefs->mMailNotification.GetValue().size())
	{
		CMailNotification notify;
		notify.SetName("Default");
		prefs->mMailNotification.Value().push_back(notify);
		prefs->mMailNotification.SetDirty();
	}
	CMailNotification& notify = (CMailNotification&) prefs->mMailNotification.GetValue().front();

	// Make copies to look for changes
	CMailAccount copy_imap(*imap);
	CAuthenticatorUserPswd temp_auth;
	CAuthenticatorUserPswd copy_auth(auth ? *auth : temp_auth);
	CSMTPAccount copy_smtp(*smtp);
	CIdentity copy_id(*id);
	CMailNotification copy_notify(notify);

	// Copy info from panel into prefs
	{
		// Determine address list
		CAddressList addr_list(id->GetFrom().c_str(), id->GetFrom().length());

		if (CAdminLock::sAdminLock.mLockReturnAddress && addr_list.size())
			mEmailAddress = addr_list.front()->GetMailAddress();
		CAddress addr(mEmailAddress, mRealName);
		id->SetFrom(addr.GetFullAddress(), true);
	}

	if ((type != CINETAccount::eLocal) && auth)
		auth->SetUID(mIMAPuid);

	if (!CAdminLock::sAdminLock.mLockServerAddress)
	{
		cdstring temp = mIMAPip;
		temp.trimspace();
		imap->SetServerIP(temp);
	}

	// SMTP may be unlocked even though other servers are locked
	if (!CAdminLock::sAdminLock.mLockServerAddress || CAdminLock::sAdminLock.mNoLockSMTP)
	{
		cdstring temp = mSMTPip;
		temp.trimspace();
		smtp->SetServerIP(temp);
	}

	if ((type != CINETAccount::eLocal) && auth)
		auth->SetSaveUID(mSaveUser);

	if ((type != CINETAccount::eLocal) && auth && !CAdminLock::sAdminLock.mLockSavePswd)
		auth->SetSavePswd(mSavePswd);
		
	if (type == CINETAccount::eIMAP)
		imap->SetLoginAtStart(mLogonStartup);

	if (type == CINETAccount::ePOP3)
		imap->SetLeaveOnServer(mLeaveOnServer);

	notify.Enable(mDoCheck);
	notify.SetCheckInterval(mCheckInterval);
	notify.SetShowAlertForeground(mNewMailAlert);

	if (type == CINETAccount::eIMAP)
		imap->SetDirDelim(mDirectorySeparator[(cdstring::size_type)0]);

	id->SetSignature(mSignature, mSignature.length());

	// Add account prefix
	cdstring copy_to = mCopyToMailbox.length() ? imap->GetName() + cMailAccountSeparator : cdstring::null_str;
	copy_to += mCopyToMailbox;

	// Always active copy to, do copy to none if no copy requested, do copy choose if mailbox name is empty
	id->SetCopyTo(copy_to, true);
	id->SetCopyToNone(!mDoCopyTo);
	id->SetCopyToChoose(copy_to.empty());

	// Set dirty if required
	if (!(copy_imap == *imap))
		prefs->mMailAccounts.SetDirty();
	if (auth && !(copy_auth == *auth))
		prefs->mMailAccounts.SetDirty();
	if (!(copy_smtp == *smtp))
		prefs->mSMTPAccounts.SetDirty();
	if (!(copy_id == *id))
		prefs->mIdentities.SetDirty();
	if (!(copy_notify == notify))
		prefs->mMailNotification.SetDirty();
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsSimple message handlers

void CPrefsSimple::InitControls(void)
{
	// Subclass buttons
	mSignatureBtnCtrl.SubclassDlgItem(IDC_SIMPLE_SIGNATURE, this, IDI_SIGNATURE);
	mCopyToMailboxPopup.SubclassDlgItem(IDC_SIMPLE_COPYTOPOPUP, this, IDI_POPUPBTN);
	mCopyToMailboxPopup.SetButtonText(false);
	mCopyToMailboxPopup.SetCopyTo(false);
	mCopyToMailboxPopup.EnableWindow(CMulberryApp::sApp->LoadedPrefs());
}

void CPrefsSimple::SetControls(void)
{
	mEmailAddressCtrl.EnableWindow(!CAdminLock::sAdminLock.mLockReturnAddress);
	mIMAPipCtrl.EnableWindow(!CAdminLock::sAdminLock.mLockServerAddress);
	mSMTPipCtrl.EnableWindow(!CAdminLock::sAdminLock.mLockServerAddress || CAdminLock::sAdminLock.mNoLockSMTP);
	mIMAPuidCtrl.EnableWindow(mDoesUIDPswd);
	//mSaveUserCtrl.EnableWindow(mDoesUIDPswd);
	mSavePswdCtrl.EnableWindow(!CAdminLock::sAdminLock.mLockSavePswd);
	mCheckIntervalCtrl.EnableWindow(mDoCheck);
	mCopyToMailboxCtrl.EnableWindow(mDoCopyTo);
	mCopyToMailboxPopup.EnableWindow(mDoCopyTo && CMulberryApp::sApp->LoadedPrefs());
	
	if (mServerType == 0)
		SetType(CINETAccount::eIMAP);
	else if (mServerType == 1)
		SetType(CINETAccount::ePOP3);
	else if (mServerType == 2)
		SetType(CINETAccount::eLocal);
}

void CPrefsSimple::OnServerIMAP()
{
	SetType(CINETAccount::eIMAP);
}

void CPrefsSimple::OnServerPOP3()
{
	SetType(CINETAccount::ePOP3);
}

void CPrefsSimple::OnServerLocal()
{
	SetType(CINETAccount::eLocal);
}

void CPrefsSimple::OnCheckEvery()
{
	// TODO: Add your control notification handler code here
	mCheckIntervalCtrl.EnableWindow(true);
}

void CPrefsSimple::OnCheckNever()
{
	// TODO: Add your control notification handler code here
	mCheckIntervalCtrl.EnableWindow(false);
}

void CPrefsSimple::OnSignatureBtn()
{
	CPrefsEditHeadFoot::PoseDialog(mCopyPrefs, mSignature, true);
}

void CPrefsSimple::OnCopyTo()
{
	// TODO: Add your control notification handler code here
	bool set = mDoCopyToCtrl.GetCheck();
	mCopyToMailboxCtrl.EnableWindow(set);
	mCopyToMailboxPopup.EnableWindow(set && CMulberryApp::sApp->LoadedPrefs());
}

// Change copy to
void CPrefsSimple::OnChangeCopyTo(UINT nID)
{
	if (nID == IDM_CopyToPopupNone)
		CUnicodeUtils::SetWindowTextUTF8(&mCopyToMailboxCtrl, cdstring::null_str);
	else
	{
		// Must set/reset control value to ensure selected mailbox is returned but
		// popup does not display check mark
		mCopyToMailboxPopup.SetValue(nID);
		cdstring mbox_name;
		if (mCopyToMailboxPopup.GetSelectedMboxName(mbox_name, true))
		{
			cdstring copy_to;
			if (::strchr(mbox_name.c_str(), cMailAccountSeparator))
				copy_to = ::strchr(mbox_name.c_str(), cMailAccountSeparator) + 1;
			else
				copy_to = mbox_name.c_str();
			CUnicodeUtils::SetWindowTextUTF8(&mCopyToMailboxCtrl, copy_to);
		}
		mCopyToMailboxPopup.SetValue(-1);
	}
}


void CPrefsSimple::SetType(CINETAccount::EINETServerType type)
{
	switch(type)
	{
	case CINETAccount::eIMAP:
	default:
		mIMAPuidCtrl.EnableWindow(true);
		mIMAPipCtrl.EnableWindow(!CAdminLock::sAdminLock.mLockServerAddress);
		mSaveUserCtrl.ShowWindow(SW_SHOW);
		mSavePswdCtrl.ShowWindow(SW_SHOW);
		mLogonStartupCtrl.ShowWindow(SW_SHOW);
		mLeaveOnServerCtrl.ShowWindow(SW_HIDE);
		mDirectorySeparatorTitle.ShowWindow(SW_SHOW);
		mDirectorySeparatorCtrl.ShowWindow(SW_SHOW);
		break;

	case CINETAccount::ePOP3:
		mIMAPuidCtrl.EnableWindow(true);
		mIMAPipCtrl.EnableWindow(!CAdminLock::sAdminLock.mLockServerAddress);
		mSaveUserCtrl.ShowWindow(SW_SHOW);
		mSavePswdCtrl.ShowWindow(SW_SHOW);
		mLogonStartupCtrl.ShowWindow(SW_HIDE);
		mLeaveOnServerCtrl.ShowWindow(SW_SHOW);
		mDirectorySeparatorTitle.ShowWindow(SW_HIDE);
		mDirectorySeparatorCtrl.ShowWindow(SW_HIDE);
		break;

	case CINETAccount::eLocal:
		mIMAPuidCtrl.EnableWindow(false);
		mIMAPipCtrl.EnableWindow(false);
		mSaveUserCtrl.ShowWindow(SW_HIDE);
		mSavePswdCtrl.ShowWindow(SW_HIDE);
		mLogonStartupCtrl.ShowWindow(SW_HIDE);
		mLeaveOnServerCtrl.ShowWindow(SW_HIDE);
		mDirectorySeparatorTitle.ShowWindow(SW_HIDE);
		mDirectorySeparatorCtrl.ShowWindow(SW_HIDE);
		break;
	}
}
