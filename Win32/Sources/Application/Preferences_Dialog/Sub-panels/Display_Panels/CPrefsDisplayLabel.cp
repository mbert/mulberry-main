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


// CPrefsDisplayLabel.cpp : implementation file
//

#include "CPrefsDisplayLabel.h"

#include "CIMAPLabelsDialog.h"
#include "CPreferences.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplayLabel dialog

IMPLEMENT_DYNAMIC(CPrefsDisplayLabel, CTabPanel)

CPrefsDisplayLabel::CPrefsDisplayLabel()
	: CTabPanel(CPrefsDisplayLabel::IDD)
{
	//{{AFX_DATA_INIT(CPrefsDisplayLabel)
	//}}AFX_DATA_INIT
}


void CPrefsDisplayLabel::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsDisplayLabel)
	for(unsigned long i = 0; i < NMessage::eMaxLabels; i++)
	{
		DDX_UTF8Text(pDX, IDC_PREFSLABEL_LABEL1 + i, mLabel[i]);
		DDX_Check(pDX, IDC_PREFSLABEL_USECOLOR1 + i, mUseColour[i]);
		DDX_Check(pDX, IDC_PREFSLABEL_USEBKGND1 + i, mUseBkgndColour[i]);
		DDX_Check(pDX, IDC_PREFSLABEL_BOLD1 + i, mBold[i]);
		DDX_Check(pDX, IDC_PREFSLABEL_ITALIC1 + i, mItalic[i]);
		DDX_Check(pDX, IDC_PREFSLABEL_STRIKE1 + i, mStrike[i]);
		DDX_Check(pDX, IDC_PREFSLABEL_UNDER1 + i, mUnderline[i]);
	}
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsDisplayLabel, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsDisplayLabel)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_PREFSLABEL_USECOLOR1, IDC_PREFSLABEL_USECOLOR1 + NMessage::eMaxLabels, OnUseColor)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_PREFSLABEL_USEBKGND1, IDC_PREFSLABEL_USEBKGND1 + NMessage::eMaxLabels, OnUseBkgndColor)
	ON_BN_CLICKED(IDC_PREFSLABEL_IMAPLABELS, OnIMAPLabels)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplayLabel message handlers

BOOL CPrefsDisplayLabel::OnInitDialog() 
{
	CTabPanel::OnInitDialog();
	
	// Subclass buttons
	for(unsigned long i = 0; i < NMessage::eMaxLabels; i++)
	{
		mColour[i].SubclassDlgItem(IDC_PREFSLABEL_COLOR1 + i, this);
		mBkgndColour[i].SubclassDlgItem(IDC_PREFSLABEL_BKGND1 + i, this);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Set data
void CPrefsDisplayLabel::SetContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Set values
	for(unsigned long i = 0; i < NMessage::eMaxLabels; i++)
	{
		const SStyleTraits2* traits = copyPrefs->mLabels.GetValue()[i];
		mLabel[i] = traits->name;
		mUseColour[i] = traits->usecolor;
		mColour[i].SetColor(traits->color);
		mColour[i].EnableWindow(traits->usecolor);
		mUseBkgndColour[i] = traits->usebkgcolor;
		mBkgndColour[i].SetColor(traits->bkgcolor);
		mBkgndColour[i].EnableWindow(traits->usebkgcolor);
		mBold[i] = ((traits->style & bold) != 0);
		mItalic[i] = ((traits->style & italic) != 0);
		mStrike[i] = ((traits->style & strike_through) != 0);
		mUnderline[i] = ((traits->style & underline) != 0);

		mIMAPLabels.push_back(copyPrefs->mIMAPLabels.GetValue()[i]);
	}
}

// Force update of data
bool CPrefsDisplayLabel::UpdateContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Get values
	for(unsigned long i = 0; i < NMessage::eMaxLabels; i++)
	{
		SStyleTraits2* traits = copyPrefs->mLabels.Value()[i];
		traits->name = mLabel[i];
		traits->usecolor = mUseColour[i];
		traits->color = mColour[i].GetColor();
		traits->usebkgcolor = mUseBkgndColour[i];
		traits->bkgcolor = mBkgndColour[i].GetColor();
		traits->style = 0;
		traits->style |= (mBold[i] ? bold : 0);
		traits->style |= (mItalic[i] ? italic : 0);
		traits->style |= (mStrike[i] ? strike_through : 0);
		traits->style |= (mUnderline[i] ? underline : 0);

		copyPrefs->mIMAPLabels.Value()[i] = mIMAPLabels[i];
	}

	return true;
}

void CPrefsDisplayLabel::OnUseColor(UINT nID)
{
	unsigned long index = nID - IDC_PREFSLABEL_USECOLOR1;
	CButton* cb = static_cast<CButton*>(GetDlgItem(nID));
	mColour[index].EnableWindow(cb->GetCheck());
}

void CPrefsDisplayLabel::OnUseBkgndColor(UINT nID)
{
	unsigned long index = nID - IDC_PREFSLABEL_USEBKGND1;
	CButton* cb = static_cast<CButton*>(GetDlgItem(nID));
	mBkgndColour[index].EnableWindow(cb->GetCheck());
}


// Set IMAP labels
void CPrefsDisplayLabel::OnIMAPLabels()
{
	// Get current names
	cdstrvect names;
	for(int i = 0; i < NMessage::eMaxLabels; i++)
	{
		names.push_back(CUnicodeUtils::GetWindowTextUTF8(GetDlgItem(IDC_PREFSLABEL_LABEL1 + i)));
	}
	
	CIMAPLabelsDialog::PoseDialog(names, mIMAPLabels);
}
