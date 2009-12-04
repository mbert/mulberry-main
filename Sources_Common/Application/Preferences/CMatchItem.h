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


// CMatchItem.h - holds status of Match menu in mailbox window

#ifndef __CMATCHITEM__MULBERRY__
#define __CMATCHITEM__MULBERRY__

#include "CSearchItem.h"

#include "cdstring.h"
#include "templs.h"

class CMessageList;

class CMatchItem
{
public:
	enum EMatchItem
	{
		eNone = 0,
		eNew,
		eUnseen,
		eFlagged,
		eAnswered,
		eDeleted,
		eDraft,
		eSentToday,
		eSentSinceYesterday,
		eSentThisWeek,
		eSentWithin7Days,
		eSentByMe,
		eSentToMe,
		eSentToList,
		eSelectedTo,
		eSelectedFrom,
		eSelectedSmart,
		eSelectedSubject,
		eSelectedDate,
		eMatchLast = eSelectedDate,
		eSelected_First = eSelectedTo,
		eSelected_Last = eSelectedDate
	};

	CMatchItem() : mBitsSet(eMatchLast + 1)
		{ mNegate = false; mUseOr = true; }
	CMatchItem(const CMatchItem& copy) : mBitsSet(copy.mBitsSet), mSearchSet(copy.mSearchSet)
		{ mNegate = copy.mNegate; mUseOr = copy.mUseOr; }
	~CMatchItem() {}

	int operator==(const CMatchItem& other) const				// Compare with same type
		{ return (mNegate == other.mNegate) &&
					(mUseOr == other.mUseOr) &&
					(mBitsSet == other.mBitsSet) &&
					(mSearchSet == other.mSearchSet); }

	CMatchItem& operator=(const CMatchItem& copy)	// Assignment with same type
		{ if (this != &copy)
			{ mNegate = copy.mNegate; mUseOr = copy.mUseOr; mBitsSet = copy.mBitsSet; mSearchSet = copy.mSearchSet; }
		  return *this; }

	void SetNegate(bool negate)
		{ mNegate = negate; }
	bool GetNegate() const
		{ return mNegate; }

	void SetUseOr(bool use_or)
		{ mUseOr = use_or; }
	bool GetUseOr() const
		{ return mUseOr; }

	void SetSingleMatch(EMatchItem item);

	void SetBitsSet(const boolvector& bits_set)
		{ mBitsSet = bits_set; }
	const boolvector& GetBitsSet() const
		{ return mBitsSet; }
	
	void SetSearchSet(const cdstrvect& search_set)
		{ mSearchSet = search_set; }
	const cdstrvect& GetSearchSet() const
		{ return mSearchSet; }

	bool NeedsSelection() const;
	CSearchItem* ConstructSearch(const CMessageList* msgs) const;

	cdstring GetInfo() const;
	bool SetInfo(char_stream& info, NumVersion vers_prefs);
	
private:
	bool				mNegate;
	bool				mUseOr;
	boolvector			mBitsSet;
	cdstrvect			mSearchSet;

	void AddSmartAddressToList(CSearchItemList* list,
							CSearchItem::ESearchType type,
							bool use_not = false) const;

	void AddAddressesToList(CSearchItemList* list,
							const cdstrvect* addrs,
							CSearchItem::ESearchType type,
							bool use_not = false) const;

	CSearchItem* AddAddressFieldToList(const CMessageList* msgs, CSearchItem::ESearchType type) const;
};

#endif
