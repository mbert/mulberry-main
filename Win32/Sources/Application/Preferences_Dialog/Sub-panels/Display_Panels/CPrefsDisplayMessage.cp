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


// CPrefsDisplayMessage.cpp : implementation file
//

#include "CPrefsDisplayMessage.h"

#include "CPreferences.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplayMessage dialog

IMPLEMENT_DYNAMIC(CPrefsDisplayMessage, CTabPanel)

CPrefsDisplayMessage::CPrefsDisplayMessage()
	: CTabPanel(CPrefsDisplayMessage::IDD)
{
	//{{AFX_DATA_INIT(CPrefsDisplayMessage)
	mUnseenBold = FALSE;
	mSeenBold = FALSE;
	mAnsweredBold = FALSE;
	mImportantBold = FALSE;
	mDeletedBold = FALSE;
	mUnseenItalic = FALSE;
	mSeenItalic = FALSE;
	mAnsweredItalic = FALSE;
	mImportantItalic = FALSE;
	mDeletedItalic = FALSE;
	mUnseenStrike = FALSE;
	mSeenStrike = FALSE;
	mAnsweredStrike = FALSE;
	mImportantStrike = FALSE;
	mDeletedStrike = FALSE;
	mUnseenUnderline = FALSE;
	mSeenUnderline = FALSE;
	mAnsweredUnderline = FALSE;
	mImportantUnderline = FALSE;
	mDeletedUnderline = FALSE;
	mMultiAddressBold = FALSE;
	mMultiAddressItalic = FALSE;
	mMultiAddressStrike = FALSE;
	mMultiAddressUnderline = FALSE;
	mUseMatch = FALSE;
	mUseNonMatch = FALSE;
	mUseLocalTimezone = FALSE;
	//}}AFX_DATA_INIT
}


