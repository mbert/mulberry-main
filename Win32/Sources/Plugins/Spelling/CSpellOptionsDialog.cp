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

// CSpellOptionsDialog.cpp : implementation file
//

#include "CSpellOptionsDialog.h"

#include "CSDIFrame.h"
#include "CSpellPlugin.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CSpellOptionsDialog dialog


CSpellOptionsDialog::CSpellOptionsDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CSpellOptionsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSpellOptionsDialog)
	mIgnoreCapitalised = FALSE;
	mIgnoreAllCaps = FALSE;
	mIgnoreWordsNumbers = FALSE;
	mIgnoreMixedCase = FALSE;
	mIgnoreDomainNames = FALSE;
	mReportDoubledWords = FALSE;
	mCaseSensitive = FALSE;
	mPhoneticSuggestions = FALSE;
	mTypoSuggestions = FALSE;
	mSuggestSplit = FALSE;
	mAutoCorrect = FALSE;
	mAutoPositionDialog = FALSE;
	mSpellOnSend = FALSE;
	mSuggestFast = 0;
	//}}AFX_DATA_INIT
}


void CSpellOptionsDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpellOptionsDialog)
	DDX_Check(pDX, IDC_SPELLOPTIONS_ICAP, mIgnoreCapitalised);
	DDX_Check(pDX, IDC_SPELLOPTIONS_ACAP, mIgnoreAllCaps);
	DDX_Check(pDX, IDC_SPELLOPTIONS_IWNO, mIgnoreWordsNumbers);
	DDX_Check(pDX, IDC_SPELLOPTIONS_IMIX, mIgnoreMixedCase);
	DDX_Check(pDX, IDC_SPELLOPTIONS_IDNS, mIgnoreDomainNames);
	DDX_Check(pDX, IDC_SPELLOPTIONS_RTWO, mReportDoubledWords);
	DDX_Check(pDX, IDC_SPELLOPTIONS_CASE, mCaseSensitive);
	DDX_Check(pDX, IDC_SPELLOPTIONS_SPHO, mPhoneticSuggestions);
	DDX_Check(pDX, IDC_SPELLOPTIONS_STYP, mTypoSuggestions);
	DDX_Check(pDX, IDC_SPELLOPTIONS_SSPL, mSuggestSplit);
	DDX_Check(pDX, IDC_SPELLOPTIONS_AUTO, mAutoCorrect);
	DDX_Check(pDX, IDC_SPELLOPTIONS_AUTOPOS, mAutoPositionDialog);
	DDX_Check(pDX, IDC_SPELLOPTIONS_SPELLONSEND, mSpellOnSend);
	DDX_Check(pDX, IDC_SPELLOPTIONS_SPELLASYOUTYPE, mSpellAsYouType);
	DDX_Check(pDX, IDC_SPELLOPTIONS_SPELLBACKGROUND, mSpellColourBackground);
	DDX_Radio(pDX, IDC_SPELLOPTIONS_FAST, mSuggestFast);
	//}}AFX_DATA_MAP
	
	if (pDX->m_bSaveAndValidate)
		mBackgroundColour = mBackgroundColourCtrl.GetColor();
}


BEGIN_MESSAGE_MAP(CSpellOptionsDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CSpellOptionsDialog)
	ON_COMMAND_RANGE(IDM_DICTIONARYStart, IDM_DICTIONARYStop, OnChangeDictionaryPopup)
	ON_COMMAND(IDC_SPELLOPTIONS_SPELLBACKGROUND, OnSpellColourBackground)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpellOptionsDialog message handlers

BOOL CSpellOptionsDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass buttons
	mDictionaries.SubclassDlgItem(IDC_SPELLOPTIONS_DICTPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mDictionaries.SetMenu(IDR_POPUP_DICTIONARY);
	InitDictionaries();

	mBackgroundColourCtrl.SubclassDlgItem(IDC_SPELLOPTIONS_SPELLCOLOUR, this);
	mBackgroundColourCtrl.SetColor(mBackgroundColour);
	if (!mSpellColourBackground)
		mBackgroundColourCtrl.EnableWindow(false);

	return true;
}

