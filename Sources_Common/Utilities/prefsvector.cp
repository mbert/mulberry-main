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


// prefsvector.cp - vectors of preference items

#include "prefsvector.h"

#include "char_stream.h"
#include "CActionItem.h"
#include "CDisplayItem.h"
#include "CFavouriteItem.h"
#include "CFilterItem.h"
#include "CFilterScript.h"
#include "CFilterTarget.h"
#include "CMailNotification.h"
#include "CIdentity.h"
#include "COptionsMap.h"
#include "CPreferenceValue.h"
#include "CSearchStyle.h"
#include "CTargetItem.h"
#include "CWindowStates.h"

#include <algorithm>

#pragma mark ____________________________prefsvector

// Create S_Expression from vector
template <class T> cdstring prefsvector<T>::GetInfo() const
{
	cdstring all = '(';

	for(typename prefsvector<T>::const_iterator iter = this->begin(); iter != this->end(); iter++)
	{
		all += '(';
		all += (*iter).GetInfo();
		all += ')';
	}

	all += ')';

	return all;
}

// Parse S-Expression into vector
template <class T> bool prefsvector<T>::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	bool result = true;

	// Clear existing
	this->clear();

	// Must have leading (
	if (!txt.start_sexpression()) return false;

	while(txt.start_sexpression())
	{
		T item;
		result = item.SetInfo(txt, vers_prefs) && result;
		this->push_back(item);
		txt.end_sexpression();
	}

	// Bump past trailing )
	txt.end_sexpression();

	return result;
}

#pragma mark ____________________________prefsptrvector

// Create S_Expression from vector
template <class T> cdstring prefsptrvector<T>::GetInfo() const
{
	cdstring all = '(';

	for(typename prefsptrvector<T>::const_iterator iter = this->begin(); iter != this->end(); iter++)
	{
		all += '(';
		all += (*iter)->GetInfo();
		all += ')';
	}

	all += ')';

	return all;
}

// Parse S-Expression into vector
template <class T> bool prefsptrvector<T>::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	bool result = true;

	// Clear existing
	this->clear();

	// Must have leading '('
	if (!txt.start_sexpression()) return false;

	while(txt.start_sexpression())
	{
		T* item = new T;
		result = item->SetInfo(txt, vers_prefs) && result;
		this->push_back(item);
		txt.end_sexpression();
	}

	// Bump past trailing )
	txt.end_sexpression();

	return result;
}

// Create S_Expression from vector
template <class T> void prefsptrvector<T>::WriteToMapMulti(COptionsMap* theMap, const char* key, bool dirty_only) const
{
	// Set section
	StMapSection section(theMap, key);

	// Initial key count
	unsigned long ctr = 1;

	cdstring newkey;
	cdstring value;
	for(typename prefsptrvector<T>::const_iterator iter = this->begin(); iter != this->end(); iter++)
	{
		// Set up numeric key with leading zeros
		newkey = ctr++;

		// Get the value for this key
		value = (*iter)->GetInfo();
		
		// Write key-value without charset conversion (already done)
		theMap->WriteValue(newkey, value, true, false);
	}
	
	// Empty any previous items up to the total last read in
	// Ideally these prefs should be removed from the map altogether
	// NB This is only relevant for options formats that have to leave empty space for enums
	if (theMap->WriteEmptyEnums())
	{
		for(unsigned long i = ctr; i <= mLastMapMultiCount; i++)
		{
			newkey = ctr++;
			theMap->WriteValue(newkey, cdstring::null_str, true, false);
		}
	}
}

// Parse S-Expression into vector
template <class T> bool prefsptrvector<T>::ReadFromMapMulti(COptionsMap* theMap, const char* key, NumVersion& vers_prefs, bool clear_if_empty)
{
	// Set section
	StMapSection section(theMap, key);

	// Get a list of all the keys in current sub-section
	cdstrvect keys;
	theMap->EnumKeys(keys);

	// NB This is only relevant for options formats that have to leave empty space for enums
	if (theMap->WriteEmptyEnums())
		mLastMapMultiCount = keys.size();

	// Now convert to numbers so we can read them in in numeric order
	ulvector nums;
	for(cdstrvect::const_iterator iter = keys.begin(); iter != keys.end(); iter++)
	{
		unsigned long num = ::atol(*iter);
		if (num)
			nums.push_back(num);
	}	

	// Sort the numbers
	std::sort(nums.begin(), nums.end());

	// Clear original data
	if (clear_if_empty || (nums.size() != 0))
		this->clear();

	// Read in each one
	CPreferenceValueMap<cdstring> pref;
	for(ulvector::const_iterator iter = nums.begin(); iter != nums.end(); iter++)
	{
		// Read key value without charset conversion (will be done later)
		cdstring key2 = *iter;
		cdstring value;
		theMap->ReadValue(key2, value, vers_prefs, false);
		
		// Ignore if empty
		if (value.empty())
			continue;
		
		// Parse into new filter
		char_stream p(value.c_str_mod());
		T* item = new T;
		item->SetInfo(p, vers_prefs);
		this->push_back(item);
	}
	
	return true;
}

template class prefsptrvector<CActionItem>;
template class prefsptrvector<CFilterItem>;
template class prefsptrvector<CTargetItem>;
template class prefsptrvector<CFilterScript>;
template class prefsptrvector<CFilterTarget>;
template class prefsvector<CDisplayItem>;
template class prefsvector<CFavouriteItem>;
template class prefsvector<CIdentity>;
template class prefsvector<CMailNotification>;
template class prefsptrvector<CSearchStyle>;
template class prefsvector<SColumnInfo>;
template class prefsvector<CScreenDimensions<Rect> >;
template class prefsvector<CScreenDimensions<CColumnInfoArray> >;
template class prefsptrvector<SStyleTraits2>;
