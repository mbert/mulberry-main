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


// CPrefsAddressCapture.h : header file
//

#ifndef __CPREFSADDRESSCAPTURE__MULBERRY__
#define __CPREFSADDRESSCAPTURE__MULBERRY__

#include "CTabPanel.h"

#include "CPopupButton.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddressCapture dialog

class CPrefsAddressCapture : public CTabPanel
{
	DECLARE_DYNAMIC(CPrefsAddressCapture)

// Construction
public:
	CPrefsAddressCapture();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsAddressCapture)
	enum { IDD = IDD_PREFS_ADDRESS_Capture };
	cdstring		mCaptureAddressBook;
	CEdit			mCaptureAddressBookCtrl;
	CPopupButton	mCapturePopup;
	BOOL			mCaptureAllowEdit;
	BOOL			mCaptureAllowChoice;
	BOOL			mCaptureRead;
	BOOL			mCaptureRespond;
	BOOL			mCaptureFrom;
	BOOL			mCaptureCc;
	BOOL			mCaptureReplyTo;
	BOOL			mCaptureTo;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsAddressCapture)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

			void	InitAddressBookPopup();

	// Generated message map functions
	//{{AFX_MSG(CPrefsAddressCapture)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeAdbk(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
