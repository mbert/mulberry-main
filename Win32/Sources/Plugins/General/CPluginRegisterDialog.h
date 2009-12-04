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

// CPluginRegisterDialog.h : header file
//

#ifndef __CPLUGINREGISTERDIALOG__MULBERRY__
#define __CPLUGINREGISTERDIALOG__MULBERRY__

#include "CHelpDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CPluginRegisterDialog dialog

class CPlugin;

class CPluginRegisterDialog : public CHelpDialog
{
// Construction
public:
	CPluginRegisterDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPluginRegisterDialog)
	enum { IDD = IDD_PLUGINREGISTER };
	CButton	mRegisterBtn;
	CButton	mCancelBtn;
	CButton	mRemoveBtn;
	CButton	mRunDemoBtn;
	cdstring	mName;
	cdstring	mVersion;
	cdstring	mType;
	cdstring	mManufacturer;
	cdstring	mDescription;
	cdstring	mRegKey;
	CEdit	mRegKeyCtrl;
	//}}AFX_DATA

	virtual void	 SetPlugin(const CPlugin& plugin, bool allow_demo, bool allow_delete);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPluginRegisterDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool mAllowDemo;
	bool mAllowDelete;

	// Generated message map functions
	//{{AFX_MSG(CPluginRegisterDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnRemove();
	afx_msg void OnRunAsDemo();
	afx_msg void OnChangeEntry();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
