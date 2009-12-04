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


// CEditGroupDialog.h : header file
//

#ifndef __CEDITGROUPDIALOG__MULBERRY__
#define __CEDITGROUPDIALOG__MULBERRY__

#include "CHelpDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CEditGroupDialog dialog

class CGroup;

class CEditGroupDialog : public CHelpDialog
{
// Construction
public:
	CEditGroupDialog(CWnd* pParent = NULL);   // standard constructor

	virtual void SetFields(const CGroup* grp);
	virtual bool GetFields(CGroup* grp);

// Dialog Data
	//{{AFX_DATA(CEditGroupDialog)
	enum { IDD = IDD_EDITGROUP };
	cdstring	mNickName;
	cdstring	mGroupName;
	cdstring	mAddressList;
	CEdit		mAddressListCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditGroupDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditGroupDialog)
	afx_msg void OnGroupEditSort();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
