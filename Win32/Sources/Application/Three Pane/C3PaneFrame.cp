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


// C3PaneFrame.cpp : implementation of the C3PaneFrame class
//


#include "C3PaneFrame.h"

#include "CHelpDialog.h"
#include "C3PaneWindow.h"

#pragma mark ____________________________

void C3PaneFrame_InitToolTipMap(ulmap& map);
void C3PaneFrame_InitToolTipMap(ulmap& map)
{
	//map.insert(ulmap::value_type(IDC_LOGINBTN, IDC_LOGINBTN));
	
	// General toolbar
	map.insert(ulmap::value_type(IDC_TOOLBAR_SHOWHIDE, IDC_TOOLBAR_SHOWHIDE));

	// Mail toolbar
#if 0
	map.insert(ulmap::value_type(IDM_FILE_NEW_DRAFT, IDC_MAILBOXNEWMSGBTN));
	map.insert(ulmap::value_type(IDC_LOGINBTN, IDC_LOGINBTN));
	map.insert(ulmap::value_type(IDM_MAILBOX_CREATE, IDC_SERVERCREATEBTN));
	map.insert(ulmap::value_type(IDC_SERVERCABINETPOPUP, IDC_SERVERCABINETPOPUP));
	map.insert(ulmap::value_type(IDM_MAILBOX_AUTOCHECK, IDC_SERVERREFRESHBTN));
	map.insert(ulmap::value_type(IDM_MESSAGES_REPLY, IDC_MAILBOXREPLYBTN));
	map.insert(ulmap::value_type(IDM_MESSAGES_FORWARD, IDC_MAILBOXFORWARDBTN));
	map.insert(ulmap::value_type(IDC_MAILBOXCOPYBTN, IDC_MAILBOXCOPYBTN));
	map.insert(ulmap::value_type(IDM_MESSAGES_DELETE, IDC_MAILBOXDELETEBTN));
	map.insert(ulmap::value_type(IDM_MAILBOX_EXPUNGE, IDC_MAILBOXEXPUNGEBTN));
	map.insert(ulmap::value_type(IDC_MAILBOXMATCHBTN, IDC_MAILBOXMATCHBTN));
	map.insert(ulmap::value_type(IDC_MAILBOXSELECTPOPUP, IDC_MAILBOXSELECTPOPUP));
	map.insert(ulmap::value_type(IDC_TOOLBARCHECKMAILBOX, IDC_TOOLBARCHECKMAILBOX));
	map.insert(ulmap::value_type(IDM_MAILBOX_SEARCH, IDC_MAILBOXSEARCHBTN));
	map.insert(ulmap::value_type(IDM_EDIT_PROPERTIES, IDC_MAILBOXDETAILSBTN));
	map.insert(ulmap::value_type(IDC_MAILBOXICONSTATE, IDC_MAILBOXICONSTATE));

	// Contacts toolbar
	map.insert(ulmap::value_type(IDC_ADDRESS_NEWMSG, IDC_ADDRESS_NEWMSG));
	map.insert(ulmap::value_type(IDM_ADDR_NEW, IDC_ADBKMGR_NEW));
	map.insert(ulmap::value_type(IDM_ADDR_OPEN, IDC_ADBKMGR_OPEN));
	map.insert(ulmap::value_type(IDM_ADDR_DELETE, IDC_ADBKMGR_DELETE));
	map.insert(ulmap::value_type(IDC_ADDRESS_NEW_SINGLE, IDC_ADDRESS_NEW_SINGLE));
	map.insert(ulmap::value_type(IDC_ADDRESS_NEW_GROUP, IDC_ADDRESS_NEW_GROUP));
	map.insert(ulmap::value_type(IDC_ADDRESS_DELETE, IDC_ADDRESS_DELETE));
	map.insert(ulmap::value_type(IDM_EDIT_PROPERTIES, IDC_ADBKMGR_PROPERTIES));
	map.insert(ulmap::value_type(IDM_ADDR_SEARCH, IDC_ADBKMGR_SEARCH));
#endif

	// General pane
	map.insert(ulmap::value_type(IDC_3PANETOOLBAR_ZOOMLIST, IDC_3PANETOOLBAR_ZOOMLIST));
	map.insert(ulmap::value_type(IDC_3PANETOOLBAR_ZOOMITEMS, IDC_3PANETOOLBAR_ZOOMITEMS));
	map.insert(ulmap::value_type(IDC_3PANETOOLBAR_ZOOMPREVIEW, IDC_3PANETOOLBAR_ZOOMPREVIEW));
	map.insert(ulmap::value_type(IDC_3PANEACCOUNTS_TABS, IDC_3PANEACCOUNTS_TABS));
	map.insert(ulmap::value_type(IDC_3PANEITEMS_TABS, IDC_3PANEITEMS_TABS));
	map.insert(ulmap::value_type(IDC_3PANEITEMS_CLOSEONE, IDC_3PANEITEMS_CLOSEONE));

	// Server pane
	map.insert(ulmap::value_type(IDC_SERVERFLATBTN, IDC_SERVERFLATBTN));

	// Mailbox pane
	map.insert(ulmap::value_type(IDC_MAILBOXSORTBTN, IDC_MAILBOXSORTBTN));
	map.insert(ulmap::value_type(IDC_MAILBOXSORTBTN, IDC_MAILBOXSORTBTN));
	map.insert(ulmap::value_type(IDC_MAILBOXTOTALTXT, IDC_MAILBOXTOTALTXT));
	map.insert(ulmap::value_type(IDC_MAILBOXUNSEENTXT, IDC_MAILBOXUNSEENTXT));
	map.insert(ulmap::value_type(IDC_MAILBOXDELNUMTXT, IDC_MAILBOXDELNUMTXT));
	map.insert(ulmap::value_type(IDC_MAILBOXMATCHNUMTXT, IDC_MAILBOXMATCHNUMTXT));

	// Message Preview pane
	map.insert(ulmap::value_type(IDC_MESSAGEPREVIEWCAPTION1, IDC_MESSAGEPREVIEWCAPTION1));
	map.insert(ulmap::value_type(IDC_MESSAGEPREVIEWCAPTION2, IDC_MESSAGEPREVIEWCAPTION2));

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

	// Address book pane
	map.insert(ulmap::value_type(IDC_ADDRESSTWIST, IDC_ADDRESSTWIST));
	map.insert(ulmap::value_type(IDC_GROUPTWIST, IDC_GROUPTWIST));
	map.insert(ulmap::value_type(IDC_ADDRESSESNUM, IDC_ADDRESSESNUM));
	map.insert(ulmap::value_type(IDC_GROUPSNUM, IDC_GROUPSNUM));

	// Address preview pane
}

