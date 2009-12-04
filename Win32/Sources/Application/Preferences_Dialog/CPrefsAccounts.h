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


// CPrefsAccounts.h : header file
//

#ifndef __CPREFSACCOUNTS__MULBERRY__
#define __CPREFSACCOUNTS__MULBERRY__

#include "CPrefsPanel.h"

#include "CINETAccount.h"
#include "CPopupButton.h"
#include "CSubPanelController.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAccounts dialog

class CPrefsAccounts : public CPrefsPanel
{
	DECLARE_DYNCREATE(CPrefsAccounts)

// Construction
public:
	CPrefsAccounts();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsAccounts)
	enum { IDD = IDD_PREFS_ACCOUNT };
	CPopupButton	mAccountPopup;
	CStatic	mServerType;
	CEdit	mServerIP;
	CStatic	mServerIPTitle;
	CSubPanelController	mPanels;
	BOOL	mMatchUIDPswd;
	//}}AFX_DATA
	int mAccountValue;
	bool mIsSMTP;

	void SetPrefs(CPreferences* prefs);					// Set up params for DDX
	void UpdatePrefs(CPreferences* prefs);				// Get params from DDX

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsAccounts)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	virtual void	InitAccountPopup(void);
	virtual void	DoNewAccount(void);								// Add new account
	virtual void	DoRenameAccount(void);							// Rename account
	virtual void	DoDeleteAccount(void);							// Delete account
	virtual void	UpdateItems(bool enable);						// Update item entry
	virtual void	SetAccount(const CINETAccount* account);		// Set account details
	virtual void	UpdateAccount(void);							// Update current account
	
	virtual void	GetCurrentAccount(CINETAccount*& acct, 
										long& index,
										CINETAccountList*& list);	// Get current account details

	virtual void	SetPanel(const CINETAccount* account);			// Set panel

	// Generated message map functions
	//{{AFX_MSG(CPrefsAccounts)
	virtual void InitControls(void);
	virtual void SetControls(void);
	afx_msg void OnAccountPopup(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
