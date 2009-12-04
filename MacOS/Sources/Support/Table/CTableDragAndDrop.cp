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


// Source for CTableDragAndDrop class

#include "CTableDragAndDrop.h"

#include "CDragIt.h"
#include "CHierarchyTableDrag.h"
#include "CMulberryCommon.h"
#include "CTableDrag.h"

#include <UGAColorRamp.h>

#include <sys/time.h>

// __________________________________________________________________________________________________
// C L A S S __ C T A B L E D R A G A N D D R O P S U P P O R T
// __________________________________________________________________________________________________

LTableView* CTableDragAndDropSupport::sTableSource = NULL;
LTableView* CTableDragAndDropSupport::sTableTarget = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTableDragAndDropSupport::CTableDragAndDropSupport(
	WindowPtr	inMacWindow,
	LPane		*inPane)
	: LDragAndDrop(inMacWindow, inPane),
	  mDragFlavors(sizeof(FlavorType)),
	  mDropFlavors(sizeof(FlavorType))
#if PP_Target_Carbon
	  , mTimerTask(this, &CTableDragAndDropSupport::TimerCallback)
#endif
{
	mTablePane = NULL;
	::SetRect(&mHiliteInset, 1, 1, 1, 1);
	mHiliteBackground = UGAColorRamp::GetWhiteColor();
	mLastHitCell.row = 0;
	mLastHitCell.col = 0;
	mLastDropCell.row = 0;
	mLastDropCell.col = 0;
	mLastDropCursor.row = 0;
	mLastDropCursor.col = 0;
	mReadOnly = false;
	mDropCell = false;
	mDropCursor = false;
	mAllowDrag = true;
	mSelfDrag = false;
	mExternalDrag = true;
	mAllowMove = true;
#if PP_Target_Carbon
	mCurrentDrag = NULL;
#endif
}

// Default destructor
CTableDragAndDropSupport::~CTableDragAndDropSupport()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Create the drag
OSErr CTableDragAndDropSupport::CreateDragEvent(const SMouseDownEvent& inMouseDown, short* mouseUpModifiers)
{

	// Begin the drag task
	CDragIt theDragTask(inMouseDown.macEvent);

	// Add send data proc
	DragSendDataUPP dragSendDataPP = NewDragSendDataUPP(LDragAndDrop::HandleDragSendData);
	SetDragSendProc(theDragTask.GetDragReference(), dragSendDataPP, (LDropArea*) this);

	// Add selected cell to drag
	AddDragCells(&theDragTask);

	// Zero drag cell
	mLastHitCell.row = 0;
	mLastHitCell.col = 0;
	mLastDropCell.row = 0;
	mLastDropCell.col = 0;
	mLastDropCursor.row = 0;
	mLastDropCursor.col = 0;

	// Do the drag
	sTableSource = mTablePane;
	sTableTarget = NULL;
	OSErr err = theDragTask.DoDragIt();
	::GetDragModifiers(theDragTask.GetDragReference(), NULL, NULL, mouseUpModifiers);

	// Look for delete of original
	if (sTableSource == sTableTarget)
		DeleteSelection();

	sTableSource = NULL;

	// Dispose of send data
	DisposeDragSendDataUPP(dragSendDataPP);

	return err;
}

// Add cells to drag
void CTableDragAndDropSupport::AddDragCells(CDragIt* theDragTask)
{
	// Add all selected messages to drag
	mTablePane->FocusDraw();
	STableCell aCell(0, 1);

	while(mTablePane->GetNextSelectedCell(aCell)) {

		// Get rect for cell
		Rect dragRect;
		mTablePane->GetLocalCellRect(aCell, dragRect);

		// Add this cell to drag with a rect
		AddCellToDrag(theDragTask, aCell, dragRect);
	}
}

// Add cells to drag
void CTableDragAndDropSupport::AddCellToDrag(CDragIt* theDragTask, const STableCell& theCell, Rect& dragRect)
{
}

// Delete selected after move drag
void CTableDragAndDropSupport::DeleteSelection(void)
{
}

// Check whether drag item is acceptable
Boolean CTableDragAndDropSupport::ItemIsAcceptable(
	DragReference	inDragRef,
	ItemReference	inItemRef)
{
	// Cannot accept if read only
	if (mReadOnly)
		return false;

	// Cannot accept if not drop self
	if ((sTableSource == mTablePane) && !mSelfDrag)
		return false;

	// Cannot accept if external
	if ((sTableSource != mTablePane) && !mExternalDrag)
		return false;

	// Check for mDropFlavors
	FlavorFlags		theFlags;
	LArrayIterator	iterator(mDropFlavors, LArrayIterator::from_Start);
	FlavorType		theFlavor;

	while (iterator.Next(&theFlavor)) {
		if (::GetFlavorFlags(inDragRef, inItemRef, theFlavor, &theFlags) == noErr)
			return true;
	}

	return false;

} // CTableDragAndDropSupport::ItemIsAcceptable

