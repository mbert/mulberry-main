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


// CPropMailboxOptions.h : header file
//

#ifndef __CPROPMAILBOXOPTIONS__MULBERRY__
#define __CPROPMAILBOXOPTIONS__MULBERRY__

#include "CHelpPropertyPage.h"

#include "CIconWnd.h"
#include "CIdentityPopup.h"
#include "CMailAccountManager.h"

/////////////////////////////////////////////////////////////////////////////
// CPropMailboxOptions dialog

class CIdentity;
class CMbox;
class CMboxList;

class CPropMailboxOptions : public CHelpPropertyPage
{
	DECLARE_DYNCREATE(CPropMailboxOptions)

// Construction
public:
	CPropMailboxOptions();
	~CPropMailboxOptions();

// Dialog Data
	//{{AFX_DATA(CPropMailboxOptions)
	enum { IDD = IDD_MAILBOX_OPTIONS };
	int mIconState;
	CIconWnd mIconStateCtrl;
	int		mCheck;
	CButton	mCheckCtrl;
	unsigned long	mAlertStyle;
	CPopupButton	mAlertStylePopup;
	int		mOpen;
	CButton	mOpenCtrl;
	int		mCopyTo;
	CButton	mCopyToCtrl;
	int		mAppendTo;
	CButton	mAppendToCtrl;
	int		mPunt;
	CButton	mPuntCtrl;
	int		mAutoSync;
	CButton	mAutoSyncCtrl;
	int		mTieIdentity;
	CButton	mTieIdentityCtrl;
	CIdentityPopup mIdentityPopup;

	cdstring	mCurrentIdentity;
	bool		mDisableAutoSync;
	bool		mAllDir;
	bool		mHasLocal;

	//}}AFX_DATA
	virtual void	SetMboxList(CMboxList* mbox_list);				// Set mbox list

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropMailboxOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    	// DDX/DDV support
	//}}AFX_VIRTUAL


private:
	CMboxList*		mMboxList;								// List of selected mboxes

	void	SetFavourite(bool set, CMailAccountManager::EFavourite fav_type);
	void	SetTiedIdentity(bool add);				// Add/remove tied identity

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropMailboxOptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckNew();
	afx_msg void OnAlertStylePopup(UINT nID);
	afx_msg void OnOpen();
	afx_msg void OnCopyTo();
	afx_msg void OnAppendTo();
	afx_msg void OnPunt();
	afx_msg void OnAutoSync();
	afx_msg void OnTieIdentity();
	afx_msg void OnIdentityPopup(UINT nID);
	afx_msg void OnRebuildLocal();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

			void	InitAlertStylePopup();
			
			CMailAccountManager::EFavourite GetStyleType(unsigned long index) const;

};

#endif
