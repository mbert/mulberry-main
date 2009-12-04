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


// CHelpFrame.h : header file
//

#ifndef __CHELPFRAME__MULBERRY__
#define __CHELPFRAME__MULBERRY__

#include "templs.h"

/////////////////////////////////////////////////////////////////////////////
// CHelpFrame view

class CToolTipHelp
{
	friend class CBetterToolTipCtrl;

// Construction
public:

	CToolTipHelp() {}   // standard constructor
	
// Implementation
protected:
	ulmap	mToolTipMap;

	virtual void	InitToolTipMap() = 0;
	virtual int		GetToolTipID(HWND aWnd) const;
	virtual int		ToolHitTest(const CWnd* aWnd, CPoint point, TOOLINFO* pTI) const;
	static  HWND	DeepChildWindowFromPoint(HWND parent, POINT pt);
};

class CBetterToolTipCtrl : public CToolTipCtrl
{
public:
	CBetterToolTipCtrl() {}
	virtual ~CBetterToolTipCtrl() {}

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CHelpFrame)
	afx_msg LRESULT OnWindowFromPoint(WPARAM, LPARAM);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CHelpFrame : public CFrameWnd, public CToolTipHelp
{
// Construction
public:

	CHelpFrame() {}   // standard constructor
	
// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CHelpFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CHelpMDIChildWnd : public CMDIChildWnd, public CToolTipHelp
{
// Construction
public:

	CHelpMDIChildWnd() {}   // standard constructor
	
// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CHelpFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
