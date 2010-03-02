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


// Header for CMailboxTable class

#ifndef __CMAILBOXTABLE__MULBERRY__
#define __CMAILBOXTABLE__MULBERRY__

#include "CTableDragAndDrop.h"

#include "CMessage.h"
#include "CMessageView.h"
#include "CMessageWindow.h"
#include "CWindowStatesFwd.h"

#include <set>

// Classes
class CKeyModifiers;
class CMbox;
class CMessageList;

class CMailboxTable : public CTableDragAndDrop
{
	friend class CMailboxView;
	friend class CMailboxInfoView;
	friend class CMailboxTitleTable;

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

public:
	static bool	sDropOnMailbox;								// Flag for drop on a mailbox

					CMailboxTable();
	virtual 		~CMailboxTable();

	virtual void	ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh);

			void	SetMbox(CMbox* anMbox);					// Set the mbox
			CMbox*	GetMbox() const							// Get the mbox
						{ return mMbox; }
			void	ForceClose();							// Force it closed

	virtual int		GetSortBy() const;
	virtual void	InitPos();								// Set initial position

	virtual BOOL	RedrawWindow(LPCRECT lpRectUpdate = NULL, CRgn* prgnUpdate = NULL, UINT flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE ) = 0;

	virtual void	SetColumnInfo(CColumnInfoArray& col_info)	// Reset header details from array
						{ mColumnInfo = &col_info; }

	bool	PreserveSelection();							// Preserve current selection
	void	UnpreserveSelection();							// Unpreserve current selection
	bool	PreserveMessage(TableIndexT row);				// Preserve message
	void	ResetSelection(bool scroll);					// Reset selection to what it was
	void	SelectionNudge(bool previous = false);			// Move selection by one item

	virtual void	DeleteSelection()						// Delete selected cells
						{ OnMessageDelete(); }

	// Key/Mouse related

	// Common updaters
	afx_msg void	OnUpdateEditSpeak(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMailboxExpunge(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateSortMenu(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateSelectionNotDeleted(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMessageCopy(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateMessageDelete(CCmdUI* pCmdUI);

	// Command handlers
	virtual void	DoSingleClick(TableIndexT row, TableIndexT col, const CKeyModifiers& mods);					// Single click on a message
	virtual void	DoDoubleClick(TableIndexT row, TableIndexT col, const CKeyModifiers& mods);					// Double click on a message

	afx_msg void	OnFileSave();
	virtual bool	SaveMailMessage(TableIndexT row);			// Save a specified mail message
	afx_msg void	OnFilePrint();
	afx_msg void	OnFilePrintPreview();
	virtual bool	PrintMailMessage(TableIndexT row, void* preview);	// Print/preview a specified mail message
	afx_msg void	OnFileOpenMessage();
	virtual void	DoFileOpenMessage(bool option_key);
	virtual bool	OpenMailMessage(TableIndexT row, bool* option_key);	// Open a specified mail message

	afx_msg void	OnEditSpeak();

	afx_msg void	OnMailboxExpunge();
	virtual bool	DoMailboxExpunge(bool closing,
										CMbox* substitute = NULL);

	afx_msg void	OnSortDirection();
	afx_msg void	OnSortItem(UINT nID);

	afx_msg void	OnMessageCopy(UINT nID);					// Copy selected messages
	afx_msg void	OnMessageCopyPopup(UINT nID);				// Copy selected messages
	afx_msg void	OnMessageCopyNow();							// Copy via Choose
	afx_msg void	OnMessageCopyCmd();							// Explicit copy
	afx_msg void	OnMessageMoveCmd();							// Explicit move
			void	OnMessageCopyIt(bool option_key);				// Common copy behaviour
			void	DoMessageCopy(CMbox* mbox_copy,
									bool option_key,
									bool force_delete = false);		// Copy selected messages
	afx_msg void	OnMessageDelete();
	virtual void	DoFlagMailMessage(NMessage::EFlags flag,		// Change selected mail messages flag
										bool set = true);

	virtual void	DoSpeakMessage();								// Speak selected messages
	virtual void	DoSpeakRecentMessages();						// Speak all recent messages
	virtual bool	SpeakMessage(TableIndexT row, bool* recent);	// Speak selected/recent message

	afx_msg void	OnCaptureAddress();
	virtual void	DoCaptureAddress();								// Capture address from selected messages
	virtual bool	CaptureAddress(TableIndexT row);				// Capture address from message

protected:	
	virtual	bool	HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);		// Handle key down
	virtual void	LClickCell(const STableCell& inCell, UINT nFlags);
	virtual void	LDblClickCell(const STableCell& inCell, UINT nFlags);

	virtual void	HandleContextMenu(CWnd*, CPoint point);

	// Drag & Drop
	virtual BOOL 	DoDrag(TableIndexT row);
			bool	AddSelectionToDrag(TableIndexT row, CMessageList* list);											// Add row to list
	virtual BOOL	OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);
			bool	AddSelectionToList(TableIndexT row, CMessageList* list);											// Add row to list
			bool	AddFullSelectionToList(TableIndexT row, CMessageList* list);										// Add selected messages to list
			bool	AddFullSelectionWithFakesToList(TableIndexT row, CMessageList* list);								// Add selected messages to list
			bool	AddMsgSelectionToList(TableIndexT row, CMessageList* list, bool only_cached, bool fakes = false);	// Add selected messages to list
	virtual DROPEFFECT GetDropEffect(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);			// Determine effect
	virtual bool	DropData(unsigned int theFlavor, char* drag_data, unsigned long data_size);							// Drop data

protected:
	CMailboxView*		mTableView;								// Owning view
	CMbox*				mMbox;									// Mbox associated with this table
	bool				mMboxError;								// Error indicator for mailbox
	CColumnInfoArray*	mColumnInfo;
	ulset				mPreserveMsgs;							// Messages selected
	bool				mSelectionPreserved;					// Flag to indication preservation of selection in progress
	static int			sSortCol;								// Header column clicked is sorted
	bool				mUpdating;								// Updating flag
	bool				mListChanging;							// In the process of changing the msg list
	bool				mUpdateRequired;						// Needs refresh
	bool				mIsSelectionValid;						// Cached selection valid state
	bool				mTestSelectionAndDeleted;				// Is entire selection deleted
	bool				mResetTable;							// Indicates that table was reset
	unsigned long		mPreviewUID;							// UID of message being previewed

	class StMailboxTablePostponeUpdate
	{
	public:
		StMailboxTablePostponeUpdate(CMailboxTable* aTable)
			{ _table = aTable; _changing = _table->mListChanging; _table->mListChanging = true; }
		~StMailboxTablePostponeUpdate()
			{ _table->mListChanging = _changing; if (!_changing && _table->mUpdateRequired) _table->RedrawWindow(NULL, NULL, RDW_INVALIDATE); }
	private:
		bool _changing;
		CMailboxTable* _table;
	};
	friend class StMailboxTablePostponeUpdate;

	void	BeginMboxUpdate()									// Starting possible update
		{ mUpdating = true; }
	void	EndMboxUpdate(bool update = true)					// Update after possible change
		{ mUpdating = false; if (update) UpdateItems(); }
	virtual void	DoSelectionChanged();
	virtual void	UpdateItems() = 0;							// Update after possible change
	virtual void	UpdateState() = 0;							// Update button/title state

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

	afx_msg void	OnPaint();
	virtual void	DrawRow(CDC* pDC, TableIndexT row,
								TableIndexT start_col,
								TableIndexT stop_col);		// Draw the message info
	virtual void	DrawMessage(CDC* pDC, 
								const CMessage* aMsg,
								const STableCell& inCell,
								const CRect &inLocalRect,
								COLORREF text_color,
								short text_style,
								bool strike_through);		// Draw the message info

	virtual void		GetDrawStyle(const CMessage* aMsg, COLORREF& color, short& style, bool& strike) const;
	virtual int			GetPlotFlag(const CMessage* aMsg) const;
	virtual bool		UsesBackground(const CMessage* aMsg) const;
	virtual COLORREF	GetBackground(const CMessage* aMsg) const;

protected:
	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

class StMailboxTableSelection : public CTable::StTableAction
{
public:
	StMailboxTableSelection(CMailboxTable* aTable, bool scroll = true);
	~StMailboxTableSelection();

private:
	CMbox*			mTemp;
	bool			mScroll;
	bool			mPreserved;
	CTable::StDeferSelectionChanged mUpdateDefer;
};

#endif
