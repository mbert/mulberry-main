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


// Header for CKeyChoiceDialog class

#ifndef __CKEYCHOICEDIALOG__MULBERRY__
#define __CKEYCHOICEDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CUserAction.h"

class CKeyChoiceDialog : public CHelpDialog
{
	friend class CPassUpKeyButton;

// Construction
public:
	CKeyChoiceDialog(CWnd* pParent = NULL);   // standard constructor

	static bool 	PoseDialog(unsigned char& key, CKeyModifiers& mods);

	virtual BOOL PreTranslateMessage( MSG* pMsg );

// Dialog Data
	//{{AFX_DATA(CKeyChoiceDialog)
	enum { IDD = IDD_KEYCHOICE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeyChoiceDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	unsigned char		mKey;
	CKeyModifiers		mMods;
	bool				mOK;

	// Generated message map functions
	//{{AFX_MSG(CKeyChoiceDialog)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
