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


// CFieldDisplay.cpp : implementation file
//


#include "CFieldDisplay.h"

#include "CAddressBookDoc.h"
#include "CAddressBookManager.h"
#include "CCopyToMenu.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CFieldDisplay

CFieldDisplay::CFieldDisplay()
{
}

CFieldDisplay::~CFieldDisplay()
{
}


BEGIN_MESSAGE_MAP(CFieldDisplay, CSpacebarEdit)
	//{{AFX_MSG_MAP(CFieldDisplay)
	ON_COMMAND(IDM_ADDR_CAPTURE, OnCaptureAddress)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFieldDisplay message handlers

void CFieldDisplay::HandleContextMenu(CWnd*, CPoint point)
{
	CMenu menu;
	VERIFY(menu.LoadMenu(mContextMenuID));
	CMenu* pPopup = menu.GetSubMenu(0);
	CMenu* pCopyPopup = pPopup->GetSubMenu(7);

	// Delete existing items in mailbox list
	while(pCopyPopup->DeleteMenu(0, MF_BYPOSITION)) {};

	// Refresh existing menu first in case of change
	CCopyToMenu::ResetMenuList();

	// Add items from existing mailbox menu to new menu
	CMenu* pCopyToPopup = CCopyToMenu::GetPopupMenu(true)->GetSubMenu(0);
	for(int i = 0; i < pCopyToPopup->GetMenuItemCount(); i++)
	{
		UINT nID;
		UINT uMenuState;
		
		uMenuState = pCopyToPopup->GetMenuState(i, MF_BYPOSITION);
		nID = pCopyToPopup->GetMenuItemID(i);
		cdstring name = CUnicodeUtils::GetMenuStringUTF8(pCopyToPopup, i, MF_BYPOSITION);
		
		if (uMenuState & MF_SEPARATOR)
			CUnicodeUtils::AppendMenuUTF8(pCopyPopup, MF_SEPARATOR, nID, name);
		else
			CUnicodeUtils::AppendMenuUTF8(pCopyPopup, LOBYTE(uMenuState), nID, name);
	}

	// Change copy/move title
	CString txt;
	txt.LoadString(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? IDS_MOVETOMAILBOX_CMD_TEXT : IDS_COPYTOMAILBOX_CMD_TEXT);
	pPopup->ModifyMenu(7, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT) pCopyPopup->m_hMenu, txt);

	// Track the popup
	CWnd* pWndPopupOwner = this;
	while (pWndPopupOwner->GetStyle() & WS_CHILD)
		pWndPopupOwner = pWndPopupOwner->GetParent();
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, pWndPopupOwner);
}

// Capture selected text as an address
void CFieldDisplay::OnCaptureAddress()
{
	// Get Selection
	cdstring selection;
	GetSelectedText(selection);
	
	// If empty, use all text
	if (selection.empty())
	{
		GetText(selection);
	}

	// Do capture if address capability available
	if (CAddressBookManager::sAddressBookManager)
		CAddressBookManager::sAddressBookManager->CaptureAddress(selection);
}

