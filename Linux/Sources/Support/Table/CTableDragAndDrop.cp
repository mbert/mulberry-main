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

#include "CLog.h"

#include <JXDisplay.h>
#include <JXDNDManager.h>
#include <JArray.h>
#include <JOrderedSetIterator.h>
#include <JXSelectionManager.h>
#include <JXDragPainter.h>
#include "StPenState.h"
#include <JXColormap.h>
#include "CMulSelectionData.h"

#include <algorithm>
#include <cassert>

#pragma mark ____________________________________CTableDropSource

const int cTableDragAutoScroll = 4;

// __________________________________________________________________________________________________
// C L A S S __ C T A B L E D R A G S U P P O R T
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// __________________________________________________________________________________________________
// C L A S S __ C T A B L E D R A G A N D D R O P S U P P O R T
// __________________________________________________________________________________________________

CTableDragAndDrop* CTableDragAndDrop::sTableDragSource = NULL;
CTableDragAndDrop* CTableDragAndDrop::sTableDropTarget = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTableDragAndDrop::CTableDragAndDrop(JXScrollbarSet* scrollbarSet, 
										JXContainer* enclosure,
										const HSizingOption hSizing, 
										const VSizingOption vSizing,
										const JCoordinate x, 
										const JCoordinate y,
										const JCoordinate w,
										const JCoordinate h)
	: CTable(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
	mLastHitCell.SetCell(0, 0);
	mLastDropCell.SetCell(0, 0);
	mLastDropCursor.SetCell(0, 0);
	mReadOnly = false;
	mDropCell = false;
	mDropCursor = false;
	mAllowDrag = true;
	mSelfDrag = false;
	mExternalDrag = true;
	mAllowMove = true;
	mHandleMove = true;
	mIsDropTarget = false;
	mDropData = NULL;
}

