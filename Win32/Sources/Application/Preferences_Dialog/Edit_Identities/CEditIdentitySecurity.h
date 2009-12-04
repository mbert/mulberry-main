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


// CEditIdentitySecurity.h : header file
//

#ifndef __CEDITIDENTITYSECURITY__MULBERRY__
#define __CEDITIDENTITYSECURITY__MULBERRY__

#include "CTabPanel.h"

#include "CPopupButton.h"

/////////////////////////////////////////////////////////////////////////////
// CEditIdentitySecurity dialog

class CEditIdentitySecurity : public CTabPanel
{
	DECLARE_DYNAMIC(CEditIdentitySecurity)

// Construction
public:
	CEditIdentitySecurity();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditIdentitySecurity)
	enum { IDD = IDD_IDENTITY_SECURITY };
	BOOL	mActive;
	BOOL	mSign;
	BOOL	mEncrypt;
	CPopupButton	mSignWithPopup;
	cdstring	mSignOther;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditIdentitySecurity)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditIdentitySecurity)
	virtual BOOL OnInitDialog();
	afx_msg void OnActive();
	afx_msg void OnSignWithPopup(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void SetActive(bool active);
};

#endif
