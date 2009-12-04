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


// Source for CAddressPersonalPanel class

#include "CAddressPersonalPanel.h"

#include "CAdbkAddress.h"
#include "CStringUtils.h"
#include "CUnicodeUtils.h"

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CAddressPersonalPanel::CAddressPersonalPanel()
	: CAddressPanelBase(CAddressPersonalPanel::IDD)
{
	//{{AFX_DATA_INIT(CAddressPersonalPanel)
	//}}AFX_DATA_INIT
}

void CAddressPersonalPanel::DoDataExchange(CDataExchange* pDX)
{
	CAddressPanelBase::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddressPersonalPanel)
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_NAME, mFullName);
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_NICKNAME, mNickName);
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_EMAIL, mEmail);
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_COMPANY, mCompany);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAddressPersonalPanel, CAddressPanelBase)
	//{{AFX_MSG_MAP(CAddressPersonalPanel)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// O T H E R  M E T H O D S ____________________________________________________________________________

// Set data
BOOL CAddressPersonalPanel::OnInitDialog()
{
	// Do inherited
	CAddressPanelBase::OnInitDialog();

	// Add alignments
	AddAlignment(new CWndAlignment(GetDlgItem(IDC_ADDRESSEDIT_NAME), CWndAlignment::eAlign_TopWidth));
	AddAlignment(new CWndAlignment(GetDlgItem(IDC_ADDRESSEDIT_NICKNAME), CWndAlignment::eAlign_TopWidth));
	AddAlignment(new CWndAlignment(GetDlgItem(IDC_ADDRESSEDIT_EMAIL), CWndAlignment::eAlign_TopWidth));
	AddAlignment(new CWndAlignment(GetDlgItem(IDC_ADDRESSEDIT_COMPANY), CWndAlignment::eAlign_TopWidth));

	return TRUE;
}

void CAddressPersonalPanel::Focus()
{
	GetDlgItem(IDC_ADDRESSEDIT_NAME)->SetFocus();
}

// Set fields in dialog
void CAddressPersonalPanel::SetFields(const CAdbkAddress* addr)
{
	mFullName = (addr ? addr->GetName() : cdstring::null_str);
	mNickName = (addr ? addr->GetADL() : cdstring::null_str);
	mEmail = (addr ? addr->GetMailAddress() : cdstring::null_str);
	mCompany = (addr ? addr->GetCompany() : cdstring::null_str);
}

// Get fields from dialog
bool CAddressPersonalPanel::GetFields(CAdbkAddress* addr)
{
	// Only for valid group
	if (!addr)
		return false;

	// Now force control update
	UpdateData(true);

	bool done_edit = false;

	if (addr->GetName() != mFullName)
	{
		addr->SetName(mFullName);
		done_edit = true;
	}

	// Nick-name cannot contain '@' and no spaces surrounding it
	cdstring nickname(mNickName);
	::strreplace(nickname.c_str_mod(), "@", '*');
	nickname.trimspace();

	if (addr->GetADL() != nickname)
	{
		addr->SetADL(nickname);
		done_edit = true;
	}

	if (addr->GetMailAddress() != mEmail)
	{
		addr->CopyMailAddress(mEmail);
		done_edit = true;
	}

	if (addr->GetCompany() != mCompany)
	{
		addr->SetCompany(mCompany);
		done_edit = true;
	}

	return done_edit;
}
