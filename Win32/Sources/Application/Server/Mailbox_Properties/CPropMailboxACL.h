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


// CPropMailboxACL.h : header file
//

#ifndef __CPROPMAILBOXACL__MULBERRY__
#define __CPROPMAILBOXACL__MULBERRY__

#include "CHelpPropertyPage.h"
#include "CListener.h"

#include "CIconWnd.h"
#include "CMboxACL.h"
#include "CACLStylePopup.h"
#include "CACLTable.h"
#include "CACLTitleTable.h"
#include "CPopupButton.h"

/////////////////////////////////////////////////////////////////////////////
// CPropMailboxACL dialog

class CMbox;
class CMboxList;

class CPropMailboxACL : public CHelpPropertyPage,
						public CListener
{
	DECLARE_DYNCREATE(CPropMailboxACL)

// Construction
public:
	CPropMailboxACL();
	~CPropMailboxACL();

// Dialog Data
	//{{AFX_DATA(CPropMailboxACL)
	enum { IDD = IDD_MAILBOX_ACL };
	CPopupButton	mMailboxPopup;
	CIconWnd		mACLLookup;
	CIconWnd		mACLRead;
	CIconWnd		mACLSeen;
	CIconWnd		mACLWrite;
	CIconWnd		mACLInsert;
	CIconWnd		mACLPost;
	CIconWnd		mACLCreate;
	CIconWnd		mACLDelete;
	CIconWnd		mACLAdmin;
	CACLTitleTable	mACLTitleTable;
	CACLTable		mACLTable;
	CButton			mNewUserBtn;
	CButton			mDeleteUserBtn;
	CACLStylePopup	mStylePopup;
	CButton			mChangeAllBtn;
	//}}AFX_DATA

	virtual void	SetMboxList(CMboxList* mbox_list);		// Set mbox list
	virtual void	DoChangeACL(CMboxACL::EMboxACL acl, bool set);	// User clicked a button

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropMailboxACL)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	virtual void	ListenTo_Message(long msg, void* param);

private:
	CMboxList*		mMboxList;									// List of selected mboxes
	bool			mCanChange;
	bool			mNoChange;
	
	CMbox*			mActionMbox;

	virtual void		SetMbox(CMbox* mbox);					// Set mbox item
	virtual void		SetMyRights(SACLRight rights);			// Set indicators from rights
	virtual void		SetButtons(bool enable, bool alloff);	// Enable/disable buttons

	// Handle actions
	virtual void	OnMailboxPopup(UINT nID);				// Mailbox popup change
	virtual void	OnNewUser(void);						// Do new user dialog
	virtual void	OnDeleteUser(void);						// Delete user from list
	virtual void	OnStylePopup(void);						// Handle popup command
	virtual void	DoChangeRights(SACLRight rights);		// Change rights
	virtual	void	OnChangeAll(void);						// Apply All button clicked

	virtual void	OnSaveStyle(void);						// Do new style
	virtual void	OnDeleteStyle(void);					// Delete style
	virtual void	OnChangeStyle(UINT nID);				// Change style

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropMailboxACL)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
