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


// Header for CVisualProgress class

#ifndef __CVISUALPROGRESS__MULBERRY__
#define __CVISUALPROGRESS__MULBERRY__

#include "CHelpDialog.h"
#include "CProgress.h"

class CProgressDialog : public CHelpDialog, public CProgress
{
// Construction
public:
	CProgressDialog(CWnd* pParent = NULL);   // standard constructor

	static CProgressDialog* StartDialog(const cdstring& rsrc);
	static void StopDialog(CProgressDialog* dlog);

	virtual void	SetDescriptor(const cdstring& title)
						{ mTitle = title; }
	virtual void	SetCount(unsigned long count) 
						{ mBarPane.SetCount(count); }
	virtual void	SetTotal(unsigned long total) 
						{ mBarPane.SetTotal(total); }
	virtual void	SetIndeterminate()
						{ mBarPane.SetIndeterminate(true); }

// Dialog Data
	//{{AFX_DATA(CCancelDialog)
	enum { IDD = IDD_PROGRESSDIALOG };
	cdstring			mTitle;
	CBarPane			mBarPane;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCancelDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCancelDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
#endif
