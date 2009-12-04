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


// Source for CPreferences class

#include "CWindowStates.h"

#include "CPreferenceVersions.h"
#include "CStringUtils.h"
#include "CUtils.h"
#include "char_stream.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if __framework == __jx
#include "CMulberryApp.h"
#include <JXDisplay.h>
#include <jXGlobals.h>
#endif

#include <algorithm>

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

#pragma mark ____________________________SStyleTraits2

cdstring SStyleTraits2::GetInfo() const
{
	cdstring info;
	info.reserve(512);
	int bold_style = (style & bold) ? 1 : 0;
	int italic_style = (style & italic) ? 1 : 0;
	int underline_style = (style & underline) ? 1 : 0;
	int strike_style = (style & strike_through) ? 1 : 0;

#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
	::snprintf(info.c_str_mod(), 512, "%d %d %d %d %s %ld %ld %ld %s %ld %ld %ld ", bold_style, italic_style, underline_style, strike_style,
				usecolor ? cValueBoolTrue : cValueBoolFalse, color.red >> 8, color.green >> 8, color.blue >> 8,
				usebkgcolor ? cValueBoolTrue : cValueBoolFalse, bkgcolor.red >> 8, bkgcolor.green >> 8, bkgcolor.blue >> 8);
#elif __dest_os == __win32_os
	::snprintf(info.c_str_mod(), 512, "%ld %ld %ld %ld %s %d %d %d %s %d %d %d ", bold_style, italic_style, underline_style, strike_style,
				usecolor ? cValueBoolTrue : cValueBoolFalse, GetRValue(color), GetGValue(color), GetBValue(color),
				usebkgcolor ? cValueBoolTrue : cValueBoolFalse, GetRValue(bkgcolor), GetGValue(bkgcolor), GetBValue(bkgcolor));
#endif

	cdstring temp = name;
	temp.quote();
	info += temp;

	return info;
}

bool SStyleTraits2::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	style = normal;

	unsigned long temp;

	txt.get(temp);
	if (temp)
		style |= bold;

	txt.get(temp);
	if (temp)
		style |= italic;

	txt.get(temp);
	if (temp)
		style |= underline;

	txt.get(temp);
	if (temp)
		style |= strike_through;

	txt.get(usecolor);
	
	unsigned long red;
	unsigned long green;
	unsigned long blue;

	txt.get(red);
	txt.get(green);
	txt.get(blue);
#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
	color.red = (red << 8) | red;
	color.green = (green << 8) | green;
	color.blue = (blue << 8) | blue;
#elif __dest_os == __win32_os
	color = RGB(red, green, blue);
#endif
	
	txt.get(usebkgcolor);
	
	txt.get(red);
	txt.get(green);
	txt.get(blue);
#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
	bkgcolor.red = (red << 8) | red;
	bkgcolor.green = (green << 8) | green;
	bkgcolor.blue = (blue << 8) | blue;
#elif __dest_os == __win32_os
	bkgcolor = RGB(red, green, blue);
#endif
	
	txt.get(name);
	
	return true;
}

#pragma mark ____________________________SColumnInfo

const char** SColumnInfo::s_descriptors = NULL;
long SColumnInfo::s_max = 0;
long SColumnInfo::s_max_value = 0;

cdstring SColumnInfo::GetInfo() const
{
	cdstring info = s_descriptors[column_type - 1];
	info.quote();
	info += cSpace;
	info += cdstring((long) column_width);

	return info;
}

bool SColumnInfo::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	// Must be v2.0 format here
	cdstring item;
	txt.get(item);
	long i = 0;
	while(i < s_max)
	{
		if (item == s_descriptors[i])
			break;
		i++;
	}
	if (i >= s_max)
		column_type = s_max_value;
	else
		column_type = i + 1;

	txt.get(column_width);

	// Never allow a width of 0!
	if (!column_width)
		column_width = 16;

	return true;
}

