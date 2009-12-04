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


// CPrefsAccountAuth.h : header file
//

#ifndef __CPREFSACCOUNTAUTH__MULBERRY__
#define __CPREFSACCOUNTAUTH__MULBERRY__

#include "CTabPanel.h"

#include "CAuthenticator.h"
#include "CPopupButton.h"
#include "CSubPanelController.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAccountAuth dialog

class CINETAccount;

class CPrefsAccountAuth : public CTabPanel
{
// Construction
public:
	CPrefsAccountAuth();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsAccountAuth)
	enum { IDD = IDD_PREFS_AUTHENTICATE };
	CPopupButton	mAuthPopup;
	CSubPanelController	mPanels;
	CPopupButton	mTLSPopup;
	CButton			mTLSUseCert;
	CPopupButton	mTLSCert;
	//}}AFX_DATA

	cdstrvect		mCertSubjects;
	cdstrvect		mCertFingerprints;

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsAccountAuth)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	cdstring	mAuthType;

			void	SetAuthPanel(const cdstring& auth_type);		// Set auth panel
			void	BuildAuthPopup(CINETAccount* account);
			void	InitTLSItems(CINETAccount* account);
			void	BuildCertPopup();
			void	TLSItemsState();

	// Generated message map functions
	//{{AFX_MSG(CPrefsAccountAuth)
	virtual BOOL OnInitDialog();
	afx_msg void OnAuthPopup(UINT nID);
	afx_msg void OnTLSPopup(UINT nID);
	afx_msg void OnUseTLSCert();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
