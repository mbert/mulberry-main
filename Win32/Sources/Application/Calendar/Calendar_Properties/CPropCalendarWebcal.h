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


// CPropCalendarWebcal.h : header file
//

#ifndef __CPropCalendarWebcal__MULBERRY__
#define __CPropCalendarWebcal__MULBERRY__

#include "CHelpPropertyPage.h"

#include "CIconWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CPropCalendarWebcal dialog

namespace calstore
{
class CCalendarStoreNode;
typedef ptrvector<CCalendarStoreNode> CCalendarStoreNodeList;
class CCalendarProtocol;
}

class CPropCalendarWebcal : public CHelpPropertyPage
{
	DECLARE_DYNCREATE(CPropCalendarWebcal)

// Construction
public:
	CPropCalendarWebcal();
	~CPropCalendarWebcal();

// Dialog Data
	//{{AFX_DATA(CPropCalendarWebcal)
	enum { IDD = IDD_CALENDAR_WEBCAL };
	int mIconState;
	CIconWnd	mIconStateCtrl;
	cdstring	mRemoteURL;
	int			mAllowChanges;
	int			mAutoPublish;
	int			mRefreshGroup;
	int			mRefreshInterval;

	//}}AFX_DATA
	bool		mMultiple;
	cdstring	mRevertURL;

	virtual void	SetCalList(calstore::CCalendarStoreNodeList* cal_list);		// Set cal list

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropCalendarWebcal)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    	// DDX/DDV support
	//}}AFX_VIRTUAL


private:
	calstore::CCalendarStoreNodeList*	mCalList;									// List of selected calendars

	void	SetTiedIdentity(bool add);				// Add/remove tied identity

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropCalendarWebcal)
	virtual BOOL OnInitDialog();
	afx_msg void OnRevertURL();
	afx_msg void OnAllowChanges();
	afx_msg void OnAutoPublish();
	afx_msg void OnRefreshNever();
	afx_msg void OnRefreshEvery();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