bool SColumnInfo::SetInfo_Old(const char** descriptors, long max, long max_value, NumVersion vers_prefs)
{
	bool not_old_vers = true;

	// If >= v1.4
	if (::VersionTest(vers_prefs, VERS_1_4_0) >= 0)
	{
		char* item = ::strtok(nil, ",");
		if (item)
		{
			item = ::strgetquotestr(&item);
			long i = 0;
			if (item)
				while(i < max)
				{
					if (::strcmp(item, descriptors[i]) == 0)
						break;
					i++;
				}
			if (i >= max)
				column_type = max_value;
			else
				column_type = i + 1;
		}
		item = ::strtok(nil, ",");
		if (item)
			column_width = ::atoi(item);
	}
	else
	{
		char* item = ::strtok(nil, ",");
		if (item)
			column_type = ::atoi(item);
		item = ::strtok(nil, ",");
		if (item)
			column_width = ::atoi(item);

		not_old_vers = false;
	}

	return not_old_vers;
}

#pragma mark ____________________________CWindowState

CWindowState::CWindowState()
{
	ResetRect();
	mState = eWindowStateNormal;
}

// Build from window
CWindowState::CWindowState(const char* name, Rect* bounds, EWindowState state)
{
	mName = name;
	mBounds.push_back(CScreenWindowSize(*bounds));
	mState = state;
}

// Assignment with same type
void CWindowState::_copy(const CWindowState& copy)
{
	mName = copy.mName;
	mBounds = copy.mBounds;
	mState = copy.mState;
	mFuture = copy.mFuture;
}

// Set default size
void CWindowState::ResetRect(void)
{
	// Use empty rect
#if __framework == __jx
	Rect empty(0,0,0,0);
#else
	Rect empty = {0, 0, 0, 0};
#endif
	CScreenWindowSize temp(empty);
	AddUniqueRect(temp);
}

CWindowState* CWindowState::FindInList(const CWindowStateArray& list) const
{
	// Check for duplicate
	for(CWindowStateArray::const_iterator iter = list.begin(); iter != list.end(); iter++)
	{
		// Check for same
		if (**iter == mName)
			return *iter;
	}

	return NULL;
}

bool CWindowState::MergeToList(CWindowStateArray& list, const CWindowState& default_state, bool& list_changed)
{
	// Prime results
	list_changed = false;

	// Check for existing
	CWindowState* old_state = FindInList(list);

	if (old_state)
	{
		// Merge new with old
		list_changed = old_state->Merge(*this);
		if (!list_changed)
			return false;

		// Prune unwanted items
		PruneDefaults(default_state);

		// Compare with default
		if (PartialCompare(default_state))
		{
			// Remove from list if the same
			CWindowStateArray::iterator found = std::find(list.begin(), list.end(), old_state);
			if (found != list.end())
			{
				list.erase(found);
				delete old_state;
				return false;
			}
			else
				return false;
		}
		else
			// Just changed original item
			return false;
	}
	else
	{
		// Compare with default
		if (PartialCompare(default_state))
		{
			// Same as default => no need to create a new one - just return
			return false;
		}
		else
		{
			// Prune unwanted items
			PruneDefaults(default_state);

			// Now add new item to list
			list.push_back(this);
			list_changed = true;

			return true;
		}
	}
}

void CWindowState::PruneDefaults(const CWindowState& default_state)
{
	// Check to see if screen sizes in one match any in the other
	for(CScreenMap::iterator iter = mBounds.begin(); iter != mBounds.end(); )
	{
		// Try to find it in the default one
		CScreenMap::const_iterator found = std::find(default_state.mBounds.begin(), default_state.mBounds.end(), *iter);
		if (found != default_state.mBounds.end())
		{
			// Found duplicate so delete it
			iter = mBounds.erase(iter);
			continue;
		}
		
		iter++;
	}
}

