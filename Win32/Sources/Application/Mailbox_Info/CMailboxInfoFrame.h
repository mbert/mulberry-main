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


// CMailboxInfoFrame.h : interface of the CMailboxInfoFrame class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CMAILBOXINFOFRAME__MULBERRY__
#define __CMAILBOXINFOFRAME__MULBERRY__

#include "CSDIFrame.h"

class CMailboxInfoFrameSDI : public CSDIFrame
{
	friend class CMailboxWindow;

	DECLARE_DYNCREATE(CMailboxInfoFrameSDI)
public:
	CMailboxInfoFrameSDI() {}

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMailboxInfoFrameSDI)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMailboxInfoFrameSDI() {}

// Generated message map functions
protected:
	virtual void	InitToolTipMap();

	//{{AFX_MSG(CMailboxInfoFrameSDI)
		afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnDestroy(void);
		afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
