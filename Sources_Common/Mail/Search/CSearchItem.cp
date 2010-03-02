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


// Search criteria data object

// Stores hierarchical search criteria that can be saved or used in searching

#include "CSearchItem.h"

#include "char_stream.h"
#include "CIMAPCommon.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CSequence.h"

#include <ctype.h>
#include <ctime>
#include <memory>
#include <stdio.h>

const unsigned long cDaySeconds = 60L*60L*24L;
const unsigned long cWeekSeconds = 7L * cDaySeconds;
const unsigned long cYearSeconds = 365L * cDaySeconds;

const char* cSEARCH_LABEL = "MULBERRY-LABEL";
const char* cSEARCH_NAMEDSTYLE = "STYLE";
const char* cSEARCH_RECIPIENT = "RECIPIENT";
const char* cSEARCH_CORRESPONDENT = "CORRESPONDENT";
const char* cSEARCH_SENDER = "SENDER";
const char* cSEARCH_SELECTED = "MULBERRY-SELECTED";

const char* cSEARCH_LPAREN = "LPAREN";
const char* cSEARCH_RPAREN = "RPAREN";

const char* cSEARCH_IS = "IS";
const char* cSEARCH_CONTAINS = "CONTAINS";
const char* cSEARCH_STARTSWITH = "STARTSWITH";
const char* cSEARCH_ENDSWITH = "ENDSWITH";
const char* cSEARCH_MATCHES = "MATCHES";
const char* cSEARCH_EXISTS = "EXISTS";

// Construct from set of message flags
CSearchItem::CSearchItem(NMessage::EFlags set_flag, NMessage::EFlags unset_flag, ESearchMatchType match)
{
	mType = eAll;
	mMatchType = match;
	mData = NULL;

	// Check for each possible flag
	CSearchItemList* and_list = new CSearchItemList;
	if (set_flag & NMessage::eRecent)
		and_list->push_back(new CSearchItem(eRecent));
	else if (unset_flag & NMessage::eRecent)
		and_list->push_back(new CSearchItem(eOld));
	if (set_flag & NMessage::eAnswered)
		and_list->push_back(new CSearchItem(eAnswered));
	else if (unset_flag & NMessage::eAnswered)
		and_list->push_back(new CSearchItem(eUnanswered));
	if (set_flag & NMessage::eFlagged)
		and_list->push_back(new CSearchItem(eFlagged));
	else if (unset_flag & NMessage::eFlagged)
		and_list->push_back(new CSearchItem(eUnflagged));
	if (set_flag & NMessage::eDeleted)
		and_list->push_back(new CSearchItem(eDeleted));
	else if (unset_flag & NMessage::eDeleted)
		and_list->push_back(new CSearchItem(eUndeleted));
	if (set_flag & NMessage::eSeen)
		and_list->push_back(new CSearchItem(eSeen));
	else if (unset_flag & NMessage::eSeen)
		and_list->push_back(new CSearchItem(eUnseen));
	if (set_flag & NMessage::eDraft)
		and_list->push_back(new CSearchItem(eDraft));
	else if (unset_flag & NMessage::eDraft)
		and_list->push_back(new CSearchItem(eUndraft));
		
	// Check for keywords
	for(unsigned long i = 0; i < NMessage::eMaxLabels; i++)
	{
		if (set_flag & (NMessage::eLabel1 << i))
			and_list->push_back(new CSearchItem(eLabel, i));
		else if (unset_flag & (NMessage::eLabel1 << i))
			and_list->push_back(new CSearchItem(eNot, new CSearchItem(eLabel, i)));
	}

	// See if more than one item specified
	if (and_list->size() > 1)
	{
		// Turn this into an And operation
		mType = eAnd;
		mData = and_list;
	}
	else if (and_list->size())
	{
		// Turn this into the first item
		_copy(*and_list->front());
		delete and_list->front();
		delete and_list;
	}
	else
		delete and_list;
}

void CSearchItem::_copy(const CSearchItem& copy)
{
	mType = copy.mType;
	mMatchType = copy.mMatchType;

	switch(mType)
	{
	case eAnd:
	case eOr:
	  {
		// Copy items in list
		CSearchItemList* old_items = reinterpret_cast<CSearchItemList*>(copy.mData);
		CSearchItemList* new_items = new CSearchItemList;
		mData = new_items;
		if (old_items)
		{
			for(CSearchItemList::const_iterator iter = old_items->begin(); iter != old_items->end(); iter++)
				new_items->push_back(new CSearchItem(**iter));
		}
		break;
	  }	

	case eBcc:
	case eCC:
	case eFrom:
	case eTo:
	case eRecipient:
	case eCorrespondent:
	case eSender:
		// NB Address item may be empty if referring to user's address
		mData = (copy.mData ? new cdstring(*reinterpret_cast<cdstring*>(copy.mData)) : NULL);
		break;

	case eBody:
	case eKeyword:
	case eSubject:
	case eText:
	case eUnkeyword:
	case eNamedStyle:
		mData = (copy.mData ? new cdstring(*reinterpret_cast<cdstring*>(copy.mData)) : NULL);
		break;

	case eHeader:
		mData = (copy.mData ? new cdstrpair(*reinterpret_cast<cdstrpair*>(copy.mData)) : NULL);
		break;

	case eNot:
		mData = (copy.mData ? new CSearchItem(*reinterpret_cast<CSearchItem*>(copy.mData)) : NULL);
		break;

	case eNumber:
	case eUID:
		mData = (copy.mData ? new ulvector(*reinterpret_cast<ulvector*>(copy.mData)) : NULL);
		break;

	default:
		// Just copy data
		mData = copy.mData;
	}
}

void CSearchItem::_tidy()
{
	// Only delete real objects
	switch(mType)
	{
	case eAnd:
	case eOr:
	  {
		CSearchItemList* items = reinterpret_cast<CSearchItemList*>(mData);
		if (items)
		{
			for(CSearchItemList::iterator iter = items->begin(); iter != items->end(); iter++)
				delete *iter;
			delete items;
		}
		break;
	  }	

	case eBcc:
	case eBody:
	case eCC:
	case eFrom:
	case eKeyword:
	case eSubject:
	case eText:
	case eTo:
	case eUnkeyword:
	case eRecipient:
	case eCorrespondent:
	case eSender:
	case eNamedStyle:
		delete reinterpret_cast<cdstring*>(mData);
		break;

	case eHeader:
		delete reinterpret_cast<cdstrpair*>(mData);
		break;

	case eNot:
		delete reinterpret_cast<CSearchItem*>(mData);
		break;

	case eNumber:
	case eUID:
		delete reinterpret_cast<ulvector*>(mData);
		break;

	default:;
	}

	mType = eAll;
	mMatchType = eDefault;
	mData = NULL;
}

