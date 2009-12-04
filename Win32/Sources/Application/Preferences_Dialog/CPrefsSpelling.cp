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


// CPrefsSpelling.cp : implementation file
//


#include "CPrefsSpelling.h"

#include "CPluginManager.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsSpellingOPtions.h"
#include "CPrefsSpellingSuggestions.h"
#include "CSpellPlugin.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsSpelling property page

IMPLEMENT_DYNCREATE(CPrefsSpelling, CPrefsPanel)

CPrefsSpelling::CPrefsSpelling() : CPrefsPanel(CPrefsSpelling::IDD)
{
	//{{AFX_DATA_INIT(CPrefsSpelling)
	//}}AFX_DATA_INIT
}

CPrefsSpelling::~CPrefsSpelling()
{
}

void CPrefsSpelling::DoDataExchange(CDataExchange* pDX)
{
	CPrefsPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsSpelling)
	mTabs.DoDataExchange(pDX);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsSpelling, CPrefsPanel)
	//{{AFX_MSG_MAP(CPrefsSpelling)
	ON_COMMAND_RANGE(IDM_DICTIONARYStart, IDM_DICTIONARYStop, OnDictionaryPopup)
	ON_NOTIFY(TCN_SELCHANGE, IDC_SPELLING_TABS, OnSelChangeSpellingTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsSpelling message handlers

void CPrefsSpelling::InitControls(void)
{
	// Subclass buttons
	mTabs.SubclassDlgItem(IDC_SPELLING_TABS, this);

	// Create tab panels
	mTabs.AddPanel(new CPrefsSpellingOptions);
	mTabs.AddPanel(new CPrefsSpellingSuggestions);

	mDictionaries.SubclassDlgItem(IDC_SPELLOPTIONS_DICTPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mDictionaries.SetMenu(IDR_POPUP_DICTIONARY);
}

void CPrefsSpelling::SetControls(void)
{
	if (mSpeller == NULL)
		return;

	// Give data to controls
	CSpellPlugin::StLoadSpellPlugin _load(mSpeller);

	InitDictionaries();
	mDictionaries.SetValue(mDictionariesValue);

	// Set existing panel
	mTabs.SetContent(mSpeller);
	mTabs.SetPanel(0);
}

// Set up params for DDX
void CPrefsSpelling::SetPrefs(CPreferences* prefs)
{
	mSpeller = CPluginManager::sPluginManager.GetSpelling();
	if (mSpeller == NULL)
		return;

	CSpellPlugin::StLoadSpellPlugin _load(mSpeller);

	// Find dictionary item that matches
	mDictionariesValue = IDM_DICTIONARYStart;
	const char* names;
	if (mSpeller->GetDictionaries(&names))
	{
		const char** name = (const char**)(names);
		while(*name)
		{
			if (mSpeller->GetPreferences()->mDictionaryName.GetValue().compare(*name) == 0)
			{
				mDictionariesName = *name;
				break;
			}
			name++;
			mDictionariesValue++;
		}
		
		if (mDictionariesName.empty())
			mDictionariesValue = IDM_DICTIONARYStart;
	}

	// Give data to controls
	mTabs.SetContent(mSpeller);
}

// Get params from DDX
void CPrefsSpelling::UpdatePrefs(CPreferences* prefs)
{
	// Get values
	mSpeller = CPluginManager::sPluginManager.GetSpelling();
	
	if (mSpeller != NULL)
	{
		CSpellPlugin::StLoadSpellPlugin _load(mSpeller);

		// Update existing panel
		mTabs.UpdateContent(mSpeller);

		if (!mDictionariesName.empty())
			mSpeller->GetPreferences()->mDictionaryName.SetValue(mDictionariesName);

		mSpeller->UpdatePreferences();
	}
}

void CPrefsSpelling::OnDictionaryPopup(UINT nID)
{
	mDictionaries.SetValue(nID);
	mDictionariesValue = nID;
	CString item_title;
	mDictionaries.GetPopupMenu()->GetMenuString(nID, item_title, MF_BYCOMMAND);
	mDictionariesName = item_title;
}

void CPrefsSpelling::OnSelChangeSpellingTabs(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	mTabs.SetPanel(mTabs.GetCurSel());

	*pResult = 0;
}

// Set dictionary popup
void CPrefsSpelling::InitDictionaries()
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

}

