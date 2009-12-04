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


// CPrefsLetterOptions.cpp : implementation file
//

#include "CPrefsLetterOptions.h"

#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsLetterOptions dialog

IMPLEMENT_DYNAMIC(CPrefsLetterOptions, CTabPanel)

CPrefsLetterOptions::CPrefsLetterOptions()
	: CTabPanel(CPrefsLetterOptions::IDD)
{
	//{{AFX_DATA_INIT(CPrefsLetterOptions)
	//record_attachments = FALSE;
	mAutoInsertSignature = TRUE;
	mDisplayAttachments = FALSE;
	mAppendDraft = FALSE;
	inbox_append = FALSE;
	mShowCC = FALSE;
	mShowBCC = FALSE;
	//}}AFX_DATA_INIT
}


void CPrefsLetterOptions::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsLetterOptions)
	DDX_Check(pDX, IDC_AUTOINSERTSIG, mAutoInsertSignature);
	DDX_Check(pDX, IDC_BLANKLINE, mSigEmptyLine);
	DDX_Check(pDX, IDC_INSERTSIGDASHES, mSigDashes);
	DDX_Check(pDX, IDC_NOSUBJECTWARN, mNoSubjectWarn);
	DDX_Check(pDX, IDC_DISPLAYATTACHMENTS, mDisplayAttachments);
	DDX_Check(pDX, IDC_APPENDDRAFT, mAppendDraft);
	DDX_Check(pDX, IDC_COPYINBOXREPLY, inbox_append);
	DDX_Check(pDX, IDC_SMARTURLPASTE, mSmartURLPaste);
	DDX_Check(pDX, IDC_WRAPTOWINDOW, mComposeWrap);
	DDX_Check(pDX, IDC_DELETEORIGINALDRAFT, mDeleteOriginalDraft);
	DDX_Check(pDX, IDC_TEMPLATEDRAFTS, mTemplateDrafts);
	DDX_Check(pDX, IDC_STRIPSIGDASHES, mReplyNoSignature);
	DDX_Check(pDX, IDC_WARNREPLYSUBJECT, mWarnReplySubject);
	DDX_Check(pDX, IDC_SHOWCC, mShowCC);
	DDX_Check(pDX, IDC_SHOWBCC, mShowBCC);
	DDX_Check(pDX, IDC_ALWAYSUNICODE, mAlwaysUnicode);
	DDX_Check(pDX, IDC_DISPLAYIDENTITYFROM, mDisplayIdentityFrom);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsLetterOptions, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsLetterOptions)
	ON_BN_CLICKED(IDC_DELETEORIGINALDRAFT, OnDeleteOriginalDraft)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsLetterOptions message handlers

// Set data
void CPrefsLetterOptions::SetContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;
	mCopyPrefs = copyPrefs;

	// Set values
	mAutoInsertSignature = copyPrefs->mAutoInsertSignature.GetValue();
	mSigEmptyLine = copyPrefs->mSignatureEmptyLine.GetValue();
	mSigDashes = copyPrefs->mSigDashes.GetValue();
	mNoSubjectWarn = copyPrefs->mNoSubjectWarn.GetValue();
	mDisplayAttachments = copyPrefs->mDisplayAttachments.GetValue();
	mAppendDraft = copyPrefs->mAppendDraft.GetValue();
	inbox_append = copyPrefs->inbox_append.GetValue();
	mSmartURLPaste = copyPrefs->mSmartURLPaste.GetValue();
	mComposeWrap = !copyPrefs->mWindowWrap.GetValue();
	mDeleteOriginalDraft = copyPrefs->mDeleteOriginalDraft.GetValue();
	mTemplateDrafts = copyPrefs->mTemplateDrafts.GetValue();
	GetDlgItem(IDC_TEMPLATEDRAFTS)->EnableWindow(mDeleteOriginalDraft);
	mReplyNoSignature = copyPrefs->mReplyNoSignature.GetValue();
	mWarnReplySubject = copyPrefs->mWarnReplySubject.GetValue();
	mShowCC = copyPrefs->mShowCCs.GetValue();
	mShowBCC = copyPrefs->mShowBCCs.GetValue();
	mAlwaysUnicode = copyPrefs->mAlwaysUnicode.GetValue();
	mDisplayIdentityFrom = copyPrefs->mDisplayIdentityFrom.GetValue();
}

// Force update of data
bool CPrefsLetterOptions::UpdateContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Get values
	copyPrefs->mAutoInsertSignature.SetValue(mAutoInsertSignature);
	copyPrefs->mSignatureEmptyLine.SetValue(mSigEmptyLine);
	copyPrefs->mSigDashes.SetValue(mSigDashes);
	copyPrefs->mNoSubjectWarn.SetValue(mNoSubjectWarn);
	copyPrefs->mDisplayAttachments.SetValue(mDisplayAttachments);
	copyPrefs->mAppendDraft.SetValue(mAppendDraft);
	copyPrefs->inbox_append.SetValue(inbox_append);
	copyPrefs->mSmartURLPaste.SetValue(mSmartURLPaste);
	copyPrefs->mWindowWrap.SetValue(!mComposeWrap);
	copyPrefs->mDeleteOriginalDraft.SetValue(mDeleteOriginalDraft);
	copyPrefs->mReplyNoSignature.SetValue(mReplyNoSignature);
	copyPrefs->mWarnReplySubject.SetValue(mWarnReplySubject);
	copyPrefs->mShowCCs.SetValue(mShowCC);
	copyPrefs->mShowBCCs.SetValue(mShowBCC);
	copyPrefs->mAlwaysUnicode.SetValue(mAlwaysUnicode);
	copyPrefs->mDisplayIdentityFrom.SetValue(mDisplayIdentityFrom);
	
	return true;
}

void CPrefsLetterOptions::OnDeleteOriginalDraft()
{
	GetDlgItem(IDC_TEMPLATEDRAFTS)->EnableWindow(static_cast<CButton*>(GetDlgItem(IDC_DELETEORIGINALDRAFT))->GetCheck());
}
