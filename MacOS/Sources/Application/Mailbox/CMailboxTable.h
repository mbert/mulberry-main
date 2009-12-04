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


// Header for CMailboxTable class

#ifndef __CMAILBOXTABLE__MULBERRY__
#define __CMAILBOXTABLE__MULBERRY__

#include "CMboxProtocol.h"
#include "CTableDrag.h"
#include "CMessageView.h"
#include "CMessageWindow.h"

#include <set>

// Panes
const	PaneIDT		paneid_MailboxSortingBtn = 'SORT';

// Messages
const	MessageT	msg_MailboxSortDirection = 'SORT';

// Resources
const	ResIDT		Txtr_DefaultMailbox = 1000;

// Classes
class CKeyModifiers;
class CMbox;
class CMessage;

class CMailboxTable : public CTableDrag, public LListener
{
	friend class CMailboxView;
	friend class CMailboxInfoView;

protected:
	CMailboxView*		mTableView;					// Owning view
	CMbox*				mMbox;						// Mbox associated with this table
	bool				mMboxError;					// Error indicator for mailbox
	ulset				mPreserveMsgs;				// Messages selected
	bool				mSelectionPreserved;		// Flag to indication preservation of selection in progress
	bool				mListChanging;				// In the process of changing the msg list
	bool				mUpdateRequired;			// Needs refresh
	bool				mIsSelectionValid;			// Cached selection valid state
	bool				mTestSelectionAndDeleted;	// Is entire selection deleted
	bool				mResetTable;				// Indicates that table was reset

	class StMailboxTablePostponeUpdate
	{
	public:
		StMailboxTablePostponeUpdate(CMailboxTable* aTable)
			{ _table = aTable; _changing = _table->mListChanging; _table->mListChanging = true; }
		~StMailboxTablePostponeUpdate()
			{ _table->mListChanging = _changing; if (!_changing && _table->mUpdateRequired) _table->Refresh(); }
	private:
		bool _changing;
		CMailboxTable* _table;
	};
	friend class StMailboxTablePostponeUpdate;

public:
	static bool		sDropOnMailbox;				// Flag for drop on a mailbox

					CMailboxTable();
					CMailboxTable(LStream *inStream);
	virtual 		~CMailboxTable();

private:
			void	InitMailboxTable();					// Do common init

protected:
	virtual void	FinishCreateSelf();					// Get details of sub-panes
public:
	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);
	virtual void	ListenToMessage(MessageT inMessage,
											void *ioParam);	// Respond to clicks in the icon buttons
protected:
	typedef std::pair<bool, bool> (CMailboxTable::*TestSelectionIgnorePP)(TableIndexT);
	typedef std::pair<bool, bool> (CMailboxTable::*TestSelectionIgnore1PP)(TableIndexT, NMessage::EFlags);

	bool				TestSelectionIgnoreAnd(TestSelectionIgnorePP proc)							// Test each cell in the selection using logical and
		{ return TestSelectionIgnore(proc, true); }
	bool				TestSelectionIgnore1And(TestSelectionIgnore1PP proc, NMessage::EFlags flag)	// Test each cell in the selection using logical and
		{ return TestSelectionIgnore1(proc, flag, true); }
	bool				TestSelectionIgnoreOr(TestSelectionIgnorePP proc)							// Test each cell in the selection using logical or
		{ return TestSelectionIgnore(proc, false); }
	bool				TestSelectionIgnore1Or(TestSelectionIgnore1PP proc, NMessage::EFlags flag)		// Test each cell in the selection using logical or
		{ return TestSelectionIgnore1(proc, flag, false); }

	bool				TestSelectionIgnore(TestSelectionIgnorePP proc, bool and_it);		// Test each cell in the selection using logical and/or
	bool				TestSelectionIgnore1(TestSelectionIgnore1PP proc, NMessage::EFlags flag, bool and_it);		// Test each cell in the selection using logical and/or

	std::pair<bool, bool>	TestSelectionFlag(TableIndexT row, NMessage::EFlags flag);			// Test for selected message flag

	std::pair<bool, bool>	TestSelectionSmart(TableIndexT row);				// Test for an outgoing message
	bool				TestSelectionFake(TableIndexT row);					// Test for a fake message

	virtual void	ClickCell(const STableCell& inCell,
								const SMouseDownEvent &inMouseDown);	// Click in the cell
	virtual void	ClickSelf(const SMouseDownEvent &inMouseDown);	// Deselect if not valid cell click
	virtual void	DrawSelf();
	virtual void	DrawRow(TableIndexT row,
								TableIndexT start_col,
								TableIndexT stop_col);		// Draw the message info
	virtual void	DrawMessage(const CMessage* aMsg,
								const STableCell& inCell,
								const Rect &inLocalRect,
								RGBColor text_color,
								Style text_style,
								bool strike_through);		// Draw the message info

	virtual void		GetDrawStyle(const CMessage* aMsg, RGBColor& color, Style& style, bool& strike) const;
	virtual ResIDT		GetPlotFlag(const CMessage* aMsg) const;
	virtual bool		UsesBackground(const CMessage* aMsg) const;
	virtual RGBColor	GetBackground(const CMessage* aMsg) const;

	virtual void	HiliteCellActively(const STableCell &inCell,
										Boolean inHilite);
	virtual void	HiliteCellInactively(const STableCell &inCell,
										Boolean inHilite);

	virtual void	DoSingleClick(unsigned long row, unsigned long col, const CKeyModifiers& mods);					// Single click on a message
	virtual void	DoDoubleClick(unsigned long row, unsigned long col, const CKeyModifiers& mods);					// Double click on a message

	virtual void	DoFileOpenMessage(bool option_key);					// Display a mail message
	virtual bool	OpenMailMessage(TableIndexT row, bool* option_key);	// Display a specified mail message

	virtual void	OnFileSave();									// Save a mail message
	virtual bool	SaveMailMessage(TableIndexT row);				// Save a specified mail message

	virtual void	DoPrintMailMessage();							// Print a mail message
	virtual bool	PrintMailMessage(TableIndexT row,
								LPrintSpec* printRecordH);			// Print a specified mail message

	virtual bool	DoMailboxExpunge(bool closing = false,			// Expunge mailbox (may be a substitiute)
										CMbox* substitute = NULL);

	virtual void	DoFlagMailMessage(NMessage::EFlags flag,		// Change selected mail messages flag
										bool set = true);

	virtual void	DoMessageCopy(CMbox* copy_mbox,
									bool option_key,
									bool force_delete = false);		// Copy selected messages

	virtual void	DoSpeakMessage();								// Speak selected messages
	virtual void	DoSpeakRecentMessages();						// Speak all recent messages
	virtual bool	SpeakMessage(TableIndexT row, bool* recent);	// Speak selected/recent message

	virtual void	DoCaptureAddress();								// Capture address from selected messages
	virtual bool	CaptureAddress(TableIndexT row);				// Capture address from message

