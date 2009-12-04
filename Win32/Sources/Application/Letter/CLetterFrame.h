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


// CLetterFrame.h : interface of the CLetterFrame class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CLETTERFRAME__MULBERRY__
#define __CLETTERFRAME__MULBERRY__

#include "CSDIFrame.h"

#include "CLetterWindow.h"

class CLetterFrameSDI : public CSDIFrame
{
	DECLARE_DYNCREATE(CLetterFrameSDI)
public:
	CLetterFrameSDI() {}

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLetterFrameSDI)
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CLetterFrameSDI() {}

// Generated message map functions
protected:

	virtual void	InitToolTipMap();

	//{{AFX_MSG(CLetterFrameSDI)
		afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnDestroy();
		afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	//}}AFX_MSG

public:
	CLetterWindow mLetterWindow;

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
