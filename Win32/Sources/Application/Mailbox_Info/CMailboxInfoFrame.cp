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


// CMailboxInfoFrame.cp : implementation of the CMailboxInfoFrame class
//


#include "CMailboxInfoFrame.h"

#include "CAdminLock.h"
#include "CDynamicMenu.h"
#include "CMailboxInfoTable.h"
#include "CMailboxInfoWindow.h"
#include "CMulberryApp.h"

#pragma mark ____________________________

void CMailboxInfoFrame_InitToolTipMap(ulmap& map);
void CMailboxInfoFrame_InitToolTipMap(ulmap& map)
{
	// Toolbar
#if 0
	map.insert(ulmap::value_type(IDC_TOOLBAR_SHOWHIDE, IDC_TOOLBAR_SHOWHIDE));
	map.insert(ulmap::value_type(IDM_FILE_NEW_DRAFT, IDC_MAILBOXNEWMSGBTN));
	map.insert(ulmap::value_type(IDM_MESSAGES_REPLY, IDC_MAILBOXREPLYBTN));
	map.insert(ulmap::value_type(IDM_MESSAGES_FORWARD, IDC_MAILBOXFORWARDBTN));
	map.insert(ulmap::value_type(IDC_MAILBOXCOPYBTN, IDC_MAILBOXCOPYBTN));
	map.insert(ulmap::value_type(IDM_MESSAGES_DELETE, IDC_MAILBOXDELETEBTN));
	map.insert(ulmap::value_type(IDM_MAILBOX_EXPUNGE, IDC_MAILBOXEXPUNGEBTN));
	map.insert(ulmap::value_type(IDC_MAILBOXMATCHBTN, IDC_MAILBOXMATCHBTN));
	map.insert(ulmap::value_type(IDC_MAILBOXSELECTPOPUP, IDC_MAILBOXSELECTPOPUP));
	map.insert(ulmap::value_type(IDM_MAILBOX_CHECK, IDC_MAILBOXCHECKBTN));
	map.insert(ulmap::value_type(IDM_MAILBOX_SEARCH, IDC_MAILBOXSEARCHBTN));
	map.insert(ulmap::value_type(IDM_EDIT_PROPERTIES, IDC_MAILBOXDETAILSBTN));
	map.insert(ulmap::value_type(IDC_MAILBOXICONSTATE, IDC_MAILBOXICONSTATE));
#endif

	// Mailbox pane
	map.insert(ulmap::value_type(IDC_MAILBOXSORTBTN, IDC_MAILBOXSORTBTN));

	map.insert(ulmap::value_type(IDC_MAILBOXTOTALTXT, IDC_MAILBOXTOTALTXT));
	map.insert(ulmap::value_type(IDC_MAILBOXUNSEENTXT, IDC_MAILBOXUNSEENTXT));
	map.insert(ulmap::value_type(IDC_MAILBOXDELNUMTXT, IDC_MAILBOXDELNUMTXT));
	map.insert(ulmap::value_type(IDC_MAILBOXMATCHNUMTXT, IDC_MAILBOXMATCHNUMTXT));

	// Preview pane
	map.insert(ulmap::value_type(IDC_MESSAGEPREVIEWCAPTION1, IDC_MESSAGEPREVIEWCAPTION1));
	map.insert(ulmap::value_type(IDC_MESSAGEPREVIEWCAPTION2, IDC_MESSAGEPREVIEWCAPTION2));
	map.insert(ulmap::value_type(IDC_3PANETOOLBAR_ZOOMPREVIEW, IDC_3PANETOOLBAR_ZOOMPREVIEW));

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

#pragma mark ____________________________CMailboxFrameSDI
/////////////////////////////////////////////////////////////////////////////
// CMailboxInfoFrameSDI

IMPLEMENT_DYNCREATE(CMailboxInfoFrameSDI, CFrameWnd)

BEGIN_MESSAGE_MAP(CMailboxInfoFrameSDI, CSDIFrame)
	//{{AFX_MSG_MAP(CMailboxInfoFrameSDI)
		ON_WM_CREATE()
		ON_WM_DESTROY()
		ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMailboxInfoFrameSDI message handlers

int CMailboxInfoFrameSDI::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CSDIFrame::OnCreate(lpCreateStruct);
	
	CMenu* edit_menu = GetMenu()->GetSubMenu(1);
	if (edit_menu)
		CDynamicMenu::CreateEditMenu(edit_menu);

	CMenu* mbox_menu = GetMenu()->GetSubMenu(2);
	if (mbox_menu)
		CDynamicMenu::CreateMailboxMenu(mbox_menu);

	CMenu* msgs_menu = GetMenu()->GetSubMenu(3);
	if (msgs_menu)
		CDynamicMenu::CreateMessagesMenu(msgs_menu);
	
	CMenu* help_menu = GetMenu()->GetSubMenu(7);
	if (help_menu)
		CDynamicMenu::CreateHelpMenu(help_menu);

	// Check for calendar and remove calendar menu items
	if (CAdminLock::sAdminLock.mPreventCalendars)
	{
		GetMenu()->DeleteMenu(5, MF_BYPOSITION);
	}

	return 0;
}

void CMailboxInfoFrameSDI::OnDestroy(void)
{
	// Do not allow shared menu to be deleted
	CMenu* edit_menu = GetMenu()->GetSubMenu(1);
	if (edit_menu)
		CDynamicMenu::DestroyEditMenu(edit_menu);

	CMenu* mbox_menu = GetMenu()->GetSubMenu(2);
	if (mbox_menu)
		CDynamicMenu::DestroyMailboxMenu(mbox_menu);

	CMenu* msgs_menu = GetMenu()->GetSubMenu(3);
	if (msgs_menu)
		CDynamicMenu::DestroyMessagesMenu(msgs_menu);

	CMenu* help_menu = GetMenu()->GetSubMenu(7);
	if (help_menu)
		CDynamicMenu::DestroyHelpMenu(help_menu);

	CSDIFrame::OnDestroy();
}

void CMailboxInfoFrameSDI::OnClose()
{
	CMailboxInfoWindow* wnd = (CMailboxInfoWindow*) GetTopWindow();

	// If close allowed do it
	if (wnd->GetMailboxView()->IsOpen() && wnd->GetMailboxView()->TestClose() || wnd->GetMailboxView()->IsClosing())
		// Close the view  - this will close the actual window at idle time
		wnd->GetMailboxView()->DoClose();
}

void CMailboxInfoFrameSDI::InitToolTipMap()
{
	::CMailboxInfoFrame_InitToolTipMap(mToolTipMap);
}
