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


// Header for CMailboxInfoTable class

#ifndef __CMAILBOXINFOTABLE__MULBERRY__
#define __CMAILBOXINFOTABLE__MULBERRY__

#include "CMailboxTable.h"

#include "CMatchItem.h"

// Panes

// Messages

// Resources
const	ResIDT		RidL_CMailboxBtns = 1000;

// Classes
class CMailboxInfoView;
class CMbox;
class CMessage;
class CDragIt;

class CMailboxInfoTable : public CMailboxTable
{
	friend class CMailboxInfoWindow;
	friend class CMailboxInfoView;

public:
	enum { class_ID = 'MbLi' };

	enum
	{
		eSelectBy_Seen = 1,
		eSelectBy_Unseen,
		eSelectBy_Important,
		eSelectBy_Unimportant,
		eSelectBy_Answered,
		eSelectBy_Unanswered,
		eSelectBy_Deleted,
		eSelectBy_Undeleted,
		eSelectBy_Draft,
		eSelectBy_NotDraft,
		eSelectBy_Separator1,
		eSelectBy_Match,
		eSelectBy_NonMatch,
		eSelectBy_Separator2,
		eSelectBy_Label1,
		eSelectBy_Label2,
		eSelectBy_Label3,
		eSelectBy_Label4,
		eSelectBy_Label5,
		eSelectBy_Label6,
		eSelectBy_Label7,
		eSelectBy_Label8
	};

					CMailboxInfoTable();
					CMailboxInfoTable(LStream *inStream);
	virtual 		~CMailboxInfoTable();

private:
			void	InitInfoTable(void);					// Do common init

protected:
	CMailboxInfoView*	mInfoTableView;						// Owning view
	CMatchItem			mMatch;								// Matching for this pane
	bool				mSearchResults;						// Displaying search results
	unsigned long		mLastTyping;						// Time of last typed character
	char				mLastChar;							// Last characters typed

	virtual void	FinishCreateSelf(void);					// Get details of sub-panes
public:
	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

	virtual	void	GetTooltipText(cdstring& txt, const STableCell &inCell);				// Get text for current tooltip cell

protected:
	virtual void	DoSingleClick(unsigned long row, unsigned long col, const CKeyModifiers& mods);					// Single click on a message

	void	DoNewLetter(bool option_key);				// Display a new draft

	void	SelectClearMessages(void);					// Select messages marked to be cleared (seen but not deleted)
	void	SelectByRow(const ulvector& rows);			// Select all messages in specified rows
	void	SelectNextNew(bool shift = true);			// Select next new message
	void	DoRollover(bool ask = true);				// Rollover to next recent
	void	CycleTabs(bool forward);					// Cycle to next/previous tab

	void	OnFileClose();								// Close it
	bool	CanFileClose() const;						// Should File:Close be active

	void	OnMailboxProperties(void);					// Properties of mailbox

	void	OnMailboxPunt(void);						// Punt unseen in mailbox

	void	OnMailboxSynchronise();						// Synchronise
	void	OnMailboxClearDisconnect();					// Clear disconnected cache

	void	OnMailboxSubscribe(void);					// Subscribe to mailbox
	void	OnMailboxUnsubscribe(void);					// Unsubscribe from mailbox

	void	OnMailboxCheck(void);						// Check for new mail

	void	OnMailboxGotoMessage();						// Select a specific message
	void	OnMailboxCacheAll();						// Cache all messages in mailbox
	void	OnMailboxUpdateMatch();						// Update current match state

	void	OnSearchMailbox();							// Do mailbox search
	void	OnSearchAgainMailbox();						// Do mailbox search again
	void	OnNextSearchMailbox();						// Recycle to next search mbox

	void	DoApplyRuleMailbox(long index);					// Apply rules
	void	OnApplyRuleMailboxToolbar(const char* name);	// Apply rules toolbar item
	void	OnMakeRuleMailbox();							// Make rule

	void	FindFlagCommandStatus(Boolean &outEnabled,
									Boolean &outUsesMark,
									UInt16 &outMark,
									NMessage::EFlags flag);

	void	DoMessageReply(EReplyTo reply_to,
									bool option_key);			// Reply to selected messages
	bool	ReplyMessage(TableIndexT row,
									EReplyTo* reply_to,
									bool* option_key);			// Reply to specified message

	void	DoMessageForward(bool option_key);			// Forward selected messages
	bool	ForwardMessage(TableIndexT row,				// Forward specified message
							bool* option_key);

	void	OnMessageBounce(void);						// Bounce selected messages
	bool	BounceMessage(TableIndexT row);				// Bounce specified message

	void	OnMessageReject(void);						// Reject selected messages
	bool	RejectMessage(TableIndexT row);				// Reject specified message

	void	OnMessageSendAgain(void);					// Send again selected messages
	bool	SendAgainMessage(TableIndexT row);			// Send again specified message

	void	OnMessageCreateDigest(void);				// Digest selected messages

	void	ResetSearch();								// Reset search UI items from mailbox
	void	OnMatchButton(bool option_key);				// Change match mode
	void	SetMatch(NMbox::EViewMode mbox_mode,		// Set match mode and criteria
						CSearchItem* spec,
						bool is_search,
						bool force_automatch = false);

	void	SetCurrentMatch(const CMatchItem& match)
		{ mMatch = match; }
	CMatchItem	GetCurrentMatch() const
		{ return mMatch; }

	void	OnSelectPopup(long select);						// Change view selection

	void	OnThreadSelectAll();
	void	OnThreadMarkSeen();
	void	OnThreadMarkImportant();
	void	OnThreadMarkDeleted();
	void	DoThreadFlag(NMessage::EFlags flag);
	void	OnThreadSkip();

	void	OnMatchUnseen();
	void	OnMatchImportant();
	void	OnMatchDeleted();
	void	OnMatchSentToday();
	void	OnMatchFrom();
	void	OnMatchSubject();
	void	OnMatchDate();
	void	DoMatch(const CMatchItem& match);

	virtual void		ResizeFrameBy(
								SInt16		inWidthDelta,
								SInt16		inHeightDelta,
								Boolean		inRefresh);

public:
	virtual	void	InitPos(void);								// Set initial position
	
	virtual void	Refresh();								// Update button & captions as well
	virtual void	RefreshSelection();						// Update button & captions as well
	virtual void	RefreshRow(const TableIndexT aRow);		// Update button & captions as well

	virtual void	DoSelectionChanged(void);

protected:
	virtual void	UpdateItems(void);							// Update after possible change

	void	UpdateState(void);						// Update buttons
	void	UpdateCaptions(void);					// Update number captions
	void	UpdateCacheIncrement();					// Update cache increment

// Drag methods
	virtual Boolean	ItemIsAcceptable(DragReference inDragRef,
								ItemReference inItemRef);				// Check its suitable
};

#endif
