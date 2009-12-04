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


// CPrefsAuthKerberos.cpp : implementation file
//

#include "CPrefsAuthKerberos.h"

#include "CAuthenticator.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAuthKerberos dialog

IMPLEMENT_DYNAMIC(CPrefsAuthKerberos, CTabPanel)

CPrefsAuthKerberos::CPrefsAuthKerberos()
	: CTabPanel(CPrefsAuthKerberos::IDD)
{
	//{{AFX_DATA_INIT(CPrefsAuthKerberos)
	mDefaultPrincipal = TRUE;
	mDefaultUID = TRUE;
	//}}AFX_DATA_INIT
}


void CPrefsAuthKerberos::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsAuthKerberos)
	DDX_Check(pDX, IDC_PREFS_AUTHENTICATE_KERBEROS_DEFAULTPRINCIPAL, mDefaultPrincipal);
	DDX_Control(pDX, IDC_PREFS_AUTHENTICATE_KERBEROS_DEFAULTPRINCIPAL, mDefaultPrincipalCtrl);
	DDX_UTF8Text(pDX, IDC_PREFS_AUTHENTICATE_KERBEROS_REALM, mPrincipal);
	DDX_Control(pDX, IDC_PREFS_AUTHENTICATE_KERBEROS_REALM, mPrincipalCtrl);
	DDX_Check(pDX, IDC_PREFS_AUTHENTICATE_KERBEROS_DEFAULTUID, mDefaultUID);
	DDX_Control(pDX, IDC_PREFS_AUTHENTICATE_KERBEROS_DEFAULTUID, mDefaultUIDCtrl);
	DDX_UTF8Text(pDX, IDC_PREFS_AUTHENTICATE_KERBEROS_UID, mUID);
	DDX_Control(pDX, IDC_PREFS_AUTHENTICATE_KERBEROS_UID, mUIDCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsAuthKerberos, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsAuthKerberos)
	ON_BN_CLICKED(IDC_PREFS_AUTHENTICATE_KERBEROS_DEFAULTPRINCIPAL, OnDefaultPrincipal)
	ON_BN_CLICKED(IDC_PREFS_AUTHENTICATE_KERBEROS_DEFAULTUID, OnDefaultUID)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsAuthKerberos message handlers

// Set data
void CPrefsAuthKerberos::SetContent(void* data)
{
	CAuthenticatorKerberos* auth = static_cast<CAuthenticatorKerberos*>(data);

	// Copy info
	mDefaultPrincipal = auth->GetDefaultPrincipal();
	mPrincipal = auth->GetServerPrincipal();
	mPrincipalCtrl.EnableWindow(!mDefaultPrincipal);
	mDefaultUID = auth->GetDefaultUID();
	mUID = auth->GetUID();
	mUIDCtrl.EnableWindow(!mDefaultUID);
}

// Force update of data
bool CPrefsAuthKerberos::UpdateContent(void* data)
{
	CAuthenticatorKerberos* auth = static_cast<CAuthenticatorKerberos*>(data);

	// Copy info from panel into prefs
	auth->SetUsePrincipal(mDefaultPrincipal);
	auth->SetServerPrincipal(mPrincipal);
	auth->SetUseUID(mDefaultUID);
	auth->SetUID(mUID);
	
	return true;
}

void CPrefsAuthKerberos::OnDefaultPrincipal()
{
	// TODO: Add your control notification handler code here
	bool set = mDefaultPrincipalCtrl.GetCheck();
	mPrincipalCtrl.EnableWindow(!set);
}

void CPrefsAuthKerberos::OnDefaultUID()
{
	// TODO: Add your control notification handler code here
	bool set = mDefaultUIDCtrl.GetCheck();
	mUIDCtrl.EnableWindow(!set);
}
