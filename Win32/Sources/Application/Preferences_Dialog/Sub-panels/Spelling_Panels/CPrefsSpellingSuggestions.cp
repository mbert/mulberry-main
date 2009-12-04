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


// CPrefsSpellingSuggestions.cpp : implementation file
//

#include "CPrefsSpellingSuggestions.h"

#include "CPreferences.h"
#include "CSpellPlugin.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsSpellingSuggestions dialog

IMPLEMENT_DYNAMIC(CPrefsSpellingSuggestions, CTabPanel)

CPrefsSpellingSuggestions::CPrefsSpellingSuggestions()
	: CTabPanel(CPrefsSpellingSuggestions::IDD)
{
	//{{AFX_DATA_INIT(CPrefsSpellingSuggestions)
	mPhoneticSuggestions = FALSE;
	mTypoSuggestions = FALSE;
	mSuggestSplit = FALSE;
	mSuggestFast = 0;
	//}}AFX_DATA_INIT
}


void CPrefsSpellingSuggestions::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsSpellingSuggestions)
	DDX_Check(pDX, IDC_SPELLOPTIONS_SPHO, mPhoneticSuggestions);
	DDX_Check(pDX, IDC_SPELLOPTIONS_STYP, mTypoSuggestions);
	DDX_Check(pDX, IDC_SPELLOPTIONS_SSPL, mSuggestSplit);
	DDX_Radio(pDX, IDC_SPELLOPTIONS_FAST, mSuggestFast);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsSpellingSuggestions, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsSpellingSuggestions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsSpellingSuggestions message handlers

BOOL CPrefsSpellingSuggestions::OnInitDialog() 
{
	CTabPanel::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Set data
void CPrefsSpellingSuggestions::SetContent(void* data)
{
	CSpellPlugin* speller = static_cast<CSpellPlugin*>(data);

	mPhoneticSuggestions = speller->GetPreferences()->mPhoneticSuggestions.GetValue();
	mTypoSuggestions = speller->GetPreferences()->mTypoSuggestions.GetValue();
	mSuggestSplit = speller->GetPreferences()->mSuggestSplit.GetValue();

	mSuggestFast = speller->GetPreferences()->mSuggestLevel.GetValue() - CSpellPreferencesDLL::eSuggest_Fast;
}

// Force update of data
bool CPrefsSpellingSuggestions::UpdateContent(void* data)
{
	CSpellPlugin* speller = static_cast<CSpellPlugin*>(data);

	// Get values
	speller->GetPreferences()->mPhoneticSuggestions.SetValue(mPhoneticSuggestions);
	speller->GetPreferences()->mTypoSuggestions.SetValue(mTypoSuggestions);
	speller->GetPreferences()->mSuggestSplit.SetValue(mSuggestSplit);
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
	
	return true;
}