// Test drop into cell
bool CTableDragAndDropSupport::IsDropCell(DragReference	inDragRef, STableCell row)
{
	return true;
}

// Test drop at cell
bool CTableDragAndDropSupport::IsDropAtCell(DragReference	inDragRef, STableCell& row)
{
	return true;
}

// Check to see whether drop from another window
bool CTableDragAndDropSupport::CheckIfViewIsAlsoSender(DragReference inDragRef)
{
	// Get drag attributes
	DragAttributes theDragAttributes;
	::GetDragAttributes(inDragRef, &theDragAttributes);

	// Compare with relevant flag and check for same table
	return ((theDragAttributes & kDragInsideSenderWindow) &&
			(mTablePane == sTableSource));

} // CTableDragAndDropSupport::CheckIfViewIsAlsoSender

// Receive the message
void CTableDragAndDropSupport::ReceiveDragItem(
	DragReference	inDragRef,
	DragAttributes	inDragAttrs,
	ItemReference	inItemRef,
	Rect			&inItemBounds)
{
	// Information about the drag contents we'll be needing.
	FlavorFlags		theFlags;		// We actually only use the flags to see if a flavor exists
	Size			theDataSize;	// How much data there is for us.
	char*			theFlavorData;	// Where we will put that data.

	// Find suitable flavor
	LArrayIterator	iterator(mDropFlavors, LArrayIterator::from_Start);
	FlavorType		theFlavor;
	bool			gotFlavor = false;

	while (iterator.Next(&theFlavor)) {
		if (::GetFlavorFlags(inDragRef, inItemRef, theFlavor, &theFlags) == noErr) {
			gotFlavor = true;
			break;
		}
	}

	// Check to make sure the drag contains a mDropFlavors item.
	if (gotFlavor) {

		// Get size of data
		::GetFlavorDataSize(inDragRef, inItemRef, theFlavor, &theDataSize);
		if (theDataSize)
		{
			// Create space for text
			theFlavorData = new char[theDataSize];

			// Get the data about the message we are receiving.
			::GetFlavorData(inDragRef, inItemRef, theFlavor, theFlavorData, &theDataSize, 0L);

			// Check for drop into cell
			if (mDropCell && mTablePane->IsValidCell(mLastDropCell))
			{
				// Do not do if going back inside own selection
				if (CheckIfViewIsAlsoSender(inDragRef) && mTablePane->CellIsSelected(mLastDropCell))
					// Check drop not in current selection or table
					Throw_(dragNotAcceptedErr);

				DropDataIntoCell(theFlavor, theFlavorData, theDataSize, mLastDropCell);
			}
			else if (mDropCursor && (mLastDropCursor.row != 0) && (mLastDropCursor.col != 0))
				DropDataAtCell(theFlavor, theFlavorData, theDataSize, mLastDropCursor);
			else
				DropData(theFlavor, theFlavorData, theDataSize);

			delete theFlavorData;
		}
	}

	// Mark this table as target
	sTableTarget = mTablePane;

} // CTableDragAndDropSupport::ReceiveDragItem

// Do correct unhiliting of drop region
void CTableDragAndDropSupport::EnterDropArea(
	DragReference	inDragRef,
	Boolean			inDragHasLeftSender)
{
	// Reset cursor
	mLastHitCell.row = 0;
	mLastHitCell.col = 0;
	mLastDropCell.row = 0;
	mLastDropCell.col = 0;
	mLastDropCursor.row = 0;
	mLastDropCursor.col = 0;

	// Do inherited
	LDragAndDrop::EnterDropArea(inDragRef, inDragHasLeftSender);

	// Set appropriate cursor
	mCopyCursor = false;

#if PP_Target_Carbon
	// Need timer on OS X for stationery mouse
	mCurrentDrag = inDragRef;
	mTimerTask.Install(::GetCurrentEventLoop(), 0.1, 0.1);
#endif
}

// Hilite/unhilite region two pixels wide round border of whole pane
void CTableDragAndDropSupport::HiliteDropArea(
	DragReference	inDragRef)
{
	StColorState	saveColor;
	Rect			dropRect;

	// Get frame
	FocusDropArea();
	mPane->CalcLocalFrameRect(dropRect);

	// Inset as specified
	dropRect.left += mHiliteInset.left;
	dropRect.top += mHiliteInset.top;
	dropRect.right -= mHiliteInset.right;
	dropRect.bottom -= mHiliteInset.bottom;

	// Create drag region
	RgnHandle	dropRgn = ::NewRgn();
	::RectRgn(dropRgn, &dropRect);

	// Set correct background and do hilite
	::RGBBackColor(&mHiliteBackground);
	::ShowDragHilite(inDragRef, dropRgn, true);

	// Dispose drag region
	::DisposeRgn(dropRgn);

} // CTableDragAndDropSupport::HiliteDropArea

