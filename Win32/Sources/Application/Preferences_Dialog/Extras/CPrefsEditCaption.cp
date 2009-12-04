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


// CPrefsEditCaption.cpp : implementation file
//

#include "CPrefsEditCaption.h"

#include "CCaptionParser.h"
#include "CFontCache.h"
#include "CPreferences.h"
#include "CPreferenceValue.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsEditCaption dialog


CPrefsEditCaption::CPrefsEditCaption(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CPrefsEditCaption::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPrefsEditCaption)
	//}}AFX_DATA_INIT
}


void CPrefsEditCaption::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsEditCaption)
	DDX_Control(pDX, IDC_CAPTION_PAGE, mPageBtn);
	DDX_Control(pDX, IDC_CAPTION_CURSOR_TOP, mCursorTopCtrl);
	DDX_Control(pDX, IDC_CAPTION_HEADER_MESSAGE, mMessage1Ctrl);
	DDX_Control(pDX, IDC_CAPTION_HEADER_DRAFT, mDraft1Ctrl);
	DDX_Control(pDX, IDC_CAPTION_HEADER_BOX, mUseBox1Ctrl);
	DDX_Control(pDX, IDC_CAPTION_PRINT_SUMMARY, mSummaryCtrl);
	DDX_Control(pDX, IDC_CAPTION_HEADER, mEditHeaderCtrl);
	DDX_Control(pDX, IDC_CAPTION_FOOTER_MESSAGE, mMessage2Ctrl);
	DDX_Control(pDX, IDC_CAPTION_FOOTER_DRAFT, mDraft2Ctrl);
	DDX_Control(pDX, IDC_CAPTION_FOOTER_BOX, mUseBox2Ctrl);
	DDX_Control(pDX, IDC_CAPTION_FOOTER, mEditFooterCtrl);
	//}}AFX_DATA_MAP
	
	// Do set/update
	if (pDX->m_bSaveAndValidate)
	{
		// Copy from data
		GetEditorText();
	}
	else
	{
		// Copy to data
		SetData(mText1Item, mText2Item, mText3Item, mText4Item, mCursorTopItem, mBox1Item, mBox2Item, mSummaryItem);
	}
}


