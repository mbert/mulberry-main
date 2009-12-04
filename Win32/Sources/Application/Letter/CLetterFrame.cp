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


// CLetterFrame.cp : implementation of the CLetterFrame class
//


#include "CLetterFrame.h"

#include "CAdminLock.h"
#include "CDynamicMenu.h"
#include "CLetterDoc.h"

#pragma mark ____________________________

void CLetterFrame_InitToolTipMap(ulmap& map);
void CLetterFrame_InitToolTipMap(ulmap& map)
{
	// Toolbar
#if 0
	map.insert(ulmap::value_type(IDC_TOOLBAR_SHOWHIDE, IDC_TOOLBAR_SHOWHIDE));
	map.insert(ulmap::value_type(IDM_FILE_NEW_DRAFT, IDC_MAILBOXNEWMSGBTN));
	map.insert(ulmap::value_type(ID_FILE_SAVE, IDC_LETTERSAVEBTN));
	map.insert(ulmap::value_type(IDM_DRAFT_ATTACH_FILE, IDC_LETTERATTACHBTN));
	map.insert(ulmap::value_type(IDM_EDIT_SPELLCHECK, IDC_LETTERSPELLBTN));
	map.insert(ulmap::value_type(IDM_DRAFT_MDNRECEIPT, IDC_LETTERMDNBTN));
	map.insert(ulmap::value_type(IDM_DRAFT_SIGN, IDC_LETTERSIGN));
	map.insert(ulmap::value_type(IDM_DRAFT_ENCRYPT, IDC_LETTERENCRYPT));
	map.insert(ulmap::value_type(IDM_DRAFT_SEND, IDC_LETTERSENDBTN));
#endif
	
	// Letter window
	map.insert(ulmap::value_type(IDC_LETTERTOTWISTER, IDC_LETTERTOTWISTER));
	map.insert(ulmap::value_type(IDC_LETTERTOFIELD, IDC_LETTERTOFIELD));
	map.insert(ulmap::value_type(IDC_LETTERCCTWISTER, IDC_LETTERCCTWISTER));
	map.insert(ulmap::value_type(IDC_LETTERCCFIELD, IDC_LETTERCCFIELD));
	map.insert(ulmap::value_type(IDC_LETTERBCCTWISTER, IDC_LETTERBCCTWISTER));
	map.insert(ulmap::value_type(IDC_LETTERBCCFIELD, IDC_LETTERBCCFIELD));

	map.insert(ulmap::value_type(IDC_LETTERSUBJECTFIELD, IDC_LETTERSUBJECTFIELD));

	map.insert(ulmap::value_type(IDC_LETTERCOPYTOPOPUP, IDC_LETTERCOPYTOPOPUP));


	map.insert(ulmap::value_type(IDC_LETTERSENTICON, IDC_LETTERSENTICON));

	map.insert(ulmap::value_type(IDC_LETTERPARTSTWISTER, IDC_LETTERPARTSTWISTER));
	map.insert(ulmap::value_type(IDC_LETTERPARTSFIELD, IDC_LETTERPARTSFIELD));
	map.insert(ulmap::value_type(IDC_LETTERPARTSICON, IDC_LETTERPARTSICON));
	map.insert(ulmap::value_type(IDC_LETTER_IDENTITYPOPUP, IDC_LETTER_IDENTITYPOPUP));

	map.insert(ulmap::value_type(IDC_LETTER_ENR_BOLD, IDC_LETTER_ENR_BOLD));
	map.insert(ulmap::value_type(IDC_LETTER_ENR_ITALIC, IDC_LETTER_ENR_ITALIC));
	map.insert(ulmap::value_type(IDC_LETTER_ENR_UNDERLINE, IDC_LETTER_ENR_UNDERLINE));
	map.insert(ulmap::value_type(IDC_LETTER_ENR_FONT, IDC_LETTER_ENR_FONT));
	map.insert(ulmap::value_type(IDC_LETTER_ENR_SIZE, IDC_LETTER_ENR_SIZE));
	map.insert(ulmap::value_type(IDC_LETTER_ENR_COLOR, IDC_LETTER_ENR_COLOR));
	map.insert(ulmap::value_type(IDC_LETTER_ENR_LEFT, IDC_LETTER_ENR_LEFT));
	map.insert(ulmap::value_type(IDC_LETTER_ENR_CENTER, IDC_LETTER_ENR_CENTER));
	map.insert(ulmap::value_type(IDC_LETTER_ENR_RIGHT, IDC_LETTER_ENR_RIGHT));
}

#pragma mark ____________________________CLetterFrameSDI
/////////////////////////////////////////////////////////////////////////////
// CLetterFrameSDI

IMPLEMENT_DYNCREATE(CLetterFrameSDI, CFrameWnd)

BEGIN_MESSAGE_MAP(CLetterFrameSDI, CSDIFrame)
	//{{AFX_MSG_MAP(CLetterFrameSDI)
		ON_WM_CREATE()
		ON_WM_DESTROY()
		ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLetterFrameSDI message handlers

int CLetterFrameSDI::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CSDIFrame::OnCreate(lpCreateStruct);
	
	ModifyStyleEx(WS_EX_CLIENTEDGE, 0);

	CMenu* edit_menu = GetMenu()->GetSubMenu(1);
	if (edit_menu)
		CDynamicMenu::CreateEditMenu(edit_menu);

	CMenu* draft_menu = GetMenu()->GetSubMenu(2);
	if (draft_menu)
		CDynamicMenu::CreateDraftMenu(draft_menu);
	
	CMenu* help_menu = GetMenu()->GetSubMenu(6);
	if (help_menu)
		CDynamicMenu::CreateHelpMenu(help_menu);

	// Check for calendar and remove calendar menu items
	if (CAdminLock::sAdminLock.mPreventCalendars)
	{
		GetMenu()->DeleteMenu(4, MF_BYPOSITION);
	}

	return 0;
}

void CLetterFrameSDI::OnDestroy()
{
	// Do not allow shared menus to be deleted
	CMenu* edit_menu = GetMenu()->GetSubMenu(1);
	if (edit_menu)
		CDynamicMenu::DestroyEditMenu(edit_menu);

	CMenu* draft_menu = GetMenu()->GetSubMenu(2);
	if (draft_menu)
		CDynamicMenu::DestroyDraftMenu(draft_menu);

	CMenu* help_menu = GetMenu()->GetSubMenu(6);
	if (help_menu)
		CDynamicMenu::DestroyHelpMenu(help_menu);

	CSDIFrame::OnDestroy();
}

BOOL CLetterFrameSDI::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	// Create complete entity
	if (!mLetterWindow.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, AFX_IDW_PANE_FIRST, pContext))
		return false;

	return true;
}

void CLetterFrameSDI::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	// Do inherited
	CSDIFrame::OnActivate(nState, pWndOther, bMinimized);

	if ((nState != WA_ACTIVE) && mLetterWindow.IsOpen())
		static_cast<CLetterDoc*>(mLetterWindow.GetDocument())->SaveTemporary();
	
	if (nState == WA_ACTIVE)
		mLetterWindow.PauseAutoSaveTimer(false);
	else
		mLetterWindow.PauseAutoSaveTimer(true);
}

void CLetterFrameSDI::InitToolTipMap()
{
	::CLetterFrame_InitToolTipMap(mToolTipMap);
}