// Default destructor
CTableDragAndDrop::~CTableDragAndDrop()
{
	if (sTableDragSource == this)
		sTableDragSource = NULL;
	if (sTableDropTarget == this)
		sTableDropTarget = NULL;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CTableDragAndDrop::OnCreate()
{
	CTable::OnCreate();
}

void CTableDragAndDrop::HandleMouseDown(const JPoint& pt, 
				      const JXMouseButton button,
				      const JSize clickCount, 
				      const JXButtonStates& buttonStates,
				      const JXKeyModifiers& modifiers)
{
	if (button == kJXButton1) {//It's a left click
		if (clickCount == 1 && mAllowDrag) {//single click
			startPoint = pt;
		}
	}
	CTable::HandleMouseDown(pt, button, clickCount, buttonStates, modifiers);
}

void CTableDragAndDrop::HandleMouseDrag(const JPoint& pt,
										const JXButtonStates& buttonStates,
										const JXKeyModifiers& modifiers)
{
	if (buttonStates.left() && 
			mAllowDrag && 
			(JLAbs(pt.x - startPoint.x) > kDebounceWidth ||
			 JLAbs(pt.y - startPoint.y) > kDebounceWidth) &&
			IsSelectionValid()) {
		DoDrag(pt, buttonStates, modifiers);
	}
}

void CTableDragAndDrop::SelectionChanged()
{
	// Only do if not deferred and not a drop target
	if (!mDeferSelectionChanged && !mIsDropTarget)
		DoSelectionChanged();
	else
		// Set this flag to force selection change when defer is turned off
		mSelectionChanged = true;
}

bool CTableDragAndDrop::DoDrag(const JPoint& pt,
								 const JXButtonStates& buttonStates,
								 const JXKeyModifiers& modifiers)
{
	CFlavorsList types;
	if (GetSelectionAtom(types))
		return BeginDND(pt, buttonStates, modifiers, new CMulSelectionData(this, types));
	else 
		return false;
}

Atom CTableDragAndDrop::GetDNDAction(const JXContainer* target,
									 const JXButtonStates& buttonStates,
									 const JXKeyModifiers& modifiers)
{
	JXDNDManager* dndMgr = GetDNDManager();
	
	Atom action;
	Atom copy = dndMgr->GetDNDActionCopyXAtom();
	Atom move = dndMgr->GetDNDActionMoveXAtom();

	// check for force copy
	if (modifiers.control())
		action = copy;
	// check for force move
	else if (modifiers.meta()) 
		action = move;
	// default -- recommended action is move
	else
		action = move;

	if ((action == move) && mAllowMove && mHandleMove)
		return move;
	else
		return copy;
}

void CTableDragAndDrop::DNDInit(const JPoint& pt, 
								const JXButtonStates& buttonStates,
								const JXKeyModifiers& modifiers)
{
	sTableDragSource = this;
	sTableDropTarget = this;
}


void CTableDragAndDrop::HandleDNDEnter()
{
	// Set current droptarget as this
	sTableDropTarget = this;

	// Set flag for drawing
	mIsDropTarget = true;

	// Reset row hiliting
	mLastHitCell.SetCell(0, 0);
	mLastDropCell.SetCell(0, 0);
	mLastDropCursor.SetCell(0, 0);
}

void CTableDragAndDrop::HandleDNDLeave()
{
	// Unhighlight if drop into cell or cursor
	if (mDropCell)
		DrawDropCell(STableCell(0, 0));
	if (mDropCursor)
		DrawDropCursor(STableCell(0, 0));

	// Set flag for drawing
	mIsDropTarget = false;

	// Need to delete data in case leaving because of a drop
	if (mDropData)
	{
		GetDisplay()->GetSelectionManager()->DeleteData(&mDropData, mDelMethod);
	}
	mDropData = NULL;
	
        JPainter* p = NULL;
        JBoolean ok = GetDragPainter(&p);
	if (ok && p != NULL)
		DeleteDragPainter();
}


void CTableDragAndDrop::HandleDNDDidDrop(const Atom& action)
{
	// This must be done by the derived class if it wants it
	// Some windows do their own post-processing so this should
	// not happen by default
#if 0
	// Handle move operation
	JXDNDManager* dndMgr = GetDNDManager();
	Atom move = dndMgr->GetDNDActionMoveXAtom();
	if ((action == move) && mAllowMove)
		DeleteSelection();
#endif
}

bool CTableDragAndDrop::GetDropData(Time time)
{
	// Get the data from the selection
	if (!mDropData)
	{
		// Check to make sure the drag contains a mDropFlavors item.
		Atom best = GetBestFlavor(mCurrentDropFlavors);
		if (best)
		{
			Atom typeReturned;
			GetDisplay()->GetSelectionManager()->
				GetData(GetDNDManager()->GetDNDSelectionName(),
								time,
								best,
								&typeReturned,
								&mDropData,
								&mDataLen,
								&mDelMethod);
		
			mDropFlavor = typeReturned;
			return true;
		}
		else
			return false;
	} 
	else
		//if mDropData isn't NULL, we already have data, so just return true;
		return true;
}

void CTableDragAndDrop::HandleDNDDrop(const JPoint& pt,
										const JArray<Atom>& typeList,
										const Atom action,
										const Time time,
										const JXWidget* source)
{

  // Clear existing cell hilight
  ClearDropCell(mDropCell);

	// Cache the action for reference
	JXDNDManager* dndMgr = GetDNDManager();
	Atom move = dndMgr->GetDNDActionMoveXAtom();
	mDropActionMove = (action == move) && mAllowMove;

	// Check to make sure the drag contains a mDropFlavors item.
	if (GetDropData(time))
	{
		if (mDataLen)
		{
			bool done = false;

			// Protect against exceptions as JX does not clean itself up properly
			// when an exception is thrown
			try
			{
				// Check for drop into cell
				bool was_dropped = false;
				if (mDropCell)
				{
					STableCell dropCell;
					JPoint imagePt;
					LocalToImagePoint(pt, imagePt);
					if (GetCellHitBy(imagePt, dropCell) && IsDropCell(mCurrentDropFlavors, dropCell))
					{
						if (mDropCursor)
						{
							// Check if close to edge of cell and drop cursor allowed
							JRect rowRect;
							GetLocalCellRect(dropCell, rowRect);
							
							if ((pt.y < rowRect.top + 2) || (pt.y > rowRect.bottom - 2))
								dropCell.row = 0;
						}

						if (dropCell.row > 0)
						{
							// Prevent drop in own selection
							if ((sTableDragSource != this) || !CellIsSelected(dropCell))
							{
								done = DropDataIntoCell(mDropFlavor, mDropData, mDataLen, dropCell);
								was_dropped = (dropCell.row > 0);
							}
						}
					}
				}

				if (mDropCursor && !was_dropped)
				{
					// Get the hit cell
					STableCell	hitCell;
					JPoint imagePt;
					LocalToImagePoint(pt, imagePt);
					GetCellHitBy(imagePt, hitCell);

					JRect rowRect;
					GetLocalCellRect(hitCell, rowRect);

					JPoint offset = pt;
					offset += JPoint(0, rowRect.height()/2);

					// Get the hit cell
					LocalToImagePoint(offset, imagePt);
					GetCellHitBy(imagePt, hitCell);
					
					if (mRows == 0)
						hitCell.row = 1;
					else if (!IsValidCell(hitCell))
						hitCell.row = mRows + 1;
					
					if ((hitCell.row > 0) && IsDropAtCell(mCurrentDropFlavors, hitCell))
					{
						done = DropDataAtCell(mDropFlavor, mDropData, mDataLen, hitCell);
						was_dropped = true;
					}
				}
				
				if (!was_dropped)
					done = DropData(mDropFlavor, mDropData, mDataLen);
				}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				done = false;
			}

			// Broadcast to source that drop occurred
			if (done && sTableDragSource)
				sTableDragSource->HandleDNDDidDrop(action);
		}
	}
#if NOTYET
	JXSelectionManager* selMgr = GetDisplay()->GetSelectionManager();
	JXDNDManager* dndMgr = GetDNDManager();
	
	if (done && action == dndMgr->GetDNDActionMoveXAtom()) {
		selMgr->SendDeleteSelectionRequest(dndMgr->GetDNDSelectionName(), time, 
																			 GetWindow());
	}
#endif
	HandleDNDLeave();
}

