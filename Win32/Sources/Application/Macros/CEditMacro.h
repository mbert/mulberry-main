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


// CEditMacro.h : header file
//

#ifndef __CEDITMACRO__MULBERRY__
#define __CEDITMACRO__MULBERRY__

#include "CHelpDialog.h"

#include "cdstring.h"

/////////////////////////////////////////////////////////////////////////////
// CEditMacro dialog

class CEditMacro : public CHelpDialog
{
// Construction
public:
	CEditMacro(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(cdstring& name, cdstring& macro);

	virtual void	SetData(const cdstring& name, const cdstring& text);		// Set data
	virtual void	GetData(cdstring& name, cdstring& text);					// Get data
	virtual void	SetRuler();													// Set current wrap length

// Dialog Data
	//{{AFX_DATA(CEditMacro)
	enum { IDD = IDD_EDITMACRO };
	cdstring	mName;
	cdstring	mEditMacro;
	CEdit		mEditMacroCtrl;
	CStatic		mEditRuler;
	CButton		mOKBtn;
	CButton		mCancelBtn;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditMacro)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CReplyChooseDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG

private:

	DECLARE_MESSAGE_MAP()
};

#endif
