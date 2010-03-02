/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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
#include "CMboxFwd.h"
#include "CMessage.h"

#include <set>

// Classes
class CMbox;
class CMessageList;
class CSearchItem;

class CMailboxInfoTable : public CMailboxTable
{

	friend class CMailboxInfoWindow;
	friend class CMailboxInfoView;

	DECLARE_DYNCREATE(CMailboxInfoTable)

public:
	enum
	{
		eSelectBy_Seen = IDM_SELECT_SEEN,
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
		eSelectBy_Label1 = IDM_SELECT_LABEL1,
		eSelectBy_Label2,
		eSelectBy_Label3,
		eSelectBy_Label4,
		eSelectBy_Label5,
		eSelectBy_Label6,
		eSelectBy_Label7,
		eSelectBy_Label8
	};

					CMailboxInfoTable();
	virtual 		~CMailboxInfoTable();

	virtual void	InitPos(void);							// Set initial position

	virtual BOOL	RedrawWindow(LPCRECT lpRectUpdate = NULL, CRgn* prgnUpdate = NULL, UINT flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE );
	virtual void	RefreshSelection(void);										// Update button & captions as well
	virtual void	RefreshRow(TableIndexT row);								// Update button & captions as well

	virtual void	SelectClearMessages(void);									// Selected mail messages to be cleared (seen but not deleted)

	virtual	void	GetTooltipText(cdstring& txt, const STableCell& cell);				// Get text for current tooltip cell

	// Key/Mouse related

