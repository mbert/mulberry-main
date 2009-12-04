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


// Source for CFileTableAction class

#include "CFileTableAction.h"
#include "CFileTable.h"
#include "CAttachment.h"



// __________________________________________________________________________________________________
// C L A S S __ CFileTableAction
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CFileTableAction::CFileTableAction(CFileTable *owner, ResIDT inStringResID, SInt16 inStringIndex)
	: LAction(inStringResID, inStringIndex, false),
	  mOwner(owner),
	  mItemPos(sizeof(TableIndexT))
{
}

// Default destructor
CFileTableAction::~CFileTableAction()
{
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ CFileTableDeleteAction
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CFileTableDeleteAction::CFileTableDeleteAction(CFileTable *owner) :
		CFileTableAction(owner, STRx_RedoFileTableActions, str_FileTableDelete)
{
	STableCell aCell(0, 0);

	while(mOwner->GetNextSelectedCell(aCell)) {

		// Do once for each row only
		if (aCell.col==1) {
			// Get file spec
			CAttachment* attach = mOwner->GetBody()->GetPart(aCell.row);

			// Copy it
			CAttachment* copy = CAttachment::CopyAttachment(*attach);

			// Add to lists
			mItems.push_back(copy);
			mItemPos.InsertItemsAt(1, LArray::index_Last, &aCell.row);
		}
	}
}

// Default destructor
CFileTableDeleteAction::~CFileTableDeleteAction()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

void CFileTableDeleteAction::RedoSelf()
{
	// Remove all items in list
	LArrayIterator iterator(mItemPos, LArrayIterator::from_End);

	TableIndexT row;

	while(iterator.Previous(&row))
	{
		//mOwner->GetBody()->RemoveAttachmentAt(row - 1);
	}

	mOwner->ResetTable();
}

void CFileTableDeleteAction::UndoSelf()
{
	// Add all items in list
	CAttachmentList::iterator iterator1 = mItems.begin();
	LArrayIterator iterator2(mItemPos, LArrayIterator::from_Start);

	TableIndexT row;

	while(iterator2.Next(&row))
	{
		// Get copy of info
		CAttachment* copy = CAttachment::CopyAttachment(**iterator1);

		//mOwner->GetBody()->InsertAttachmentAt(row - 1, copy);

		// Move to next attachment
		iterator1++;
	}

	mOwner->ResetTable();
}



#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ CFileTableAddAction
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CFileTableAddAction::CFileTableAddAction(CFileTable *owner, CAttachment* add) :
		CFileTableAction(owner, STRx_RedoFileTableActions, str_FileTableAdd)
{
	// Add to lists
	mItems.push_back(add);
	TableIndexT pos = mOwner->GetBody()->CountParts()+1;
	mItemPos.InsertItemsAt(1, LArray::index_Last, &pos);
}

// Default destructor
CFileTableAddAction::~CFileTableAddAction()
{
	// Make sure owner disposes of cached action
	mOwner->ProcessCommand(cmd_ActionDeleted, this);
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Default constructor
void CFileTableAddAction::Add(CAttachment* add)
{
	// Delete existing if undone and new copy takes place
	if (!mIsDone) {
		// Delete all items in lists
		mItems.clear();
		mItemPos.RemoveItemsAt(mItemPos.GetCount(), LArray::index_Last);

		// Reset state
		mIsDone = true;
	}

	// Add to lists
	mItems.push_back(add);
	TableIndexT pos = mOwner->GetBody()->CountParts()+1;
	mItemPos.InsertItemsAt(1, LArray::index_Last, &pos);

	// Add to list
	CAttachment* copy;
	copy = CAttachment::CopyAttachment(*add);

	//mOwner->GetAttachList()->InsertAttachmentAt(pos - 1, copy);
	mOwner->ResetTable();
}

void CFileTableAddAction::RedoSelf()
{
	// Add all items in list
	CAttachmentList::iterator iterator1 = mItems.begin();
	LArrayIterator iterator2(mItemPos, LArrayIterator::from_Start);

	TableIndexT row;

	while(iterator2.Next(&row))
	{
		// Get copy of info
		CAttachment* copy = CAttachment::CopyAttachment(**iterator1);

		//mOwner->GetAttachList()->InsertAttachmentAt(row - 1, copy);

		// Move to next attachment
		iterator1++;
	}

	mOwner->ResetTable();
}

void CFileTableAddAction::UndoSelf()
{
	// Remove all items in list
	LArrayIterator iterator(mItemPos, LArrayIterator::from_End);

	TableIndexT row;

	while(iterator.Previous(&row))
	{
		//mOwner->GetAttachList()->RemoveAttachmentAt(row - 1);
	}

	mOwner->ResetTable();
}