// Unhilite region two pixels wide round border of whole pane
void CTableDragAndDropSupport::UnhiliteDropArea(
	DragReference	inDragRef)
{
	StColorState	saveColor;

	FocusDropArea();

	// Set correct background and do hilite
	::RGBBackColor(&mHiliteBackground);
	::HideDragHilite(inDragRef);

} // CTableDragAndDropSupport::UnhiliteDropArea

// Do correct unhiliting of drop region
void CTableDragAndDropSupport::LeaveDropArea(
	DragReference	inDragRef)
{

#if PP_Target_Carbon
	// Need timer on OS X for stationery mouse
	mCurrentDrag = NULL;
	mTimerTask.Remove();
#endif

	// Remove cursor and reset
	if (mDropCell)
	{
		STableCell invalid_cell(0, 0);
		DrawDropCell(inDragRef, invalid_cell);
	}
	if (mDropCursor)
	{
		STableCell invalid_cell(0, 0);
		DrawDropAtCell(inDragRef, invalid_cell);
	}

	// Remove frame highlight
	if (mIsHilited)
	{
		UnhiliteDropArea(inDragRef);
		mIsHilited = false;
	}

	mCanAcceptCurrentDrag = false;

	// Always reset cursor to arrow
	UCursor::SetArrow();

} // CTableDragAndDropSupport::LeaveDropArea

// Handle drag into cells
void CTableDragAndDropSupport::InsideDropArea(DragReference	inDragRef)
{
#if PP_Target_Carbon
	// Prevent timer re-entrancy
	StValueChanger<DragReference> _temp(mCurrentDrag, NULL);
#endif

	// Get global mouse pos from Drag manager
	Point	mouseLoc;
	::GetDragMouse(inDragRef, NULL, &mouseLoc);

	// Convert via a tortous route to an image point
	mTablePane->GlobalToPortPoint(mouseLoc);
	mTablePane->PortToLocalPoint(mouseLoc);
	SPoint32	imagePt;
	mTablePane->LocalToImagePoint(mouseLoc, imagePt);

	TableIndexT rows, cols;
	mTablePane->GetTableSize(rows, cols);

	// Do cell highlight if drop into cell
	bool hilite = false;
	if (mDropCell && rows)
	{
		// Get the hit cell
		STableCell	hitCell;
		mTablePane->GetCellHitBy(imagePt, hitCell);
		mLastHitCell = hitCell;

		// Must be able to drop
		if (!IsDropCell(inDragRef, hitCell))
			hitCell = STableCell(0, 0);
		else if (mDropCursor)
		{
			// Check if close to edge of cell and drop cursor allowed
			Rect cellRect;
			mTablePane->GetLocalCellRect(hitCell, cellRect);

			if ((mouseLoc.v < cellRect.top + 2) || (mouseLoc.v > cellRect.bottom - 2))
				hitCell = STableCell(0, 0);
			else
				hilite = true;
		}
		else
			hilite = true;

		// Draw new cursor
		DrawDropCell(inDragRef, hitCell);
	}
	if (mDropCursor && rows)
	{
		// Get the hit cell
		STableCell	hitCell;
		mTablePane->GetCellHitBy(imagePt, hitCell);

		Rect cellRect;
		mTablePane->GetLocalCellRect(hitCell, cellRect);

		SPoint32 offset = imagePt;
		offset.v += (cellRect.bottom - cellRect.top)/2;

		// Get the adjusted hit cell
		mTablePane->GetCellHitBy(offset, hitCell);

		if (!hilite)
		{
			if (!mTablePane->IsValidCell(hitCell))
				hitCell = STableCell(rows + 1, 1);
			if (!IsDropAtCell(inDragRef, hitCell))
				hitCell = STableCell(0, 0);
		}
		else
			hitCell = STableCell(0, 0);

		DrawDropAtCell(inDragRef, hitCell);
	}

	// Check for auto scroll
	SInt16	hScroll = 0;
	SInt16	vScroll = 0;
	Rect frame;

	mTablePane->CalcLocalFrameRect(frame);
	if (mouseLoc.v < frame.top + cTableDragAutoScroll)
		vScroll = -mTablePane->GetRowHeight(1);
	else if (mouseLoc.v > frame.bottom - cTableDragAutoScroll)
		vScroll = mTablePane->GetRowHeight(1);
	else if (mouseLoc.h < frame.left + cTableDragAutoScroll)
		hScroll = -16;
	else if (mouseLoc.h > frame.right - cTableDragAutoScroll)
		hScroll = 16;

	ScrollDropArea(inDragRef, hScroll, vScroll);

	// Set appropriate cursor
	AdjustCursorDropArea(inDragRef);

} // CTableDragAndDropSupport::InsideDropArea