bool CSearchItem::IsMultiple(bool expand_me) const
{
	switch(mType)
	{
	case eAnd:
	case eOr:
		// Always true - these should never contain a single item in their lists
		return true;

	case eBcc:
	case eCC:
	case eFrom:
	case eTo:
	case eRecipient:
	case eCorrespondent:
	case eSender:
		// Need to check for smart addresses
		if (!expand_me || GetData())
			// Single address
			return false;
		else
			// If any smarts => multiple items
			return (CPreferences::sPrefs->mSmartAddressList.GetValue().size() > 0);

	case eNumber:
	case eUID:
		// Check size of lists
	  	return mData && (reinterpret_cast<const ulvector*>(mData)->size() > 1);

	default:
		return false;
	}
}

void CSearchItem::GenerateItems(cdstrboolvect& items, bool expand_me) const
{
	cdstring temp;

	// Add comparitor first
	switch(GetMatchType())
	{
	case eDefault:
		// Add nothing - this is the old default
		break;
	case eIs:
		items.push_back(cdstrbool(cSEARCH_IS, true));
		break;
	case eContains:
		items.push_back(cdstrbool(cSEARCH_CONTAINS, true));
		break;
	case eStartsWith:
		items.push_back(cdstrbool(cSEARCH_STARTSWITH, true));
		break;
	case eEndsWith:
		items.push_back(cdstrbool(cSEARCH_ENDSWITH, true));
		break;
	case eMatches:
		items.push_back(cdstrbool(cSEARCH_MATCHES, true));
		break;
	case eExists:
		items.push_back(cdstrbool(cSEARCH_EXISTS, true));
		break;
	}

	// Now do criteria
	switch(GetType())
	{

	case eAll:			// -
		items.push_back(cdstrbool(cSEARCH_ALL, true));
		break;

	case eAnd:			// CSearchItemList*
		// Just push all onto command line
		{
			const CSearchItemList* sub_items = reinterpret_cast<const CSearchItemList*>(GetData());
			if (sub_items)
			{
				for(CSearchItemList::const_iterator iter = sub_items->begin(); iter != sub_items->end(); iter++)
				{
					// May need to be grouped
					if (expand_me && (*iter)->IsMultiple(expand_me) && ((*iter)->GetType() != eAnd))
						items.push_back(cdstrbool("(", false));
					(*iter)->GenerateItems(items, expand_me);
					if (expand_me && (*iter)->IsMultiple(expand_me) && ((*iter)->GetType() != eAnd))
						items.push_back(cdstrbool(")", false));
				}
			}
		}
		break;

	case eAnswered:	// -
		items.push_back(cdstrbool(cSEARCH_ANSWERED, true));
		break;

	case eBcc:			// cdstring*
		GenerateAddress(items, cSEARCH_BCC, NULL, expand_me);
		break;

	case eBefore:		// date
		items.push_back(cdstrbool(cSEARCH_BEFORE, true));
		items.push_back(cdstrbool(GenerateDate(expand_me), true));
		break;

	case eBody:		// cdstring*
		items.push_back(cdstrbool(cSEARCH_BODY, true));
		temp = (GetData() ? *reinterpret_cast<const cdstring*>(GetData()) : cdstring::null_str);
		items.push_back(cdstrbool(temp, true));
		break;

	case eCC:			// cdstring*
		GenerateAddress(items, cSEARCH_CC, NULL, expand_me);
		break;

	case eDeleted:		// -
		items.push_back(cdstrbool(cSEARCH_DELETED, true));
		break;

	case eDraft:		// -
		items.push_back(cdstrbool(cSEARCH_DRAFT, true));
		break;

	case eFlagged:		// -
		items.push_back(cdstrbool(cSEARCH_FLAGGED, true));
		break;

	case eFrom:		// cdstring*
		GenerateAddress(items, cSEARCH_FROM, NULL, expand_me);
		break;

	case eGroup:	// CSearchItem*
		// Just push all onto command line
		{
			const CSearchItem* sub_item = reinterpret_cast<const CSearchItem*>(GetData());
			if (sub_item)
			{
				// Parenthesised list of items
				items.push_back(cdstrbool(expand_me ? "(" : cSEARCH_LPAREN, expand_me ? false : true));
				sub_item->GenerateItems(items, expand_me);
				items.push_back(cdstrbool(expand_me ? ")" : cSEARCH_RPAREN, expand_me ? false : true));
			}
		}
		break;

	case eHeader:		// cdstrpair*
		items.push_back(cdstrbool(cSEARCH_HEADER, true));
		temp = (GetData() ? reinterpret_cast<const cdstrpair*>(GetData())->first : cdstring::null_str);
		items.push_back(cdstrbool(temp, true));
		temp = (GetData() ? reinterpret_cast<const cdstrpair*>(GetData())->second : cdstring::null_str);
		items.push_back(cdstrbool(temp, true));
		break;

	case eKeyword:		// cdstring*
		items.push_back(cdstrbool(cSEARCH_KEYWORD, true));
		temp = (GetData() ? *reinterpret_cast<const cdstring*>(GetData()) : cdstring::null_str);
		items.push_back(cdstrbool(temp, true));
		break;

	case eLabel:		// unsigned long
	{
		unsigned long index = reinterpret_cast<long>(GetData());
		if (index < NMessage::eMaxLabels)
		{
			if (expand_me)
			{
				items.push_back(cdstrbool(cSEARCH_KEYWORD, true));
				temp = CPreferences::sPrefs->mIMAPLabels.GetValue()[index];
				items.push_back(cdstrbool(temp, true));
			}
			else
			{
				items.push_back(cdstrbool(cSEARCH_LABEL, true));
				temp = cdstring(index);
				items.push_back(cdstrbool(temp, true));
			}
		}
		break;
	}

	case eLarger:		// long
		items.push_back(cdstrbool(cSEARCH_LARGER, true));
		temp = reinterpret_cast<long>(GetData());
		//temp = long(GetData());
		items.push_back(cdstrbool(temp, true));
		break;

	case eNew:			// -
		items.push_back(cdstrbool(cSEARCH_NEW, true));
		break;

	case eNot:			// CSearchItem*
	  {
		items.push_back(cdstrbool(cSEARCH_NOT, true));

		// May need to be grouped
		int no_space_index = 0;
		if (GetData())
		{
			if (reinterpret_cast<const CSearchItem*>(GetData())->IsMultiple(expand_me))
				items.push_back(cdstrbool("(", false));
			reinterpret_cast<const CSearchItem*>(GetData())->GenerateItems(items, expand_me);
			if (reinterpret_cast<const CSearchItem*>(GetData())->IsMultiple(expand_me))
				items.push_back(cdstrbool(")", false));
		}
		break;
	  }
	case eNumber:		// ulvector* only - no key
		{
			// Convert to sequence text (set processing of text to false as the sequence
			// text will be a valid atom string)
			const ulvector* ulv = reinterpret_cast<const ulvector*>(GetData());
			if (ulv)
			{
				CSequence sequence(*ulv);
				items.push_back(cdstrbool(sequence.GetSequenceText(), false));
			}
		}
		break;

	case eOld:			// -
		items.push_back(cdstrbool(cSEARCH_OLD, true));
		break;

	case eOn:			// date
		items.push_back(cdstrbool(cSEARCH_ON, true));
		items.push_back(cdstrbool(GenerateDate(expand_me), true));
		break;

	case eOr:			// CSearchItemList*
		// Just push all onto command line
		{
			const CSearchItemList* sub_items = reinterpret_cast<const CSearchItemList*>(GetData());
			if (sub_items)
			{
				unsigned long remaining = sub_items->size();
				for(CSearchItemList::const_iterator iter = sub_items->begin(); iter != sub_items->end(); iter++, remaining--)
				{
					// Add OR key if two or more remain
					if (remaining > 1)
						items.push_back(cdstrbool(cSEARCH_OR, true));

					// May need to be grouped
					if ((*iter)->IsMultiple(expand_me))
						items.push_back(cdstrbool("(", false));
					(*iter)->GenerateItems(items, expand_me);
					if ((*iter)->IsMultiple(expand_me))
						items.push_back(cdstrbool(")", false));
				}
			}
		}
		break;

	case eRecent:		// -
		items.push_back(cdstrbool(cSEARCH_RECENT, true));
		break;

	case eSeen:		// -
		items.push_back(cdstrbool(cSEARCH_SEEN, true));
		break;

	case eSelected:			// -
		items.push_back(cdstrbool(cSEARCH_SELECTED, true));
		break;

	case eSentBefore:	// date
		items.push_back(cdstrbool(cSEARCH_SENTBEFORE, true));
		items.push_back(cdstrbool(GenerateDate(expand_me), true));
		break;

	case eSentOn:		// date
		items.push_back(cdstrbool(cSEARCH_SENTON, true));
		items.push_back(cdstrbool(GenerateDate(expand_me), true));
		break;

	case eSentSince:	// date
		items.push_back(cdstrbool(cSEARCH_SENTSINCE, true));
		items.push_back(cdstrbool(GenerateDate(expand_me), true));
		break;

	case eSince:		// date
		items.push_back(cdstrbool(cSEARCH_SINCE, true));
		items.push_back(cdstrbool(GenerateDate(expand_me), true));
		break;

	case eSmaller:		// long
		items.push_back(cdstrbool(cSEARCH_SMALLER, true));
		temp = reinterpret_cast<long>(GetData());
		//temp = long(GetData());
		items.push_back(cdstrbool(temp, true));
		break;

	case eSubject:		// cdstring*
		items.push_back(cdstrbool(cSEARCH_SUBJECT, true));
		temp = (GetData() ? *reinterpret_cast<const cdstring*>(GetData()) : cdstring::null_str);
		items.push_back(cdstrbool(temp, true));
		break;

	case eText:		// cdstring*
		items.push_back(cdstrbool(cSEARCH_TEXT, true));
		temp = (GetData() ? *reinterpret_cast<const cdstring*>(GetData()) : cdstring::null_str);
		items.push_back(cdstrbool(temp, true));
		break;

	case eTo:			// cdstring*
		GenerateAddress(items, cSEARCH_TO, NULL, expand_me);
		break;

	case eUID:			// ulvector*
		items.push_back(cdstrbool(cSEARCH_UID, true));
		{
			// Convert to sequence text (set processing of text to false as the sequence
			// text will be a valid atom string)
			const ulvector* ulv = reinterpret_cast<const ulvector*>(GetData());
			if (ulv)
			{
				CSequence sequence(*ulv);
				items.push_back(cdstrbool(sequence.GetSequenceText(), false));
			}
		}
		break;

	case eUnanswered:	// -
		items.push_back(cdstrbool(cSEARCH_UNANSWERED, true));
		break;

	case eUndeleted:	// -
		items.push_back(cdstrbool(cSEARCH_UNDELETED, true));
		break;

	case eUndraft:		// -
		items.push_back(cdstrbool(cSEARCH_UNDRAFT, true));
		break;

	case eUnflagged:	// -
		items.push_back(cdstrbool(cSEARCH_UNFLAGGED, true));
		break;

	case eUnkeyword:		// cdstring*
		items.push_back(cdstrbool(cSEARCH_UNKEYWORD, true));
		temp = (GetData() ? *reinterpret_cast<const cdstring*>(GetData()) : cdstring::null_str);
		items.push_back(cdstrbool(temp, true));
		break;

	case eUnseen:		// -
		items.push_back(cdstrbool(cSEARCH_UNSEEN, true));
		break;

	case eRecipient:	// cdstring*
		if (expand_me)
		{
			// Create OR list and push into OR spec
			CSearchItemList* temp_list = new CSearchItemList;
			CSearchItem* temp_item = new CSearchItem(eOr, temp_list);

			// Add a To, CC, Bcc
			temp_list->push_back(new CSearchItem(*this));
			temp_list->back()->SetType(eTo);
			temp_list->push_back(new CSearchItem(*this));
			temp_list->back()->SetType(eCC);
			temp_list->push_back(new CSearchItem(*this));
			temp_list->back()->SetType(eBcc);

			// Now force temp item to write itself to list
			temp_item->GenerateItems(items, expand_me);

			// delete all temp items
			delete temp_item;
		}
		else
		{
			items.push_back(cdstrbool(cSEARCH_RECIPIENT, true));
			temp = (GetData() ? *reinterpret_cast<const cdstring*>(GetData()) : cdstring::null_str);
			items.push_back(cdstrbool(temp, true));
		}
		break;

	case eCorrespondent:	// cdstring*
		if (expand_me)
		{
			// Create OR list and push into OR spec
			CSearchItemList* temp_list = new CSearchItemList;
			CSearchItem* temp_item = new CSearchItem(eOr, temp_list);

			// Add a To, CC, Bcc
			temp_list->push_back(new CSearchItem(*this));
			temp_list->back()->SetType(eTo);
			temp_list->push_back(new CSearchItem(*this));
			temp_list->back()->SetType(eCC);
			temp_list->push_back(new CSearchItem(*this));
			temp_list->back()->SetType(eBcc);
			temp_list->push_back(new CSearchItem(*this));
			temp_list->back()->SetType(eFrom);

			// Now force temp item to write itself to list
			temp_item->GenerateItems(items, expand_me);

			// delete all temp items
			delete temp_item;
		}
		else
		{
			items.push_back(cdstrbool(cSEARCH_CORRESPONDENT, true));
			temp = (GetData() ? *reinterpret_cast<const cdstring*>(GetData()) : cdstring::null_str);
			items.push_back(cdstrbool(temp, true));
		}
		break;

	case eSender:	// cdstring*
		if (expand_me)
			GenerateAddress(items, cSEARCH_HEADER, cSEARCH_SENDER, expand_me);
		else
			GenerateAddress(items, cSEARCH_SENDER, NULL, expand_me);
		break;

	case eNamedStyle:	// cdstring*
		if (expand_me)
		{
			// Get named style from prefs
			temp = (GetData() ? *reinterpret_cast<const cdstring*>(GetData()) : cdstring::null_str);
			const CSearchStyle* style = CPreferences::sPrefs->mSearchStyles.GetValue().FindStyle(temp);

			// Add style to this list
			if (style)
			{
				const CSearchItem* spec = style->GetSearchItem();

				// May need to be grouped
				if (spec->IsMultiple(expand_me))
					items.push_back(cdstrbool("(", false));
				spec->GenerateItems(items, expand_me);
				if (spec->IsMultiple(expand_me))
					items.push_back(cdstrbool(")", false));
			}
			else
				// Must add something even if style is missing
				items.push_back(cdstrbool(cSEARCH_ALL, true));
		}
		else
		{
			items.push_back(cdstrbool(cSEARCH_NAMEDSTYLE, true));
			temp = (GetData() ? *reinterpret_cast<const cdstring*>(GetData()) : cdstring::null_str);
			items.push_back(cdstrbool(temp, true));
		}
		break;
	default:;
	}
}