// Set the options
void CSpellOptionsDialog::SetOptions(CSpellPlugin* speller)
{
	mIgnoreCapitalised = speller->GetPreferences()->mIgnoreCapitalised.GetValue();
	mIgnoreAllCaps = speller->GetPreferences()->mIgnoreAllCaps.GetValue();
	mIgnoreWordsNumbers = speller->GetPreferences()->mIgnoreWordsNumbers.GetValue();
	mIgnoreMixedCase = speller->GetPreferences()->mIgnoreMixedCase.GetValue();
	mIgnoreDomainNames = speller->GetPreferences()->mIgnoreDomainNames.GetValue();
	mReportDoubledWords = speller->GetPreferences()->mReportDoubledWords.GetValue();
	mCaseSensitive = speller->GetPreferences()->mCaseSensitive.GetValue();
	mPhoneticSuggestions = speller->GetPreferences()->mPhoneticSuggestions.GetValue();
	mTypoSuggestions = speller->GetPreferences()->mTypoSuggestions.GetValue();
	mSuggestSplit = speller->GetPreferences()->mSuggestSplit.GetValue();
	mAutoCorrect = speller->GetPreferences()->mAutoCorrect.GetValue();

	mAutoPositionDialog = speller->GetPreferences()->mAutoPositionDialog.GetValue();
	mSpellOnSend = speller->GetPreferences()->mSpellOnSend.GetValue();
	mSpellAsYouType = speller->GetPreferences()->mSpellAsYouType.GetValue();
	mSpellColourBackground = speller->GetPreferences()->mSpellColourBackground.GetValue();
	mBackgroundColour = speller->GetPreferences()->mSpellBackgroundColour.GetValue();

	mSuggestFast = speller->GetPreferences()->mSuggestLevel.GetValue() - CSpellPreferencesDLL::eSuggest_Fast;
	
	mSpeller = speller;
}

// Get the options
void CSpellOptionsDialog::GetOptions(CSpellPlugin* speller)
{
	speller->GetPreferences()->mIgnoreCapitalised.SetValue(mIgnoreCapitalised);
	speller->GetPreferences()->mIgnoreAllCaps.SetValue(mIgnoreAllCaps);
	speller->GetPreferences()->mIgnoreWordsNumbers.SetValue(mIgnoreWordsNumbers);
	speller->GetPreferences()->mIgnoreMixedCase.SetValue(mIgnoreMixedCase);
	speller->GetPreferences()->mIgnoreDomainNames.SetValue(mIgnoreDomainNames);
	speller->GetPreferences()->mReportDoubledWords.SetValue(mReportDoubledWords);
	speller->GetPreferences()->mCaseSensitive.SetValue(mCaseSensitive);
	speller->GetPreferences()->mPhoneticSuggestions.SetValue(mPhoneticSuggestions);
	speller->GetPreferences()->mTypoSuggestions.SetValue(mTypoSuggestions);
	speller->GetPreferences()->mSuggestSplit.SetValue(mSuggestSplit);
	speller->GetPreferences()->mAutoCorrect.SetValue(mAutoCorrect);

	speller->GetPreferences()->mAutoPositionDialog.SetValue(mAutoPositionDialog);
	speller->GetPreferences()->mSpellOnSend.SetValue(mSpellOnSend);
	speller->GetPreferences()->mSpellAsYouType.SetValue(mSpellAsYouType);
	speller->GetPreferences()->mSpellColourBackground.SetValue(mSpellColourBackground);
	speller->GetPreferences()->mSpellBackgroundColour.SetValue(mBackgroundColour);

	switch(mSuggestFast)
	{
	case 0:
		speller->GetPreferences()->mSuggestLevel.SetValue(CSpellPreferencesDLL::eSuggest_Fast);
		break;
	case 1:
		speller->GetPreferences()->mSuggestLevel.SetValue(CSpellPreferencesDLL::eSuggest_Medium);
		break;
	case 2:
		speller->GetPreferences()->mSuggestLevel.SetValue(CSpellPreferencesDLL::eSuggest_Slow);
		break;
	}
	
	CString item_title;
	mDictionaries.GetPopupMenu()->GetMenuString(mDictionaries.GetValue(), item_title, MF_BYCOMMAND);
	speller->GetPreferences()->mDictionaryName.SetValue(cdstring(item_title));

	speller->UpdatePreferences();
}

// Set dictionary popup
void CSpellOptionsDialog::InitDictionaries()
{
	CMenu* pPopup = mDictionaries.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);

	// Add each dictionary
	const char* names;
	if (mSpeller->GetDictionaries(&names))
	{
		const char** name = (const char**)(names);
		int menu_id = IDM_DICTIONARYStart;
		while(*name)
			CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id++, *name++);
	}
	mDictionaries.SetValue(IDM_DICTIONARYStart);

	// Find item that matches
	num_menu = pPopup->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
	{
		cdstring item_title = CUnicodeUtils::GetMenuStringUTF8(pPopup, i, MF_BYPOSITION);
		if (item_title == mSpeller->GetPreferences()->mDictionaryName.GetValue())
		{
			mDictionaries.SetValue(i + IDM_DICTIONARYStart);
			break;
		}
	}
}

void CSpellOptionsDialog::OnChangeDictionaryPopup(UINT nID) 
{
	mDictionaries.SetValue(nID);
}

void CSpellOptionsDialog::OnSpellColourBackground() 
{
	mBackgroundColourCtrl.EnableWindow(static_cast<CButton*>(GetDlgItem(IDC_SPELLOPTIONS_SPELLBACKGROUND))->GetCheck());
}

bool CSpellOptionsDialog::PoseDialog(CSpellPlugin* speller)
{
	bool result = false;

	// Create the dialog
	CSpellOptionsDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetOptions(speller);

	if (dlog.DoModal() == IDOK)
	{
		dlog.GetOptions(speller);
		result = true;
	}

	return result;
}