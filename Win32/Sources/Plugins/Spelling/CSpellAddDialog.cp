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

// CSpellAddDialog.cpp : implementation file
//

#include "CSpellAddDialog.h"

#include "CSDIFrame.h"
#include "CSpellPlugin.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CSpellAddDialog dialog


CSpellAddDialog::CSpellAddDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CSpellAddDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSpellAddDialog)
	for(int i = 0; i < 19; i++)
	{
		mCheck[i] = FALSE;
		mSuggestion[i] = _T("");
	}
	mCheck[0] = TRUE;
	mCapitalise = FALSE;
	//}}AFX_DATA_INIT
}


void CSpellAddDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpellAddDialog)
	for(int i = 0; i < 19; i++)
	{
		DDX_Control(pDX, IDC_SPELLADD_SUG1 + i, mSuggestionCtrl[i]);
		DDX_UTF8Text(pDX, IDC_SPELLADD_SUG1 + i, mSuggestion[i]);
		DDX_Control(pDX, IDC_SPELLADD_CHK1 + i, mCheckCtrl[i]);
		DDX_Check(pDX, IDC_SPELLADD_CHK1 + i, mCheck[i]);
	}
	DDX_Control(pDX, IDC_SPELLADD_CAPS, mCapitaliseCtrl);
	DDX_Check(pDX, IDC_SPELLADD_CAPS, mCapitalise);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpellAddDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CSpellAddDialog)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_SPELLADD_CHK1, IDC_SPELLADD_CHKJ, OnSpellAddChk)
	ON_BN_CLICKED(IDC_SPELLADD_CAPS, OnSpellAddCaps)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpellAddDialog message handlers

// Set the details
void CSpellAddDialog::SetDetails(CSpellPlugin* speller, const char* word)
{
	// Add default word
	mSuggestion[0] = word;
	
	// Do suffixes for others
	for(int i = 1; i < 19; i++)
	{
		char buf[256];
		char* s1;
		const char* suffix = speller->GetAddSuffix(i);
		::strcpy(buf, word);
		s1 = buf + ::strlen(buf);

		/* build conjugated string */
		if (i == 1)
		{
			if (*(s1-1) == 'y')
			{
				suffix = "ies";
				*(s1-1) = 0;
			}
		}
		else if ((i >= 4) && (i <= 6))
		{
			if (*(s1-1) == 'e')
				*(s1-1) = 0;
		}
		else if ((i == 14) || (i == 15))
		{
			switch(*(s1-1))
			{
			case 'a':
			case 'e':
			case 'i':
			case 'y':
				*(s1-1) = 0;
				break;
			default:;
			}
		}
		
		// Now add suffix
		::strcat(buf, suffix);
		
		// Add to text field
		mSuggestion[i] = buf;
	}

	// Now do capitalisation
	mCapitalise = (::isupper(*word) != 0);
}

// Do dictionary add
void CSpellAddDialog::DoAdd(CSpellPlugin* speller)
{
	// Add requested words
	for(int i = 0; i < 19; i++)
	{
		if (mCheck[i])
			speller->AddWord(mSuggestion[i]);
	}
}

void CSpellAddDialog::OnSpellAddChk(UINT nID) 
{
	if (mCheckCtrl[nID - IDC_SPELLADD_CHK1].GetCheck())
		mSuggestionCtrl[nID - IDC_SPELLADD_CHK1].EnableWindow(true);
	else
		mSuggestionCtrl[nID - IDC_SPELLADD_CHK1].EnableWindow(false);
}

void CSpellAddDialog::OnSpellAddCaps() 
{
	// Convert all entries to upper/lower as required
	bool capitals = mCapitaliseCtrl.GetCheck();
	for(int i = 0; i < 19; i++)
	{
		CString str;
		mSuggestionCtrl[i].GetWindowText(str);
		if (!str.IsEmpty())
		{
			str.SetAt(0, capitals ? ::toupper(*str) : ::tolower(*str));
			mSuggestionCtrl[i].SetWindowText(str);
		}
	}
}


bool CSpellAddDialog::PoseDialog(CSpellPlugin* speller, const char* add_word)
{
	bool result = false;

	// Create the dialog
	CSpellAddDialog	dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetDetails(speller, add_word);

	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{
		dlog.DoAdd(speller);
		result = true;
	}

	return result;
}
