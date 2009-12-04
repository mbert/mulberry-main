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


// CPrefsDisplayStyles.cpp : implementation file
//

#include "CPrefsDisplayStyles.h"

#include "CPreferences.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplayStyles dialog

IMPLEMENT_DYNAMIC(CPrefsDisplayStyles, CTabPanel)

CPrefsDisplayStyles::CPrefsDisplayStyles()
	: CTabPanel(CPrefsDisplayStyles::IDD)
{
	//{{AFX_DATA_INIT(CPrefsDisplayStyles)
	mURLBold = FALSE;
	mURLItalic = FALSE;
	mURLUnder = FALSE;
	mURLSeenBold = FALSE;
	mURLSeenItalic = FALSE;
	mURLSeenUnder = FALSE;
	mHeaderBold = FALSE;
	mHeaderItalic = FALSE;
	mHeaderUnder = FALSE;
	mTagBold = FALSE;
	mTagItalic = FALSE;
	mTagUnder = FALSE;
	//}}AFX_DATA_INIT
}


void CPrefsDisplayStyles::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsDisplayStyles)
	DDX_Check(pDX, IDC_URLBOLD, mURLBold);
	DDX_Check(pDX, IDC_URLITALIC, mURLItalic);
	DDX_Check(pDX, IDC_URLUNDER, mURLUnder);
	DDX_Check(pDX, IDC_URLSEENBOLD, mURLSeenBold);
	DDX_Check(pDX, IDC_URLSEENITALIC, mURLSeenItalic);
	DDX_Check(pDX, IDC_URLSEENUNDER, mURLSeenUnder);
	DDX_Check(pDX, IDC_HEADERBOLD, mHeaderBold);
	DDX_Check(pDX, IDC_HEADERITALIC, mHeaderItalic);
	DDX_Check(pDX, IDC_HEADERUNDER, mHeaderUnder);
	DDX_Check(pDX, IDC_TAGBOLD, mTagBold);
	DDX_Check(pDX, IDC_TAGITALIC, mTagItalic);
	DDX_Check(pDX, IDC_TAGUNDER, mTagUnder);
	DDX_UTF8Text(pDX, IDC_RECOGNISEURLS, mRecogniseURLs);
	DDX_Control(pDX, IDC_RECOGNISEURLS, mRecogniseURLsCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsDisplayStyles, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsDisplayStyles)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplayStyles message handlers

BOOL CPrefsDisplayStyles::OnInitDialog() 
{
	CTabPanel::OnInitDialog();
	
	mURLColourCtrl.SubclassDlgItem(IDC_URLCOLOUR, this);
	mURLSeenColourCtrl.SubclassDlgItem(IDC_URLSEENCOLOUR, this);
	mHeaderColourCtrl.SubclassDlgItem(IDC_HEADERCOLOUR, this);
	mTagColourCtrl.SubclassDlgItem(IDC_TAGCOLOUR, this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Set data
void CPrefsDisplayStyles::SetContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Set values
	mURLColourCtrl.SetColor(copyPrefs->mURLStyle.GetValue().color);
	mURLBold = (copyPrefs->mURLStyle.GetValue().style & bold) ? 1 : 0;
	mURLItalic = (copyPrefs->mURLStyle.GetValue().style & italic) ? 1 : 0;
	mURLUnder = (copyPrefs->mURLStyle.GetValue().style & underline) ? 1 : 0;
	mURLSeenColourCtrl.SetColor(copyPrefs->mURLSeenStyle.GetValue().color);
	mURLSeenBold = (copyPrefs->mURLSeenStyle.GetValue().style & bold) ? 1 : 0;
	mURLSeenItalic = (copyPrefs->mURLSeenStyle.GetValue().style & italic) ? 1 : 0;
	mURLSeenUnder = (copyPrefs->mURLSeenStyle.GetValue().style & underline) ? 1 : 0;
	mHeaderColourCtrl.SetColor(copyPrefs->mHeaderStyle.GetValue().color);
	mHeaderBold = (copyPrefs->mHeaderStyle.GetValue().style & bold) ? 1 : 0;
	mHeaderItalic = (copyPrefs->mHeaderStyle.GetValue().style & italic) ? 1 : 0;
	mHeaderUnder = (copyPrefs->mHeaderStyle.GetValue().style & underline) ? 1 : 0;
	mTagColourCtrl.SetColor(copyPrefs->mTagStyle.GetValue().color);
	mTagBold = (copyPrefs->mTagStyle.GetValue().style & bold) ? 1 : 0;
	mTagItalic = (copyPrefs->mTagStyle.GetValue().style & italic) ? 1 : 0;
	mTagUnder = (copyPrefs->mTagStyle.GetValue().style & underline) ? 1 : 0;

	mRecogniseURLs.clear();
	for(short i = 0; i < copyPrefs->mRecognizeURLs.GetValue().size(); i++)
	{
		mRecogniseURLs += copyPrefs->mRecognizeURLs.GetValue()[i] + "\r\n";
	}
}

// Force update of data
bool CPrefsDisplayStyles::UpdateContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Get values
	SStyleTraits mtraits;

	mtraits.color = mURLColourCtrl.GetColor();
	mtraits.style = 0;
	mtraits.style |= (mURLBold ? bold : 0);
	mtraits.style |= (mURLItalic ? italic : 0);
	mtraits.style |= (mURLUnder ? underline : 0);
	copyPrefs->mURLStyle.SetValue(mtraits);

	mtraits.color = mURLSeenColourCtrl.GetColor();
	mtraits.style = 0;
	mtraits.style |= (mURLSeenBold ? bold : 0);
	mtraits.style |= (mURLSeenItalic ? italic : 0);
	mtraits.style |= (mURLSeenUnder ? underline : 0);
	copyPrefs->mURLSeenStyle.SetValue(mtraits);

	mtraits.color = mHeaderColourCtrl.GetColor();
	mtraits.style = 0;
	mtraits.style |= (mHeaderBold ? bold : 0);
	mtraits.style |= (mHeaderItalic ? italic : 0);
	mtraits.style |= (mHeaderUnder ? underline : 0);
	copyPrefs->mHeaderStyle.SetValue(mtraits);

	mtraits.color = mTagColourCtrl.GetColor();
	mtraits.style = 0;
	mtraits.style |= (mTagBold ? bold : 0);
	mtraits.style |= (mTagItalic ? italic : 0);
	mtraits.style |= (mTagUnder ? underline : 0);
	copyPrefs->mTagStyle.SetValue(mtraits);

	cdstrvect accumulate;
	cdstring temp(mRecogniseURLs);
	char* p = ::strtok(temp.c_str_mod(), "\r\n");
	while(p)
	{
		accumulate.push_back(p);
		p = ::strtok(NULL, "\r\n");
	}	
	copyPrefs->mRecognizeURLs.SetValue(accumulate);
	
	return true;
}
