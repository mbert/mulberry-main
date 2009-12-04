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


// Source for CGroupList class

#include "CGroupList.h"

#include "CGroup.h"
#include "CGroupComparators.h"
#include <algorithm>

// Specializations

#if 0
typedef bool (*voidCompare)(const void*, const void*);
void stable_sort (CGroupList::iterator first, CGroupList::iterator last, GroupCompare comp);
void stable_sort (CGroupList::iterator first, CGroupList::iterator last, GroupCompare comp)
{
	stable_sort(reinterpret_cast<vector<void*>::iterator>(first),
			reinterpret_cast<vector<void*>::iterator>(last),
			reinterpret_cast<voidCompare>(comp));
}

CGroupList::iterator lower_bound (CGroupList::iterator first, CGroupList::iterator last, const CGroup*& value, GroupCompare comp);
CGroupList::iterator lower_bound (CGroupList::iterator first, CGroupList::iterator last, const CGroup*& value, GroupCompare comp)
{
	return
		reinterpret_cast<CGroupList::iterator>(lower_bound(
				reinterpret_cast<vector<void*>::iterator>(first),
				reinterpret_cast<vector<void*>::iterator>(last),
				reinterpret_cast<const void*>(value),
				reinterpret_cast<voidCompare>(comp)));
}

CGroupList::const_iterator lower_bound (CGroupList::const_iterator first, CGroupList::const_iterator last, const CGroup*& value, GroupCompare comp);
CGroupList::const_iterator lower_bound (CGroupList::const_iterator first, CGroupList::const_iterator last, const CGroup*& value, GroupCompare comp)
{
	return
		reinterpret_cast<CGroupList::const_iterator>(lower_bound(
				reinterpret_cast<vector<void*>::const_iterator>(first),
				reinterpret_cast<vector<void*>::const_iterator>(last),
				reinterpret_cast<const void*>(value),
				reinterpret_cast<voidCompare>(comp)));
}
#endif

// __________________________________________________________________________________________________
// C L A S S __ C G R O U P L I S T
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CGroupList::CGroupList(unsigned long capacity)
{
	InitGroupList(capacity);

} // CGroupList::CGroupList

// Copy constructor
CGroupList::CGroupList(const CGroupList &copy, unsigned long capacity)
	: ptrvector<CGroup>(copy)
{
	InitGroupList(capacity);

} // CGroupList::CGroupList

// Default destructor
CGroupList::~CGroupList()
{
	delete mComparator;
	mComparator = NULL;

} // CGroupList::~CGroupList

// O T H E R  M E T H O D S ____________________________________________________________________________

void CGroupList::InitGroupList(unsigned long capacity)
{
	if (capacity)
		reserve(capacity);

	mComparator = NULL;
}

// Sort groups in list
void CGroupList::Sort()
{
	if (mComparator)
		std::stable_sort(begin(), end(), mComparator->GetComparator());
}

// Insert non-duplicates
bool CGroupList::InsertUniqueItem(CGroup* aGrp)
{
	// Compare with existing groups
	if (IsDuplicate(aGrp))
		return false;
	else
	{
		// Add if not found
		push_back(aGrp);
		return true;
	}
}

// Find duplicate in list
bool CGroupList::IsDuplicate(const CGroup* aGrp) const
{
	for(CGroupList::const_iterator iter = begin(); iter != end(); iter++)
	{
		// Compare ptrs and addresses
		if ((*iter == aGrp) || (**iter == *aGrp))
			return true;
	}

	return false;
}

// Add group in sorted order
void CGroupList::push_back_sorted(CGroup* aGrp)
{
	CGroupList::iterator found = end();

	if (mComparator)
		found = std::lower_bound(begin(), end(), (const CGroup*&) aGrp, mComparator->GetComparator());

	insert(found, aGrp);
}

// Find position for group
unsigned long CGroupList::FetchIndexOf(const CGroup* grp) const
{
	CGroupList::const_iterator found = find(begin(), end(), (CGroup*) grp);

	if (found != end())
	{
		return (found - begin()) + 1;
	}
	else
		return 0;
}

// Find position for sorted insert
unsigned long CGroupList::FetchInsertIndexOf(const CGroup* grp) const
{
	CGroupList::const_iterator found = lower_bound(begin(), end(), (CGroup*) grp, mComparator->GetComparator());

	if (found != end())
	{
		return (found - begin()) + 1;
	}
	else
		return 0;
}

// Delete all entries without deleting addresses
void CGroupList::clear_without_delete()
{
	// Do inherited erase
	std::vector<CGroup*>::erase(begin(), end());
}

// Remove matching group (without deleting it)
void CGroupList::RemoveGroup(CGroupList* grps)
{
	if (!grps)
		return;

	for(CGroupList::const_iterator iter = grps->begin(); iter != grps->end(); iter++)
		RemoveGroup(*iter);
}

// Remove matching group (without deleting it)
void CGroupList::RemoveGroup(CGroup* grp)
{
	if (!grp)
		return;

	CGroupList::iterator found = std::find(begin(), end(), grp);
	if (found != end())
		std::vector<CGroup*>::erase(found);
}

