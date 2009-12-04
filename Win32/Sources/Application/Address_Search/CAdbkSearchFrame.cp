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


// CAdbkSearchFrame.cpp : implementation of the CAdbkSearchFrame class
//


#include "CAdbkSearchFrame.h"

#include "CAdminLock.h"
#include "CDynamicMenu.h"

#pragma mark ____________________________

void CAdbkSearchFrame_InitToolTipMap(ulmap& map);
void CAdbkSearchFrame_InitToolTipMap(ulmap& map)
{
	map.insert(ulmap::value_type(IDC_SEARCH_SOURCEPOPUP, IDC_SEARCH_SOURCEPOPUP));
	map.insert(ulmap::value_type(IDC_SEARCH_SERVERPOPUP, IDC_SEARCH_SERVERPOPUP));
	map.insert(ulmap::value_type(IDC_SEARCH_FIELDPOPUP, IDC_SEARCH_FIELDPOPUP));
	map.insert(ulmap::value_type(IDC_SEARCH_METHODPOPUP, IDC_SEARCH_METHODPOPUP));
	map.insert(ulmap::value_type(IDC_SEARCHTEXT, IDC_SEARCHTEXT));
	map.insert(ulmap::value_type(IDC_SEARCHBTN, IDC_SEARCHBTN));
	map.insert(ulmap::value_type(IDC_SEARCHCLEARBTN, IDC_SEARCHCLEARBTN));
	map.insert(ulmap::value_type(IDC_SEARCHNEWMSGBTN, IDC_SEARCHNEWMSGBTN));
}

#pragma mark ____________________________CAdbkSearchFrameSDI
/////////////////////////////////////////////////////////////////////////////
// CAdbkSearchFrameSDI

IMPLEMENT_DYNCREATE(CAdbkSearchFrameSDI, CFrameWnd)

BEGIN_MESSAGE_MAP(CAdbkSearchFrameSDI, CSDIFrame)
	//{{AFX_MSG_MAP(CAdbkSearchFrameSDI)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdbkSearchFrameSDI message handlers
void CAdbkSearchFrameSDI::InitToolTipMap()
{
	::CAdbkSearchFrame_InitToolTipMap(mToolTipMap);
}

void CAdbkSearchFrameSDI::OnCreate(LPCREATESTRUCT lpCreateStruct)
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
}

void CAdbkSearchFrameSDI::OnDestroy(void)
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
