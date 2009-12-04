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


// Source for CGroupTableAction class

#include "CGroupTableAction.h"

#include "CGroup.h"
#include "CGroupList.h"
#include "CGroupTable.h"
#include "CMulberryCommon.h"
#include "CXStringResources.h"

// __________________________________________________________________________________________________
// C L A S S __ CGroupTableNewAction
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CGroupTableNewAction::CGroupTableNewAction(CGroupTable* itsTable,
												CGroup* grp,
												bool wasDirty)
{
	mItsTable = itsTable;
	mNewItems = new CGroupList;
	mNewItems->push_back(grp);
	mWasDirty = wasDirty;
}

// Default destructor
CGroupTableNewAction::~CGroupTableNewAction()
{
	// Delete all items in list if not done
	if (IsDone())
		mNewItems->clear_without_delete();
	delete mNewItems;
	mNewItems = nil;

	// Inform owner
	mItsTable->ProcessCommand(cmd_ActionDeleted, this);
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Add new item
void CGroupTableNewAction::AddGroup(CGroup* new_grp)
{
	// Add to list - assume unique test already done
	mNewItems->push_back(new_grp);
}

void CGroupTableNewAction::GetDescription(Str255 outRedoString, Str255 outUndoString) const
{
	LStr255 redo(rsrc::GetString("UI::Adbk::GroupNew_Redo"));
	LStr255 undo(rsrc::GetString("UI::Adbk::GroupNew_Undo"));
	
	::PLstrcpy(outRedoString, redo);
	::PLstrcpy(outUndoString, undo);
}

void CGroupTableNewAction::RedoSelf()
{
	// Add groups back into table without copy
	mItsTable->AddGroupsFromList(mNewItems);
}

void CGroupTableNewAction::UndoSelf()
{
	// Delete groups from table
	mItsTable->RemoveGroupsFromList(mNewItems);

	// Restore former dirty state
	if (!mWasDirty)
		mItsTable->SetDirty(false);
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ CGroupTableEditAction
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CGroupTableEditAction::CGroupTableEditAction(CGroupTable* itsTable,
												bool wasDirty)
{
	mOldItems = nil;
	mNewItems = nil;

	mItsTable = itsTable;

	mOldItems = new CGroupList;
	mNewItems = new CGroupList;
	mWasDirty = wasDirty;
}

// Default destructor
CGroupTableEditAction::~CGroupTableEditAction()
{
	// Always delete
	if (IsDone())
		mNewItems->clear_without_delete();
	else
		mOldItems->clear_without_delete();
	delete mOldItems;
	mOldItems = nil;
	delete mNewItems;
	mNewItems = nil;

	// Inform owner
	mItsTable->ProcessCommand(cmd_ActionDeleted, this);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Add edited item
void CGroupTableEditAction::AddEdit(CGroup* old_grp, CGroup* new_grp)
{
	// Has group already been edited?
	long existing = mOldItems->FetchIndexOf(old_grp);
	if (existing)
	{
		// Replace the old one
		mNewItems->at(existing - 1) = new_grp;

	}
	else
	{
		// Save old
		mOldItems->push_back(old_grp);

		// Save new
		mNewItems->push_back(new_grp);
	}
}

void CGroupTableEditAction::GetDescription(Str255 outRedoString, Str255 outUndoString) const
{
	LStr255 redo(rsrc::GetString("UI::Adbk::GroupEdit_Redo"));
	LStr255 undo(rsrc::GetString("UI::Adbk::GroupEdit_Undo"));
	
	::PLstrcpy(outRedoString, redo);
	::PLstrcpy(outUndoString, undo);
}

void CGroupTableEditAction::RedoSelf()
{
	// Change old to new
	mItsTable->ChangeGroupsFromList(mOldItems, mNewItems);
}

void CGroupTableEditAction::UndoSelf()
{
	// Change new to old
	mItsTable->ChangeGroupsFromList(mNewItems, mOldItems);

	// Restore former dirty state
	if (!mWasDirty)
		mItsTable->SetDirty(false);
}


#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ CGroupTableDeleteAction
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CGroupTableDeleteAction::CGroupTableDeleteAction(CGroupTable* itsTable,
												CGroupList* deletedItems,
												bool wasDirty)
{
	mItsTable = itsTable;
	mDeletedItems = deletedItems;
	mWasDirty = wasDirty;
}

// Default destructor
CGroupTableDeleteAction::~CGroupTableDeleteAction()
{
	// Delete all items in list
	delete mDeletedItems;
}


// O T H E R  M E T H O D S ____________________________________________________________________________

void CGroupTableDeleteAction::GetDescription(Str255 outRedoString, Str255 outUndoString) const
{
	LStr255 redo(rsrc::GetString("UI::Adbk::GroupDelete_Redo"));
	LStr255 undo(rsrc::GetString("UI::Adbk::GroupDelete_Undo"));
	
	::PLstrcpy(outRedoString, redo);
	::PLstrcpy(outUndoString, undo);
}

void CGroupTableDeleteAction::RedoSelf()
{
	// Remove groups from table
	mItsTable->RemoveGroupsFromList(mDeletedItems);
}

void CGroupTableDeleteAction::UndoSelf()
{
	// Add all groups back into table
	mItsTable->AddGroupsFromList(mDeletedItems);

	// Restore former dirty state
	if (!mWasDirty)
		mItsTable->SetDirty(false);
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ CGroupTableReplaceAction
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CGroupTableReplaceAction::CGroupTableReplaceAction(CGroupTable* itsTable,
																CGroup* old_grp,
																CGroup* new_grp,
																bool wasDirty)
{
	mOldItems = nil;
	mNewItems = nil;

	mItsTable = itsTable;

	mOldItems = new CGroupList;
	mNewItems = new CGroupList;
	mWasDirty = wasDirty;
	
	mOldItems->push_back(old_grp);
	mNewItems->push_back(new_grp);
}

// Default destructor
CGroupTableReplaceAction::~CGroupTableReplaceAction()
{
	// Always delete
	if (IsDone())
		mNewItems->clear_without_delete();
	else
		mOldItems->clear_without_delete();
	delete mOldItems;
	mOldItems = nil;
	delete mNewItems;
	mNewItems = nil;

	// Inform owner
	mItsTable->ProcessCommand(cmd_ActionDeleted, this);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CGroupTableReplaceAction::GetDescription(Str255 outRedoString, Str255 outUndoString) const
{
	LStr255 redo(rsrc::GetString("UI::Adbk::GroupEdit_Redo"));
	LStr255 undo(rsrc::GetString("UI::Adbk::GroupEdit_Undo"));
	
	::PLstrcpy(outRedoString, redo);
	::PLstrcpy(outUndoString, undo);
}

void CGroupTableReplaceAction::RedoSelf()
{
	// Change old to new
	mItsTable->ChangeGroupsFromList(mOldItems, mNewItems);
}

void CGroupTableReplaceAction::UndoSelf()
{
	// Change new to old
	mItsTable->ChangeGroupsFromList(mNewItems, mOldItems);

	// Restore former dirty state
	if (!mWasDirty)
		mItsTable->SetDirty(false);
}