void CSearchItem::GenerateAddress(cdstrboolvect& items, const char* search_key1, const char* search_key2, bool expand_me) const
{
	// If its empty that means use 'me' as the address
	if (GetData() == NULL)
	{
		// Expand into all possible aliases if requested
		if (expand_me)
		{
			// Are there any smart addresses?
			if (CPreferences::sPrefs->mSmartAddressList.GetValue().size())
			{
				// Create OR list and push into OR spec
				CSearchItemList* temp_list = new CSearchItemList;
				CSearchItem* temp_item = new CSearchItem(eOr, temp_list);

				// Add all smart addresses
				AddSmartAddressToList(temp_list, GetType());

				// Now force temp item to write itself to list
				temp_item->GenerateItems(items, expand_me);

				// delete all temp items
				delete temp_item;
			}
			else
			{
				// Get default identity
				cdstring addr_txt = CPreferences::sPrefs->mIdentities.GetValue().front().GetFrom();
				CAddress addr(addr_txt);

				// Just a single text address
				items.push_back(cdstrbool(search_key1, true));
				if (search_key2)
					items.push_back(cdstrbool(search_key2, true));
				items.push_back(cdstrbool(addr.GetMailAddress(), true));
			}
		}
		else
		{
			// Write out empty string
			items.push_back(cdstrbool(search_key1, true));
			if (search_key2)
				items.push_back(cdstrbool(search_key2, true));
			items.push_back(cdstrbool(cdstring::null_str, true));
		}
	}
	else
	{
		// Just a single text address
		items.push_back(cdstrbool(search_key1, true));
		if (search_key2)
			items.push_back(cdstrbool(search_key2, true));
		cdstring temp = *reinterpret_cast<const cdstring*>(GetData());
		items.push_back(cdstrbool(temp, true));
	}
}

