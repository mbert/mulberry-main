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


// CPrefsLetter.cpp : implementation file
//


#include "CPrefsLetter.h"

#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsLetterGeneral.h"
#include "CPrefsLetterOptions.h"
#include "CPrefsLetterStyled.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsLetter dialog

IMPLEMENT_DYNCREATE(CPrefsLetter, CPrefsPanel)

CPrefsLetter::CPrefsLetter() : CPrefsPanel(CPrefsLetter::IDD)
{
	//{{AFX_DATA_INIT(CPrefsLetter)
	//record_attachments = FALSE;
	//}}AFX_DATA_INIT
}

CPrefsLetter::~CPrefsLetter()
{
}

void CPrefsLetter::DoDataExchange(CDataExchange* pDX)
{
	CPrefsPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsLetter)
	mTabs.DoDataExchange(pDX);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsLetter, CPrefsPanel)
	//{{AFX_MSG_MAP(CPrefsLetter)
	ON_NOTIFY(TCN_SELCHANGE, IDC_LETTER_TABS, OnSelChangeLetterTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Set up params for DDX
void CPrefsLetter::SetPrefs(CPreferences* prefs)
{
	mCopyPrefs = prefs;
	mTabs.SetContent(prefs);
}

// Get params from DDX
void CPrefsLetter::UpdatePrefs(CPreferences* prefs)
{
	// Get values
	mTabs.UpdateContent(prefs);
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsLetter message handlers

void CPrefsLetter::InitControls(void)
{
	// Subclass buttons
	mTabs.SubclassDlgItem(IDC_LETTER_TABS, this);

	// Create tab panels
	CPrefsLetterGeneral* letter_general = new CPrefsLetterGeneral;
	mTabs.AddPanel(letter_general);

	CPrefsLetterOptions* letter_options = new CPrefsLetterOptions;
	mTabs.AddPanel(letter_options);

	CPrefsLetterStyled* letter_styled = new CPrefsLetterStyled;
	mTabs.AddPanel(letter_styled);
}

void CPrefsLetter::SetControls(void)
{
	// Give data to tab panels
	mTabs.SetContent(mCopyPrefs);
	mTabs.SetPanel(0);
}

void CPrefsLetter::OnSelChangeLetterTabs(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	mTabs.SetPanel(mTabs.GetCurSel());

	*pResult = 0;
}