bool CWindowState::PartialCompare(const CWindowState& default_state)
{
	if (mState != default_state.mState)
		return false;

	// Check to see if screen sizes in one match any in the other
	for(CScreenMap::const_iterator iter = mBounds.begin(); iter != mBounds.end(); iter++)
	{
		// Try to find it in the default one
		CScreenMap::const_iterator found = std::find(default_state.mBounds.begin(), default_state.mBounds.end(), *iter);
		if (found == default_state.mBounds.end())
			// Not found so must be error
			return false;
	}

	return true;
}

bool CWindowState::Merge(const CWindowState& state)
{
	// Always copy over current zoom state
	bool changed = (mState != state.mState);
	mState = state.mState;

	// See if window state already exists
	const CScreenWindowSize& comp = state.mBounds.front();

	// Do not merge rects if one being merged is empty
	// This is needed because 3-pane views save state with empty bounds and
	// that should not affect the 1-pane equivalents.
	if ((comp.GetData().left != 0) ||
		(comp.GetData().right != 0) ||
		(comp.GetData().top != 0) ||
		(comp.GetData().bottom != 0))
	{
		// Look for exact match in this one
		CScreenMap::const_iterator found = GetExactRect(comp);
		bool found_equal = (found != mBounds.end()) && (*found == comp);

		if (found == mBounds.end())
		{
			// Add to this one
			mBounds.push_back(comp);
			std::sort(mBounds.begin(), mBounds.end());
			changed = true;
		}
		else if (!found_equal)
		{
			// Replace this one
			const_cast<CScreenWindowSize&>(*found) = comp;
			changed = true;
		}
	}
	
	return changed;
}

// Look for column info that best matches current screen size
const Rect& CWindowState::GetBestRect(const CWindowState& default_state)
{
	// Get dimensions of current screen
	CScreenWindowSize screen;

	// See if exact match in this item
	CScreenMap::const_iterator found = GetExactRect(screen);
	if (found != mBounds.end())
		return (*found).GetData();

	// If not default look for exact match in default
	if (this != &default_state)
	{
		found = default_state.GetExactRect(screen);
		if (found != default_state.mBounds.end())
			return (*found).GetData();
	}

	// Try closest match to this item
	found = GetClosestRect(screen);
	if (found != mBounds.end())
		return (*found).GetData();

	// If not default try closest match from default
	if (this != &default_state)
	{
		found = default_state.GetClosestRect(screen);
		if (found != default_state.mBounds.end())
			return (*found).GetData();
	}

	// Nothing close was found => create based on current size
	ResetRect();
	return (*GetExactRect(screen)).GetData();
}

CWindowState::CScreenMap::const_iterator CWindowState::GetExactRect(const CScreenWindowSize& comp) const
{
	CScreenMap::const_iterator found = mBounds.end();
	for(CScreenMap::const_iterator iter = mBounds.begin(); iter != mBounds.end(); iter++)
	{
		// Check for smaller width
		if ((*iter).GetWidth() < comp.GetWidth())
			continue;

		// Check for equal width
		else if ((*iter).GetWidth() == comp.GetWidth())
		{
			// Check for smaller height
			if ((*iter).GetHeight() < comp.GetHeight())
				// Smaller height => mark as possible candidate
				continue;

			// Check for equal height
			else if ((*iter).GetHeight() == comp.GetHeight())
			{
				// Equal width & height => exact match
				found = iter;
				break;
			}
			else
				// Larger height => gone past possible matches
				break;
		}
		else
			// Large width => gone past possible matches
			break;
	}

	return found;
}