void CSearchItem::AddSmartAddressToList(CSearchItemList* list, ESearchType type) const
{
	// Do default identity
	cdstring addr_txt = CPreferences::sPrefs->mIdentities.GetValue().front().GetFrom();
	CAddress addr(addr_txt);
	list->push_back(new CSearchItem(type, addr.GetMailAddress()));

	// Now do each smart address
	const cdstrvect& addrs = CPreferences::sPrefs->mSmartAddressList.GetValue();
	for(cdstrvect::const_iterator iter = addrs.begin(); iter != addrs.end(); iter++)
		list->push_back(new CSearchItem(type, *iter));
}

static const char *search_month_name[]
	= {"Jan", "Feb", "Mar","Apr","May","Jun",
	   "Jul", "Aug",  "Sep", "Oct", "Nov","Dec"};

cdstring CSearchItem::GenerateDate(bool expand_me) const
{
	// Do special things for special dates
	if (!expand_me)
	{
		// Get date value
	  	time_t date = reinterpret_cast<time_t>(GetData());
	  	//time_t date = (time_t)(GetData());
		switch(date)
		{
		case eToday:
		case eSinceYesterday:
		case eThisWeek:
		case eWithin7Days:
		case eThisMonth:
		case eThisYear:
			{
				// Just write out numeric value
				cdstring temp = (unsigned long)date;
				return temp;
			}
		default:;
		}
		
		switch(date & eWithinMask)
		{
		case eWithinDays:
		case eWithinWeeks:
		case eWithinMonths:
		case eWithinYears:
			{
				// Just write out numeric value
				cdstring temp = (long)date;
				return temp;
			}
		default:;
		}
	}

	time_t date = ResolveDate();

	// Turn into string (without localised month name)
	struct tm* date_spec = ::localtime(&date);
	cdstring temp;
	temp.reserve(32);
	::strftime(temp.c_str_mod(), 31, "%d-%%s-%Y", date_spec);

	// Add in c-locale month name
	cdstring date_str;
	date_str.reserve(32);
	::snprintf(date_str.c_str_mod(), 32, temp.c_str(), search_month_name[date_spec->tm_mon]);

	return date_str;
}

