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


// CMulberryWindow.cpp : implementation of the CMulberryWindow class
//



#include "CMulberryWindow.h"

#include "CAdminLock.h"
#include "CErrorHandler.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPeriodicCheck.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CStatusWindow.h"
#include "C3PaneWindow.h"

#include "CLetterWindow.h"
#include "CLetterHeaderView.h"
#include "CMessageWindow.h"
#include "CMessageHeaderView.h"

#include "CServerWindow.h"

#include "CTCPSocket.h"
#include "StValueChanger.h"

#define	WM_TCP_ASYNCDNR	(WM_USER + 2)
#define	WM_TCP_ASYNCSELECT	(WM_USER + 3)

/////////////////////////////////////////////////////////////////////////////
// CMulberryWindow

IMPLEMENT_DYNAMIC(CMulberryWindow, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMulberryWindow, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMulberryWindow)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_WM_ENDSESSION()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_ACTIVATE()
	ON_WM_MOUSEACTIVATE()
	ON_WM_INITMENUPOPUP()

	ON_MESSAGE(MYWM_NOTIFYICON, OnNotifyCallback)
	ON_MESSAGE(WM_TCP_ASYNCDNR, OnWinsockDnrCallback)
	ON_MESSAGE(WM_TCP_ASYNCSELECT, OnWinsockSelectCallback)

	ON_MESSAGE(WM_POWERBROADCAST, OnPowerBroadcast)

	//}}AFX_MSG_MAP
	// Globals help commands
	ON_COMMAND(ID_HELP_FINDER, CMDIFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP, CMDIFrameWnd::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CMDIFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, CMDIFrameWnd::OnHelpFinder)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMulberryWindow construction/destruction

CMulberryWindow::CMulberryWindow()
{
	// TODO: add member initialization code here
	mClosing = false;
	
	mTimerID1 = 0;
	mTimerID2 = 0;
	
	mPendingRestore = false;
}

CMulberryWindow::~CMulberryWindow()
{
	if (CSDIFrame::GetAppTopWindow() == this)
		CSDIFrame::SetAppTopWindow(NULL);
}

BOOL CMulberryWindow::PreTranslateMessage(MSG* pMsg)
{
	// Special hack for non-English keyboards and AltGr keys on Win95/98
	
	static bool right_key = false;
	
	if (afxData.bWin95)
	{
		if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_MENU))
			right_key = (pMsg->lParam & (1L << 24));

		// Look for right-alt key (AltGr)
		else if (((pMsg->message == WM_CHAR) || (pMsg->message == WM_KEYDOWN)) &&
			(::GetKeyState(VK_MENU) < 0) && right_key)
		{
			// Check keyboard locale
			if (PRIMARYLANGID(::GetKeyboardLayout(0)) != LANG_ENGLISH)
				return false;
		}
	}

	// Do default action
	return CMDIFrameWnd::PreTranslateMessage(pMsg);
}

int CMulberryWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// Do inherited
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Subclass MDI client
	if (!m_wndMDIClient.SubclassWindow(m_hWndMDIClient))
	{
		TRACE0("Failed to subclass MDI client\n");
		return -1;      // fail to create
	}

	// Generate our status bar
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// Give status bar to status window object
	CStatusWindow::InitStatusBar(&m_wndStatusBar);

	// Determine application font size settings
	{
		CDC* dc = GetDC();
		CFont* old_font = (CFont*) dc->SelectStockObject(SYSTEM_FONT);
		TEXTMETRIC tmSys;
		dc->GetTextMetrics(&tmSys);
		
		dc->SelectObject(CMulberryApp::sAppFont);
		TEXTMETRIC tmNew;
		dc->GetTextMetrics(&tmNew);
		
		dc->SelectObject(old_font);
		
		ReleaseDC(dc);
		
		int nTemp = tmNew.tmHeight + min(tmNew.tmHeight, tmSys.tmHeight)/2;
		int nTempSys = tmSys.tmHeight + tmSys.tmHeight/2;
		
		CMulberryApp::sLargeFont = (tmSys.tmHeight > 16);
		
		// Now force UI element update
		CMessageWindow::UpdateUIPos();
		CMessageHeaderView::UpdateUIPos();
		CLetterWindow::UpdateUIPos();
		CLetterHeaderView::UpdateUIPos();
	}

	// Create periodic timers
	mTimerID1 = SetTimer(1, 1000, NULL);
	mTimerID2 = SetTimer(2, 100, NULL);


	// Check for calendar and remove calendar menu items
	if (CAdminLock::sAdminLock.mPreventCalendars)
	{
		GetMenu()->DeleteMenu(5, MF_BYPOSITION);
	}

	return 0;
}