CWindowState::CScreenMap::const_iterator CWindowState::GetClosestRect(const CScreenWindowSize& comp) const
{
	CScreenMap::const_iterator found = mBounds.end();
	for(CScreenMap::const_iterator iter = mBounds.begin(); iter != mBounds.end(); iter++)
	{
		// Check for smaller width
		if ((*iter).GetWidth() < comp.GetWidth())
			// Smaller width => mark as possible candidate
			found = iter;

		// Check for equal width
		else if ((*iter).GetWidth() == comp.GetWidth())
		{
			// Check for smaller height
			if ((*iter).GetHeight() < comp.GetHeight())
				// Smaller height => mark as possible candidate
				found = iter;

			// Check for equal height
			else if ((*iter).GetHeight() == comp.GetHeight())
			{
				// Equal width & height => exact match
				found = iter;
				break;
			}
			else
				// Larger height => gone past possible matches
				break;
		}
		else
			// Large width => gone past possible matches
			break;
	}

	return found;
}

void CWindowState::AddUniqueRect(CScreenWindowSize& item)
{
	// Scan map for unique match
	CWindowState::CScreenMap::const_iterator equal_match = mBounds.end();

	for(CWindowState::CScreenMap::const_iterator iter = mBounds.begin(); iter != mBounds.end(); iter++)
	{
		if (((*iter).GetWidth() == item.GetWidth()) &&
			((*iter).GetHeight() == item.GetHeight()))
			equal_match = iter;
	}

	if (equal_match != mBounds.end())
		// Replace existing
		const_cast<CWindowState::CScreenWindowSize&>(*equal_match) = item;
	else
	{
		// Not found so add it
		mBounds.push_back(item);
		std::sort(mBounds.begin(), mBounds.end());
	}
}

const char* cWindowStateDescriptors[] = {"", "Normal", "Maximised", "Minimised"};

cdstring CWindowState::GetInfo(void) const
{
	cdstring info = mName;
	info.quote(true);
	info.ConvertFromOS();

	// Add a bool so we can look for future items when reading in
	info += cSpace;
	info += cValueBoolTrue;

	// Add future items inside sexpression
	info += cSpace;
	info += "(";
	
	// Write out window state
	info += cWindowStateDescriptors[mState];
	if (mFuture.size())
		info += cSpace;
	
	// Write 
	info += mFuture.GetInfo();
	info += ")";

	info += cSpace;
	info += mBounds.GetInfo();

	return info;
}

bool CWindowState::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	// Clear array before reading in
	mBounds.clear();

	// Check for pre-v2.0 format
	if (::VersionTest(vers_prefs, VERS_2_0_0_A_1) < 0)
	{
		cdstring temp(txt);
		return SetInfo_Old(temp, vers_prefs);
	}

	txt.get(mName, true);

	// Look for non-sexpression => future items since the next item mBounds
	// appears as an sexpression
	if (!txt.test_start_sexpression())
	{
		// Read and ignore the bool
		bool temp;
		txt.get(temp);

		// Look for sexpression
		if (txt.start_sexpression())
		{
			// Get the window state
			char* p = txt.get();
			mState = static_cast<EWindowState>(::strindexfind(p, cWindowStateDescriptors, eWindowStateNormal));

			mFuture.SetInfo(txt, vers_prefs);
			txt.end_sexpression();
		}
		else
			mFuture.clear();
	}
	else
		mFuture.clear();

	// Get remainder
	mBounds.SetInfo(txt, vers_prefs);
	return true;
}

bool CWindowState::SetInfo_Old(cdstring& info, NumVersion vers_prefs)
{
	bool not_old_vers = true;

	char* txt = info;

	// If >= v1.3d1
	if (::VersionTest(vers_prefs, VERS_1_3_0_D_1) >= 0)
	{
		mName = ::strgetquotestr(&txt);

		// Bump past comma & space
		txt += 2;
	}
	else
		not_old_vers = false;

	// Read bounds
	Rect bounds;
	char* item = ::strtok(txt, ",");
	if (item)
		bounds.left = ::atoi(item);
	item = ::strtok(nil, ",");
	if (item)
		bounds.top = ::atoi(item);
	item = ::strtok(nil, ",");
	if (item)
		bounds.right = ::atoi(item);
	item = ::strtok(nil, ",");
	if (item)
		bounds.bottom = ::atoi(item);

	// Treat as current screen
	mBounds.clear();
	mBounds.push_back(CScreenWindowSize(bounds));

	return not_old_vers;
}

