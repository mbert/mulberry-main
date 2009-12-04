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

// CSpellEditDialog.cpp : implementation file
//

#include "CSpellEditDialog.h"

#include "CSDIFrame.h"
#include "CSpellAddDialog.h"
#include "CSpellPlugin.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CSpellEditDialog dialog


CSpellEditDialog::CSpellEditDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CSpellEditDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSpellEditDialog)
	//}}AFX_DATA_INIT
}


void CSpellEditDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpellEditDialog)
	DDX_Control(pDX, IDC_SPELLEDIT_PAGE, mPageCount);
	DDX_Control(pDX, IDC_SPELLEDIT_WORDBOTTOM, mWordBottom);
	DDX_Control(pDX, IDC_SPELLEDIT_WORDTOP, mWordTop);
	DDX_Control(pDX, IDC_SPELLEDIT_WORD, mWord);
	DDX_Control(pDX, IDC_SPELLEDIT_REMOVE, mRemove);
	DDX_Control(pDX, IDC_SPELLEDIT_FIND, mFind);
	DDX_Control(pDX, IDC_SPELLEDIT_ADD, mAdd);
	DDX_Control(pDX, IDC_SPELLEDIT_LIST, mList);
	DDX_Control(pDX, IDC_SPELLEDIT_DICT, mDictName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpellEditDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CSpellEditDialog)
	ON_BN_CLICKED(IDC_SPELLEDIT_ADD, OnSpellEditAdd)
	ON_BN_CLICKED(IDC_SPELLEDIT_FIND, OnSpellEditFind)
	ON_BN_CLICKED(IDC_SPELLEDIT_REMOVE, OnSpellEditRemove)
	ON_LBN_SELCANCEL(IDC_SPELLEDIT_LIST, OnSelcancelSpellEditList)
	ON_LBN_SELCHANGE(IDC_SPELLEDIT_LIST, OnSelchangeSpellEditList)
	ON_EN_CHANGE(IDC_SPELLEDIT_WORD, OnChangeSpellEditWord)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpellEditDialog message handlers

BOOL CSpellEditDialog::OnInitDialog() 
{
	CHelpDialog::OnInitDialog();
	
	// Set details in scroller
	mList.SetDetails(mSpeller, &mPageCount);
	mList.SetRangeCaptions(&mWordTop, &mWordBottom);
	mList.DisplayPages(true);

	// Start at first page
	mList.InitPage();

	// Remove Find button if only one page
	if (!mSpeller->CountPages())
		mFind.ShowWindow(SW_HIDE);

	// Set dictionary name
	CString dname = cdustring(mSpeller->GetPreferences()->mDictionaryName.GetValue());
	mDictName.SetWindowText(dname);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSpellEditDialog::OnSpellEditAdd() 
{
	cdstring add_word = CUnicodeUtils::GetWindowTextUTF8(&mWord);
	
	// Must contain something
	if (add_word.empty())
		return;

	// Must not be in dictionary
	if (mSpeller->ContainsWord(add_word))
	{
		// Put error alert
		
		return;
	}
	
	// Create the dialog
	CSpellAddDialog::PoseDialog(mSpeller, add_word);
}

void CSpellEditDialog::OnSpellEditFind() 
{
	cdstring find_word = CUnicodeUtils::GetWindowTextUTF8(&mWord);
	
	// Must contain something
	if (find_word.empty())
		return;

	// Refresh dict view
	mList.DisplayDictionary(find_word);
}

void CSpellEditDialog::OnSpellEditRemove() 
{
	int num_sel = mList.GetSelCount();
	int* selected = nil;
	try
	{
		selected = new int(num_sel);
		mList.GetSelItems(num_sel, selected);
		for(int i = 0; i < num_sel; i++)
		{
			CString str;
			mList.GetText(selected[i], str);

			// Delete in dict
			mSpeller->RemoveWord(cdstring(str));
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Clean up and throw up
		delete selected;
		
		CLOG_LOGRETHROW;
		throw;
	}

	// Refresh dict view
	mList.DisplayDictionary(nil);
}

void CSpellEditDialog::OnSelcancelSpellEditList() 
{
	if (mList.GetSelCount() == 0)
		mRemove.EnableWindow(false);
	else
		mRemove.EnableWindow(true);
}

void CSpellEditDialog::OnSelchangeSpellEditList() 
{
	if (mList.GetSelCount() == 0)
		mRemove.EnableWindow(false);
	else
		mRemove.EnableWindow(true);
}

void CSpellEditDialog::OnChangeSpellEditWord() 
{
	if (mWord.GetWindowTextLength() == 0)
		mFind.EnableWindow(false);
	else
		mFind.EnableWindow(true);
}

bool CSpellEditDialog::PoseDialog(CSpellPlugin* speller)
{
	bool result = false;

	// Create the dialog
	CSpellEditDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetSpeller(speller);

	dlog.DoModal();

	return result;
}