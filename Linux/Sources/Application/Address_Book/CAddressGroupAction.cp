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


// Source for CAddressGroupAction class


#include "CAddressGroupAction.h"

#include "CAddress.h"
#include "CAddressList.h"
#include "CAddressTable.h"
#include "CGroup.h"
#include "CGroupList.h"
#include "CGroupTable.h"
#include "CMulberryCommon.h"
#include "CXStringResources.h"

#include "HResourceMap.h"

// __________________________________________________________________________________________________
// C L A S S __ CAddressGroupNewAction
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressGroupNewAction::CAddressGroupNewAction(CAddressTable* addrTable,
												CAddress* addr,
												CGroupTable* grpTable,
												CGroup* grp,
												bool wasDirty) :
		LAction(rsrc::GetString("UI::Adbk::AddressNew_Redo"), false)
{
	mAddrTable = addrTable;
	mNewAddrItems = new CAddressList;
	mNewAddrItems->push_back(addr);
	mGrpTable = grpTable;
	mNewGrpItems = new CGroupList;
	mNewGrpItems->push_back(grp);
	mWasDirty = wasDirty;
}

// Default constructor
CAddressGroupNewAction::CAddressGroupNewAction(CAddressTable* addrTable,
												CAddressList* addrs,
												CGroupTable* grpTable,
												CGroupList* grps,
												bool wasDirty) :
		LAction(rsrc::GetString("UI::Adbk::AddressNew_Redo"), false)
{
	mAddrTable = addrTable;
	mNewAddrItems = addrs;
	mGrpTable = grpTable;
	mNewGrpItems = grps;
	mWasDirty = wasDirty;
}

// Default destructor
CAddressGroupNewAction::~CAddressGroupNewAction()
{
	// Delete lists without deleting items if done
	if (IsDone())
	{
		if (mNewAddrItems)
			mNewAddrItems->clear_without_delete();
		if (mNewGrpItems)
			mNewGrpItems->clear_without_delete();
	}
	delete mNewAddrItems;
	delete mNewGrpItems;

	// Inform owners
	if (mAddrTable)
		mAddrTable->ActionDeleted();
	if (mGrpTable)
		mGrpTable->ActionDeleted();
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Add new items
void CAddressGroupNewAction::AddAddresses(CAddressList* new_addrs)
{
	if (mNewAddrItems)
	{
		// Add all to list
		for(CAddressList::iterator iter = new_addrs->begin(); iter != new_addrs->end(); iter++)
		{
			// Test for uniqueness
			if (!mNewAddrItems->IsDuplicate(*iter))
				mNewAddrItems->push_back(*iter);
		}
	}
}

// Add new item
void CAddressGroupNewAction::AddGroups(CGroupList* new_grps)
{
	if (mNewGrpItems)
	{
		// Add all to list
		for(CGroupList::iterator iter = new_grps->begin(); iter != new_grps->end(); iter++)
		{
			// Test for uniqueness
			if (!mNewGrpItems->IsDuplicate(*iter))
				mNewGrpItems->push_back(*iter);
		}
	}
}

void CAddressGroupNewAction::RedoSelf()
{
	// Add items back into tables without copy
	if (mAddrTable)
		mAddrTable->AddAddressesFromList(mNewAddrItems);
	if (mGrpTable)
		mGrpTable->AddGroupsFromList(mNewGrpItems);
}

void CAddressGroupNewAction::UndoSelf()
{
	// Delete items from tables
	if (mAddrTable)
		mAddrTable->RemoveAddressesFromList(mNewAddrItems);
	if (mGrpTable)
		mGrpTable->RemoveGroupsFromList(mNewGrpItems);
	
	// Restore former dirty state
	if (!mWasDirty)
	{
		if (mAddrTable)
			mAddrTable->SetDirty(false);
		if (mGrpTable)
			mGrpTable->SetDirty(false);
	}
}
