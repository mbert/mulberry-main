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


// CPropCalendarGeneral.h : header file
//

#ifndef __CPropCalendarGeneral__MULBERRY__
#define __CPropCalendarGeneral__MULBERRY__

#include "CHelpPropertyPage.h"

#include "CIconWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CPropCalendarGeneral dialog

namespace calstore
{
class CCalendarStoreNode;
typedef ptrvector<CCalendarStoreNode> CCalendarStoreNodeList;
class CCalendarProtocol;
}

class CPropCalendarGeneral : public CHelpPropertyPage
{
	DECLARE_DYNCREATE(CPropCalendarGeneral)

// Construction
public:
	CPropCalendarGeneral();
	~CPropCalendarGeneral();

// Dialog Data
	//{{AFX_DATA(CPropCalendarGeneral)
	enum { IDD = IDD_CALENDAR_GENERAL };
	int			mIconState;
	CIconWnd	mIconStateCtrl;
	cdstring	mName;
	cdstring	mServer;
	cdstring	mHierarchy;
	cdstring	mSeparator;
	cdstring	mSize;
	cdstring	mStatus;
	cdstring	mSynchronise;
	//}}AFX_DATA

	bool		mUseSynchronise;
	bool		mIsDir;

	virtual void	SetCalList(calstore::CCalendarStoreNodeList* cal_list);		// Set cal list

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropCalendarGeneral)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    	// DDX/DDV support
	//}}AFX_VIRTUAL


private:
	calstore::CCalendarStoreNodeList*	mCalList;									// List of selected calendars

	virtual void	SetCalendar(calstore::CCalendarStoreNode* node);

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropCalendarGeneral)
	virtual BOOL OnInitDialog();
	afx_msg void OnCalculateSize();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
