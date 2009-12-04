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


// Source for CAddressList class

#include "CAddressList.h"

#include "CAddress.h"
#include "CAdbkAddress.h"
#include "CAddressComparators.h"
#include "CCalendarAddress.h"
#ifdef __MULBERRY
#include "CMailControl.h"
#endif
#include "CStringUtils.h"

#include <string.h>
#include <algorithm>
#include <ostream>

// Specializations

#if 0
typedef bool (*voidCompare)(const void*, const void*);
void stable_sort (CAddressList::iterator first, CAddressList::iterator last, AddressCompare comp);
void stable_sort (CAddressList::iterator first, CAddressList::iterator last, AddressCompare comp)
{
	stable_sort(reinterpret_cast<vector<void*>::iterator>(first),
				reinterpret_cast<vector<void*>::iterator>(last),
				reinterpret_cast<voidCompare>(comp));
}

CAddressList::iterator lower_bound (CAddressList::iterator first, CAddressList::iterator last, const CAddress*& value, AddressCompare comp);
CAddressList::iterator lower_bound (CAddressList::iterator first, CAddressList::iterator last, const CAddress*& value, AddressCompare comp)
{
	return
		reinterpret_cast<CAddressList::iterator>(lower_bound(
				reinterpret_cast<vector<void*>::iterator>(first),
				reinterpret_cast<vector<void*>::iterator>(last),
				reinterpret_cast<const void*>(value),
				reinterpret_cast<voidCompare>(comp)));
}

CAddressList::const_iterator lower_bound (CAddressList::const_iterator first, CAddressList::const_iterator last, const CAddress*& value, AddressCompare comp);
CAddressList::const_iterator lower_bound (CAddressList::const_iterator first, CAddressList::const_iterator last, const CAddress*& value, AddressCompare comp)
{
	return
		reinterpret_cast<CAddressList::const_iterator>(lower_bound(
				reinterpret_cast<vector<void*>::const_iterator>(first),
				reinterpret_cast<vector<void*>::const_iterator>(last),
				reinterpret_cast<const void*>(value),
				reinterpret_cast<voidCompare>(comp)));
}
#endif

// __________________________________________________________________________________________________
// C L A S S __ C A D D R E S S L I S T
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressList::CAddressList(unsigned long capacity)
{
	InitAddressList(capacity);

} // CAddressList::CAddressList

// Copy constructor
CAddressList::CAddressList(const CAddressList& copy, unsigned long capacity)
{
	InitAddressList(capacity);

	// Check for adbk addresses
	bool use_adbk = copy.size() && dynamic_cast<CAdbkAddress*>(copy.front());

	// Copy all addresses
	for(CAddressList::const_iterator iter = copy.begin(); iter != copy.end(); iter++)
		push_back(use_adbk ? new CAdbkAddress(*static_cast<CAdbkAddress*>(*iter)) : new CAddress(**iter));

} // CAddressList::CAddressList

// Constructor from delimited text list
CAddressList::CAddressList(const char* txt, long txt_length, unsigned long capacity, bool resolve)
{
	InitAddressList(capacity);

	// Just exit if nothing to parse
	if (!txt || !*txt || !txt_length)
		return;

	// Make local copy of text to be tokenised
	char* s = ::strndup(txt, txt_length);

	char* p = s;
	while(*p == ' ') p++;
	char* q = s;
	char* adl = NULL;
	bool more = true;

	// Check each character - need to balance "É" & (É)
	while(more) {
		switch (*p) {

			case '"':
				// Cache for adl
				adl = p++;

				// Find match and terminate and step over
				while(*p)
				{
					if (*p == '\\')
					{
						p++;
						if (*p) p++;
					}
					else if (*p != '"') p++;
					else break;
				}
				if (*p)
					p++;
				else
					more = false;
				break;

			case '(':
				long level;
				level = 0;
				// Find match and step over
				while(*p && ((*p != ')') || (level > 1))) {
					// Balance nested brackets
					switch (*p) {

						case '(':
							level++;
							break;

						case ')':
							level--;
							break;

						default:;
					}
					p++;
				}
				if (*p)
					p++;
				else
					more = false;
				break;

			case '\0':
				// Force exit next time
				more = false;

				// Check that there is something to copy
				if (p==q) break;

				// Fall through to get address
			case '\r':
			case '\n':
			case ',':
				// Tie off
				*p++ = '\0';

				// Is there anyone there?
				if (!::strlen(q))
				{
					// Do nothing
				}
				// Check for full email address (includes an @)
				else if (::strchr(q, '@')) {
					// Insert into list
					push_back(new CAddress(q));
				}
#ifdef __MULBERRY
				// Try nick name
				else if (resolve && CMailControl::ResolveNickName(q, this)) {
					// Inserted into list
				}
				// Try group name
				else if (resolve && CMailControl::ResolveGroupName(q, this)) {
					// Inserted into list
				}
#endif
				else {
					// Must have local address
					push_back(new CAddress(q));
				}

				// Strip leading space, CR's, LF's & ','s and point to next bit
				while((*p == ' ') || (*p == '\r') || (*p == '\n') || (*p == ',')) p++;
				q = p;

				// Wipe name
				adl = NULL;
				break;

			default:
				// Just advance
				p++;
		}
	}

	delete s;

} // CAddressList::CAddressList