public:
	virtual	void	SetMbox(CMbox* anMbox);					// Set the mbox
			CMbox*	GetMbox() const							// Get the mbox
				{ return mMbox; }
			void	ForceClose();							// Force it closed
	
	virtual	void	InitPos();								// Set initial position
	
	bool	PreserveSelection();							// Preserve current selection
	void	UnpreserveSelection();							// Unpreserve current selection
	bool	PreserveMessage(TableIndexT row);				// Preserve message
	void	ResetSelection(bool scroll);					// Reset selection to what it was
	void	SelectionNudge(bool previous = false);			// Move selection on to next message

	virtual void	DoSelectionChanged();

protected:
	bool			mUpdating;								// Updating flag
	unsigned long	mPreviewUID;							// UID of message being previewed

	void	BeginMboxUpdate()								// Starting possible update
		{ mUpdating = true; }
	void	EndMboxUpdate(bool update = true)				// Update after possible change
		{ mUpdating = false; if (update) UpdateItems(); }
	virtual void	UpdateItems() = 0;						// Update after possible change
	virtual void	UpdateState() = 0;						// Update button/title state

	void	ResetTable(bool scroll_new = false);			// Reset the table from the mbox
	void	ClearTable();									// Clear the table of all contents
	void	ScrollForNewMessages();							// Scroll to ensure new messages are in view

	unsigned long GetPreviewMessageUID() const				// Get the current message UID being previewed
		{ return mPreviewUID; }
	void	PreviewMessage(bool clear = false);				// Do possible message preview
	void	PreviewMessageUID(unsigned long uid,			// Message preview via UI
								const CMessageView::SMessageViewState* state);
	
	void	HandleSpacebar(bool shift_key);				// Spacebar scroll page/preview

	void	GetScrollPos(long& h, long& v) const;			// Get scrollbar pos
	void	SetScrollPos(long h, long v);					// Set scrollbar pos

	const ulset&	GetPreservedMessages() const			// Get the preserved set
		{ return mPreserveMsgs; }
	void			SetPreservedMessages(ulset pset)		// Set the preserved set
		{ mPreserveMsgs = pset; }

public:
	virtual void	ScrollImageBy(SInt32 inLeftDelta,
									SInt32 inTopDelta,
									Boolean inRefresh);		// Keep titles in sync

// Drag methods
protected:
	bool	AddSelectionToList(TableIndexT row, CMessageList* list);				// Add selected messages to list
	bool	AddFullSelectionToList(TableIndexT row, CMessageList* list);			// Add selected messages to list
	bool	AddFullSelectionWithFakesToList(TableIndexT row, CMessageList* list);				// Add selected messages to list
	bool	AddMsgSelectionToList(TableIndexT row, CMessageList* list, bool only_cached, bool fakes = false);			// Add selected messages to list

	virtual void	AddCellToDrag(CDragIt* theDragTask,
										const STableCell& aCell,
										Rect& dragRect);			// Add mail message to drag

	virtual bool	IsCopyCursor(DragReference inDragRef);			// Use copy cursor?
	virtual void	DoDragReceive(DragReference	inDragRef);			// Get multiple text items
	virtual void	DropData(FlavorType theFlavor,
									char* drag_data,
									Size data_size);				// Drop data into table

	virtual void	DoDragSendData(FlavorType inFlavor,
									ItemReference inItemRef,
									DragReference inDragRef);	// Other flavor requested by receiver
};

class StMailboxTableSelection : public CTableDrag::StTableAction
{
public:
	StMailboxTableSelection(CMailboxTable* aTable, bool scroll = true);
	~StMailboxTableSelection();

private:
	CMbox* mTemp;
	bool mScroll;
	bool mPreserved;
	CTableDrag::StDeferSelectionChanged mUpdateDefer;
};

#endif
