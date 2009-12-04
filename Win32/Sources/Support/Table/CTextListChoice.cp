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


// CTextListChoice.cpp : implementation file
//


#include "CTextListChoice.h"

#include "CSDIFrame.h"
#include "CUnicodeUtils.h"
#include "CXStringResources.h"

/////////////////////////////////////////////////////////////////////////////
// CTextListChoice dialog


CTextListChoice::CTextListChoice(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CTextListChoice::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTextListChoice)
	//}}AFX_DATA_INIT
	
	mUseEntry = false;
	mSingleSelection = false;
	mNoSelection = false;
	mSelectFirst = false;
	mTextList = NULL;
}


void CTextListChoice::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTextListChoice)
	DDX_Control(pDX, IDOK, mActionButton);
	DDX_Control(pDX, IDC_TEXTLISTENTERDESCRIPTION, mTextListEnterDescriptionCtrl);
	DDX_Control(pDX, IDC_TEXTLISTENTER, mTextListEnterCtrl);
	DDX_Control(pDX, IDC_TEXTLISTNOSEL, mTextListNoSel);
	DDX_Control(pDX, IDC_TEXTLISTSINGLESEL, mTextListSingleSel);
	DDX_Control(pDX, IDC_TEXTLISTMULTISEL, mTextListMultiSel);

	if (!mButtonTitle.empty())
		DDX_UTF8Text(pDX, IDOK, mButtonTitle);
	DDX_UTF8Text(pDX, IDC_TEXTLISTDESCRIPTION, mTextListDescription);
	DDX_UTF8Text(pDX, IDC_TEXTLISTENTER, mTextListEnter);
	DDX_UTF8Text(pDX, IDC_TEXTLISTENTERDESCRIPTION, mTextListEnterDescription);
	//}}AFX_DATA_MAP
	
	// Set selection if exiting
	if (pDX->m_bSaveAndValidate && !mNoSelection)
	{
		if (mSingleSelection)
			mSelection.push_back(mTextList->GetCurSel());
		else
		{
			int cnt = mTextList->GetSelCount();
			int* selected = new int[cnt];
			mTextList->GetSelItems(cnt, selected);
			for(int i = 0; i < cnt; i++)
				mSelection.push_back(selected[i]);
			delete[] selected;
		}
	}
}


BEGIN_MESSAGE_MAP(CTextListChoice, CHelpDialog)
	//{{AFX_MSG_MAP(CTextListChoice)
	ON_LBN_SELCHANGE(IDC_TEXTLISTNOSEL, OnSelchangeList)
	ON_LBN_DBLCLK(IDC_TEXTLISTNOSEL, OnDblclickList)
	ON_LBN_SELCHANGE(IDC_TEXTLISTSINGLESEL, OnSelchangeList)
	ON_LBN_DBLCLK(IDC_TEXTLISTSINGLESEL, OnDblclickList)
	ON_LBN_SELCHANGE(IDC_TEXTLISTMULTISEL, OnSelchangeList)
	ON_LBN_DBLCLK(IDC_TEXTLISTMULTISEL, OnDblclickList)
	ON_EN_CHANGE(IDC_TEXTLISTENTER, OnChangeEntry)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextListChoice message handlers

