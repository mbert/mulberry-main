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


// Header for CCommonViewOptions class

#ifndef __CCOMMONOPTIONS__MULBERRY__
#define __CCOMMONOPTIONS__MULBERRY__

#include "CHelpPropertyPage.h"

#include "CUserAction.h"
#include "CUserActionOptions.h"

// Classes

class CCommonViewOptions : public CHelpPropertyPage
{
	DECLARE_DYNCREATE(CCommonViewOptions)

// Construction
public:
	CCommonViewOptions();
	CCommonViewOptions(UINT nID);
	~CCommonViewOptions();

	virtual void	SetData(void* data) {}
	virtual bool	UpdateData(void* data) { return true; }

	void	SetCommonViewData(const CUserAction& listPreview,
								const CUserAction& listFullView,
								const CUserAction& itemsPreview,
								const CUserAction& itemsFullView,
								bool is3pane);
	void	GetCommonViewData(CUserAction& listPreview,
								CUserAction& listFullView,
								CUserAction& itemsPreview,
								CUserAction& itemsFullView);

// Dialog Data
	//{{AFX_DATA(CCommonViewOptions)
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CCommonViewOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CUserActionOptions	mListPreview;
	CUserActionOptions	mListFullView;
	CUserActionOptions	mItemsPreview;
	CUserActionOptions	mItemsFullView;
	bool				mIs3Pane;

	void	DisableItems();

	// Generated message map functions
	//{{AFX_MSG(CCommonViewOptions)
	afx_msg void OnChoose1();
	afx_msg void OnChoose2();
	afx_msg void OnChoose3();
	afx_msg void OnChoose4();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
