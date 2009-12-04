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


// Header for CMDNPromptDialog class


#ifndef __CMDNPROMPTDIALOG__MULBERRY__
#define __CMDNPROMPTDIALOG__MULBERRY__

#include "CHelpDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CMDNPromptDialog dialog

class CMDNPromptDialog : public CHelpDialog
{
// Construction
public:
	CMDNPromptDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(const cdstring& addr);

	void	SetDetails(const cdstring& addr);				// Set the dialogs info
	void	GetDetails(bool& save);							// Set the dialogs return info

// Dialog Data
	//{{AFX_DATA(CMDNPromptDialog)
	enum { IDD = IDD_MDNPROMPT };
	cdstring	mAddress;
	BOOL		mSave;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMDNPromptDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMDNPromptDialog)
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif