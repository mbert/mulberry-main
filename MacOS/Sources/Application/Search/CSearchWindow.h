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

#include "LWindow.h"
#include "CSearchBase.h"

#include "CSearchListPanel.h"
#include <LPushButton.h>

#include "CMboxRefList.h"

// Constants
const	PaneIDT		paneid_SearchWindow = 1010;
const	PaneIDT		paneid_SearchSearchSets = 'FSET';
const	PaneIDT		paneid_SearchScroller = 'SCRL';
const	PaneIDT		paneid_SearchCriteriaArea = 'CRIT';
const	PaneIDT		paneid_SearchCriteriaMove = 'SMOV';
const	PaneIDT		paneid_SearchMore = 'MCHS';
const	PaneIDT		paneid_SearchFewer = 'FCHS';
const	PaneIDT		paneid_SearchClear = 'CLRS';
const	PaneIDT		paneid_SearchSearch = 'SRCH';
const	PaneIDT		paneid_SearchCancel = 'CANC';
const	PaneIDT		paneid_SearchBottomArea = 'BTMV';

// Messages
const	MessageT	msg_SearchSearchSets = 'FSET';
const	MessageT	msg_SearchMore = 'MCHS';
const	MessageT	msg_SearchFewer = 'FCHS';
const	MessageT	msg_SearchClear = 'CLRS';
const	MessageT	msg_SearchSearch = 'SRCH';
const	MessageT	msg_SearchCancel = 'CANC';

// Resources
const	ResIDT		RidL_CSearchWindowBtns = 1010;

// Classes

class CBarPane;
class CMailboxWindow;
class CMbox;
class CMboxList;
class CSearchItem;
class LPopupButton;

class	CSearchWindow : public LWindow,
						 	public LListener,
							public CSearchBase
{
	friend class CSearchEngine;
	friend class CSearchListPanel;

public:
	enum { class_ID = 'Srch' };

	static CSearchWindow*	sSearchWindow;

					CSearchWindow();
					CSearchWindow(LStream *inStream);
	virtual 		~CSearchWindow();

	static void CreateSearchWindow(CSearchItem* spec = nil);	// Create it or bring it to the front
	static void DestroySearchWindow();							// Destroy the window
	static void AddMbox(const CMbox* mbox, bool reset = true);	// Add a mailbox
	static void AddMboxList(const CMboxList* list);				// Add a list of mailboxes
	static void SearchAgain(CMbox* mbox);						// Search one mbox again
	static void SearchAgain(CMboxList* list);					// Search an mbox list again

	virtual Boolean	AttemptQuitSelf(SInt32 inSaveOption);
	virtual void	AttemptClose(void);

	void StartSearch();
	void NextSearch(unsigned long item);
	void EndSearch();
	void SetProgress(unsigned long progress);
	void SetFound(unsigned long found);
	void SetMessages(unsigned long msgs);
	void SetHitState(unsigned long item, bool hit, bool clear = false);

protected:
	LPopupButton*	mSearchStyles;
	LView*			mScroller;
	LView*			mCriteria;
	LView*			mCriteriaMove;
	LPushButton*	mMoreBtn;
	LPushButton*	mFewerBtn;
	LPushButton*	mClearBtn;
	LPushButton*	mSearchBtn;
	LPushButton*	mCancelBtn;
	LView*			mBottomArea;
	CSearchListPanel	mMailboxListPanel;
	LPushButton*	mOptionsBtn;

	virtual void	FinishCreateSelf(void);
	virtual void	ActivateSelf(void);						// Activate text item

	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	virtual void		ListenToMessage(MessageT inMessage,
										void *ioParam);				// Respond to clicks in the icon buttons
	virtual void		ListenTo_Message(long msg, void* param);	// Respond to list changes

	void	OnStyles(long item);
	void	OnSearch();
	void	OnCancel();
	void	OnTwist();

	void	InitStyles();
	void	SaveStyleAs();
	void	DeleteStyle();
	void	SetStyle(const CSearchItem* spec);

	virtual LWindow* GetContainerWnd()
		{ return this; }
	virtual LView* GetParentView()
		{ return mCriteria; }
	virtual LPane* GetFewerBtn()
		{ return mFewerBtn; }

	virtual void Resized(int dy);

	void	InitCabinets();
	void	OnCabinet(long item);

	void	ResetMailboxList();
	void	OnAddMailboxList();
	void	OnClearMailboxList();
	void	OnOpenMailboxList();

	void	SearchInProgress(bool searching);
	void	WaitForCancel();

private:
	void	ResetState(bool force = false);				// Reset state from prefs
	void	SaveDefaultState(void);					// Save state in prefs
};

#endif
