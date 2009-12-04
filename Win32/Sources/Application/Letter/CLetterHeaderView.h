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


// CLetterHeaderView.h : header file
//

#ifndef __CLETTERHEADERVIEW__MULBERRY__
#define __CLETTERHEADERVIEW__MULBERRY__

#include "CContainerView.h"
#include "CWndAligner.h"

#include "CAddressDisplay.h"
#include "CGrayBackground.h"
#include "CIdentityPopup.h"
#include "CIconButton.h"
#include "CIconWnd.h"
#include "CMailboxPopup.h"
#include "CNumberEdit.h"
#include "CTextButton.h"
#include "CTwister.h"

/////////////////////////////////////////////////////////////////////////////
// CLetterHeaderView view

class CLetterHeaderView : public CContainerView, public CWndAligner
{
	friend class CLetterWindow;
	friend class CLetterTextEditView;

protected:
	CLetterHeaderView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CLetterHeaderView)

// Implementation
protected:
	virtual ~CLetterHeaderView();

public:
	static void		UpdateUIPos();

	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command

	// Generated message map functions
protected:
	//{{AFX_MSG(CLetterHeaderView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

private:
	CGrayBackground		mBackground;				// Background

	CGrayBackground		mHeaderMove;				// From move
	CGrayBackground		mFromMove;					// From move
	CStatic				mFromTitle;					// Identity title
	CIdentityPopup		mIdentityPopup;				// Identity popup
	CIconButton			mIdentityEditBtn;			// Edit identity button
	CGrayBackground		mRecipientMove;				// Recipient move
	CStatic				mRecipientTitle;			// Recipient title
	CStatic				mRecipientText;				// Recipient text

	CGrayBackground		mToMove;					// To move
	CStatic				mToTitle;					// To title
	CTwister			mToTwister;					// To twist button
	CAddressDisplay		mToField;					// To text
	CGrayBackground		mCCMove;					// CC move
	CStatic				mCCTitle;					// CC title
	CTwister			mCCTwister;					// To twist button
	CAddressDisplay		mCCField;					// CC text
	CGrayBackground		mBCCMove;					// BCC move
	CStatic				mBCCTitle;					// BCC title
	CTwister			mBCCTwister;				// BCC twist button
	CAddressDisplay		mBCCField;					// BCC text
	CGrayBackground		mSubjectMove;				// Subject move
	CStatic				mSubjectTitle;				// Subject title
	CCmdEdit			mSubjectField;				// Subject text
	CIconWnd			mSentIcon;					// Indicate send

	CGrayBackground		mPartsMove;					// Parts move
	CStatic				mPartsTitle;				// Parts title
	CStatic				mAttachmentsTitle;			// Attachments title
	CTwister			mPartsTwister;				// Parts twist button
	CNumberEdit			mPartsField;				// Number of parts
	CIconWnd			mAttachments;				// Indicate send
	CStatic				mCopyToTitle;				// Copy to title
	CMailboxPopup		mCopyTo;					// Copy to combo

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