#pragma mark ____________________________C3PaneFrameMDI
/////////////////////////////////////////////////////////////////////////////
// C3PaneFrameMDI

IMPLEMENT_DYNCREATE(C3PaneFrameMDI, CMDIChildWnd)

BEGIN_MESSAGE_MAP(C3PaneFrameMDI, CHelpMDIChildWnd)
	//{{AFX_MSG_MAP(C3PaneFrameMDI)
		ON_WM_CREATE()
		ON_WM_CLOSE()
		ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// C3PaneFrameMDI message handlers

BOOL C3PaneFrameMDI::PreCreateWindow(CREATESTRUCT& cs)
{
	// Maximise it, remove unwanted title bar bits
	cs.style |= WS_MAXIMIZE;
	cs.style &= ~(WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | FWS_ADDTOTITLE);

	return CMDIChildWnd::PreCreateWindow(cs);
}

int C3PaneFrameMDI::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CHelpMDIChildWnd::OnCreate(lpCreateStruct);
	
	ModifyStyleEx(WS_EX_CLIENTEDGE, 0);

	// Always turn off the style that adds MDI child window names to title
	SetTitle(_T(" "));

	return 0;
}

void C3PaneFrameMDI::OnClose()
{
	C3PaneWindow* wnd = (C3PaneWindow*) GetTopWindow();

	// If close allowed do it
	if (wnd->AttemptClose())
		CHelpMDIChildWnd::OnClose();
}

void C3PaneFrameMDI::OnSysCommand(UINT nID, LPARAM lParam)
{
	switch (nID & 0xFFF0)
	{
	case SC_CLOSE:
		// Ignore ctrl-F4 closes
		return;
	default:;
	}
	
	CHelpMDIChildWnd::OnSysCommand(nID, lParam); 
}

void C3PaneFrameMDI::InitToolTipMap()
{
	::C3PaneFrame_InitToolTipMap(mToolTipMap);
}
