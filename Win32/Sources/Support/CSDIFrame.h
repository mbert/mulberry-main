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


// CSDIFrame.h : interface of the CSDIFrame class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CSDIFRAME__MULBERRY__
#define __CSDIFRAME__MULBERRY__

#include "CHelpFrame.h"

class CSDIFrame : public CHelpFrame
{
public:
	CSDIFrame() { mOnce = false; }

	static void EnterModal(CWnd* modal_wnd)
		{ sModalWnd.push_back(modal_wnd); }
	static void ExitModal()
		{ sModalWnd.pop_back(); }
	static CWnd* GetModal()
		{ return (sModalWnd.size() ? sModalWnd.back() : NULL); }

	static void SelectWindowItem(UINT nID);
	static void UpdateWindows();
	static void AddWindowsToMenu(CMenu* pPopupMenu);

	static void MinimiseAll(bool minimise);
	static bool IsTopWindow(CFrameWnd* frame);
	
	static void WindowToTop(CWnd* wnd);
	static bool WindowIsTop(CWnd* wnd);

	static void SetAppTopWindow(CWnd* wnd)
		{ sTopWindow = wnd; }
	static CWnd* GetAppTopWindow()
		{ return sTopWindow; }

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSDIFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSDIFrame();

// Generated message map functions
protected:
	//{{AFX_MSG(CSDIFrame)
		afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnDestroy(void);
		afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
		afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	static std::vector<CWnd*> sModalWnd;
	static CWnd* sTaskbarParent;
	static int sTaskbarCount;
	static HMENU sWindowsMenu;
	static std::vector<CWnd*> sWindowList;
	static std::vector<CWnd*> sWindowZList;
	static bool sWindowZListActive;
	static CWnd* sTopWindow;
	bool mOnce;

	static void AddWindow(CWnd* aWnd);
	static void RemoveWindow(CWnd* aWnd);
};

int ModalMessageBox(LPCTSTR lpszText, UINT nType = MB_OK, UINT nIDHelp = 0);

/////////////////////////////////////////////////////////////////////////////

#endif