// Get the actual specified date
time_t CSearchItem::ResolveDate() const
{
	// Get date value
  	time_t date = reinterpret_cast<time_t>(GetData());
  	//time_t  date = time_t(GetData());
		
	// Process relative dates to absolute dates
	struct tm* ltime;
	switch(date)
	{
	case eToday:
		date = ::time(NULL);
		break;
	case eSinceYesterday:
		date = ::time(NULL);
		// Subtract one days worth of seconds
		date -= cDaySeconds;
		break;
	case eThisWeek:
		date = ::time(NULL);
		ltime = ::localtime(&date);
		// Subtract (today - Sunday) days worth of seconds
		date -= cDaySeconds * ltime->tm_wday;
		break;
	case eWithin7Days:
		date = ::time(NULL);
		// Subtract seven days worth of seconds
		date -= cDaySeconds * 7;
		break;
	case eThisMonth:
		date = ::time(NULL);
		ltime = ::localtime(&date);
		// Subtract (today - 1st day of month) days worth of seconds
		date -= cDaySeconds * (ltime->tm_mday  - 1);
		break;
	case eThisYear:
		date = ::time(NULL);
		ltime = ::localtime(&date);
		// Subtract (today - 1st day of year) days worth of seconds
		date -= cDaySeconds * ltime->tm_yday;
		break;
	default:;
	}
	unsigned long within = date & eWithinValueMask;
	switch(date & eWithinMask)
	{
	case eWithinDays:
		date = ::time(NULL);
		// Subtract 'within' days worth of seconds
		date -= cDaySeconds * within;
		break;
	case eWithinWeeks:
		date = ::time(NULL);
		// Subtract 'within' weeks worth of seconds
		date -= cWeekSeconds * within;
		break;
	case eWithinMonths:
		{
			date = ::time(NULL);
			ltime = ::localtime(&date);
			
			int original_month = ltime->tm_mon;
			bool at_month_end = false;
			switch(ltime->tm_mon)
			{
			case 0: // Jan
			case 2: // Mar
			case 4: // May
			case 6: // July
			case 7: // August
			case 9: // October
			case 11: // December
				// These are 31 day months
				at_month_end = (ltime->tm_mday == 31);
				break;
			case 3: // April
			case 5: // June
			case 8: // September
			case 10: // November
				// These are 30 day months
				at_month_end = (ltime->tm_mday == 30);
				break;
			case 1: // February
				// These are 28/29 day months
				if (((ltime->tm_year % 4) == 0) &&
					(((ltime->tm_year % 100) != 0) || ((ltime->tm_year % 400) == 0)))
					at_month_end = (ltime->tm_mday == 29);
				else
					at_month_end = (ltime->tm_mday == 28);
				break;
			}

			// Drop back n months
			ltime->tm_mon -= within;
			
			// Normalise month
			while(ltime->tm_mon < 0)
			{
				ltime->tm_mon += 12;
				ltime->tm_year--;
			}
			
			// Now adjust day if greater than 27
			if (ltime->tm_mday > 27)
			{
				switch(ltime->tm_mon)
				{
				case 0: // Jan
				case 2: // Mar
				case 4: // May
				case 6: // July
				case 7: // August
				case 9: // October
				case 11: // December
					// These are 31 day months
					if (at_month_end)
						ltime->tm_mday = 31;
					break;
				case 3: // April
				case 5: // June
				case 8: // September
				case 10: // November
					// These are 30 day months
					if (at_month_end)
						ltime->tm_mday = 30;
					break;
				case 1: // February
					// These are 28/29 day months
					int max_days = 28;
					if (((ltime->tm_year % 4) == 0) &&
						(((ltime->tm_year % 100) != 0) || ((ltime->tm_year % 400) == 0)))
						max_days = 29;
					if (at_month_end || (ltime->tm_mday > max_days))
						ltime->tm_mday = max_days;
					break;
				}
			}

			// To to adjusted time
			date = ::mktime(ltime);
		}
		break;
	case eWithinYears:
		date = ::time(NULL);
		// Subtract 'within' years worth of seconds
		date -= cYearSeconds * within;
		break;
	default:;
	}

	return date;
}

cdstring CSearchItem::GetInfo(void) const
{
	cdstrboolvect list;
	GenerateItems(list, false);

	cdstring sexpression("(");
	bool first = true;
	for(cdstrboolvect::iterator iter = list.begin(); iter != list.end(); iter++)
	{
		if (first)
			first = false;
		else if ((*iter).second)
		{
			sexpression += ' ';
			(*iter).first.quote();
			(*iter).first.ConvertFromOS();
		}
		else if ((*iter).first == "(")
		{
			sexpression += ' ';
			first = true;
		}
		sexpression += (*iter).first;
	}
	sexpression += ")";
	return sexpression;
}

void CSearchItem::SetInfo(char_stream& txt)
{
	if (txt.start_sexpression())
	{
		CSearchItem* first = NULL;

		// Get first one
		first = ParseItem(txt, true);

		while(!txt.end_sexpression())
		{
			CSearchItem* item = ParseItem(txt, true);

			if (first && item)
			{
				// Make this into an AND list
				mType = eAnd;
				mData = new CSearchItemList;
				reinterpret_cast<CSearchItemList*>(mData)->push_back(first);
				reinterpret_cast<CSearchItemList*>(mData)->push_back(item);
				first = NULL;
			}
			else if (item && mData)
				reinterpret_cast<CSearchItemList*>(mData)->push_back(item);
		}

		// Grab data if only one
		if (first)
		{
			// Grab data
			mType = first->mType;
			mMatchType = first->mMatchType;
			mData = first->mData;

			// Clear and delete
			first->mType = eAll;
			first->mData = NULL;
			delete first;
		}
	}
}

