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


// CCalendarFrame.cp : implementation of the CCalendarFrame class
//


#include "CCalendarFrame.h"

#include "CDynamicMenu.h"

#pragma mark ____________________________

void CCalendarFrame_InitToolTipMap(ulmap& map);
void CCalendarFrame_InitToolTipMap(ulmap& map)
{
}

#pragma mark ____________________________CCalendarFrameSDI

/////////////////////////////////////////////////////////////////////////////
// CCalendarFrameSDI

IMPLEMENT_DYNCREATE(CCalendarFrameSDI, CFrameWnd)

BEGIN_MESSAGE_MAP(CCalendarFrameSDI, CSDIFrame)
	//{{AFX_MSG_MAP(CCalendarFrameSDI)
		ON_WM_CREATE()
		ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCalendarFrameSDI message handlers

void CCalendarFrameSDI::InitToolTipMap()
{
	::CCalendarFrame_InitToolTipMap(mToolTipMap);
}

int CCalendarFrameSDI::OnCreate(LPCREATESTRUCT lpCreateStruct)
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

void CCalendarFrameSDI::OnDestroy(void)
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
