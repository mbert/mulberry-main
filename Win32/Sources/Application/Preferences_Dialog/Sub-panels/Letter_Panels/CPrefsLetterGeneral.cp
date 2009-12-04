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


// CPrefsLetterGeneral.cpp : implementation file
//

#include "CPrefsLetterGeneral.h"

#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CPrefsEditCaption.h"
#include "CPrefsEditHeadFoot.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsLetterGeneral dialog

IMPLEMENT_DYNAMIC(CPrefsLetterGeneral, CTabPanel)

CPrefsLetterGeneral::CPrefsLetterGeneral()
	: CTabPanel(CPrefsLetterGeneral::IDD)
{
	//{{AFX_DATA_INIT(CPrefsLetterGeneral)
	//record_attachments = FALSE;
	spaces_per_tab = 0;
	wrap_length = 0;
	mSeparateBCC = FALSE;
	mSaveDraft = 0;
	//}}AFX_DATA_INIT
}


void CPrefsLetterGeneral::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsLetterGeneral)
	DDX_UTF8Text(pDX, IDC_DEFAULTDOMAIN, mMailDomain);
	DDX_UTF8Text(pDX, IDC_SPACESPERTAB, spaces_per_tab);
	DDX_Check(pDX, IDC_TABSPACE, mTabSpace);
	DDV_MinMaxInt(pDX, spaces_per_tab, 0, 32767);
	DDX_UTF8Text(pDX, IDC_WRAPLENGTH, wrap_length);
	DDV_MinMaxInt(pDX, wrap_length, 0, 32767);
	DDX_UTF8Text(pDX, IDC_INCLUDEPREFIX, mReplyQuote);
	DDX_UTF8Text(pDX, IDC_FORWARDPREFIX, mForwardQuote);
	DDX_UTF8Text(pDX, IDC_FORWARDSUBJECT, mForwardSubject);
	DDX_Check(pDX, IDC_SEPARATEBCC, mSeparateBCC);
	DDX_Radio(pDX, IDC_SAVEDRAFTTOFILE, mSaveDraft);
	DDX_UTF8Text(pDX, IDC_SAVEDRAFTMAILBOXNAME, mSaveToMailboxName);
	DDX_Control(pDX, IDC_SAVEDRAFTMAILBOXNAME, mSaveToMailboxNameCtrl);
	DDX_Check(pDX, IDC_AUTOSAVEDRAFTS, mAutoSaveDrafts);
	DDX_UTF8Text(pDX, IDC_AUTOSAVEDRAFTSINTERVAL, mAutoSaveDraftsInterval);
	DDV_MinMaxInt(pDX, mAutoSaveDraftsInterval, 0, 32767);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsLetterGeneral, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsLetterGeneral)
	ON_BN_CLICKED(IDC_REPLYCAPTION, OnReplyCaption)
	ON_BN_CLICKED(IDC_FORWARDCAPTION, OnForwardCaption)
	ON_BN_CLICKED(IDC_LTRPRINTCAPTION, OnPrintCaption)
	ON_BN_CLICKED(IDC_SEPARATEBCC, OnSeparateBCC)
	ON_BN_CLICKED(IDC_SAVEDRAFTTOFILE, OnSaveDraftDisable)
	ON_BN_CLICKED(IDC_SAVEDRAFTTOMAILBOX, OnSaveDraftEnable)
	ON_BN_CLICKED(IDC_SAVEDRAFTCHOOSE, OnSaveDraftDisable)
	ON_COMMAND_RANGE(IDM_AppendToPopupNone, IDM_AppendToPopupEnd, OnChangeSaveTo)
	ON_BN_CLICKED(IDC_BCCCAPTION, OnBCCCaption)
	ON_BN_CLICKED(IDC_AUTOSAVEDRAFTS, OnAutoSaveDrafts)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsLetterGeneral message handlers

BOOL CPrefsLetterGeneral::OnInitDialog() 
{
	CTabPanel::OnInitDialog();
	
	// Subclass buttons
	mSaveToMailboxPopup.SubclassDlgItem(IDC_SAVEDRAFTMAILBOXPOPUP, this, IDI_POPUPBTN);
	mSaveToMailboxPopup.SetButtonText(false);
	mSaveToMailboxPopup.SetCopyTo(false);
	mSaveToMailboxPopup.EnableWindow(CMulberryApp::sApp->LoadedPrefs());
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Set data
void CPrefsLetterGeneral::SetContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;
	mCopyPrefs = copyPrefs;

	// Set values
	mMailDomain = copyPrefs->mMailDomain.GetValue();
	spaces_per_tab = copyPrefs->spaces_per_tab.GetValue();
	mTabSpace = copyPrefs->mTabSpace.GetValue();
	wrap_length = copyPrefs->wrap_length.GetValue();
	mReplyQuote = copyPrefs->mReplyQuote.GetValue();
	mForwardQuote = copyPrefs->mForwardQuote.GetValue();
	mForwardSubject = copyPrefs->mForwardSubject.GetValue();
	mSeparateBCC = copyPrefs->mSeparateBCC.GetValue();
	if (!mSeparateBCC)
		GetDlgItem(IDC_BCCCAPTION)->EnableWindow(false);
	mBCCCaption = copyPrefs->mBCCCaption.GetValue();
	
	mSaveDraft = copyPrefs->mSaveOptions.GetValue().GetValue();
	mSaveToMailboxName = copyPrefs->mSaveMailbox.GetValue();
	if (mSaveDraft != eSaveDraftToMailbox)
		OnSaveDraftDisable();
	
	mAutoSaveDrafts = copyPrefs->mAutoSaveDrafts.GetValue();
	mAutoSaveDraftsInterval = copyPrefs->mAutoSaveDraftsInterval.GetValue();
	GetDlgItem(IDC_AUTOSAVEDRAFTSINTERVAL)->EnableWindow(mAutoSaveDrafts);
}

