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


// Source for CAddressContactPanel class

#include "CAddressContactPanel.h"

#include "CAdbkAddress.h"
#include "CUnicodeUtils.h"

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CAddressContactPanel::CAddressContactPanel()
	: CAddressPanelBase(CAddressContactPanel::IDD)
{
	//{{AFX_DATA_INIT(CAddressContactPanel)
	//}}AFX_DATA_INIT
}

void CAddressContactPanel::DoDataExchange(CDataExchange* pDX)
{
	CAddressPanelBase::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddressContactPanel)
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_PHONEWORK, mPhoneWork);
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_PHONEHOME, mPhoneHome);
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_FAX, mFax);
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_ADDRESS, mAddress);
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_URL, mURL);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddressContactPanel, CAddressPanelBase)
	//{{AFX_MSG_MAP(CAddressContactPanel)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// O T H E R  M E T H O D S ____________________________________________________________________________

// Set data
BOOL CAddressContactPanel::OnInitDialog()
{
	// Do inherited
	CAddressPanelBase::OnInitDialog();

	// Add alignments
	AddAlignment(new CWndAlignment(GetDlgItem(IDC_ADDRESSEDIT_ADDRESS), CWndAlignment::eAlign_TopWidth));
	AddAlignment(new CWndAlignment(GetDlgItem(IDC_ADDRESSEDIT_PHONEWORK), CWndAlignment::eAlign_TopWidth));
	AddAlignment(new CWndAlignment(GetDlgItem(IDC_ADDRESSEDIT_PHONEHOME), CWndAlignment::eAlign_TopWidth));
	AddAlignment(new CWndAlignment(GetDlgItem(IDC_ADDRESSEDIT_FAX), CWndAlignment::eAlign_TopWidth));
	AddAlignment(new CWndAlignment(GetDlgItem(IDC_ADDRESSEDIT_URL), CWndAlignment::eAlign_TopWidth));

	return TRUE;
}

void CAddressContactPanel::Focus()
{
	GetDlgItem(IDC_ADDRESSEDIT_ADDRESS)->SetFocus();
}

// Set fields in dialog
void CAddressContactPanel::SetFields(const CAdbkAddress* addr)
{
	mPhoneWork = (addr ? addr->GetPhone(CAdbkAddress::eWorkPhoneType) : cdstring::null_str);
	mPhoneHome = (addr ? addr->GetPhone(CAdbkAddress::eHomePhoneType) : cdstring::null_str);
	mFax = (addr ? addr->GetPhone(CAdbkAddress::eFaxType) : cdstring::null_str);
	mAddress = (addr ? addr->GetAddress(CAdbkAddress::eDefaultAddressType) : cdstring::null_str);
	mURL = (addr ? addr->GetURL() : cdstring::null_str);
}

// Get fields from dialog
bool CAddressContactPanel::GetFields(CAdbkAddress* addr)
{
	// Only for valid group
	if (!addr)
		return false;

	// Now force control update
	UpdateData(true);

	bool done_edit = false;

	if (addr->GetPhone(CAdbkAddress::eWorkPhoneType) != mPhoneWork)
	{
		addr->SetPhone(mPhoneWork, CAdbkAddress::eWorkPhoneType);
		done_edit = true;
	}
	if (addr->GetPhone(CAdbkAddress::eHomePhoneType) != mPhoneHome)
	{
		addr->SetPhone(mPhoneHome, CAdbkAddress::eHomePhoneType);
		done_edit = true;
	}
	if (addr->GetPhone(CAdbkAddress::eFaxType) != mFax)
	{
		addr->SetPhone(mFax, CAdbkAddress::eFaxType);
		done_edit = true;
	}
	if (addr->GetAddress(CAdbkAddress::eDefaultAddressType) != mAddress)
	{
		addr->SetAddress(mAddress, CAdbkAddress::eDefaultAddressType);
		done_edit = true;
	}
	if (addr->GetURL() != mURL)
	{
		addr->SetURL(mURL);
		done_edit = true;
	}
	
	return done_edit;
}
