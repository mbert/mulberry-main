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


// CBrowseMailboxDialog.h : header file
//

#ifndef __CBROWSEMAILBOXDIALOG__MULBERRY__
#define __CBROWSEMAILBOXDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CMboxList.h"
#include "CServerBrowseTable.h"

/////////////////////////////////////////////////////////////////////////////
// CBrowseMailboxDialog dialog

class CBrowseMailboxDialog : public CHelpDialog
{
// Construction
public:
	CBrowseMailboxDialog(bool browse_copy, CWnd* pParent = NULL);   // standard constructor

	static bool	PoseDialog(bool open_mode, bool sending, CMbox*& mbox, bool& set_as_default);
	static bool	PoseDialog(CMboxList& mbox_list);

// Dialog Data
	//{{AFX_DATA(CBrowseMailboxDialog)
	enum { IDD1 = IDD_SERVERBROWSE, IDD2 = IDD_SERVERBROWSECOPY };
	CServerBrowseTable	mTable;
	CButton				mOKButton;
	CButton				mCreateButton;
	BOOL				mSetAsDefault;
	CButton				mSetAsDefaultBtn;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBrowseMailboxDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool mBrowseCopy;
	bool mOpenMode;
	bool mSending;
	bool mMultiple;
	CMbox* mMbox;
	CMboxList mMboxList;
	int mOldWidth;
	int mOldHeight;

	void	SetOpenMode(bool mode)
		{ mOpenMode = mode; }
	void	SetSending(bool sending)
		{ mSending = sending; }
	void	SetMultipleSelection()				// Turn on multiple selection
		{ mMultiple = true; }
	CMbox*	GetSelectedMbox(void)				// Get selected mbox
		{ return mMbox; }
	void	GetSelectedMboxes(CMboxList& mbox_list)	// Get all selected mboxes
		{ mbox_list = mMboxList; }

	// Generated message map functions
	//{{AFX_MSG(CBrowseMailboxDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowseMailboxNoCopy();
	afx_msg void OnBrowseMailboxCreate();
	afx_msg void OnDestroy(void);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	virtual void	ResetState(void);						// Reset state from prefs
	virtual void	SaveState(void);						// Save state in prefs
};

#endif
