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


// Source for CAddressNotesPanel class

#include "CAddressNotesPanel.h"

#include "CAdbkAddress.h"
#include "CUnicodeUtils.h"

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CAddressNotesPanel::CAddressNotesPanel()
	: CAddressPanelBase(CAddressNotesPanel::IDD)
{
	//{{AFX_DATA_INIT(CEditAddressDialog)
	//}}AFX_DATA_INIT
}

void CAddressNotesPanel::DoDataExchange(CDataExchange* pDX)
{
	CAddressPanelBase::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddressNotesPanel)
	DDX_UTF8Text(pDX, IDC_ADDRESSEDIT_NOTES, mNotes);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddressNotesPanel, CAddressPanelBase)
	//{{AFX_MSG_MAP(CAddressNotesPanel)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// O T H E R  M E T H O D S ____________________________________________________________________________

// Set data
BOOL CAddressNotesPanel::OnInitDialog()
{
	// Do inherited
	CAddressPanelBase::OnInitDialog();

	// Add alignments
	AddAlignment(new CWndAlignment(GetDlgItem(IDC_ADDRESSEDIT_NOTES), CWndAlignment::eAlign_WidthHeight));

	return TRUE;
}

void CAddressNotesPanel::Focus()
{
	GetDlgItem(IDC_ADDRESSEDIT_NOTES)->SetFocus();
}

// Set fields in dialog
void CAddressNotesPanel::SetFields(const CAdbkAddress* addr)
{
	mNotes = (addr ? addr->GetNotes() : cdstring::null_str);
}

// Get fields from dialog
bool CAddressNotesPanel::GetFields(CAdbkAddress* addr)
{
	// Only for valid group
	if (!addr)
		return false;

	// Now force control update
	UpdateData(true);

	bool done_edit = false;

	if (addr->GetNotes() != mNotes)
	{
		addr->SetNotes(mNotes);
		done_edit = true;
	}

	return done_edit;
}
