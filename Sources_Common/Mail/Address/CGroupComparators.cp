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


// Source for CGroupComparators class

#include "CGroupComparators.h"

#include "CGroup.h"
#include "CStringUtils.h"

// __________________________________________________________________________________________________
// C L A S S __ C G R O U P C O M P A R A T O R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CGroupComparator::CGroupComparator()
{
} // CGroupComparator::CGroupComparator

// Default destructor
CGroupComparator::~CGroupComparator()
{
} // CGroupComparator::~CGroupComparator

bool CGroupComparator::CompareGroupName(const CGroup* aGrp1, const CGroup* aGrp2)
{
	// Compare ptrs first
	if (aGrp1 == aGrp2)
		return false;

	// Get names
	const char* name1 = aGrp1->GetName();
	const char* name2 = aGrp2->GetName();

	if (!name1 && !name2)
		return false;
	else if (!name1)
		return false;
	else if (!name2)
		return true;

	// Do compare
	return (::strcmpnocase(name1, name2) < 0);
}
