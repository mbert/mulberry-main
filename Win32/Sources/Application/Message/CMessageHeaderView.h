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


// CMessageHeaderView.h : header file
//

#ifndef __CMESSAGEHEADERVIEW__MULBERRY__
#define __CMESSAGEHEADERVIEW__MULBERRY__

#include "CContainerView.h"
#include "CWndAligner.h"

#include "CFieldDisplay.h"
#include "CGrayBackground.h"
#include "CIconButton.h"
#include "CIconWnd.h"
#include "CPopupButton.h"
#include "CTwister.h"

/////////////////////////////////////////////////////////////////////////////
// CMessageHeaderView view

class CMessageHeaderView : public CContainerView, public CWndAligner
{
	friend class CMessageWindow;

protected:
	CMessageHeaderView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CMessageHeaderView)

// Implementation
protected:
	virtual ~CMessageHeaderView();

public:
	static void		UpdateUIPos(void);

	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command

	// Generated message map functions
protected:
	//{{AFX_MSG(CMessageHeaderView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

private:
	CGrayBackground		mBackground;				// Background

	CGrayBackground		mFromMove;					// From move
	CStatic				mFromTitle;					// From title
	CTwister			mFromTwister;				// From twist button
	CFieldDisplay		mFromField;					// From text
	CGrayBackground		mToMove;					// To move
	CStatic				mToTitle;					// To title
	CTwister			mToTwister;					// To twist button
	CFieldDisplay		mToField;					// To text
	CGrayBackground		mCCMove;					// CC move
	CStatic				mCCTitle;					// CC title
	CTwister			mCCTwister;					// CC twist button
	CFieldDisplay		mCCField;					// CC text
	CGrayBackground		mSubjectMove;				// Subject/date move
	CStatic				mSubjectTitle;				// Subject title
	CFieldDisplay		mSubjectField;				// Subject text
	CStatic				mDateTitle;					// Date title
	CFieldDisplay		mDateField;					// Date text
	CStatic				mNumberField;				// Message number

	CGrayBackground		mPartsMove;					// Header for parts table
	CStatic				mPartsTitle;				// Parts title
	CTwister			mPartsTwister;				// Parts twist button
	CStatic				mPartsField;				// Number of parts
	CIconButton			mFlatHierarchyBtn;			// Flat/hierarchy button
	CIconWnd			mAttachments;				// Indicate attachments
	CIconWnd			mAlternative;				// Indicate alternatives
	CIconButton			mHeaderBtn;					// Show header button
	CPopupButton		mTextFormat;				// Text format	 popup menu
	CIconButton			mFontIncreaseBtn;			// Increase font size
	CIconButton			mFontDecreaseBtn;			// Decrease font size
	CStatic				mFontScaleField;			// Font scale value
	CPopupButton		mQuoteDepthPopup;			// Quote depth popup menu

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
