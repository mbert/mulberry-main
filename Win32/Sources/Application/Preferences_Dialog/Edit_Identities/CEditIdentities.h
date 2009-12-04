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


// CEditIdentities.h : header file
//

#ifndef __CEDITIDENTITIES__MULBERRY__
#define __CEDITIDENTITIES__MULBERRY__

#include "CHelpDialog.h"

#include "CIdentity.h"
#include "CPopupButton.h"
#include "CTabController.h"

/////////////////////////////////////////////////////////////////////////////
// CEditIdentities dialog

class CPreferences;

class CEditIdentities : public CHelpDialog
{
// Construction
public:
	CEditIdentities(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditIdentities)
	enum { IDD = IDD_IDENTITYEDIT };
	cdstring		mName;
	cdstring		mInheritName;
	CPopupButton	mInheritPopup;
	cdstring		mAccountName;
	CPopupButton	mAccountPopup;
	BOOL			mAccountInherit;
	CTabController	mTabs;
	//}}AFX_DATA

	virtual void	SetIdentity(const CPreferences* prefs, const CIdentity* identity);
	virtual void	GetIdentity(CIdentity* identity);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditIdentities)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CIdentity			mCopyIdentity;
	const CPreferences*	mCopyPrefs;

	bool				mFromEnabled;
	bool				mReplyToEnabled;
	bool				mSenderEnabled;

	virtual void	InitServerPopup(const CPreferences* prefs);
	virtual void	InitInheritPopup(const CPreferences* prefs, const CIdentity* identity);

	// Generated message map functions
	//{{AFX_MSG(CEditIdentities)
	virtual BOOL OnInitDialog();
	afx_msg void OnInherit(UINT nID);
	afx_msg void OnSMTPAccount(UINT nID);
	afx_msg void OnSMTPAccountInherit();
	afx_msg void OnSelChangeIdentityTabs(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
