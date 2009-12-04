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


// Header for CMessagePaneOptions class

#ifndef __CMESSAGEPANEOPTIONS__MULBERRY__
#define __CMESSAGEPANEOPTIONS__MULBERRY__

#include "CCommonViewOptions.h"

#include "C3PaneOptions.h"

// Classes
class CMailViewOptions;

class CMessagePaneOptions : public CCommonViewOptions
{
	DECLARE_DYNCREATE(CMessagePaneOptions)

// Construction
public:
	CMessagePaneOptions();
	~CMessagePaneOptions();

	void	SetData(const CUserAction& listPreview,
					const CUserAction& listFullView,
					const CUserAction& itemsPreview,
					const CUserAction& itemsFullView,
					const CMailViewOptions& options,
					bool is3pane);
	void	GetData(CUserAction& listPreview,
					CUserAction& listFullView,
					CUserAction& itemsPreview,
					CUserAction& itemsFullView,
					CMailViewOptions& options);

// Dialog Data
	//{{AFX_DATA(CMessagePaneOptions)
	enum { IDD = IDD_3PANE_MAILOPTIONS };
	BOOL	mUseTabs;
	BOOL	mRestoreTabs;
	BOOL	mMessageAddress;
	BOOL	mMessageSummary;
	BOOL	mMessageParts;
	int		mMarkSeen;
	int		mMarkSeenDelay;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMessagePaneOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMessagePaneOptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnUseTabs();
	afx_msg void OnDelayDisable();
	afx_msg void OnDelayEnable();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
