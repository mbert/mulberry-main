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


// CPropMailboxServer.h : header file
//

#ifndef __CPROPMAILBOXSERVER__MULBERRY__
#define __CPROPMAILBOXSERVER__MULBERRY__

#include "CHelpPropertyPage.h"

#include "CIconWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CPropMailboxServer dialog

class CMboxProtocol;

class CPropMailboxServer : public CHelpPropertyPage
{
	DECLARE_DYNCREATE(CPropMailboxServer)

// Construction
public:
	CPropMailboxServer();
	~CPropMailboxServer();

// Dialog Data
	//{{AFX_DATA(CPropMailboxServer)
	enum { IDD = IDD_MAILBOX_SERVER };
	int 		mIconState;
	CIconWnd	mIconStateCtrl;
	cdstring	mMailboxServerName;
	cdstring	mMailboxServerType;
	cdstring	mMailboxServerGreeting;
	cdstring	mMailboxServerCapability;
	cdstring	mMailboxServerConnection;
	//}}AFX_DATA
	bool		mViewCert;

	void	SetProtocol(CMboxProtocol* server);		// Set fields in dialog

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropMailboxServer)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    	// DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:
	CMboxProtocol*	mProto;

	// Generated message map functions
	//{{AFX_MSG(CPropMailboxServer)
	virtual BOOL OnInitDialog();
	afx_msg void OnViewCert();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
