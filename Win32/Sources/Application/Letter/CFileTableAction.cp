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

#include "CAttachment.h"
#include "CAttachmentList.h"
#include "CFileTable.h"

// __________________________________________________________________________________________________
// C L A S S __ CFileTableAction
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CFileTableAction::CFileTableAction(CFileTable *owner, int inStringIndex)
	: LAction(inStringIndex, false),
	  mOwner(owner)
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
		CFileTableAction(owner, IDS_FileTableDelete_REDO)
{
	int row = 0;
	
	//while(mOwner->GetNextSelectedCell(row))
	{
		// Get file spec
		CAttachment* attach = NULL;//mOwner->GetAttachList()->at(row);

		// Copy it
		CAttachment* copy = CAttachment::CopyAttachment(*attach);

		// Add to lists
		mItems.push_back(copy);
		mItemPos.Add(row);
	}
}

// Default destructor
CFileTableDeleteAction::~CFileTableDeleteAction()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

void CFileTableDeleteAction::RedoSelf()
{
	// Remove all items in list (in reverse)
	for(int i = mItemPos.GetSize() - 1; i >= 0; i--)
	{
		int row = mItemPos[i];
	
		//mOwner->GetAttachList()->RemoveAttachmentAt(row);
	}
	
	mOwner->ResetTable();
}

void CFileTableDeleteAction::UndoSelf()
{
	// Add all items in list
	CAttachmentList::iterator iter = mItems.begin();
	for(int i = 0; i < mItemPos.GetSize(); i++, iter++)
	{
		int row = mItemPos[i];
	
		// Get copy of info
		CAttachment* copy = CAttachment::CopyAttachment(**iter);

		//mOwner->GetAttachList()->InsertAttachmentAt(row, copy);
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
		CFileTableAction(owner, IDS_FileTableAdd_REDO)
{
	// Add to lists
	mItems.push_back(add);
	int pos = 0; //mOwner->GetAttachList()->size();
	mItemPos.Add(pos);
}

// Default destructor
CFileTableAddAction::~CFileTableAddAction()
{
	// Make sure owner disposes of cached action
	mOwner->SendMessage(WM_COMMAND, IDC_ActionDeleted);
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Default constructor
void CFileTableAddAction::Add(CAttachment* add)
{
	// Delete existing if undone and new copy takes place
	if (!mIsDone)
	{
		// Delete all items in lists
		mItems.DeleteAll();
		mItemPos.RemoveAll();

		// Reset state
		mIsDone = true;
	}

	// Add to lists
	mItems.push_back(add);
	int pos = 0; //mOwner->GetAttachList()->size();
	mItemPos.Add(pos);
	
	// Add to list
	CAttachment* copy;
	copy = CAttachment::CopyAttachment(*add);

	//mOwner->GetAttachList()->InsertAttachmentAt(pos, copy);
	mOwner->ResetTable();
}

void CFileTableAddAction::RedoSelf()
{
	// Add all items in list
	CAttachmentList::iterator iter = mItems.begin();
	for(int i = 0; i < mItemPos.GetSize(); i++, iter++)
	{
		int row = mItemPos[i];
	
		// Get copy of info
		CAttachment* copy = CAttachment::CopyAttachment(**iter);

		//mOwner->GetAttachList()->InsertAttachmentAt(row, copy);
	}
	
	mOwner->ResetTable();
}

void CFileTableAddAction::UndoSelf()
{
	// Remove all items in list (in reverse)
	for(int i = mItemPos.GetSize() - 1; i >= 0; i--)
	{
		int row = mItemPos[i];
	
		//mOwner->GetAttachList()->RemoveAttachmentAt(row);
	}
	
	mOwner->ResetTable();
}