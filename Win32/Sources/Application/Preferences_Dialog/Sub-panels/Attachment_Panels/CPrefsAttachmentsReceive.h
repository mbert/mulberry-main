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


// CPrefsAttachmentsReceive.h : header file
//

#ifndef __CPREFSATTACHMENTSRECEIVE__MULBERRY__
#define __CPREFSATTACHMENTSRECEIVE__MULBERRY__

#include "CTabPanel.h"

#include "CIconButton.h"
#include "CMIMEMap.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAttachmentsReceive dialog

class CPreferences;

class CPrefsAttachmentsReceive : public CTabPanel
{
	DECLARE_DYNCREATE(CPrefsAttachmentsReceive)

// Construction
public:
	CPrefsAttachmentsReceive();
	~CPrefsAttachmentsReceive();

// Dialog Data
	//{{AFX_DATA(CPrefsAttachmentsReceive)
	enum { IDD = IDD_PREFS_ATTACHMENT_RECEIVE };
	int			mViewDoubleClick;
	int			mAskDownload;
	cdstring	mDefaultDownload;
	int			mAppLaunch;
	BOOL		mLaunchText;
	cdstring	mShellLaunch;
	
	CIconButton	mExplicitMappingBtnCtrl;
	CEdit		mDefaultDownloadCtrl;
	CButton		mDefaultChooseBtnCtrl;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsAttachmentsReceive)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsAttachmentsReceive)
	afx_msg void OnChooseDefaultDownload();
	afx_msg void OnExplicitMapping();
	//}}AFX_MSG

private:
	CMIMEMapVector	mMappings;
	bool			mMapChange;

	DECLARE_MESSAGE_MAP()

};

#endif
