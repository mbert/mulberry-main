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


// CNewACLDialog.h : header file
//

#ifndef __CNEWACLDIALOG__MULBERRY__
#define __CNEWACLDIALOG__MULBERRY__

#include "CHelpDialog.h"
#include "CACLStylePopup.h"
#include "CIconButton.h"

#include "CAdbkACL.h"
#include "CCalendarACL.h"
#include "CMboxACL.h"

/////////////////////////////////////////////////////////////////////////////
// CNewACLDialog dialog

class CNewACLDialog : public CHelpDialog
{
// Construction
public:
	CNewACLDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNewACLDialog)
	enum { IDD = IDD_NEW_ACL };
	CACLStylePopup	mStylePopup;
	CIconButton		mAdminBtn;
	CIconButton		mDeleteBtn;
	CIconButton		mCreateBtn;
	CIconButton		mPostBtn;
	CIconButton		mScheduleBtn;
	CIconButton		mInsertBtn;
	CIconButton		mWriteBtn;
	CIconButton		mSeenBtn;
	CIconButton		mReadBtn;
	CIconButton		mLookupBtn;
	CEdit			mUIDList;
	//}}AFX_DATA
	cdstrvect		mResult;
	SACLRight		mRights;

	virtual void				SetDetails(bool mbox, bool adbk, bool cal);						// Specify mbox or adbk or cal version
	virtual CMboxACLList*		GetDetailsMbox(void);			// Get details from dialog
	virtual CAdbkACLList*		GetDetailsAdbk(void);			// Get details from dialog
	virtual CCalendarACLList*	GetDetailsCal(void);			// Get details from dialog

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewACLDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

private:
	bool			mMbox;
	bool			mAdbk;
	bool			mCalendar;

	virtual void		SetACL(SACLRight rights);				// Set buttons from rights
	virtual SACLRight	GetRights(void);						// Get rights from buttons

	// Handle actions
	virtual void	OnStylePopup(void);						// Handle popup command
	virtual void	OnACLLookup(void);						// Do ACL Lookup button
	virtual void	OnACLRead(void);						// Do ACL Read button
	virtual void	OnACLSeen(void);						// Do ACL Seen button
	virtual void	OnACLWrite(void);						// Do ACL Write button
	virtual void	OnACLInsert(void);						// Do ACL Insert button
	virtual void	OnACLSchedule(void);					// Do ACL Schedule button
	virtual void	OnACLPost(void);						// Do ACL Post button
	virtual void	OnACLCreate(void);						// Do ACL Create button
	virtual void	OnACLDelete(void);						// Do ACL Delete button
	virtual void	OnACLAdmin(void);						// Do ACL Admin button

	virtual void	OnSaveStyle(void);						// Do new style
	virtual void	OnDeleteStyle(void);					// Delete style
	virtual void	OnChangeStyle(UINT nID);				// Change style

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNewACLDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
