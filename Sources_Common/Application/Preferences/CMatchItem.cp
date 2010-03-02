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


// CMatchItem.cp - holds status of Match menu in mailbox window

#include "CMatchItem.h"

#include "CAddressList.h"
#include "CMessage.h"
#include "CMessageList.h"
#include "CPreferences.h"
#include "CStringUtils.h"
#include "char_stream.h"

#include <algorithm>

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

const char* cMatchDescriptors[] = {"None"
									"New",
									"Unseen",
									"Important",
									"Answered",
									"Deleted",
									"Draft",
									"SentToday",
									"SentSinceYesterday",
									"SentThisWeek",
									"SentWithin7Days",
									"SentByMe",
									"eSentToMe",
									"eSentToList",
									"eSelectedTo",
									"eSelectedFrom",
									"eSelectedSmart",
									"eSelectedSubject",
									"eSelectedDate",
									NULL};

void CMatchItem::SetSingleMatch(EMatchItem item)
{
	// Clear existing items
	mNegate = false;
	mUseOr = true;
	std::fill(mBitsSet.begin(), mBitsSet.end(), false);
	mSearchSet.clear();

	mBitsSet[item] = true;
}

bool CMatchItem::NeedsSelection() const
{
	// Count number of true items in selected range
	bool temp = true;
	return std::count(mBitsSet.begin() + eSelected_First, mBitsSet.begin() + eSelected_Last + 1, temp);
}