void CMulberryWindow::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	// Do not do add to title if 3-pane
	CMDIFrameWnd::OnUpdateFrameTitle(CPreferences::sPrefs->mUse3Pane.GetValue() ? false : bAddToTitle);
}

// Restore window state to what it was on previous close
void CMulberryWindow::RestoreState(void)
{
	// Get current placement info
	WINDOWPLACEMENT wp;
	GetWindowPlacement(&wp);
	
	// Check for available state
	CMDIWindowState& state = CPreferences::sPrefs->mMDIWindowDefault.Value();

	// Do not set if empty
	CRect set_rect = state.GetBestRect(CPreferences::sPrefs->mMDIWindowDefault.GetValue());
	if (!set_rect.IsRectNull())
	{
		// Clip to screen
		::RectOnScreen(set_rect, NULL);

	}
	else
		set_rect = wp.rcNormalPosition;

	// Now collapse if SDI and not 3-pane
	if (!CPreferences::sPrefs->mUse3Pane.GetValue())
	{
		CRect wnd_rect;
		GetWindowRect(wnd_rect);
		CRect client_rect;
		GetClientRect(client_rect);
		set_rect.bottom = set_rect.top + wnd_rect.Height() - client_rect.Height();
		
		// Adjust for status bar
		m_wndStatusBar.GetWindowRect(wnd_rect);
		set_rect.bottom += wnd_rect.Height();
	}

	// Set placement bounds to new value
	wp.rcNormalPosition = set_rect;

	// Reset placement and do immediate redraw
	SetWindowPlacement(&wp);
	ShowWindow((state.GetState() == eWindowStateMax) ? SW_SHOWMAXIMIZED : SW_SHOW);
	UpdateWindow();
}

