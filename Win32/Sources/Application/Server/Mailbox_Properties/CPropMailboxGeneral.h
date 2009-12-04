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


// CPropMailboxGeneral.h : header file
//

#ifndef __CPROPMAILBOXGENERAL__MULBERRY__
#define __CPROPMAILBOXGENERAL__MULBERRY__

#include "CHelpPropertyPage.h"

#include "CIconWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CPropMailboxGeneral dialog

class CMbox;
class CMboxList;

class CPropMailboxGeneral : public CHelpPropertyPage
{
	DECLARE_DYNCREATE(CPropMailboxGeneral)

// Construction
public:
	CPropMailboxGeneral();
	~CPropMailboxGeneral();

// Dialog Data
	//{{AFX_DATA(CPropMailboxGeneral)
	enum { IDD = IDD_MAILBOX_GENERAL };
	int mIconState;
	CIconWnd	mIconStateCtrl;
	cdstring	mName;
	cdstring	mServer;
	cdstring	mHierarchy;
	cdstring	mSeparator;
	cdstring	mTotal;
	cdstring	mRecent;
	cdstring	mUnseen;
	cdstring	mDeleted;
	cdstring	mFullSync;
	cdstring	mPartialSync;
	cdstring	mMissingSync;
	cdstring	mSize;
	cdstring	mStatus;
	cdstring	mUIDValidity;
	cdstring	mSynchronise;
	//}}AFX_DATA
	bool	mUseSynchronise;
	bool	mIsDir;

	virtual void	SetMboxList(CMboxList* mbox_list);				// Set mbox list

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropMailboxGeneral)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    	// DDX/DDV support
	//}}AFX_VIRTUAL


private:
	CMboxList*		mMboxList;								// List of selected mboxes

	virtual void	SetMbox(CMbox* mbox);					// Set mbox item

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropMailboxGeneral)
	virtual BOOL OnInitDialog();
	afx_msg void OnCalculateSize();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
