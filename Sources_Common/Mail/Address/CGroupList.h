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


// Header for CGroupList class

#ifndef __CGROUPLIST__MULBERRY__
#define __CGROUPLIST__MULBERRY__

#include "ptrvector.h"
#include "CGroup.h"
#include "CGroupComparators.h"

// Classes

class CGroupList : public ptrvector<CGroup>
{

public:
	explicit CGroupList(unsigned long capacity = 0);
	CGroupList(const CGroupList &copy, unsigned long capacity = 0);			// Copy constructor
	virtual ~CGroupList();

	virtual void	SetComparator(CGroupComparator* comp)
						{ delete mComparator; mComparator = comp; if (comp) Sort(); }
	virtual void 	Sort();										// Force resort

	virtual bool	InsertUniqueItem(CGroup* aGrp);					// Insert non-duplicates
	virtual bool	IsDuplicate(const CGroup* aGrp) const;			// Find duplicate in list
	virtual void	push_back_sorted(CGroup* aGrp);					// Add group in sorted order

	virtual unsigned long	FetchIndexOf(const CGroup* grp) const;			// Find position for address
	virtual unsigned long	FetchInsertIndexOf(const CGroup* grp) const;	// Find position for sorted insert

	virtual void	clear_without_delete();							// Delete all entries without deleting groups

	virtual void	RemoveGroup(CGroupList* grps);					// Remove matching group
	virtual void	RemoveGroup(CGroup* grp);						// Remove matching group

private:
	CGroupComparator*	mComparator;

	virtual void InitGroupList(unsigned long capacity);
};

#endif
