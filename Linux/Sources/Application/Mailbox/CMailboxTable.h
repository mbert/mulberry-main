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
#include "CTableDragAndDrop.h"
#include "CMessageView.h"
#include "CMessageWindow.h"
#include <JColorList.h>
#include <set>
#include <vector>

// Classes
class CKeyModifiers;
class CMbox;
class CMessage;
class JXTextMenu;

class CMailboxTable : public CTableDragAndDrop
{
	friend class CMailboxView;
	friend class CMailboxInfoView;
	friend class CMailboxTitleTable;

protected:
	CMailboxView*		mTableView;					// Owning 	CMbox*				mMbox;						// Mbox associated with this table
	CMbox*				mMbox;									// Mbox associated with this table
	bool				mMboxError;					// Error indicator for mailbox
	ulset				mPreserveMsgs;				// Messages selected
	bool				mSelectionPreserved;		// Flag to indication preservation of selection in progress
	bool				mListChanging;				// In the process of changing the msg list
	bool				mUpdateRequired;			// Needs refresh
	bool				mIsSelectionValid;			// Cached selection valid state
	bool				mTestSelectionAndDeleted;	// Is entire selection deleted
	bool				mResetTable;				// Indicates that table was reset
	unsigned long		mPreviewUID;				// UID of message being previewed
	CMbox*				mCopyFromMbox;				// Mbox to copy from during drop
	ulvector			mCopyMsgNums;				// Messages numbers to copy during drop

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

	CMailboxTable(JXScrollbarSet* scrollbarSet,
					 JXContainer* enclosure,
					 const HSizingOption hSizing,
					 const VSizingOption vSizing,
					 const JCoordinate x, const JCoordinate y,
					 const JCoordinate w, const JCoordinate h);
	virtual 		~CMailboxTable();

	virtual void	ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh);

private:
			void	InitMailboxTable();					// Do common init

protected:
	virtual void	OnCreate();

	void	OnUpdateSelectionUndeleted(CCmdUI* cmdui);
	void	OnUpdateMailboxExpunge(CCmdUI* cmdui);
	void	OnUpdateSortMenu(CCmdUI* cmdui);
	void	OnUpdateMessageCopy(CCmdUI* cmdui);
	void	OnUpdateMessageDelete(CCmdUI* cmdui);

	typedef std::pair<bool, bool> (CMailboxTable::*TestSelectionIgnorePP)(TableIndexT);
	typedef std::pair<bool, bool> (CMailboxTable::*TestSelectionIgnore1PP)(TableIndexT, NMessage::EFlags);

	bool				TestSelectionIgnoreAnd(TestSelectionIgnorePP proc)		// Test each cell in the selection using logical and
		{ return TestSelectionIgnore(proc, true); }
	bool				TestSelectionIgnore1And(TestSelectionIgnore1PP proc, NMessage::EFlags flag)	// Test each cell in the selection using logical and
		{ return TestSelectionIgnore1(proc, flag, true); }
	bool				TestSelectionIgnoreOr(TestSelectionIgnorePP proc)		// Test each cell in the selection using logical or
		{ return TestSelectionIgnore(proc, false); }
	bool				TestSelectionIgnore1Or(TestSelectionIgnore1PP proc, NMessage::EFlags flag)		// Test each cell in the selection using logical or
		{ return TestSelectionIgnore1(proc, flag, false); }

	bool				TestSelectionIgnore(TestSelectionIgnorePP proc, bool and_it);		// Test each cell in the selection using logical and/or
	bool				TestSelectionIgnore1(TestSelectionIgnore1PP proc, NMessage::EFlags flag, bool and_it);		// Test each cell in the selection using logical and/or

	std::pair<bool, bool>	TestSelectionFlag(TableIndexT row, NMessage::EFlags flag);			// Test for selected message flag

	std::pair<bool, bool>	TestSelectionSmart(TableIndexT row);				// Test for an outgoing message
	bool				TestSelectionFake(TableIndexT row);					// Test for a fake message

	virtual void LClickCell(const STableCell& cell, const JXKeyModifiers& modifiers);
	virtual void LDblClickCell(const STableCell& cell, const JXKeyModifiers& modifiers);

	virtual void Draw(JXWindowPainter& pDC, const JRect& localRect);
	virtual void DrawRow(JPainter* pDC, TableIndexT row,
								TableIndexT start_col,
								TableIndexT stop_col);		// Draw the message info
	virtual void DrawMessage(JPainter* pDC,
							 const CMessage* aMsg,
							 const STableCell& cell,
							 const JRect& inLocalRect,
							 const JRGB& color,
							 short text_style,
							 unsigned long bkgnd);

	virtual void		GetDrawStyle(const CMessage* aMsg, JRGB& color, short& style) const;

	virtual long		GetPlotFlag(const CMessage* aMsg) const;
	virtual bool		UsesBackground(const CMessage* aMsg) const;
	virtual JColorIndex	GetBackground(const CMessage* aMsg) const;

	virtual void	DoSingleClick(TableIndexT row, TableIndexT col, const CKeyModifiers& mods);					// Single click on a message
	virtual void	DoDoubleClick(TableIndexT row, TableIndexT col, const CKeyModifiers& mods);					// Double click on a message

	virtual void	DoFileOpenMessage(bool ctrl_key);					// Display a mail message
	virtual bool	OpenMailMessage(TableIndexT row, bool* option_key);	// Display a specified mail message

	virtual void	OnFileSave();								// Save a mail message
	virtual bool	SaveMailMessage(TableIndexT row);					// Save a specified mail message


	void OnFileOpenMessage();
	void OnFilePrint();
	virtual bool	PrintMailMessage(TableIndexT row);					// Print a specified mail message
	void OnMailboxExpunge();
	void OnSortDirection();
	void OnSortItem(JIndex choice);
	void OnMessageCopy(JXTextMenu* menu, JIndex choice);
	void OnMessageCopyPopup();
	void OnMessageCopyNow();
	void OnMessageCopyCmd();							// Explicit copy
	void OnMessageMoveCmd();							// Explicit move
	void OnMessageCopyIt(bool option_key);				// Common copy behaviour

	virtual void	OnMessageDelete();

	virtual void	OnCaptureAddress();
	virtual void	DoCaptureAddress();								// Capture address from selected messages
	virtual bool	CaptureAddress(TableIndexT row);				// Capture address from message

	virtual bool	DoMailboxExpunge(bool closing,
										CMbox* substitute = NULL);

	virtual void	DoFlagMailMessage(NMessage::EFlags flag,	// Change selected mail messages flag
																	bool set = true);

	virtual void	DoMessageCopy(CMbox* copy_mbox,
										bool option_key,
										bool force_delete = false);	// Copy selected messages