void CTableDragAndDropSupport::ScrollDropArea(DragReference	inDragRef, short hScroll, short vScroll)
{
	if (hScroll || vScroll)
	{
		// Remove current selection
		STableCell aLastDropCell = mLastDropCell;
		STableCell aLastDropCursor = mLastDropCursor;
		if (mDropCell)
		{
			STableCell invalid_cell(0, 0);
			DrawDropCell(inDragRef, invalid_cell);
		}
		if (mDropCursor)
		{
			STableCell invalid_cell(0, 0);
			DrawDropAtCell(inDragRef, invalid_cell);
		}

		if (mIsHilited)
			UnhiliteDropArea(inDragRef);
		mTablePane->ScrollPinnedImageBy(hScroll, vScroll, true);
		if (mIsHilited)
			HiliteDropArea(inDragRef);

		// Restore selection
		if (mDropCell)
			DrawDropCell(inDragRef, aLastDropCell);
		if (mDropCursor)
			DrawDropAtCell(inDragRef, aLastDropCursor);
	}
}

// Adjust cursor over drop area
void CTableDragAndDropSupport::AdjustCursorDropArea(DragReference inDragRef)
{
	// No special cursor processing
	if (IsCopyCursor(inDragRef) && !mCopyCursor)
	{
		UCursor::SetThemeCursor(kThemeCopyArrowCursor);
		mCopyCursor = true;
	}
	else if (!IsCopyCursor(inDragRef) && mCopyCursor)
	{
		UCursor::SetArrow();
		mCopyCursor = false;
	}
}

// Adjust cursor over drop area
bool CTableDragAndDropSupport::IsCopyCursor(DragReference inDragRef)
{
	return false;
}

// Draw drag insert cell
void CTableDragAndDropSupport::DrawDropCell(DragReference	inDragRef,
										STableCell& theCell)
{
	// Only do if both cells are valid
	if (mLastDropCell.row == theCell.row)
		return;

	// Remove highlight frame
	if (mIsHilited)
		UnhiliteDropArea(inDragRef);

	// First remove current drag hilite
	if (mTablePane->IsValidCell(mLastDropCell))
	{
		mTablePane->FocusDraw();
		Rect	cellRect;
		mTablePane->GetLocalCellRect(mLastDropCell, cellRect);
		//mTablePane->DrawCell(mLastDragCell, cellRect);
	}

	// Then draw new drag hilite
	if (mTablePane->IsValidCell(theCell))
	{
		mTablePane->FocusDraw();

		Rect frame;
		mTablePane->GetLocalCellRect(theCell, frame);
		StColorPenState save_state;
		StColorPenState::Normalize();
		::PenSize(2,2);
		StColorPenState::SetGrayPattern();
		::PenMode(patOr);
		::FrameRect(&frame);
	}

	// Now add hilite again
	if (mIsHilited)
		HiliteDropArea(inDragRef);

	// Reset current value
	mLastDropCell = theCell;
}

// Draw drag insert cursor
void CTableDragAndDropSupport::DrawDropAtCell(DragReference	inDragRef,
										STableCell& theCell)
{
	// Only do if both cells are valid
	if (mLastDropCursor.row == theCell.row)
		return;

	// Remove highlight frame
	if (mIsHilited)
		UnhiliteDropArea(inDragRef);

	// Get current table size
	TableIndexT rows, cols;
	mTablePane->GetTableSize(rows, cols);

	// First remove current drag hilite
	if (mTablePane->IsValidCell(mLastDropCursor))
	{
		mTablePane->FocusDraw();
		Rect	cellRect;
		mTablePane->GetLocalCellRect(mLastDropCursor, cellRect);
		//mTablePane->DrawCell(mLastDragCell, cellRect);
	}

	// Then draw new drag hilite
	if (mTablePane->IsValidCell(theCell))
	{
		mTablePane->FocusDraw();

		Rect frame;
		if (theCell.row <= rows)
		{
			mTablePane->GetLocalCellRect(theCell, frame);
			frame.bottom = frame.top + 1;
		}
		else
		{
			STableCell temp = theCell;
			temp.row--;
			mTablePane->GetLocalCellRect(temp, frame);
			frame.top = frame.bottom - 1;
		}
		StColorPenState save_state;
		StColorPenState::Normalize();
		::PenSize(2,2);
		StColorPenState::SetGrayPattern();
		::PenMode(patOr);
		::FrameRect(&frame);
	}

	// Now add hilite again
	if (mIsHilited)
		HiliteDropArea(inDragRef);

	// Reset current value
	mLastDropCursor = theCell;
}