// Parse single item from stream
CSearchItem* CSearchItem::ParseItem(char_stream& txt, bool convert)
{
	if (!*txt)
		return NULL;

	// Special for (..)
	if (txt.start_sexpression())
	{
		CSearchItemList* list = new CSearchItemList;

		// Parse all items
		while(!txt.end_sexpression())
			list->push_back(ParseItem(txt, convert));

		// Create AND item
		return new CSearchItem(eAnd, list);
	}
	else
	{
		const char* str = txt.get();
		if (!str || !*str)
			return NULL;

		cdstring temp = str;
		temp.ConvertToOS();
		str = temp.c_str();

		// Look for comparator type
		ESearchMatchType match = eDefault;
		if (!::strcmp(str, cSEARCH_IS))
			match = eIs;
		else if (!strcmp(str, cSEARCH_CONTAINS))
			match = eContains;
		else if (!strcmp(str, cSEARCH_STARTSWITH))
			match = eStartsWith;
		else if (!strcmp(str, cSEARCH_ENDSWITH))
			match = eEndsWith;
		else if (!strcmp(str, cSEARCH_MATCHES))
			match = eMatches;
		else if (!strcmp(str, cSEARCH_EXISTS))
			match = eExists;
		
		// Punt to next item if comparitor found
		if (match != eDefault)
		{
			txt.get(temp, true);
			str = temp.c_str();
		}

		// Look for each possible type!
		if (::strcmp(str, cSEARCH_ALL) == 0)				// -
			return new CSearchItem(eAll, match);

		else if (::strcmp(str, cSEARCH_ANSWERED) == 0)		// -
			return new CSearchItem(eAnswered, match);

		else if (::strcmp(str, cSEARCH_BCC) == 0)			// cdstring*
			return ParseAddress(txt, eBcc, match, convert);

		else if (::strcmp(str, cSEARCH_BEFORE) == 0)		// date
			return new CSearchItem(eBefore, ParseDate(txt), match);

		else if (::strcmp(str, cSEARCH_BODY) == 0)			// cdstring*
			return new CSearchItem(eBody, convert ? cdstring::ConvertToOS(txt.get()) : cdstring(txt.get()), match);

		else if (::strcmp(str, cSEARCH_CC) == 0)			// cdstring*
			return ParseAddress(txt, eCC, match, convert);

		else if (::strcmp(str, cSEARCH_DELETED) == 0)		// -
			return new CSearchItem(eDeleted, match);

		else if (::strcmp(str, cSEARCH_DRAFT) == 0)			// -
			return new CSearchItem(eDraft, match);

		else if (::strcmp(str, cSEARCH_FLAGGED) == 0)		// -
			return new CSearchItem(eFlagged, match);

		else if (::strcmp(str, cSEARCH_FROM) == 0)			// cdstring*
			return ParseAddress(txt, eFrom, match, convert);

		else if (::strcmp(str, cSEARCH_HEADER) == 0)		// cdstrpair*
		{
			cdstring temp1;
			txt.get(temp1, convert);
			cdstring temp2;
			txt.get(temp2, convert);
			return new CSearchItem(eHeader, temp1, temp2, match);
		}

		else if (::strcmp(str, cSEARCH_KEYWORD) == 0)		// cdstring*
			return new CSearchItem(eKeyword, convert ? cdstring::ConvertToOS(txt.get()) : cdstring(txt.get()), match);

		else if (::strcmp(str, cSEARCH_LABEL) == 0)		// unsigned long
			return new CSearchItem(eLabel, (unsigned long) ::atol(txt.get()), match);

		else if (::strcmp(str, cSEARCH_LARGER) == 0)		// long
			return new CSearchItem(eLarger, ::atol(txt.get()), match);

		else if (::strcmp(str, cSEARCH_LPAREN) == 0)		// CSearchItemList*
		{
			std::auto_ptr<CSearchItemList> list(new CSearchItemList);

			// Parse items until NULL
			CSearchItem* item = ParseItem(txt, convert);
			while(item)
			{
				list->push_back(item);
				item = ParseItem(txt, convert);
			}

			// If more than one => implicit AND
			if (list->size() > 1)
			{
				// Make current set of items into an AND and add that to a new list
				CSearchItem* and_item = new CSearchItem(eAnd, list.release());
				list.reset(new CSearchItemList);
				list->push_back(and_item);
			}

			// Create group (parenthesised) item
			return new CSearchItem(eGroup, list->size() != 0 ? list->front() : NULL, match);
		}

		else if (::strcmp(str, cSEARCH_NEW) == 0)			// -
			return new CSearchItem(eNew, match);

		else if (::strcmp(str, cSEARCH_NOT) == 0)			// CSearchItem*
			return new CSearchItem(eNot, ParseItem(txt, convert), match);

		else if (::strcmp(str, cSEARCH_OLD) == 0)			// -
			return new CSearchItem(eOld, match);

		else if (::strcmp(str, cSEARCH_ON) == 0)			// date
			return new CSearchItem(eOn, ParseDate(txt), match);

		else if (::strcmp(str, cSEARCH_OR) == 0)			// CSearchItemList*
		{
			CSearchItemList* list = new CSearchItemList;

			// Parse both items
			CSearchItem* item1 = ParseItem(txt, convert);
			CSearchItem* item2 = ParseItem(txt, convert);

			// Add each and collapse if another OR
			if (item1->GetType() == eOr)
			{
				// Add items in sublist
				if (item1->GetData())
					list->insert(list->end(), reinterpret_cast<const CSearchItemList*>(item1->GetData())->begin(),
												reinterpret_cast<const CSearchItemList*>(item1->GetData())->end());

				// Delete item without deleting items
				delete reinterpret_cast<const CSearchItemList*>(item1->GetData());
				item1->mType = eAll;
				item1->mData = NULL;
				delete item1;
			}
			else
				list->push_back(item1);
			if (item2->GetType() == eOr)
			{
				// Add items in sublist
				if (item2->GetData())
					list->insert(list->end(), reinterpret_cast<const CSearchItemList*>(item2->GetData())->begin(),
												reinterpret_cast<const CSearchItemList*>(item2->GetData())->end());

				// Delete item without deleting items
				delete reinterpret_cast<const CSearchItemList*>(item2->GetData());
				item2->mType = eAll;
				item2->mData = NULL;
				delete item2;
			}
			else
				list->push_back(item2);

			// Create OR item
			return new CSearchItem(eOr, list, match);
		}

		else if (::strcmp(str, cSEARCH_RECENT) == 0)		// -
			return new CSearchItem(eRecent, match);

		else if (::strcmp(str, cSEARCH_RPAREN) == 0)		// End of parenthesised list
			// Just return NULL to indicate end of list
			return NULL;

		else if (::strcmp(str, cSEARCH_SEEN) == 0)			// -
			return new CSearchItem(eSeen, match);

		if (::strcmp(str, cSEARCH_SELECTED) == 0)				// -
			return new CSearchItem(eSelected, match);

		else if (::strcmp(str, cSEARCH_SENTBEFORE) == 0)	// date
			return new CSearchItem(eSentBefore, ParseDate(txt), match);

		else if (::strcmp(str, cSEARCH_SENTON) == 0)		// date
			return new CSearchItem(eSentOn, ParseDate(txt), match);

		else if (::strcmp(str, cSEARCH_SENTSINCE) == 0)		// date
			return new CSearchItem(eSentSince, ParseDate(txt), match);

		else if (::strcmp(str, cSEARCH_SINCE) == 0)			// date
			return new CSearchItem(eSince, ParseDate(txt), match);

		else if (::strcmp(str, cSEARCH_SMALLER) == 0)		// long
			return new CSearchItem(eSmaller, ::atol(txt.get()), match);

		else if (::strcmp(str, cSEARCH_SUBJECT) == 0)		// cdstring*
			return new CSearchItem(eSubject, convert ? cdstring::ConvertToOS(txt.get()) : cdstring(txt.get()), match);

		else if (::strcmp(str, cSEARCH_TEXT) == 0)			// cdstring*
			return new CSearchItem(eText, convert ? cdstring::ConvertToOS(txt.get()) : cdstring(txt.get()), match);

		else if (::strcmp(str, cSEARCH_TO) == 0)			// cdstring*
			return ParseAddress(txt, eTo, match, convert);

		else if (::strcmp(str, cSEARCH_UID) == 0)			// ulvector*
			return new CSearchItem(eUID, ParseSequence(txt), match);

		else if (::strcmp(str, cSEARCH_UNANSWERED) == 0)	// -
			return new CSearchItem(eUnanswered, match);

		else if (::strcmp(str, cSEARCH_UNDELETED) == 0)		// -
			return new CSearchItem(eUndeleted, match);

		else if (::strcmp(str, cSEARCH_UNDRAFT) == 0)		// -
			return new CSearchItem(eUndraft, match);

		else if (::strcmp(str, cSEARCH_UNFLAGGED) == 0)		// -
			return new CSearchItem(eUnflagged, match);

		else if (::strcmp(str, cSEARCH_UNKEYWORD) == 0)		// cdstring*
			return new CSearchItem(eUnkeyword, convert ? cdstring::ConvertToOS(txt.get()) : cdstring(txt.get()), match);

		else if (::strcmp(str, cSEARCH_UNSEEN) == 0)		// -
			return new CSearchItem(eUnseen, match);

		else if (::strcmp(str, cSEARCH_RECIPIENT) == 0)			// cdstring*
			return ParseAddress(txt, eRecipient, match, convert);

		else if (::strcmp(str, cSEARCH_CORRESPONDENT) == 0)		// cdstring*
			return ParseAddress(txt, eCorrespondent, match, convert);

		else if (::strcmp(str, cSEARCH_SENDER) == 0)		// cdstring*
			return ParseAddress(txt, eSender, match, convert);

		else if (::strcmp(str, cSEARCH_NAMEDSTYLE) == 0)	// cdstring*
			return new CSearchItem(eNamedStyle, convert ? cdstring::ConvertToOS(txt.get()) : cdstring(txt.get()), match);

		else												// ulvector*
			return new CSearchItem(eNumber, ParseSequence(txt), match);
	}
}