BEGIN_MESSAGE_MAP(CPrefsEditCaption, CHelpDialog)
	//{{AFX_MSG_MAP(CPrefsEditCaption)
	ON_BN_CLICKED(IDC_CAPTION_MYNAME, OnCaptionMyName)
	ON_BN_CLICKED(IDC_CAPTION_MYEMAIL, OnCaptionMyEmail)
	ON_BN_CLICKED(IDC_CAPTION_MYFULL, OnCaptionMyFull)
	ON_BN_CLICKED(IDC_CAPTION_MYFIRST, OnCaptionMyFirst)
	ON_BN_CLICKED(IDC_CAPTION_SMARTNAME, OnCaptionSmartName)
	ON_BN_CLICKED(IDC_CAPTION_SMARTEMAIL, OnCaptionSmartEmail)
	ON_BN_CLICKED(IDC_CAPTION_SMARTFULL, OnCaptionSmartFull)
	ON_BN_CLICKED(IDC_CAPTION_SMARTFIRST, OnCaptionSmartFirst)
	ON_BN_CLICKED(IDC_CAPTION_FROMNAME, OnCaptionFromName)
	ON_BN_CLICKED(IDC_CAPTION_FROMEMAIL, OnCaptionFromEmail)
	ON_BN_CLICKED(IDC_CAPTION_FROMFULL, OnCaptionFromFull)
	ON_BN_CLICKED(IDC_CAPTION_FROMFIRST, OnCaptionFromFirst)
	ON_BN_CLICKED(IDC_CAPTION_TONAME, OnCaptionToName)
	ON_BN_CLICKED(IDC_CAPTION_TOEMAIL, OnCaptionToEmail)
	ON_BN_CLICKED(IDC_CAPTION_TOFULL, OnCaptionToFull)
	ON_BN_CLICKED(IDC_CAPTION_TOFIRST, OnCaptionToFirst)
	ON_BN_CLICKED(IDC_CAPTION_CCNAME, OnCaptionCCName)
	ON_BN_CLICKED(IDC_CAPTION_CCEMAIL, OnCaptionCCEmail)
	ON_BN_CLICKED(IDC_CAPTION_CCFULL, OnCaptionCCFull)
	ON_BN_CLICKED(IDC_CAPTION_CCFIRST, OnCaptionCCFirst)
	ON_BN_CLICKED(IDC_CAPTION_SUBJECT, OnCaptionSubject)
	ON_BN_CLICKED(IDC_CAPTION_SENTSHORT, OnCaptionSentShort)
	ON_BN_CLICKED(IDC_CAPTION_SENTLONG, OnCaptionSentLong)
	ON_BN_CLICKED(IDC_CAPTION_DATENOW, OnCaptionDateNow)
	ON_BN_CLICKED(IDC_CAPTION_PAGE, OnCaptionPage)
	ON_BN_CLICKED(IDC_CAPTION_REVERT, OnCaptionRevert)
	ON_BN_CLICKED(IDC_CAPTION_HEADER_MESSAGE, OnCaptionMessage1)
	ON_BN_CLICKED(IDC_CAPTION_HEADER_DRAFT, OnCaptionDraft1)
	ON_BN_CLICKED(IDC_CAPTION_FOOTER_MESSAGE, OnCaptionMessage2)
	ON_BN_CLICKED(IDC_CAPTION_FOOTER_DRAFT, OnCaptionDraft2)
	ON_EN_SETFOCUS(IDC_CAPTION_HEADER, OnHeaderSetFocus)
	ON_EN_SETFOCUS(IDC_CAPTION_FOOTER, OnFooterSetFocus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPrefsEditCaption::PoseDialog(CPreferenceValueMap<cdstring>* txt1,
									CPreferenceValueMap<cdstring>* txt2,
									CPreferenceValueMap<cdstring>* txt3,
									CPreferenceValueMap<cdstring>* txt4,
									CPreferenceValueMap<bool>* cursor_top,
									CPreferenceValueMap<bool>* use_box1,
									CPreferenceValueMap<bool>* use_box2,
									CPreferenceValueMap<bool>* summary)
{
	// Create dlog
	CPrefsEditCaption dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetCurrentSpacesPerTab(CPreferences::sPrefs->spaces_per_tab.GetValue());
	dlog.SetFont(CFontCache::GetDisplayFont());
	dlog.mText1Item = txt1;
	dlog.mText2Item = txt2;
	dlog.mText3Item = txt3;
	dlog.mText4Item = txt4;
	dlog.mCursorTopItem = cursor_top;
	dlog.mBox1Item = use_box1;
	dlog.mBox2Item = use_box2;
	dlog.mSummaryItem = summary;

	dlog.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsEditCaption message handlers

BOOL CPrefsEditCaption::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Set fonts to prefs default
	mEditHeaderCtrl.SetFont(mFont);
	mEditFooterCtrl.SetFont(mFont);

	mEditHeaderCtrl.SetFocus();
	mEditFocusCtrl = &mEditHeaderCtrl;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPrefsEditCaption::AddCaptionItem(const char* item)
{
	cdstring add_text = "%";
	add_text += item;
	mEditFocusCtrl->ReplaceSel(add_text.win_str(), true);
	mEditFocusCtrl->SetFocus();
}

void CPrefsEditCaption::OnCaptionMyName() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eMe]);
}

void CPrefsEditCaption::OnCaptionMyEmail() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eMeEmail]);
}

void CPrefsEditCaption::OnCaptionMyFull() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eMeFull]);
}

void CPrefsEditCaption::OnCaptionMyFirst() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eMeFirst]);
}

void CPrefsEditCaption::OnCaptionSmartName() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eSmart]);
}

