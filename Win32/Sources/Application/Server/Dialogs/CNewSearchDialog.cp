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


// CNewSearchDialog.cpp : implementation file
//

#include "CNewSearchDialog.h"

#include "CINETCommon.h"
#include "CMailAccount.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CNewSearchDialog dialog


CNewSearchDialog::CNewSearchDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CNewSearchDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewSearchDialog)
	mFullHierarchy = 1;
	mText = _T("");
	//}}AFX_DATA_INIT
	mStartIndex = 0;
}


void CNewSearchDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewSearchDialog)
	DDX_UTF8Text(pDX, IDC_NEWSEARCH_TEXT, mText);
	DDX_Control(pDX, IDC_NEWSEARCH_TEXT, mTextCtrl);
	DDX_Radio(pDX, IDC_NEWSEARCH_FULL, mFullHierarchy);
	DDX_Control(pDX, IDC_NEWSEARCH_FULL, mFullHierarchyCtrl);
	DDX_Control(pDX, IDC_NEWSEARCH_TOP, mTopHierarchyCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewSearchDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CNewSearchDialog)
	ON_COMMAND_RANGE(IDM_AccountStart, IDM_AccountStop, OnNewSearchAccount)
	ON_COMMAND_RANGE(IDM_MAILBOX_SEARCH_STARTS, IDM_MAILBOX_SEARCH_HIER, OnNewSearchCriteria)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewSearchDialog message handlers

BOOL CNewSearchDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass buttons
	mAccountPopup.SubclassDlgItem(IDC_NEWSEARCH_ACCOUNT, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mAccountPopup.SetMenu(IDR_POPUP_MAILBOX_SEARCH_ACCOUNTS);
	InitAccountMenu();
	if (mSingle)
	{
		// Disable all menu items except for relevant one
		CMenu* menu = mAccountPopup.GetPopupMenu();
		for(short i = 0; i < menu->GetMenuItemCount(); i++)
		{
			if (i + 1 != mStartIndex)
				menu->EnableMenuItem(i + IDM_AccountStart, MF_GRAYED | MF_BYCOMMAND);
		}
	}
	
	if (mStartIndex >= 1)
		mAccountPopup.SetValue(IDM_AccountStart + mStartIndex - 1);
	else
		mAccountPopup.SetValue(IDM_AccountStart);

	mCriteriaPopup.SubclassDlgItem(IDC_NEWSEARCH_CRITERIA, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mCriteriaPopup.SetMenu(IDR_POPUP_MAILBOX_SEARCH_CRITERIA);
	mCriteriaPopup.SetValue(IDM_MAILBOX_SEARCH_SUB);

	return true;
}


void CNewSearchDialog::SetDetails(bool single, long index)
{
	mSingle = single;
	mStartIndex = index;
}

// Called during idle
void CNewSearchDialog::GetDetails(cdstring& change, unsigned long& index)
{
	// Get account index
	index = mAccountPopup.GetValue() - IDM_AccountStart + 1;

	// Get criteria and name
	short criteria = mCriteriaPopup.GetValue();
	cdstring copyStr = mText;
	bool full = !mFullHierarchy;

	// Set leading criteria value
	switch(criteria)
	{
	case IDM_MAILBOX_SEARCH_ENDS:
	case IDM_MAILBOX_SEARCH_CONTAINS:
		change += cWILDCARD;
		break;
	case IDM_MAILBOX_SEARCH_STARTS:
	case IDM_MAILBOX_SEARCH_IS:
	case IDM_MAILBOX_SEARCH_SUB:
	case IDM_MAILBOX_SEARCH_HIER:
		break;
	}

	// Only if not full
	if (criteria != IDM_MAILBOX_SEARCH_HIER)
		change += copyStr;

	// Set trailing criteria value
	switch(criteria)
	{
	case IDM_MAILBOX_SEARCH_STARTS:
	case IDM_MAILBOX_SEARCH_CONTAINS:
		change += cWILDCARD;
		break;
	case IDM_MAILBOX_SEARCH_ENDS:
	case IDM_MAILBOX_SEARCH_IS:
		break;
	case IDM_MAILBOX_SEARCH_SUB:
		{
			char dir_delim = CPreferences::sPrefs->mMailAccounts.GetValue().at(index - 1)->GetDirDelim();
			// Append dir delim if not already
			if (dir_delim && change.length() && (change.c_str()[change.length() - 1] != dir_delim))
				change += dir_delim;
		}
		// Append full wildcard only
		if (full)
			change += cWILDCARD;
		break;

	case IDM_MAILBOX_SEARCH_HIER:
		// Append appropriate wildcard
		if (full)
			change += cWILDCARD;
		else
			change += cWILDCARD_NODIR;
		break;
	}

}

void CNewSearchDialog::OnNewSearchAccount(UINT nID) 
{
	mAccountPopup.SetValue(nID);
}

void CNewSearchDialog::OnNewSearchCriteria(UINT nID) 
{
	switch(nID)
	{
	case IDM_MAILBOX_SEARCH_STARTS:
	case IDM_MAILBOX_SEARCH_ENDS:
	case IDM_MAILBOX_SEARCH_CONTAINS:
	case IDM_MAILBOX_SEARCH_IS:
		mFullHierarchyCtrl.EnableWindow(false);
		mTopHierarchyCtrl.EnableWindow(false);
		mTextCtrl.EnableWindow(true);
		break;
	case IDM_MAILBOX_SEARCH_SUB:
		mFullHierarchyCtrl.EnableWindow(true);
		mTopHierarchyCtrl.EnableWindow(true);
		mTextCtrl.EnableWindow(true);
		break;
	case IDM_MAILBOX_SEARCH_HIER:
		mFullHierarchyCtrl.EnableWindow(true);
		mTopHierarchyCtrl.EnableWindow(true);
		mTextCtrl.EnableWindow(false);
		break;
	}
	
	mCriteriaPopup.SetValue(nID);
}

void CNewSearchDialog::InitAccountMenu(void) 
{
	CMenu* pPopup = mAccountPopup.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);

	// Now add current items
	int menu_id = IDM_AccountStart;
	for(CMailAccountList::const_iterator iter = CPreferences::sPrefs->mMailAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mMailAccounts.GetValue().end(); iter++)
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id++, (*iter)->GetName());
}

bool CNewSearchDialog::PoseDialog(bool single, unsigned long& index, cdstring& wd)
{
	bool result = false;

	CNewSearchDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetDetails(single, index);

	if (dlog.DoModal() == msg_OK)
	{
		dlog.GetDetails(wd, index);
		result = true;
	}

	return result;
}
