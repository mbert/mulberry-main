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


// CPropMailboxQuota.h : header file
//

#ifndef __CPROPMAILBOXQUOTA__MULBERRY__
#define __CPROPMAILBOXQUOTA__MULBERRY__

#include "CHelpPropertyPage.h"
#include "CQuotaTable.h"
#include "CSimpleTitleTable.h"

/////////////////////////////////////////////////////////////////////////////
// CPropMailboxQuota dialog

class CMbox;
class CMboxList;

class CPropMailboxQuota : public CHelpPropertyPage
{
	DECLARE_DYNCREATE(CPropMailboxQuota)

// Construction
public:
	CPropMailboxQuota();
	~CPropMailboxQuota();

// Dialog Data
	//{{AFX_DATA(CPropMailboxQuota)
	enum { IDD = IDD_MAILBOX_QUOTA };
	CSimpleTitleTable	mTitles;
	CQuotaTable			mTable;
	//}}AFX_DATA

	virtual void	SetMboxList(CMboxList* mbox_list);				// Set mbox list

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropMailboxQuota)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


private:
	CMboxList*		mMboxList;								// List of selected mboxes

	virtual void	SetMbox(CMbox* mbox);					// Set mbox item

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropMailboxQuota)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
