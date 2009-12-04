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

// CCertManagerDialog.h : header file
//

#ifndef __CCERTMANAGERDIALOG__MULBERRY__
#define __CCERTMANAGERDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "cdstring.h"

#include "CCertificate.h"
#include "CTextTable.h"

/////////////////////////////////////////////////////////////////////////////
// CCertManagerDialog dialog

class CCertManagerDialog : public CHelpDialog
{
// Construction
public:
	enum ECertificateType
	{
		eNone = 0,
		eCACertificates,
		eServerCertificates,
		eUserCertificates,
		ePersonalCertificates
	};

	CCertManagerDialog(CWnd* pParent = NULL);   // standard constructor

	static void		PoseDialog();

// Dialog Data
	//{{AFX_DATA(CCertManagerDialog)
	enum { IDD = IDD_CERTIFICATEMANAGER };
	CTabCtrl		mTabs;
	CTextTable		mList;
	CButton			mViewBtn;
	CButton			mDeleteBtn;
	CStatic			mTotal;
	CStatic			mSubject;
	CStatic			mIssuer;
	CStatic			mNotBefore;
	CStatic			mNotAfter;
	CStatic			mFingerprint;
	CStatic			mLocation;
	//}}AFX_DATA
	unsigned long		mIndex;
	CCertificateList	mCerts;
	bool				mNotBeforeError;
	bool				mNotAfterError;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCertManagerDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
			void	RefreshList();

			void	SetCounter();
			void	ShowCertificate(const CCertificate* cert);

	// Generated message map functions
	//{{AFX_MSG(CCertManagerDialog)
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSelChangeTabs(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnListSelChange();
	afx_msg void OnListDblClick();
	afx_msg void OnImport();
	afx_msg void OnView();
	afx_msg void OnDelete();
	afx_msg void OnReload();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
