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


// CSplashBox.h : header file
//

#ifndef __CSPLASHBOX__MULBERRY__
#define __CSPLASHBOX__MULBERRY__

/////////////////////////////////////////////////////////////////////////////
// CSplashBox dialog

class CSplashBox : public CDialog
{
// Construction
public:
	CSplashBox();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSplashBox)
	enum { IDD_SMALL = IDD_SPLASHBOX,
			IDD_LARGE =  IDD_SPLASHBOXBIG };
	cdstring	mLicensee;
	cdstring	mSerial;
	cdstring	mVersion;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSplashBox)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSplashBox)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
