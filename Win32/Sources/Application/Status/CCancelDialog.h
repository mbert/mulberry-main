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


// CCancelDialog.h : header file
//

#ifndef __CCANCELDIALOG__MULBERRY__
#define __CCANCELDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CMacProgressCtrl.h"

#include "cdstring.h"

/////////////////////////////////////////////////////////////////////////////
// CCancelDialog dialog

class CCancelDialog : public CHelpDialog
{
// Construction
public:
	CCancelDialog(CWnd* pParent = NULL);   // standard constructor

	void	SetBusyDescriptor(const cdstring& desc);
	void	SetTime(unsigned long secs);

	void	StartModal();
	bool	ModalLoop();
	void	StopModal();

// Dialog Data
	//{{AFX_DATA(CCancelDialog)
	enum { IDD = IDD_CANCELDIALOG };
	CStatic		mTime;
	cdstring	mDescriptor;
	//}}AFX_DATA
	CMacProgressCtrl	mProgress;
	unsigned long	mLastSecs;
	bool			mCancelled;

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
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
#endif
