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


// Header for C3PaneWindow class

#ifndef __C3PANEWINDOW__MULBERRY__
#define __C3PANEWINDOW__MULBERRY__

#include "C3PaneWindowFwd.h"

#include "CCommander.h"
#include "CWindowStatus.h"

#include "CHelpFrame.h"

#include "CSplitterView.h"
#include "CToolbarView.h"
#include "C3PaneAccounts.h"
#include "C3PaneItems.h"
#include "C3PaneMainPanel.h"
#include "C3PanePreview.h"

// Messages

// Resources

// Classes
class CMbox;
class CServerView;
class CCalendarStoreView;
class CSplitterView;
class CToolbarView;
class C3PaneMainPanel;
class C3PaneAccounts;
class C3PaneItems;
class C3PanePreview;
class C3PaneAdbkToolbar;
class C3PaneCalendarToolbar;
class C3PaneMailboxToolbar;

class C3PaneWindow : public CView,
						public CWindowStatus,
						public CCommander
{
	friend class C3PaneParentPanel;

	DECLARE_DYNCREATE(C3PaneWindow)

public:
	static C3PaneWindow* 		s3PaneWindow;
	static CMultiDocTemplate*	s3PaneDocTemplate;

					C3PaneWindow();
	virtual 		~C3PaneWindow();

	static void Create3PaneWindow();		// Create it or bring it to the front
	static void Destroy3PaneWindow();		// Destroy the window

	static C3PaneWindow* ManualCreate();					// Manually create document

	virtual void	OnDraw(CDC* pDC) {}

	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command
	virtual bool	HandleChar(UINT nChar, UINT nRepCnt, UINT nFlags);

			void	SetGeometry(N3Pane::EGeometry geometry);
			void	SetViewType(N3Pane::EViewType geometry);

			void	SetUseSubstitute(bool subs);

			bool	GetZoomList() const
				{ return mListZoom; }
			void	ZoomList(bool zoom);
			bool	GetZoomItems() const
				{ return mItemsZoom; }
			void	ZoomItems(bool zoom);
			bool	GetZoomPreview() const
				{ return mPreviewZoom; }
			void	ZoomPreview(bool zoom);

			void	ShowList(bool show);
			bool	IsListVisible() const
				{ return mListVisible; }
			void	ShowItems(bool show);
			bool	IsItemsVisible() const
				{ return mItemsVisible; }
			void	ShowPreview(bool show);
			bool	IsPreviewVisible() const
				{ return mPreviewVisible; }

			void	ShowStatus(bool show);
			void	UpdateView();

			void	DoneInitMailAccounts();
			void	DoneInitAdbkAccounts();
			void	DoneInitCalendarAccounts();

			bool	AttemptClose();
			void	DoClose();

	C3PaneMailboxToolbar*	GetMailboxToolbar() const
		{ return mMailboxToolbar; }
	C3PaneAdbkToolbar*		GetAdbkToolbar() const
		{ return mAdbkToolbar; }
	C3PaneCalendarToolbar*	GetCalendarToolbar() const
		{ return mCalendarToolbar; }

	CServerView*		GetServerView() const;
	CAdbkManagerView*	GetContactsView() const;
	CCalendarStoreView*	GetCalendarStoreView() const;

			void	GetOpenItems(cdstrvect& items) const;
			void	CloseOpenItems();
			void	SetOpenItems(const cdstrvect& items);

protected:
	CToolbarView			mToolbarView;
	C3PaneMailboxToolbar*	mMailboxToolbar;
	C3PaneAdbkToolbar*		mAdbkToolbar;
	C3PaneCalendarToolbar*	mCalendarToolbar;
	CSplitterView			mSplitter1;
	CSplitterView			mSplitter2;
	C3PaneMainPanel			mListView;
	C3PaneMainPanel			mItemsView;
	C3PaneMainPanel			mPreviewView;
	C3PaneAccounts			mAccounts;
	C3PaneItems				mItems;
	C3PanePreview			mPreview;

	N3Pane::EGeometry		mGeometry;
	N3Pane::EViewType		mViewType;
	bool					mListVisible;
	bool					mItemsVisible;
	bool					mPreviewVisible;
	bool					mStatusVisible;
	bool					mListZoom;
	bool					mItemsZoom;
	bool					mPreviewZoom;

	void OptionsInit();
	void OptionsSetView();
	void OptionsSaveView();

	CToolbarView*	GetToolbarView()
		{ return &mToolbarView; }
	C3PaneMainPanel* GetListView()
		{ return &mListView; }
	C3PaneMainPanel* GetItemsView()
		{ return &mItemsView; }
	C3PaneMainPanel* GetPreviewView()
		{ return &mPreviewView; }

private:
			void	Init3PaneWindow();
			void	InstallToolbars();
			void	InstallViews();

public:
	virtual void	ResetState(bool force = false);			// Reset state from prefs
	virtual void	SaveDefaultState();						// Save state in prefs

protected:
	// Common updaters
	afx_msg void OnUpdateNever(CCmdUI* pCmdUI);

	afx_msg void OnUpdateZoomList(CCmdUI* pCmdUI);
	afx_msg void OnUpdateZoomItems(CCmdUI* pCmdUI);
	afx_msg void OnUpdateZoomPreview(CCmdUI* pCmdUI);

	afx_msg void OnUpdateShowList(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShowItems(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShowPreview(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMenuShowList(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMenuShowItems(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMenuShowPreview(CCmdUI* pCmdUI);

	afx_msg void OnUpdateWindowLayout(CCmdUI* pCmdUI);

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy(void);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	afx_msg void OnZoomList(void);
	afx_msg void OnZoomItems(void);
	afx_msg void OnZoomPreview(void);

	afx_msg void OnShowList(void);
	afx_msg void OnShowItems(void);
	afx_msg void OnShowPreview(void);

	afx_msg void OnResetPaneState(void);
	afx_msg void OnSaveDefaultPaneState(void);

	afx_msg void OnWindowLayout(UINT nID);

	DECLARE_MESSAGE_MAP()
};

#endif