void CTableDragAndDrop::HandleDNDHere(const JPoint& pt, const JXWidget* source)
{
	JPoint	imagePt;
	LocalToImagePoint(pt, imagePt);

	bool hilite = false;
	STableCell lastDrop = mLastDropCell;
	STableCell lastCursor = mLastDropCursor;

	if (mDropCell && GetItemCount())
	{
		// Get the hit cell
		STableCell	hitCell;
		GetCellHitBy(imagePt, hitCell);
		mLastHitCell = hitCell;

		// Must be able to drop
		if (!IsDropCell(mCurrentDropFlavors, hitCell))
			hitCell.SetCell(0, 0);
		else if (mDropCursor)
		{
			// Check if close to edge of cell and drop cursor allowed
			JRect rowRect;
			GetLocalCellRect(hitCell, rowRect);
			
			if ((pt.y < rowRect.top + 2) || (pt.y > rowRect.bottom - 2))
				hitCell.SetCell(0, 0);
			else
				hilite = true;
		}
		else
			hilite = true;

		// Draw new cursor
		DrawDropCell(hitCell);
	}

	if (mDropCursor && GetItemCount())
	{
		// Get the hit cell
		STableCell	hitCell;
		GetCellHitBy(imagePt, hitCell);

		JRect rowRect;
		GetLocalCellRect(hitCell, rowRect);

		JPoint offset = imagePt;
		offset += JPoint(0, rowRect.height()/2);

		// Get the hit cell
		GetCellHitBy(imagePt, hitCell);
		
		if (!hilite)
		{
			if (!IsValidCell(hitCell))
				hitCell.SetCell(mRows + 1, 1);
			
			if (!IsDropAtCell(mCurrentDropFlavors, hitCell))
				hitCell.SetCell(0, 0);
		}
		else
			hitCell.SetCell(0, 0);

		DrawDropCursor(hitCell);
	}

	// Check for auto scroll
	if (GetItemCount())
	{
		int	hScroll = 0;
		int	vScroll = 0;
		JRect frame = GetFrameLocal();

		if (pt.y < frame.top + cTableDragAutoScroll)
			vScroll = -GetRowHeight(1);
		else if (pt.y > frame.bottom - cTableDragAutoScroll)
			vScroll = GetRowHeight(1);
		else if (pt.x < frame.left + cTableDragAutoScroll)
			hScroll = -16;
		else if (pt.x > frame.right - cTableDragAutoScroll)
			hScroll = 16;

		Scroll(-hScroll, -vScroll);
	}
}

