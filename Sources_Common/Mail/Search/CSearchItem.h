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


// Search criteria data object

// Stores hierarchical search criteria that can be saved or used in searching

#ifndef __CSEARCHITEM__MULBERRY__
#define __CSEARCHITEM__MULBERRY__

#include "cdstring.h"
#include "templs.h"
#include "CMessageFwd.h"

#include <time.h>

// Typedefs
class CSearchItem;

typedef std::vector<CSearchItem*> CSearchItemList;

// CSearchItem: represents a single search criteria

class CSearchItem
{
public:
	enum ESearchType
	{
		// IMAP ops
		eAll,			// -
		eAnd,			// CSearchItemList*
		eAnswered,		// -
		eBcc,			// cdstring*
		eBefore,		// date
		eBody,			// cdstring*
		eCC,			// cdstring*
		eDeleted,		// -
		eDraft,			// -
		eFlagged,		// -
		eFrom,			// cdstring*
		eGroup,			// CSearchItem*
		eHeader,		// cdstrpair*
		eKeyword,		// cdstring*
		eLarger,		// long
		eNew,			// -
		eNot,			// CSearchItem*
		eNumber,		// ulvector*
		eOld,			// -
		eOn,			// date
		eOr,			// CSearchItemList*
		eRecent,		// -
		eSeen,			// -
		eSentBefore,	// date
		eSentOn,		// date
		eSentSince,		// date
		eSince,			// date
		eSmaller,		// long
		eSubject,		// cdstring*
		eText,			// cdstring*
		eTo,			// cdstring*
		eUID,			// ulvector*
		eUnanswered,	// -
		eUndeleted,		// -
		eUndraft,		// -
		eUnflagged,		// -
		eUnkeyword,		// cdstring*
		eUnseen,		// -

		// These are local items that expand to something else on the server
		eRecipient,		// cdstring*
		eCorrespondent,	// cdstring*
		eSender,		// cdstring*
		eNamedStyle,	// cdstring*
		eLabel,			// unsigned long

		// This is used by rules being applied manually
		eSelected,		// -
		
		eLast
	};

	enum ESearchDate
	{
		eToday = 1,
		eSinceYesterday,
		eThisWeek,
		eWithin7Days,
		eThisMonth,
		eThisYear,
		eWithinDays = 0x00010000,
		eWithinWeeks = 0x00020000,
		eWithinMonths = 0x00030000,
		eWithinYears = 0x00040000,
		eWithinMask = 0xFFFF0000,
		eWithinValueMask = 0x0000FFFF
	};

	enum ESearchMatchType
	{
		eDefault,
		eIs,
		eContains,
		eStartsWith,
		eEndsWith,
		eMatches,
		eExists
	};

	CSearchItem()
		{ mType = eAll; mMatchType = eDefault; mData = NULL; }
	CSearchItem(ESearchType item, ESearchMatchType match = eDefault)
		{ mType = item; mMatchType = match; mData = NULL; }
	CSearchItem(ESearchType item, const cdstring& text, ESearchMatchType match = eDefault)
		{ mType = item; mMatchType = match; mData = new cdstring(text); }
	CSearchItem(ESearchType item, const cdstring& text1, const cdstring& text2, ESearchMatchType match = eDefault)
		{ mType = item; mMatchType = match; mData = new cdstrpair(text1, text2); }
	CSearchItem(ESearchType item, unsigned long date, ESearchMatchType match = eDefault)
		{ mType = item; mMatchType = match; mData = (void*) date; }
	CSearchItem(ESearchType item, long size, ESearchMatchType match = eDefault)
		{ mType = item; mMatchType = match; mData = (void*) size; }
	CSearchItem(ESearchType item, CSearchItem* subitem, ESearchMatchType match = eDefault)
		{ mType = item; mMatchType = match; mData = subitem; }
	CSearchItem(ESearchType item, CSearchItemList* itemlist, ESearchMatchType match = eDefault)
		{ mType = item; mMatchType = match; mData = itemlist; }
	CSearchItem(ESearchType item, const ulvector& set, ESearchMatchType match = eDefault)
		{ mType = item; mMatchType = match; mData = new ulvector(set); }
	CSearchItem(const CSearchItem& copy)
		{ _copy(copy); }
	CSearchItem(NMessage::EFlags set_flag, NMessage::EFlags unset_flag, ESearchMatchType match = eDefault);	// Construct from set of message flags

	
	~CSearchItem()
		{ _tidy(); }

	CSearchItem& operator=(const CSearchItem& copy)							// Assignment with same type
		{ if (this != &copy) { _tidy(); _copy(copy); } return *this; }

	void clear()															// Clear out existing data
		{ _tidy(); }

	cdstring GetInfo(void) const;											// Get text expansion for prefs
	void SetInfo(char_stream& info);										// Convert text to items

	ESearchType GetType() const												// Get type of item
		{ return mType; }
	void SetType(ESearchType type)											// Set type of item
		{ mType = type; }

	ESearchMatchType GetMatchType() const									// Get type of match
		{ return mMatchType; }
	void SetType(ESearchMatchType match)									// Set type of item
		{ mMatchType = match; }

	const void* GetData() const												// Get data
		{ return mData; }

	bool IsMultiple(bool expand_me) const;									// Will it expand to more than one?

	void GenerateItems(cdstrboolvect& items, bool expand_me = true) const;	// Generate list of expanded text items
	cdstring GenerateDate(bool expand_me = true) const;												// Generate date string
	time_t ResolveDate() const;												// Get the actual specified date

	void GenerateSIEVEScript(std::ostream& out) const;

protected:
	ESearchType			mType;
	ESearchMatchType	mMatchType;
	void*				mData;

	void GenerateAddress(cdstrboolvect& items, const char* search_key1,				// Generate list of expanded address items
							const char* search_key2, bool expand_me = true) const;
	void AddSmartAddressToList(CSearchItemList* list, ESearchType type) const;		// Add smart address items to list

	CSearchItem* ParseItem(char_stream& txt, bool convert);							// Parse single item from stream
	CSearchItem* ParseAddress(char_stream& txt, ESearchType type,
								ESearchMatchType match, bool convert);				// Parse search item address
	unsigned long ParseDate(char_stream& txt);										// Parse search item date
	ulvector ParseSequence(char_stream& txt);										// Parse search item sequence

	void GenerateAddressSIEVE(std::ostream& out, const char* hdr) const;					// Generate list of expanded address items
	void GenerateSIEVEMatchType(std::ostream& out) const;								// Generate match type
	void GenerateSIEVEKey(cdstring& key) const;										// Generate key prefix
	void GenerateSIEVENumber(std::ostream& out) const;									// Generate number

private:
	void _copy(const CSearchItem& copy);
	void _tidy();
};

#endif
