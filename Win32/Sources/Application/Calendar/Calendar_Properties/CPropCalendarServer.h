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


// CPropCalendarServer.h : header file
//

#ifndef __CPropCalendarServer__MULBERRY__
#define __CPropCalendarServer__MULBERRY__

#include "CHelpPropertyPage.h"

#include "CIconWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CPropCalendarServer dialog

namespace calstore
{
class CCalendarStoreNode;
typedef ptrvector<CCalendarStoreNode> CCalendarStoreNodeList;
class CCalendarProtocol;
}

class CPropCalendarServer : public CHelpPropertyPage
{
	DECLARE_DYNCREATE(CPropCalendarServer)

// Construction
public:
	CPropCalendarServer();
	~CPropCalendarServer();

// Dialog Data
	//{{AFX_DATA(CPropCalendarServer)
	enum { IDD = IDD_CALENDAR_SERVER };
	int mIconState;
	CIconWnd	mIconStateCtrl;
	cdstring	mName;
	cdstring	mType;
	cdstring	mGreeting;
	cdstring	mCapability;
	cdstring	mConnection;
	//}}AFX_DATA
	bool		mViewCert;

	void	SetProtocol(calstore::CCalendarProtocol* proto);		// Set fields in dialog

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropCalendarServer)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    	// DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:
	calstore::CCalendarProtocol*	mProto;

	// Generated message map functions
	//{{AFX_MSG(CPropCalendarServer)
	virtual BOOL OnInitDialog();
	afx_msg void OnViewCert();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
