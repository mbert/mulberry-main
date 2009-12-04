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


// Header for CForwardOptionsDialog class


#ifndef __CFORWARDOPTIONSDIALOG__MULBERRY__
#define __CFORWARDOPTIONSDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CMessageWindow.h"

/////////////////////////////////////////////////////////////////////////////
// CForwardOptionsDialog dialog

class CForwardOptionsDialog : public CHelpDialog
{
// Construction
public:
	CForwardOptionsDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(EForwardOptions& forward, EReplyQuote& quote, bool has_selection);

	void	SetDetails(EReplyQuote quote, bool has_selection);									// Set the dialogs info
	void	GetDetails(EReplyQuote& quote, bool& save);							// Set the dialogs return info

// Dialog Data
	//{{AFX_DATA(CForwardOptionsDialog)
	enum { IDD = IDD_FORWARDOPTIONS };
	BOOL	mForwardQuoteOriginal;
	BOOL	mForwardHeaders;
	BOOL	mForwardAttachment;
	BOOL	mForwardRFC822;
	BOOL	mSave;
	int		mQuote;
	//}}AFX_DATA
	bool	mHasSelection;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CForwardOptionsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	static EForwardOptions sForward;
	static bool sForwardInit;

	// Generated message map functions
	//{{AFX_MSG(CForwardOptionsDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnForwardOptionsQuote();
	afx_msg void OnForwardOptionsAttachment();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	void InitOptions();

};

#endif