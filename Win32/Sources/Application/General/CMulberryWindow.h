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


// CMulberryWindow.h : interface of the CMulberryWindow class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CMULBERRYWINDOW__MULBERRY__
#define __CMULBERRYWINDOW__MULBERRY__

#include "CStatusWindow.h"

class CMDIClientWnd : public CWnd
{
public:
	CMDIClientWnd() {}

// Generated message map functions
protected:
	//{{AFX_MSG(CMulberryWindow)
	afx_msg LRESULT OnMDIDestroy(WPARAM wParam, LPARAM lParam);
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CMulberryWindow : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMulberryWindow)
public:
	CMulberryWindow();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMulberryWindow)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMulberryWindow();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

	virtual void RestoreState(void);

protected:  // control bar embedded members
	CMDIClientWnd	m_wndMDIClient;
	CUTF8StatusBar	m_wndStatusBar;
	bool			mClosing;
	UINT			mTimerID1;
	UINT			mTimerID2;
	bool			mPendingRestore;

// Generated message map functions
protected:
	//{{AFX_MSG(CMulberryWindow)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnEndSession(BOOL bEnding);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg LRESULT OnNotifyCallback(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWinsockDnrCallback(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWinsockSelectCallback(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPowerBroadcast(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
