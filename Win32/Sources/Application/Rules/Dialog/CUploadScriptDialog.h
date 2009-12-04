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


// CUploadScriptDialog.h : header file
//

#ifndef __CUPLOADSCRIPTDIALOG__MULBERRY__
#define __CUPLOADSCRIPTDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CPopupButton.h"

#include "cdstring.h"

/////////////////////////////////////////////////////////////////////////////
// CUploadScriptDialog dialog

class CUploadScriptDialog : public CHelpDialog
{
// Construction
public:
	CUploadScriptDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUploadScriptDialog)
	enum { IDD = IDD_UPLOADSCRIPTDIALOG };

	struct SUploadScript
	{
		bool			mFile;
		unsigned long	mAccountIndex;
		bool			mUpload;
		bool			mActivate;
	};

	int				mFile;
	CPopupButton	mAccountPopup;
	int				mAccountValue;
	int				mUploadGroup;
	//}}AFX_DATA
	
	static bool PoseDialog(SUploadScript& details);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUploadScriptDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	static SUploadScript sLastChoice;

	void	SetDetails(SUploadScript& create);		// Set the dialogs info
	void	GetDetails(SUploadScript& result);		// Get the dialogs return info

	// Generated message map functions
	//{{AFX_MSG(CUploadScriptDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnFileBtn();
	afx_msg void OnServerBtn();
	afx_msg void OnAccountPopup(UINT nID);
	//}}AFX_MSG
	
	void InitAccountMenu(void);

	DECLARE_MESSAGE_MAP()
};

#endif
