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

// CSaveDraftDialog.h : header file
//

#ifndef __CSAVEDRAFTDIALOG__MULBERRY__
#define __CSAVEDRAFTDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "cdstring.h"
#include "CMailboxPopup.h"

/////////////////////////////////////////////////////////////////////////////
// CSaveDraftDialog dialog

class CSaveDraftDialog : public CHelpDialog
{
// Construction
public:
	struct SSaveDraft
	{
		bool		mFile;
		cdstring	mMailboxName;
		
		SSaveDraft() { mFile = true; mMailboxName = "\1"; }
	};

	CSaveDraftDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(SSaveDraft& details, bool allow_file);

	void	SetDetails(bool allow_file);			// Set the dialogs info
	bool	GetDetails();							// Set the dialogs return info

// Dialog Data
	//{{AFX_DATA(CSaveDraftDialog)
	enum { IDD = IDD_SAVEDRAFTCHOOSE };
	int		mFile;
	CMailboxPopup	mMailboxPopup;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSaveDraftDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool mAllowFile;
	cdstring mMailboxName;

	// Generated message map functions
	//{{AFX_MSG(CSaveDraftDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSaveDraftToFile();
	afx_msg void OnSaveDraftToMailbox();
	afx_msg void OnCopyToPopup(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	static SSaveDraft sDetails;
};

#endif