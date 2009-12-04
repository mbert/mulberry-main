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


// CAddressBookFrame.cp : implementation of the CAddressBookFrame class
//


#include "CAddressBookFrame.h"

#include "CAdminLock.h"
#include "CDynamicMenu.h"
#include "CMulberryApp.h"

#pragma mark ____________________________

void CAddressBookFrame_InitToolTipMap(ulmap& map);
void CAddressBookFrame_InitToolTipMap(ulmap& map)
{
	// Toolbar
#if 0
	map.insert(ulmap::value_type(IDC_ADDRESS_NEWMSG, IDC_ADDRESS_NEWMSG));
	map.insert(ulmap::value_type(IDC_ADDRESS_NEW_SINGLE, IDC_ADDRESS_NEW_SINGLE));
	map.insert(ulmap::value_type(IDC_ADDRESS_NEW_GROUP, IDC_ADDRESS_NEW_GROUP));
	map.insert(ulmap::value_type(IDC_ADDRESS_EDIT, IDC_ADDRESS_EDIT));
	map.insert(ulmap::value_type(IDC_ADDRESS_DELETE, IDC_ADDRESS_DELETE));
#endif

	// Address book pane
	map.insert(ulmap::value_type(IDC_ADDRESSTWIST, IDC_ADDRESSTWIST));
	map.insert(ulmap::value_type(IDC_GROUPTWIST, IDC_GROUPTWIST));
	map.insert(ulmap::value_type(IDC_ADDRESSESNUM, IDC_ADDRESSESNUM));
	map.insert(ulmap::value_type(IDC_GROUPSNUM, IDC_GROUPSNUM));
}

#pragma mark ____________________________CAddressBookFrameSDI

/////////////////////////////////////////////////////////////////////////////
// CAddressBookFrameSDI

IMPLEMENT_DYNCREATE(CAddressBookFrameSDI, CFrameWnd)

BEGIN_MESSAGE_MAP(CAddressBookFrameSDI, CSDIFrame)
	//{{AFX_MSG_MAP(CAddressBookFrameSDI)
		ON_WM_CREATE()
		ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddressBookFrameSDI message handlers

void CAddressBookFrameSDI::InitToolTipMap()
{
	::CAddressBookFrame_InitToolTipMap(mToolTipMap);
}

int CAddressBookFrameSDI::OnCreate(LPCREATESTRUCT lpCreateStruct)
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

void CAddressBookFrameSDI::OnDestroy(void)
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