void CMulberryWindow::OnClose(void)
{
	if (mClosing)
		return;

	StValueChanger<bool> change(mClosing, true);

	// Get bounds
	CRect bounds;
	WINDOWPLACEMENT wp;
	GetWindowPlacement(&wp);
	bool zoomed = (wp.showCmd == SW_SHOWMAXIMIZED);
	bounds = wp.rcNormalPosition;

	// Add info to prefs
	CMDIWindowState state(NULL, &bounds, zoomed ? eWindowStateMax : eWindowStateNormal);
	if (CPreferences::sPrefs->mMDIWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mMDIWindowDefault.SetDirty();

	// Tell app to close
	if (CMulberryApp::sApp->OnClose())
	{ 
		CMDIFrameWnd::OnClose();
		
		// No more periodic
		if (mTimerID1 != 0)
		{
			//KillTimer(mTimerID1);
			mTimerID1 = 0;
		}
		if (mTimerID2 != 0)
		{
			//KillTimer(mTimerID2);
			mTimerID2 = 0;
		}
	}
}

// when Windows session ends, close all documents
void CMulberryWindow::OnEndSession(BOOL bEnding)
{
	if (mClosing)
		return;

	StValueChanger<bool> change(mClosing, true);

	CWinApp* pApp = AfxGetApp();
	ASSERT_VALID(pApp);
	if (bEnding && pApp->m_pMainWnd == this)
	{
		AfxOleSetUserCtrl(TRUE);    // keeps from randomly shutting down
		CMulberryApp::sApp->OnClose();
		pApp->CloseAllDocuments(TRUE);

		// allow application to save settings, etc.
		pApp->ExitInstance();
	}
}

void CMulberryWindow::OnTimer(UINT nIDEvent)
{
	// Do periodic chore
	if (CMulberryApp::sApp->sMailCheck && ((nIDEvent == mTimerID1) || (nIDEvent == mTimerID2)))
		CMulberryApp::sApp->sMailCheck->SpendTime(nIDEvent == mTimerID2);
}

LRESULT CMulberryWindow::OnNotifyCallback(WPARAM wParam, LPARAM lParam)
{
	// Show window on button down
	if ((lParam == WM_LBUTTONDOWN) ||
		(lParam == WM_RBUTTONDOWN))
	{
		// Look for 3-pane and operate on top-level MDI parent
		if (C3PaneWindow::s3PaneWindow)
		{
			SetForegroundWindow();
			if (IsIconic())
				ShowWindow(SW_RESTORE);
		}
		else
		{
			// Just get primary server window
			CServerView* view = CMailAccountManager::sMailAccountManager->GetMainView();
			if (view)
			{
				view->GetParentFrame()->SetForegroundWindow();
				if (view->GetParentFrame()->IsIconic())
					view->GetParentFrame()->ShowWindow(SW_RESTORE);
			}
		}
	}
	
	// Hide taskbar icon on button up
	if ((lParam == WM_LBUTTONUP) ||
		(lParam == WM_RBUTTONUP))
		CMulberryApp::sApp->HideNotification();

	return 0;
}

LRESULT CMulberryWindow::OnWinsockDnrCallback(WPARAM wParam, LPARAM lParam)
{
	// Get error code and do standard processing
	long errCode = WSAGETASYNCERROR(lParam);
	CTCPSocket::TCPProcessAsyncDNRCompletion((HANDLE) wParam, errCode);
	return 0;
}

LRESULT CMulberryWindow::OnWinsockSelectCallback(WPARAM wParam, LPARAM lParam)
{
	// Get error code and do standard processing
	long errCode = WSAGETASYNCERROR(lParam);
	CTCPSocket::TCPProcessAsyncSelectCompletion((int) wParam, errCode);
	return 0;
}

LRESULT CMulberryWindow::OnPowerBroadcast(WPARAM wParam, LPARAM lParam)
{
	// Pass to app
	CMulberryApp::sApp->OnPowerBroadcast(wParam, lParam);
	return 0;
}

BOOL CMulberryWindow::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style &= ~FWS_ADDTOTITLE;

	return CMDIFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMulberryWindow diagnostics

#ifdef _DEBUG
void CMulberryWindow::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMulberryWindow::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMulberryWindow message handlers

// Resized
void CMulberryWindow::OnSize(UINT nType, int cx, int cy)
{
	CMDIFrameWnd::OnSize(nType, cx, cy);
	
	// May need to do minimise/restore of others
	mPendingRestore = false;
	if (!CMulberryApp::sMultiTaskbar)
	{
		if (nType == SIZE_MINIMIZED)
			CSDIFrame::MinimiseAll(nType == SIZE_MINIMIZED);
		else
			mPendingRestore = true;
	}
}

void CMulberryWindow::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	// Special for SDI only
	if ((nState == WA_ACTIVE) && CSDIFrame::GetModal() && (CSDIFrame::GetModal()->GetSafeHwnd() != GetSafeHwnd()))
	{
		if (CSDIFrame::GetModal()->IsWindowVisible())
		{
			::MessageBeep(-1);
			CSDIFrame::GetModal()->SetActiveWindow();
			return;
		}
	}

	CMDIFrameWnd::OnActivate(nState, pWndOther, bMinimized);
	
	// Cancel any notifications
	CMulberryApp::sApp->HideNotification();
	
	if ((nState == WA_ACTIVE) && !bMinimized)
	{
		if (mPendingRestore)
		{
			// May need to re-active this one to bring it to the top if
			// it was the top one before
			bool re_activate = CSDIFrame::WindowIsTop(this);
			CSDIFrame::MinimiseAll(false);
			mPendingRestore = false;
			
			if (re_activate)
				SetActiveWindow();
		}
		else
		{
			CSDIFrame::WindowToTop(this);
		}
		CSDIFrame::SetAppTopWindow(this);
	}
}

