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


// CPropAdbkACL.h : header file
//

#ifndef __CPROPADBKACL__MULBERRY__
#define __CPROPADBKACL__MULBERRY__

#include "CHelpPropertyPage.h"
#include "CAddressBook.h"
#include "CAdbkACL.h"
#include "CACLStylePopup.h"
#include "CACLTable.h"
#include "CACLTitleTable.h"
#include "CIconWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CPropAdbkACL dialog

class CAddressBook;

class CPropAdbkACL : public CHelpPropertyPage,
						public CListener
{
	DECLARE_DYNCREATE(CPropAdbkACL)

// Construction
public:
	CPropAdbkACL();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPropAdbkACL)
	enum { IDD = IDD_ADBK_ACL };
	CIconWnd		mACLLookup;
	CIconWnd		mACLRead;
	CIconWnd		mACLWrite;
	CIconWnd		mACLCreate;
	CIconWnd		mACLDelete;
	CIconWnd		mACLAdmin;
	CACLTable		mACLTable;
	CACLTitleTable	mACLTitleTable;
	CButton			mNewUserBtn;
	CButton			mDeleteUserBtn;
	CACLStylePopup	mStylePopup;
	//}}AFX_DATA

	virtual void	SetAdbkList(CAddressBookList* adbk_list);								// Set mbox list
	virtual void	DoChangeACL(CAdbkACL::EAdbkACL acl, bool set);	// User clicked a button

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropAdbkACL)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	virtual void	ListenTo_Message(long msg, void* param);

private:
	CAddressBookList*	mAdbkList;								// List of selected adbks
	bool				mCanChange;

	virtual void		SetAdbk(CAddressBook* adbk);		// Set adbk item
	virtual void		SetMyRights(SACLRight rights);		// Set indicators from rights
	virtual void		SetButtons(bool enable);			// Enable/disable buttons

	// Handle actions
	virtual void	OnNewUser(void);						// Do new user dialog
	virtual void	OnDeleteUser(void);						// Delete user from list
	virtual void	OnStylePopup(void);						// Handle popup command
	virtual void	DoChangeRights(SACLRight rights);		// Change rights

	virtual void	OnSaveStyle(void);						// Do new style
	virtual void	OnDeleteStyle(void);					// Delete style
	virtual void	OnChangeStyle(UINT nID);				// Change style

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPropAdbkACL)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
