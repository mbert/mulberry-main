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


// CHelpDialog.h : header file
//

#ifndef __CHELPDIALOG__MULBERRY__
#define __CHELPDIALOG__MULBERRY__

/////////////////////////////////////////////////////////////////////////////
// CHelpDialog dialog

class CHelpDialog : public CDialog
{
	DECLARE_DYNAMIC(CHelpDialog)

// Construction
public:

	CHelpDialog();   // standard constructor
	CHelpDialog(UINT nID, CWnd* pParent = NULL);   // standard constructor
	
#ifdef __MULBERRY
	virtual int DoModal();
#endif

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CHelpDialog)
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
