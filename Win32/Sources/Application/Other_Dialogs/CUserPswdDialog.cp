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


// CUserPswdDialog.cpp : implementation file
//


#include "CUserPswdDialog.h"

#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"
#include "CXStringResources.h"

/////////////////////////////////////////////////////////////////////////////
// CUserPswdDialog dialog


CUserPswdDialog::CUserPswdDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CUserPswdDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUserPswdDialog)
	//}}AFX_DATA_INIT
	
	mTimerID = 0;
}


void CUserPswdDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUserPswdDialog)
	DDX_UTF8Text(pDX, IDC_USERPSWDDESCRIPTION, m_Description);
	DDX_UTF8Text(pDX, IDC_SERVERNAME, m_ServerName);
	DDX_UTF8Text(pDX, IDC_USERPSWDAUTHMETHOD, m_Method);
	DDX_UTF8Text(pDX, IDC_USERPSWDSECURE, m_Secure);
	DDX_UTF8Text(pDX, IDC_USERIDEDIT, m_UserID);
	if (!pDX->m_bSaveAndValidate)
		DDX_UTF8Text(pDX, IDC_USERIDSTATIC, m_UserID);
	DDX_UTF8Text(pDX, IDC_PASSWORD, m_Password);
	DDX_Control(pDX, IDC_USERPSWDCAPSLOCK, mCapsLock);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUserPswdDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CUserPswdDialog)
		ON_WM_TIMER()
		ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUserPswdDialog message handlers

BOOL CUserPswdDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// TODO: Add extra initialization here

	// Set icon
	mIcon.SubclassDlgItem(IDC_USERPSWDICON, this, m_IconID);

	// Set initial user id & password
	//SetDlgItemText(IDC_USERPSWDDESCRIPTION, m_Description);
	//SetDlgItemText(IDC_SERVERNAME, m_ServerName);
	//SetDlgItemText(IDC_USERPSWDAUTHMETHOD, m_Method);
	//SetDlgItemText(IDC_USERPSWDSECURE, m_Secure);
	//SetDlgItemText(IDC_USERIDEDIT, m_UserID);
	//SetDlgItemText(IDC_USERIDSTATIC, m_UserID);
	//SetDlgItemText(IDC_PASSWORD, m_Password);

	// Hide user id if required
	if (m_SaveUser)
		GetDlgItem(IDC_USERIDEDIT)->ModifyStyle(WS_VISIBLE, WS_DISABLED);
	else
		GetDlgItem(IDC_USERIDSTATIC)->ModifyStyle(WS_VISIBLE, WS_DISABLED);

	if (!m_SaveUser && (m_UserID.empty() || m_SavePswd))
	{
		GetDlgItem(IDC_USERIDEDIT)->SetFocus();
		static_cast<CEdit*>(GetDlgItem(IDC_USERIDEDIT))->SetSel(0, -1);
	}
	else
	{
		GetDlgItem(IDC_PASSWORD)->SetFocus();
		static_cast<CEdit*>(GetDlgItem(IDC_PASSWORD))->SetSel(0, -1);
	}

	mCapsLock.ShowWindow(SW_HIDE);
	mCaps = false;

	// Start a timer
	mTimerID = SetTimer('Pswd', 250, NULL);

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUserPswdDialog::OnDestroy(void)
{
	// Do not allow shared menus to be deleted
	KillTimer(mTimerID);
	mTimerID = 0;

	CHelpDialog::OnDestroy();
}

void CUserPswdDialog::OnTimer(UINT nIDEvent)
{
	if ((::GetKeyState(VK_CAPITAL) & 0x0001) ^ mCaps)
	{
		mCaps = !mCaps;
		mCapsLock.ShowWindow(mCaps ? SW_SHOW : SW_HIDE);
	}
}

bool CUserPswdDialog::PoseDialog(cdstring& uid, cdstring& pswd, bool save_user, bool save_pswd,
									const cdstring& title, const cdstring& server_ip, const cdstring& method, bool secure, int icon)
{
	// Create the dialog
	CUserPswdDialog dlog(CSDIFrame::GetAppTopWindow());

	dlog.m_IconID = icon;
	dlog.m_Description = title;
	dlog.m_ServerName = server_ip;
	dlog.m_Method = method;
	dlog.m_Secure = rsrc::GetString(secure ? "UI::UserPswd::Secure" : "UI::UserPswd::Insecure");
	dlog.m_UserID = uid;
	dlog.m_Password = pswd;
	dlog.m_SaveUser = save_user;
	dlog.m_SavePswd = save_pswd;

	int result = dlog.DoModal();

	// Let DialogHandler process events
	if (result == IDOK)
	{
		// Get items from dlog
		if (!save_user)
			uid = dlog.m_UserID;

		pswd = dlog.m_Password;

		return true;
	}
	else
		return false;
}
