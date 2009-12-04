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


// CGrayBorder.h : header file
//

#ifndef __CGRAYBORDER__MULBERRY__
#define __CGRAYBORDER__MULBERRY__

#include "CWndAligner.h"

/////////////////////////////////////////////////////////////////////////////
// CGrayBorder window

class CGrayBorder : public CButton, public CWndAligner
{
	DECLARE_DYNAMIC(CGrayBorder)

// Construction
public:
	CGrayBorder();
	virtual ~CGrayBorder();

	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command

	// Generated message map functions
protected:
	//{{AFX_MSG(CGrayBorder)
	//{{AFX_MSG(CGrayBackground)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
