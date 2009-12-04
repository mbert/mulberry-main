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


// Header for CTextTable class

#ifndef __CTEXTTABLE__MULBERRY__
#define __CTEXTTABLE__MULBERRY__

#include "CTableDrag.h"
#include "cdstring.h"

// Consts
const	ResIDT		Txtr_DefaultList = 1000;

// Classes

class LArray;

class CTextTable : public CTableDrag,
						public LBroadcaster
{
public:
	enum { class_ID = 'TxTb' };

	enum
	{
		eBroadcast_Drag = 'drag'
	};

	struct SBroadcastDrag
	{
		CTextTable* mTable;
		ulvector mDragged;
		unsigned long mDropped;
	};

					CTextTable();
					CTextTable(LStream *inStream);
	virtual 		~CTextTable();

protected:
	virtual void		FinishCreateSelf(void);							// Get details of sub-panes

public:
	void				SetDoubleClickMsg(MessageT inMessage)
		{ mDoubleClickMsg = inMessage; }
	MessageT			GetDoubleClickMsg() const
		{ return mDoubleClickMsg; }
	
	void				SetSelectionMsg(MessageT inMessage)
		{ mSelectionMsg = inMessage; }
	MessageT			GetSelectionMsg() const
		{ return mSelectionMsg; }
	
	void				SetDeleteMsg(MessageT inMessage)
		{ mDeleteMsg = inMessage; }
	MessageT			GetDeleteMsg() const
		{ return mDeleteMsg; }
	
	virtual void		EnableDragAndDrop();

	virtual void		SelectionChanged();
	virtual void		SetSingleSelection(void);
	virtual void		SetNoSelection(void);
	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

	virtual void		AddItem(const cdstring& item);
	virtual void		SetContents(const cdstrvect& items);
	virtual void		GetSelection(ulvector& selection) const;
	virtual void		GetSelection(cdstrvect& selection) const;

protected:
	TableIndexT			mStringColumn;

	virtual void		ClickSelf(const SMouseDownEvent &inMouseDown);

	virtual void		ClickCell(const STableCell& inCell,
								const SMouseDownEvent &inMouseDown);	// Click in the cell
	virtual void		DrawCell(const STableCell& inCell,
									const Rect &inLocalRect);		// Draw the address

			void		GetCellString(const STableCell& inCell, cdstring& str) const;

private:
	MessageT		mDoubleClickMsg;
	MessageT		mSelectionMsg;
	MessageT		mDeleteMsg;

			void		InitTextTable(void);

// Drag methods
private:
	virtual void	AddCellToDrag(CDragIt* theDragTask,
									const STableCell& theCell,
									Rect& dragRect);				// Add cell to drag with rect

protected:
	SBroadcastDrag	mDragged;

	virtual void	DoDragReceive(DragReference	inDragRef);			// Get multiple text items
	virtual void	DropDataAtCell(FlavorType theFlavor,
										char* drag_data,
										Size data_size,
										const STableCell& beforeCell);					// Drop data at cell

};

#endif
