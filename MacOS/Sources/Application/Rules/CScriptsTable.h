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

// Consts

// Panes

// Resources

// Messages

// Classes
class CTitleTable;
class LBevelButton;

class CScriptsTable : public CHierarchyTableDrag,
						public LListener
{
public:
	enum { class_ID = 'ScTb' };

					CScriptsTable();
					CScriptsTable(LStream *inStream);
	virtual 		~CScriptsTable();

private:
	CTitleTable*			mTitles;
	LBevelButton*			mNewBtn;
	LBevelButton*			mEditBtn;
	LBevelButton*			mDeleteBtn;
	LBevelButton*			mGenerateBtn;
	CFilterItem::EType		mType;

	virtual void		InitScriptsTable();

protected:
	virtual void		FinishCreateSelf();								// Get details of sub-panes

public:
			void		SetFilterType(CFilterItem::EType type);

	virtual void		ListenToMessage(MessageT inMessage,
										void *ioParam);				// Respond to clicks in the icon buttons
	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);

protected:
	virtual void		ClickCell(const STableCell& inCell,
								const SMouseDownEvent &inMouseDown);		// Click in the cell
	virtual void		ClickSelf(const SMouseDownEvent &inMouseDown);		// Make it target first
	virtual void		DrawCell(const STableCell& inCell,
									const Rect &inLocalRect);				// Draw the message info

	virtual void		BeTarget();
	virtual void		DontBeTarget();

public:
	virtual void		AdaptToNewSurroundings();						// Adjust column widths

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
private:
	virtual bool		IsCopyCursor(DragReference inDragRef);				// Use copy cursor?
	virtual void		AddCellToDrag(CDragIt* theDragTask,
										const STableCell& aCell,
										Rect& dragRect);					// Add group to drag
	bool				TestSelectionScript(TableIndexT row);				// Test for selected item script
	bool				TestSelectionRule(TableIndexT row);					// Test for selected item rule
	bool				TestCellSameScript(TableIndexT row,
												TableIndexT* parent);		// Test for selected item group
protected:
	virtual bool		IsDropCell(DragReference inDragRef, STableCell row);// Can cell be dropped into
	virtual bool		IsDropAtCell(DragReference inDragRef, STableCell& cell);// Can cell be dropped at

	virtual void		DropDataIntoCell(FlavorType theFlavor,
										char* drag_data,
										Size data_size,
										const STableCell& theCell);			// Drop data into cell
	virtual void		DropDataAtCell(FlavorType theFlavor,
										char* drag_data,
										Size data_size,
										const STableCell& beforeCell);	// Drop data at cell

};

#endif