// Force update of data
bool CPrefsLetterGeneral::UpdateContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Get values
	copyPrefs->mMailDomain.SetValue(mMailDomain);
	copyPrefs->spaces_per_tab.SetValue(spaces_per_tab);
	copyPrefs->mTabSpace.SetValue(mTabSpace);
	copyPrefs->wrap_length.SetValue(wrap_length);
	copyPrefs->mReplyQuote.SetValue(mReplyQuote);
	copyPrefs->mForwardQuote.SetValue(mForwardQuote);
	copyPrefs->mForwardSubject.SetValue(mForwardSubject);
	copyPrefs->mSeparateBCC.SetValue(mSeparateBCC);
	copyPrefs->mBCCCaption.SetValue(mBCCCaption);
	copyPrefs->mSaveOptions.SetValue(static_cast<ESaveOptions>(mSaveDraft));
	copyPrefs->mSaveMailbox.SetValue(mSaveToMailboxName);
	copyPrefs->mAutoSaveDrafts.SetValue(mAutoSaveDrafts);
	copyPrefs->mAutoSaveDraftsInterval.SetValue(mAutoSaveDraftsInterval);
	
	return true;
}

void CPrefsLetterGeneral::OnReplyCaption()
{
	CPrefsEditCaption::PoseDialog(&mCopyPrefs->mReplyStart, NULL, &mCopyPrefs->mReplyEnd, NULL, &mCopyPrefs->mReplyCursorTop, NULL, NULL, NULL);
}

void CPrefsLetterGeneral::OnForwardCaption()
{
	CPrefsEditCaption::PoseDialog(&mCopyPrefs->mForwardStart, NULL, &mCopyPrefs->mForwardEnd, NULL, &mCopyPrefs->mForwardCursorTop, NULL, NULL, NULL);
}

void CPrefsLetterGeneral::OnPrintCaption()
{
	CPrefsEditCaption::PoseDialog(&mCopyPrefs->mHeaderCaption,
									&mCopyPrefs->mLtrHeaderCaption,
									&mCopyPrefs->mFooterCaption,
									&mCopyPrefs->mLtrFooterCaption,
									NULL,
									&mCopyPrefs->mHeaderBox,
									&mCopyPrefs->mFooterBox,
									&mCopyPrefs->mPrintSummary);
}

void CPrefsLetterGeneral::OnSeparateBCC()
{
	GetDlgItem(IDC_BCCCAPTION)->EnableWindow(static_cast<CButton*>(GetDlgItem(IDC_SEPARATEBCC))->GetCheck());
}

void CPrefsLetterGeneral::OnBCCCaption()
{
	CPrefsEditHeadFoot::PoseDialog(mCopyPrefs, mBCCCaption, false);
}

void CPrefsLetterGeneral::OnSaveDraftEnable()
{
	mSaveToMailboxNameCtrl.EnableWindow(true);
	mSaveToMailboxPopup.EnableWindow(CMulberryApp::sApp->LoadedPrefs());
}

void CPrefsLetterGeneral::OnSaveDraftDisable()
{
	mSaveToMailboxNameCtrl.EnableWindow(false);
	mSaveToMailboxPopup.EnableWindow(false);
}

void CPrefsLetterGeneral::OnChangeSaveTo(UINT nID)
{
	if (nID == IDM_CopyToPopupNone)
		CUnicodeUtils::SetWindowTextUTF8(&mSaveToMailboxNameCtrl, cdstring::null_str);
	else
	{
		// Must set/reset control value to ensure selected mailbox is returned but
		// popup does not display check mark
		mSaveToMailboxPopup.SetValue(nID);
		cdstring mbox_name;
		if (mSaveToMailboxPopup.GetSelectedMboxName(mbox_name, true))
			CUnicodeUtils::SetWindowTextUTF8(&mSaveToMailboxNameCtrl, mbox_name);
		mSaveToMailboxPopup.SetValue(-1);
	}
}

void CPrefsLetterGeneral::OnAutoSaveDrafts()
{
	GetDlgItem(IDC_AUTOSAVEDRAFTSINTERVAL)->EnableWindow(static_cast<CButton*>(GetDlgItem(IDC_AUTOSAVEDRAFTS))->GetCheck());
}

