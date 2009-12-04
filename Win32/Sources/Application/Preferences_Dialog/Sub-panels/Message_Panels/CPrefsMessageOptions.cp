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


// CPrefsMessageOptions.cpp : implementation file
//


#include "CPrefsMessageOptions.h"

#include "CAdminLock.h"
#include "CPreferences.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsMessageOptions property page

IMPLEMENT_DYNCREATE(CPrefsMessageOptions, CTabPanel)

CPrefsMessageOptions::CPrefsMessageOptions() : CTabPanel(CPrefsMessageOptions::IDD)
{
	//{{AFX_DATA_INIT(CPrefsMessageOptions)
	deleteAfterCopy = FALSE;
	mOpenDeleted = FALSE;
	mOpenReuse = FALSE;
	saveMessageHeader = FALSE;
	showMessageHeader = FALSE;
	mShowStyledText = FALSE;
	mQuoteSelection = FALSE;
	alwaysQuote = FALSE;
	mMDNOptions = 0;
	//}}AFX_DATA_INIT
}

void CPrefsMessageOptions::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsMessageOptions)
	DDX_Check(pDX, IDC_DELETECOPY, deleteAfterCopy);
	DDX_Check(pDX, IDC_OPENDELETED, mOpenDeleted);
	DDX_Check(pDX, IDC_CLOSEDELETED, mCloseDeleted);
	DDX_Check(pDX, IDC_OPENREUSE, mOpenReuse);
	DDX_Check(pDX, IDC_QUOTESELECTION, mQuoteSelection);
	DDX_Check(pDX, IDC_ALWAYSQUOTE, alwaysQuote);
	DDX_Check(pDX, IDC_AUTODIGEST, mAutoDigest);
	DDX_Check(pDX, IDC_EXPANDHEADER, mExpandHeader);
	DDX_Check(pDX, IDC_EXPANDPARTS, mExpandParts);
	DDX_Check(pDX, IDC_SAVEHEADER, saveMessageHeader);
	DDX_Check(pDX, IDC_SHOWHEADER, showMessageHeader);
	DDX_Check(pDX, IDC_SHOWSTYLEDTEXT, mShowStyledText);
	DDX_Radio(pDX, IDC_MDNALWAYSSEND, mMDNOptions);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsMessageOptions, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsMessageOptions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Set up params for DDX
void CPrefsMessageOptions::SetContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Set values
	showMessageHeader = copyPrefs->showMessageHeader.GetValue();
	mShowStyledText = copyPrefs->showStyled.GetValue();
	saveMessageHeader = copyPrefs->saveMessageHeader.GetValue();
	deleteAfterCopy = copyPrefs->deleteAfterCopy.GetValue();
	mOpenDeleted = copyPrefs->mOpenDeleted.GetValue();
	mCloseDeleted = copyPrefs->mCloseDeleted.GetValue();
	mOpenReuse = copyPrefs->mOpenReuse.GetValue();
	mQuoteSelection = copyPrefs->mQuoteSelection.GetValue();
	alwaysQuote = copyPrefs->mAlwaysQuote.GetValue();
	mAutoDigest = copyPrefs->mAutoDigest.GetValue();
	mExpandHeader = copyPrefs->mExpandHeader.GetValue();
	mExpandParts = copyPrefs->mExpandParts.GetValue();

	// Admin lock
	if (CAdminLock::sAdminLock.mLockMDN)
	{
		mMDNOptions = eMDNAlwaysSend;
		GetDlgItem(IDC_MDNALWAYSSEND)->EnableWindow(false);
		GetDlgItem(IDC_MDNNEVERSEND)->EnableWindow(false);
		GetDlgItem(IDC_MDNPROMPTSEND)->EnableWindow(false);
	}
	else
		mMDNOptions = copyPrefs->mMDNOptions.GetValue().GetValue();
}

// Get params from DDX
bool CPrefsMessageOptions::UpdateContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Get values
	copyPrefs->showMessageHeader.SetValue(showMessageHeader);
	copyPrefs->showStyled.SetValue(mShowStyledText);
	copyPrefs->saveMessageHeader.SetValue(saveMessageHeader);
	copyPrefs->deleteAfterCopy.SetValue(deleteAfterCopy);
	copyPrefs->mOpenDeleted.SetValue(mOpenDeleted);
	copyPrefs->mCloseDeleted.SetValue(mCloseDeleted);
	copyPrefs->mOpenReuse.SetValue(mOpenReuse);
	copyPrefs->mQuoteSelection.SetValue(mQuoteSelection);
	copyPrefs->mAlwaysQuote.SetValue(alwaysQuote);
	copyPrefs->mAutoDigest.SetValue(mAutoDigest);
	copyPrefs->mExpandHeader.SetValue(mExpandHeader);
	copyPrefs->mExpandParts.SetValue(mExpandParts);

	// Admin lock
	if (CAdminLock::sAdminLock.mLockMDN)
		copyPrefs->mMDNOptions.SetValue(eMDNAlwaysSend);
	else
		copyPrefs->mMDNOptions.SetValue(static_cast<EMDNOptions>(mMDNOptions));
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsMessageOptions message handlers