#pragma mark ____________________________CTableWindowState

// Build from window
CTableWindowState::CTableWindowState(const char* name, Rect* bounds, EWindowState state, CColumnInfoArray* col_info)
	: CWindowState(name, bounds, state)
{
	mColumnMap.push_back(CScreenColumnInfo(*col_info));
}

// Assignment with same type
void CTableWindowState::_copy(const CWindowState& copy)
{
	CWindowState::_copy(copy);
	mColumnMap = dynamic_cast<const CTableWindowState&>(copy).mColumnMap;
}

void CTableWindowState::PruneDefaults(const CWindowState& default_state)
{
	// Check to see if column infos in one match any in the other
	for(CColumnMap::iterator iter = mColumnMap.begin(); iter != mColumnMap.end(); )
	{
		// Try to find it in the default one
		CColumnMap::const_iterator found = std::find(static_cast<const CTableWindowState&>(default_state).mColumnMap.begin(),
													static_cast<const CTableWindowState&>(default_state).mColumnMap.end(), *iter);
		if (found != static_cast<const CTableWindowState&>(default_state).mColumnMap.end())
		{
			// Found duplicate so delete it
			iter = mColumnMap.erase(iter);
			continue;
		}
		
		iter++;
	}

	CWindowState::PruneDefaults(default_state);
}

bool CTableWindowState::PartialCompare(const CWindowState& default_state)
{
	// Check to see if screen sizes in one match any in the other
	for(CColumnMap::const_iterator iter = mColumnMap.begin(); iter != mColumnMap.end(); iter++)
	{
		// Try to find it in the default one
		CColumnMap::const_iterator found = std::find(static_cast<const CTableWindowState&>(default_state).mColumnMap.begin(),
													static_cast<const CTableWindowState&>(default_state).mColumnMap.end(), *iter);
		if (found == static_cast<const CTableWindowState&>(default_state).mColumnMap.end())
			// Not found so must be error
			return false;
	}

	return CWindowState::PartialCompare(default_state);
}

bool CTableWindowState::Merge(const CWindowState& istate)
{
	const CTableWindowState& state = static_cast<const CTableWindowState&>(istate);

	// Do inherited
	bool result = CWindowState::Merge(state);

	// See if window state already exists
	const CScreenColumnInfo& comp = state.mColumnMap.front();

	// Look for exact match in this one
	CColumnMap::const_iterator found = GetExactColumnInfo(comp);
	bool found_equal = (found != mColumnMap.end()) && (*found == comp);

	if (found == mColumnMap.end())
	{
		// Add to this one
		mColumnMap.push_back(comp);
		std::sort(mColumnMap.begin(), mColumnMap.end());
		return true;
	}
	else
	{
		if (found_equal)
			return result;
		else
		{
			// Replace this one
			const_cast<CScreenColumnInfo&>(*found) = comp;
			return true;
		}
	}
}

// Look for column info that best matches current screen size
const CColumnInfoArray& CTableWindowState::GetBestColumnInfo(const CTableWindowState& default_state)
{
	// Get dimensions of current screen
	CScreenColumnInfo screen;

	// See if exact match in this item
	CColumnMap::const_iterator found = GetExactColumnInfo(screen);
	if (found != mColumnMap.end())
		return (*found).GetData();

	// If not default look for exact match in default
	if (this != &default_state)
	{
		found = default_state.GetExactColumnInfo(screen);
		if (found != default_state.mColumnMap.end())
			return (*found).GetData();
	}

	// Try closest match to this item
	found = GetClosestColumnInfo(screen);
	if (found != mColumnMap.end())
		return (*found).GetData();

	// If not default try closest match from default
	if (this != &default_state)
	{
		found = default_state.GetClosestColumnInfo(screen);
		if (found != default_state.mColumnMap.end())
			return (*found).GetData();
	}

	// Nothing close was found => create based on current size
	ResetColumns();
	return (*GetExactColumnInfo(screen)).GetData();
}

CTableWindowState::CColumnMap::const_iterator CTableWindowState::GetExactColumnInfo(const CScreenColumnInfo& comp) const
{
	CColumnMap::const_iterator found = mColumnMap.end();
	for(CColumnMap::const_iterator iter = mColumnMap.begin(); iter != mColumnMap.end(); iter++)
	{
		// Check for smaller width
		if ((*iter).GetWidth() < comp.GetWidth())
			// Smaller width => mark as possible candidate
			continue;

		// Check for equal width
		else if ((*iter).GetWidth() == comp.GetWidth())
		{
			// Check for smaller height
			if ((*iter).GetHeight() < comp.GetHeight())
				// Smaller height => mark as possible candidate
				continue;

			// Check for equal height
			else if ((*iter).GetHeight() == comp.GetHeight())
			{
				// Equal width & height => exact match
				found = iter;
				break;
			}
			else
				// Larger height => gone past possible matches
				break;
		}
		else
			// Large width => gone past possible matches
			break;
	}

	return found;
}

CTableWindowState::CColumnMap::const_iterator CTableWindowState::GetClosestColumnInfo(const CScreenColumnInfo& comp) const
{
	CColumnMap::const_iterator found = mColumnMap.end();
	for(CColumnMap::const_iterator iter = mColumnMap.begin(); iter != mColumnMap.end(); iter++)
	{
		// Check for smaller width
		if ((*iter).GetWidth() < comp.GetWidth())
			// Smaller width => mark as possible candidate
			found = iter;

		// Check for equal width
		else if ((*iter).GetWidth() == comp.GetWidth())
		{
			// Check for smaller height
			if ((*iter).GetHeight() < comp.GetHeight())
				// Smaller height => mark as possible candidate
				found = iter;

			// Check for equal height
			else if ((*iter).GetHeight() == comp.GetHeight())
			{
				// Equal width & height => exact match
				found = iter;
				break;
			}
			else
				// Larger height => gone past possible matches
				break;
		}
		else
			// Large width => gone past possible matches
			break;
	}

	return found;
}

void CTableWindowState::AddUniqueColumnInfo(CScreenColumnInfo& item)
{
	// Scan map for unique match
	CTableWindowState::CColumnMap::const_iterator equal_match = mColumnMap.end();

	for(CTableWindowState::CColumnMap::const_iterator iter = mColumnMap.begin(); iter != mColumnMap.end(); iter++)
	{
		if (((*iter).GetWidth() == item.GetWidth()) &&
			((*iter).GetHeight() == item.GetHeight()))
			equal_match = iter;
	}

	if (equal_match != mColumnMap.end())
		// Replace existing
		const_cast<CTableWindowState::CScreenColumnInfo&>(*equal_match) = item;
	else
	{
		// Not found so add it
		mColumnMap.push_back(item);
		std::sort(mColumnMap.begin(), mColumnMap.end());
	}
}

cdstring CTableWindowState::GetInfo(void) const
{
	cdstring info = CWindowState::GetInfo();
	info += cSpace;
	info += mColumnMap.GetInfo();

	return info;
}

bool CTableWindowState::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	// Clear array before reading in
	mColumnMap.clear();

	// Check for pre-v2.0 format
	if (::VersionTest(vers_prefs, VERS_2_0_0_A_1) < 0)
	{
		cdstring temp(static_cast<char*>(txt));
		return SetInfo_Old(temp, vers_prefs);
	}

	CWindowState::SetInfo(txt, vers_prefs);
	mColumnMap.SetInfo(txt, vers_prefs);

	// Always sort the column info
	std::sort(mColumnMap.begin(), mColumnMap.end());

	return true;
}

