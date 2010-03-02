/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CSDIFrame.cpp : implementation of the CSDIFrame class
//


#include "CSDIFrame.h"

#include "CLog.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CUnicodeUtils.h"

#include "StValueChanger.h"

#include <algorithm>

/////////////////////////////////////////////////////////////////////////////
// CSDIFrame

std::vector<CWnd*> CSDIFrame::sModalWnd;
CWnd* CSDIFrame::sTaskbarParent = NULL;
int CSDIFrame::sTaskbarCount = 0;
HMENU CSDIFrame::sWindowsMenu = NULL;
std::vector<CWnd*> CSDIFrame::sWindowList;
std::vector<CWnd*> CSDIFrame::sWindowZList;
bool CSDIFrame::sWindowZListActive = false;
CWnd* CSDIFrame::sTopWindow = NULL;

BEGIN_MESSAGE_MAP(CSDIFrame, CHelpFrame)
	//{{AFX_MSG_MAP(CSDIFrame)
		ON_WM_CREATE()
		ON_WM_DESTROY()
		ON_WM_ACTIVATE()
		ON_WM_MOUSEACTIVATE()
	//}}AFX_MSG_MAP
	// Globals help commands
	ON_COMMAND(ID_HELP_FINDER, CHelpFrame::OnHelpFinder)
	ON_COMMAND(ID_HELP, CHelpFrame::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CHelpFrame::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, CHelpFrame::OnHelpFinder)
END_MESSAGE_MAP()

CSDIFrame::~CSDIFrame()
{
	// Decrement count on taskbar parent
	if (sTaskbarParent)
	{
		if (!--sTaskbarCount)
		{
			sTaskbarParent->DestroyWindow();
			delete sTaskbarParent;
			sTaskbarParent = NULL;
		}
	}
	
	if (GetAppTopWindow() == this)
		SetAppTopWindow(NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CSDIFrame message handlers

BOOL CSDIFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CHelpFrame::PreCreateWindow(cs))
		return FALSE;

	// Add taskbar pArent if not using multiple taskbar items
	if (!CMulberryApp::sMultiTaskbar)
	{
		// Create parent if needed
		if (!sTaskbarParent)
		{
			sTaskbarParent = new CWnd;
			LPCTSTR pstrOwnerClass = AfxRegisterWndClass(0);

			if (!sTaskbarParent->CreateEx(WS_EX_TOOLWINDOW, pstrOwnerClass, _T(""), WS_POPUP,
					CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, 0))
			{
				delete sTaskbarParent;
				sTaskbarParent = NULL;
				return FALSE;
			}
		}
		if (!mOnce)
			sTaskbarCount++;
		cs.hwndParent = sTaskbarParent->GetSafeHwnd();
		mOnce = true;
	}
	return TRUE;
}

void CSDIFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	// Do inherited
	CHelpFrame::OnUpdateFrameTitle(bAddToTitle);
	
	// Now refresh Windows menu
	UpdateWindows();
}

int CSDIFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	int result = CHelpFrame::OnCreate(lpCreateStruct);
	
	// Need to grab Windows menu if not already present
	if (!sWindowsMenu)
	{
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
						sWindowsMenu = hMenuPop;
						break;
					}
				}
			}
		}
	}

	// Add shared Windows menu
	CMenu* main_menu = GetMenu();
	int wnd_pos = main_menu->GetMenuItemCount() - 2;
	CString old_text;
	main_menu->GetMenuString(wnd_pos, old_text, MF_BYPOSITION);
	main_menu->DeleteMenu(wnd_pos, MF_BYPOSITION);
	main_menu->InsertMenu(wnd_pos, MF_STRING | MF_BYPOSITION | MF_POPUP, (UINT) sWindowsMenu, old_text);
	
	// Add to windows list
	AddWindow(this);

	return result;
}

