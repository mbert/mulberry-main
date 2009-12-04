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


// Source for CAddressTableAction class


#include "CAddressTableAction.h"

#include "CAdbkAddress.h"
#include "CAddressList.h"
#include "CAddressTable.h"
#include "CMulberryCommon.h"
#include "CXStringResources.h"

// __________________________________________________________________________________________________
// C L A S S __ CAddressTableNewAction
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressTableNewAction::CAddressTableNewAction(CAddressTable* itsTable,
												CAdbkAddress* addr,
												bool wasDirty) :
		LAction("UI::Adbk::AddressNew_Redo", "UI::Adbk::AddressNew_Undo", false)
{
	mItsTable = itsTable;
	mNewItems = new CAddressList;
	mNewItems->push_back(addr);
	mWasDirty = wasDirty;
}

// Default constructor
CAddressTableNewAction::CAddressTableNewAction(CAddressTable* itsTable,
												CAddressList* addrs,
												bool wasDirty) :
		LAction(rsrc::GetString("UI::Adbk::AddressNew_Redo"), rsrc::GetString("UI::Adbk::AddressNew_Undo"), false)
{
	mItsTable = itsTable;
	mNewItems = addrs;
	mWasDirty = wasDirty;
}

// Default destructor
CAddressTableNewAction::~CAddressTableNewAction()
{
	// Delete list without deleting addresses if done
	if (IsDone())
		mNewItems->clear_without_delete();
	delete mNewItems;
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Add new items
void CAddressTableNewAction::AddAddresses(CAddressList* new_addrs)
{
	// Add all to list
	for(CAddressList::iterator iter = new_addrs->begin(); iter != new_addrs->end(); iter++)
	{
		// Test for uniqueness
		if (!mNewItems->IsDuplicate(*iter))
			mNewItems->push_back(*iter);
		else
			// Must delete unwanted
			delete *iter;
	}
}

void CAddressTableNewAction::RedoSelf()
{
	// Add address back into table without copy
	mItsTable->AddAddressesFromList(mNewItems);
}

void CAddressTableNewAction::UndoSelf()
{
	// Delete addresses from table
	mItsTable->RemoveAddressesFromList(mNewItems);
	
	// Restore former dirty state
	if (!mWasDirty)
		mItsTable->SetDirty(false);
}


#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ CAddressTableEditAction
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressTableEditAction::CAddressTableEditAction(CAddressTable* itsTable,
												bool wasDirty) :
		LAction(rsrc::GetString("UI::Adbk::AddressEdit_Redo"), rsrc::GetString("UI::Adbk::AddressEdit_Undo"), false)
{
	mOldItems = nil;
	mNewItems = nil;

	mItsTable = itsTable;

	mOldItems = new CAddressList;
	mNewItems = new CAddressList;
	mWasDirty = wasDirty;
}

// Default destructor
CAddressTableEditAction::~CAddressTableEditAction()
{
	// Always delete address not used
	if (IsDone())
		mNewItems->clear_without_delete();
	else
		mOldItems->clear_without_delete();
	delete mOldItems;
	delete mNewItems;

	// Inform owner
	mItsTable->ActionDeleted();
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Add edited item
void CAddressTableEditAction::AddEdit(CAdbkAddress* old_addr, CAdbkAddress* new_addr)
{
	// Save orginal
	mOldItems->push_back(old_addr);
	
	// Save new
	mNewItems->push_back(new_addr);
}

void CAddressTableEditAction::RedoSelf()
{
	// Change old to new
	mItsTable->ChangeAddressesFromList(mOldItems, mNewItems);
}

void CAddressTableEditAction::UndoSelf()
{
	// Change new to old
	mItsTable->ChangeAddressesFromList(mNewItems, mOldItems);
	
	// Restore former dirty state
	if (!mWasDirty)
		mItsTable->SetDirty(false);
}


#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ CAddressTableDeleteAction
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressTableDeleteAction::CAddressTableDeleteAction(CAddressTable* itsTable,
												CAddressList* deletedItems,
												bool wasDirty) :
		LAction(rsrc::GetString("UI::Adbk::AddressDelete_Redo"), rsrc::GetString("UI::Adbk::AddressDelete_Undo"), false)
{
	mItsTable = itsTable;
	mDeletedItems = deletedItems;
	mWasDirty = wasDirty;
}

// Default destructor
CAddressTableDeleteAction::~CAddressTableDeleteAction()
{
	// Delete all items in list if done
	if (!IsDone())
		mDeletedItems->clear_without_delete();
	delete mDeletedItems;
}


// O T H E R  M E T H O D S ____________________________________________________________________________

void CAddressTableDeleteAction::RedoSelf()
{
	// Add all addresses back into table
	mItsTable->RemoveAddressesFromList(mDeletedItems);
}

void CAddressTableDeleteAction::UndoSelf()
{
	// Add all addresses back into table without copy
	mItsTable->AddAddressesFromList(mDeletedItems);
	
	// Restore former dirty state
	if (!mWasDirty)
		mItsTable->SetDirty(false);
}
