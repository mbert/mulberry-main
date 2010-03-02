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


// CEditIdentityAddress.cpp : implementation file
//

#include "CEditIdentityAddress.h"

#include "CAddressList.h"
#include "CAdminLock.h"
#include "CCalendarAddress.h"
#include "CPreferences.h"
#include "CUnicodeUtils.h"

#include "cdstring.h"

/////////////////////////////////////////////////////////////////////////////
// CEditIdentityAddress dialog

IMPLEMENT_DYNAMIC(CEditIdentityAddress, CTabPanel)

CEditIdentityAddress::CEditIdentityAddress(bool outgoing)
	: CTabPanel(outgoing ? CEditIdentityAddress::IDD2 : CEditIdentityAddress::IDD1)
{
	//{{AFX_DATA_INIT(CEditIdentityAddress)
	mActive = FALSE;
	mMultiple = 0;
	//}}AFX_DATA_INIT
	
	mSingle = true;
}


void CEditIdentityAddress::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditIdentityAddress)
	DDX_Control(pDX, IDC_IDENTITY_MADDRESS, mTextCtrl);
	DDX_Control(pDX, IDC_IDENTITY_TEMAIL, mEmailTitleCtrl);
	DDX_Control(pDX, IDC_IDENTITY_TNAME, mNameTitleCtrl);
	DDX_Control(pDX, IDC_IDENTITY_SEMAIL, mEmailCtrl);
	DDX_Control(pDX, IDC_IDENTITY_SNAME, mNameCtrl);
	DDX_Check(pDX, IDC_IDENTITY_ACTIVE, mActive);
	DDX_Radio(pDX, IDC_IDENTITY_SINGLE, mMultiple);
	DDX_Control(pDX, IDC_IDENTITY_MULTIPLE, mMultipleBtn);
	DDX_UTF8Text(pDX, IDC_IDENTITY_SNAME, mName);
	DDX_UTF8Text(pDX, IDC_IDENTITY_SEMAIL, mEmail);
	DDX_UTF8Text(pDX, IDC_IDENTITY_MADDRESS, mText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditIdentityAddress, CTabPanel)
	//{{AFX_MSG_MAP(CEditIdentityAddress)
	ON_BN_CLICKED(IDC_IDENTITY_ACTIVE, OnActive)
	ON_BN_CLICKED(IDC_IDENTITY_SINGLE, OnSingleAddress)
	ON_BN_CLICKED(IDC_IDENTITY_MULTIPLE, OnMultipleAddress)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditIdentityAddress message handlers

// Set data
BOOL CEditIdentityAddress::OnInitDialog()
{
	// Do inherited
	CTabPanel::OnInitDialog();

	// Hide items
	if (mSingle)
		mTextCtrl.ShowWindow(SW_HIDE);
	else
	{
		mNameTitleCtrl.ShowWindow(SW_HIDE);
		mEmailTitleCtrl.ShowWindow(SW_HIDE);
		mNameCtrl.ShowWindow(SW_HIDE);
		mEmailCtrl.ShowWindow(SW_HIDE);
	}
	
	SetActive(mActive);

	return TRUE;
}

// Set data
void CEditIdentityAddress::SetContent(void* data)
{
	CIdentity* id = (CIdentity*) data;

	// Copy info
	if (mFrom)
		SetItemData(id->UseFrom(), id->GetFrom());
	else if (mReplyTo)
		SetItemData(id->UseReplyTo(), id->GetReplyTo());
	else if (mSender)
		SetItemData(id->UseSender(), id->GetSender());
	else if (mTo)
		SetItemData(id->UseAddTo(), id->GetAddTo());
	else if (mCC)
		SetItemData(id->UseAddCC(), id->GetAddCC());
	else if (mBcc)
		SetItemData(id->UseAddBcc(), id->GetAddBcc());
	else if (mCalendar)
		SetItemData(id->UseCalendar(), id->GetCalendar());
	
	// Must update the display if changed to multiple
	if (!mSingle)
	{
		mNameTitleCtrl.ShowWindow(SW_HIDE);
		mEmailTitleCtrl.ShowWindow(SW_HIDE);
		mNameCtrl.ShowWindow(SW_HIDE);
		mEmailCtrl.ShowWindow(SW_HIDE);
		mTextCtrl.ShowWindow(SW_SHOW);
	}
}

// Force update of data
bool CEditIdentityAddress::UpdateContent(void* data)
{
	CIdentity* id = (CIdentity*) data;

	bool active;
	cdstring address;
	GetItemData(active, address);

	if (mFrom)
		id->SetFrom(address, active);
	else if (mReplyTo)
		id->SetReplyTo(address, active);
	else if (mSender)
		id->SetSender(address, active);
	else if (mTo)
		id->SetAddTo(address, active);
	else if (mCC)
		id->SetAddCC(address, active);
	else if (mBcc)
		id->SetAddBcc(address, active);
	else if (mCalendar)
		id->SetCalendar(address, active);
	
	return true;
}

// Change to single
void CEditIdentityAddress::OnActive(void)
{
	mActive = !mActive;
	SetActive(mActive);
}

// Change to single
void CEditIdentityAddress::OnSingleAddress(void)
{
	SetSingle(true);
}

// Change to single
void CEditIdentityAddress::OnMultipleAddress(void)
{
	SetSingle(false);
}