// Called on DND entry
JBoolean CTableDragAndDrop::WillAcceptDrop(const JArray<Atom>& typeList, 
											 Atom* action,
											 const Time time, 
											 const JXWidget* source)
{
	// Cache flavors being dragged
	mCurrentDropFlavors = typeList;

	// Test for acceptable flavors
	if (!ItemIsAcceptable(typeList) ||
		((sTableDragSource == this) && !mSelfDrag) ||
		((sTableDragSource != this) && !mExternalDrag))
		return kFalse;
	else
		return kTrue;			
}

// Check whether drag item is acceptable
bool CTableDragAndDrop::ItemIsAcceptable(const JArray<Atom>& typeList)
{
	// Cannot accept if read only
	if (mReadOnly)
		return kFalse;

	return FlavorsMatch(typeList, mDropFlavors);

} // CTableDragAndDrop::ItemIsAcceptable

// Check whether drag item is acceptable
bool CTableDragAndDrop::ItemIsAcceptable(Atom type)
{
	// Cannot accept if read only
	if (mReadOnly)
		return kFalse;

	CFlavorsList::const_iterator found = std::find(mDropFlavors.begin(), mDropFlavors.end(), type);
	return found != mDropFlavors.end();

} // CTableDragAndDrop::ItemIsAcceptable

// Test drop into cell
bool CTableDragAndDrop::IsDropCell(JArray<Atom>& falvorList, const STableCell& cell)
{
	return true;
}

// Test drop at cell
bool CTableDragAndDrop::IsDropAtCell(JArray<Atom>& flavorList, STableCell& cell)
{
	return true;
}

// Get rect of row
void CTableDragAndDrop::GetLocalRowRect(TableIndexT row, JRect& theRect)
{
	STableCell aCell(row, 1);

	// Get rect for row (scan columns for visible rows)
	while(!GetLocalCellRect(aCell, theRect) && (aCell.col <= mCols))
		aCell.col++;
	JRect endRect;
	aCell.col = mCols;
	while(!GetLocalCellRect(aCell, endRect) && (aCell.col >= 1))
		aCell.col--;
	theRect.right = endRect.right;
}

void CTableDragAndDrop::DrawDropCell(const STableCell& cell)
{
	// Only do if different
	if (mLastDropCell.row == cell.row)
		return;

        JPainter* p = NULL;
        JBoolean ok = GetDragPainter(&p);
	if (!ok || p  == NULL)
		CreateDragInsidePainter();

        ok = GetDragPainter(&p);
        assert( ok );

	// First remove current drag hilite
	if (mLastDropCell.row > 0)
	{
		// Get row rect
		JRect rowRect;
		GetLocalCellRect(mLastDropCell, rowRect);

 		p->RectInside(rowRect);	
	}

	// Then draw new drag hilite
	if (cell.row > 0)
	{
		// Get row rect
		JRect rowRect;
		GetLocalCellRect(cell, rowRect);

		p->RectInside(rowRect);	
	}

	// Reset current value
	mLastDropCell = cell;
}

