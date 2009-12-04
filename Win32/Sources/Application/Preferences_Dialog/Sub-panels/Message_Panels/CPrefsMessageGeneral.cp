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


// CPrefsMessageGeneral.cpp : implementation file
//


#include "CPrefsMessageGeneral.h"

#include "CPreferences.h"
#include "CPrefsEditCaption.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsMessageGeneral property page

IMPLEMENT_DYNCREATE(CPrefsMessageGeneral, CTabPanel)

CPrefsMessageGeneral::CPrefsMessageGeneral()
	 : CTabPanel(CPrefsMessageGeneral::IDD)
{
	//{{AFX_DATA_INIT(CPrefsMessageGeneral)
	mDoSizeWarn = -1;
	optionKeyReplyDialog = -1;
	warnMessageSize = 0;
	//}}AFX_DATA_INIT
}

void CPrefsMessageGeneral::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsMessageGeneral)
	DDX_Control(pDX, IDC_WARNSIZE, mWarnSizeCtrl);
	DDX_Radio(pDX, IDC_NOLIMIT, mDoSizeWarn);
	DDX_Radio(pDX, IDC_REPLYALTDOWN, optionKeyReplyDialog);
	DDX_UTF8Text(pDX, IDC_SAVEEXTENSION, saveCreator);
	DDX_UTF8Text(pDX, IDC_WARNSIZE, warnMessageSize);
	DDV_MinMaxLong(pDX, warnMessageSize, 0, 2147483647);
	DDX_Radio(pDX, IDC_PREFSMESSAGE_FORWARDCHOICE, mForwardChoice);
	DDX_Check(pDX, IDC_PREFSMESSAGE_FORWARDQUOTE, mForwardQuoteOriginal);
	DDX_Check(pDX, IDC_PREFSMESSAGE_FORWARDHEADERS, mForwardHeaders);
	DDX_Check(pDX, IDC_PREFSMESSAGE_FORWARDATTACHMENT, mForwardAttachment);
	DDX_Check(pDX, IDC_PREFSMESSAGE_FORWARDRFC822, mForwardRFC822);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsMessageGeneral, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsMessageGeneral)
	ON_BN_CLICKED(IDC_LIMITSIZE, OnLimitSize)
	ON_BN_CLICKED(IDC_NOLIMIT, OnNoLimit)
	ON_BN_CLICKED(IDC_PRINTCAPTION, OnPrintCaption)
	ON_BN_CLICKED(IDC_PREFSMESSAGE_FORWARDCHOICE, OnForwardChoice)
	ON_BN_CLICKED(IDC_PREFSMESSAGE_FORWARDOPTIONS, OnForwardOptions)
	ON_BN_CLICKED(IDC_PREFSMESSAGE_FORWARDQUOTE, OnForwardQuoteOriginal)
	ON_BN_CLICKED(IDC_PREFSMESSAGE_FORWARDATTACHMENT, OnForwardAttachment)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Set up params for DDX
void CPrefsMessageGeneral::SetContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;
	mCopyPrefs = copyPrefs;

	// Set values
	saveCreator = CString(copyPrefs->mSaveCreator.GetValue());
	mDoSizeWarn = copyPrefs->mDoSizeWarn.GetValue();
	warnMessageSize = copyPrefs->warnMessageSize.GetValue();
	optionKeyReplyDialog = !copyPrefs->optionKeyReplyDialog.GetValue();

	mForwardChoice = copyPrefs->mForwardChoice.GetValue() ? 0 : 1;
	if (!mForwardChoice)
		OnForwardChoice();
	mForwardQuoteOriginal = copyPrefs->mForwardQuoteOriginal.GetValue();
	if (!mForwardQuoteOriginal)
		GetDlgItem(IDC_PREFSMESSAGE_FORWARDHEADERS)->EnableWindow(false);
	mForwardHeaders = copyPrefs->mForwardHeaders.GetValue();
	mForwardAttachment = copyPrefs->mForwardAttachment.GetValue();
	if (!mForwardAttachment)
		GetDlgItem(IDC_PREFSMESSAGE_FORWARDRFC822)->EnableWindow(false);
	mForwardRFC822 = copyPrefs->mForwardRFC822.GetValue();

	// set initial control states
	mWarnSizeCtrl.EnableWindow(mDoSizeWarn);
}

// Get params from DDX
bool CPrefsMessageGeneral::UpdateContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Get values
	if (saveCreator[(cdstring::size_type)0] == '.')
		copyPrefs->mSaveCreator.SetValue(((const char*) saveCreator) + 1);
	else
		copyPrefs->mSaveCreator.SetValue(saveCreator);
	copyPrefs->mDoSizeWarn.SetValue(mDoSizeWarn);
	copyPrefs->warnMessageSize.SetValue(warnMessageSize);
	copyPrefs->optionKeyReplyDialog.SetValue((optionKeyReplyDialog == 0));

	copyPrefs->mForwardChoice.SetValue(mForwardChoice == 0);
	copyPrefs->mForwardQuoteOriginal.SetValue(mForwardQuoteOriginal);
	copyPrefs->mForwardHeaders.SetValue(mForwardHeaders);
	copyPrefs->mForwardAttachment.SetValue(mForwardAttachment);
	copyPrefs->mForwardRFC822.SetValue(mForwardRFC822);
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsMessageGeneral message handlers

void CPrefsMessageGeneral::OnLimitSize()
{
	// TODO: Add your control notification handler code here
	mWarnSizeCtrl.EnableWindow(true);
}

void CPrefsMessageGeneral::OnNoLimit()
{
	// TODO: Add your control notification handler code here
	mWarnSizeCtrl.EnableWindow(false);
}

void CPrefsMessageGeneral::OnPrintCaption()
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

void CPrefsMessageGeneral::OnForwardChoice()
{
	GetDlgItem(IDC_PREFSMESSAGE_FORWARDQUOTE)->EnableWindow(false);
	GetDlgItem(IDC_PREFSMESSAGE_FORWARDHEADERS)->EnableWindow(false);
	GetDlgItem(IDC_PREFSMESSAGE_FORWARDRFC822)->EnableWindow(false);
}

void CPrefsMessageGeneral::OnForwardOptions()
{
	GetDlgItem(IDC_PREFSMESSAGE_FORWARDQUOTE)->EnableWindow(true);
	GetDlgItem(IDC_PREFSMESSAGE_FORWARDHEADERS)->EnableWindow(true);
	GetDlgItem(IDC_PREFSMESSAGE_FORWARDRFC822)->EnableWindow(true);
}

void CPrefsMessageGeneral::OnForwardQuoteOriginal()
{
	GetDlgItem(IDC_PREFSMESSAGE_FORWARDHEADERS)->EnableWindow(static_cast<CButton*>(GetDlgItem(IDC_PREFSMESSAGE_FORWARDQUOTE))->GetCheck());
}

void CPrefsMessageGeneral::OnForwardAttachment()
{
	GetDlgItem(IDC_PREFSMESSAGE_FORWARDRFC822)->EnableWindow(static_cast<CButton*>(GetDlgItem(IDC_PREFSMESSAGE_FORWARDATTACHMENT))->GetCheck());
}
