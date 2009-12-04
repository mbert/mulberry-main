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


// CServerFrame.cpp : implementation of the CServerFrame class
//


#include "CServerFrame.h"

#include "CAdminLock.h"
#include "CDynamicMenu.h"
#include "CHelpDialog.h"
#include "CServerWindow.h"

#pragma mark ____________________________

void CServerFrame_InitToolTipMap(ulmap& map);
void CServerFrame_InitToolTipMap(ulmap& map)
{
	// Mailbox pane
#if 0
	map.insert(ulmap::value_type(IDC_TOOLBAR_SHOWHIDE, IDC_TOOLBAR_SHOWHIDE));
	map.insert(ulmap::value_type(IDM_FILE_NEW_DRAFT, IDC_MAILBOXNEWMSGBTN));
	map.insert(ulmap::value_type(IDC_LOGINBTN, IDC_LOGINBTN));
	map.insert(ulmap::value_type(IDM_MAILBOX_CREATE, IDC_SERVERCREATEBTN));
	map.insert(ulmap::value_type(IDC_SERVERCABINETPOPUP, IDC_SERVERCABINETPOPUP));
	map.insert(ulmap::value_type(IDM_MAILBOX_AUTOCHECK, IDC_SERVERREFRESHBTN));
	map.insert(ulmap::value_type(IDM_MAILBOX_SEARCH, IDC_MAILBOXSEARCHBTN));
	map.insert(ulmap::value_type(IDM_EDIT_PROPERTIES, IDC_SERVERPROPBTN));
#endif

	// Server pane
	map.insert(ulmap::value_type(IDC_SERVERFLATBTN, IDC_SERVERFLATBTN));
}

#pragma mark ____________________________CServerFrameSDI
/////////////////////////////////////////////////////////////////////////////
// CServerFrameSDI

IMPLEMENT_DYNCREATE(CServerFrameSDI, CFrameWnd)

BEGIN_MESSAGE_MAP(CServerFrameSDI, CSDIFrame)
	//{{AFX_MSG_MAP(CServerFrameSDI)
		ON_WM_CREATE()
		ON_WM_DESTROY()
		ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServerFrameSDI message handlers

int CServerFrameSDI::OnCreate(LPCREATESTRUCT lpCreateStruct)
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

void CServerFrameSDI::OnDestroy(void)
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

void CServerFrameSDI::OnClose()
{
	CServerWindow* wnd = (CServerWindow*) GetTopWindow();

	// If close allowed do it
	if (wnd->AttemptClose())
		CSDIFrame::OnClose();
}
void CServerFrameSDI::InitToolTipMap()
{
	::CServerFrame_InitToolTipMap(mToolTipMap);
}