// Other flavor requested by receiver
void CTableDragAndDropSupport::DoDragSendData(
	FlavorType		inFlavor,
	ItemReference	inItemRef,
	DragReference	inDragRef)
{
	if (mDragFlavors.FetchIndexOf(&inFlavor) != LArray::index_Bad) {
		// Give drag this data
		Ptr drag_data;
		Size drag_size;
		GetDragItemData(inFlavor, inItemRef, &drag_data, &drag_size);
		ThrowIfOSErr_(::SetDragItemFlavorData(inDragRef, inItemRef, inFlavor,
										drag_data, drag_size, 0L));
	}
	else
		Throw_(badDragFlavorErr);
}

// Drop data into whole table
void CTableDragAndDropSupport::DropData(FlavorType theFlavor, char* drag_data, Size data_size)
{
	// Do not accept - sub-classes will handle
	Throw_(dragNotAcceptedErr);
}

// Drop data into cell
void CTableDragAndDropSupport::DropDataIntoCell(FlavorType theFlavor, char* drag_data,
											Size data_size, const STableCell& theCell)
{
	// Do not accept - sub-classes will handle
	Throw_(dragNotAcceptedErr);
}

// Drop data at cell
void CTableDragAndDropSupport::DropDataAtCell(FlavorType theFlavor, char* drag_data,
											Size data_size, const STableCell& theCell)
{
	// Do not accept - sub-classes will handle
	Throw_(dragNotAcceptedErr);
}

// Fetch data for drag
void CTableDragAndDropSupport::GetDragItemData(FlavorType theFlavor,
										ItemReference inItemRef,
										Ptr *drag_data,
										Size *drag_size)
{
}

#if PP_Target_Carbon
// Trigger inside notifications
void CTableDragAndDropSupport::TimerCallback(LTimerTask *inTask)
{
	if (mCurrentDrag)
		InsideDropArea(mCurrentDrag);
}
#endif

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C T A B L E D R A G A N D D R O P S U P P O R T
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTableDragAndDrop::CTableDragAndDrop(
	WindowPtr	inMacWindow,
	LPane		*inPane)
	: CTableDragAndDropSupport(inMacWindow, inPane)
{
}

// Default destructor
CTableDragAndDrop::~CTableDragAndDrop()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Add cells to drag
void CTableDragAndDrop::AddDragCells(CDragIt* theDragTask)
{
	// Add all selected messages to drag
	mTablePane->FocusDraw();
	STableCell aCell(0, 1);

	while(mTableDragPane->GetNextSelectedCell(aCell)) {

		// Do for rows only
		if (!mTableDragPane->GetRowSelect() && !mTableDragPane->GetColSelect())
		{
			// Get rect for cell
			Rect dragRect;
			mTableDragPane->GetLocalCellRect(aCell, dragRect);

			// Add this cell to drag with a rect
			AddCellToDrag(theDragTask, aCell, dragRect);
		}
		else if (mTableDragPane->GetRowSelect() && (aCell.col==1))
		{
			// Get rect for cell
			Rect dragRect;
			mTableDragPane->GetLocalRowRect(aCell.row, dragRect);

			// Add this cell to drag with a rect
			AddCellToDrag(theDragTask, aCell, dragRect);
		}
		else if (mTableDragPane->GetColSelect() && (aCell.row==1))
		{
			// Get rect for cell
			Rect dragRect;
			mTableDragPane->GetLocalColRect(aCell.col, dragRect);

			// Add this cell to drag with a rect
			AddCellToDrag(theDragTask, aCell, dragRect);
		}

	}
}

// Scroll the drop area
void CTableDragAndDrop::ScrollDropArea(DragReference inDragRef, short hScroll, short vScroll)
{
	if ((hScroll || vScroll) && mTableDragPane->CanScrollImageBy(hScroll, vScroll))
		// Do inherited
		CTableDragAndDropSupport::ScrollDropArea(inDragRef, hScroll, vScroll);

} // CTableDragAndDrop::ScrollDropArea

