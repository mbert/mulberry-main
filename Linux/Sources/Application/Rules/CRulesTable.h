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

#include "CTableDragAndDrop.h"

#include "CFilterItem.h"

// Classes
class CMessageList;
class CRulesWindow;
class CScriptsTable;
class CTitleTable;

class CRulesTable : public CTableDragAndDrop
{

	friend class CRulesWindow;

private:
	CRulesWindow*			mWindow;				// Owner window
	CTitleTable*			mTitles;
	ulvector				mDragged;
	unsigned long			mDropRow;
	CFilterItem::EType		mType;
	CScriptsTable*			mScriptsTable;

public:
	CRulesTable(JXScrollbarSet* scrollbarSet,
				 JXContainer* enclosure,
				 const HSizingOption hSizing,
				 const VSizingOption vSizing,
				 const JCoordinate x, const JCoordinate y,
				 const JCoordinate w, const JCoordinate h);
	virtual 		~CRulesTable();

private:
			void	InitRulesTable(void);

protected:
	virtual void	OnCreate();

public:
	virtual void	ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh);

			void		SetFilterType(CFilterItem::EType type);
			void		SetScriptsTable(CScriptsTable* table)
				{ mScriptsTable = table; }

	bool	AddSelectionToList(TableIndexT row, CFilterItemList* list);		// Add selected rules to list

	virtual bool		HandleChar(const int key, const JXKeyModifiers& modifiers);
	virtual bool		ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void		UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

protected:
	virtual void	LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers);

	virtual void	DrawCell(JPainter* p, const STableCell& inCell, const JRect& localRect);
	virtual void	ApertureResized(const JCoordinate dw, const JCoordinate dh);

protected:
	void	OnNewRules();

	void	OnEditRules();
	bool	EditRules(TableIndexT row);						// Display a specified address book

	void	OnDeleteRules(void);
	bool	DeleteRules(TableIndexT row);					// Delete a specified address book

	void	OnApplyRules(void);

	void	MakeRule(const CMessageList& msgs);						// Make rule from example messages

public:
	virtual void	DoSelectionChanged(void);
	virtual void	UpdateButtons(void);							// Update buttons
	virtual void	ResetTable(void);								// Reset the table from the mboxList

// Drag methods
protected:
			bool 	AddSelectionToDrag(TableIndexT row, CFilterItems* list);
	virtual bool	RenderSelectionData(CMulSelectionData* selection, Atom type);
	virtual bool	DropDataAtCell(Atom theFlavor,
										unsigned char* drag_data,
										unsigned long data_size, const STableCell& before_cell);	// Drop data at cell
};

#endif