void CSDIFrame::OnDestroy(void)
{
	// Remove from windows list
	RemoveWindow(this);

	// Do not allow shared Windows menu to be deleted
	CMenu* main_menu = GetMenu();
	int wnd_pos = main_menu->GetMenuItemCount() - 2;
	main_menu->RemoveMenu(wnd_pos, MF_BYPOSITION);

	CHelpFrame::OnDestroy();
}

void CSDIFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	if ((nState == WA_ACTIVE) && sModalWnd.size() &&
		sModalWnd.back()->GetSafeHwnd() && (sModalWnd.back()->GetSafeHwnd() != GetSafeHwnd()))
	{
		if (sModalWnd.back()->IsWindowVisible())
		{
			::MessageBeep(-1);
			sModalWnd.back()->SetActiveWindow();
			return;
		}
	}

	CHelpFrame::OnActivate(nState, pWndOther, bMinimized);
	
	// Cancel any notifications
	CMulberryApp::sApp->HideNotification();
	
	if (nState == WA_ACTIVE)
	{
		SetAppTopWindow(this);
	
		// Move in Z-order
		WindowToTop(this);
	}
}

int CSDIFrame::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	if (sModalWnd.size() &&
		sModalWnd.back()->GetSafeHwnd() && (sModalWnd.back()->GetSafeHwnd() != GetSafeHwnd()))
	{
		if (sModalWnd.back()->IsWindowVisible())
		{
			::MessageBeep(-1);
			sModalWnd.back()->SetActiveWindow();
			return MA_NOACTIVATEANDEAT;
		}
	}

	// Cancel any notifications
	CMulberryApp::sApp->HideNotification();
	
	SetAppTopWindow(this);

	// Move in Z-order
	WindowToTop(this);

	return CHelpFrame::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

void CSDIFrame::WindowToTop(CWnd* wnd)
{
	// Move in Z-order
	if (!sWindowZListActive)
	{
		// Check whether current in Z list and remove it
		sWindowZList.erase(std::remove(sWindowZList.begin(), sWindowZList.end(), wnd), sWindowZList.end());
		sWindowZList.push_back(wnd);
	}

}

bool CSDIFrame::WindowIsTop(CWnd* wnd)
{
	return (sWindowZList.size() != 0) ? (sWindowZList.back() == wnd) : false;
}

#pragma mark ____________________________Windows Menu

void CSDIFrame::SelectWindowItem(UINT nID)
{
	CWnd* wnd = sWindowList.at(nID - IDM_WINDOWS_First);
	if (wnd->IsIconic())
		wnd->ShowWindow(SW_RESTORE);
	wnd->BringWindowToTop();
}

void CSDIFrame::AddWindow(CWnd* aWnd)
{
	CMenu wnds;
	wnds.Attach(sWindowsMenu);

	// Add to list
	sWindowList.push_back(aWnd);
	sWindowZList.push_back(aWnd);
	
	// Append to menu - add separator if first item and not 3-pane
	if ((sWindowList.size() == 1) && !CPreferences::sPrefs->mUse3Pane.GetValue())
		wnds.AppendMenu(MF_SEPARATOR);
	cdstring title = CUnicodeUtils::GetWindowTextUTF8(aWnd);
	if (title.empty())
		title = "???";
	CUnicodeUtils::AppendMenuUTF8(&wnds, MF_STRING, IDM_WINDOWS_First + sWindowList.size() - 1, title);

	wnds.Detach();
}

