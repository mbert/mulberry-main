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


// CGetStringDialog.h : header file
//

#ifndef __CGETSTRINGDIALOG__MULBERRY__
#define __CGETSTRINGDIALOG__MULBERRY__

#include "CHelpDialog.h"
#include "cdstring.h"

/////////////////////////////////////////////////////////////////////////////
// CGetStringDialog dialog

class CGetStringDialog : public CHelpDialog
{
// Construction
public:
	CGetStringDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(short nWindow, short nTitle, cdstring& name);
	static bool PoseDialog(const char* window, const char* title, cdstring& name);
	static bool PoseDialog(const char* title, cdstring& name);

// Dialog Data
	//{{AFX_DATA(CGetStringDialog)
	enum { IDD = IDD_GETSTRINGDIALOG };
	cdstring	mName;
	cdstring	mTitle;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGetStringDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGetStringDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	static bool _PoseDialog(const char* window, const char* title, cdstring& name);

};

#endif
