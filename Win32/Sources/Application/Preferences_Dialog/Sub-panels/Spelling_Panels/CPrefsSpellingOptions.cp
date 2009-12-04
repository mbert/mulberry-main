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


// CPrefsSpellingOptions.cpp : implementation file
//

#include "CPrefsSpellingOptions.h"

#include "CPreferences.h"
#include "CSpellPlugin.h"


/////////////////////////////////////////////////////////////////////////////
// CPrefsSpellingOptions dialog

IMPLEMENT_DYNAMIC(CPrefsSpellingOptions, CTabPanel)

CPrefsSpellingOptions::CPrefsSpellingOptions()
	: CTabPanel(CPrefsSpellingOptions::IDD)
{
	//{{AFX_DATA_INIT(CPrefsSpellingOptions)
	mCaseSensitive = FALSE;
	mIgnoreCapitalised = FALSE;
	mIgnoreAllCaps = FALSE;
	mIgnoreWordsNumbers = FALSE;
	mIgnoreMixedCase = FALSE;
	mIgnoreDomainNames = FALSE;
	mReportDoubledWords = FALSE;
	mAutoCorrect = FALSE;
	mAutoPositionDialog = FALSE;
	mSpellOnSend = FALSE;
	//}}AFX_DATA_INIT
}


void CPrefsSpellingOptions::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsSpellingOptions)
	DDX_Check(pDX, IDC_SPELLOPTIONS_CASE, mCaseSensitive);
	DDX_Check(pDX, IDC_SPELLOPTIONS_ICAP, mIgnoreCapitalised);
	DDX_Check(pDX, IDC_SPELLOPTIONS_ACAP, mIgnoreAllCaps);
	DDX_Check(pDX, IDC_SPELLOPTIONS_IWNO, mIgnoreWordsNumbers);
	DDX_Check(pDX, IDC_SPELLOPTIONS_IMIX, mIgnoreMixedCase);
	DDX_Check(pDX, IDC_SPELLOPTIONS_IDNS, mIgnoreDomainNames);
	DDX_Check(pDX, IDC_SPELLOPTIONS_RTWO, mReportDoubledWords);
	DDX_Check(pDX, IDC_SPELLOPTIONS_AUTO, mAutoCorrect);
	DDX_Check(pDX, IDC_SPELLOPTIONS_AUTOPOS, mAutoPositionDialog);
	DDX_Check(pDX, IDC_SPELLOPTIONS_SPELLONSEND, mSpellOnSend);
	DDX_Check(pDX, IDC_SPELLOPTIONS_SPELLASYOUTYPE, mSpellAsYouType);
	DDX_Check(pDX, IDC_SPELLOPTIONS_SPELLBACKGROUND, mSpellColourBackground);
	//}}AFX_DATA_MAP
	
	if (pDX->m_bSaveAndValidate)
		mBackgroundColour = mBackgroundColourCtrl.GetColor();
}


BEGIN_MESSAGE_MAP(CPrefsSpellingOptions, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsSpellingOptions)
	ON_COMMAND(IDC_SPELLOPTIONS_SPELLBACKGROUND, OnSpellColourBackground)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsSpellingOptions message handlers

BOOL CPrefsSpellingOptions::OnInitDialog() 
{
	CTabPanel::OnInitDialog();
	
	mBackgroundColourCtrl.SubclassDlgItem(IDC_SPELLOPTIONS_SPELLCOLOUR, this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Set data
void CPrefsSpellingOptions::SetContent(void* data)
{
	CSpellPlugin* speller = static_cast<CSpellPlugin*>(data);

	mCaseSensitive = speller->GetPreferences()->mCaseSensitive.GetValue();
	mIgnoreCapitalised = speller->GetPreferences()->mIgnoreCapitalised.GetValue();
	mIgnoreAllCaps = speller->GetPreferences()->mIgnoreAllCaps.GetValue();
	mIgnoreWordsNumbers = speller->GetPreferences()->mIgnoreWordsNumbers.GetValue();
	mIgnoreMixedCase = speller->GetPreferences()->mIgnoreMixedCase.GetValue();
	mIgnoreDomainNames = speller->GetPreferences()->mIgnoreDomainNames.GetValue();
	mReportDoubledWords = speller->GetPreferences()->mReportDoubledWords.GetValue();
	mAutoCorrect = speller->GetPreferences()->mAutoCorrect.GetValue();

	mAutoPositionDialog = speller->GetPreferences()->mAutoPositionDialog.GetValue();
	mSpellOnSend = speller->GetPreferences()->mSpellOnSend.GetValue();
	mSpellAsYouType = speller->GetPreferences()->mSpellAsYouType.GetValue();
	mSpellColourBackground = speller->GetPreferences()->mSpellColourBackground.GetValue();
	mBackgroundColourCtrl.SetColor(speller->GetPreferences()->mSpellBackgroundColour.GetValue());
	if (!mSpellColourBackground)
		mBackgroundColourCtrl.EnableWindow(false);
}

// Force update of data
bool CPrefsSpellingOptions::UpdateContent(void* data)
{
	CSpellPlugin* speller = static_cast<CSpellPlugin*>(data);

	// Get values
	speller->GetPreferences()->mCaseSensitive.SetValue(mCaseSensitive);
	speller->GetPreferences()->mIgnoreCapitalised.SetValue(mIgnoreCapitalised);
	speller->GetPreferences()->mIgnoreAllCaps.SetValue(mIgnoreAllCaps);
	speller->GetPreferences()->mIgnoreWordsNumbers.SetValue(mIgnoreWordsNumbers);
	speller->GetPreferences()->mIgnoreMixedCase.SetValue(mIgnoreMixedCase);
	speller->GetPreferences()->mIgnoreDomainNames.SetValue(mIgnoreDomainNames);
	speller->GetPreferences()->mReportDoubledWords.SetValue(mReportDoubledWords);
	speller->GetPreferences()->mAutoCorrect.SetValue(mAutoCorrect);

	speller->GetPreferences()->mAutoPositionDialog.SetValue(mAutoPositionDialog);
	speller->GetPreferences()->mSpellOnSend.SetValue(mSpellOnSend);
	speller->GetPreferences()->mSpellAsYouType.SetValue(mSpellAsYouType);
	speller->GetPreferences()->mSpellColourBackground.SetValue(mSpellColourBackground);
	speller->GetPreferences()->mSpellBackgroundColour.SetValue(mBackgroundColour);
	
	return true;
}

void CPrefsSpellingOptions::OnSpellColourBackground() 
{
	mBackgroundColourCtrl.EnableWindow(static_cast<CButton*>(GetDlgItem(IDC_SPELLOPTIONS_SPELLBACKGROUND))->GetCheck());
}

