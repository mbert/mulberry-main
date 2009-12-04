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

// CForwardOptionsDialog.cpp : implementation file
//

#include "CForwardOptionsDialog.h"

#include "CPreferences.h"
#include "CSDIFrame.h"

/////////////////////////////////////////////////////////////////////////////
// CForwardOptionsDialog dialog

EForwardOptions CForwardOptionsDialog::sForward = eForwardQuote;
bool CForwardOptionsDialog::sForwardInit = false;

CForwardOptionsDialog::CForwardOptionsDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CForwardOptionsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CForwardOptionsDialog)
	mForwardQuoteOriginal = FALSE;
	mForwardHeaders = FALSE;
	mForwardAttachment = FALSE;
	mForwardRFC822 = FALSE;
	mSave = FALSE;
	//}}AFX_DATA_INIT
	
	InitOptions();
}


void CForwardOptionsDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CForwardOptionsDialog)
	DDX_Check(pDX, IDC_FORWARDOPTIONS_QUOTE, mForwardQuoteOriginal);
	DDX_Check(pDX, IDC_FORWARDOPTIONS_HEADERS, mForwardHeaders);
	DDX_Check(pDX, IDC_FORWARDOPTIONS_ATTACHMENT, mForwardAttachment);
	DDX_Check(pDX, IDC_FORWARDOPTIONS_RFC822, mForwardRFC822);
	DDX_Check(pDX, IDC_FORWARDOPTIONS_SAVE, mSave);
	DDX_Radio(pDX, IDC_FORWARDOPTIONS_QUOTESELECTION, mQuote);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CForwardOptionsDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CForwardOptionsDialog)
	ON_BN_CLICKED(IDC_FORWARDOPTIONS_QUOTE, OnForwardOptionsQuote)
	ON_BN_CLICKED(IDC_FORWARDOPTIONS_ATTACHMENT, OnForwardOptionsAttachment)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CForwardOptionsDialog message handlers

BOOL CForwardOptionsDialog::OnInitDialog() 
{
	CHelpDialog::OnInitDialog();
	
	if (!mForwardQuoteOriginal)
		GetDlgItem(IDC_FORWARDOPTIONS_HEADERS)->EnableWindow(false);
	
	if (!mForwardAttachment)
		GetDlgItem(IDC_FORWARDOPTIONS_RFC822)->EnableWindow(false);

	if (!mHasSelection)
		GetDlgItem(IDC_FORWARDOPTIONS_QUOTESELECTION)->EnableWindow(false);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CForwardOptionsDialog::OnForwardOptionsQuote() 
{
	GetDlgItem(IDC_FORWARDOPTIONS_HEADERS)->EnableWindow(static_cast<CButton*>(GetDlgItem(IDC_FORWARDOPTIONS_QUOTE))->GetCheck());
}

void CForwardOptionsDialog::OnForwardOptionsAttachment() 
{
	GetDlgItem(IDC_FORWARDOPTIONS_RFC822)->EnableWindow(static_cast<CButton*>(GetDlgItem(IDC_FORWARDOPTIONS_ATTACHMENT))->GetCheck());
}

void CForwardOptionsDialog::InitOptions()
{
	if (!sForwardInit)
	{
		sForward = static_cast<EForwardOptions>(0);
		if (CPreferences::sPrefs->mForwardQuoteOriginal.GetValue())
			sForward = static_cast<EForwardOptions>(sForward | eForwardQuote);
		if (CPreferences::sPrefs->mForwardHeaders.GetValue())
			sForward = static_cast<EForwardOptions>(sForward | eForwardHeaders);
		if (CPreferences::sPrefs->mForwardAttachment.GetValue())
			sForward = static_cast<EForwardOptions>(sForward | eForwardAttachment);
		if (CPreferences::sPrefs->mForwardRFC822.GetValue())
			sForward = static_cast<EForwardOptions>(sForward | eForwardRFC822);

		sForwardInit = true;
	}
}

// Set the dialogs info
void CForwardOptionsDialog::SetDetails(EReplyQuote quote, bool has_selection)
{
	mForwardQuoteOriginal = (sForward & eForwardQuote) ? true : false;
	mForwardHeaders = (sForward & eForwardHeaders) ? true : false;
	mForwardAttachment = (sForward & eForwardAttachment) ? true : false;
	mForwardRFC822 = (sForward & eForwardRFC822) ? true : false;
	
	// Set it up
	switch(quote)
	{
	case eQuoteSelection:
		mQuote = (has_selection ? 0 : 1);
		break;
	case eQuoteAll:
		mQuote = 1;
		break;
	case eQuoteNone:
		mQuote = 2;
		break;
	}
	mHasSelection = has_selection;
	
	mSave = false;
}

// Set the dialogs info
void CForwardOptionsDialog::GetDetails(EReplyQuote& quote, bool& save)
{
	sForward = static_cast<EForwardOptions>(0);
	if (mForwardQuoteOriginal)
		sForward = static_cast<EForwardOptions>(sForward | eForwardQuote);
	if (mForwardHeaders)
		sForward = static_cast<EForwardOptions>(sForward | eForwardHeaders);
	if (mForwardAttachment)
		sForward = static_cast<EForwardOptions>(sForward | eForwardAttachment);
	if (mForwardRFC822)
		sForward = static_cast<EForwardOptions>(sForward | eForwardRFC822);
	save = mSave;

	if (mQuote == 0)
		quote = eQuoteSelection;
	else if (mQuote == 1)
		quote = eQuoteAll;
	else if (mQuote == 2)
		quote = eQuoteNone;
}

bool CForwardOptionsDialog::PoseDialog(EForwardOptions& forward, EReplyQuote& quote, bool has_selection)
{
	bool result = false;

	CForwardOptionsDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetDetails(quote, has_selection);

	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{
		bool save;
		dlog.GetDetails(quote, save);
		forward = sForward;

		// Set preferences if required
		if (save)
		{
			CPreferences::sPrefs->mForwardChoice.SetValue(false);
			CPreferences::sPrefs->mForwardQuoteOriginal.SetValue(sForward & eForwardQuote);
			CPreferences::sPrefs->mForwardHeaders.SetValue(sForward & eForwardHeaders);
			CPreferences::sPrefs->mForwardAttachment.SetValue(sForward & eForwardAttachment);
			CPreferences::sPrefs->mForwardRFC822.SetValue(sForward & eForwardRFC822);
		}
		result = true;
	}

	return result;
}
