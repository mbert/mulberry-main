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


// CEditIdentityOptions.cpp : implementation file
//

#include "CEditIdentityOptions.h"

#include "CAdminLock.h"
#include "CErrorHandler.h"
#include "CFontCache.h"
#include "CIdentity.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CPrefsEditHeadFoot.h"
#include "CRFC822.h"
#include "CTextEngine.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CEditIdentityOptions dialog

IMPLEMENT_DYNAMIC(CEditIdentityOptions, CTabPanel)

CEditIdentityOptions::CEditIdentityOptions()
	: CTabPanel(CEditIdentityOptions::IDD)
{
	//{{AFX_DATA_INIT(CEditIdentityOptions)
	mCopyToActive = FALSE;
	mCopyToOption = -1;
	mCopyTo = _T("");
	mHeaderActive = FALSE;
	mFooterActive = FALSE;
	//}}AFX_DATA_INIT
}


void CEditIdentityOptions::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditIdentityOptions)
	DDX_Control(pDX, IDC_IDENTITY_COPYTO, mCopyToCtrl);
	DDX_Check(pDX, IDC_IDENTITY_COPYTO_ACTIVE, mCopyToActive);
	DDX_Radio(pDX, IDC_IDENTITY_COPYTO1, mCopyToOption);
	DDX_UTF8Text(pDX, IDC_IDENTITY_COPYTO, mCopyTo);
	DDX_Check(pDX, IDC_IDENTITY_ALSO_COPYTO, mAlsoCopyTo);
	DDX_Check(pDX, IDC_IDENTITY_HEADER_ACTIVE, mHeaderActive);
	DDX_Check(pDX, IDC_IDENTITY_SIGNATURE_ACTIVE, mFooterActive);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditIdentityOptions, CTabPanel)
	//{{AFX_MSG_MAP(CEditIdentityOptions)
	ON_COMMAND_RANGE(IDM_AppendToPopupNone, IDM_AppendToPopupEnd, OnCopyToPopup)
	ON_BN_CLICKED(IDC_IDENTITY_COPYTO_ACTIVE, OnActive)
	ON_BN_CLICKED(IDC_IDENTITY_COPYTO1, OnCopyToEnable)
	ON_BN_CLICKED(IDC_IDENTITY_COPYTO2, OnCopyToDisable1)
	ON_BN_CLICKED(IDC_IDENTITY_COPYTO3, OnCopyToDisable2)
	ON_BN_CLICKED(IDC_IDENTITY_HEADER_ACTIVE, OnHeaderActive)
	ON_BN_CLICKED(IDC_IDENTITY_HEADERBTN, OnHeaderBtn)
	ON_BN_CLICKED(IDC_IDENTITY_SIGNATURE_ACTIVE, OnSignatureActive)
	ON_BN_CLICKED(IDC_IDENTITY_SIGNATUREBTN, OnSignatureBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditIdentityOptions message handlers

// Set data
BOOL CEditIdentityOptions::OnInitDialog()
{
	// Do inherited
	CTabPanel::OnInitDialog();

	// Subclass buttons
	mHeaderBtn.SubclassDlgItem(IDC_IDENTITY_HEADERBTN, this, IDI_XHEADERLINES);
	mFooterBtn.SubclassDlgItem(IDC_IDENTITY_SIGNATUREBTN, this, IDI_SIGNATURE);
	mCopyToPopup.SubclassDlgItem(IDC_IDENTITY_COPYTO_POPUP, this, IDI_POPUPBTN);
	mCopyToPopup.SetCopyTo(false);
	mCopyToPopup.SetDefault();
	mCopyToPopup.EnableWindow(CMulberryApp::sApp->LoadedPrefs());

	SetActive(mCopyToActive);
	SetHeaderActive(mHeaderActive);
	SetSignatureActive(mFooterActive);

	return TRUE;
}

// Set data
void CEditIdentityOptions::SetContent(void* data)
{
	CIdentity* id = (CIdentity*) data;

	// Copy info
	mCopyTo = id->GetCopyTo();
	mCopyToActive = id->UseCopyTo();
	if (id->GetCopyToNone())
		OnCopyToDisable1();
	else if(id->GetCopyToChoose())
		OnCopyToDisable2();
	else
		OnCopyToEnable();
	mAlsoCopyTo = id->GetCopyReplied();

	mHeader = id->GetHeader();
	mHeaderActive = id->UseHeader() && CAdminLock::sAdminLock.mAllowXHeaders;

	mFooter = id->GetSignatureRaw();
	mFooterActive = id->UseSignature();

	UpdateData(false);
	SetActive(mCopyToActive);
	SetHeaderActive(mHeaderActive);
	SetSignatureActive(mFooterActive);
}

// Force update of data
bool CEditIdentityOptions::UpdateContent(void* data)
{
	CIdentity* id = (CIdentity*) data;

	id->SetCopyTo(mCopyTo, mCopyToActive);
	id->SetCopyToNone(mCopyToOption == 1);
	id->SetCopyToChoose(mCopyToOption == 2);
	id->SetCopyReplied(mAlsoCopyTo);

	if (!CAdminLock::sAdminLock.mAllowXHeaders)
		mHeader = cdstring::null_str;
	id->SetHeader(mHeader, mHeaderActive && CAdminLock::sAdminLock.mAllowXHeaders);

	id->SetSignature(mFooter, mFooterActive);
	
	return true;
}

// Change to single
void CEditIdentityOptions::OnActive(void)
{
	mCopyToActive = !mCopyToActive;
	SetActive(mCopyToActive);
}

// Change to single
void CEditIdentityOptions::OnHeaderActive(void)
{
	mHeaderActive = !mHeaderActive;
	SetHeaderActive(mHeaderActive);
}

// Change to single
void CEditIdentityOptions::OnSignatureActive(void)
{
	mFooterActive = !mFooterActive;
	SetSignatureActive(mFooterActive);
}

// Change control states
void CEditIdentityOptions::OnCopyToEnable(void)
{
	GetDlgItem(IDC_IDENTITY_COPYTO)->EnableWindow(true);
	GetDlgItem(IDC_IDENTITY_COPYTO_POPUP)->EnableWindow(CMulberryApp::sApp->LoadedPrefs());
	mCopyToOption = 0;
}

// Change control states
void CEditIdentityOptions::OnCopyToDisable1(void)
{
	GetDlgItem(IDC_IDENTITY_COPYTO)->EnableWindow(false);
	GetDlgItem(IDC_IDENTITY_COPYTO_POPUP)->EnableWindow(false);
	mCopyToOption = 1;
}

void CEditIdentityOptions::OnCopyToDisable2(void)
{
	GetDlgItem(IDC_IDENTITY_COPYTO)->EnableWindow(false);
	GetDlgItem(IDC_IDENTITY_COPYTO_POPUP)->EnableWindow(false);
	mCopyToOption = 2;
}

void CEditIdentityOptions::OnCopyToPopup(UINT nID) 
{
	if (nID == IDM_AppendToPopupNone)
	{
		static_cast<CButton*>(GetDlgItem(IDC_IDENTITY_COPYTO1))->SetCheck(0);
		static_cast<CButton*>(GetDlgItem(IDC_IDENTITY_COPYTO3))->SetCheck(0);
		static_cast<CButton*>(GetDlgItem(IDC_IDENTITY_COPYTO2))->SetCheck(1);
		OnCopyToDisable1();
	}
	else if (nID == IDM_AppendToPopupChoose)
	{
		static_cast<CButton*>(GetDlgItem(IDC_IDENTITY_COPYTO1))->SetCheck(0);
		static_cast<CButton*>(GetDlgItem(IDC_IDENTITY_COPYTO2))->SetCheck(0);
		static_cast<CButton*>(GetDlgItem(IDC_IDENTITY_COPYTO3))->SetCheck(1);
		OnCopyToDisable2();
	}
	else
	{
		// Must set/reset control value to ensure selected mailbox is returned but
		// popup does not display check mark
		mCopyToPopup.SetValue(nID);
		cdstring mbox_name;
		if (mCopyToPopup.GetSelectedMboxName(mbox_name, true))
			CUnicodeUtils::SetWindowTextUTF8(&mCopyToCtrl, mbox_name);
		mCopyToPopup.SetValue(-1);
	}
}

void CEditIdentityOptions::OnHeaderBtn() 
{
	CPrefsEditHeadFoot::PoseDialog(CPreferences::sPrefs, mHeader, false);
}

void CEditIdentityOptions::OnSignatureBtn() 
{
	CPrefsEditHeadFoot::PoseDialog(CPreferences::sPrefs, mFooter, true);
}

void CEditIdentityOptions::SetActive(bool active)
{
	if (active)
	{
		GetDlgItem(IDC_IDENTITY_COPYTO1)->EnableWindow(true);
		GetDlgItem(IDC_IDENTITY_COPYTO2)->EnableWindow(true);
		GetDlgItem(IDC_IDENTITY_COPYTO3)->EnableWindow(true);
		switch(mCopyToOption)
		{
		case 0:
		default:
			OnCopyToEnable();
			break;
		case 1:
			OnCopyToDisable1();
			break;
		case 2:
			OnCopyToDisable2();
			break;
		}
		GetDlgItem(IDC_IDENTITY_ALSO_COPYTO)->EnableWindow(true);
	}
	else
	{
		GetDlgItem(IDC_IDENTITY_COPYTO1)->EnableWindow(false);
		GetDlgItem(IDC_IDENTITY_COPYTO2)->EnableWindow(false);
		GetDlgItem(IDC_IDENTITY_COPYTO3)->EnableWindow(false);
		mCopyToCtrl.EnableWindow(false);
		mCopyToPopup.EnableWindow(false);
		GetDlgItem(IDC_IDENTITY_ALSO_COPYTO)->EnableWindow(false);
		UpdateData(true);
	}
}

void CEditIdentityOptions::SetHeaderActive(bool active)
{
	GetDlgItem(IDC_IDENTITY_HEADERBTN)->EnableWindow(CAdminLock::sAdminLock.mAllowXHeaders && active);
}

void CEditIdentityOptions::SetSignatureActive(bool active)
{
	GetDlgItem(IDC_IDENTITY_SIGNATUREBTN)->EnableWindow(active);
}