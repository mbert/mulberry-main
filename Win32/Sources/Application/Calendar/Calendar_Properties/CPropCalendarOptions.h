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


// CPropCalendarOptions.h : header file
//

#ifndef __CPropCalendarOptions__MULBERRY__
#define __CPropCalendarOptions__MULBERRY__

#include "CHelpPropertyPage.h"

#include "CIconWnd.h"
#include "CIdentityPopup.h"

/////////////////////////////////////////////////////////////////////////////
// CPropCalendarOptions dialog

class CIdentity;

namespace calstore
{
class CCalendarStoreNode;
typedef ptrvector<CCalendarStoreNode> CCalendarStoreNodeList;
class CCalendarProtocol;
}

class CPropCalendarOptions : public CHelpPropertyPage
{
	DECLARE_DYNCREATE(CPropCalendarOptions)

// Construction
public:
	CPropCalendarOptions();
	~CPropCalendarOptions();

// Dialog Data
	//{{AFX_DATA(CPropCalendarOptions)
	enum { IDD = IDD_CALENDAR_OPTIONS };
	int mIconState;
	CIconWnd mIconStateCtrl;
	int		mSubscribe;
	CButton	mSubscribeCtrl;
	int		mTieIdentity;
	CButton	mTieIdentityCtrl;
	CIdentityPopup mIdentityPopup;

	cdstring	mCurrentIdentity;
	bool		mAllDir;

	//}}AFX_DATA

	virtual void	SetCalList(calstore::CCalendarStoreNodeList* cal_list);		// Set cal list

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropCalendarOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    	// DDX/DDV support
	//}}AFX_VIRTUAL


private:
	calstore::CCalendarStoreNodeList*	mCalList;									// List of selected calendars

	void	SetTiedIdentity(bool add);				// Add/remove tied identity

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropCalendarOptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnSubscribe();
	afx_msg void OnTieIdentity();
	afx_msg void OnIdentityPopup(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