int CMulberryWindow::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	// Special for SDI only
	if (CSDIFrame::GetModal() && (CSDIFrame::GetModal()->GetSafeHwnd() != GetSafeHwnd()))
	{
		if (CSDIFrame::GetModal()->IsWindowVisible())
		{
			::MessageBeep(-1);
			CSDIFrame::GetModal()->SetActiveWindow();
			return MA_NOACTIVATEANDEAT;
		}
	}

	// Cancel any notifications
	CMulberryApp::sApp->HideNotification();
	
	CSDIFrame::SetAppTopWindow(this);

	return CMDIFrameWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

void CMulberryWindow::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	HMENU pWindowsMenu = NULL;
	HMENU hMenuBar = AfxGetMainWnd()->GetMenu()->GetSafeHmenu();
	int iItem = ::GetMenuItemCount(hMenuBar);
	while (iItem--)
	{
		HMENU hMenuPop = ::GetSubMenu(hMenuBar, iItem);
		if (hMenuPop != NULL)
		{
			int iItemMax = ::GetMenuItemCount(hMenuPop);
			for (int iItemPop = 0; iItemPop < iItemMax; iItemPop++)
			{
				UINT nID = GetMenuItemID(hMenuPop, iItemPop);
				if (nID == IDM_WINDOWS_OPTIONS)
				{
					pWindowsMenu = hMenuPop;
					break;
				}
			}
		}
	}

	// Look for windows menu on MDI and update
	if (CPreferences::sPrefs->mUse3Pane.GetValue() &&
		(pPopupMenu->GetSafeHmenu() == pWindowsMenu))
	{
		// Remove existing MDI/SDI child names from Window menu
		int item_count = pPopupMenu->GetMenuItemCount();
		for(int i = 0; i < item_count; i++)
		{
			UINT nID = pPopupMenu->GetMenuItemID(i);
			if ((nID != -1L) &&
				((nID >= AFX_IDM_FIRST_MDICHILD) ||
				 ((nID >= IDM_WINDOWS_First) && (nID <= IDM_WINDOWS_Last))))
			{
				// Delete this one and all the remaining ones in the menu
				for(int j = item_count - 1; j >= i; j--)
					pPopupMenu->DeleteMenu(j, MF_BYPOSITION);
					
				// Break out of the loop as we're done
				break;
			}
		}
		
		// Get the active MDI child
		HWND active_hwnd = (HWND)::SendMessage(m_hWndMDIClient, WM_MDIGETACTIVE, 0, 0L);

		// Add menu item for the 3pane
		int i = 0;
		while(true)
		{
			CWnd* mdiwnd = FromHandle(::GetDlgItem(m_hWndMDIClient, AFX_IDM_FIRST_MDICHILD + i));
			if (!mdiwnd)
				break;

			CString wndTitle;
			mdiwnd->GetWindowText(wndTitle);

			CString menuTitle;
			menuTitle.Format(i < 10 ? _T("&%d ") : _T("%d "), i + 1);
			menuTitle += _T("Mulberry");
			pPopupMenu->AppendMenu(MF_STRING,  AFX_IDM_FIRST_MDICHILD + i, menuTitle);

			// Check the menuitem corresponding to the active MDI child (not in 3-pane)
			if (!CPreferences::sPrefs->mUse3Pane.GetValue() &&
				(active_hwnd == mdiwnd->GetSafeHwnd()))
				pPopupMenu->CheckMenuItem(AFX_IDM_FIRST_MDICHILD + i, MF_BYCOMMAND | MF_CHECKED);
			
			i++;
		}
		
		// Add items for the SDI windows in 3pane mode
		if (CPreferences::sPrefs->mUse3Pane.GetValue())
			CSDIFrame::AddWindowsToMenu(pPopupMenu);
	}

	// Do inherited
	CMDIFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
}

#pragma mark ___________________________________CMDIClientWnd

BEGIN_MESSAGE_MAP(CMDIClientWnd, CWnd)
	//{{AFX_MSG_MAP(CMDIClientWnd)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	ON_MESSAGE(WM_MDIDESTROY, OnMDIDestroy)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// MSDN fix for Window menu problems
LRESULT CMDIClientWnd::OnMDIDestroy(WPARAM wParam, LPARAM lParam)
{
	HWND hwndChild = (HWND) wParam;
	::ShowWindow(hwndChild, SW_HIDE);
	return Default();
}