// Default destructor
CAddressList::~CAddressList()
{
	delete mComparator;
	mComparator = NULL;

} // CAddressList::~CAddressList

// Common init
void CAddressList::InitAddressList(unsigned long capacity)
{
	if (capacity)
		reserve(capacity);

	mComparator = NULL;

} // CAddressList::InitAddressList

// O T H E R  M E T H O D S ____________________________________________________________________________

void CAddressList::SetComparator(CAddressComparator* comp)
{
	delete mComparator;
	mComparator = comp;
	if (comp)
		Sort();
}

// Sort addresses in list
void CAddressList::Sort()
{
	if (mComparator)
		std::stable_sort(begin(), end(), mComparator->GetComparator());
}

// Merge new list into current - filter out duplicates
void CAddressList::MergeList(const CAddressList* addrs)
{
	// Protect against empty list
	if (!addrs)
		return;

	for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
	{
		// Check duplicate
		if (!IsDuplicate(*iter))
			// Add to list
			push_back(new CAddress(**iter));
	}
}

// Remove any addresses in another list
void CAddressList::SubtractList(const CAddressList* addrs)
{
	// Protect against empty list
	if (!addrs)
		return;

	for(CAddressList::const_iterator iter1 = addrs->begin(); iter1 != addrs->end(); iter1++)
	{
		for(CAddressList::iterator iter2 = begin(); iter2 != end(); iter2++)
		{
			// Compare addresses
			if ((**iter1 == **iter2))
			{
				iter2 = erase(iter2);
				iter2--;
			}
		}
	}
}

// Ensure only unique items
void CAddressList::MakeUnique()
{
	// First sort it
	SetComparator(new CEmailComparator);

	// Now unique it
	CEmailOnlyComparator comp;
	for(iterator iter = begin(); iter != end(); )
	{
		if (iter == begin())
		{
			iter++;
			continue;
		}
		
		// Remove if same as previous
		if ((*comp.GetComparator())(*iter, *(iter-1)))
		{
			iter = erase(iter);
			continue;
		}
		
		iter++;
	}
}

// Insert text address & resolve nick-names
bool CAddressList::InsertTextAddress(const char* txt, unsigned long level)
{
	// Only if text exists
	if (!txt || !*txt)
		return false;

	bool inserted = true;

	// Check for full email address (includes an @)
	if (::strchr(txt, '@'))
	{
		// Make a new address and insert into list
		push_back(new CAddress(txt));
	}
#ifdef __MULBERRY
	// Try nick name
	else if (CMailControl::ResolveNickName(txt, this))
	{
		// Inserted into list
	}
	// Try group name - but do not allow any further recursion
	else if ((level < 10) && CMailControl::ResolveGroupName(txt, this, level + 1))
	{
		// Inserted into list
	}
#endif
	else if (level < 10)
	{
		// Must have local address
		push_back(new CAddress(txt));
	}
	else
		inserted = false;

	return inserted;
}

// Insert non-duplicates
bool CAddressList::InsertUniqueItem(CAddress* anAddr)
{
	// Must have address
	if (!anAddr)
		return false;

	// Compare with existing addresses
	if (IsDuplicate(anAddr))
		return false;
	else
	{
		// Add if not found
		push_back(anAddr);
		return true;
	}
}

// Merge new list into current - filter out duplicates
void CAddressList::AddUnique(const CAddress* anAddr)
{
	// Must have address
	if (!anAddr)
		return;

	// Check duplicate
	if (!IsDuplicate(anAddr))
		// Add to list
		push_back(new CAddress(*anAddr));
}

// Add address in sorted order
void CAddressList::push_back_sorted(CAddress* anAddr)
{
	// Must have address
	if (!anAddr)
		return;

	CAddressList::iterator found = end();

	if (mComparator)
		found = std::lower_bound(begin(), end(), (const CAddress*&) anAddr, mComparator->GetComparator());

	insert(found, anAddr);
}

// Find position for address
unsigned long CAddressList::FetchIndexOf(const CAddress* addr) const
{
	// Must have address
	if (!addr)
		return 0;

	CAddressList::const_iterator found = std::find(begin(), end(), addr);

	if (found != end())
	{
		return (found - begin()) + 1;
	}
	else
		return 0;
}

