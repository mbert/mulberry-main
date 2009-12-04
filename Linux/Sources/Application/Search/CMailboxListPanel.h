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


// Header for CMailboxListPanel class

#ifndef __CMAILBOXLISTPANEL__MULBERRY__
#define __CMAILBOXLISTPANEL__MULBERRY__

#include <JXWidgetSet.h>

#include "HPopupMenu.h"

#include "templs.h"

// Classes

class CMbox;
class CMboxList;
class CMboxRefList;

class CTwister;
class JXWindowDirector;
class JXStaticText;
class JXTextButton;
class JXProgressIndicator;
class CIconTextTable;

class CMailboxListPanel : public JXWidgetSet
{
	friend class CSearchWindow;
	friend class CDisconnectDialog;

public:
	CMailboxListPanel(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h);
	virtual	~CMailboxListPanel();

	void SetProgress(unsigned long progress);
	void SetFound(unsigned long found);
	void SetMessages(unsigned long msgs);
	void SetHitState(unsigned long item, bool hit, bool clear = false);

	virtual void SetInProgress(bool in_progress);

protected:
	enum EMboxListState
	{
		eNotHit = 0,
		eHit,
		eMiss
	};

	JXWindowDirector*		mParent;
	JXWidgetSet*			mContainer;
	JXWidgetSet*			mStopResize;
// begin JXLayout1

    CTwister*            mTwister;
    JXStaticText*        mShowCaption;
    JXStaticText*        mHideCaption;
    JXWidgetSet*         mMailboxArea;
    HPopupMenu*          mCabinets;
    JXTextButton*        mAddListBtn;
    JXTextButton*        mClearListBtn;
    JXTextButton*        mOptionsBtn;
    JXStaticText*        mTotal;
    JXStaticText*        mFound;
    JXStaticText*        mMessages;
    JXProgressIndicator* mProgress;

// end JXLayout1
	CIconTextTable*	mMailboxList;

	bool			mTwisted;
	bool			mInProgress;

	virtual void	OnCreate(JXWindowDirector* parent, JXWidgetSet* container, JXWidgetSet* stop_resize);

	virtual void	OnTwist();

	void	InitCabinets();
	virtual void	OnCabinet(long item);

	virtual void	ResetMailboxList(const CMboxRefList& targets, const ulvector& target_hits);
	virtual void	OnAddMailboxList();
	virtual void	AddMboxList(const CMboxList* list);				// Add a list of mailboxes
	virtual void	OnClearMailboxList();
	virtual void	OnOpenMailboxList();
};

#endif
