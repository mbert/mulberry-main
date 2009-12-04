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


// CEditIdentityAddress.h : header file
//

#ifndef __CEDITIDENTITYADDRESS__MULBERRY__
#define __CEDITIDENTITYADDRESS__MULBERRY__

#include "CTabPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CEditIdentityAddress dialog

class cdstring;

class CEditIdentityAddress : public CTabPanel
{
	DECLARE_DYNAMIC(CEditIdentityAddress)

// Construction
public:
	CEditIdentityAddress(bool outgoing = false);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditIdentityAddress)
	enum { IDD1 = IDD_IDENTITY_ADDRESS,
			IDD2 = IDD_IDENTITY_OUTGOING_ADDRESS };
	CStatic	mNameTitleCtrl;
	CEdit	mNameCtrl;
	CEdit	mEmailCtrl;
	CStatic	mEmailTitleCtrl;
	CEdit	mTextCtrl;
	BOOL	mActive;
	int		mMultiple;
	CButton mMultipleBtn;
	cdstring	mName;
	cdstring	mEmail;
	cdstring	mText;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data
	virtual void	SetAddressType(bool from, bool reply_to, bool sender,
									bool to, bool cc, bool bcc)
		{ mFrom = from; mReplyTo = reply_to; mSender = sender;
			mTo = to; mCC = cc; mBcc = bcc; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditIdentityAddress)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool mFrom;
	bool mReplyTo;
	bool mSender;
	bool mTo;
	bool mCC;
	bool mBcc;
	bool mSingle;

	// Generated message map functions
	//{{AFX_MSG(CEditIdentityAddress)
	virtual BOOL OnInitDialog();
	afx_msg void OnActive();
	afx_msg void OnSingleAddress();
	afx_msg void OnMultipleAddress();
	//}}AFX_MSG
	
	virtual void SetActive(bool active);
	virtual void SetSingle(bool single);
	virtual void SetItemData(bool active, const cdstring& address);
	virtual void GetItemData(bool& active, cdstring& address);

	DECLARE_MESSAGE_MAP()
};

#endif
