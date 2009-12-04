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


// Header for CRulesTable class

#ifndef __CRULESTABLE__MULBERRY__
#define __CRULESTABLE__MULBERRY__

#include "CTableDrag.h"

#include "CFilterItem.h"

#include "cdstring.h"


// Consts

// Panes
const	PaneIDT		paneid_RulesBtnNewRule = 'NEWR';
const	PaneIDT		paneid_RulesBtnNewTarget = 'NEWT';
const	PaneIDT		paneid_RulesBtnEdit = 'EDIT';
const	PaneIDT		paneid_RulesBtnDelete = 'DELE';
const	PaneIDT		paneid_RulesBtnApply = 'APLY';
const	PaneIDT		paneid_RulesBtnNewScript = 'NEWS';
const	PaneIDT		paneid_RulesBtnGenerateScript = 'SCRP';

// Messages
const	MessageT	msg_RulesNewRule = 'NEWR';
const	MessageT	msg_RulesNewTarget = 'NEWT';
const	MessageT	msg_RulesEdit = 'EDIT';
const	MessageT	msg_RulesDelete = 'DELE';
const	MessageT	msg_RulesApply = 'APLY';
const	MessageT	msg_RulesNewScript = 'NEWS';
const	MessageT	msg_RulesGenerateScript = 'SCRP';

// Resources
const	ResIDT		Txtr_DefaultRulesList = 1000;

// Classes
class CMessageList;
class CRulesWindow;
class LBevelButton;
class LCheckBox;
class CStaticText;
class CScriptsTable;
class CTitleTable;

class CRulesTable : public CTableDrag,
						public LListener {

	friend class CRulesWindow;

private:
	CRulesWindow*			mWindow;				// Owner window
	CTitleTable*			mTitles;
	LBevelButton*			mNewBtn;
	LBevelButton*			mEditBtn;
	LBevelButton*			mDeleteBtn;
	LBevelButton*			mApplyBtn;
	ulvector				mDragged;
	unsigned long			mDropRow;
	CFilterItem::EType		mType;
	CScriptsTable*			mScriptsTable;

public:
	enum { class_ID = 'RuTb' };

					CRulesTable();
					CRulesTable(LStream *inStream);
	virtual 		~CRulesTable();

private:
			void	InitRulesTable(void);

protected:
	virtual void		FinishCreateSelf(void);			// Get details of sub-panes
public:
			void		SetFilterType(CFilterItem::EType type);
			void		SetScriptsTable(CScriptsTable* table)
				{ mScriptsTable = table; }

	bool	AddSelectionToList(TableIndexT row, CFilterItemList* list);		// Add selected rules to list

	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);
	virtual void		ListenToMessage(MessageT inMessage,
										void *ioParam);				// Respond to clicks in the icon buttons

protected:

	virtual void	ClickSelf(const SMouseDownEvent &inMouseDown);	// Click
	virtual void	ClickCell(const STableCell& inCell,
								const SMouseDownEvent& inMouseDown);// Click in the cell

	virtual void	DrawCell(const STableCell &inCell,
								const Rect &inLocalRect);			// Draw the message info

	virtual void	AdaptToNewSurroundings(void);				// Adjust column widths

	virtual void	BeTarget();
	virtual void	DontBeTarget();

protected:
	void	OnNewRules(void);

	void	OnEditRules(void);
	bool	EditRules(TableIndexT row);						// Display a specified address book

	void	OnDeleteRules(void);
	bool	DeleteRules(TableIndexT row);						// Delete a specified address book

	void	OnApplyRules(void);

	void	MakeRule(const CMessageList& msgs);						// Make rule from example messages

public:
	virtual void	DoSelectionChanged(void);
	virtual void	UpdateButtons(void);							// Update buttons
	virtual void	ResetTable(void);								// Reset the table from the mboxList
	virtual void	ScrollImageBy(SInt32 inLeftDelta,
									SInt32 inTopDelta,
									Boolean inRefresh);				// Keep titles in sync

// Drag methods
private:
	virtual void	AddCellToDrag(CDragIt* theDragTask,
									const STableCell& theCell,
									Rect& dragRect);				// Add cell to drag with rect

protected:
	virtual void	DoDragReceive(DragReference	inDragRef);			// Get multiple text items
	virtual void	DropDataAtCell(FlavorType theFlavor,
										char* drag_data,
										Size data_size,
										const STableCell& beforeCell);					// Drop data at cell
};

#endif
