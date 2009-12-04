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

// CDisconnectDialog.cpp : implementation file
//

#include "CDisconnectDialog.h"

#include "CConnectionManager.h"
#include "CErrorHandler.h"
#include "CMailAccountManager.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CSMTPAccountManager.h"
#include "CSMTPWindow.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CDisconnectDialog dialog

bool CDisconnectDialog::sIsTwisted = false;

CDisconnectDialog* CDisconnectDialog::sDisconnectDialog = NULL;

CDisconnectDialog::CDisconnectDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CDisconnectDialog::IDD, pParent), mSyncTargets(true)
{
	//{{AFX_DATA_INIT(CDisconnectDialog)
	mAll = -1;
	mList = FALSE;
	mWait = FALSE;
	mPermanent = -1;
	mPlayback = FALSE;
	mPOP3 = FALSE;
	mSend = FALSE;
	mFull = -1;
	mSize = 0;
	mAdbk1 = FALSE;
	mAdbk2 = FALSE;
	//}}AFX_DATA_INIT
	mProgressVisible = true;

	sDisconnectDialog = this;
}

// Default destructor
CDisconnectDialog::~CDisconnectDialog()
{
	sDisconnectDialog = NULL;
}


void CDisconnectDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDisconnectDialog)
	DDX_Radio(pDX, IDC_DISCONNECT_ALL, mAll);
	DDX_Check(pDX, IDC_DISCONNECT_LIST, mList);
	DDX_Check(pDX, IDC_DISCONNECT_WAIT, mWait);
	DDX_Radio(pDX, IDC_DISCONNECT_PERMANENT, mPermanent);
	DDX_Check(pDX, IDC_DISCONNECT_PLAYBACK, mPlayback);
	DDX_Check(pDX, IDC_DISCONNECT_POP3, mPOP3);
	DDX_Check(pDX, IDC_DISCONNECT_SEND, mSend);
	DDX_Radio(pDX, IDC_DISCONNECT_FULL, mFull);
	DDX_UTF8Text(pDX, IDC_DISCONNECT_SIZE, mSize);
	DDX_Check(pDX, IDC_DISCONNECT_ADBK1, mAdbk1);
	DDX_Check(pDX, IDC_DISCONNECT_ADBK2, mAdbk2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDisconnectDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CDisconnectDialog)
	ON_BN_CLICKED(IDC_DISCONNECT_ALL, OnDisconnectAll)
	ON_BN_CLICKED(IDC_DISCONNECT_NEW, OnDisconnectNew)
	ON_BN_CLICKED(IDC_DISCONNECT_NONE, OnDisconnectNone)
	ON_BN_CLICKED(IDC_DISCONNECT_PERMANENT, OnDisconnectPermanent)
	ON_BN_CLICKED(IDC_DISCONNECT_UPDATE, OnDisconnectUpdate)
	ON_BN_CLICKED(IDC_DISCONNECT_FULL, OnDisconnectFull)
	ON_BN_CLICKED(IDC_DISCONNECT_BELOW, OnDisconnectBelow)
	ON_BN_CLICKED(IDC_DISCONNECT_PARTIAL, OnDisconnectPartial)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDisconnectDialog message handlers

