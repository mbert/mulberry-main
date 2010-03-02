/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CGetPassphraseDialog.h : header file
//

#ifndef __CGETPASSPHRASEDIALOG__MULBERRY__
#define __CGETPASSPHRASEDIALOG__MULBERRY__

#include "CHelpDialog.h"
#include "CPopupButton.h"
#include "cdstring.h"

/////////////////////////////////////////////////////////////////////////////
// CGetPassphraseDialog dialog

class CGetPassphraseDialog : public CHelpDialog
{
// Construction
public:
	CGetPassphraseDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(cdstring& passphrase, const char* title);
	static bool PoseDialog(cdstring& change, const char** keys, cdstring& chosen_key, unsigned long& index, const char* title = NULL);

// Dialog Data
	//{{AFX_DATA(CGetPassphraseDialog)
	//enum { IDD = IDD_GetPassphraseDIALOG };
	enum { IDD = IDD_GETPASSPHRASE };
	cdstring	mPassphrase1;
	CEdit		mPassphrase1Ctrl;
	cdstring	mPassphrase2;
	CEdit		mPassphrase2Ctrl;
	CPopupButton mKeyPopup;
	cdstring	mKeyPopupText;
	BOOL		mHideTyping;
	CButton		mHideTypingCtrl;
	//}}AFX_DATA
	const char** mKeys;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGetPassphraseDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGetPassphraseDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnHideTyping();
	afx_msg void OnKeyPopup(UINT nID);
	//}}AFX_MSG
	
	void InitKeysMenu(void);

	DECLARE_MESSAGE_MAP()
};

#endif
