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


// CPrefsDisplayQuotes.cpp : implementation file
//

#include "CPrefsDisplayQuotes.h"

#include "CPreferences.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplayQuotes dialog

IMPLEMENT_DYNAMIC(CPrefsDisplayQuotes, CTabPanel)

CPrefsDisplayQuotes::CPrefsDisplayQuotes()
	: CTabPanel(CPrefsDisplayQuotes::IDD)
{
	//{{AFX_DATA_INIT(CPrefsDisplayQuotes)
	mQuotationBold = FALSE;
	mQuotationItalic = FALSE;
	mQuotationUnder = FALSE;
	mUseQuotation = FALSE;
	//}}AFX_DATA_INIT
}


void CPrefsDisplayQuotes::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsDisplayQuotes)
	DDX_Check(pDX, IDC_QUOTATIONBOLD, mQuotationBold);
	DDX_Check(pDX, IDC_QUOTATIONITALIC, mQuotationItalic);
	DDX_Check(pDX, IDC_QUOTATIONUNDER, mQuotationUnder);
	DDX_Check(pDX, IDC_USEQUOTATION, mUseQuotation);
	DDX_Control(pDX, IDC_USEQUOTATION, mUseQuotationBtn);
	DDX_UTF8Text(pDX, IDC_RECOGNISEQUOTES, mRecogniseQuotes);
	DDX_Control(pDX, IDC_RECOGNISEQUOTES, mRecogniseQuotesCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsDisplayQuotes, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsDisplayQuotes)
	ON_BN_CLICKED(IDC_USEQUOTATION, OnUseQuotation)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplayQuotes message handlers

BOOL CPrefsDisplayQuotes::OnInitDialog() 
{
	CTabPanel::OnInitDialog();
	
	mQuotation1ColourCtrl.SubclassDlgItem(IDC_QUOTATION1COLOUR, this);
	mQuotation2ColourCtrl.SubclassDlgItem(IDC_QUOTATION2COLOUR, this);
	mQuotation3ColourCtrl.SubclassDlgItem(IDC_QUOTATION3COLOUR, this);
	mQuotation4ColourCtrl.SubclassDlgItem(IDC_QUOTATION4COLOUR, this);
	mQuotation5ColourCtrl.SubclassDlgItem(IDC_QUOTATION5COLOUR, this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Set data
void CPrefsDisplayQuotes::SetContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Set values
	mQuotation1ColourCtrl.SetColor(copyPrefs->mQuotationStyle.GetValue().color);
	mQuotationBold = (copyPrefs->mQuotationStyle.GetValue().style & bold) ? 1 : 0;
	mQuotationItalic = (copyPrefs->mQuotationStyle.GetValue().style & italic) ? 1 : 0;
	mQuotationUnder = (copyPrefs->mQuotationStyle.GetValue().style & underline) ? 1 : 0;
	mQuotation2ColourCtrl.SetColor(copyPrefs->mQuoteColours.GetValue().at(0));
	mQuotation3ColourCtrl.SetColor(copyPrefs->mQuoteColours.GetValue().at(1));
	mQuotation4ColourCtrl.SetColor(copyPrefs->mQuoteColours.GetValue().at(2));
	mQuotation5ColourCtrl.SetColor(copyPrefs->mQuoteColours.GetValue().at(3));

	mUseQuotation = copyPrefs->mUseMultipleQuotes.GetValue() ? 1 : 0;
	mQuotation2ColourCtrl.EnableWindow(mUseQuotation);
	mQuotation3ColourCtrl.EnableWindow(mUseQuotation);
	mQuotation4ColourCtrl.EnableWindow(mUseQuotation);
	mQuotation5ColourCtrl.EnableWindow(mUseQuotation);
		
	mRecogniseQuotes.clear();
	for(short i = 0; i < copyPrefs->mRecognizeQuotes.GetValue().size(); i++)
	{
		mRecogniseQuotes += copyPrefs->mRecognizeQuotes.GetValue()[i] + "\r\n";
	}
}

// Force update of data
bool CPrefsDisplayQuotes::UpdateContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Get values
	SStyleTraits mtraits;

	mtraits.color = mQuotation1ColourCtrl.GetColor();
	mtraits.style = 0;
	mtraits.style |= (mQuotationBold ? bold : 0);
	mtraits.style |= (mQuotationItalic ? italic : 0);
	mtraits.style |= (mQuotationUnder ? underline : 0);
	copyPrefs->mQuotationStyle.SetValue(mtraits);

	bool dirty_quotes = false;
	if (copyPrefs->mQuoteColours.Value().at(0) != mQuotation2ColourCtrl.GetColor())
	{
		copyPrefs->mQuoteColours.Value().at(0) = mQuotation2ColourCtrl.GetColor();
		dirty_quotes = true;
	}
	if (copyPrefs->mQuoteColours.Value().at(1) != mQuotation3ColourCtrl.GetColor())
	{
		copyPrefs->mQuoteColours.Value().at(1) = mQuotation3ColourCtrl.GetColor();
		dirty_quotes = true;
	}
	if (copyPrefs->mQuoteColours.Value().at(2) != mQuotation4ColourCtrl.GetColor())
	{
		copyPrefs->mQuoteColours.Value().at(2) = mQuotation4ColourCtrl.GetColor();
		dirty_quotes = true;
	}
	if (copyPrefs->mQuoteColours.Value().at(3) != mQuotation5ColourCtrl.GetColor())
	{
		copyPrefs->mQuoteColours.Value().at(3) = mQuotation5ColourCtrl.GetColor();
		dirty_quotes = true;
	}
	copyPrefs->mQuoteColours.SetDirty();

	copyPrefs->mUseMultipleQuotes.SetValue(mUseQuotation);

	cdstrvect accumulate;
	cdstring temp(mRecogniseQuotes);
	char* p = ::strtok(temp.c_str_mod(), "\r\n");
	while(p)
	{
		accumulate.push_back(p);
		p = ::strtok(nil, "\r\n");
	}	
	copyPrefs->mRecognizeQuotes.SetValue(accumulate);
	
	return true;
}

void CPrefsDisplayQuotes::OnUseQuotation()
{
	mQuotation2ColourCtrl.EnableWindow(mUseQuotationBtn.GetCheck());
	mQuotation3ColourCtrl.EnableWindow(mUseQuotationBtn.GetCheck());
	mQuotation4ColourCtrl.EnableWindow(mUseQuotationBtn.GetCheck());
	mQuotation5ColourCtrl.EnableWindow(mUseQuotationBtn.GetCheck());
}

