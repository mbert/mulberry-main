/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CSearchWindow.h

#ifndef __CSEARCHWINDOW__MULBERRY__
#define __CSEARCHWINDOW__MULBERRY__

#include "CSearchBase.h"
#include "CWndAligner.h"

#include "CMboxRefList.h"

#include "CCommanderProtect.h"
#include "CGrayBackground.h"
#include "CGrayBorder.h"
#include "CIconTextTable.h"
#include "CSearchListPanel.h"
#include "CPopupButton.h"
#include "CTwister.h"

// Classes
class CMbox;
class CMboxList;
class CSearchCriteria;
class CSearchItem;

typedef std::vector<CSearchCriteria*> CSearchCriteriaList;

class CSearchWindow : public CView, public CSearchBase, public CWndAligner
{
	DECLARE_DYNCREATE(CSearchWindow)

	friend class CSearchEngine;
	friend class CSearchListPanel;

public:
	static CSearchWindow*	sSearchWindow;
	static CMultiDocTemplate*		sSearchDocTemplate;

				CSearchWindow();
	virtual		~CSearchWindow();

	static CSearchWindow* ManualCreate(void);			// Manually create document

	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);

	static void CreateSearchWindow(CSearchItem* spec = nil);	// Create it or bring it to the front
	static void DestroySearchWindow();							// Destroy the window
	static void AddMbox(const CMbox* mbox, bool reset = true);	// Add a mailbox
	static void AddMboxList(const CMboxList* list);				// Add a list of mailboxes
	static void SearchAgain(CMbox* mbox);						// Search one mbox again
	static void SearchAgain(CMboxList* list);					// Search an mbox list again

	void StartSearch();
	void NextSearch(unsigned long item);
	void EndSearch();
	void SetProgress(unsigned long progress);
	void SetFound(unsigned long found);
	void SetMessages(unsigned long msgs);
	void SetHitState(unsigned long item, bool hit, bool clear = false);
	bool CloseAction(void);

	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command

protected:
	CGrayBackground			mHeader;						// Header pane
	CStatic					mSearchStylesTitle;
	CPopupButton			mSearchStyles;
	CGrayBackground			mScroller;
	CGrayBorder				mCriteria;
	CGrayBackground			mBottomArea;
	CButton					mMoreBtn;
	CButton					mFewerBtn;
	CButton					mClearBtn;
	CButton					mSearchBtn;
	CButton					mCancelBtn;
	CSearchListPanel		mMailboxListPanel;
	CCommanderProtect		mCmdProtect;					// Protect commands

	int						mScrollerImageHeight;

	virtual void	ListenTo_Message(long msg, void* param);	// Respond to list changes

	virtual void	ResetState(bool force = false);		// Reset state from prefs

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy(void);
	virtual void OnDraw(CDC* pDC) {}
	afx_msg void OnSaveDefaultState(void);				// Save state in prefs

	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnStyles(UINT nID);
	afx_msg void OnMoreBtn();
	afx_msg void OnFewerBtn();
	afx_msg void OnClearBtn();
	afx_msg void OnSearch();
	afx_msg void OnCancel();

	void	InitStyles();
	void	SaveStyleAs();
	void	DeleteStyle();
	void	SetStyle(const CSearchItem* spec);

	virtual CWnd* GetContainerWnd()
		{ return &mCriteria; }
	virtual CView* GetParentView()
		{ return this; }
	virtual CButton* GetFewerBtn()
		{ return &mFewerBtn; }

	virtual void Resized(int dy);

	void	SearchInProgress(bool searching);

	DECLARE_MESSAGE_MAP()
};

#endif
