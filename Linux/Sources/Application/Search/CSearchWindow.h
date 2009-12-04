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


// Header for CSearchWindow class

#ifndef __CSEARCHWINDOW__MULBERRY__
#define __CSEARCHWINDOW__MULBERRY__

#include "CWindow.h"
#include "CSearchBase.h"

#include "HPopupMenu.h"

#include <JXEngravedRect.h>
#include <JXScrollableWidget.h>

#include "CMboxRefList.h"

enum
{
	str_SaveStyleAs = 1,
	str_DeleteStyleTitle,
	str_DeleteStyleDesc,
	str_DeleteStyleButton
};

class CSearchItem;
class CMboxList;
class CSearchCriteria;
class CSearchListPanel;

class JXEngravedRect;
class CBetterScrollbarSet;
class JXScrollableWidget;
class JXTextButton;
class JXWidgetSet;

class CBlankScrollable : public JXScrollableWidget
{
public:
	CBlankScrollable(JXScrollbarSet* scrollbarSet,
					   JXContainer* enclosure,
					   const HSizingOption hSizing, const VSizingOption vSizing,
					   const JCoordinate x, const JCoordinate y,
					   const JCoordinate w, const JCoordinate h) :
	JXScrollableWidget(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
		{ WantInput(kFalse); }

			void Init();
	virtual void Draw(JXWindowPainter& p, const JRect& rect);
			void AdjustBounds(const JCoordinate dw, const JCoordinate dh);
	virtual void ApertureResized(const JCoordinate dw, const JCoordinate dh);
};

class	CSearchWindow : public CWindow, 
						public CSearchBase
{
	friend class CSearchEngine;
	friend class CSearchListPanel;

public:

	static CSearchWindow*	sSearchWindow;

	CSearchWindow(JXDirector* supervisor);
	
	virtual ~CSearchWindow();

	static void CreateSearchWindow(CSearchItem* spec = NULL);	// Create it or bring it to the front
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

protected:
// begin JXLayout

    HPopupMenu*          mSearchStyles;
    JXWidgetSet*         mBottomArea;
    JXTextButton*        mMoreBtn;
    JXTextButton*        mFewerBtn;
    JXTextButton*        mClearBtn;
    JXTextButton*        mSearchBtn;
    JXTextButton*        mCancelBtn;
    CSearchListPanel*    mMailboxListPanel;
    CBetterScrollbarSet* mScroller;
    CBlankScrollable*    mScrollPane;
    JXEngravedRect*      mCriteria;

// end JXLayout
	bool				 mPendingResize;

	virtual JXWidget* GetContainerWnd()
		{ return mCriteria; }
	virtual CWindow* GetParentView()
		{ return this; }
	virtual JXTextButton* GetFewerBtn()
		{ return mFewerBtn; }

			void BoundsResized(const JCoordinate dw, const JCoordinate dh);
	virtual void Resized(int dy);

	virtual void OnCreate(void);

	virtual void Activate();
	virtual void WindowFocussed(JBoolean focussed);
	virtual void Receive(JBroadcaster* sender, const Message& message);
	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);
	virtual void ListenTo_Message(long msg, void* param);	// Respond to list changes

	void	OnStyles(long item);
	void	OnSearch();
	void	OnCancel();
	void	OnTwist();
	void	OnOptions();

	void	InitStyles();
	void	SaveStyleAs();
	void	DeleteStyle();
	void	SetStyle(const CSearchItem* spec);

	void	SearchInProgress(bool searching);

private:
	void	ResetState(bool force = false);				// Reset state from prefs
	void	SaveDefaultState(void);					// Save state in prefs
};

#endif