	// Common updaters
	afx_msg void	OnUpdateFileClose(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMailboxSubscribe(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMailboxUnsubscribe(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMailboxSynchronise(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMailboxClearDisconnect(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMailboxPartialCache(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMailboxUpdateMatch(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMailboxApplyRules(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMailboxApplyRulesBtn(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMailboxMakeRule(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMessageReject(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMessageSendAgain(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateFlagsDeleted(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateFlagsSeen(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateFlagsAnswered(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateFlagsImportant(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateFlagsDraft(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateFlagsLabel(CCmdUI* pCmdUI);
			void	OnUpdateFlags(CCmdUI* pCmdUI, NMessage::EFlags flag);
	afx_msg void	OnUpdateMatchButton(CCmdUI* pCmdUI);

	// Command handlers
	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command

	virtual void	DoSingleClick(unsigned long row, unsigned long col, const CKeyModifiers& mods);					// Single click on a message

	afx_msg void	OnFileNewDraft(void);
			void	DoNewLetter(bool option_key);				// Display a new draft

	afx_msg void	OnFileClose(void);
			bool	CanFileClose() const;						// Should File:Close be active

			void	SelectByRow(const ulvector& rows);			// Select all messages in specific rows
			void	SelectNextNew(bool shift);					// Select next new message
			void	DoRollover(bool ask = true);				// Rollover to next recent
			void	CycleTabs(bool forward);					// Cycle to next/previous tab

	afx_msg void	OnMailboxProperties(void);

	afx_msg void	OnMailboxPunt(void);

	afx_msg void	OnMailboxSynchronise(void);					// Synchronise
	afx_msg void	OnMailboxClearDisconnect(void);				// Clear disconnected cache

	afx_msg void	OnMailboxSubscribe(void);
	afx_msg void	OnMailboxUnsubscribe(void);

	afx_msg void	OnMailboxCheck(void);						// Check the selected mailboxes
	afx_msg void	OnMailboxGotoMessage();						// Select a specific message
	afx_msg void	OnMailboxCacheAll();						// Cache all messages in mailbox
	afx_msg	void	OnMailboxUpdateMatch();						// Update current match state
	afx_msg void	OnSearchMailbox(void);						// Search the mailbox
	afx_msg void	OnSearchAgainMailbox(void);					// Search the mailbox again
	afx_msg void	OnNextSearchMailbox(void);					// Recycle to next search mbox

	afx_msg void	OnApplyRuleMailbox(UINT index);				// Apply rules
	afx_msg void	DoApplyRuleMailbox(long index);				// Apply rules
			void	OnApplyRuleMailboxToolbar(const char* name);	// Apply rules toolbar item
	afx_msg void	OnMakeRuleMailbox();						// Make rule

	afx_msg void	OnMessageReply(void);
	afx_msg void	OnMessageReplySender(void);
	afx_msg void	OnMessageReplyFrom(void);
	afx_msg void	OnMessageReplyAll(void);
	afx_msg void	DoMessageReply(EReplyTo reply_to, bool option_key);
	virtual bool	ReplyMessage(TableIndexT row, EReplyTo* reply_to, bool* option_key);	// Reply to message
	afx_msg void	OnMessageForward(void);
	virtual void	DoMessageForward(bool option_key);
	virtual bool	ForwardMessage(TableIndexT row, bool* option_key);	// Forward specified message
	afx_msg void	OnMessageBounce(void);
	virtual bool	BounceMessage(TableIndexT row);						// Bounce specified message
	afx_msg void	OnMessageReject(void);
	virtual bool	RejectMessage(TableIndexT row);						// Reject specified message
	afx_msg void	OnMessageSendAgain(void);
	virtual bool	SendAgainMessage(TableIndexT row);					// Send again specified message
	afx_msg void	OnMessageCreateDigest(void);						// Create digest
	afx_msg void	OnFlagsSeen(void);
	afx_msg void	OnFlagsAnswered(void);
	afx_msg void	OnFlagsImportant(void);
	afx_msg void	OnFlagsDraft(void);
	afx_msg void	OnFlagsLabel(UINT nID);

	        void	ResetSearch();								// Reset search UI items from mailbox
	afx_msg void	OnMatch();
	afx_msg void	OnMatchOption();
			void	OnMatchButton(bool option_key);
			void	SetMatch(NMbox::EViewMode mbox_mode,		// Set match mode and criteria
						CSearchItem* spec,
						bool is_search,
						bool force_automatch = false);

	void	SetCurrentMatch(const CMatchItem& match)
		{ mMatch = match; }
	CMatchItem	GetCurrentMatch() const
		{ return mMatch; }

	afx_msg void	OnSelectPopup(UINT nID);
	afx_msg void	OnCacheButton(void);

	afx_msg void	OnThreadSelectAll();
	afx_msg void	OnThreadMarkSeen();
	afx_msg void	OnThreadMarkImportant();
	afx_msg void	OnThreadMarkDeleted();
			void	DoThreadFlag(NMessage::EFlags flag);
	afx_msg void	OnThreadSkip();

	afx_msg void	OnMatchUnseen();
	afx_msg void	OnMatchImportant();
	afx_msg void	OnMatchDeleted();
	afx_msg void	OnMatchSentToday();
	afx_msg void	OnMatchFrom();
	afx_msg void	OnMatchSubject();
	afx_msg void	OnMatchDate();
			void	DoMatch(const CMatchItem& match);


protected:	
	CMailboxInfoView*	mInfoTableView;							// Owning view
	CMatchItem			mMatch;									// Matching for this pane
	bool				mSearchResults;							// Displaying search results
	unsigned long		mLastTyping;							// Time of last typed character
	char				mLastChar;								// Last characters typed

	virtual	bool	HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);		// Handle key down
	virtual	bool	HandleChar(UINT nChar, UINT nRepCnt, UINT nFlags);			// Handle character

	virtual void	DoSelectionChanged(void);
	virtual void	UpdateItems(void);							// Update after possible change
	void			UpdateCacheIncrement();						// Update cache increment
	virtual void	UpdateState(void);							// Update buttons
	virtual void	UpdateCaptions(void);						// Update captions

	virtual bool	ItemIsAcceptable(COleDataObject* pDataObject);				// Check its suitable

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

#endif
