/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// Header for CGroup class

#ifndef __CGROUP__MULBERRY__
#define __CGROUP__MULBERRY__

#include "CAdbkAddress.h"
#include "cdstring.h"

#include <vector>

class CAddressList;

class CGroup
{
public:
	// find_if functors
	class same_entry
	{
	public:
		same_entry(const CGroup* data) :
			mData(data) {}

		bool operator() (const CGroup* item) const
			{ return (item && mData) ? (item->GetEntry() == mData->GetEntry()) : false; }
	private:
		const CGroup* mData;
	};

	class same_entry_str
	{
	public:
		same_entry_str(const char* data) :
			mData(data) {}

		bool operator() (const CGroup* item) const
			{ return item ? (item->GetEntry() == mData) : false; }
	private:
		cdstring mData;
	};

	CGroup();
	CGroup(const CGroup &copy);									// Copy constructor
	explicit CGroup(const char* name,							// Construct from names
					 const char* nickname = NULL);

	CGroup& operator=(const CGroup& copy);						// Assignment with same type

	~CGroup();

	int operator==(const char* grp) const;						// Compare with named group
	int operator==(const CGroup& grp) const;					// Compare with another

	bool Search(const cdstring& text, const CAdbkAddress::CAddressFields& fields) const;	// Compare specific field

	void SetEntry(const char* theEntry)								// Set entry
		{ mEntry = theEntry; }
	const cdstring&	GetEntry() const								// Get entry
		{ return mEntry; }

	cdstring&		GetNickName()							// Get nick-name
						{ return mNickName; }
	const cdstring&	GetNickName() const						// Get nick-name
						{ return mNickName; }

	cdstring&		GetName()								// Get name
						{ return mName; }
	const cdstring&	GetName() const							// Get name
						{ return mName; }

	cdstrvect& GetAddressList()								// Get list of addresses
				{ return mAddresses; }
	const cdstrvect& GetAddressList() const					// Get list of addresses
				{ return mAddresses; }

	void	AddAddress(const char* addr);						// Add unique address
	void	AddAddresses(const CAddressList& list, bool full_name = false);				// Add addresses from list

	void	Merge(CGroup* grp);									// Merge existing group with new one

	bool	IsEmpty() const;									// Check whether it has some info

private:
	cdstring			mEntry;								// Entry name
	cdstring			mNickName;							// Group Nick-name
	cdstring			mName;								// Name of this group
	cdstrvect			mAddresses;							// Addresses in this group

};

#endif