// Called during startup
BOOL CTextListChoice::OnInitDialog(void)
{
	CHelpDialog::OnInitDialog();

	// Change window title
	CUnicodeUtils::SetWindowTextUTF8(this, mTextListDescription);

	// Hide entry bits
	if (!mUseEntry)
	{
		mTextListEnterCtrl.ShowWindow(SW_HIDE);
		mTextListEnterDescriptionCtrl.ShowWindow(SW_HIDE);

		// Shorten dialog
		CRect rect;
		GetWindowRect(rect);
		SetWindowPos(nil, 0, 0, rect.Width(), rect.Height() - 10, SWP_NOZORDER | SWP_NOMOVE);
	}
	else if (mTextListEnter.empty())
		mActionButton.EnableWindow(false);

	// Deletrmine list selection type
	if (mNoSelection)
	{
		mTextListSingleSel.ShowWindow(SW_HIDE);
		mTextListSingleSel.EnableWindow(false);
		mTextListMultiSel.ShowWindow(SW_HIDE);
		mTextListMultiSel.EnableWindow(false);
		mTextList = &mTextListNoSel;
	}
	else if (mSingleSelection)
	{
		mTextListNoSel.ShowWindow(SW_HIDE);
		mTextListNoSel.EnableWindow(false);
		mTextListMultiSel.ShowWindow(SW_HIDE);
		mTextListMultiSel.EnableWindow(false);
		mTextList = &mTextListSingleSel;
	}
	else
	{
		mTextListNoSel.ShowWindow(SW_HIDE);
		mTextListNoSel.EnableWindow(false);
		mTextListSingleSel.ShowWindow(SW_HIDE);
		mTextListSingleSel.EnableWindow(false);
		mTextList = &mTextListMultiSel;
	}

	// Add items to list
	for(cdstrvect::const_iterator iter = mItems.begin(); iter != mItems.end(); iter++)
		mTextList->AddString((*iter).win_str());

	// Select first item
	if (mSelectFirst && !mNoSelection)
	{
		if (mSingleSelection)
			mTextList->SetCurSel(0);
		else
			mTextList->SetSel(0);
	}

	// Set the focus
	if (mUseEntry)
		GotoDlgCtrl(&mTextListEnterCtrl);
	else
		GotoDlgCtrl(mTextList);

	return true;
}

void CTextListChoice::OnSelchangeList()
{
	BOOL enabled = false;

	// Disable buttons when no selection
	if (mUseEntry)
	{
		// Change the entry
		if (mTextList->GetCurSel() != LB_ERR)
		{
			CString entry;
			mTextList->GetText(mTextList->GetCurSel(), entry);
			GetDlgItem(IDC_TEXTLISTENTER)->SetWindowText(entry);			
		}
	}
	else
	{
		if (mSingleSelection && (mTextList->GetCurSel() != LB_ERR))
			enabled = true;
		else if (!mSingleSelection && (mTextList->GetSelCount() > 0))
			enabled = true;
		mActionButton.EnableWindow(enabled);
	}
}

void CTextListChoice::OnDblclickList()
{
	// Only if selections allowed
	if (!mNoSelection)
	{
		if (mUseEntry)
		{
			// Change the entry
			if (mTextList->GetCurSel() != LB_ERR)
			{
				CString entry;
				mTextList->GetText(mTextList->GetCurSel(), entry);
				GetDlgItem(IDC_TEXTLISTENTER)->SetWindowText(entry);			
			}
		}
		else
			SendMessage(WM_COMMAND, IDOK);
	}
}

void CTextListChoice::OnChangeEntry()
{
	if (mUseEntry)
	{
		CString entry;
		GetDlgItem(IDC_TEXTLISTENTER)->GetWindowText(entry);

		mActionButton.EnableWindow(entry.GetLength() > 0);
	}
}

bool CTextListChoice::PoseDialog(const char* title, const char* desc, const char* item, bool use_entry,
						bool single_selection, bool no_selection, bool select_first,
						cdstrvect& items, cdstring& text, ulvector& selection, const char* btn)
{
	CTextListChoice dlog(CSDIFrame::GetAppTopWindow());
	dlog.mSingleSelection = single_selection;
	dlog.mNoSelection = no_selection;
	dlog.mSelectFirst = select_first;
	if (desc)
		dlog.mTextListDescription = rsrc::GetString(desc);
	if (item)
		dlog.mTextListEnterDescription = rsrc::GetString(item);
	if (btn)
		dlog.mButtonTitle = rsrc::GetString(btn);
	dlog.mTextListEnter = text;

	// Add the rest
	for(cdstrvect::const_iterator iter = items.begin(); iter != items.end(); iter++)
		dlog.mItems.push_back(*iter);

	// Let Dialog process events
	if (dlog.DoModal() == IDOK)
	{
		// Copy back items
		if (use_entry)
			text = dlog.mTextListEnter;
		else
			selection = dlog.mSelection;
			
		return true;
	}
	else
		return false;
}
