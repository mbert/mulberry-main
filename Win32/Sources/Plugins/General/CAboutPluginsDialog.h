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

// CAboutPluginsDialog.h : header file
//

#ifndef __CABOUTPLUGINSDIALOG__MULBERRY__
#define __CABOUTPLUGINSDIALOG__MULBERRY__

#include "CHelpDialog.h"
#include <WIN_LTextHierTable.h>
#include "CPlugin.h"

/////////////////////////////////////////////////////////////////////////////
// CAboutPluginsDialog dialog

class CAboutPluginsDialog;

class CAboutPluginsTable : public LTextHierTable
{
public:
	CAboutPluginsTable();
	void SetDlog(CAboutPluginsDialog* dlog)
		{ mDlog = dlog; }
protected:
	CAboutPluginsDialog* mDlog;
	virtual void DoSelectionChanged();
};

class CAboutPluginsDialog : public CHelpDialog
{
	friend class CAboutPluginsTable;

// Construction
public:
	CAboutPluginsDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAboutPluginsDialog)
	enum { IDD = IDD_ABOUTPLUGINS };
	CButton	mRegisterBtn;
	CAboutPluginsTable	mList;
	CStatic	mName;
	CStatic	mVersion;
	CStatic	mType;
	CStatic	mManufacturer;
	CStatic	mDescription;
	CStatic	mMode;
	//}}AFX_DATA
	CPluginList		mHierPlugins;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutPluginsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

private:
	virtual void	ShowPluginInfo(const CPlugin* plugin);

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAboutPluginsDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnAboutPluginsRegister();
	afx_msg void OnClickAboutPluginsList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