// Parse search item address
CSearchItem* CSearchItem::ParseAddress(char_stream& txt, ESearchType type, ESearchMatchType match, bool convert)
{
	char* addr = txt.get();

	if (!addr || !*addr)
		return new CSearchItem(type, match);
	else
		return new CSearchItem(type, cdstring::ConvertToOS(addr), match);
}

// Parse search item date
unsigned long CSearchItem::ParseDate(char_stream& txt)
{
	// This maybe in dd-mmm-yyyy format or a single number
	char* s = txt.get();
	unsigned long date = CRFC822::ParseDate(s);
	if (!date)
		date = ::atol(s);

	return date;
}

// Parse search item sequence
ulvector CSearchItem::ParseSequence(char_stream& txt)
{
	ulvector nums;
	unsigned long num1 = 0;
	unsigned long num_start = 0;

	const char* s = txt.get();

	while(true)
	{
		char c = *s++;
		if (isdigit(c))
			// Bump up count
			num1 = 10*num1 + (c - '0');
		else if ((c == ',') || !c)
		{
			// Is it end of range (nn:mm)?
			if (num_start)
			{
				// Add all items in range num_start:num1
				for(; num_start <= num1; num_start++)
					nums.push_back(num_start);
			}
			else
				// End of single num
				nums.push_back(num1);

			num1 = 0;
			num_start = 0;

			// Check termination
			if (!c)
				break;
		}
		else if (c == ':')
		{
			// Got a range
			num_start = num1;
			num1 = 0;
		}
	}

	return nums;
}

