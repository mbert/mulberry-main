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


// Header for CCalendarWindow class

#ifndef __CCalendarWindow__MULBERRY__
#define __CCalendarWindow__MULBERRY__

#include "CWindowStatus.h"
#include "CListener.h"
#include "CCommander.h"

#include "CCalendarView.h"
#include "CEventPreview.h"
#include "CSplitterView.h"
#include "CToolbarView.h"

#include "cdmutexprotect.h"

// Classes
namespace calstore
{
	class CCalendarStoreNode;
};

class CCalendarWindow : public CView,
						public CWindowStatus,
						public CListener,
						public CCommander
{
	DECLARE_DYNCREATE(CCalendarWindow)

public:
	typedef vector<CCalendarWindow*>	CCalendarWindowList;
	static cdmutexprotect<CCalendarWindowList> sCalendarWindows;	// List of windows (protected for multi-thread access)
	static CMultiDocTemplate*		sCalendarDocTemplate;

					CCalendarWindow();
	virtual 		~CCalendarWindow();

	static void MakeWindow(calstore::CCalendarStoreNode* node);
	static void CreateSubscribedWindow();
	static CCalendarWindow* ManualCreate();

	static	CCalendarWindow* FindWindow(const calstore::CCalendarStoreNode* node);
	static bool	WindowExists(const CCalendarWindow* wnd);		// Check for window

	virtual void		ListenTo_Message(long msg, void* param);	// Respond to list changes

	virtual void	OnInitialUpdate(void);				// Focus
	virtual void	OnDraw(CDC* pDC) {}
	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command

	CCalendarView* GetCalendarView()
	{
		return &mCalendarView;
	}

	virtual CToolbarView* GetToolbarView()
		{ return &mToolbarView; }

	virtual CSplitterView* GetSplitter()
		{ return &mSplitterView; }

	virtual void	ResetState(bool force = false);				// Reset window state
	virtual void	SaveState();								// Save current state in prefs
	virtual void	SaveDefaultState();							// Save current state as default

protected:
	CToolbarView		mToolbarView;
	CSplitterView		mSplitterView;
	CCalendarView		mCalendarView;
	CEventPreview		mPreview;
	bool				mPreviewVisible;
	calstore::CCalendarStoreNode*	mNode;

			void 	SetNode(calstore::CCalendarStoreNode* node);
			void 	DeleteNode(calstore::CCalendarStoreNode* node);
			void 	RefreshNode(calstore::CCalendarStoreNode* node);

			void	ShowPreview(bool preview);

	// Command updaters
	afx_msg void	OnUpdateMenuShowPreview(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateShowPreview(CCmdUI* pCmdUI);

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnShowPreview(void);
	afx_msg void	OnResetDefaultState();
	afx_msg void	OnSaveDefaultState();				// Save state in prefs

	DECLARE_MESSAGE_MAP()
};

#endif
