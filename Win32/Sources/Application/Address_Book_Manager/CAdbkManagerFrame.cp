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


// CAdbkManagerFrame.cpp : implementation of the CAdbkManagerFrame class
//


#include "CAdbkManagerFrame.h"

#include "CAdbkManagerWindow.h"
#include "CAdminLock.h"
#include "CDynamicMenu.h"

#pragma mark ____________________________

void CAdbkManagerFrame_InitToolTipMap(ulmap& map);
void CAdbkManagerFrame_InitToolTipMap(ulmap& map)
{
#if 0
	map.insert(ulmap::value_type(IDM_ADDR_NEW, IDC_ADBKMGR_NEW));
	map.insert(ulmap::value_type(IDM_ADDR_OPEN, IDC_ADBKMGR_OPEN));
	map.insert(ulmap::value_type(IDM_ADDR_DELETE, IDC_ADBKMGR_DELETE));
	map.insert(ulmap::value_type(IDM_EDIT_PROPERTIES, IDC_ADBKMGR_PROPERTIES));
	map.insert(ulmap::value_type(IDM_ADDR_SEARCH, IDC_ADBKMGR_SEARCH));
#endif
}

#pragma mark ____________________________CAdbkManagerFrameSDI
/////////////////////////////////////////////////////////////////////////////
// CAdbkManagerFrameSDI

IMPLEMENT_DYNCREATE(CAdbkManagerFrameSDI, CFrameWnd)

BEGIN_MESSAGE_MAP(CAdbkManagerFrameSDI, CSDIFrame)
	//{{AFX_MSG_MAP(CAdbkManagerFrameSDI)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdbkManagerFrameSDI message handlers

int CAdbkManagerFrameSDI::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CSDIFrame::OnCreate(lpCreateStruct);
	
	CMenu* edit_menu = GetMenu()->GetSubMenu(1);
	if (edit_menu)
		CDynamicMenu::CreateEditMenu(edit_menu);
	
	CMenu* help_menu = GetMenu()->GetSubMenu(5);
	if (help_menu)
		CDynamicMenu::CreateHelpMenu(help_menu);

	// Check for calendar and remove calendar menu items
	if (CAdminLock::sAdminLock.mPreventCalendars)
	{
		GetMenu()->DeleteMenu(3, MF_BYPOSITION);
	}

	return 0;
}

void CAdbkManagerFrameSDI::OnDestroy(void)
{
	// Do not allow shared menus to be deleted
	CMenu* edit_menu = GetMenu()->GetSubMenu(1);
	if (edit_menu)
		CDynamicMenu::DestroyEditMenu(edit_menu);

	CMenu* help_menu = GetMenu()->GetSubMenu(5);
	if (help_menu)
		CDynamicMenu::DestroyHelpMenu(help_menu);

	CSDIFrame::OnDestroy();
}

void CAdbkManagerFrameSDI::OnClose()
{
	CAdbkManagerWindow* wnd = (CAdbkManagerWindow*) GetTopWindow();

	// If close allowed do it
	if (wnd->GetAdbkManagerView()->TestClose())
		CSDIFrame::OnClose();
}
void CAdbkManagerFrameSDI::InitToolTipMap()
{
	::CAdbkManagerFrame_InitToolTipMap(mToolTipMap);
}
