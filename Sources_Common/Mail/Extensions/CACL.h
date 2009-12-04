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


// ACL Support

#ifndef __CACL__MULBERRY__
#define __CACL__MULBERRY__

#include "cdstring.h"

// Typedefs
class CACL;
typedef std::vector<CACL> CACLList;

struct SACLRight;
typedef std::pair<cdstring, SACLRight> SACLStyle;
typedef std::vector<SACLStyle> SACLStyleList;

// SACLRight: the base struct for the rights object
// This should be used where 'raw' rights are required
// e.g. 'MYRIGHTS' embedded in an object (e.g. mailboxes)

struct SACLRight
{
	enum
	{
		eACL_NoRights = 0L,
		eACL_AllRights = -1L
	};

	long mRights;
	
	SACLRight()
		{ mRights = eACL_AllRights; }				// Default to start with all rights available
	explicit SACLRight(long rights)
		{ mRights = rights; }						// Start with specific rights
	int operator==(const SACLRight& test) const		// Compare with same type
		{ return (mRights == test.mRights); }

	void	MergeRights(SACLRight merge)
	{
		mRights |= merge.mRights;
	}
	void	SetRight(long right, bool set)			// Set rights flag on/off
		{ if (set) mRights |= right; else mRights &= ~right; }
	bool	HasRight(long right) const				// Check rights flag
		{ return (mRights & right) != 0; }
};


// CACL: contains specific ACL item
// This is an abstract base class that should be derived for different types of ACL models
// e.g. for mailboxes, address books, preferences etc

class CACL
{
public:
	CACL() {}
	CACL(const CACL& copy);						// Copy construct

	virtual 		~CACL() {}
	
	CACL& operator=(const CACL& copy);			// Assignment with same type
	int operator==(const CACL& test) const		// Compare with same type
		{ return (mUID == test.GetUID()); }		// Just compare uids

	virtual void	SetUID(const char* uid)
		{ mUID = uid; }
	virtual const cdstring&	GetUID() const
		{ return mUID; }
	virtual const cdstring&	GetDisplayUID() const
		{ return mUID; }
	
	virtual void	SetRights(const char* rights)
		{ ParseRights(rights, mCurrent); }
	virtual void	SetRights(SACLRight rights)
		{ mCurrent = rights; }
	virtual void	MergeRights(SACLRight merge)
		{ mCurrent.MergeRights(merge); }
	virtual SACLRight	GetRights() const
		{ return mCurrent; }

	virtual void	SetAllowedRights(const char* rights)
		{ ParseRights(rights, mAllowed); }
	virtual void	SetAllowedRights(SACLRight rights)
		{ mAllowed = rights; }
	virtual SACLRight	GetAllowedRights() const
		{ return mAllowed; }

	virtual cdstring	GetTextRights() const = 0;		// Get text form of rights
	virtual cdstring	GetFullTextRights() const = 0;	// Get full text form of rights

	virtual void	SetRight(long right, bool set)			// Set rights flag on/off
		{ mCurrent.SetRight(right, set); }
	virtual bool	HasRight(long right) const				// Check rights flag
		{ return mCurrent.HasRight(right); }
	
	virtual void	SetAllowedRight(long right, bool set)	// Set allowed rights flag on/off
		{ mAllowed.SetRight(right, set); }
	virtual bool	HasAllowedRight(long right) const		// Check allowed rights
		{ return mAllowed.HasRight(right); }
	
protected:
	cdstring	mUID;
	SACLRight	mCurrent;
	SACLRight	mAllowed;

private:
	virtual void	ParseRights(const char* txt,
									SACLRight& rights) = 0;	// Parse string to specified rigths location
};

#endif
