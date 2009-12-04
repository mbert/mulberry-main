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


// Header for CMailboxInfoView class

#ifndef __CMAILBOXINFOVIEW__MULBERRY__
#define __CMAILBOXINFOVIEW__MULBERRY__

#include "CMailboxView.h"
#include <JXIdleTask.h>

#include "CMailboxInfoTable.h"

#include "HPopupMenu.h"

// Classes
class CMboxProtocol;
class CQuickSearchText;
class CQuickSearchSavedPopup;
class CProgressBar;
class CSearchItem;
class CTableScrollbarSet;
class JXFlatRect;
class JXImageWidget;
class JXMultiImageButton;
class JXMultiImageCheckbox;
class JXStaticText;

class CMailboxInfoView : public CMailboxView,
						public JXIdleTask
{
	friend class CMailboxInfoTable;

public:
	CMailboxInfoView(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h);
	virtual	~CMailboxInfoView();

	virtual void	OnCreate();

public:
	virtual void	ListenTo_Message(long msg, void* param);

	virtual void	MakeToolbars(CToolbarView* parent);

	virtual void	DoProtocolLogoff(const CMboxProtocol* proto);
	virtual void	DoRemoveWD(const CMboxList* wd);

	virtual bool	TestClose();
			bool	TestCloseOne(unsigned long index);
			bool	TestCloseAll(bool not_locked = false);
			bool	TestCloseOthers(unsigned long index);
			bool	TestClose(const CMboxProtocol* proto);
			bool	TestClose(CMbox* mbox);

			void	DoOpenMbox(CMbox* mbox,
								bool is_search = false);

	virtual void	DoClose();
			void	DoClose(CMbox* mbox);
			void	DoClose(const CMboxProtocol* proto);
			void	DoCloseOne(unsigned long index);
			void	DoCloseAll(bool not_locked = false);
			void	DoCloseOthers(unsigned long index);
			void	DoCloseMbox();
			void	DoCloseMbox(CMbox* mbox);

	virtual void	ViewMbox(CMbox* mbox,					// Open and set it
						bool use_dynamic = true,
						bool is_search = false);

	virtual void	SetMbox(CMbox* anMbox,					// Set the mbox and init
						bool is_search = false);
			void	ResetMbox(CMbox* anMbox);				// Reset the mbox
			void	InitMbox(CMbox* anMbox);				// Init the mbox
	virtual bool	Recycle(CMbox* anMbox,					// Recycle to new mbox
						bool is_search = false);
			bool	RecycleSubstitute(CMbox* anMbox,		// Recycle to new mbox
						bool is_search = false);
			bool	RecycleClose(CMbox* anMbox,				// Recycle to new mbox
						bool is_search = false);
	virtual bool	Substitute(CMbox* anMbox,				// Replace current displayed mailbox with another
						bool select = true,
						bool is_search = false);

	virtual void	SetUseSubstitute(bool use_substitute);

	virtual CMailboxInfoTable* GetInfoTable()
		{ return static_cast<CMailboxInfoTable*>(mTable); }

	virtual CMailboxToolbarPopup* GetCopyBtn() const;

			void	SearchMailbox(CSearchItem* spec);		// Do search of mailbox
			void	ResetSearch();							// Mailbox search has changed

	virtual void	RenamedMailbox(const CMbox* mbox);		// Mailbox was renamed

	virtual void	SelectNextNew(bool backwards = false);	// Select next new message

			void	EnableQuickSearch(bool enable);			// Enable quick search items
			void	ShowQuickSearch(bool show);				// Show quick search area
			void	DoQuickSearchPopup(JIndex value);		// Do quick search popup
			void	DoQuickSearch(bool change_focus);		// Do quick search
			void	DoQuickSearchSavedPopup(long value);	// Do quick search saved popup
			void	DoQuickSearchCancel();					// Cancel quick search
			void	SyncQuickSearch();						// Sync quick search with current mailbox Match state
			void	StartQuickSearchTimer();
	virtual	void	Perform(const Time delta, Time* maxSleepTime);


	virtual void	ResetState(bool force = false);			// Reset window state
	virtual void	SaveState();							// Save current state in prefs
	virtual void	SaveDefaultState();						// Save current state as default

protected:
	enum
	{
		eQuickSearchFrom = 1,
		eQuickSearchTo,
		eQuickSearchRecipient,
		eQuickSearchCorrespondent,
		eQuickSearchSubject,
		eQuickSearchBody,
		//eQuickSearchDivider,
		eQuickSearchSaved
	};

// begin JXLayout1

    JXFlatRect*             mQuickSearchArea;
    JXImageWidget*          mQuickSearchIndicator;
    HPopupMenu*             mQuickSearchCriteria;
    CQuickSearchText*       mQuickSearchText;
    CQuickSearchSavedPopup* mQuickSearchSaved;
    CProgressBar*           mQuickSearchProgress;
    JXMultiImageButton*     mQuickSearchCancel;
    CTableScrollbarSet*     mScroller;
    JXMultiImageCheckbox*   mSortDirectionBtn;
    JXStaticText*           mTotal;
    JXStaticText*           mUnseen;
    JXStaticText*           mDeleted;
    JXStaticText*           mMatched;

// end JXLayout1
	ulset				mAutoMatchMsgs;				// Messages Msgs selected at start of auto match
	bool				mNoSearchReset;
	bool				mShowQuickSearch;
	unsigned long		mQuickSearchTriggerTime;					// Time at which to trigger seen flag change
	bool				mQuickSearchTimerPending;

	virtual void	Receive(JBroadcaster* sender, const Message& message);
	virtual bool	ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void	UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	// Common updaters
			void OnUpdateWindowsShowQuickSearch(CCmdUI* pCmdUI);

	// Command handlers
			void OnWindowsShowQuickSearch();

};

#endif