// Draw drag insert cell
void CTableDragAndDrop::DrawDropCell(DragReference	inDragRef,
										STableCell& theCell)
{
	// Only do if cells are different
	if (mLastDropCell.row == theCell.row)
		return;

	// Remove highlight frame
	if (mIsHilited)
		UnhiliteDropArea(inDragRef);

	// First remove current drag hilite
	if (mTableDragPane->IsValidCell(mLastDropCell))
	{
		mTableDragPane->FocusDraw();
		if (!mTableDragPane->GetRowSelect() && !mTableDragPane->GetColSelect())
		{
			Rect	cellRect;
			mTableDragPane->GetLocalCellRect(mLastDropCell, cellRect);
			mTableDragPane->DrawCell(mLastDropCell, cellRect);
		}
		else if (mTableDragPane->GetRowSelect())
			mTableDragPane->DrawRow(mLastDropCell.row);
		else if (mTableDragPane->GetColSelect())
			mTableDragPane->DrawColumn(mLastDropCell.col);

		// May need to redraw selection hilite
		if (mTableDragPane->CellIsSelected(mLastDropCell))
		{
			if (!mTableDragPane->GetRowSelect() && !mTableDragPane->GetColSelect())
				mTableDragPane->HiliteCell(mLastDropCell, true);
			else if (mTableDragPane->GetRowSelect())
				mTableDragPane->HiliteRow(mLastDropCell.row, true);
			else if (mTableDragPane->GetColSelect())
				mTableDragPane->HiliteColumn(mLastDropCell.col, true);
		}
	}

	// Then draw new drag hilite
	if (mTableDragPane->IsValidCell(theCell))
	{
		mTableDragPane->FocusDraw();

		Rect frame;
		if (!mTableDragPane->GetRowSelect() && !mTableDragPane->GetColSelect())
			mTableDragPane->GetLocalCellRect(theCell, frame);
		else if (mTableDragPane->GetRowSelect())
			mTableDragPane->GetLocalRowRect(theCell.row, frame);
		else if (mTableDragPane->GetColSelect())
			mTableDragPane->GetLocalColRect(theCell.col, frame);
		StColorPenState save_state;
		StColorPenState::Normalize();
		::PenSize(2,2);
		StColorPenState::SetGrayPattern();
		::PenMode(patOr);
		::FrameRect(&frame);
	}

	// Now add hilite again
	if (mIsHilited)
		HiliteDropArea(inDragRef);

	// Reset current value
	mLastDropCell = theCell;
}

