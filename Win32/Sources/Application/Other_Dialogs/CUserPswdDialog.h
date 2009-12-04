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


// CUserPswdDialog.h : header file
//

#ifndef __CUSERPSWDDIALOG__MULBERRY__
#define __CUSERPSWDDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CIconWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CUserPswdDialog dialog

class CUserPswdDialog : public CHelpDialog
{
// Construction
public:
	CUserPswdDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(cdstring& uid, cdstring& pswd, bool save_user, bool save_pswd,
								const cdstring& title, const cdstring& server_ip, const cdstring& method, bool secure, int icon);

// Dialog Data
	//{{AFX_DATA(CUserPswdDialog)
	enum { IDD = IDD_PASSWORD };
	cdstring	m_Description;
	cdstring	m_ServerName;
	cdstring	m_Method;
	cdstring	m_Secure;
	cdstring	m_UserID;
	cdstring	m_Password;
	CStatic		mCapsLock;
	int			m_IconID;
	BOOL    	m_SaveUser;
	BOOL    	m_SavePswd;
	bool 		mCaps;
	//}}AFX_DATA

	CIconWnd	mIcon;
	UINT		mTimerID;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUserPswdDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUserPswdDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif
