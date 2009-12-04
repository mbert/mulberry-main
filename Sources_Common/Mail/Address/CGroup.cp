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


// Source for CGroup class

#include "CGroup.h"

#include "CAddress.h"
#include "CAddressList.h"

#include <algorithm>

// __________________________________________________________________________________________________
// C L A S S __ C G R O U P
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CGroup::CGroup()
{
} // CGroup::CGroup

// Copy constructor
CGroup::CGroup(const CGroup &copy)
	:	mEntry(copy.mEntry),
		mNickName(copy.mNickName),
		mName(copy.mName),
		mAddresses(copy.mAddresses)
{
} // CGroup::CGroup

// Constructor from name
CGroup::CGroup(const char* name, const char* nickname)
{
	// Copy nick-name
	mNickName = nickname;

	// Copy name
	mName = name;

} // CGroup::CGroup

// Assignment with same type
CGroup& CGroup::operator=(const CGroup& copy)
{
	// Do not assign this to this
	if (this != &copy)
	{
		// Copy bits
		mEntry = copy.mEntry;
		mNickName = copy.mNickName;
		mName = copy.mName;
		mAddresses = copy.mAddresses;
	}

	return *this;
}

// Default destructor
CGroup::~CGroup()
{
} // CGroup::~CGroup

// Compare with name
int CGroup::operator==(const char* grp) const
{
	return (::strcmp(mName, grp) == 0);
}

// Compare with another
int CGroup::operator==(const CGroup& grp) const
{
	return (mName == grp.GetName());
}

// Compare specific field
bool CGroup::Search(const cdstring& text, const CAdbkAddress::CAddressFields& fields) const
{
	bool result = false;

	for(CAdbkAddress::CAddressFields::const_iterator iter = fields.begin(); iter != fields.end(); iter++)
	{
		switch(*iter)
		{
		case CAdbkAddress::eName:
			result = GetName().PatternMatch(text);
			break;
		case CAdbkAddress::eNickName:
			result = GetNickName().PatternMatch(text);
			break;
		default:;
		}
		
		if (result)
			break;
	}
	
	return result;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Add unique address
void CGroup::AddAddress(const char* addr)
{
	cdstring aStr(addr);
	mAddresses.push_back(aStr);
}

// Add addresses from list
void CGroup::AddAddresses(const CAddressList& list, bool full_name)
{
	for(CAddressList::const_iterator iter = list.begin(); iter != list.end(); iter++)
	{
		cdstring aStr = full_name ? (*iter)->GetFullAddress() : (*iter)->GetMailAddress();
		mAddresses.push_back(aStr);
	}
}

// Merge existing group with new one
void CGroup::Merge(CGroup* grp)
{
	for(cdstrvect::const_iterator iter = grp->mAddresses.begin();
		iter != grp->mAddresses.end(); iter++)
	{
		// Check for unique
		if (find(mAddresses.begin(), mAddresses.end(), *iter) == mAddresses.end())
			mAddresses.push_back(*iter);
	}
}

// Check whether it has some info
bool	CGroup::IsEmpty() const
{
	return mNickName.empty() && mName.empty() && mAddresses.empty();
}
