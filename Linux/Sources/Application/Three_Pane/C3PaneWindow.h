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

#include "CWindow.h"

// Classes
class CAdbkManagerView;
class CMbox;
class CServerView;
class CCalendarStoreView;
class CSplitterView;
class CStaticText;
class CToolbarView;
class C3PaneMainPanel;
class C3PaneAccounts;
class C3PaneItems;
class C3PanePreview;
class C3PaneAdbkToolbar;
class C3PaneCalendarToolbar;
class C3PaneMailboxToolbar;

class C3PaneWindow : public CWindow
{
	friend class C3PaneParentPanel;

public:
	static C3PaneWindow* s3PaneWindow;

					C3PaneWindow(JXDirector* owner);
	virtual 		~C3PaneWindow();

	static void Create3PaneWindow();		// Create it or bring it to the front
	static void Destroy3PaneWindow();		// Destroy the window

	virtual bool HandleChar(const int key, const JXKeyModifiers& modifiers);
	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	virtual void	Activate();

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
			void	ShowItems(bool show);
			void	ShowPreview(bool show);

			void	ShowStatus(bool show);
			void	UpdateView();

			void	DoneInitMailAccounts();
			void	DoneInitAdbkAccounts();
			void	DoneInitCalendarAccounts();

			bool	AttemptClose();
			bool	AttemptCloseSelf();

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

			void	SetStatus(const cdstring& str);
			void	SetProgress(const cdstring& str);

protected:
// begin JXLayout

    CStaticText* mStatus;
    CStaticText* mProgress;

// end JXLayout
	CToolbarView*			mToolbarView;
	C3PaneMailboxToolbar*	mMailboxToolbar;
	C3PaneAdbkToolbar*		mAdbkToolbar;
	C3PaneCalendarToolbar*	mCalendarToolbar;
	CSplitterView*			mSplitter1;
	CSplitterView*			mSplitter2;
	C3PaneMainPanel*		mListView;
	C3PaneMainPanel*		mItemsView;
	C3PaneMainPanel*		mPreviewView;
	C3PaneAccounts*			mAccounts;
	C3PaneItems*			mItems;
	C3PanePreview*			mPreview;

	N3Pane::EGeometry		mGeometry;
	N3Pane::EViewType		mViewType;
	bool					mListVisible;
	bool					mItemsVisible;
	bool					mPreviewVisible;
	bool					mStatusVisible;
	bool					mListZoom;
	bool					mItemsZoom;
	bool					mPreviewZoom;
	
	bool					mDoQuitOnClose;

	virtual void	OnCreate();

	void OptionsInit();
	void OptionsSetView();
	void OptionsSaveView();

	CToolbarView*	GetToolbarView() const
		{ return mToolbarView; }
	C3PaneMainPanel* GetListView() const
		{ return mListView; }
	C3PaneMainPanel* GetItemsView() const
		{ return mItemsView; }
	C3PaneMainPanel* GetPreviewView() const
		{ return mPreviewView; }

	// Common updaters
	void OnUpdateZoomList(CCmdUI* pCmdUI);
	void OnUpdateZoomItems(CCmdUI* pCmdUI);
	void OnUpdateZoomPreview(CCmdUI* pCmdUI);

	void OnUpdateShowList(CCmdUI* pCmdUI);
	void OnUpdateShowItems(CCmdUI* pCmdUI);
	void OnUpdateShowPreview(CCmdUI* pCmdUI);
	void OnUpdateMenuShowList(CCmdUI* pCmdUI);
	void OnUpdateMenuShowItems(CCmdUI* pCmdUI);
	void OnUpdateMenuShowPreview(CCmdUI* pCmdUI);

	void OnUpdateWindowLayout(CCmdUI* pCmdUI);

	void OnWindowOptions();

	void	ResetPaneState();						// Reset state from prefs
	void	SaveDefaultPaneState();					// Save state in prefs

private:
			void	Init3PaneWindow();
			void	InstallToolbars();
			void	InstallViews();

public:
	virtual void	ResetState(bool force = false);			// Reset state from prefs
	virtual void	SaveDefaultState();						// Save state in prefs
};

#endif
