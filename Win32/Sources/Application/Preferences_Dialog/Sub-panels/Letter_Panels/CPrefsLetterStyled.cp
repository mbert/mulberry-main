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


// CPrefsLetterStyled.cpp : implementation file
//

#include "CPrefsLetterStyled.h"

#include "CAdminLock.h"
#include "CPreferences.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsLetterStyled dialog

IMPLEMENT_DYNAMIC(CPrefsLetterStyled, CTabPanel)

CPrefsLetterStyled::CPrefsLetterStyled()
	: CTabPanel(CPrefsLetterStyled::IDD)
{
	//{{AFX_DATA_INIT(CPrefsLetterStyled)
	mEnrichedPlain = FALSE;
	mEnrichedHTML = FALSE;
	mHTMLPlain = FALSE;
	mHTMLEnriched = FALSE;
	mFormatFlowed = FALSE;
	//}}AFX_DATA_INIT
}


void CPrefsLetterStyled::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsLetterStyled)
	DDX_Control(pDX, IDC_ENRICHED_PLAIN, mEnrichedPlainCtrl);
	DDX_Control(pDX, IDC_ENRICHED_HTML, mEnrichedHTMLCtrl);
	DDX_Control(pDX, IDC_HTML_PLAIN, mHTMLPlainCtrl);
	DDX_Control(pDX, IDC_HTML_ENRICHED, mHTMLEnrichedCtrl);

	DDX_Check(pDX, IDC_ENRICHED_PLAIN, mEnrichedPlain);
	DDX_Check(pDX, IDC_ENRICHED_HTML, mEnrichedHTML);
	DDX_Check(pDX, IDC_HTML_PLAIN, mHTMLPlain);
	DDX_Check(pDX, IDC_HTML_ENRICHED, mHTMLEnriched);
	DDX_Check(pDX, IDC_FORMAT_FLOWED, mFormatFlowed);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsLetterStyled, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsLetterStyled)
	ON_COMMAND_RANGE(IDM_DRAFT_CURRENT_PLAIN, IDM_DRAFT_CURRENT_HTML, OnChangeComposeAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsLetterStyled message handlers

BOOL CPrefsLetterStyled::OnInitDialog() 
{
	CTabPanel::OnInitDialog();
	
	// Subclass buttons
	mComposeAsPopup.SubclassDlgItem(IDC_COMPOSEAS, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mComposeAsPopup.SetMenu(IDR_POPUP_COMPOSEAS);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Set data
void CPrefsLetterStyled::SetContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Set values
	if (CAdminLock::sAdminLock.mAllowStyledComposition)
	{
		mComposeAs = copyPrefs->compose_as.GetValue() - 1 + IDM_DRAFT_CURRENT_PLAIN;
		if (mComposeAs > IDM_DRAFT_CURRENT_PLAIN)
			mComposeAs++;
		mEnrichedPlain = copyPrefs->enrMultiAltPlain.GetValue();
		mEnrichedHTML = copyPrefs->enrMultiAltHTML.GetValue();
		mHTMLPlain = copyPrefs->htmlMultiAltPlain.GetValue();
		mHTMLEnriched = copyPrefs->htmlMultiAltEnriched.GetValue();
	}
	mFormatFlowed = copyPrefs->mFormatFlowed.GetValue();

	// set initial control states
	if (CAdminLock::sAdminLock.mAllowStyledComposition)
		mComposeAsPopup.SetValue(mComposeAs);
	else
	{
		mComposeAsPopup.EnableWindow(false);
		mEnrichedPlainCtrl.EnableWindow(false);
		mEnrichedHTMLCtrl.EnableWindow(false);
		mHTMLPlainCtrl.EnableWindow(false);
		mHTMLEnrichedCtrl.EnableWindow(false);
	}
}

// Force update of data
bool CPrefsLetterStyled::UpdateContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Get values
	if (CAdminLock::sAdminLock.mAllowStyledComposition)
	{
		if (mComposeAs > IDM_DRAFT_CURRENT_PLAIN)
			mComposeAs--;
		copyPrefs->compose_as.SetValue((EContentSubType) (mComposeAs - IDM_DRAFT_CURRENT_PLAIN + 1));

		copyPrefs->enrMultiAltPlain.SetValue(mEnrichedPlain);
		copyPrefs->enrMultiAltHTML.SetValue(mEnrichedHTML);
		copyPrefs->htmlMultiAltPlain.SetValue(mHTMLPlain);
		copyPrefs->htmlMultiAltEnriched.SetValue(mHTMLEnriched);
	}
	
	copyPrefs->mFormatFlowed.SetValue(mFormatFlowed);
	
	return true;
}

// Change compose as
void CPrefsLetterStyled::OnChangeComposeAs(UINT nID)
{
	mComposeAs = nID;
	mComposeAsPopup.SetValue(mComposeAs);
}
