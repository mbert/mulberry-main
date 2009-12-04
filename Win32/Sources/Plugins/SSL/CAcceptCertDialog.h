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

// CAcceptCertDialog.h : header file
//

#ifndef __CACCEPTCERTDIALOG__MULBERRY__
#define __CACCEPTCERTDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "cdstring.h"

/////////////////////////////////////////////////////////////////////////////
// CAcceptCertDialog dialog

class CAcceptCertDialog : public CHelpDialog
{
// Construction
public:
	enum
	{
		eAcceptOnce = 1,
		eNoAccept,
		eAcceptSave
	};

	CAcceptCertDialog(CWnd* pParent = NULL);   // standard constructor

	static int PoseDialog(const char* certificate, const cdstrvect& errors);

// Dialog Data
	//{{AFX_DATA(CAcceptCertDialog)
	enum { IDD = IDD_ACCEPTCERT };
	CListBox	mErrors;
	cdstring	mCert;
	//}}AFX_DATA
	cdstrvect	mItems;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAcceptCertDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAcceptCertDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnAcceptSave();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