BOOL CDisconnectDialog::OnInitDialog() 
{
	CHelpDialog::OnInitDialog();
	
	// Get size of this dialog
	CRect r;
	GetWindowRect(r);
	CRect rc;
	GetClientRect(rc);

	// Reset size
	const int cExtraHeight = 214;
	MoveWindow(r.left, r.top, r.Width(), r.Height() + cExtraHeight);

	// Create mailbox list area
	rc = CRect(0, rc.Height(), rc.Width(), rc.Height() + cExtraHeight);
	mMailboxListPanel.Create(_T(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP, rc, this, IDC_STATIC);
	mMailboxListPanel.CreateSelf(this, NULL, NULL, rc.Width(), rc.Height());
	mMailboxListPanel.InitCabinets();
	mMailboxListPanel.OnCabinet(CMailAccountManager::eFavouriteAutoSync + IDM_SEARCH_CABINETS_First);

	// Recenter on screen while its still at full size
	CenterWindow(CWnd::GetDesktopWindow());

	mMailboxListPanel.OnTwist();

	// Do twist reset
	if (sIsTwisted)
		mMailboxListPanel.OnTwist();

	mProgress1.SubclassDlgItem(IDC_DISCONNECT_PROGRESS_ACCOUNTS, this);
	mProgress2.SubclassDlgItem(IDC_DISCONNECT_PROGRESS_ITEMS, this);

	// Hide unwanted panels
	if (mConnecting)
	{
		GetDlgItem(IDC_DISCONNECT_DISCONNECTING)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DISCONNECT_ALL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DISCONNECT_NEW)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DISCONNECT_NONE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DISCONNECT_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DISCONNECT_WAIT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DISCONNECT_ADBK2)->ShowWindow(SW_HIDE);
		
		EnableMessageItems(mPermanent);
		if (mPermanent == 0)
			ShowProgressPanel(false);
		
		// Hide address books if no remote accounts, else disable
		if (!CPreferences::sPrefs->mAddressAccounts.GetValue().size())
			GetDlgItem(IDC_DISCONNECT_ADBK1)->ShowWindow(SW_HIDE);

		cdstring title;
		title.FromResource("UI::Disconnect::Connect");
		CUnicodeUtils::SetWindowTextUTF8(this, title);
	}
	else
	{
		GetDlgItem(IDC_DISCONNECT_CONNECTING)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DISCONNECT_PERMANENT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DISCONNECT_UPDATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DISCONNECT_PLAYBACK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DISCONNECT_POP3)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DISCONNECT_SEND)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DISCONNECT_ADBK1)->ShowWindow(SW_HIDE);
		
		EnableMessageItems(mAll != 2);

		// Hide address books if no remote accounts
		if (!CPreferences::sPrefs->mAddressAccounts.GetValue().size())
			GetDlgItem(IDC_DISCONNECT_ADBK2)->ShowWindow(SW_HIDE);

		cdstring title;
		title.FromResource("UI::Disconnect::Disconnect");
		CUnicodeUtils::SetWindowTextUTF8(this, title);
	}

	if (mFull != 1)
		GetDlgItem(IDC_DISCONNECT_SIZE)->EnableWindow(false);

	// Always hide playback progress
	ShowPlaybackProgressPanel(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDisconnectDialog::OnDisconnectAll() 
{
	EnableMessageItems(true);
}

void CDisconnectDialog::OnDisconnectNew() 
{
	EnableMessageItems(true);
}

void CDisconnectDialog::OnDisconnectNone() 
{
	EnableMessageItems(false);
}

void CDisconnectDialog::OnDisconnectPermanent() 
{
	//::SendMessage(GetDlgItem(IDC_DISCONNECT_PLAYBACK)->GetSafeHwnd(), BM_SETCHECK, 1, 0);
	//::SendMessage(GetDlgItem(IDC_DISCONNECT_WAIT)->GetSafeHwnd(), BM_SETCHECK, 1, 0);

	EnableMessageItems(false);
	ShowProgressPanel(false);
}

void CDisconnectDialog::OnDisconnectUpdate() 
{
	//::SendMessage(GetDlgItem(IDC_DISCONNECT_PLAYBACK)->GetSafeHwnd(), BM_SETCHECK, 0, 0);
	//::SendMessage(GetDlgItem(IDC_DISCONNECT_WAIT)->GetSafeHwnd(), BM_SETCHECK, 0, 0);

	EnableMessageItems(true);
	ShowProgressPanel(true);
}

void CDisconnectDialog::OnDisconnectFull() 
{
	GetDlgItem(IDC_DISCONNECT_SIZE)->EnableWindow(false);
}

void CDisconnectDialog::OnDisconnectBelow() 
{
	GetDlgItem(IDC_DISCONNECT_SIZE)->EnableWindow(true);
}

void CDisconnectDialog::OnDisconnectPartial() 
{
	GetDlgItem(IDC_DISCONNECT_SIZE)->EnableWindow(false);
}

void CDisconnectDialog::EnableMessageItems(bool enable)
{
	if (mConnecting)
		GetDlgItem(IDC_DISCONNECT_POP3)->EnableWindow(enable);

	GetDlgItem(IDC_DISCONNECT_MESSAGE)->EnableWindow(enable);
	GetDlgItem(IDC_DISCONNECT_FULL)->EnableWindow(enable);
	GetDlgItem(IDC_DISCONNECT_BELOW)->EnableWindow(enable);
	GetDlgItem(IDC_DISCONNECT_PARTIAL)->EnableWindow(enable);
	bool below_set = ::SendMessage(GetDlgItem(IDC_DISCONNECT_BELOW)->GetSafeHwnd(), BM_GETCHECK, 0, 0);
	GetDlgItem(IDC_DISCONNECT_SIZE)->EnableWindow(below_set && enable);
}

// Show or hide the progress panel
void CDisconnectDialog::ShowProgressPanel(bool show)
{
	const int cShrinkOffset = 12;
	if (show && !mProgressVisible)
	{
		CRect r;
		mMailboxListPanel.GetWindowRect(r);
		mMailboxListPanel.ShowWindow(SW_SHOW);
		::ResizeWindowBy(this, 0, r.Height() - cShrinkOffset);
		mProgressVisible = true;
	}
	else if (!show && mProgressVisible)
	{
		CRect r;
		mMailboxListPanel.GetWindowRect(r);
		mMailboxListPanel.ShowWindow(SW_HIDE);
		::ResizeWindowBy(this, 0, -r.Height() + cShrinkOffset);
		mProgressVisible = false;
	}
}

// Show or hide the progress panel
void CDisconnectDialog::ShowPlaybackProgressPanel(bool show)
{
	GetDlgItem(IDC_DISCONNECT_MESSAGE)->ShowWindow(show ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_DISCONNECT_FULL)->ShowWindow(show ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_DISCONNECT_BELOW)->ShowWindow(show ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_DISCONNECT_SIZE)->ShowWindow(show ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_DISCONNECT_SIZE_UNITS)->ShowWindow(show ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_DISCONNECT_PARTIAL)->ShowWindow(show ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_DISCONNECT_PLAYBACKPROGRESS)->ShowWindow(show ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_DISCONNECT_PROGRESS_ACCOUNTS_TITLE)->ShowWindow(show ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_DISCONNECT_PROGRESS_ACCOUNTS)->ShowWindow(show ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_DISCONNECT_PROGRESS_ITEMS_TITLE)->ShowWindow(show ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_DISCONNECT_PROGRESS_ITEMS)->ShowWindow(show ? SW_SHOW : SW_HIDE);
}

// Set the dialogs info
void CDisconnectDialog::SetDetails(bool connecting)
{
	// Get dis/connection options from prefs
	const CConnectionManager::CConnectOptions& connect = CPreferences::sPrefs->mConnectOptions.GetValue();
	const CConnectionManager::CDisconnectOptions& disconnect = CPreferences::sPrefs->mDisconnectOptions.GetValue();

	// Disable unwanted options
	if (connecting)
	{
		mPermanent = connect.mStayConnected ? 0 : 1;
		mPlayback = connect.mMboxPlayback;
		mPOP3 = connect.mUpdatePOP3;
		mSend = connect.mSMTPSend;

		mAdbk1 = connect.mAdbkPlayback;
		
		mConnecting = true;
	}
	else
	{
		mAll = disconnect.mMboxSync;
		mList = disconnect.mListSync;
		mWait = disconnect.mSMTPWait;
		
		mAdbk2 = disconnect.mAdbkSync;
		
		mConnecting = false;
	}

	mFull = disconnect.mMsgSync;
	mSize = disconnect.mMsgSyncSize/1024;
}

// Get the details
void CDisconnectDialog::GetDetailsDisconnect()
{
	// Get disconnection options from prefs
	CConnectionManager::CDisconnectOptions orig_disconnect = CPreferences::sPrefs->mDisconnectOptions.GetValue();
	CConnectionManager::CDisconnectOptions& disconnect = CPreferences::sPrefs->mDisconnectOptions.Value();

	// Sync if requested
	disconnect.mMboxSync = static_cast<CConnectionManager::EMboxSync>(mAll);

	// Mailbox list sync
	disconnect.mListSync = mList;

	// Wait for sends to complete
	disconnect.mSMTPWait = mWait;

	// Message options
	disconnect.mMsgSync = static_cast<CConnectionManager::EMessageSync>(mFull);
	disconnect.mMsgSyncSize = mSize * 1024;

	// Adbk sync
	disconnect.mAdbkSync = mAdbk2;
	
	// Now set up progress items
	if (disconnect.mMboxSync != CConnectionManager::eNoMessages)
	{
		mMailboxListPanel.SetFound(0);
		mMailboxListPanel.SetMessages(0);
		mMailboxListPanel.mProgress.SetTotal(mSyncTargets.size());
		mMailboxListPanel.mProgress.ShowWindow(SW_SHOW);
	}
	
	// Mark prefs as dirty if needed
	if (!(orig_disconnect == disconnect))
		CPreferences::sPrefs->mDisconnectOptions.SetDirty();
}

// Get the details
void CDisconnectDialog::GetDetailsConnect()
{
	// Get dis/connection options from prefs
	CConnectionManager::CConnectOptions orig_connect = CPreferences::sPrefs->mConnectOptions.Value();
	CConnectionManager::CConnectOptions& connect = CPreferences::sPrefs->mConnectOptions.Value();
	CConnectionManager::CDisconnectOptions orig_disconnect = CPreferences::sPrefs->mDisconnectOptions.Value();
	CConnectionManager::CDisconnectOptions& disconnect = CPreferences::sPrefs->mDisconnectOptions.Value();

	// Update if requested
	connect.mStayConnected = (mPermanent == 0);

	// Do playback
	connect.mMboxPlayback = mPlayback;

	// POP3 update
	connect.mUpdatePOP3 = mPOP3;

	// Enable SMTP
	connect.mSMTPSend = mSend;

	// Message options
	disconnect.mMsgSync = static_cast<CConnectionManager::EMessageSync>(mFull);
	disconnect.mMsgSyncSize = mSize * 1024;

	// Adbk sync
	connect.mAdbkPlayback = mAdbk1;

	// Now switch in progress items
	ShowPlaybackProgressPanel(true);
	
	// Now set up progress items
	if (!connect.mStayConnected)
	{
		mMailboxListPanel.SetFound(0);
		mMailboxListPanel.SetMessages(0);
		mMailboxListPanel.mProgress.SetTotal(mSyncTargets.size());
		mMailboxListPanel.mProgress.ShowWindow(SW_SHOW);
	}
	
	// Mark prefs as dirty if needed
	if (!(orig_connect == connect))
		CPreferences::sPrefs->mConnectOptions.SetDirty();
	if (!(orig_disconnect == disconnect))
		CPreferences::sPrefs->mDisconnectOptions.SetDirty();
}

void CDisconnectDialog::NextItem(unsigned long item)
{
	// Set pointer in list
	mMailboxListPanel.mMailboxList.SetIcon(item + 1, IDI_SEARCH_POINTER);
	STableCell aCell(item + 1, 1);
	mMailboxListPanel.mMailboxList.ScrollCellIntoFrame(aCell);
}

void CDisconnectDialog::DoneItem(unsigned long item, bool hit)
{
	// Set hit on previous
	mMailboxListPanel.SetHitState(item, hit, false);
	mMailboxListPanel.SetFound(item + 1);
	mMailboxListPanel.SetProgress(item + 1);
}

void CDisconnectDialog::OnOK()
{
	// Must get dialog data into local vars
	UpdateData(true);

	// Disable buttons
	GetDlgItem(IDOK)->EnableWindow(false);
	GetDlgItem(IDCANCEL)->EnableWindow(false);
	mMailboxListPanel.SetInProgress(true);
	GetDlgItem(IDC_DISCONNECT_ALL)->SetFocus();

	if (CConnectionManager::sConnectionManager.IsConnected())
	{
		GetDetailsDisconnect();
		
		// Do sync here while dialog is still open
		CConnectionManager::sConnectionManager.Disconnect(false, false, NULL);
	}
	else
	{
		// Preserve disconnect options
		CConnectionManager::CDisconnectOptions disconnect = CPreferences::sPrefs->mDisconnectOptions.Value();
		
		GetDetailsConnect();
		
		// Do sync here while dialog is still open
		CConnectionManager::sConnectionManager.Connect(&mProgress1, &mProgress2);

		// Recover disconnect options
		CPreferences::sPrefs->mDisconnectOptions.Value() = disconnect;
	}

	EndDialog(IDOK);
}

int CDisconnectDialog::DoModal()
{
	// Do inherited
	return CHelpDialog::DoModal();
}

void CDisconnectDialog::PoseDialog()
{
	bool disconnecting = CConnectionManager::sConnectionManager.IsConnected();

	// Check to see if any mail accounts are actually marked for disconnect
	if (disconnecting && !CMailAccountManager::sMailAccountManager->HasDisconnectedProtocols())
	{
		// Put up caution alert
		if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Disconnect::NoDisconnectedCaution") != CErrorHandler::Ok)
			return;
	}

	{
		// Create the dialog
		CDisconnectDialog dlog(CSDIFrame::GetAppTopWindow());
		dlog.SetDetails(!disconnecting);
		
		// Let DialogHandler process events
		int result = dlog.DoModal();
		
		// Get twist state
		sIsTwisted = dlog.mMailboxListPanel.mTwisted;

		if (result != IDOK)
			return;
	}

	CPreferences::sPrefs->mDisconnected.SetValue(!CConnectionManager::sConnectionManager.IsConnected());

	// Check to see whether there are messages available and force queue window on screen
	if (CSMTPAccountManager::sSMTPAccountManager->ItemsHeld())
		CSMTPWindow::OpenSMTPWindow();
}