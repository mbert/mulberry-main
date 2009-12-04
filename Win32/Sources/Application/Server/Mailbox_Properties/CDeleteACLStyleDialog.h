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


// CDeleteACLStyleDialog.h : header file
//

#ifndef __CDELETEACLSTYLEDIALOG__MULBERRY__
#define __CDELETEACLSTYLEDIALOG__MULBERRY__

#include "CHelpDialog.h"
#include "CTextTable.h"
#include "templs.h"

/////////////////////////////////////////////////////////////////////////////
// CDeleteACLStyleDialog dialog

class CDeleteACLStyleDialog : public CHelpDialog
{
// Construction
public:
	CDeleteACLStyleDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDeleteACLStyleDialog)
	enum { IDD = IDD_DELETE_ACL_STYLE };
	CTextTable	mStyleList;
	ulvector	mStyleSelect;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeleteACLStyleDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDeleteACLStyleDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
