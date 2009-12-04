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


// CSearchOptionsDialog.cpp : implementation file
//

#include "CSearchOptionsDialog.h"

#include "CPreferences.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CSearchOptionsDialog dialog


CSearchOptionsDialog::CSearchOptionsDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CSearchOptionsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSearchOptionsDialog)
	mMultiple = FALSE;
	mMaximum = 0;
	mLoadBalance = FALSE;
	mOpenFirst = FALSE;
	//}}AFX_DATA_INIT
}


void CSearchOptionsDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSearchOptionsDialog)
	DDX_Check(pDX, IDC_SEARCHOPTIONS_MULTI, mMultiple);
	DDX_Control(pDX, IDC_SEARCHOPTIONS_MULTI, mMultipleCtrl);
	DDX_UTF8Text(pDX, IDC_SEARCHOPTIONS_MAXIMUM, mMaximum);
	DDX_Control(pDX, IDC_SEARCHOPTIONS_MAXIMUM, mMaximumCtrl);
	DDX_Check(pDX, IDC_SEARCHOPTIONS_LOAD, mLoadBalance);
	DDX_Control(pDX, IDC_SEARCHOPTIONS_LOAD, mLoadBalanceCtrl);
	DDX_Check(pDX, IDC_SEARCHOPTIONS_OPENFIRST, mOpenFirst);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSearchOptionsDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CSearchOptionsDialog)
	ON_BN_CLICKED(IDC_SEARCHOPTIONS_MULTI, OnMultiple)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSearchOptionsDialog message handlers

BOOL CSearchOptionsDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	UpdateItems();

	return true;
}

void CSearchOptionsDialog::OnMultiple() 
{
	// TODO: Add your control notification handler code here
	UpdateItems();
}

// Update display items
void CSearchOptionsDialog::UpdateItems()
{
	mMaximumCtrl.EnableWindow(mMultipleCtrl.GetCheck());
	mLoadBalanceCtrl.EnableWindow(mMultipleCtrl.GetCheck());
}

// Set items from prefs
void CSearchOptionsDialog::SetItems()
{
	mMultiple = CPreferences::sPrefs->mMultipleSearch.GetValue();
	mMaximum = CPreferences::sPrefs->mNumberMultipleSearch.GetValue();
	mLoadBalance = CPreferences::sPrefs->mLoadBalanceSearch.GetValue();
	mOpenFirst = CPreferences::sPrefs->mOpenFirstSearchResult.GetValue();
}

// Set prefs from items
void CSearchOptionsDialog::GetItems()
{
	CPreferences::sPrefs->mMultipleSearch.SetValue(mMultiple);
	CPreferences::sPrefs->mNumberMultipleSearch.SetValue(mMaximum);
	CPreferences::sPrefs->mLoadBalanceSearch.SetValue(mLoadBalance);
	CPreferences::sPrefs->mOpenFirstSearchResult.SetValue(mOpenFirst);
}
