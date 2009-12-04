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


// Header for CCreateCalendarDialog class

#ifndef __CCREATECALENDARDIALOG__MULBERRY__
#define __CCREATECALENDARDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CCmdEdit.h"
#include "CINETAccount.h"
#include "CPopupButton.h"

#include "cdstring.h"

// Constants

// Type

class CCreateCalendarDialog : public CHelpDialog
{
public:
	struct SCreateCalendar
	{
		cdstring	new_name;
		cdstring	parent;
		cdstring	account;
		CINETAccount::EINETServerType	account_type;
		bool		use_wd;
		bool		subscribe;
		bool		directory;
	};

	static bool		PoseDialog(SCreateCalendar& details);

					CCreateCalendarDialog(CWnd* pParent = NULL);
	virtual 		~CCreateCalendarDialog();

protected:
// Dialog Data
	//{{AFX_DATA(CCreateCalendarDialog)
	enum { IDD = IDD_CALENDAR_CREATE };
	CButton			mCalendar;
	CButton			mDirectory;
	CCmdEdit		mCalendarName;
	CButton			mSubscribe;
	CButton			mFullPath;
	CButton			mUseDirectory;
	CStatic			mHierarchy;
	CStatic			mAccount;
	CPopupButton	mAccountPopup;
	//}}AFX_DATA

	bool				mUseSubscribe;
	bool				mHasLocal;
	bool				mHasRemote;
	
	SCreateCalendar*	mData;

// Overrides
			void	InitControls();
			void 	InitAccountMenu();

			void	SetDetails(SCreateCalendar* create);		// Set the dialogs info
			void	GetDetails(SCreateCalendar* result);		// Get the dialogs return info

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateCalendarDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CCreateCalendarDialog)
	afx_msg void OnCalendar();
	afx_msg void OnDirectory();
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};

#endif