void CTableDragAndDrop::ClearDropCell(const STableCell& cell)
{
  // Nothing to do here
}

// Draw drag insert cursor
void CTableDragAndDrop::DrawDropCursor(const STableCell& cell)
{
	// Only do if different
	if (mLastDropCursor.row == cell.row)
		return;

        JPainter* p = NULL;
        JBoolean ok = GetDragPainter(&p);
        if (!ok || p  == NULL)
                CreateDragInsidePainter();

        ok = GetDragPainter(&p);
        assert( ok );

	// First remove current drag hilite
	if (mLastDropCursor.row > 0)
	{
		// Get row rect
		JRect rowRect;
		if (mLastDropCursor <= GetItemCount())
		{
			GetLocalRowRect(mLastDropCursor.row, rowRect);
			rowRect.bottom = rowRect.top + 2;
		}
		else
		{
			STableCell temp = mLastDropCursor;
			temp.row--;
			GetLocalRowRect(temp.row, rowRect);
			rowRect.top = rowRect.bottom - 2;
		}

		// Line goes all the way across the visible part of the list
		p->RectInside(rowRect);
	}

	// Then draw new drag hilite
	if (cell.row > 0)
	{
		// Get row rect
		JRect rowRect;
		if (cell.row <= GetItemCount())
		{
			GetLocalRowRect(cell.row, rowRect);
			rowRect.bottom = rowRect.top + 2;
		}
		else
		{
			STableCell temp = cell;
			temp.row--;
			GetLocalRowRect(temp.row, rowRect);
			rowRect.top = rowRect.bottom - 2;
		}

		// Line goes all the way across the visible part of the list
		p->RectInside(rowRect);
	}

	// Reset current value
	mLastDropCursor = cell;
}

// Check whether drag item is acceptable
Atom CTableDragAndDrop::GetBestFlavor(const JArray<Atom>& list)
{
	// Check against mDropFlavors in order
	for(CFlavorsList::const_iterator iter = mDropFlavors.begin(); iter != mDropFlavors.end(); iter++)
	{
		for(JIndex index = 1; index <= list.GetElementCount(); index++)
		{
			if (list.GetElement(index) == *iter)
				return *iter;
		}
	}
	
	return 0;

}

// Drop data into whole table
bool CTableDragAndDrop::DropData(Atom theFlavor,
									unsigned char* drag_data,
									unsigned long data_size)
{
	return false;
}

// Drop data into cell
bool CTableDragAndDrop::DropDataIntoCell(Atom theFlavor, 
											unsigned char* drag_data,
											unsigned long data_size, const STableCell& cell)
{
	return false;
}

// Drop data at cell
bool CTableDragAndDrop::DropDataAtCell(Atom theFlavor, 
										unsigned char* drag_data,
										unsigned long data_size, const STableCell& cell)
{
	return false;
}


bool CTableDragAndDrop::FlavorsMatch(const JArray<Atom>& list1, 
									 const CFlavorsList& list2, 
									 Atom* match) const
{
	JOrderedSetIterator<Atom>* ti = list1.NewIterator();
	Atom i;
	while (ti->Next(&i))
	{
		CFlavorsList::const_iterator found = std::find(list2.begin(), list2.end(), i);
		if (found != list2.end()) {
			if (match) *match = i;
			delete ti;
			return true;
		}
	}
	delete ti;
	return false;
}

bool CTableDragAndDrop::GetSelectionAtom(CFlavorsList& atoms)
{
	// Default - add all drag flavors
	mDragFlavor = (mDragFlavors.size() ? mDragFlavors.front() : 0);
	atoms = mDragFlavors;
	return mDragFlavors.size();
}

bool CTableDragAndDrop::RenderSelectionData(CMulSelectionData* selection, Atom type)
{
	// Does nothing - sub-classes must override
	return false;
}
