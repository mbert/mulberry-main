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


// CNewWildcardDialog.cpp : implementation file
//

#include "CNewWildcardDialog.h"

#include "CINETCommon.h"
#include "CMailAccount.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CNewWildcardDialog dialog


CNewWildcardDialog::CNewWildcardDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CNewWildcardDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewWildcardDialog)
	//}}AFX_DATA_INIT
}


void CNewWildcardDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewWildcardDialog)
	DDX_UTF8Text(pDX, IDC_CABINETENTRY_TEXT, mText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewWildcardDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CNewWildcardDialog)
	ON_COMMAND_RANGE(IDM_MAILBOXES_WILDCARD, IDM_AccountStop, OnNewWildcardAccount)
	ON_COMMAND_RANGE(IDM_MAILBOX_WILDCARD_STARTS, IDM_MAILBOX_WILDCARD_TOP, OnNewWildcardCriteria)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewWildcardDialog message handlers

BOOL CNewWildcardDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass buttons
	mAccountPopup.SubclassDlgItem(IDC_CABINETENTRY_ACCOUNT, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mAccountPopup.SetMenu(IDR_POPUP_MAILBOX_WILDCARD_ACCOUNTS);
	InitAccountMenu();
	mAccountPopup.SetValue(IDM_MAILBOXES_WILDCARD);

	mCriteriaPopup.SubclassDlgItem(IDC_CABINETENTRY_CRITERIA, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mCriteriaPopup.SetMenu(IDR_POPUP_MAILBOX_WILDCARD_CRITERIA);
	mCriteriaPopup.SetValue(IDM_MAILBOX_WILCARD_ANYWHERE);

	return true;
}


// Called during idle
void CNewWildcardDialog::GetDetails(cdstring& change)
{
	// Add account name
	if (mAccountPopup.GetValue() == IDM_MAILBOXES_WILDCARD)
		change += cWILDCARD;
	else
	{
		short index = mAccountPopup.GetValue() - IDM_AccountStart;

		change += CPreferences::sPrefs->mMailAccounts.GetValue().at(index)->GetName();
	}
	change += cMailAccountSeparator;

	// Get criteria and name
	short criteria = mCriteriaPopup.GetValue();
	cdstring copyStr = mText;

	// Set leading criteria value
	switch(criteria)
	{
	case IDM_MAILBOX_WILDCARD_ENDS:
	case IDM_MAILBOX_WILDCARD_CONTAINS:
		change += cWILDCARD;
		break;
	case IDM_MAILBOX_WILDCARD_STARTS:
	case IDM_MAILBOX_WILDCARD_IS:
	case IDM_MAILBOX_WILCARD_ANYWHERE:
	case IDM_MAILBOX_WILDCARD_TOP:
		break;
	}

	change += copyStr;

	// Set trailing criteria value
	switch(criteria)
	{
	case IDM_MAILBOX_WILDCARD_STARTS:
	case IDM_MAILBOX_WILDCARD_CONTAINS:
		change += cWILDCARD;
		break;
	case IDM_MAILBOX_WILDCARD_ENDS:
	case IDM_MAILBOX_WILDCARD_IS:
		break;
	case IDM_MAILBOX_WILCARD_ANYWHERE:
	case IDM_MAILBOX_WILDCARD_TOP:
		{
			short acct_num = mAccountPopup.GetValue() - IDM_AccountStart;
			if (acct_num < 0)
				acct_num = 0;
			char dir_delim = CPreferences::sPrefs->mMailAccounts.GetValue().at(acct_num)->GetDirDelim();

			// Append dir delim if not already
			if (dir_delim && change.length() && (change.c_str()[change.length() - 1UL] != dir_delim) && !copyStr.empty())
				change += dir_delim;

			// Append appropriate wildcard
			if (criteria == IDM_MAILBOX_WILCARD_ANYWHERE)
				change += cWILDCARD;
			else
				change += cWILDCARD_NODIR;
		}
		break;
	}


}

void CNewWildcardDialog::OnNewWildcardAccount(UINT nID) 
{
	mAccountPopup.SetValue(nID);
}

void CNewWildcardDialog::OnNewWildcardCriteria(UINT nID) 
{
	mCriteriaPopup.SetValue(nID);
}

void CNewWildcardDialog::InitAccountMenu(void) 
{
	CMenu* pPopup = mAccountPopup.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = 2; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);

	// Now add current items
	int menu_id = IDM_AccountStart;
	for(CMailAccountList::const_iterator iter = CPreferences::sPrefs->mMailAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mMailAccounts.GetValue().end(); iter++)
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id++, (*iter)->GetName());
}

bool CNewWildcardDialog::PoseDialog(cdstring& name)
{
	bool result = false;

	CNewWildcardDialog dlog(CSDIFrame::GetAppTopWindow());
	if (dlog.DoModal() == msg_OK)
	{
		dlog.GetDetails(name);
		result = true;
	}

	return result;
}
