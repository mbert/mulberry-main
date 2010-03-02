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


// CPropCalendarACL.h : header file
//

#ifndef __CPropCalendarACL__MULBERRY__
#define __CPropCalendarACL__MULBERRY__

#include "CHelpPropertyPage.h"
#include "CAdbkList.h"
#include "CAdbkACL.h"
#include "CACLStylePopup.h"
#include "CACLTable.h"
#include "CACLTitleTable.h"
#include "CIconWnd.h"
#include "CPopupButton.h"

#include "CCalendarACL.h"
#include "CCalendarStoreNode.h"

/////////////////////////////////////////////////////////////////////////////
// CPropCalendarACL dialog

class CAddressBook;

class CPropCalendarACL : public CHelpPropertyPage,
						public CListener
{
	DECLARE_DYNCREATE(CPropCalendarACL)

// Construction
public:
	CPropCalendarACL();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPropCalendarACL)
	enum { IDD = IDD_CALENDAR_ACL };
	CPopupButton	mCalendarPopup;
	CIconWnd		mACLFreebusy;
	CIconWnd		mACLRead;
	CIconWnd		mACLWrite;
	CIconWnd		mACLSchedule;
	CIconWnd		mACLCreate;
	CIconWnd		mACLDelete;
	CIconWnd		mACLAdmin;
	CACLTable		mACLTable;
	CACLTitleTable	mACLTitleTable;
	CButton			mNewUserBtn;
	CButton			mDeleteUserBtn;
	CButton			mChangeAllBtn;
	CACLStylePopup	mStylePopup;
	//}}AFX_DATA

	virtual void	SetCalList(calstore::CCalendarStoreNodeList* cal_list);		// Set cal list
	virtual void	DoChangeACL(CCalendarACL::ECalACL, bool set);	// User clicked a button

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropCalendarACL)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	virtual void	ListenTo_Message(long msg, void* param);

private:
	bool			mCanChange;
	bool			mNoChange;

	calstore::CCalendarStoreNodeList*	mCalList;									// List of selected calendars
	calstore::CCalendarStoreNode*		mActionCal;

	virtual void		SetCalendar(calstore::CCalendarStoreNode* cal);		// Set mbox item
	virtual void		SetMyRights(SACLRight rights);		// Set indicators from rights
	virtual void		SetButtons(bool enable, bool alloff);			// Enable/disable buttons

	// Handle actions
	virtual void	OnCalendarPopup(UINT nID);				// Mailbox popup change
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
	//{{AFX_MSG(CPropCalendarACL)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
