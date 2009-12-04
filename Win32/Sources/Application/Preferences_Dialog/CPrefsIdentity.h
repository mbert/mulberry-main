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


// CPrefsIdentity.h : header file
//

#ifndef __CPREFSIDENTITY__MULBERRY__
#define __CPREFSIDENTITY__MULBERRY__

#include "CPrefsPanel.h"
#include "CListener.h"

#include "CPreferences.h"
#include "CTextTable.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsIdentity dialog

class CPreferences;

class CPrefsIdentity : public CPrefsPanel,
						public CListener
{
	DECLARE_DYNCREATE(CPrefsIdentity)

// Construction
public:
	CPrefsIdentity();
	~CPrefsIdentity();

// Dialog Data
	//{{AFX_DATA(CPrefsIdentity)
	enum { IDD = IDD_PREFS_IDENTITY };
	CTextTable	mIdentitiesCtrl;
	CButton		mChangeIdentityBtn;
	CButton		mDuplicateIdentityBtn;
	CButton		mDeleteIdentityBtn;

	BOOL	mContextTied;
	BOOL	mMsgTied;
	BOOL	mTiedMboxInherit;

	cdstring	mSmartAddressSet;
	CEdit		mSmartAddressList;
	//}}AFX_DATA
	CPreferenceValueMap<CIdentityList>*	mList;

	virtual void	ListenTo_Message(long msg, void* param);

	void SetPrefs(CPreferences* prefs);					// Set up params for DDX
	void UpdatePrefs(CPreferences* prefs);				// Get params from DDX

	void	DoMoveIdentities(const ulvector& from,	// Move identities
										unsigned long to);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsIdentity)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	virtual void InitControls(void);
	virtual void SetControls(void);
	virtual void InitIdentitiesList(void);				// Initialise the list

	// Generated message map functions
	//{{AFX_MSG(CPrefsIdentity)
	afx_msg void OnSelchangeIdentities();
	afx_msg void OnAddIdentity();
	afx_msg void OnChangeIdentity();
	afx_msg void OnDuplicateIdentity();
	afx_msg void OnDeleteIdentity();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