void CPrefsDisplayMessage::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsDisplayMessage)
	DDX_Check(pDX, IDC_MSG_UNSEENBOLD, mUnseenBold);
	DDX_Check(pDX, IDC_MSG_SEENBOLD, mSeenBold);
	DDX_Check(pDX, IDC_MSG_ANSWEREDBOLD, mAnsweredBold);
	DDX_Check(pDX, IDC_MSG_IMPORTANTBOLD, mImportantBold);
	DDX_Check(pDX, IDC_MSG_DELETEDBOLD, mDeletedBold);
	DDX_Check(pDX, IDC_MSG_UNSEENITALIC, mUnseenItalic);
	DDX_Check(pDX, IDC_MSG_SEENITALIC, mSeenItalic);
	DDX_Check(pDX, IDC_MSG_ANSWEREDITALIC, mAnsweredItalic);
	DDX_Check(pDX, IDC_MSG_IMPORTANTITALIC, mImportantItalic);
	DDX_Check(pDX, IDC_MSG_DELETEDITALIC, mDeletedItalic);
	DDX_Check(pDX, IDC_MSG_UNSEENSTRIKE, mUnseenStrike);
	DDX_Check(pDX, IDC_MSG_SEENSTRIKE, mSeenStrike);
	DDX_Check(pDX, IDC_MSG_ANSWEREDSTRIKE, mAnsweredStrike);
	DDX_Check(pDX, IDC_MSG_IMPORTANTSTRIKE, mImportantStrike);
	DDX_Check(pDX, IDC_MSG_DELETEDSTRIKE, mDeletedStrike);
	DDX_Check(pDX, IDC_MSG_UNSEENUNDER, mUnseenUnderline);
	DDX_Check(pDX, IDC_MSG_SEENUNDER, mSeenUnderline);
	DDX_Check(pDX, IDC_MSG_ANSWEREDUNDER, mAnsweredUnderline);
	DDX_Check(pDX, IDC_MSG_IMPORTANTUNDER, mImportantUnderline);
	DDX_Check(pDX, IDC_MSG_DELETEDUNDER, mDeletedUnderline);
	DDX_Check(pDX, IDC_MSG_MULTIBOLD, mMultiAddressBold);
	DDX_Check(pDX, IDC_MSG_MULTIITALIC, mMultiAddressItalic);
	DDX_Check(pDX, IDC_MSG_MULTISTRIKE, mMultiAddressStrike);
	DDX_Check(pDX, IDC_MSG_MULTIUNDER, mMultiAddressUnderline);
	DDX_Check(pDX, IDC_MSG_USEMATCH, mUseMatch);
	DDX_Check(pDX, IDC_MSG_USENONMATCH, mUseNonMatch);
	DDX_Control(pDX, IDC_MSG_USEMATCH, mUseMatchBtn);
	DDX_Control(pDX, IDC_MSG_USENONMATCH, mUseNonMatchBtn);
	DDX_Check(pDX, IDC_MSG_USELOCALTIMEZONE, mUseLocalTimezone);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsDisplayMessage, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsDisplayMessage)
	ON_BN_CLICKED(IDC_MSG_USENONMATCH, OnUseNonMatch)
	ON_BN_CLICKED(IDC_MSG_USEMATCH, OnUseMatch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplayMessage message handlers

BOOL CPrefsDisplayMessage::OnInitDialog() 
{
	CTabPanel::OnInitDialog();
	
	// Subclass buttons
	mUnseenColour.SubclassDlgItem(IDC_MSG_UNSEENCOLOUR, this);
	mSeenColour.SubclassDlgItem(IDC_MSG_SEENCOLOUR, this);
	mAnsweredColour.SubclassDlgItem(IDC_MSG_ANSWEREDCOLOUR, this);
	mImportantColour.SubclassDlgItem(IDC_MSG_IMPORTANTCOLOUR, this);
	mDeletedColour.SubclassDlgItem(IDC_MSG_DELETEDCOLOUR, this);
	mMatchColour.SubclassDlgItem(IDC_MSG_MATCHCOLOUR, this);
	mNonMatchColour.SubclassDlgItem(IDC_MSG_NONMATCHCOLOUR, this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Set data
void CPrefsDisplayMessage::SetContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	mUnseenColour.SetColor(copyPrefs->unseen.GetValue().color);
	mSeenColour.SetColor(copyPrefs->seen.GetValue().color);
	mAnsweredColour.SetColor(copyPrefs->answered.GetValue().color);
	mImportantColour.SetColor(copyPrefs->important.GetValue().color);
	mDeletedColour.SetColor(copyPrefs->deleted.GetValue().color);

	mUnseenBold = (copyPrefs->unseen.GetValue().style & bold) ? 1 : 0;
	mSeenBold = (copyPrefs->seen.GetValue().style & bold) ? 1 : 0;
	mAnsweredBold = (copyPrefs->answered.GetValue().style & bold) ? 1 : 0;
	mImportantBold = (copyPrefs->important.GetValue().style & bold) ? 1 : 0;
	mDeletedBold = (copyPrefs->deleted.GetValue().style & bold) ? 1 : 0;

	mUnseenItalic = (copyPrefs->unseen.GetValue().style & italic) ? 1 : 0;
	mSeenItalic = (copyPrefs->seen.GetValue().style & italic) ? 1 : 0;
	mAnsweredItalic = (copyPrefs->answered.GetValue().style & italic) ? 1 : 0;
	mImportantItalic = (copyPrefs->important.GetValue().style & italic) ? 1 : 0;
	mDeletedItalic = (copyPrefs->deleted.GetValue().style & italic) ? 1 : 0;

	mUnseenStrike = (copyPrefs->unseen.GetValue().style & strike_through) ? 1 : 0;
	mSeenStrike = (copyPrefs->seen.GetValue().style & strike_through) ? 1 : 0;
	mAnsweredStrike = (copyPrefs->answered.GetValue().style & strike_through) ? 1 : 0;
	mImportantStrike = (copyPrefs->important.GetValue().style & strike_through) ? 1 : 0;
	mDeletedStrike = (copyPrefs->deleted.GetValue().style & strike_through) ? 1 : 0;

	mUnseenUnderline = (copyPrefs->unseen.GetValue().style & underline) ? 1 : 0;
	mSeenUnderline = (copyPrefs->seen.GetValue().style & underline) ? 1 : 0;
	mAnsweredUnderline = (copyPrefs->answered.GetValue().style & underline) ? 1 : 0;
	mImportantUnderline = (copyPrefs->important.GetValue().style & underline) ? 1 : 0;
	mDeletedUnderline = (copyPrefs->deleted.GetValue().style & underline) ? 1 : 0;

	mMultiAddressBold = (copyPrefs->mMultiAddress.GetValue() & bold) ? 1 : 0;
	mMultiAddressItalic = (copyPrefs->mMultiAddress.GetValue() & italic) ? 1 : 0;
	mMultiAddressStrike = (copyPrefs->mMultiAddress.GetValue() & strike_through) ? 1 : 0;
	mMultiAddressUnderline = (copyPrefs->mMultiAddress.GetValue() & underline) ? 1 : 0;

	mMatchColour.SetColor(copyPrefs->mMatch.GetValue().color);
	mUseMatch = copyPrefs->mMatch.GetValue().style;
	if (!mUseMatch)
		mMatchColour.EnableWindow(false);
	mNonMatchColour.SetColor(copyPrefs->mNonMatch.GetValue().color);
	mUseNonMatch = copyPrefs->mNonMatch.GetValue().style;
	if (!mUseNonMatch)
		mNonMatchColour.EnableWindow(false);
	mUseLocalTimezone = copyPrefs->mUseLocalTimezone.GetValue();
}

// Force update of data
bool CPrefsDisplayMessage::UpdateContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Get values
	SStyleTraits mtraits;
	mtraits.color = mUnseenColour.GetColor();
	mtraits.style = 0;
	mtraits.style |= (mUnseenBold ? bold : 0);
	mtraits.style |= (mUnseenItalic ? italic : 0);
	mtraits.style |= (mUnseenStrike ? strike_through : 0);
	mtraits.style |= (mUnseenUnderline ? underline : 0);
	copyPrefs->unseen.SetValue(mtraits);

	mtraits.color = mSeenColour.GetColor();
	mtraits.style = 0;
	mtraits.style |= (mSeenBold ? bold : 0);
	mtraits.style |= (mSeenItalic ? italic : 0);
	mtraits.style |= (mSeenStrike ? strike_through : 0);
	mtraits.style |= (mSeenUnderline ? underline : 0);
	copyPrefs->seen.SetValue(mtraits);

	mtraits.color = mAnsweredColour.GetColor();
	mtraits.style = 0;
	mtraits.style |= (mAnsweredBold ? bold : 0);
	mtraits.style |= (mAnsweredItalic ? italic : 0);
	mtraits.style |= (mAnsweredStrike ? strike_through : 0);
	mtraits.style |= (mAnsweredUnderline ? underline : 0);
	copyPrefs->answered.SetValue(mtraits);

	mtraits.color = mImportantColour.GetColor();
	mtraits.style = 0;
	mtraits.style |= (mImportantBold ? bold : 0);
	mtraits.style |= (mImportantItalic ? italic : 0);
	mtraits.style |= (mImportantStrike ? strike_through : 0);
	mtraits.style |= (mImportantUnderline ? underline : 0);
	copyPrefs->important.SetValue(mtraits);

	mtraits.color = mDeletedColour.GetColor();
	mtraits.style = 0;
	mtraits.style |= (mDeletedBold ? bold : 0);
	mtraits.style |= (mDeletedItalic ? italic : 0);
	mtraits.style |= (mDeletedStrike ? strike_through : 0);
	mtraits.style |= (mDeletedUnderline ? underline : 0);
	copyPrefs->deleted.SetValue(mtraits);

	long style = 0;
	style |= (mMultiAddressBold ? bold : 0);
	style |= (mMultiAddressItalic ? italic : 0);
	style |= (mMultiAddressStrike ? strike_through : 0);
	style |= (mMultiAddressUnderline ? underline : 0);
	copyPrefs->mMultiAddress.SetValue(style);

	mtraits.color = mMatchColour.GetColor();
	mtraits.style = mUseMatch;
	copyPrefs->mMatch.SetValue(mtraits);

	mtraits.color = mNonMatchColour.GetColor();
	mtraits.style = mUseNonMatch;
	copyPrefs->mNonMatch.SetValue(mtraits);
	
	copyPrefs->mUseLocalTimezone.SetValue(mUseLocalTimezone);

	return true;
}

void CPrefsDisplayMessage::OnUseMatch()
{
	mMatchColour.EnableWindow(mUseMatchBtn.GetCheck());
}

void CPrefsDisplayMessage::OnUseNonMatch()
{
	mNonMatchColour.EnableWindow(mUseNonMatchBtn.GetCheck());
}