void CSDIFrame::RemoveWindow(CWnd* aWnd)
{
	// Find position in list
	sWindowZList.erase(std::remove(sWindowZList.begin(), sWindowZList.end(), aWnd), sWindowZList.end());
	std::vector<CWnd*>::iterator found = std::find(sWindowList.begin(), sWindowList.end(), aWnd);
	if (found != sWindowList.end())
	{
		int pos = found - sWindowList.begin();
		
		CMenu wnds;
		wnds.Attach(sWindowsMenu);

		// Now remove it
		wnds.RemoveMenu(IDM_WINDOWS_First + pos, MF_BYCOMMAND);
		sWindowList.erase(found);

		// Renumber existing items
		for(int i = pos; i < sWindowList.size(); i++)
		{
			cdstring title = CUnicodeUtils::GetMenuStringUTF8(&wnds, IDM_WINDOWS_First + i + 1, MF_BYCOMMAND);
			CUnicodeUtils::ModifyMenuUTF8(&wnds, IDM_WINDOWS_First + i + 1, MF_STRING | MF_BYCOMMAND, IDM_WINDOWS_First + i, title);
		}

		wnds.Detach();
	}
}

void CSDIFrame::UpdateWindows()
{
	// Only if set
	if (!sWindowsMenu)
		return;

	CMenu wnds;
	wnds.Attach(sWindowsMenu);

	// Refresh entire list
	int pos = 0;
	for(std::vector<CWnd*>::const_iterator iter = sWindowList.begin(); iter != sWindowList.end(); iter++, pos++)
	{
		cdstring title = CUnicodeUtils::GetWindowTextUTF8(*iter);
		if (title.empty())
			title = "???";
		CUnicodeUtils::ModifyMenuUTF8(&wnds, IDM_WINDOWS_First + pos, MF_STRING | MF_BYCOMMAND, IDM_WINDOWS_First + pos, title);
	}

	wnds.Detach();
}

void CSDIFrame::AddWindowsToMenu(CMenu* pPopupMenu)
{
	int pos = IDM_WINDOWS_First;
	for(std::vector<CWnd*>::const_iterator iter = sWindowList.begin(); iter != sWindowList.end(); iter++, pos++)
	{
		cdstring title = CUnicodeUtils::GetWindowTextUTF8(*iter);
		if (title.empty())
			title = "???";
		CUnicodeUtils::AppendMenuUTF8(pPopupMenu, MF_STRING, pos, title);
	}
}

void CSDIFrame::MinimiseAll(bool minimise)
{
	if (sTaskbarParent)
	{
		// When minimising use the SDI window list which does not include the CMulberryWindow,
		// as that window ill handle minimise itself.
		
		// When restoring use the Z-order list which will include the CMulberryWindow and will ensure
		// that it gets placed in the proper Z-order with the other SDI windows
		
		if (minimise)
		{
			sWindowZListActive = true;
			for(std::vector<CWnd*>::const_iterator iter = sWindowList.begin(); iter != sWindowList.end(); iter++)
				(*iter)->ShowWindow(SW_HIDE);
		}
		else
		{
			sWindowZListActive = false;
			StValueChanger<bool> _change(sWindowZListActive, true);
			for(std::vector<CWnd*>::const_iterator iter = sWindowZList.begin(); iter != sWindowZList.end(); iter++)
				(*iter)->ShowWindow(SW_SHOW);
		}
	}
}

bool CSDIFrame::IsTopWindow(CFrameWnd* frame)
{
	return (GetForegroundWindow() == frame);
}

// Static fn

int ModalMessageBox(LPCTSTR lpszText, UINT nType, UINT nIDHelp)
{
	CSDIFrame::EnterModal(CSDIFrame::GetAppTopWindow());
	int result = 0;

	CWnd* make_active = NULL;
	make_active = CSDIFrame::GetAppTopWindow();
	HWND set_focus = CWnd::GetFocus()->GetSafeHwnd();

	try
	{
		result = ::AfxMessageBox(lpszText, nType, nIDHelp);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		CSDIFrame::ExitModal();
		if (make_active)
			make_active->BringWindowToTop();
		CLOG_LOGRETHROW;
		throw;
	}
	
	CSDIFrame::ExitModal();
	if (make_active)
	{
		make_active->BringWindowToTop();
		if (set_focus && ::IsWindow(set_focus))
			::SetFocus(set_focus);
	}
	
	return result;
}