void CEditIdentityAddress::SetActive(bool active)
{
	if (active)
	{
		GetDlgItem(IDC_IDENTITY_SINGLE)->EnableWindow(true);
		mMultipleBtn.EnableWindow(!mSender && (!mFrom || !CAdminLock::sAdminLock.mLockReturnAddress));
		mNameCtrl.EnableWindow(true);
		mEmailCtrl.EnableWindow(!mFrom || !CAdminLock::sAdminLock.mLockReturnAddress);
		mTextCtrl.EnableWindow(!mFrom || !CAdminLock::sAdminLock.mLockReturnAddress);
	}
	else
	{
		GetDlgItem(IDC_IDENTITY_SINGLE)->EnableWindow(false);
		mMultipleBtn.EnableWindow(false);
		mNameCtrl.EnableWindow(false);
		mEmailCtrl.EnableWindow(false);
		mTextCtrl.EnableWindow(false);
	}
}

void CEditIdentityAddress::SetSingle(bool single)
{
	if (single && !mSingle)
	{
		cdstring text = CUnicodeUtils::GetWindowTextUTF8(&mTextCtrl);
		
		if (!mCalendar)
		{
			CAddressList addr_list(text, text.length());

			if (addr_list.size())
				CUnicodeUtils::SetWindowTextUTF8(&mNameCtrl, addr_list.front()->GetName());
			else
				CUnicodeUtils::SetWindowTextUTF8(&mNameCtrl, cdstring::null_str);
			if (addr_list.size())
				CUnicodeUtils::SetWindowTextUTF8(&mEmailCtrl, addr_list.front()->GetMailAddress());
			else
				CUnicodeUtils::SetWindowTextUTF8(&mEmailCtrl, cdstring::null_str);
		}
		else
		{
			CCalendarAddress addr(text);

			CUnicodeUtils::SetWindowTextUTF8(&mNameCtrl, addr.GetName());
			CUnicodeUtils::SetWindowTextUTF8(&mEmailCtrl, addr.GetAddress());
		}
		
		mTextCtrl.ShowWindow(SW_HIDE);
		mNameTitleCtrl.ShowWindow(SW_SHOW);
		mEmailTitleCtrl.ShowWindow(SW_SHOW);
		mNameCtrl.ShowWindow(SW_SHOW);
		mEmailCtrl.ShowWindow(SW_SHOW);
		
		mSingle = true;
	}
	else if (!single && mSingle)
	{
		cdstring name = CUnicodeUtils::GetWindowTextUTF8(&mNameCtrl);
		cdstring email = CUnicodeUtils::GetWindowTextUTF8(&mEmailCtrl);

		if (!mCalendar)
		{
			CAddress addr(email, name);
			cdstring address = addr.GetFullAddress();

			CUnicodeUtils::SetWindowTextUTF8(mTextCtrl, address.c_str());
		}
		else
		{
			CCalendarAddress addr(email, name);
			cdstring address = addr.GetFullAddress();

			CUnicodeUtils::SetWindowTextUTF8(mTextCtrl, address.c_str());
		}
		
		mNameTitleCtrl.ShowWindow(SW_HIDE);
		mEmailTitleCtrl.ShowWindow(SW_HIDE);
		mNameCtrl.ShowWindow(SW_HIDE);
		mEmailCtrl.ShowWindow(SW_HIDE);
		mTextCtrl.ShowWindow(SW_SHOW);
		
		mSingle = false;
	}
}

void CEditIdentityAddress::SetItemData(bool active, const cdstring& address)
{
	mActive = active;
	
	if (!mCalendar)
	{
		// Determine address list
		CAddressList addr_list(address.c_str(), address.length());

		// Truncate to single address for sender or locked from
		if ((addr_list.size() > 1) &&
			(mSender || (mFrom && CAdminLock::sAdminLock.mLockReturnAddress)))
			addr_list.erase(addr_list.begin() + 1, addr_list.end());

		if (addr_list.size() > 1)
		{
			// Select single items
			mSingle = false;

			// Set multi text
			for(CAddressList::const_iterator iter = addr_list.begin(); iter != addr_list.end(); iter++)
				mText += (*iter)->GetFullAddress() + "\r\n";
			mMultiple = 1;
		}
		else
		{
			// Select single items
			mSingle = true;

			// Insert text
			if (addr_list.size() == 1)
			{
				mName = addr_list.front()->GetName();
				mEmail = addr_list.front()->GetMailAddress();
			}
			mMultiple = 0;
		}
	}
	else
	{
		// Determine address list
		CCalendarAddressList addrs;
		CCalendarAddress::FromIdentityText(address, addrs);

		if (addrs.size() > 1)
		{
			// Select single items
			mSingle = false;

			// Set multi text
			for(CCalendarAddressList::const_iterator iter = addrs.begin(); iter != addrs.end(); iter++)
				mText += (*iter)->GetFullAddress() + "\r\n";
			mMultiple = 1;
		}
		else
		{
			// Select single items
			mSingle = true;

			// Insert text
			if (addrs.size() == 1)
			{
				mName = addrs.front()->GetName();
				mEmail = addrs.front()->GetCalendarAddress();
			}
			mMultiple = 0;
		}
	}

	SetActive(mActive);
}

void CEditIdentityAddress::GetItemData(bool& active, cdstring& address)
{
	active = mActive;
	
	bool single = (mMultiple == 0);
	
	if (single)
	{
		if (!mCalendar)
		{
			CAddress addr(mEmail, mName);
			address = addr.GetFullAddress();
		}
		else
		{
			CCalendarAddress addr(mEmail, mName);
			address = addr.GetFullAddress();
		}
	}
	else
		// Copy handle to text with null terminator
		address = mText;
}