// Draw drag insert cursor
void CTableDragAndDrop::DrawDropAtCell(DragReference	inDragRef,
										STableCell& theCell)
{
	// Only do if cells are different
	if (mLastDropCursor.row == theCell.row)
		return;

	// Remove highlight frame
	if (mIsHilited)
		UnhiliteDropArea(inDragRef);

	// Get current table size
	TableIndexT rows, cols;
	mTablePane->GetTableSize(rows, cols);

	// First remove current drag hilite
	if (mTableDragPane->IsValidCell(mLastDropCursor))
	{
		mTableDragPane->FocusDraw();
		if (!mTableDragPane->GetRowSelect() && !mTableDragPane->GetColSelect())
		{
			Rect	cellRect;
			mTableDragPane->GetLocalCellRect(mLastDropCursor, cellRect);
			mTableDragPane->DrawCell(mLastDropCursor, cellRect);
		}
		else if (mTableDragPane->GetRowSelect())
			mTableDragPane->DrawRow(mLastDropCursor.row);
		else if (mTableDragPane->GetColSelect())
			mTableDragPane->DrawColumn(mLastDropCursor.col);

		// May need to redraw selection hilite
		if (mTableDragPane->CellIsSelected(mLastDropCursor))
		{
			if (!mTableDragPane->GetRowSelect() && !mTableDragPane->GetColSelect())
				mTableDragPane->HiliteCell(mLastDropCursor, true);
			else if (mTableDragPane->GetRowSelect())
				mTableDragPane->HiliteRow(mLastDropCursor.row, true);
			else if (mTableDragPane->GetColSelect())
				mTableDragPane->HiliteColumn(mLastDropCursor.col, true);
		}
	}

	// Then draw new drag hilite
	if (mTableDragPane->IsValidCell(theCell))
	{
		mTableDragPane->FocusDraw();

		Rect frame;
		if (!mTableDragPane->GetRowSelect() && !mTableDragPane->GetColSelect())
			mTableDragPane->GetLocalCellRect(theCell, frame);
		else if (mTableDragPane->GetRowSelect())
		{
			if (theCell.row <= rows)
			{
				mTableDragPane->GetLocalRowRect(theCell.row, frame);
				frame.bottom = frame.top + 2;
			}
			else
			{
				mTableDragPane->GetLocalRowRect(theCell.row - 1, frame);
				frame.top = frame.bottom - 2;
			}
		}
		else if (mTableDragPane->GetColSelect())
		{
			if (theCell.row <= rows)
			{
				mTableDragPane->GetLocalColRect(theCell.col, frame);
				frame.right = frame.left + 2;
			}
			else
			{
				mTableDragPane->GetLocalColRect(theCell.col - 1, frame);
				frame.left = frame.right - 2;
			}
		}
		StColorPenState save_state;
		StColorPenState::Normalize();
		::PenSize(2,2);
		StColorPenState::SetGrayPattern();
		::PenMode(patOr);
		::FrameRect(&frame);
	}

	// Now add hilite again
	if (mIsHilited)
		HiliteDropArea(inDragRef);

	// Reset current value
	mLastDropCursor = theCell;
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C B O D Y T A B L E D R A G A N D D R O P
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CHierarchyTableDragAndDrop::CHierarchyTableDragAndDrop(
	WindowPtr	inMacWindow,
	LPane		*inPane)
	: CTableDragAndDropSupport(inMacWindow, inPane)
{
}

// Default destructor
CHierarchyTableDragAndDrop::~CHierarchyTableDragAndDrop()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do correct unhiliting of drop region
void CHierarchyTableDragAndDrop::EnterDropArea(
	DragReference	inDragRef,
	Boolean			inDragHasLeftSender)
{
	// Do inherited
	CTableDragAndDropSupport::EnterDropArea(inDragRef, inDragHasLeftSender);

	// Clear expansion array
	mExpandedRows.clear();
}

// Do correct unhiliting of drop region
void CHierarchyTableDragAndDrop::LeaveDropArea(
	DragReference	inDragRef)
{
	// Do inherited
	CTableDragAndDropSupport::LeaveDropArea(inDragRef);

	// Collapse all expanded rows in reverse
	for(ulvector::reverse_iterator riter = mExpandedRows.rbegin(); riter != mExpandedRows.rend(); riter++)
	{
		mHierarchyTableDragPane->CollapseRow(*riter);
		TableIndexT exposed = mHierarchyTableDragPane->GetExposedIndex(*riter);
		mHierarchyTableDragPane->RefreshRow(exposed);
	}

	// If anything collapsed, do redraw/update immediately
	if (mExpandedRows.size())
		mHierarchyTableDragPane->UpdatePort();

} // CTableDragAndDropSupport::LeaveDropArea

// Handle drag into cells
void CHierarchyTableDragAndDrop::InsideDropArea(DragReference inDragRef)
{
#if PP_Target_Carbon
	// Prevent timer re-entrancy
	StValueChanger<DragReference> _temp(mCurrentDrag, NULL);
#endif

	STableCell old_cell = mLastHitCell;

	// Do inherited
	CTableDragAndDropSupport::InsideDropArea(inDragRef);

	STableCell new_cell = mLastHitCell;

	// If not equal reset clock
	if (new_cell.row != old_cell.row)
	{
		struct timeval tv;
		::gettimeofday(&tv, NULL);
		mTimeInCell = (tv.tv_sec << 6) + (tv.tv_usec << 6) / 1000000;
	}
	else
	{
		struct timeval tv;
		::gettimeofday(&tv, NULL);
		long time_now = (tv.tv_sec << 6) + (tv.tv_usec << 6) / 1000000;

		// Check time
		if (time_now - mTimeInCell > (1L << 6) / 2)
		{
			TableIndexT woRow = mHierarchyTableDragPane->GetWideOpenIndex(old_cell.row);
			// Check for expandability
			if (CanDropExpand(inDragRef, woRow))
			{
				// Do expand
				DoDropExpand(inDragRef, woRow);

				// Cache value for later collapse
				mExpandedRows.push_back(woRow);
			}
		}
	}

} // CTableDragAndDropSupport::InsideDropArea

// Add cells to drag
void CHierarchyTableDragAndDrop::AddDragCells(CDragIt* theDragTask)
{
	// Add all selected messages to drag
	mHierarchyTableDragPane->FocusDraw();
	STableCell aCell(0, 1);

	while(mHierarchyTableDragPane->GetNextSelectedCell(aCell)) {

		// Do for rows only
		if (aCell.col==1)
		{
			// Get rect for row
			Rect dragRect;
			mHierarchyTableDragPane->GetLocalRowRect(aCell.row, dragRect);

			// Add this cell to drag with a rect
			AddCellToDrag(theDragTask, aCell, dragRect);
		}

	}
}

// Can cell expand for drop
bool CHierarchyTableDragAndDrop::CanDropExpand(DragReference inDragRef, unsigned long woRow)
{
	return mHierarchyTableDragPane->IsCollapsable(woRow) && !mHierarchyTableDragPane->IsExpanded(woRow);
}

// Do cell expand for drop
void CHierarchyTableDragAndDrop::DoDropExpand(DragReference inDragRef, unsigned long woRow)
{
	const unsigned long cFlashTickDelay = 5;

	bool flash_on = (woRow == mLastDropCell.row);

	// Hilite cell a couple of times to flash
	TableIndexT exposed = mHierarchyTableDragPane->GetExposedIndex(woRow);
	mHierarchyTableDragPane->HiliteRowActively(exposed, flash_on ? false : true);
	unsigned long ticker = ::TickCount();
	while(::TickCount() < ticker + cFlashTickDelay) {}
	mHierarchyTableDragPane->HiliteRowActively(exposed, flash_on ? true : false);
	ticker = ::TickCount();
	while(::TickCount() < ticker + cFlashTickDelay) {}
	mHierarchyTableDragPane->HiliteRowActively(exposed, flash_on ? false : true);
	ticker = ::TickCount();
	while(::TickCount() < ticker + cFlashTickDelay) {}
	mHierarchyTableDragPane->HiliteRowActively(exposed, flash_on ? true : false);

	// Force expansion and immediate redraw/update (must unhilite drag then rehilite)
	if (mIsHilited)
		UnhiliteDropArea(inDragRef);
	mHierarchyTableDragPane->ExpandRow(woRow);
	mHierarchyTableDragPane->RefreshRow(exposed);
	mHierarchyTableDragPane->UpdatePort();
	//mHierarchyTableDragPane->HiliteRowActively(exposed, flash_on ? true : false);
	if (mIsHilited)
		HiliteDropArea(inDragRef);
}

// Scroll the drop area
void CHierarchyTableDragAndDrop::ScrollDropArea(DragReference inDragRef, short hScroll, short vScroll)
{
	if ((hScroll || vScroll) && mHierarchyTableDragPane->CanScrollImageBy(hScroll, vScroll))
		// Do inherited
		CTableDragAndDropSupport::ScrollDropArea(inDragRef, hScroll, vScroll);

} // CTableDragAndDrop::ScrollDropArea

// Draw drag insert cell
void CHierarchyTableDragAndDrop::DrawDropCell(DragReference	inDragRef,
										STableCell& theCell)
{
	// Only do if cells are different
	if (mLastDropCell.row == theCell.row)
		return;

	// Remove highlight frame
	if (mIsHilited)
		UnhiliteDropArea(inDragRef);

	// First remove current drag hilite
	if (mLastDropCell.row > 0)
	{
		mHierarchyTableDragPane->FocusDraw();
		if (mHierarchyTableDragPane->IsActive())
		{
			if (!mHierarchyTableDragPane->CellIsSelected(mLastDropCell))
				mHierarchyTableDragPane->HiliteRowActively(mLastDropCell.row, false);
		}
		else
		{
			if (!mHierarchyTableDragPane->CellIsSelected(mLastDropCell))
				mHierarchyTableDragPane->HiliteRowActively(mLastDropCell.row, false);
			else
			{
				mHierarchyTableDragPane->HiliteRowActively(mLastDropCell.row, false);
				mHierarchyTableDragPane->HiliteRowInactively(mLastDropCell.row, true);
			}
		}
	}

	// Then draw new drag hilite
	if (theCell.row > 0)
	{
		mHierarchyTableDragPane->FocusDraw();

		if (mHierarchyTableDragPane->IsActive())
		{
			if (!mHierarchyTableDragPane->CellIsSelected(theCell))
				mHierarchyTableDragPane->HiliteRowActively(theCell.row, true);
		}
		else
		{
			if (!mHierarchyTableDragPane->CellIsSelected(theCell))
				mHierarchyTableDragPane->HiliteRowActively(theCell.row, true);
			else
			{
				mHierarchyTableDragPane->HiliteRowInactively(theCell.row, false);
				mHierarchyTableDragPane->HiliteRowActively(theCell.row, true);
			}
		}
	}

	// Now add hilite again
	if (mIsHilited)
		HiliteDropArea(inDragRef);

	// Reset current value
	mLastDropCell = theCell;
}

// Draw drag insert cursor
void CHierarchyTableDragAndDrop::DrawDropAtCell(DragReference inDragRef, STableCell& theCell)
{
	// Only do if cells are different
	if (mLastDropCursor.row == theCell.row)
		return;

	// Remove highlight frame
	if (mIsHilited)
		UnhiliteDropArea(inDragRef);

	// Get current table size
	TableIndexT rows, cols;
	mTablePane->GetTableSize(rows, cols);

	// First remove current drag hilite
	if (mLastDropCursor.row > 0)
	{
		STableCell actual(mLastDropCursor);
		if (actual.row > rows)
			actual.row--;
		mHierarchyTableDragPane->FocusDraw();
		mHierarchyTableDragPane->DrawRow(actual.row);
		if (mHierarchyTableDragPane->CellIsSelected(actual))
			mHierarchyTableDragPane->HiliteRow(actual.row, true);
		if (actual.row == mLastDropCell.row)
		{
			mLastDropCell.row = 0;
			DrawDropCell(inDragRef, actual);
		}
	}

	// Then draw new drag hilite
	if (theCell.row > 0)
	{
		mHierarchyTableDragPane->FocusDraw();

		Rect frame;
		if (theCell.row <= rows)
		{
			mHierarchyTableDragPane->GetLocalRowRect(theCell.row, frame);
			frame.bottom = frame.top + 2;
		}
		else
		{
			mHierarchyTableDragPane->GetLocalRowRect(theCell.row - 1, frame);
			frame.top = frame.bottom - 2;
		}
		StColorPenState save_state;
		StColorPenState::Normalize();
		::PenSize(2,2);
		StColorPenState::SetGrayPattern();
		::PenMode(patOr);
		::FrameRect(&frame);
	}

	// Now add hilite again
	if (mIsHilited)
		HiliteDropArea(inDragRef);

	// Reset current value
	mLastDropCursor = theCell;
}
