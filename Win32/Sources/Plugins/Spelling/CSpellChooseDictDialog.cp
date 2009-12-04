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

// CSpellChooseDictDialog.cpp : implementation file
//

#include "CSpellChooseDictDialog.h"

#include "CSDIFrame.h"
#include "CSpellPlugin.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CSpellChooseDictDialog dialog


CSpellChooseDictDialog::CSpellChooseDictDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CSpellChooseDictDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSpellChooseDictDialog)
	//}}AFX_DATA_INIT
}


void CSpellChooseDictDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpellChooseDictDialog)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpellChooseDictDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CSpellChooseDictDialog)
	ON_COMMAND_RANGE(IDM_DICTIONARYStart, IDM_DICTIONARYStop, OnChangeDictionaryPopup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpellChooseDictDialog message handlers

BOOL CSpellChooseDictDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass buttons
	mDictionaries.SubclassDlgItem(IDC_SPELLCHOOSEDICT_DICTPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mDictionaries.SetMenu(IDR_POPUP_DICTIONARY);
	InitDictionaries();

	return true;
}

// Set the options
void CSpellChooseDictDialog::SetOptions(CSpellPlugin* speller)
{
	mSpeller = speller;
}

// Get the options
void CSpellChooseDictDialog::GetOptions(CSpellPlugin* speller)
{
	CString item_title;
	mDictionaries.GetPopupMenu()->GetMenuString(mDictionaries.GetValue(), item_title, MF_BYCOMMAND);
	speller->GetPreferences()->mDictionaryName.SetValue(cdstring(item_title));
}

// Set dictionary popup
void CSpellChooseDictDialog::InitDictionaries()
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

void CSpellChooseDictDialog::OnChangeDictionaryPopup(UINT nID) 
{
	mDictionaries.SetValue(nID);
}

bool CSpellChooseDictDialog::PoseDialog(CSpellPlugin* speller)
{
	bool result = false;

	// Create the dialog
	CSpellChooseDictDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetOptions(speller);

	if (dlog.DoModal() == IDOK)
	{
		dlog.GetOptions(speller);
		result = true;
	}

	return result;
}