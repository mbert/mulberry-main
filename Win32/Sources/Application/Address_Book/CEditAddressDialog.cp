/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_CALENDAR, mCalendar);
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
		static_cast<CEdit*>(GetDlgItem(IDC_ADDRESSEDIT_CALENDAR))->SetReadOnly();
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
	mEmail = addr->GetEmail(CAdbkAddress::eDefaultEmailType);
	mCalendar = addr->GetCalendar();
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
	bool done_edit = false;

	// Nick-name cannot contain '@' and no spaces surrounding it
	cdstring txt = mNickName;
	::strreplace(txt.c_str_mod(), "@", '*');
	txt.trimspace();

	if (addr->GetADL() != txt)
	{
		addr->SetADL(txt);
		done_edit = true;
	}

	txt = mFullName;
	if (addr->GetName() != txt)
	{
		addr->SetName(txt);
		done_edit = true;
	}

	txt = mEmail;
	cdstring test_addr = addr->GetEmail(CAdbkAddress::eDefaultEmailType);
	if (test_addr != txt)
	{
		addr->SetEmail(txt, CAdbkAddress::eDefaultEmailType);
		done_edit = true;
	}

	txt = mCalendar;
	if (addr->GetCalendar() != txt)
	{
		addr->SetCalendar(txt);
		done_edit = true;
	}

	txt = mCompany;
	if (addr->GetCompany() != txt)
	{
		addr->SetCompany(txt);
		done_edit = true;
	}

	txt = mAddress;

	// Give to address if not same as previous
	if (addr->GetAddress(CAdbkAddress::eDefaultAddressType) != txt)
	{
		addr->SetAddress(txt, CAdbkAddress::eDefaultAddressType);
		done_edit = true;
	}

	txt = mURL;

	// Give to URL if not same as previous
	if (addr->GetURL() != txt)
	{
		addr->SetURL(txt);
		done_edit = true;
	}

	txt = mPhoneWork;
	if (addr->GetPhone(CAdbkAddress::eWorkPhoneType) != txt)
	{
		addr->SetPhone(txt, CAdbkAddress::eWorkPhoneType);
		done_edit = true;
	}

	txt = mPhoneHome;
	if (addr->GetPhone(CAdbkAddress::eHomePhoneType) != txt)
	{
		addr->SetPhone(txt, CAdbkAddress::eHomePhoneType);
		done_edit = true;
	}

	txt = mFax;
	if (addr->GetPhone(CAdbkAddress::eFaxType) != txt)
	{
		addr->SetPhone(txt, CAdbkAddress::eFaxType);
		done_edit = true;
	}

	txt = mNotes;
	if (addr->GetNotes() != txt)
	{
		addr->SetNotes(txt);
		done_edit = true;
	}

	return done_edit;
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
