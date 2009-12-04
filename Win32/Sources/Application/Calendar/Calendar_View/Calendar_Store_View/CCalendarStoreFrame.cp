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


// CCalendarStoreFrame.cpp : implementation of the CCalendarStoreFrame class
//


#include "CCalendarStoreFrame.h"

#include "CDynamicMenu.h"
#include "CHelpDialog.h"
#include "CCalendarStoreWindow.h"

#pragma mark ____________________________

void CCalendarStoreFrame_InitToolTipMap(ulmap& map);
void CCalendarStoreFrame_InitToolTipMap(ulmap& map)
{
	// Server pane
	map.insert(ulmap::value_type(IDC_SERVERFLATBTN, IDC_SERVERFLATBTN));
}

#pragma mark ____________________________CCalendarStoreFrameSDI
/////////////////////////////////////////////////////////////////////////////
// CCalendarStoreFrameSDI

IMPLEMENT_DYNCREATE(CCalendarStoreFrameSDI, CFrameWnd)

BEGIN_MESSAGE_MAP(CCalendarStoreFrameSDI, CSDIFrame)
	//{{AFX_MSG_MAP(CCalendarStoreFrameSDI)
		ON_WM_CREATE()
		ON_WM_DESTROY()
		ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCalendarStoreFrameSDI message handlers

int CCalendarStoreFrameSDI::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CSDIFrame::OnCreate(lpCreateStruct);
	
	CMenu* edit_menu = GetMenu()->GetSubMenu(1);
	if (edit_menu)
		CDynamicMenu::CreateEditMenu(edit_menu);

	CMenu* help_menu = GetMenu()->GetSubMenu(5);
	if (help_menu)
		CDynamicMenu::CreateHelpMenu(help_menu);

	return 0;
}

void CCalendarStoreFrameSDI::OnDestroy(void)
{
	// Do not allow shared menu to be deleted
	CMenu* edit_menu = GetMenu()->GetSubMenu(1);
	if (edit_menu)
		CDynamicMenu::DestroyEditMenu(edit_menu);

	CMenu* help_menu = GetMenu()->GetSubMenu(5);
	if (help_menu)
		CDynamicMenu::DestroyHelpMenu(help_menu);

	CSDIFrame::OnDestroy();
}

void CCalendarStoreFrameSDI::OnClose()
{
	CCalendarStoreWindow* wnd = (CCalendarStoreWindow*) GetTopWindow();

	// If close allowed do it
	if (wnd->GetCalendarStoreView()->TestClose())
		CSDIFrame::OnClose();
}
void CCalendarStoreFrameSDI::InitToolTipMap()
{
	::CCalendarStoreFrame_InitToolTipMap(mToolTipMap);
}
