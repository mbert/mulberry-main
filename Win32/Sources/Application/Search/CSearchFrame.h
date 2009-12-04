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


// CSearchFrame.h : interface of the CSearchFrame class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CSEARCHFRAME__MULBERRY__
#define __CSEARCHFRAME__MULBERRY__

#include "CSDIFrame.h"

class CSearchFrameSDI : public CSDIFrame
{
	DECLARE_DYNCREATE(CSearchFrameSDI)
public:
	CSearchFrameSDI() {}

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSearchFrameSDI)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSearchFrameSDI() {}

// Generated message map functions
protected:

	virtual void	InitToolTipMap();

	//{{AFX_MSG(CSearchFrameSDI)
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy(void);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
