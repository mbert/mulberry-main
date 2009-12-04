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


// CEditIdentityOptions.h : header file
//

#ifndef __CEDITIDENTITYOPTIONS__MULBERRY__
#define __CEDITIDENTITYOPTIONS__MULBERRY__

#include "CTabPanel.h"

#include "CIconButton.h"
#include "CMailboxPopup.h"
#include "cdstring.h"

/////////////////////////////////////////////////////////////////////////////
// CEditIdentityOptions dialog

class CEditIdentityOptions : public CTabPanel
{
	DECLARE_DYNAMIC(CEditIdentityOptions)

// Construction
public:
	CEditIdentityOptions();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditIdentityOptions)
	enum { IDD = IDD_IDENTITY_OPTIONS };
	CIconButton	mFooterBtn;
	CIconButton	mHeaderBtn;
	CMailboxPopup	mCopyToPopup;
	BOOL		mCopyToActive;
	int			mCopyToOption;
	cdstring	mCopyTo;
	CEdit		mCopyToCtrl;
	BOOL		mAlsoCopyTo;
	cdstring	mHeader;
	cdstring	mFooter;
	BOOL		mHeaderActive;
	BOOL		mFooterActive;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditIdentityOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditIdentityOptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnActive();
	afx_msg void OnCopyToEnable();
	afx_msg void OnCopyToDisable1();
	afx_msg void OnCopyToDisable2();
	afx_msg void OnCopyToPopup(UINT nID);
	afx_msg void OnHeaderActive();
	afx_msg void OnHeaderBtn();
	afx_msg void OnSignatureActive();
	afx_msg void OnSignatureBtn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual void SetActive(bool active);
	virtual void SetHeaderActive(bool active);
	virtual void SetSignatureActive(bool active);
};

#endif