void CSearchItem::GenerateSIEVEScript(std::ostream& out) const
{
	// Now do criteria
	switch(GetType())
	{
	case eAll:			// -
		out << "true";
		break;

	case eAnd:			// CSearchItemList*
		{
			out << "allof (";
			const CSearchItemList* sub_items = reinterpret_cast<const CSearchItemList*>(GetData());
			if (sub_items)
			{
				for(CSearchItemList::const_iterator iter = sub_items->begin(); iter != sub_items->end(); iter++)
				{
					if (iter != sub_items->begin())
						out << ", ";
					(*iter)->GenerateSIEVEScript(out);
				}
			}
			out << ")";
		}
		break;

	case eBcc:			// cdstring*
		GenerateAddressSIEVE(out, "\"Bcc\"");
		break;

	case eCC:			// cdstring*
		GenerateAddressSIEVE(out, "\"CC\"");
		break;

	case eFrom:		// cdstring*
		GenerateAddressSIEVE(out, "\"From\"");
		break;

	case eGroup:	// CSearchItem*
		{
			//out << "(";
			const CSearchItem* sub_item = reinterpret_cast<const CSearchItem*>(GetData());
			if (sub_item)
				sub_item->GenerateSIEVEScript(out);
			//out << ")";
		}
		break;

	case eHeader:		// cdstrpair*
	{
		out << "header ";
		GenerateSIEVEMatchType(out);
		out << " [";
		cdstring temp = (GetData() ? reinterpret_cast<const cdstrpair*>(GetData())->first : cdstring::null_str);
		temp.quote(true);
		out << temp;
		out << "] [";
		temp = (GetData() ? reinterpret_cast<const cdstrpair*>(GetData())->second : cdstring::null_str);
		GenerateSIEVEKey(temp);
		out << temp;
		out << "]";
		break;
	}

	case eLarger:		// long
	{
		out << "size :over ";
		GenerateSIEVENumber(out);
		break;
	}

	case eNot:			// CSearchItem*
	  {
	  	out << "not ";
	  	if (GetData())
			reinterpret_cast<const CSearchItem*>(GetData())->GenerateSIEVEScript(out);
		break;
	  }

	case eOr:			// CSearchItemList*
		{
			out << "anyof (";
			const CSearchItemList* sub_items = reinterpret_cast<const CSearchItemList*>(GetData());
			if (sub_items)
			{
				for(CSearchItemList::const_iterator iter = sub_items->begin(); iter != sub_items->end(); iter++)
				{
					if (iter != sub_items->begin())
						out << ", ";
					(*iter)->GenerateSIEVEScript(out);
				}
			}
			out << ")";
		}
		break;

	case eSmaller:		// long
	{
		out << "size :under ";
		GenerateSIEVENumber(out);
		break;
	}

	case eSubject:		// cdstring*
	{
		out << "header ";
		GenerateSIEVEMatchType(out);
		out << " [\"Subject\"] [";
		cdstring temp = (GetData() ? *reinterpret_cast<const cdstring*>(GetData()) : cdstring::null_str);
		GenerateSIEVEKey(temp);
		out << temp;
		out << "]";
		break;
	}

	case eTo:			// cdstring*
		GenerateAddressSIEVE(out, "\"To\"");
		break;

	case eRecipient:	// cdstring*
		GenerateAddressSIEVE(out, "[\"To\", \"CC\", \"Bcc\"]");
		break;

	case eCorrespondent:	// cdstring*
		GenerateAddressSIEVE(out, "[\"From\", \"To\", \"CC\", \"Bcc\"]");
		break;

	case eSender:			// cdstring*
		GenerateAddressSIEVE(out, "\"Sender\"");
		break;
	
	default:;
	}
}

// Generate list of expanded address items
void CSearchItem::GenerateAddressSIEVE(std::ostream& out, const char* hdr) const
{
	out << "address ";
	GenerateSIEVEMatchType(out);
	out << " " << hdr << " ";

	// If its empty that means use 'me' as the address
	if (GetData() == NULL)
	{
		// Are there any smart addresses?
		if (CPreferences::sPrefs->mSmartAddressList.GetValue().size())
		{
			out << "[";

			// Add default identity
			cdstring addr_txt = CPreferences::sPrefs->mIdentities.GetValue().front().GetFrom();
			CAddress addr(addr_txt);
			cdstring temp = addr.GetMailAddress();
			GenerateSIEVEKey(temp);
			out << temp;

			// Now do each smart address
			const cdstrvect& addrs = CPreferences::sPrefs->mSmartAddressList.GetValue();
			for(cdstrvect::const_iterator iter = addrs.begin(); iter != addrs.end(); iter++)
			{
				out << ", ";
				temp = *iter;
				GenerateSIEVEKey(temp);
				out << temp;
			}

			out << "]";
		}
		else
		{
			// Get default identity
			cdstring addr_txt = CPreferences::sPrefs->mIdentities.GetValue().front().GetFrom();
			CAddress addr(addr_txt);

			// Just a single text address
			cdstring temp = addr.GetMailAddress();
			GenerateSIEVEKey(temp);
			out << temp;
		}
	}
	else
	{
		// Just a single text address
		cdstring temp = *reinterpret_cast<const cdstring*>(GetData());
		GenerateSIEVEKey(temp);
		out << temp;
	}
	
}

// Generate list of expanded address items
void CSearchItem::GenerateSIEVEMatchType(std::ostream& out) const
{
	switch(GetMatchType())
	{
	case eDefault:
	case eContains:
	default:
		out << ":contains";
		break;
	case eIs:
		out << ":is";
		break;
	case eStartsWith:
	case eEndsWith:
	case eMatches:
		out << ":matches";
		break;
	}
}

// Generate key
void CSearchItem::GenerateSIEVEKey(cdstring& key) const
{
	switch(GetMatchType())
	{
	default:
		break;
	case eStartsWith:
		key += "*";
		break;
	case eEndsWith:
	{
		cdstring temp = "*";
		key = temp + cdstring(key);
		break;
	}
	}
	
	key.quote(true);
}

// Generate number
void CSearchItem::GenerateSIEVENumber(std::ostream& out) const
{
	long num = reinterpret_cast<long>(GetData());
	
	if (num >= 1024L * 1024L)
		out << cdstring(num / (1024L * 1024L)) << "M";
	else if (num >= 1024L)
		out << cdstring(num / 1024L) << "K";
	else
		out << cdstring(num);
}