void CPrefsEditCaption::OnCaptionSmartEmail() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eSmartEmail]);
}

void CPrefsEditCaption::OnCaptionSmartFull() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eSmartFull]);
}

void CPrefsEditCaption::OnCaptionSmartFirst() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eSmartFirst]);
}

void CPrefsEditCaption::OnCaptionFromName() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eFrom]);
}

void CPrefsEditCaption::OnCaptionFromEmail() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eFromEmail]);
}

void CPrefsEditCaption::OnCaptionFromFull() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eFromFull]);
}

void CPrefsEditCaption::OnCaptionFromFirst() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eFromFirst]);
}

void CPrefsEditCaption::OnCaptionToName() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eTo]);
}

void CPrefsEditCaption::OnCaptionToEmail() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eToEmail]);
}

void CPrefsEditCaption::OnCaptionToFull() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eToFull]);
}

void CPrefsEditCaption::OnCaptionToFirst() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eToFirst]);
}

void CPrefsEditCaption::OnCaptionCCName() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eCC]);
}

void CPrefsEditCaption::OnCaptionCCEmail() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eCCEmail]);
}

void CPrefsEditCaption::OnCaptionCCFull() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eCCFull]);
}

void CPrefsEditCaption::OnCaptionCCFirst() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eCCFirst]);
}

void CPrefsEditCaption::OnCaptionSubject() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eSubject]);
}

void CPrefsEditCaption::OnCaptionSentShort() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eDateSentShort]);
}

void CPrefsEditCaption::OnCaptionSentLong() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eDateSentFull]);
}

void CPrefsEditCaption::OnCaptionDateNow() 
{
	AddCaptionItem(cCaptions[CCaptionParser::eDateNow]);
}

void CPrefsEditCaption::OnCaptionPage() 
{
	AddCaptionItem(cCaptions[CCaptionParser::ePage]);
}

void CPrefsEditCaption::OnCaptionRevert() 
{
	SetData(mText1Item, mText2Item, mText3Item, mText4Item, mCursorTopItem, mBox1Item, mBox2Item, mSummaryItem);
}

void CPrefsEditCaption::OnCaptionMessage1()
{
	UpdateDisplay1();
	SetDisplay1(true);
	showing1 = true;
}

void CPrefsEditCaption::OnCaptionDraft1()
{
	UpdateDisplay1();
	SetDisplay1(false);
	showing1 = false;
}

void CPrefsEditCaption::OnCaptionMessage2()
{
	UpdateDisplay2();
	SetDisplay2(true);
	showing3 = true;
}

void CPrefsEditCaption::OnCaptionDraft2()
{
	UpdateDisplay2();
	SetDisplay2(false);
	showing3 = false;
}

// Set text in editor
void CPrefsEditCaption::SetData(CPreferenceValueMap<cdstring>* txt1,
								CPreferenceValueMap<cdstring>* txt2,
								CPreferenceValueMap<cdstring>* txt3,
								CPreferenceValueMap<cdstring>* txt4,
								CPreferenceValueMap<bool>* cursor_top,
								CPreferenceValueMap<bool>* use_box1,
								CPreferenceValueMap<bool>* use_box2,
								CPreferenceValueMap<bool>* summary)
{
	// Cache ptr to prefs item
	mText1Item = txt1;
	mText2Item = txt2;
	mText3Item = txt3;
	mText4Item = txt4;
	mCursorTopItem = cursor_top;
	mBox1Item = use_box1;
	mBox2Item = use_box2;
	mSummaryItem = summary;

	// Set box item if present
	if (mBox1Item && mBox2Item)
	{
		mUseBox1Ctrl.SetCheck(mBox1Item->GetValue());
		mUseBox2Ctrl.SetCheck(mBox2Item->GetValue());
		mCursorTopCtrl.ShowWindow(SW_HIDE);
	}
	else
	{
		mCursorTopCtrl.SetCheck(mCursorTopItem->GetValue());
		mPageBtn.ShowWindow(SW_HIDE);
		mUseBox1Ctrl.ShowWindow(SW_HIDE);
		mUseBox2Ctrl.ShowWindow(SW_HIDE);
	}

	// Set summary item if present
	if (mSummaryItem)
		mSummaryCtrl.SetCheck(mSummaryItem->GetValue());
	else
		mSummaryCtrl.ShowWindow(SW_HIDE);

	// Keep copy of text
	text1 = mText1Item->GetValue();
	if (mText2Item)
	{
		text2 = mText2Item->GetValue();
		mMessage1Ctrl.SetCheck(true);
	}
	else
	{
		mMessage1Ctrl.ShowWindow(SW_HIDE);
		mDraft1Ctrl.ShowWindow(SW_HIDE);
	}
	text3 = mText3Item->GetValue();
	if (mText4Item)
	{
		text4 = mText4Item->GetValue();
		mMessage2Ctrl.SetCheck(true);
	}
	else
	{
		mMessage2Ctrl.ShowWindow(SW_HIDE);
		mDraft2Ctrl.ShowWindow(SW_HIDE);
	}

	// Update items
	showing1 = true;
	showing3 = true;
	SetDisplay2(true);
	SetDisplay1(true);
}

