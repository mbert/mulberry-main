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

#include "CMboxFwd.h"
#include "CMatchItem.h"

#include <JXDisplay.h>

// Classes
class CMailboxInfoView;

class CMailboxInfoTable : public CMailboxTable
{
	friend class CMailboxInfoWindow;
	friend class CMailboxInfoView;

public:
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
		//eSelectBy_Separator1,
		eSelectBy_Match,
		eSelectBy_NonMatch,
		//eSelectBy_Separator2,
		eSelectBy_Label1,
		eSelectBy_Label2,
		eSelectBy_Label3,
		eSelectBy_Label4,
		eSelectBy_Label5,
		eSelectBy_Label6,
		eSelectBy_Label7,
		eSelectBy_Label8
	};

	CMailboxInfoTable(JXScrollbarSet* scrollbarSet, 
						JXContainer* enclosure,
						const HSizingOption hSizing, 
						const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h);
	virtual 		~CMailboxInfoTable();

private:
			void	InitInfoTable();					// Do common init

protected:	
	CMailboxInfoView*	mInfoTableView;							// Owning view
	CMatchItem			mMatch;									// Matching for this pane
	bool				mSearchResults;							// Displaying search results
	unsigned long		mLastTyping;							// Time of last typed character
	char				mLastChar;								// Last characters typed

protected:
	virtual void	OnCreate();					// Get details of sub-panes
public:
	virtual bool	HandleChar(const int key, const JXKeyModifiers& modifiers);

	virtual	void	GetTooltipText(cdstring& txt, const STableCell& cell);				// Get text for current tooltip cell

protected:
	virtual void	DoSingleClick(unsigned long row, unsigned long col, const CKeyModifiers& mods);					// Single click on a message

	void	OnUpdateFileClose(CCmdUI* cmdui);
	void	OnUpdateMailboxSubscribe(CCmdUI* cmdui);
	void	OnUpdateMailboxUnsubscribe(CCmdUI* cmdui);
	void	OnUpdateMailboxSynchronise(CCmdUI* cmdui);
	void	OnUpdateMailboxClearDisocnnect(CCmdUI* cmdui);
	void	OnUpdateMailboxPartialCache(CCmdUI* cmdui);
	void	OnUpdateMailboxUpdateMatch(CCmdUI* pCmdUI);
	void	OnUpdateMailboxApplyRules(CCmdUI* cmdui);
	void	OnUpdateMailboxApplyRulesBtn(CCmdUI* cmdui);
	void	OnUpdateMailboxMakeRule(CCmdUI* cmdui);
	void	OnUpdateMessageReject(CCmdUI* cmdui);
	void	OnUpdateMessageSendAgain(CCmdUI* cmdui);
	void	OnUpdateFlags(CCmdUI* cmdui);
	void	OnUpdateFlagsSeen(CCmdUI* cmdui);
	void	OnUpdateFlagsImportant(CCmdUI* cmdui);
	void	OnUpdateFlagsAnswered(CCmdUI* cmdui);
	void	OnUpdateFlagsDeleted(CCmdUI* cmdui);
	void	OnUpdateFlagsDraft(CCmdUI* cmdui);
	void	OnUpdateFlagsLabel(CCmdUI* cmdui);
	void	OnUpdateFlags(CCmdUI* cmdui, NMessage::EFlags flag);
	void	OnUpdateMatchBtn(CCmdUI* cmdui);

	void	OnFileNewDraft(void);
	void	DoNewLetter(bool option_key);				// Display a new draft

	void	SelectClearMessages();						// Select messages marked to be cleared (seen but not deleted)
	void	SelectByRow(const ulvector& rows);			// Select all messages in specified rows
	void	SelectNextNew(bool shift);					// Select next new message
	void	DoRollover(bool ask = true);				// Rollover to next recent
	void	CycleTabs(bool forward);					// Cycle to next/previous tab

	void	OnFileClose();								// Close it
	bool	CanFileClose() const;						// Should File:Close be active

	void	OnMailboxProperties();

	void	OnMailboxPunt();						// Punt unseen in mailbox

	void	OnMailboxSynchronise();						// Synchronise
	void	OnMailboxClearDisconnect();					// Clear disconnected cache

	void	OnMailboxSubscribe();					// Subscribe to mailbox
	void	OnMailboxUnsubscribe();					// Unsubscribe from mailbox

	void	OnMailboxCheck();						// Check for new mail

	void	OnMailboxGotoMessage();						// Select a specific message
	void	OnMailboxCacheAll();						// Cache all messages in mailbox

	void	OnMailboxUpdateMatch();						// Update match state

	void	OnSearchMailbox();							// Do mailbox search
	void	OnSearchAgainMailbox();						// Do mailbox search again
	void	OnNextSearchMailbox();						// Recycle to next search mbox

	void	OnApplyRuleMailbox(JIndex index);			// Apply rules
	void	DoApplyRuleMailbox(long index);				// Apply rules
	void	OnApplyRuleMailboxToolbar(const char* name);	// Apply rules toolbar item
	void	OnMakeRuleMailbox();						// Make rule

	void	OnMessageReply();
	void	OnMessageReplySender();
	void	OnMessageReplyFrom();
	void	OnMessageReplyAll();
	void	DoMessageReply(EReplyTo reply_to, bool option_key);			// Reply to selected messages
	bool	ReplyMessage(JIndex row,  EReplyTo* reply_to, bool* option_key);			// Reply to specified message
	
	void	OnMessageForward();
	void	DoMessageForward(bool option_key);
	bool	ForwardMessage(TableIndexT row, bool* option_key);	// Forward specified message

	void	OnMessageBounce();							// Bounce selected messages
	bool	BounceMessage(TableIndexT row);				// Bounce specified message

	void	OnMessageReject();
	bool	RejectMessage(TableIndexT row);						// Reject specified message

	void	OnMessageSendAgain();							// Send again selected messages
	bool	SendAgainMessage(TableIndexT row);				// Send again specified message

	void	OnMessageCreateDigest();						// Digest selected messages

	void	ResetSearch();								// Reset search UI items from mailbox
	void	OnMatch();
	void	OnMatchButton(bool option_key);
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

	virtual void 	ContextEvent(const JPoint& pt,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);

public:
	virtual	void	InitPos();								// Set initial position
	
	virtual void	Refresh() const;						// Update button & captions as well
	virtual void	RefreshSelection();						// Update button & captions as well
	virtual void	RefreshRow(TableIndexT aRow);			// Update button & captions as well

	virtual void	DoSelectionChanged();

	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

protected:

	void	UpdateFlagCommand(CCmdUI* cmdui, NMessage::EFlags flag);

	virtual void	UpdateItems();							// Update after possible change

	void	UpdateState();									// Update toolbar buttons
	void	UpdateCaptions();								// Update number captions
	void	UpdateCacheIncrement();							// Update cache increment

	virtual bool	ItemIsAcceptable(const JArray<Atom>& typeList); // Check its suitable
};

#endif
