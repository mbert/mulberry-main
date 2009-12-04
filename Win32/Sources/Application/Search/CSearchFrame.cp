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


// CSearchFrame.cpp : implementation of the CSearchFrame class
//


#include "CSearchFrame.h"

#include "CAdminLock.h"
#include "CDynamicMenu.h"
#include "CSearchWindow.h"

#pragma mark ____________________________

void CSearchFrame_InitToolTipMap(ulmap& map);
void CSearchFrame_InitToolTipMap(ulmap& map)
{
	map.insert(ulmap::value_type(IDC_SEARCH_STYLESPOPUP, IDC_SEARCH_STYLESPOPUP));
	map.insert(ulmap::value_type(IDC_SEARCH_MORE, IDC_SEARCH_MORE));
	map.insert(ulmap::value_type(IDC_SEARCH_FEWER, IDC_SEARCH_FEWER));
	map.insert(ulmap::value_type(IDC_SEARCH_CLEAR, IDC_SEARCH_CLEAR));
	map.insert(ulmap::value_type(IDC_SEARCH_SEARCH, IDC_SEARCH_SEARCH));
	map.insert(ulmap::value_type(IDC_SEARCH_CANCEL, IDC_SEARCH_CANCEL));
	map.insert(ulmap::value_type(IDC_SEARCH_TWIST, IDC_SEARCH_TWIST));
	map.insert(ulmap::value_type(IDC_SEARCH_CABINETPOPUP, IDC_SEARCH_CABINETPOPUP));
	map.insert(ulmap::value_type(IDC_SEARCH_ADDLIST, IDC_SEARCH_ADDLIST));
	map.insert(ulmap::value_type(IDC_SEARCH_CLEARLIST, IDC_SEARCH_CLEARLIST));
	map.insert(ulmap::value_type(IDC_SEARCH_OPTIONS, IDC_SEARCH_OPTIONS));

	map.insert(ulmap::value_type(IDC_SEARCHCRITERIA_POPUP1, IDC_SEARCHCRITERIA_POPUP1));
	map.insert(ulmap::value_type(IDC_SEARCHCRITERIA_POPUP2, IDC_SEARCHCRITERIA_POPUP2));
	map.insert(ulmap::value_type(IDC_SEARCHCRITERIA_POPUP3, IDC_SEARCHCRITERIA_POPUP3));
	map.insert(ulmap::value_type(IDC_SEARCHCRITERIA_POPUP4, IDC_SEARCHCRITERIA_POPUP4));
	map.insert(ulmap::value_type(IDC_SEARCHCRITERIA_TEXT1, IDC_SEARCHCRITERIA_TEXT1));
	map.insert(ulmap::value_type(IDC_SEARCHCRITERIA_TEXT2, IDC_SEARCHCRITERIA_TEXT2));
	map.insert(ulmap::value_type(IDC_SEARCHCRITERIA_TEXT3, IDC_SEARCHCRITERIA_TEXT3));
	map.insert(ulmap::value_type(IDC_SEARCHCRITERIA_DATE, IDC_SEARCHCRITERIA_DATE));
}

#pragma mark ____________________________CSearchFrameSDI
/////////////////////////////////////////////////////////////////////////////
// CSearchFrameSDI

IMPLEMENT_DYNCREATE(CSearchFrameSDI, CFrameWnd)

BEGIN_MESSAGE_MAP(CSearchFrameSDI, CSDIFrame)
	//{{AFX_MSG_MAP(CSearchFrameSDI)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSearchFrameSDI message handlers

int CSearchFrameSDI::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CSDIFrame::OnCreate(lpCreateStruct);
	
	CMenu* edit_menu = GetMenu()->GetSubMenu(1);
	if (edit_menu)
		CDynamicMenu::CreateEditMenu(edit_menu);

	CMenu* mbox_menu = GetMenu()->GetSubMenu(2);
	if (mbox_menu)
		CDynamicMenu::CreateMailboxMenu(mbox_menu);
	
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

void CSearchFrameSDI::OnDestroy(void)
{
	// Do not allow shared menu to be deleted
	CMenu* edit_menu = GetMenu()->GetSubMenu(1);
	if (edit_menu)
		CDynamicMenu::DestroyEditMenu(edit_menu);

	CMenu* mbox_menu = GetMenu()->GetSubMenu(2);
	if (mbox_menu)
		CDynamicMenu::DestroyMailboxMenu(mbox_menu);

	CMenu* help_menu = GetMenu()->GetSubMenu(6);
	if (help_menu)
		CDynamicMenu::DestroyHelpMenu(help_menu);

	CSDIFrame::OnDestroy();
}

void CSearchFrameSDI::OnClose()
{
	CSearchWindow* wnd = (CSearchWindow*) GetTopWindow();

	// If close allowed do it
	if (wnd->CloseAction())
		CSDIFrame::OnClose();
}

void CSearchFrameSDI::InitToolTipMap()
{
	::CSearchFrame_InitToolTipMap(mToolTipMap);
}