CSearchItem* CMatchItem::ConstructSearch(const CMessageList* msgs) const
{
	// If nothing selected return
	bool temp = true;
	size_t bitsset = std::count(mBitsSet.begin(), mBitsSet.end(), temp);
	bitsset += mSearchSet.size();
	if (!bitsset)
		return NULL;

	// If more than one then need to do list
	CSearchItem* result = NULL;
	CSearchItemList* items = NULL;
	if (bitsset > 1)
	{
		items = new CSearchItemList;
		result = new CSearchItem(mUseOr ? CSearchItem::eOr : CSearchItem::eAnd, items);
	}

	unsigned long index = 0;
	for(boolvector::const_iterator iter = mBitsSet.begin(); iter != mBitsSet.end(); iter++, index++)
	{
		// Only if set
		if (!*iter)
			continue;

		CSearchItem* this_item = NULL;

		switch(index)
		{
		case eNew:
			this_item = new CSearchItem(CSearchItem::eNew);
			break;

		case eUnseen:
			this_item = new CSearchItem(CSearchItem::eUnseen);
			break;

		case eFlagged:
			this_item = new CSearchItem(CSearchItem::eFlagged);
			break;

		case eAnswered:
			this_item = new CSearchItem(CSearchItem::eAnswered);
			break;

		case eDeleted:
			this_item = new CSearchItem(CSearchItem::eDeleted);
			break;

		case eDraft:
			this_item = new CSearchItem(CSearchItem::eDraft);
			break;

		case eSentToday:
			this_item = new CSearchItem(CSearchItem::eSentOn, static_cast<unsigned long>(CSearchItem::eToday));
			break;

		case eSentSinceYesterday:
			this_item = new CSearchItem(CSearchItem::eSentSince, static_cast<unsigned long>(CSearchItem::eSinceYesterday));
			break;

		case eSentThisWeek:
			this_item = new CSearchItem(CSearchItem::eSentSince, static_cast<unsigned long>(CSearchItem::eThisWeek));
			break;

		case eSentWithin7Days:
			this_item = new CSearchItem(CSearchItem::eSentSince, static_cast<unsigned long>(CSearchItem::eWithin7Days));
			break;

		case eSentByMe:
			// Use smart addresses
			this_item = new CSearchItem(CSearchItem::eFrom);
			break;

		case eSentToMe:
			// Use smart addresses
			this_item = new CSearchItem(CSearchItem::eTo);
			break;

		case eSentToList:
			{
				// Has to be done as one big NOT:
				// SEARCH NOT ( OR [FROM main_id FROM smart1 ...] OR [TO main_id TO smart1 ...] OR [CC main_id  CC smart1 ...] )

				// Create the OR spec
				CSearchItemList* list = new CSearchItemList;
				this_item = new CSearchItem(CSearchItem::eNot, new CSearchItem(CSearchItem::eOr, list));
				list->push_back(new CSearchItem(CSearchItem::eFrom));
				list->push_back(new CSearchItem(CSearchItem::eTo));
				list->push_back(new CSearchItem(CSearchItem::eCC));
			}
			break;

		case eSelectedTo:
			// Add all unique To's in message lists
			this_item = AddAddressFieldToList(msgs, CSearchItem::eTo);
			break;

		case eSelectedFrom:
			// Add all unique From's in message lists
			this_item = AddAddressFieldToList(msgs, CSearchItem::eFrom);
			break;

		case eSelectedSmart:
			if (msgs && msgs->size())
			{
				// Policy:
				// get smart addresses : saddrs;
				// get from/to/cc addresses : addrs;
				// get corrspondent addresses : caddrs (= addrs - saddrs)
				// SEARCH OR FROM caddrs OR TO caddrs OR CC caddrs

				// Get smart addresses as text
				cdstrvect saddrs;

				// Add default identity
				cdstring addr_txt = CPreferences::sPrefs->mIdentities.GetValue().front().GetFrom();
				CAddress addr(addr_txt);
				saddrs.push_back(addr.GetMailAddress());

				// Add all aliases
				if (CPreferences::sPrefs->mSmartAddressList.GetValue().size())
					saddrs.insert(saddrs.end(), CPreferences::sPrefs->mSmartAddressList.GetValue().begin(),
												CPreferences::sPrefs->mSmartAddressList.GetValue().end());

				// Get all To's and From's
				cdstrvect addrs;
				for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
				{
					// Do nothing if message not cached
					if (!*iter)
						continue;

					// Get envelope - do nothing if not cached
					CEnvelope* env = const_cast<CMessage*>(*iter)->GetEnvelope();
					if (!env)
						continue;

					// Try to add each address
					env->GetFrom()->AddMailAddressToList(addrs, false);
					env->GetTo()->AddMailAddressToList(addrs, false);
					env->GetCC()->AddMailAddressToList(addrs, false);
				}

				// Now remove saddrs from addrs
				cdstrvect caddrs(addrs);
				std::sort(addrs.begin(), addrs.end());
				std::sort(saddrs.begin(), saddrs.end());
				cdstrvect::iterator set_end = std::set_difference(addrs.begin(), addrs.end(), saddrs.begin(), saddrs.end(), caddrs.begin());
				caddrs.erase(set_end, caddrs.end());

				// Are there any to do?
				if (caddrs.size())
				{
					// Create OR list and push into OR spec
					CSearchItemList* list = new CSearchItemList;
					this_item = new CSearchItem(CSearchItem::eOr, list);

					// Add search for matching From, To & CC
					AddAddressesToList(list, &caddrs, CSearchItem::eFrom);
					AddAddressesToList(list, &caddrs, CSearchItem::eTo);
					AddAddressesToList(list, &caddrs, CSearchItem::eCC);
				}
			}
			break;

		case eSelectedSubject:
			if (msgs && msgs->size())
			{
				// Accumulate subjects (actually threads) from all messages
				cdstrvect threads;
				for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
				{
					if (!*iter)
						continue;

					CEnvelope* env = (*iter)->GetEnvelope();
					if (!env)
						continue;

					// Add to list (make sure lower case for later comparisons)
					cdstring thread = env->GetMatchSubject();
					::strlower(thread.c_str_mod());
					threads.push_back(thread);
				}

				// Sort, collapse, shorten
				std::sort(threads.begin(), threads.end());
				cdstrvect::iterator unique_end = std::unique(threads.begin(), threads.end());
				threads.erase(unique_end, threads.end());

				// Is there more than one?
				if (threads.size() > 1)
				{
					// Create OR list and push into OR spec
					CSearchItemList* list = new CSearchItemList;
					this_item = new CSearchItem(CSearchItem::eOr, list);

					// Add all threads
					for(cdstrvect::const_iterator iter = threads.begin(); iter != threads.end(); iter++)
						list->push_back(new CSearchItem(CSearchItem::eSubject, *iter));
				}
				else if (threads.size())
				{
					// Just add single item
					this_item = new CSearchItem(CSearchItem::eSubject, threads.front());
				}
			}
			break;

		case eSelectedDate:
			if (msgs && msgs->size())
			{
				// Accumulate dates from all messages
				ulvector dates;
				for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
				{
					if (!*iter)
						continue;

					CEnvelope* env = (*iter)->GetEnvelope();
					if (!env)
						continue;

					// Convert to mid-night
					time_t date = env->GetDate();
					struct tm* tms = ::localtime(&date);
					tms->tm_sec = 0;
					tms->tm_min = 0;
					tms->tm_hour = 0;
					date = ::mktime(tms);

					// Add to list
					dates.push_back(date);
				}

				// Sort and filter duplicates
				std::sort(dates.begin(), dates.end());
				ulvector::iterator unique_end = std::unique(dates.begin(), dates.end());
				dates.erase(unique_end, dates.end());

				// Is there more than one?
				if (dates.size() > 1)
				{
					// Create OR list and push into OR spec
					CSearchItemList* list = new CSearchItemList;
					this_item = new CSearchItem(CSearchItem::eOr, list);

					// Add all dates
					for(ulvector::const_iterator iter = dates.begin(); iter != dates.end(); iter++)
						list->push_back(new CSearchItem(CSearchItem::eSentOn, *iter));
				}
				else if (dates.size())
				{
					// Just add single item
					this_item = new CSearchItem(CSearchItem::eSentOn, dates.front());
				}
			}
			break;

		default:
			break;
		}

		// Add to list if available
		if (items && this_item)
			items->push_back(this_item);
		else if (this_item)
			result = this_item;
	}

	for(cdstrvect::const_iterator iter = mSearchSet.begin(); iter != mSearchSet.end(); iter++)
	{
		CSearchItem* this_item = NULL;

		// Get style
		this_item = new CSearchItem(CSearchItem::eNamedStyle, *iter);

		// Add to list if available
		if (items && this_item)
			items->push_back(this_item);
		else if (this_item)
			result = this_item;
	}

	// Look for not
	if (result && mNegate)
		result = new CSearchItem(CSearchItem::eNot, result);

	// Check for valid multiple items
	if (items && !items->size())
	{
		// Delete the search item - not needed
		delete result;
		return NULL;
	}
	else
		return result;
}

