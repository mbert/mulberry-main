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


// CAddressBookFrame.h : interface of the CAddressBookFrame class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CADDRESSBOOKFRAME__MULBERRY__
#define __CADDRESSBOOKFRAME__MULBERRY__

#include "CSDIFrame.h"

class CAddressBookFrameSDI : public CSDIFrame
{
	DECLARE_DYNCREATE(CAddressBookFrameSDI)
public:
	CAddressBookFrameSDI() {}

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddressBookFrameSDI)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAddressBookFrameSDI() {}

// Generated message map functions
protected:

	virtual void	InitToolTipMap();

	//{{AFX_MSG(CAddressBookFrameSDI)
		afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnDestroy(void);
	//}}AFX_MSG

public:
	// message handlers

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
