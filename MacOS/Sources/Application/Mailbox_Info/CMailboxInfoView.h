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

#include "CMailboxInfoTable.h"

// Constants
const	PaneIDT		paneid_MailboxQuickSearchArea = 'VSRC';
const	PaneIDT		paneid_MailboxQuickSearchIndicator = 'SIND';
const	PaneIDT		paneid_MailboxQuickSearchCriteria = 'SPOP';
const	PaneIDT		paneid_MailboxQuickSearchText = 'STXT';
const	PaneIDT		paneid_MailboxQuickSearchSaved = 'APOP';
const	PaneIDT		paneid_MailboxQuickSearchProgress = 'SPRO';
const	PaneIDT		paneid_MailboxQuickSearchCancel = 'SBTN';
const	PaneIDT		paneid_MailboxTableArea = 'VTAB';
const	PaneIDT		paneid_MailboxTotalNum = 'TNUM';
const	PaneIDT		paneid_MailboxNewNum = 'NNUM';
const	PaneIDT		paneid_MailboxDeletedNum = 'DNUM';
const	PaneIDT		paneid_MailboxMatchedNum = 'MNUM';

// Messages
const	MessageT	msg_MailboxQuickSearchCriteria = 'SPOP';
const	MessageT	msg_MailboxQuickSearchText = 'STXT';
const	MessageT	msg_MailboxQuickSearchSaved = 'APOP';
const	MessageT	msg_MailboxQuickSearchCancel = 'SBTN';

// Classes
class CMboxProtocol;
class CSearchItem;
class CStaticText;
class CTextFieldX;
class LBevelButton;
class LPopupButton;

class CMailboxInfoView : public CMailboxView,
							public LListener,
							public LPeriodical
{
	friend class CMailboxInfoTable;

protected:
	enum
	{
		eQuickSearchFrom = 1,
		eQuickSearchTo,
		eQuickSearchRecipient,
		eQuickSearchCorrespondent,
		eQuickSearchSubject,
		eQuickSearchBody,
		eQuickSearchDivider,
		eQuickSearchSaved
	};

	LView*				mQuickSearchArea;
	LPane*				mQuickSearchIndicator;
	LPopupButton*		mQuickSearchCriteria;
	CTextFieldX*		mQuickSearchText;
	LPopupButton*		mQuickSearchSaved;
	LView*				mQuickSearchProgress;
	LBevelButton*		mQuickSearchCancel;
	LView*				mTableArea;
	CStaticText*		mTotal;						// Total number caption
	CStaticText*		mUnseen;					// Unseen number caption
	CStaticText*		mDeleted;					// Deleted number caption
	CStaticText*		mMatched;					// Matched number caption
	ulset				mAutoMatchMsgs;				// Messages Msgs selected at start of auto match
	bool				mNoSearchReset;
	bool				mShowQuickSearch;
	unsigned long		mQuickSearchLastTime;		// Time of last typed character

public:
	enum { class_ID = 'MbVi' };

					CMailboxInfoView();
					CMailboxInfoView(LStream *inStream);
	virtual 		~CMailboxInfoView();

	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);
	virtual void	ListenToMessage(MessageT inMessage,
											void *ioParam);	// Respond to clicks in the icon buttons

protected:
	virtual void	FinishCreateSelf();

public:
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
			void	DoQuickSearchPopup(long value);			// Do quick search popup
			void	DoQuickSearch(bool change_focus);		// Do quick search
			void	DoQuickSearchSavedPopup(long value);	// Do quick search saved popup
			void	DoQuickSearchCancel();					// Cancel quick search
			void	SyncQuickSearch();						// Sync quick search with current mailbox Match state

	virtual	void	SpendTime(const EventRecord &inMacEvent);		// Called during idle
			void	StartQuickSearchTimer();						// Start timer to trigger quick search

	virtual void	ResetState(bool force = false);			// Reset window state
	virtual void	SaveState();							// Save current state in prefs
	virtual void	SaveDefaultState();						// Save current state as default
};

#endif