void CMatchItem::AddSmartAddressToList(CSearchItemList* list, CSearchItem::ESearchType type, bool use_not) const
{
	// Do default identity
	cdstring addr_txt = CPreferences::sPrefs->mIdentities.GetValue().front().GetFrom();
	CAddress addr(addr_txt);
	CSearchItem* comp_item = new CSearchItem(type, addr.GetMailAddress());

	if (use_not)
		list->push_back(new CSearchItem(CSearchItem::eNot, comp_item));
	else
		list->push_back(comp_item);

	// Now do each smart address
	AddAddressesToList(list, &CPreferences::sPrefs->mSmartAddressList.GetValue(), type, use_not);
}

void CMatchItem::AddAddressesToList(CSearchItemList* list, const cdstrvect* addrs,
											CSearchItem::ESearchType type, bool use_not) const
{
	// Now do each address
	for(cdstrvect::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
	{
		CSearchItem* comp_item = new CSearchItem(type, *iter);
		if (use_not)
			list->push_back(new CSearchItem(CSearchItem::eNot, comp_item));
		else
			list->push_back(comp_item);
	}
}

CSearchItem* CMatchItem::AddAddressFieldToList(const CMessageList* msgs, CSearchItem::ESearchType type) const
{
	CSearchItem* this_item = NULL;

	// Only if msgs
	if (msgs && msgs->size())
	{
		// Create list of unique To addresses
		cdstrvect addrs;
		for(CMessageList::const_iterator iter1 = msgs->begin(); iter1 != msgs->end(); iter1++)
		{
			// Do nothing if message not cached
			if (!*iter1)
				continue;

			// Get envelope - do nothing if not cached
			CEnvelope* env = const_cast<CMessage*>(*iter1)->GetEnvelope();
			if (!env)
				continue;

			// Try to add each address
			const CAddressList* addr_list = NULL;
			switch(type)
			{
			case CSearchItem::eTo:
				addr_list = env->GetTo();
				break;
			case CSearchItem::eFrom:
				addr_list = env->GetFrom();
				break;
			case CSearchItem::eCC:
				addr_list = env->GetCC();
				break;
			case CSearchItem::eBcc:
				addr_list = env->GetBcc();
				break;
			default:;
			}

			if (addr_list)
				addr_list->AddMailAddressToList(addrs, false);
		}

		// Sort, collapse, shorten
		std::sort(addrs.begin(), addrs.end());
		cdstrvect::iterator unique_end = std::unique(addrs.begin(), addrs.end());
		addrs.erase(unique_end, addrs.end());

		// Is there more than one?
		if (addrs.size() > 1)
		{
			// Create OR list and push into OR spec
			CSearchItemList* list = new CSearchItemList;
			this_item = new CSearchItem(CSearchItem::eOr, list);

			// Add all smart addresses
			AddAddressesToList(list, &addrs, type);
		}
		else if (addrs.size())
		{
			// Just add single item
			this_item = new CSearchItem(type, addrs.front());
		}
	}

	return this_item;
}

cdstring CMatchItem::GetInfo(void) const
{
	cdstring info;

	info += mNegate ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;

	info += mUseOr ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;

	info += "(";
	bool first = true;
	for(int i = eNew; i <= eMatchLast; i++)
	{
		if (mBitsSet[i])
		{
			if (first)
				first = false;
			else
				info += cSpace;
			info += cMatchDescriptors[i];
		}
	}
	info += ")";
	info += cSpace;
	cdstring temp;
	temp.CreateSExpression(mSearchSet);
	temp.ConvertFromOS();
	info += temp;

	return info;
}

bool CMatchItem::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	txt.get(mNegate);

	txt.get(mUseOr);

	// Get bits
	if (txt.start_sexpression())
	{
		do
		{
			char* item = txt.get();
			long i = ::strindexfind(item, cMatchDescriptors, eMatchLast + 1);
			if (i <= eMatchLast)
				mBitsSet[i] = true;
		} while(!txt.end_sexpression());
	}

	// Get search sets
	cdstring::ParseSExpression(txt, mSearchSet, true);

	return true;
}
