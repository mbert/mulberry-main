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


// CEditIdentityDSN.cpp : implementation file
//

#include "CEditIdentityDSN.h"

#include "CIdentity.h"

/////////////////////////////////////////////////////////////////////////////
// CEditIdentityDSN dialog

IMPLEMENT_DYNAMIC(CEditIdentityDSN, CTabPanel)

CEditIdentityDSN::CEditIdentityDSN()
	: CTabPanel(CEditIdentityDSN::IDD)
{
	//{{AFX_DATA_INIT(CEditIdentityDSN)
	mActive = FALSE;
	mUseDSN = FALSE;
	mSuccess = FALSE;
	mFailure = FALSE;
	mDelay = FALSE;
	mFull = -1;
	//}}AFX_DATA_INIT
}


void CEditIdentityDSN::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditIdentityDSN)
	DDX_Check(pDX, IDC_IDENTITY_DSN_ACTIVE, mActive);
	DDX_Check(pDX, IDC_IDENTITY_DSN_USEDSN, mUseDSN);
	DDX_Check(pDX, IDC_IDENTITY_DSN_SUCCESS, mSuccess);
	DDX_Check(pDX, IDC_IDENTITY_DSN_FAILURE, mFailure);
	DDX_Check(pDX, IDC_IDENTITY_DSN_DELAY, mDelay);
	DDX_Radio(pDX, IDC_IDENTITY_DSN_FULL, mFull);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditIdentityDSN, CTabPanel)
	//{{AFX_MSG_MAP(CEditIdentityDSN)
	ON_BN_CLICKED(IDC_IDENTITY_DSN_ACTIVE, OnIdentityDSNActive)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditIdentityDSN message handlers

// Set data
BOOL CEditIdentityDSN::OnInitDialog()
{
	// Do inherited
	CTabPanel::OnInitDialog();

	// Active/deactivate panel
	EnablePanel(mActive);

	return TRUE;
}

// Set data
void CEditIdentityDSN::SetContent(void* data)
{
	CIdentity* identity = (CIdentity*) data;

	mActive = identity->GetUseDSN();
	mUseDSN = identity->GetDSN().GetRequest();
	mSuccess = identity->GetDSN().GetSuccess();
	mFailure = identity->GetDSN().GetFailure();
	mDelay = identity->GetDSN().GetDelay();
	mFull = identity->GetDSN().GetFull() ? 0 : 1;
	
	EnablePanel(mActive);
}

// Force update of data
bool CEditIdentityDSN::UpdateContent(void* data)
{
	CIdentity* identity = (CIdentity*) data;

	identity->SetUseDSN(mActive);
	identity->SetDSN().SetRequest(mUseDSN);
	identity->SetDSN().SetSuccess(mSuccess);
	identity->SetDSN().SetFailure(mFailure);
	identity->SetDSN().SetDelay(mDelay);
	identity->SetDSN().SetFull(mFull == 0);
	
	return true;
}

void CEditIdentityDSN::OnIdentityDSNActive() 
{
	// Change active state
	mActive = !mActive;

	// Active/deactivate panel
	EnablePanel(mActive);	
}

void CEditIdentityDSN::EnablePanel(bool enable) 
{
	GetDlgItem(IDC_IDENTITY_DSN_USEDSN)->EnableWindow(enable);
	GetDlgItem(IDC_IDENTITY_DSN_SUCCESS)->EnableWindow(enable);
	GetDlgItem(IDC_IDENTITY_DSN_FAILURE)->EnableWindow(enable);
	GetDlgItem(IDC_IDENTITY_DSN_DELAY)->EnableWindow(enable);
	GetDlgItem(IDC_IDENTITY_DSN_FULL)->EnableWindow(enable);
	GetDlgItem(IDC_IDENTITY_DSN_HEADERS)->EnableWindow(enable);
}
