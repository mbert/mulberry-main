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


// CMessageFrame.cp : implementation of the CMessageFrame class
//


#include "CMessageFrame.h"

#include "CAdminLock.h"
#include "CDynamicMenu.h"

#pragma mark ____________________________

void CMessageFrame_InitToolTipMap(ulmap& map);
void CMessageFrame_InitToolTipMap(ulmap& map)
{
	// Toolbar
#if 0
	map.insert(ulmap::value_type(IDC_TOOLBAR_SHOWHIDE, IDC_TOOLBAR_SHOWHIDE));
	map.insert(ulmap::value_type(IDM_FILE_NEW_DRAFT, IDC_MAILBOXNEWMSGBTN));
	map.insert(ulmap::value_type(IDM_MESSAGES_REPLY, IDC_MESSAGES_REPLY));
	map.insert(ulmap::value_type(IDM_MESSAGES_FORWARD, IDC_MESSAGES_FORWARD));
	map.insert(ulmap::value_type(IDM_MESSAGES_BOUNCE, IDC_MESSAGES_BOUNCE));
	map.insert(ulmap::value_type(IDC_MAILBOXCOPYBTN, IDC_MESSAGES_COPY));
	map.insert(ulmap::value_type(IDM_MESSAGES_DELETE, IDC_MESSAGES_DELETE));
	map.insert(ulmap::value_type(ID_FILE_PRINT, IDC_MESSAGES_PRINT));
	map.insert(ulmap::value_type(IDM_MESSAGES_DELETE_READ, IDC_MESSAGES_DELETE_READ));
	map.insert(ulmap::value_type(IDM_MESSAGES_READ_PREV, IDC_MESSAGES_READ_PREV));
	map.insert(ulmap::value_type(IDM_MESSAGES_READ_NEXT, IDC_MESSAGES_READ_NEXT));
#endif

	// Message window
	map.insert(ulmap::value_type(IDC_MESSAGEFROMTWISTER, IDC_MESSAGEFROMTWISTER));
	map.insert(ulmap::value_type(IDC_MESSAGEFROMFIELD, IDC_MESSAGEFROMFIELD));
	map.insert(ulmap::value_type(IDC_MESSAGETOTWISTER, IDC_MESSAGETOTWISTER));
	map.insert(ulmap::value_type(IDC_MESSAGETOFIELD, IDC_MESSAGETOFIELD));
	map.insert(ulmap::value_type(IDC_MESSAGECCTWISTER, IDC_MESSAGECCTWISTER));
	map.insert(ulmap::value_type(IDC_MESSAGECCFIELD, IDC_MESSAGECCFIELD));
	map.insert(ulmap::value_type(IDC_MESSAGESUBJECT, IDC_MESSAGESUBJECT));
	map.insert(ulmap::value_type(IDC_MESSAGEDATE, IDC_MESSAGEDATE));

	map.insert(ulmap::value_type(IDC_MESSAGEPARTSTWISTER, IDC_MESSAGEPARTSTWISTER));
	map.insert(ulmap::value_type(IDC_MESSAGEPARTSFIELD, IDC_MESSAGEPARTSFIELD));
	map.insert(ulmap::value_type(IDC_MESSAGEPARTSFLATBTN, IDC_MESSAGEPARTSFLATBTN));
	map.insert(ulmap::value_type(IDC_MESSAGEPARTSATTACHMENTS, IDC_MESSAGEPARTSATTACHMENTS));
	map.insert(ulmap::value_type(IDC_MESSAGEPARTSALTERNATIVE, IDC_MESSAGEPARTSALTERNATIVE));

	map.insert(ulmap::value_type(IDM_MESSAGES_SHOW_HEADER, IDC_MESSAGES_SHOW_HEADER));
	map.insert(ulmap::value_type(IDM_MESSAGES_TEXT_FORMAT, IDC_MESSAGES_TEXT_FORMAT));
	map.insert(ulmap::value_type(IDC_MESSAGES_FONTINCREASE, IDC_MESSAGES_FONTINCREASE));
	map.insert(ulmap::value_type(IDC_MESSAGES_FONTDECREASE, IDC_MESSAGES_FONTDECREASE));
	map.insert(ulmap::value_type(IDC_MESSAGES_FONTSCALE, IDC_MESSAGES_FONTSCALE));
	map.insert(ulmap::value_type(IDM_QUOTEDEPTH_ALL, IDC_MESSAGES_QUOTEDEPTHBTN));
}

#pragma mark ____________________________CMessageFrameSDI
/////////////////////////////////////////////////////////////////////////////
// CMessageFrameSDI

IMPLEMENT_DYNCREATE(CMessageFrameSDI, CFrameWnd)

BEGIN_MESSAGE_MAP(CMessageFrameSDI, CSDIFrame)
	//{{AFX_MSG_MAP(CMessageFrameSDI)
		ON_WM_CREATE()
		ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessageFrameSDI message handlers

int CMessageFrameSDI::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CSDIFrame::OnCreate(lpCreateStruct);
	
	ModifyStyleEx(WS_EX_CLIENTEDGE, 0);

	CMenu* edit_menu = GetMenu()->GetSubMenu(1);
	if (edit_menu)
		CDynamicMenu::CreateEditMenu(edit_menu);

	CMenu* msgs_menu = GetMenu()->GetSubMenu(2);
	if (msgs_menu)
		CDynamicMenu::CreateMessagesMenu(msgs_menu);
	
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

void CMessageFrameSDI::OnDestroy()
{
	// Do not allow shared menu to be deleted
	CMenu* edit_menu = GetMenu()->GetSubMenu(1);
	if (edit_menu)
		CDynamicMenu::DestroyEditMenu(edit_menu);

	CMenu* msgs_menu = GetMenu()->GetSubMenu(2);
	if (msgs_menu)
		CDynamicMenu::DestroyMessagesMenu(msgs_menu);

	CMenu* help_menu = GetMenu()->GetSubMenu(6);
	if (help_menu)
		CDynamicMenu::DestroyHelpMenu(help_menu);

	CSDIFrame::OnDestroy();
}

BOOL CMessageFrameSDI::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	// Create complete entity
	if (!mMessageWindow.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), this, AFX_IDW_PANE_FIRST, pContext))
		return false;

	return true;
}
void CMessageFrameSDI::InitToolTipMap()
{
	::CMessageFrame_InitToolTipMap(mToolTipMap);
}
