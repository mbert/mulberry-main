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


// CEditAddressDialog.cpp : implementation file
//


#include "CEditAddressDialog.h"

#include "CAdbkAddress.h"
#include "CSDIFrame.h"
#include "CStringUtils.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CEditAddressDialog dialog


CEditAddressDialog::CEditAddressDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CEditAddressDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditAddressDialog)
	//}}AFX_DATA_INIT
}


void CEditAddressDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditAddressDialog)
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_NICKNAME, mNickName);
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_NAME, mFullName);
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_EMAIL, mEmail);
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_COMPANY, mCompany);
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_PHONEWORK, mPhoneWork);
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_PHONEHOME, mPhoneHome);
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_FAX, mFax);
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_ADDRESS, mAddress);
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_URL, mURL);
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_NOTES, mNotes);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditAddressDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CEditAddressDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CEditAddressDialog::OnInitDialog() 
{
	CHelpDialog::OnInitDialog();

	// Disable items if required
	if (!mAllowEdit)
	{
		static_cast<CEdit*>(GetDlgItem(IDC_ADDRESSEDIT_NICKNAME))->SetReadOnly();
		static_cast<CEdit*>(GetDlgItem(IDC_ADDRESSEDIT_NAME))->SetReadOnly();
		static_cast<CEdit*>(GetDlgItem(IDC_ADDRESSEDIT_EMAIL))->SetReadOnly();
		static_cast<CEdit*>(GetDlgItem(IDC_ADDRESSEDIT_COMPANY))->SetReadOnly();
		static_cast<CEdit*>(GetDlgItem(IDC_ADDRESSEDIT_PHONEWORK))->SetReadOnly();
		static_cast<CEdit*>(GetDlgItem(IDC_ADDRESSEDIT_PHONEHOME))->SetReadOnly();
		static_cast<CEdit*>(GetDlgItem(IDC_ADDRESSEDIT_FAX))->SetReadOnly();
		static_cast<CEdit*>(GetDlgItem(IDC_ADDRESSEDIT_ADDRESS))->SetReadOnly();
		static_cast<CEdit*>(GetDlgItem(IDC_ADDRESSEDIT_URL))->SetReadOnly();
		static_cast<CEdit*>(GetDlgItem(IDC_ADDRESSEDIT_NOTES))->SetReadOnly();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
// CEditAddressDialog message handlers

void CEditAddressDialog::SetFields(CAdbkAddress* addr, bool allow_edit)
{
	mNickName = addr->GetADL();
	mFullName = addr->GetName();
	mEmail = addr->GetMailAddress();
	mCompany = addr->GetCompany();
	mPhoneWork = addr->GetPhone(CAdbkAddress::eWorkPhoneType);
	mPhoneHome = addr->GetPhone(CAdbkAddress::eHomePhoneType);
	mFax = addr->GetPhone(CAdbkAddress::eFaxType);
	mAddress = addr->GetAddress(CAdbkAddress::eDefaultAddressType);
	mURL = addr->GetURL();
	mNotes = addr->GetNotes();
	
	mAllowEdit = allow_edit;
}

bool CEditAddressDialog::GetFields(CAdbkAddress* addr)
{
	// Nick-name cannot contain '@' and no spaces surrounding it
	cdstring nickname(mNickName);
	::strreplace(nickname.c_str_mod(), "@", '*');
	nickname.trimspace();

	addr->SetADL(nickname);
	addr->SetName(mFullName);
	addr->CopyMailAddress(mEmail);
	addr->SetCompany(mCompany);
	addr->SetPhone(mPhoneWork, CAdbkAddress::eWorkPhoneType);
	addr->SetPhone(mPhoneHome, CAdbkAddress::eHomePhoneType);
	addr->SetPhone(mFax, CAdbkAddress::eFaxType);
	addr->SetAddress(mAddress, CAdbkAddress::eDefaultAddressType);
	addr->SetURL(mURL);
	addr->SetNotes(mNotes);
	
	return true;
}

bool CEditAddressDialog::PoseDialog(CAdbkAddress* addr, bool allow_edit)
{
	bool result = false;

	// Create the dialog
	CEditAddressDialog	dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetFields(addr, allow_edit);

	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{
		// Get strings and update address if changed
		dlog.GetFields(addr);
		result = true;
	}
	
	return result;
}
