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


// CQuickSearchSavedPopup.cpp : implementation file
//


#include "CQuickSearchSavedPopup.h"

#include "CPreferences.h"
#include "CSearchStyle.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CQuickSearchSavedPopup

CQuickSearchSavedPopup::CQuickSearchSavedPopup() :
	CPopupButton(true)
{
}

CQuickSearchSavedPopup::~CQuickSearchSavedPopup()
{
}


BEGIN_MESSAGE_MAP(CQuickSearchSavedPopup, CPopupButton)
	//{{AFX_MSG_MAP(CQuickSearchSavedPopup)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int CQuickSearchSavedPopup::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPopupButton::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Set menu
	SetMenu(IDR_POPUP_QUICKSEARCH_SAVED);

	// Init the popup menu
	SyncMenu();
	
	return 0;
}

void CQuickSearchSavedPopup::SetupCurrentMenuItem(bool check)
{
	// Force reset of menu
	SyncMenu();

	// Do inherited
	CPopupButton::SetupCurrentMenuItem(check);
}

void CQuickSearchSavedPopup::SyncMenu()
{
	// Remove any existing items from main menu
	UINT num_menu = GetPopupMenu()->GetMenuItemCount();
	for(UINT i = eFirst - eChoose; i < num_menu; i++)
		GetPopupMenu()->RemoveMenu(eFirst - eChoose, MF_BYPOSITION);

	UINT menu_id = IDM_SEARCH_STYLES_Start;
	for(CSearchStyleList::const_iterator iter = CPreferences::sPrefs->mSearchStyles.GetValue().begin();
			iter != CPreferences::sPrefs->mSearchStyles.GetValue().end(); iter++)
		CUnicodeUtils::AppendMenuUTF8(GetPopupMenu(), MF_STRING, menu_id++, (*iter)->GetName());
}
