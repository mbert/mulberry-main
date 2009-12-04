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


// Header for Address class

#ifndef __CADDRESSLIST__MULBERRY__
#define __CADDRESSLIST__MULBERRY__

#include "cdstring.h"
#include "ptrvector.h"

// Classes
class CAddress;
class CAddressComparator;

class CAddressList : public ptrvector<CAddress>
{
public:
	explicit CAddressList(unsigned long capacity = 0);
	CAddressList(const CAddressList& copy,
					unsigned long capacity = 0);						// Copy constructor
	explicit CAddressList(const char* txt,
					long txt_length,
					unsigned long reserve = 0,
					bool resolve = true);								// Constructor from delimited text list
	~CAddressList();

	void	SetComparator(CAddressComparator* comp);
	void 	Sort();											// Force resort

	void	MergeList(const CAddressList* addrs);
	void	SubtractList(const CAddressList* addrs);
	void	MakeUnique();									// Ensure only unique items

	bool	InsertTextAddress(const char* txt, unsigned long level);	// Insert text address & resolve nick-names
	bool	InsertUniqueItem(CAddress* anAddr);							// Insert non-duplicates
	void	AddUnique(const CAddress* anAddr);							// Add unique address
	void	push_back_sorted(CAddress* anAddr);							// Add address in sorted order

	unsigned long	FetchIndexOf(const CAddress* addr) const;			// Find position for address
	unsigned long	FetchInsertIndexOf(const CAddress* addr) const;		// Find position for sorted insert

	void	clear_without_delete();									// Delete all entries without deleting addresses

	void	RemoveAddress(CAddressList* addrs);						// Removing matching addresses
	void	RemoveAddress(CAddress* addr);							// Removing matching address

	bool	IsDuplicate(const CAddress* anAddr) const;				// Look for duplicate in list

	void	QualifyAddresses(const char* qualify_domain);			// Qualify all unqualified addresses in the list
	bool	GetUnqualifiedAddress(const CAddress*& unqual) const;	// Get first unqualified address

	bool	GetIllegalAddress(const CAddress*& unqual) const;		// Get first illegal address

	void	AddMailAddressToList(cdstrvect& list,					// Add email addresses to list
								 bool full) const;
	void	AddCalendarAddressToList(cdstrvect& list,				// Add calendar addresses to list
								 bool full) const;
	void	WriteToStream(std::ostream& stream) const;					// Write to stream

private:
	CAddressComparator*	mComparator;

	void	InitAddressList(unsigned long capacity);
};

#endif
