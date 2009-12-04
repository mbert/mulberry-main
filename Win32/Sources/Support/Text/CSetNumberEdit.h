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


// CSetNumberEdit.h : header file
//

#ifndef __CSETNUMBEREDIT__MULBERRY__
#define __CSETNUMBEREDIT__MULBERRY__

#include "CNumberEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CSetNumberEdit window

class CTwister;

class CSetNumberEdit : public CNumberEdit
{
// Construction
public:
	CSetNumberEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetNumberEdit)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSetNumberEdit();

	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID, UINT nSpinID, bool read_only = false);

	virtual CSpinButtonCtrl* GetSpinner(void)
				{ return &mSpinner; }

	// Generated message map functions
protected:
	//{{AFX_MSG(CSetNumberEdit)
	afx_msg void OnMove(int x, int y);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

private:
	CSpinButtonCtrl		mSpinner;
	bool				mVisible;

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
