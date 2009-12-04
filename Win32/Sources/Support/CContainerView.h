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


// CContainerView.h : header file
//

#ifndef __CCONTAINERVIEW__MULBERRY__
#define __CCONTAINERVIEW__MULBERRY__

#include "CCommanderProtect.h"

/////////////////////////////////////////////////////////////////////////////
// CContainerView view

class CContainerView : public CView
{
protected:
	HWND m_hWndFocus;   // last window to have focus

	CContainerView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CContainerView)

// Attributes
public:

// Operations
public:

			void ExpandChildToFit();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CContainerView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CContainerView();

	virtual void OnActivateView(BOOL, CView*, CView*);
	virtual void OnActivateFrame(UINT, CFrameWnd*);
	BOOL SaveFocusControl();    // updates m_hWndFocus

public:
	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command
	// Generated message map functions
protected:
	CCommanderProtect		mCmdProtect;					// Protect commands

	//{{AFX_MSG(CContainerView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