// Get text from editor
void CPrefsEditCaption::GetEditorText(void)
{
	// Get box item if present
	if (mCursorTopItem)
		mCursorTopItem->SetValue(mCursorTopCtrl.GetCheck());
	if (mBox1Item)
		mBox1Item->SetValue(mUseBox1Ctrl.GetCheck());
	if (mBox2Item)
		mBox2Item->SetValue(mUseBox2Ctrl.GetCheck());
	if (mSummaryItem)
		mSummaryItem->SetValue(mSummaryCtrl.GetCheck());

	// Update existing text
	UpdateDisplay1();
	UpdateDisplay2();

	// Copy info from panel into prefs
	mText1Item->SetValue(text1);
	if (mText2Item)
		mText2Item->SetValue(text2);
	mText3Item->SetValue(text3);
	if (mText4Item)
		mText4Item->SetValue(text4);
}

void CPrefsEditCaption::SetDisplay1(bool show1)
{
	cdstring& item = (show1 ? text1 : text2);

	// Put text into editor
	if (item.empty())
		CUnicodeUtils::SetWindowTextUTF8(&mEditHeaderCtrl, cdstring::null_str);
	else
		CUnicodeUtils::SetWindowTextUTF8(&mEditHeaderCtrl, item);

	// Set selection at start
	mEditHeaderCtrl.SetFocus();
	mEditHeaderCtrl.SetSel(0, 0);
}

void CPrefsEditCaption::UpdateDisplay1()
{
	cdstring& item = (showing1 ? text1 : text2);

	// Copy info from panel into prefs
	item = CUnicodeUtils::GetWindowTextUTF8(&mEditHeaderCtrl);
}

void CPrefsEditCaption::SetDisplay2(bool show3)
{
	cdstring& item = (show3 ? text3 : text4);

	// Put text into editor
	if (item.empty())
		CUnicodeUtils::SetWindowTextUTF8(&mEditFooterCtrl, cdstring::null_str);
	else
		CUnicodeUtils::SetWindowTextUTF8(&mEditFooterCtrl, item);

	// Set selection at start
	mEditFooterCtrl.SetFocus();
	mEditFooterCtrl.SetSel(0, 0);
}

void CPrefsEditCaption::UpdateDisplay2()
{
	cdstring& item = (showing3 ? text3 : text4);

	// Copy info from panel into prefs
	item = CUnicodeUtils::GetWindowTextUTF8(&mEditFooterCtrl);
}

void CPrefsEditCaption::OnHeaderSetFocus()
{
	mEditFocusCtrl = &mEditHeaderCtrl;
}

void CPrefsEditCaption::OnFooterSetFocus()
{
	mEditFocusCtrl = &mEditFooterCtrl;
}