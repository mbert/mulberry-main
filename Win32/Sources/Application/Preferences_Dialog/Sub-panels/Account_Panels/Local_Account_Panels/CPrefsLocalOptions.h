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


// CPrefsLocalOptions.h : header file
//

#ifndef __CPREFSLOCALOPTIONS__MULBERRY__
#define __CPREFSLOCALOPTIONS__MULBERRY__

#include "CTabPanel.h"

#include "CPopupButton.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsLocalOptions dialog

class CPrefsLocalOptions : public CTabPanel
{
	DECLARE_DYNAMIC(CPrefsLocalOptions)

// Construction
public:
	CPrefsLocalOptions();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsLocalOptions)
	enum { IDD = IDD_PREFS_ACCOUNT_Local_Options };
	int				mUseDefault;
	CButton			mUseDefaultCtrl;
	CButton			mSpecifyCtrl;
	cdstring		mPath;
	CEdit			mPathCtrl;
	CButton			mRelativeCtrl;
	CButton			mAbsoluteCtrl;
	CPopupButton	mFileFormatPopup;
	//}}AFX_DATA
	bool				mLocalAddress;
	bool				mDisconnected;

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

			void	SetLocalAddress()
		{ mLocalAddress = true; }
			void	SetDisconnected()
		{ mDisconnected = true; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsLocalOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsLocalOptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnPrefsAccountLocalUseDefault();
	afx_msg void OnPrefsAccountLocalSpecify();
	afx_msg void OnPrefsAccountLocalChoose();
	afx_msg void OnPrefsAccountLocalRelative();
	afx_msg void OnPrefsAccountLocalAbsolute();
	afx_msg void OnFileFormatPopup(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
			void	SetUseLocal(bool use);
			void	SetPath(const char* path);
			void	DoChooseLocalFolder();				// Choose local folder using browser
			void	SetRelative();
			void	SetAbsolute();
};

#endif
