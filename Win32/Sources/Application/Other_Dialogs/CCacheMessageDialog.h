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


// CCacheMessageDialog.h : header file
//

#ifndef __CCACHEMESSAGEDIALOG__MULBERRY__
#define __CCACHEMESSAGEDIALOG__MULBERRY__

#include "CHelpDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CCacheMessageDialog dialog

class CCacheMessageDialog : public CHelpDialog
{
// Construction
public:
	CCacheMessageDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(unsigned long& goto_num);

// Dialog Data
	//{{AFX_DATA(CCacheMessageDialog)
	enum { IDD = IDD_CACHEMESSAGE };
	UINT	mNumber;
	//}}AFX_DATA

	void	GetDetails(unsigned long& goto_num);		// Get the dialogs return info

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCacheMessageDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCacheMessageDialog)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
