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


// CEditIdentitySecurity.cpp : implementation file
//

#include "CEditIdentitySecurity.h"

#include "CPreferences.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CEditIdentitySecurity dialog

IMPLEMENT_DYNAMIC(CEditIdentitySecurity, CTabPanel)

CEditIdentitySecurity::CEditIdentitySecurity()
	: CTabPanel(CEditIdentitySecurity::IDD)
{
	//{{AFX_DATA_INIT(CEditIdentitySecurity)
	mActive = FALSE;
	mSign = FALSE;
	mEncrypt = FALSE;
	//}}AFX_DATA_INIT
}

void CEditIdentitySecurity::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditIdentitySecurity)
	DDX_Check(pDX, IDC_IDENTITY_SECURITY_ACTIVE, mActive);
	DDX_Check(pDX, IDC_IDENTITY_SIGN, mSign);
	DDX_Check(pDX, IDC_IDENTITY_ENCRYPT, mEncrypt);
	DDX_UTF8Text(pDX, IDC_IDENTITY_KEY_EDIT, mSignOther);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditIdentitySecurity, CTabPanel)
	//{{AFX_MSG_MAP(CEditIdentitySecurity)
	ON_BN_CLICKED(IDC_IDENTITY_SECURITY_ACTIVE, OnActive)
	ON_COMMAND_RANGE(IDM_IDENTITY_DEFAULT_KEY, IDM_IDENTITY_OTHER_KEY, OnSignWithPopup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditIdentitySecurity message handlers

// Set data
BOOL CEditIdentitySecurity::OnInitDialog()
{
	// Do inherited
	CTabPanel::OnInitDialog();

	// Subclass buttons
	mSignWithPopup.SubclassDlgItem(IDC_IDENTITY_KEY_POPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mSignWithPopup.SetMenu(IDR_POPUP_IDENTITY_SIGNWITH);

	// Active/deactivate panel
	SetActive(mActive);

	return TRUE;
}

// Set data
void CEditIdentitySecurity::SetContent(void* data)
{
	CIdentity* identity = (CIdentity*) data;

	// Copy info
	mActive = identity->GetUseSecurity();
	mSign = identity->GetSign();
	mEncrypt = identity->GetEncrypt();
	OnSignWithPopup(IDM_IDENTITY_DEFAULT_KEY + identity->GetSignWith());
	mSignOther = identity->GetSignOther();

	// Active/deactivate panel
	SetActive(mActive);
}

// Force update of data
bool CEditIdentitySecurity::UpdateContent(void* data)
{
	CIdentity* identity = (CIdentity*) data;

	identity->SetUseSecurity(mActive);
	identity->SetSign(mSign);
	identity->SetEncrypt(mEncrypt);

	identity->SetSignWith(static_cast<CIdentity::ESignWith>(mSignWithPopup.GetValue() - IDM_IDENTITY_DEFAULT_KEY), mSignOther);

	return true;
}

void CEditIdentitySecurity::OnActive() 
{
	// Change active state
	mActive = !mActive;

	// Active/deactivate panel
	SetActive(mActive);	
}

void CEditIdentitySecurity::OnSignWithPopup(UINT nID) 
{	
	mSignWithPopup.SetValue(nID);
	
	GetDlgItem(IDC_IDENTITY_KEY_EDIT)->EnableWindow(nID == IDM_IDENTITY_OTHER_KEY);
}

void CEditIdentitySecurity::SetActive(bool active) 
{
	GetDlgItem(IDC_IDENTITY_SIGN)->EnableWindow(active);
	GetDlgItem(IDC_IDENTITY_ENCRYPT)->EnableWindow(active);
	GetDlgItem(IDC_IDENTITY_KEY_POPUP)->EnableWindow(active);
	
	UINT nID = mSignWithPopup.GetValue();
	GetDlgItem(IDC_IDENTITY_KEY_EDIT)->EnableWindow(active && (nID == IDM_IDENTITY_OTHER_KEY));
}
