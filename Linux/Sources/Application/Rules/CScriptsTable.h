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


// Header for CScriptsTable class

#ifndef __CSCRIPTSTABLE__MULBERRY__
#define __CSCRIPTSTABLE__MULBERRY__

#include "CHierarchyTableDrag.h"

#include "CFilterItem.h"

// Classes
class CRulesWindow;
class CTitleTable;

class CScriptsTable : public CHierarchyTableDrag
{
	friend class CRulesWindow;

public:
	CScriptsTable(JXScrollbarSet* scrollbarSet, 
					JXContainer* enclosure,
					const HSizingOption hSizing, 
					const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);
	virtual 		~CScriptsTable();

	virtual void		OnCreate();

private:
	CRulesWindow*			mWindow;
	CFilterItem::EType		mType;

			void		InitScriptsTable();

public:
			void		SetFilterType(CFilterItem::EType type);

	virtual bool		HandleChar(const int key, const JXKeyModifiers& modifiers);
	virtual bool		ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void		UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

protected:
	virtual void		LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers);

	virtual void		DrawCell(JPainter* p, const STableCell& inCell, const JRect& localRect);
	virtual void		ApertureResized(const JCoordinate dw, const JCoordinate dh);

public:
	virtual void		DoSelectionChanged();
	virtual void		UpdateButtons();								// Update buttons
	virtual void		ResetTable();									// Reset the table from the entry list

protected:
	void		OnNewScript();									// Create a new address
	void		OnEditScript();									// Edit selected entries
	bool		EditScript(TableIndexT row);					// Edit specified address
	void		OnDeleteScript();								// Delete selected entries
	bool		DeleteScripts(TableIndexT row);					// Delete a specified entry
	void		OnGenerateScript();								// Generate selected entries
	bool		GenerateScript(TableIndexT row, void* data);	// Generate a specified entry

// Drag methods
	bool				TestSelectionScript(TableIndexT row);				// Test for selected item script
	bool				TestSelectionRule(TableIndexT row);					// Test for selected item rule
	bool				TestCellSameScript(TableIndexT row,
												TableIndexT* parent);		// Test for selected item group

	virtual Atom		GetDNDAction(const JXContainer* target, const JXButtonStates& buttonStates, const JXKeyModifiers& modifiers);
	virtual bool		IsDropCell(JArray<Atom>& typeList, const STableCell& cell);		// Test drop into cell
	virtual bool		IsDropAtCell(JArray<Atom>& typeList, STableCell& cell);	// Test drop at cell
	virtual bool		DropDataIntoCell(Atom theFlavor,
											unsigned char* drag_data,
											unsigned long data_size,
											const STableCell& cell);						// Drop data into cell
	virtual bool		DropDataAtCell(Atom theFlavor,
											unsigned char* drag_data,
											unsigned long data_size,
											const STableCell& cell);						// Drop data at cell
};

#endif