public:
	virtual	void	SetMbox(CMbox* anMbox);					// Set the mbox
	CMbox*	GetMbox() const								// Get the mbox
		{ return mMbox; }
			void	ForceClose();							// Force it closed
	
	virtual	void	InitPos();								// Set initial position
	
	bool	PreserveSelection();							// Preserve current selection
	void	UnpreserveSelection();							// Unpreserve current selection
	bool	PreserveMessage(TableIndexT row);				// Preserve message
	void	ResetSelection(bool scroll);					// Reset selection to what it was
	void	SelectionNudge(bool previous = false);			// Move selection by one item

	virtual void	DoSelectionChanged();

	virtual bool HandleChar(const int key, const JXKeyModifiers& modifiers);

	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

protected:
	bool	mUpdating;										// Updating flag

	void	BeginMboxUpdate()								// Starting possible update
		{ mUpdating = true; }
	void	EndMboxUpdate(bool update = true)				// Update after possible change
		{ mUpdating = false; if (update) UpdateItems(); Refresh();}
	virtual void	UpdateItems() = 0;						// Update after possible change
	virtual void	UpdateState() = 0;						// Update after possible change

	void	ResetTable(bool scroll_new = false);				// Reset the table from the mbox
	void	ClearTable();										// Clear the table of all contents
	void	ScrollForNewMessages();								// Scroll to ensure new messages are in view

	unsigned long GetPreviewMessageUID() const				// Get the current message UID being previewed
		{ return mPreviewUID; }
	void	PreviewMessage(bool clear = false);					// Do possible message preview
	void	PreviewMessageUID(unsigned long uid,			// Message preview via UI
								const CMessageView::SMessageViewState* state);

	void	HandleSpacebar(bool shift_key);					// Spacebar scroll page/preview

	void	GetScrollPos(long& h, long& v) const;			// Get scrollbar pos
	void	SetScrollPos(long h, long v);					// Set scrollbar pos

	const ulset&	GetPreservedMessages() const			// Get the preserved set
		{ return mPreserveMsgs; }
	void			SetPreservedMessages(ulset pset)		// Set the preserved set
		{ mPreserveMsgs = pset; }	

// Drag methods
protected:
	virtual bool	RenderSelectionData(CMulSelectionData* selection, Atom type);
			bool	AddSelectionToDrag(TableIndexT row, CMessageList* list); // Add selected messages to list
			bool	AddSelectionToList(TableIndexT row, CMessageList* list); // Add selected messages to list
			bool	AddFullSelectionToList(TableIndexT row, CMessageList* list);							// Add selected messages to list
			bool	AddFullSelectionWithFakesToList(TableIndexT row, CMessageList* list);				// Add selected messages to list
			bool	AddMsgSelectionToList(TableIndexT row, CMessageList* list, bool only_cached, bool fakes = false);			// Add selected messages to list

	virtual Atom	GetDNDAction(const JXContainer* target, const JXButtonStates& buttonStates, const JXKeyModifiers& modifiers);

	virtual bool	DropData(Atom theFlavor,
							 unsigned char* drag_data,
							 unsigned long data_size);     // Drop data into whole table

	static JColorList* sColorList;
protected:
	enum
	{
		eDeleted_Flag = 0,
		eAnswered_Flag,
		eRecent_Flag,
		eUnseen_Flag,
		eRead_Flag,
		eDraft_Flag,
		eAttachment_Flag,
		eAlternative_Flag,
		eSigned_Flag,
		eEncrypted_Flag,
		eMatch_Flag,
		eCached_Flag,
		ePartial_Flag,
		ePaused_Flag,
		eSending_Flag,
		eSendError_Flag,
		eSigned_Attachment_Flag,
		eSigned_Alternative_Flag,
		eSigned_Alternative_Attachment_Flag,
		eAlternative_Attachment_Flag,
		eMaxFlag = eAlternative_Attachment_Flag
	};
	enum
	{
		eSmartFrom_Flag = eMaxFlag + 1,
		eSmartTo_Flag,
		eSmartCC_Flag,
		eSmartBCC_Flag,
		eMaxSmartFlag = eSmartBCC_Flag
	};

};

class StMailboxTableSelection : public CTable::StTableAction
{
public:
	StMailboxTableSelection(CMailboxTable* aTable, bool scroll = true);
	~StMailboxTableSelection();

private:
	CMbox* mTemp;
	bool mScroll;
	bool mPreserved;
	CTable::StDeferSelectionChanged mUpdateDefer;
};

#endif
