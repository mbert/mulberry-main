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


// CSpacebarEditView.h : header file
//

#ifndef __CSPACEBAREDITVIEW__MULBERRY__
#define __CSPACEBAREDITVIEW__MULBERRY__

#include "CCmdEditView.h"

/////////////////////////////////////////////////////////////////////////////
// CSpacebarEditView window

class CMessageView;
class CMessageWindow;

class CSpacebarEditView : public CCmdEditView
{
	friend class CSpacebarEdit;

	DECLARE_DYNCREATE(CSpacebarEditView)

// Construction
public:
	CSpacebarEditView();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpacebarEditView)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSpacebarEditView();

	virtual void SetMessageView(CMessageView* msgView)
					{ mMsgView = msgView; }
	virtual void SetMessageWindow(CMessageWindow* msgWindow)
					{ mMsgWindow = msgWindow; }

	// Common updaters
	afx_msg void	OnUpdateAlways(CCmdUI* pCmdUI);

	virtual bool 	HandleChar(UINT nChar, UINT nRepCnt, UINT nFlags);

protected:
	// Printing
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	
	// Generated message map functions
protected:

private:
	CMessageView*	mMsgView;
	CMessageWindow*	mMsgWindow;
	bool			mAdjustedPrintMargins;

			void	AdjustPrintMargins(CDC* pDC, CPrintInfo* pInfo);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