// Find position for sorted insert
unsigned long CAddressList::FetchInsertIndexOf(const CAddress* addr) const
{
	// Must have address
	if (!addr)
		return 0;

	CAddressList::const_iterator found = std::lower_bound(begin(), end(), addr, mComparator->GetComparator());

	if (found != end())
	{
		// Check for exact match with previous item - if so we hit on that
		if ((found != begin()) && (mComparator->GetEquality()(addr, *(found -1)) == 0))
			found--;
		
		return (found - begin()) + 1;
	}
	else
		return 0;
}

// Delete all entries without deleting addresses
void CAddressList::clear_without_delete()
{
	// Do inherited erase
	std::vector<CAddress*>::erase(begin(), end());
}

// Remove matching address (without deleting it)
void CAddressList::RemoveAddress(CAddressList* addrs)
{
	// Must have address
	if (!addrs)
		return;

	for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
		RemoveAddress(*iter);
}

// Remove matching address (without deleting it)
void CAddressList::RemoveAddress(CAddress* addr)
{
	// Must have address
	if (!addr)
		return;

	CAddressList::iterator found = std::find(begin(), end(), addr);
	if (found != end())
		std::vector<CAddress*>::erase(found);
}

// Find duplicate in list
bool CAddressList::IsDuplicate(const CAddress* anAddr) const
{
	// Must have address
	if (!anAddr)
		return false;

	for(CAddressList::const_iterator iter = begin(); iter != end(); iter++)
	{
		// Compare ptrs and addresses
		if ((*iter == anAddr) || (**iter == *anAddr))
			return true;
	}

	return false;

}

// Qualify all unqualified addresses in the list
void CAddressList::QualifyAddresses(const char* qualify_domain)
{
	// Only if domain exists
	if (!qualify_domain || !*qualify_domain)
		return;

	// Qualify all in this list
	for(CAddressList::iterator iter = begin(); iter != end(); iter++)
		(*iter)->QualifyAddress(qualify_domain);

}

// Get first unqualified address
bool CAddressList::GetUnqualifiedAddress(const CAddress*& unqual) const
{
	// Check for any unqualified
	for(CAddressList::const_iterator iter = begin(); iter != end(); iter++)
	{
		// Check for valid host
		if ((*iter)->GetHost().empty())
		{
			unqual = *iter;
			return true;
		}
	}

	// Nothing found
	unqual = NULL;
	return false;

}

// Get first illegal address
bool CAddressList::GetIllegalAddress(const CAddress*& illegal) const
{
	// Check for any unqualified
	for(CAddressList::const_iterator iter = begin(); iter != end(); iter++)
	{
		// Check for valid address
		if (!(*iter)->IsValid())
		{
			illegal = *iter;
			return true;
		}
	}

	// Nothing found
	illegal = NULL;
	return false;

}

// Add email addresses to list
void CAddressList::AddMailAddressToList(cdstrvect& list, bool full) const
{
	for(const_iterator iter = begin(); iter != end(); iter++)
	{
		if (!(*iter)->GetMailAddress().empty())
		{
			const CAdbkAddress* addr = dynamic_cast<const CAdbkAddress*>(*iter);
			if (addr != NULL)
			{
				addr->AddMailAddressToList(list, full);
			}
			else
			{
				if (full)
					list.push_back((*iter)->GetFullAddress());
				else
					list.push_back((*iter)->GetMailAddress());
			}
		}
	}
}

// Add calendar addresses to list
void CAddressList::AddCalendarAddressToList(cdstrvect& list, bool full) const
{
	for(const_iterator iter = begin(); iter != end(); iter++)
	{
		const CAdbkAddress* addr = dynamic_cast<const CAdbkAddress*>(*iter);
		if (addr && (!addr->GetCalendar().empty() || !addr->GetMailAddress().empty()) ||
			!(*iter)->GetMailAddress().empty())
		{
			if (addr != NULL)
			{
				addr->AddCalendarAddressToList(list, full);
			}
			else
			{
				CCalendarAddress caddr((*iter)->GetMailAddress(), (*iter)->GetName());
				if (full)
					list.push_back(caddr.GetFullAddress());
				else
					list.push_back(caddr.GetCalendarAddress());
			}
		}
	}
}

// Write to stream
void CAddressList::WriteToStream(std::ostream& stream) const
{
	// Get text of all addresses
	bool first = true;
	for(CAddressList::const_iterator iter = begin(); iter != end(); iter++)
	{
		if (!first)
			stream.write(os_endl, os_endl_len);
		else
			first = false;
		cdstring txt = (*iter)->GetFullAddress();
		stream.write(txt, txt.length());
	}
}
