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


// Header for CTableDragAndDrop class

#ifndef __CTABLEDRAGANDDROP__MULBERRY__
#define __CTABLEDRAGANDDROP__MULBERRY__

#include "templs.h"

#include <time.h>

#include <TTimerTask.h>

// Consts

const	SInt16		cTableDragAutoScroll = 4;

const	ResIDT		crsr_DDCopy = 133;

// Classes
class CTableDrag;
class CHierarchyTableDrag;
class CDragIt;

class CTableDragAndDropSupport : public LDragAndDrop {

protected:
	LTableView*			mTablePane;					// Pane containing table
	static LTableView*	sTableSource;				// Origin of drag
	static LTableView*	sTableTarget;				// Target of drag
	bool				mCopyCursor;				// Is copy cursor showing
	STableCell			mLastHitCell;				// Last cell hit by mouse
	STableCell			mLastDropCell;				// Last cell
	STableCell			mLastDropCursor;			// Last row for cursor
	Rect				mHiliteInset;				// Hilite inset
	RGBColor			mHiliteBackground;			// Color of background for hilite
	bool				mReadOnly;					// Table not editable
	bool				mDropCell;					// Drop into individual cells
	bool				mDropCursor;				// Drop at cell
	bool				mAllowDrag;					// Allow drag from self
	bool				mSelfDrag;					// Allow drag to self
	bool				mExternalDrag;				// Allow drag from outside source
	bool				mAllowMove;					// Allow drag to delete originals
	LArray				mDragFlavors;				// List of acceptable flavors
	LArray				mDropFlavors;				// List of acceptable flavors

#if PP_Target_Carbon
	TTimerTask<CTableDragAndDropSupport>	mTimerTask;
	DragReference							mCurrentDrag;
#endif

public:
					CTableDragAndDropSupport(WindowPtr inMacWindow,
									LPane *inPane);
	virtual 		~CTableDragAndDropSupport();

	virtual	void	SetTable(LTableView* theTablePane,
								bool canScroll = 0)
						{ mTablePane = theTablePane;
						  mDropCell = canScroll; }					// Set Table details
	virtual void	SetHiliteInset(Rect &hiliteInset)
						{ mHiliteInset = hiliteInset; }				// Set hilite inset
	virtual void	SetHiliteBackground(RGBColor &hiliteBackground)
						{ mHiliteBackground = hiliteBackground; }	// Set hilite background color

	virtual void	SetDDReadOnly(bool readOnly)
						{ mReadOnly = readOnly; }					// Set read only
	virtual void	SetDropCell(bool dropCell)					// Set drop into cell
						{ mDropCell = dropCell; }
	virtual void	SetDropCursor(bool dropCursor)				// Set drop at cell
						{ mDropCursor = dropCursor; }
	virtual void	SetAllowDrag(bool allowDrag)					// Set allow drag
						{ mAllowDrag = allowDrag; }
	virtual void	SetSelfDrag(bool selfDrag)					// Set self drag
						{ mSelfDrag = selfDrag; }
	virtual void	SetExternalDrag(bool externalDrag)					// Set self drag
						{ mExternalDrag = externalDrag; }
	virtual void	SetAllowMove(bool allowMove)					// Set allow move
						{ mAllowMove = allowMove; }
	virtual void	AddDragFlavor(FlavorType theFlavor,
									SInt32 pos = LArray::index_Last)	// Set its drag flavor
						{ mDragFlavors.InsertItemsAt(1, pos, &theFlavor); }
	virtual void	AddDropFlavor(FlavorType theFlavor,
									SInt32 pos = LArray::index_Last)	// Set its drop flavor
						{ mDropFlavors.InsertItemsAt(1, pos, &theFlavor); }
	
	virtual STableCell& GetLastDragCell(void)
						{ return mLastDropCell; }					// Get last drag cell
	virtual STableCell&	GetLastDragCursor(void)
						{ return mLastDropCursor; }					// Get last drag cursor

#if PP_Target_Carbon
			void	TimerCallback(LTimerTask *inTask);
#endif

// Drag methods
protected:
	virtual OSErr	CreateDragEvent(const SMouseDownEvent& inMouseDown,
									short* mouseUpModifiers = nil);			// Create the drag event
	virtual void	AddDragCells(CDragIt* theDragTask);						// Add cells to drag
	virtual void	AddCellToDrag(CDragIt* theDragTask,
									const STableCell& theCell,
									Rect& dragRect);						// Add cell to drag
	virtual void	DeleteSelection(void);									// Delete selected after move drag

	virtual Boolean	ItemIsAcceptable(DragReference inDragRef,
								ItemReference inItemRef);				// Check its suitable
	virtual bool	IsDropCell(DragReference inDragRef, STableCell row);				// Can cell be dropped into
	virtual bool	IsDropAtCell(DragReference inDragRef, STableCell& row);			// Can cell be dropped at (modify if not)
	virtual bool 	CheckIfViewIsAlsoSender(DragReference inDragRef);	// Check to see whether drop from another window
	virtual void	ReceiveDragItem(DragReference inDragRef,
									DragAttributes inDragAttrs,
									ItemReference inItemRef,
									Rect &inItemBounds);			// Receive the data

