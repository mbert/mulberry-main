/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CPropAdbkGeneral.h : header file
//

#ifndef __CPROPADBKGENERAL__MULBERRY__
#define __CPROPADBKGENERAL__MULBERRY__

#include "CHelpPropertyPage.h"

#include "CIconWnd.h"
#include "CAddressBook.h"

/////////////////////////////////////////////////////////////////////////////
// CPropAdbkGeneral dialog

class CAddressBook;

class CPropAdbkGeneral : public CHelpPropertyPage
{
	DECLARE_DYNCREATE(CPropAdbkGeneral)

// Construction
public:
	CPropAdbkGeneral();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPropAdbkGeneral)
	enum { IDD = IDD_ADBK_GENERAL };
	int mIconState;
	CIconWnd	mIconStateCtrl;
	cdstring	mName;
	cdstring	mServer;
	int			mTotal;
	int			mSingle;
	int			mGroup;
	CButton		mOpenOnStartCtrl;
	BOOL		mOpenOnStart;
	CButton		mNickNameCtrl;
	BOOL		mNickName;
	CButton		mSearchCtrl;
	BOOL		mSearch;
	CButton		mAutoSyncCtrl;
	BOOL		mAutoSync;
	//CButton	mAddCtrl;
	//BOOL	mAdd;
	//}}AFX_DATA
	bool	mDisableAutoSync;

	void	SetAdbkList(CAddressBookList* adbk_list);								// Set mbox list

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropAdbkGeneral)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

private:
	CAddressBookList*	mAdbkList;								// List of selected adbks

	virtual void	SetAdbk(CAddressBook* adbk);			// Set adbk item

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPropAdbkGeneral)
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckOpenAtStart();
	afx_msg void OnCheckNickName();
	afx_msg void OnCheckSearch();
	afx_msg void OnCheckAutoSync();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