	virtual void	EnterDropArea(DragReference inDragRef,
									Boolean inDragHasLeftSender);	// Reset drop cursor
	virtual void	HiliteDropArea(DragReference inDragRef);		// Hilite region border
	virtual void	UnhiliteDropArea(DragReference inDragRef);		// Unhilite region border
	virtual void	LeaveDropArea(DragReference inDragRef);			// Do correct unhiliting of drop region
	virtual void	InsideDropArea(DragReference inDragRef);		// Handle drag into cells
	virtual void	AdjustCursorDropArea(DragReference inDragRef);	// Adjust cursor over drop area
	virtual bool	IsCopyCursor(DragReference inDragRef);			// Use copy cursor?

	virtual void	ScrollDropArea(DragReference inDragRef,
									short hScroll, short vScroll);	// Scroll the drop area
	virtual void	DrawDropCell(DragReference	inDragRef,
									STableCell& theCell);			// Draw drag cell frame
	virtual void	DrawDropAtCell(DragReference	inDragRef,
									STableCell& theCell);			// Draw drag cursor

	virtual void	DoDragSendData(FlavorType inFlavor,
									ItemReference inItemRef,
									DragReference inDragRef);		// Other flavor requested by receiver

	virtual void	DropData(FlavorType theFlavor,
										char* drag_data,
										Size data_size);			// Drop data into whole table
	virtual void	DropDataIntoCell(FlavorType theFlavor,
										char* drag_data,
										Size data_size,
										const STableCell& theCell);	// Drop data into cell
	virtual void	DropDataAtCell(FlavorType theFlavor,
										char* drag_data,
										Size data_size,
										const STableCell& beforeCell);	// Drop data before cell
									
	virtual void	GetDragItemData(FlavorType theFlavor,
									ItemReference inItemRef,
									Ptr *drag_data,
									Size *drag_size);				// Fetch data for drag
};

class CTableDragAndDrop : public CTableDragAndDropSupport {

private:
	CTableDrag*			mTableDragPane;				// Pane containing table

public:
					CTableDragAndDrop(WindowPtr inMacWindow,
									LPane *inPane);
	virtual 		~CTableDragAndDrop();

	virtual	void	SetTable(CTableDrag* theTableDragPane,
								bool canScroll = 0)
						{ CTableDragAndDropSupport::SetTable((LTableView*) theTableDragPane, canScroll); 
						  mTableDragPane = theTableDragPane; }					// Set Table details

// Drag methods
protected:
	virtual void	AddDragCells(CDragIt* theDragTask);					// Add cells to drag

	virtual void	ScrollDropArea(DragReference inDragRef,
									short hScroll, short vScroll);	// Scroll the drop area

	virtual void	DrawDropCell(DragReference	inDragRef,
									STableCell& theCell);			// Draw drag cell frame
	virtual void	DrawDropAtCell(DragReference inDragRef,
									STableCell& theCell);			// Draw drag cursor

};

class CHierarchyTableDragAndDrop : public CTableDragAndDropSupport {

private:
	CHierarchyTableDrag*	mHierarchyTableDragPane;				// Pane containing table
	ulvector				mExpandedRows;							// Array of rows expanded during current drag
	long					mTimeInCell;							// Time in current cell

public:
					CHierarchyTableDragAndDrop(WindowPtr inMacWindow,
									LPane *inPane);
	virtual 		~CHierarchyTableDragAndDrop();

	virtual	void	SetTable(CHierarchyTableDrag* theHierarchyTablePane,
								bool canScroll = 0)
						{ CTableDragAndDropSupport::SetTable((LTableView*) theHierarchyTablePane, canScroll); 
						  mHierarchyTableDragPane = theHierarchyTablePane; }					// Set Table details

// Drag methods
protected:
	virtual void	EnterDropArea(DragReference inDragRef,
									Boolean inDragHasLeftSender);	// Reset drop cursor
	virtual void	LeaveDropArea(DragReference inDragRef);			// Do correct unhiliting of drop region
	virtual void	InsideDropArea(DragReference inDragRef);		// Handle drag into cells

	virtual void	AddDragCells(CDragIt* theDragTask);				// Add cells to drag

	virtual bool	CanDropExpand(DragReference inDragRef,
									TableIndexT woRow);				// Can row expand for drop
	virtual void	DoDropExpand(DragReference inDragRef,
									TableIndexT woRow);				// Do row expand for drop

	virtual void	ScrollDropArea(DragReference inDragRef,
									short hScroll, short vScroll);	// Scroll the drop area

	virtual void	DrawDropCell(DragReference	inDragRef,
									STableCell& theCell);			// Draw drag cell frame
	virtual void	DrawDropAtCell(DragReference inDragRef,
									STableCell& theCell);			// Draw drag cursor

};

#endif
