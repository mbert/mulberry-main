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


// Source for CPreferences class

#include "CWindowStates.h"

#include "CPreferenceVersions.h"
#include "CStringUtils.h"
#include "CUtils.h"
#include "C3PaneWindowFwd.h"
#include "char_stream.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if __dest_os == __linux_os
#include "lround.h"
#endif

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

#pragma mark ____________________________Status Window

// Default constructor
CStatusWindowState::CStatusWindowState()
{
	mClosed = false;
}

// Build from window
CStatusWindowState::CStatusWindowState(const char* name, Rect* bounds, EWindowState state, bool closed)
		: CWindowState(name, bounds, state)
{
	mClosed = closed;
}

// Copy constructor
void CStatusWindowState::_copy(const CWindowState& copy)
{
	CWindowState::_copy(copy);
	mClosed = dynamic_cast<const CStatusWindowState&>(copy).mClosed;
}

bool CStatusWindowState::PartialCompare(const CWindowState& default_state)
{
	return (mClosed == static_cast<const CStatusWindowState&>(default_state).mClosed) &&
			CWindowState::PartialCompare(default_state);
}

bool CStatusWindowState::Merge(const CWindowState& istate)
{
	const CStatusWindowState& state = static_cast<const CStatusWindowState&>(istate);

	// Do inherited
	bool result = CWindowState::Merge(state);

	// Now copy across other items
	result = result ||
				(mClosed != state.mClosed);

	mClosed = state.mClosed;

	return result;
}

cdstring CStatusWindowState::GetInfo(void) const
{
	cdstring info = CWindowState::GetInfo();
	info += cSpace;
	info += mClosed ? cValueBoolTrue : cValueBoolFalse;

	// Got it all
	return info;
}

bool CStatusWindowState::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	// Check for pre-v2.0 format
	if (::VersionTest(vers_prefs, VERS_2_0_0_A_1) < 0)
	{
		cdstring temp(static_cast<char*>(txt));
		return SetInfo_Old(temp, vers_prefs);
	}

	CWindowState::SetInfo(txt, vers_prefs);
	txt.get(mClosed);

	return true;
}

bool CStatusWindowState::SetInfo_Old(cdstring& info, NumVersion vers_prefs)
{
	char* txt = info;
	mName = ::strgetquotestr(&txt);

	// Bump past comma & space
	txt += 2;

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
	mBounds.clear();
	mBounds.push_back(CScreenWindowSize(bounds));

	mClosed = ::atoi(::strtok(nil, ","));

	return true;
}

#pragma mark ____________________________MDI Window

#if __dest_os == __win32_os
// Default constructor
CMDIWindowState::CMDIWindowState()
{
	ResetRect();
}

// Build from window
CMDIWindowState::CMDIWindowState(const char* name, Rect* bounds, EWindowState state)
		: CWindowState(name, bounds, state)
{
}

// Copy constructor
CMDIWindowState::CMDIWindowState(const CMDIWindowState& copy)
		: CWindowState(copy)
{
}

// Default constructor
void CMDIWindowState::ResetRect()
{
	Rect bounds;
	bounds.left = 4;
	bounds.top = 4;
	bounds.right = 636;
	bounds.bottom = 450;

	CScreenWindowSize temp(bounds);
	AddUniqueRect(temp);
}

bool CMDIWindowState::PartialCompare(const CWindowState& default_state)
{
	return CWindowState::PartialCompare(default_state);
}

#endif

#pragma mark ____________________________3-pane Window

const char* c3PaneGeometryDescriptors[] =
	{"None",
	 "ListVert",
	 "ListHoriz",
	 "ItemsVert",
	 "ItemsHoriz",
	 "PreviewVert",
	 "PreviewHoriz",
	 "AllVert",
	 "AllHoriz",
	 NULL};

// Default constructor
C3PaneWindowState::C3PaneWindowState()
{
	ResetRect();
	
	mGeometry = N3Pane::eListVert;
	mShowList = true;
	mShowItems = true;
	mShowPreview = true;
	mShowStatus = true;
	mSplitter1Size = 0x10000000 / 2; // Splitter ratio of 0.5
	mSplitter2Size = 0x10000000 / 2; // Splitter ratio of 0.5
	mShowButtonToolbar = true;
}

// Build from window
C3PaneWindowState::C3PaneWindowState(const char* name, Rect* bounds, EWindowState state,
										N3Pane::EGeometry geometry, bool show_list,
										bool show_items, bool show_preview, bool show_status,
										long splitter1_size, long splitter2_size,
										bool show_button_toolbar)
		: CWindowState(name, bounds, state)
{
	
	mGeometry = geometry;
	mShowList = show_list;
	mShowItems = show_items;
	mShowPreview = show_preview;
	mShowStatus = show_status;
	mSplitter1Size = splitter1_size;
	mSplitter2Size = splitter2_size;
	mShowButtonToolbar = show_button_toolbar;
}

// Copy constructor
C3PaneWindowState::C3PaneWindowState(const C3PaneWindowState& copy)
		: CWindowState(copy)
{
	mGeometry = copy.mGeometry;
	mShowList = copy.mShowList;
	mShowItems = copy.mShowItems;
	mShowPreview = copy.mShowPreview;
	mShowStatus = copy.mShowStatus;
	mSplitter1Size = copy.mSplitter1Size;
	mSplitter2Size = copy.mSplitter2Size;
	mShowButtonToolbar = copy.mShowButtonToolbar;
}

// Default constructor
void C3PaneWindowState::ResetRect()
{
	Rect bounds;
	bounds.left = 4;
	bounds.top = 4;
	bounds.right = 636;
	bounds.bottom = 450;

	CScreenWindowSize temp(bounds);
	AddUniqueRect(temp);
}

bool C3PaneWindowState::PartialCompare(const CWindowState& default_state)
{
	return (mGeometry == static_cast<const C3PaneWindowState&>(default_state).mGeometry) &&
			(mShowList == static_cast<const C3PaneWindowState&>(default_state).mShowList) &&
			(mShowItems == static_cast<const C3PaneWindowState&>(default_state).mShowItems) &&
			(mShowPreview == static_cast<const C3PaneWindowState&>(default_state).mShowPreview) &&
			(mShowStatus == static_cast<const C3PaneWindowState&>(default_state).mShowStatus) &&
			(mSplitter1Size == static_cast<const C3PaneWindowState&>(default_state).mSplitter1Size) &&
			(mSplitter2Size == static_cast<const C3PaneWindowState&>(default_state).mSplitter2Size) &&
			(mShowButtonToolbar == static_cast<const C3PaneWindowState&>(default_state).mShowButtonToolbar) &&
			CWindowState::PartialCompare(default_state);
}

bool C3PaneWindowState::Merge(const CWindowState& istate)
{
	const C3PaneWindowState& state = static_cast<const C3PaneWindowState&>(istate);

	// Do inherited
	bool result = CWindowState::Merge(state);

	// Now copy across other items
	result = result ||
				(mGeometry != state.mGeometry) ||
				(mShowList != state.mShowList) ||
				(mShowItems != state.mShowItems) ||
				(mShowPreview != state.mShowPreview) ||
				(mShowStatus != state.mShowStatus) ||
				(mSplitter1Size != state.mSplitter1Size) ||
				(mSplitter2Size != state.mSplitter2Size) ||
				(mShowButtonToolbar != state.mShowButtonToolbar);

	mGeometry = state.mGeometry;
	mShowList = state.mShowList;
	mShowItems = state.mShowItems;
	mShowPreview = state.mShowPreview;
	mShowStatus = state.mShowStatus;
	mSplitter1Size = state.mSplitter1Size;
	mSplitter2Size = state.mSplitter2Size;
	mShowButtonToolbar = state.mShowButtonToolbar;

	return result;
}

cdstring C3PaneWindowState::GetInfo(void) const
{
	cdstring info = CWindowState::GetInfo();
	info += cSpace;

	cdstring temp = c3PaneGeometryDescriptors[mGeometry];
	temp.quote();
	info += temp;
	info += cSpace;

	info += mShowList ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;

	info += mShowItems ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;

	info += mShowPreview ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;

	info += mShowStatus ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;

	info += cdstring(mSplitter1Size);
	info += cSpace;

	info += cdstring(mSplitter2Size);
	info += cSpace;

	info += mShowButtonToolbar ? cValueBoolTrue : cValueBoolFalse;

	// Got it all
	return info;
}

bool C3PaneWindowState::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	CWindowState::SetInfo(txt, vers_prefs);

	{
		cdstring item;
		txt.get(item);
		long i = ::strindexfind(item, c3PaneGeometryDescriptors, N3Pane::eListVert);
		mGeometry = static_cast<N3Pane::EGeometry>(i);
	}

	txt.get(mShowList);
	txt.get(mShowItems);
	txt.get(mShowPreview);
	txt.get(mShowStatus);
	txt.get(mSplitter1Size);
	txt.get(mSplitter2Size);
	txt.get(mShowButtonToolbar);

	return true;
}

bool C3PaneWindowState::SetInfo_Old(cdstring& info, NumVersion vers_prefs)
{
	return true;
}

#pragma mark ____________________________Server Window

const char* cServerColumnDescriptors[] =
	{"Flags",
	 "Name",
	 "Total",
	 "New",
	 "Unseen",
	 "Favourite",
	 "Size",
	 NULL};

// Default constructor
CServerWindowState::CServerWindowState()
{
	ResetRect();
	ResetColumns();
}

// Build from window
CServerWindowState::CServerWindowState(const char* name, Rect* bounds, EWindowState state,
										CColumnInfoArray* col_info)
		: CTableWindowState(name, bounds, state, col_info)
{
}

// Assignment with same type
void CServerWindowState::_copy(const CWindowState& copy)
{
	CTableWindowState::_copy(copy);
}

// Default constructor
void CServerWindowState::ResetRect()
{
#if __dest_os != __mac_os && __dest_os != __mac_os_x
	Rect bounds;
	bounds.left = 0;
	bounds.top = 0;
	bounds.right = 342;
	bounds.bottom = 356;

	CScreenWindowSize temp(bounds);
	AddUniqueRect(temp);
#else
	CTableWindowState::ResetRect();
#endif
}

// Default constructor
void CServerWindowState::ResetColumns()
{
	// Remove existing items
	CColumnInfoArray info;

	SColumnInfo newInfo;

	// Add default columns

	// Name, width = 216/232
	newInfo.column_type = eServerColumnName;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	newInfo.column_width = 216;
#elif __dest_os == __win32_os
	newInfo.column_width = 232;
#elif __dest_os == __linux_os
	newInfo.column_width = 232;
#else
#error __dest_os
#endif
	info.push_back(newInfo);

	// Total, width = 48
	newInfo.column_type = eServerColumnTotal;
	newInfo.column_width = 48;
	info.push_back(newInfo);

	// Unseen, width = 32
	newInfo.column_type = eServerColumnUnseen;
	newInfo.column_width = 32;
	info.push_back(newInfo);

	CScreenColumnInfo temp(info);
	AddUniqueColumnInfo(temp);
}

bool CServerWindowState::PartialCompare(const CWindowState& default_state)
{
	return CTableWindowState::PartialCompare(default_state);
}

bool CServerWindowState::Merge(const CWindowState& state)
{
	// Do inherited
	return CTableWindowState::Merge(state);
}

cdstring CServerWindowState::GetInfo(void) const
{
	SColumnInfo::s_descriptors = cServerColumnDescriptors;
	return CTableWindowState::GetInfo();
}

bool CServerWindowState::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	// Check for pre-v2.0 format
	if (::VersionTest(vers_prefs, VERS_2_0_0_A_1) < 0)
	{
		cdstring temp(static_cast<char*>(txt));
		return SetInfo_Old(temp, vers_prefs);
	}

	SColumnInfo::s_descriptors = cServerColumnDescriptors;
	SColumnInfo::s_max = eServerColumnMax;
	SColumnInfo::s_max_value = eServerColumnFlags;
	CTableWindowState::SetInfo(txt, vers_prefs);

	return true;
}

bool CServerWindowState::SetInfo_Old(cdstring& info, NumVersion vers_prefs)
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
	mBounds.clear();
	mBounds.push_back(CScreenWindowSize(bounds));

	// If >= v1.3d1
	if (::VersionTest(vers_prefs, VERS_1_3_0_D_1) >= 0)
	{
		CColumnInfoArray colinfo;

		// Get column info count
		item = ::strtok(nil, ",");
		int col_count = 0;
		if (item)
			col_count = ::atoi(item);

		// Read column info
		for(int i = 0; i < col_count; i++)
		{
			SColumnInfo col_info;
			col_info.SetInfo_Old(cServerColumnDescriptors, eServerColumnMax, eServerColumnFlags, vers_prefs);

			colinfo.push_back(col_info);
		}

		mColumnMap.clear();
		mColumnMap.push_back(CScreenColumnInfo(colinfo));
	}
	else
		not_old_vers = false;

	return not_old_vers;
}

#pragma mark ____________________________Mailbox Window

const char* cSortByDescriptors[] = {"To",
									"From",
									"Reply To",
									"Sender",
									"Cc",
									"Subject",
									"Thread",
									"Date Sent",
									"Date Received",
									"Size",
									"Flags",
									"Number",
									"Smart Address",
									"Attachments",
									"Parts",
									"Match",
									"Disconnected",
									NULL};

const char* cShowByDescriptors[] = {"Ascending",
									"Descending",
									NULL};
const char* cViewModeDescriptors[] = {"All",
										"All-Matched",
										"Match",
										NULL};

// Default constructor
CMailboxWindowState::CMailboxWindowState()
{
	ResetRect();
	ResetColumns();

	mSortBy = cSortMessageNumber;
	mShowBy = cShowMessageAscending;
	mViewMode = NMbox::eViewMode_All;
	mSplitterSize = 0x10000000 / 2; // Splitter ratio of 0.5
}

// Build from window
CMailboxWindowState::CMailboxWindowState(const char* name, Rect* bounds, EWindowState state,
											CColumnInfoArray* col_info,
											ESortMessageBy sort_by,
											EShowMessageBy show_by,
											NMbox::EViewMode view_mode,
											CMatchItem* match,
											long splitter)
		: CTableWindowState(name, bounds, state, col_info),
		  mMatchItem(*match)
{
	mSortBy = sort_by;
	mShowBy = show_by;
	mViewMode = view_mode;
	mSplitterSize = splitter;
}

// Assignment with same type
void CMailboxWindowState::_copy(const CWindowState& copy)
{
	CTableWindowState::_copy(copy);

	mSortBy = dynamic_cast<const CMailboxWindowState&>(copy).mSortBy;
	mShowBy = dynamic_cast<const CMailboxWindowState&>(copy).mShowBy;
	mViewMode = dynamic_cast<const CMailboxWindowState&>(copy).mViewMode;
	mMatchItem = dynamic_cast<const CMailboxWindowState&>(copy).mMatchItem;
	mSplitterSize = dynamic_cast<const CMailboxWindowState&>(copy).mSplitterSize;
}

// Default constructor
void CMailboxWindowState::ResetRect()
{
#if __dest_os != __mac_os && __dest_os != __mac_os_x
	Rect bounds;
	bounds.left = 92;
	bounds.top = 0;
	bounds.right = 622;
	bounds.bottom = 336;

	CScreenWindowSize temp(bounds);
	AddUniqueRect(temp);
#else
	CTableWindowState::ResetRect();
#endif
}

// Default constructor
void CMailboxWindowState::ResetColumns(void)
{
	// Remove existing items
	CColumnInfoArray info;

	SColumnInfo newInfo;

	// Add default columns

	// Number, width = 40
	newInfo.column_type = eMboxColumnNumber;
	newInfo.column_width = 40;
	info.push_back(newInfo);

	// Flags, width = 16
	newInfo.column_type = eMboxColumnFlags;
	newInfo.column_width = 16;
	info.push_back(newInfo);

	// Attachments, width = 16
	newInfo.column_type = eMboxColumnAttachments;
	newInfo.column_width = 16;
	info.push_back(newInfo);

	// Smart, width = 104
	newInfo.column_type = eMboxColumnSmart;
	newInfo.column_width = 104;
	info.push_back(newInfo);

	// Date, width = 104
	newInfo.column_type = eMboxColumnDateSent;
	newInfo.column_width = 104;
	info.push_back(newInfo);

	// Subject, width = 204/226
	newInfo.column_type = eMboxColumnThread;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	newInfo.column_width = 204;
#elif __dest_os == __win32_os
	newInfo.column_width = 226;
#elif __dest_os == __linux_os
	newInfo.column_width = 226;
#else
#error __dest_os
#endif
	info.push_back(newInfo);

	CScreenColumnInfo temp(info);
	AddUniqueColumnInfo(temp);
}

bool CMailboxWindowState::PartialCompare(const CWindowState& default_state)
{
	return (mSortBy == static_cast<const CMailboxWindowState&>(default_state).mSortBy) &&
			(mShowBy == static_cast<const CMailboxWindowState&>(default_state).mShowBy) &&
			(mViewMode == static_cast<const CMailboxWindowState&>(default_state).mViewMode) &&
			(mMatchItem == static_cast<const CMailboxWindowState&>(default_state).mMatchItem) &&
			(mSplitterSize == static_cast<const CMailboxWindowState&>(default_state).mSplitterSize) &&
			CTableWindowState::PartialCompare(default_state);
}

bool CMailboxWindowState::Merge(const CWindowState& istate)
{
	const CMailboxWindowState& state = static_cast<const CMailboxWindowState&>(istate);

	// Do inherited
	bool result = CTableWindowState::Merge(state);

	// Now copy across other items
	result = result ||
				(mSortBy != state.mSortBy) ||
				(mShowBy != state.mShowBy) ||
				(mViewMode != state.mViewMode) ||
				!(mMatchItem == state.mMatchItem) ||
				(mSplitterSize != state.mSplitterSize);

	mSortBy = state.mSortBy;
	mShowBy = state.mShowBy;
	mViewMode = state.mViewMode;
	mMatchItem = state.mMatchItem;
	
	// Only merge when non-zero
	if (state.mSplitterSize != 0)
		mSplitterSize = state.mSplitterSize;

	return result;
}

cdstring CMailboxWindowState::GetInfo(void) const
{
	SColumnInfo::s_descriptors = cSortByDescriptors;
	cdstring info = CTableWindowState::GetInfo();
	info += cSpace;

	cdstring temp = cSortByDescriptors[mSortBy - 1];
	temp.quote();
	info += temp;
	info += cSpace;

	temp = cShowByDescriptors[mShowBy - 1];
	temp.quote();
	info += temp;
	info += cSpace;

	temp = cViewModeDescriptors[mViewMode];
	temp.quote();
	info += temp;
	info += cSpace;

	info += "(";
	info += mMatchItem.GetInfo();
	info += ")";
	info += cSpace;

	info += cdstring(mSplitterSize);

	// Got it all
	return info;
}

bool CMailboxWindowState::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	// Check for pre-v2.0 format
	if (::VersionTest(vers_prefs, VERS_2_0_0_A_1) < 0)
	{
		cdstring temp(static_cast<char*>(txt));
		return SetInfo_Old(temp, vers_prefs);
	}

	SColumnInfo::s_descriptors = cSortByDescriptors;
	SColumnInfo::s_max = cSortMessageMax;
	SColumnInfo::s_max_value = eMboxColumnNumber;
	CTableWindowState::SetInfo(txt, vers_prefs);

	{
		cdstring item;
		txt.get(item);
		long i = ::strindexfind(item, cSortByDescriptors, cSortMessageNumber - 1);
		mSortBy = static_cast<ESortMessageBy>(i + 1);
	}

	{
		cdstring item;
		txt.get(item);
		long i = ::strindexfind(item, cShowByDescriptors, cShowMessageAscending - 1);
		mShowBy = static_cast<EShowMessageBy>(i + 1);
	}

	{
		cdstring item;
		txt.get(item);
		long i = ::strindexfind(item, cViewModeDescriptors, NMbox::eViewMode_All);
		mViewMode = static_cast<NMbox::EViewMode>(i);
	}

	txt.start_sexpression();
	mMatchItem.SetInfo(txt, vers_prefs);
	txt.end_sexpression();

	txt.get(mSplitterSize);

	return true;
}

bool CMailboxWindowState::SetInfo_Old(cdstring& info, NumVersion vers_prefs)
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
	mBounds.clear();
	mBounds.push_back(CScreenWindowSize(bounds));

	// Get column info count
	CColumnInfoArray cinfo;
	item = ::strtok(nil, ",");
	int col_count = 0;
	if (item)
		col_count = ::atoi(item);

	// Read column info
	for(int i = 0; i < col_count; i++)
	{
		SColumnInfo col_info;
		col_info.SetInfo_Old(cSortByDescriptors, cSortMessageMax, eMboxColumnNumber, vers_prefs);

		// If < v1.2
		if (::VersionTest(vers_prefs, VERS_1_2_0) < 0)
		{
			// Adjust for old format
			if (col_info.column_type >= eMboxColumnCc)				// Inserted CC column
				col_info.column_type = (col_info.column_type + 1);
			if (col_info.column_type >= eMboxColumnThread)			// Inserted Thread column
				col_info.column_type = (col_info.column_type + 1);

			not_old_vers = false;
		}

		// If < v1.4
		if (::VersionTest(vers_prefs, VERS_1_4_0) < 0)
		{
			// Adjust for old format
			if (col_info.column_type >= eMboxColumnDateReceived)	// Inserted Date Received column
				col_info.column_type = (col_info.column_type + 1);

			not_old_vers = false;
		}

		cinfo.push_back(col_info);
	}

	mColumnMap.clear();
	mColumnMap.push_back(CScreenColumnInfo(cinfo));

	// If >= v1.4
	if (::VersionTest(vers_prefs, VERS_1_4_0) >= 0)
	{
		// Get sorting type and order
		item = ::strtok(nil, ",");
		if (item)
		{
			item = ::strgetquotestr(&item);
			long i = ::strindexfind(item, cSortByDescriptors, cSortMessageNumber - 1);
			mSortBy = static_cast<ESortMessageBy>(i + 1);
		}

		item = ::strtok(nil, ",");
		if (item)
		{
			item = ::strgetquotestr(&item);
			long i = ::strindexfind(item, cShowByDescriptors, cShowMessageAscending - 1);
			mShowBy = static_cast<EShowMessageBy>(i + 1);
		}

		item = ::strtok(nil, ",");
		if (item)
		{
			item = ::strgetquotestr(&item);
			long i = ::strindexfind(item, cViewModeDescriptors, NMbox::eViewMode_All);
			mViewMode = static_cast<NMbox::EViewMode>(i);
		}

		item = ::strtok(nil, "");
		if (item)
		{
			char_stream s(item);
			s.start_sexpression();
			mMatchItem.SetInfo(s, vers_prefs);
			s.end_sexpression();
		}
	}
	else
	{
		// Get sorting type and order
		item = ::strtok(nil, ",");
		if (item)
			mSortBy = (ESortMessageBy) ::atoi(item);
		// If < v1.2
		if (::VersionTest(vers_prefs, VERS_1_2_0) < 0)
		{
			// Adjust for old format
			if (mSortBy >= cSortMessageCc)
				mSortBy = (ESortMessageBy) (mSortBy + 1);
			if (mSortBy >= cSortMessageThread)
				mSortBy = (ESortMessageBy) (mSortBy + 1);

			not_old_vers = false;
		}
		// Adjust for old format
		if (mSortBy >= cSortMessageDateReceived)
			mSortBy = (ESortMessageBy) (mSortBy + 1);

		if ((mSortBy < 1) || (mSortBy > cSortMessageMax))
			mSortBy = cSortMessageNumber;

		item = ::strtok(nil, ",");
		if (item)
			mShowBy = (EShowMessageBy) ::atoi(item);
		if ((mShowBy < 1) || (mShowBy > cShowMessageMax))
			mShowBy = cShowMessageAscending;
	}


	return not_old_vers;
}

#pragma mark ____________________________Message Window

// Default constructor
CMessageWindowState::CMessageWindowState()
{
	ResetRect();

	mSplitChange = 0;
	mPartsTwisted = false;
	mCollapsed = false;
	mFlat = true;
	mInline = false;
}

// Build from window
CMessageWindowState::CMessageWindowState(const char* name, Rect* bounds, EWindowState state,
										long split_change,
										bool parts_twisted,
										bool collapsed,
										bool flat,
										bool in_line)
		: CWindowState(name, bounds, state)
{
	mSplitChange = split_change;
	mPartsTwisted = parts_twisted;
	mCollapsed = collapsed;
	mFlat = flat;
	mInline = in_line;
}

// Assignment with same type
void CMessageWindowState::_copy(const CWindowState& copy)
{
	CWindowState::_copy(copy);
	mSplitChange = dynamic_cast<const CMessageWindowState&>(copy).mSplitChange;
	mPartsTwisted = dynamic_cast<const CMessageWindowState&>(copy).mPartsTwisted;
	mCollapsed = dynamic_cast<const CMessageWindowState&>(copy).mCollapsed;
	mFlat = dynamic_cast<const CMessageWindowState&>(copy).mFlat;
	mInline = dynamic_cast<const CMessageWindowState&>(copy).mInline;
}

// Default constructor
void CMessageWindowState::ResetRect()
{
#if __dest_os != __mac_os && __dest_os != __mac_os_x
	Rect bounds;
	bounds.left = 0;
	bounds.top = 18;
	bounds.right = 574;
	bounds.bottom = 384;

	CScreenWindowSize temp(bounds);
	AddUniqueRect(temp);
#else
	CWindowState::ResetRect();
#endif
}

bool CMessageWindowState::PartialCompare(const CWindowState& default_state)
{
	return (mSplitChange == static_cast<const CMessageWindowState&>(default_state).mSplitChange) &&
			(mPartsTwisted == static_cast<const CMessageWindowState&>(default_state).mPartsTwisted) &&
			(mCollapsed == static_cast<const CMessageWindowState&>(default_state).mCollapsed) &&
			(mFlat == static_cast<const CMessageWindowState&>(default_state).mFlat) &&
			(mInline == static_cast<const CMessageWindowState&>(default_state).mInline) &&
			CWindowState::PartialCompare(default_state);
}

bool CMessageWindowState::Merge(const CWindowState& istate)
{
	const CMessageWindowState& state = static_cast<const CMessageWindowState&>(istate);

	// Do inherited
	bool result = CWindowState::Merge(state);

	// Now copy across other items
	result = result ||
				(mSplitChange != state.mSplitChange) ||
				(mPartsTwisted != state.mPartsTwisted) ||
				(mCollapsed != state.mCollapsed) ||
				(mFlat != state.mFlat) ||
				(mInline != state.mInline);

	mSplitChange = state.mSplitChange;
	mPartsTwisted = state.mPartsTwisted;
	mCollapsed = state.mCollapsed;
	mFlat = state.mFlat;
	mInline = state.mInline;

	return result;
}

cdstring CMessageWindowState::GetInfo(void) const
{
	cdstring info = CWindowState::GetInfo();
	info += cSpace;

	// Add split and twist info
	info += cdstring(mSplitChange);
	info += cSpace;
	info += mPartsTwisted ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;
	info += mCollapsed ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;
	info += mFlat ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;
	info += mInline ? cValueBoolTrue : cValueBoolFalse;

	// Got it all
	return info;
}

bool CMessageWindowState::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	// Check for pre-v2.0 format
	if (::VersionTest(vers_prefs, VERS_2_0_0_A_1) < 0)
	{
		cdstring temp(static_cast<char*>(txt));
		return SetInfo_Old(temp, vers_prefs);
	}

	CWindowState::SetInfo(txt, vers_prefs);

	txt.get(mSplitChange);
	txt.get(mPartsTwisted);
	txt.get(mCollapsed);
	if (::VersionTest(vers_prefs, VERS_2_0_0_A_4) >= 0)
	{
		txt.get(mFlat);
		txt.get(mInline);
	}

	return true;
}

bool CMessageWindowState::SetInfo_Old(cdstring& info, NumVersion vers_prefs)
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
	mBounds.clear();
	mBounds.push_back(CScreenWindowSize(bounds));

	// Read split and twist info
	item = ::strtok(nil, ",");
	if (item)
		mSplitChange = ::atoi(item);
	item = ::strtok(nil, ",");
	if (item)
		mPartsTwisted = ::atoi(item);

	return not_old_vers;
}

#pragma mark ____________________________Letter Window

// Default constructor
CLetterWindowState::CLetterWindowState()
{
	ResetRect();

	mSplitChange = 0;
	mPartsTwisted = false;
	mCollapsed = false;
}

// Build from window
CLetterWindowState::CLetterWindowState(const char* name, Rect* bounds, EWindowState state,
										long split_change,
										bool parts_twisted,
										bool collapsed)
		: CWindowState(name, bounds, state)
{
	mSplitChange = split_change;
	mPartsTwisted = parts_twisted;
	mCollapsed = collapsed;
}

// Assignment with same type
void CLetterWindowState::_copy(const CWindowState& copy)
{
	CWindowState::_copy(copy);
	mSplitChange = dynamic_cast<const CLetterWindowState&>(copy).mSplitChange;
	mPartsTwisted = dynamic_cast<const CLetterWindowState&>(copy).mPartsTwisted;
	mCollapsed = dynamic_cast<const CLetterWindowState&>(copy).mCollapsed;
}

// Default constructor
void CLetterWindowState::ResetRect()
{
#if __dest_os  != __mac_os
	Rect bounds;
	bounds.left = 46;
	bounds.top = 18;
	bounds.right = 620;
	bounds.bottom = 384;

	CScreenWindowSize temp(bounds);
	AddUniqueRect(temp);
#else
	CWindowState::ResetRect();
#endif
}

bool CLetterWindowState::PartialCompare(const CWindowState& default_state)
{
	return (mSplitChange == static_cast<const CLetterWindowState&>(default_state).mSplitChange) &&
			(mPartsTwisted == static_cast<const CLetterWindowState&>(default_state).mPartsTwisted) &&
			(mCollapsed == static_cast<const CLetterWindowState&>(default_state).mCollapsed) &&
			CWindowState::PartialCompare(default_state);
}

bool CLetterWindowState::Merge(const CWindowState& istate)
{
	const CLetterWindowState& state = static_cast<const CLetterWindowState&>(istate);

	// Do inherited
	bool result = CWindowState::Merge(state);

	// Now copy across other items
	result = result ||
				(mSplitChange != state.mSplitChange) ||
				(mPartsTwisted != state.mPartsTwisted) ||
				(mCollapsed != state.mCollapsed);

	mSplitChange = state.mSplitChange;
	mPartsTwisted = state.mPartsTwisted;
	mCollapsed = state.mCollapsed;

	return result;
}

cdstring CLetterWindowState::GetInfo(void) const
{
	cdstring info = CWindowState::GetInfo();
	info += cSpace;

	// Add split and twist info
	info += cdstring(mSplitChange);
	info += cSpace;
	info += mPartsTwisted ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;
	info += mCollapsed ? cValueBoolTrue : cValueBoolFalse;

	// Got it all
	return info;
}

bool CLetterWindowState::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	// Check for pre-v2.0 format
	if (::VersionTest(vers_prefs, VERS_2_0_0_A_1) < 0)
	{
		cdstring temp(static_cast<char*>(txt));
		return SetInfo_Old(temp, vers_prefs);
	}

	CWindowState::SetInfo(txt, vers_prefs);

	txt.get(mSplitChange);
	txt.get(mPartsTwisted);
	txt.get(mCollapsed);

	return true;
}

bool CLetterWindowState::SetInfo_Old(cdstring& info, NumVersion vers_prefs)
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
	mBounds.clear();
	mBounds.push_back(CScreenWindowSize(bounds));

	// Read split and twist info
	item = ::strtok(nil, ",");
	if (item)
		mSplitChange = ::atoi(item);
	item = ::strtok(nil, ",");
	if (item)
		mPartsTwisted = ::atoi(item);

	return not_old_vers;
}

#pragma mark ____________________________Address Book Window

const char* cAdbkColumnDescriptors[] =
	{"Name",
	 "Name LastFirst",
	 "Nick Name",
	 "Email",
	 "Company",
	 "Address",
	 "Phone Work",
	 "Phone Home",
	 "Fax",
	 "URLs",
	 "Notes",
	 NULL};

// Default constructor
CAddressBookWindowState::CAddressBookWindowState()
{
	ResetRect();

	mSort = cSortAddressLastName;
	mShow = cShowAddressFullName;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mSplitChange = 0;
#elif __dest_os == __win32_os || __dest_os == __linux_os
	mAddressSize = 128;
	mGroupSize = 128;
#else
#error __dest_os
#endif
	mAddressShow = true;
	mGroupShow = true;
	mVisible = true;
}

// Build from window
CAddressBookWindowState::CAddressBookWindowState(const char* name, Rect* bounds, EWindowState state,
													ESortAddressBy sort,
													EShowAddressBy show,
#if __dest_os == __mac_os || __dest_os == __mac_os_x
													long split_change,
#elif __dest_os == __win32_os || __dest_os == __linux_os
													long address_size,
													long group_size,
#else
#error __dest_os
#endif
													bool addressShow,
													bool groupShow,
													bool visible)
		: CWindowState(name, bounds, state)
{
	mSort = sort;
	mShow = show;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mSplitChange = split_change;
#elif __dest_os == __win32_os || __dest_os == __linux_os
	mAddressSize = address_size;
	mGroupSize = group_size;
#else
#error __dest_os
#endif
	mAddressShow = addressShow;
	mGroupShow = groupShow;
	mVisible = visible;
}

// Assignment with same type
void CAddressBookWindowState::_copy(const CWindowState& copy)
{
	CWindowState::_copy(copy);
	mSort = dynamic_cast<const CAddressBookWindowState&>(copy).mSort;
	mShow = dynamic_cast<const CAddressBookWindowState&>(copy).mShow;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mSplitChange = dynamic_cast<const CAddressBookWindowState&>(copy).mSplitChange;
#elif __dest_os == __win32_os || __dest_os == __linux_os
	mAddressSize = dynamic_cast<const CAddressBookWindowState&>(copy).mAddressSize;
	mGroupSize = dynamic_cast<const CAddressBookWindowState&>(copy).mGroupSize;
#else
#error __dest_os
#endif
	mAddressShow = dynamic_cast<const CAddressBookWindowState&>(copy).mAddressShow;
	mGroupShow = dynamic_cast<const CAddressBookWindowState&>(copy).mGroupShow;
	mVisible = dynamic_cast<const CAddressBookWindowState&>(copy).mVisible;
}

// Default constructor
void CAddressBookWindowState::ResetRect()
{
#if __dest_os != __mac_os && __dest_os != __mac_os_x
	Rect bounds;
	bounds.left = 386;
	bounds.top = 0;
	bounds.right = 620;
	bounds.bottom = 374;

	CScreenWindowSize temp(bounds);
	AddUniqueRect(temp);
#else
	CWindowState::ResetRect();
#endif
}

bool CAddressBookWindowState::PartialCompare(const CWindowState& default_state)
{
	return (mSort == static_cast<const CAddressBookWindowState&>(default_state).mSort) &&
			(mShow == static_cast<const CAddressBookWindowState&>(default_state).mShow) &&
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			(mSplitChange == static_cast<const CAddressBookWindowState&>(default_state).mSplitChange) &&
#elif __dest_os == __win32_os || __dest_os == __linux_os
			(mAddressSize == static_cast<const CAddressBookWindowState&>(default_state).mAddressSize) &&
			(mGroupSize == static_cast<const CAddressBookWindowState&>(default_state).mGroupSize) &&
#else
#error __dest_os
#endif
			(mAddressShow == static_cast<const CAddressBookWindowState&>(default_state).mAddressShow) &&
			(mGroupShow == static_cast<const CAddressBookWindowState&>(default_state).mGroupShow) &&
			(mVisible == static_cast<const CAddressBookWindowState&>(default_state).mVisible) &&
			CWindowState::PartialCompare(default_state);
}

bool CAddressBookWindowState::Merge(const CWindowState& istate)
{
	const CAddressBookWindowState& state = static_cast<const CAddressBookWindowState&>(istate);

	// Do inherited
	bool result = CWindowState::Merge(state);

	// Now copy across other items
	result = result ||
				(mSort != state.mSort) ||
				(mShow != state.mShow) ||
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				(mSplitChange != state.mSplitChange) ||
#elif __dest_os == __win32_os || __dest_os == __linux_os
				(mAddressSize != state.mAddressSize) ||
				(mGroupSize != state.mGroupSize) ||
#else
#error __dest_os
#endif
				(mAddressShow != state.mAddressShow) ||
				(mGroupShow != state.mGroupShow) ||
				(mVisible != state.mVisible);

	mSort = state.mSort;
	mShow = state.mShow;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mSplitChange = state.mSplitChange;
#elif __dest_os == __win32_os || __dest_os == __linux_os
	mAddressSize = state.mAddressSize;
	mGroupSize = state.mGroupSize;
#else
#error __dest_os
#endif
	mAddressShow = state.mAddressShow;
	mGroupShow = state.mGroupShow;
	mVisible = state.mVisible;

	return result;
}

cdstring CAddressBookWindowState::GetInfo(void) const
{
	cdstring info = CWindowState::GetInfo();
	info += cSpace;

	// Add address book info
	info += cdstring((long) mSort);
	info += cSpace;
	info += cdstring((long) mShow);
	info += cSpace;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	info += cdstring(mSplitChange);
	info += cSpace;
#elif __dest_os == __win32_os || __dest_os == __linux_os
	info += cdstring(mAddressSize);
	info += cSpace;
	info += cdstring(mGroupSize);
	info += cSpace;
#else
#error __dest_os
#endif
	info += mAddressShow ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;
	info += mGroupShow ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;
	info += mVisible ? cValueBoolTrue : cValueBoolFalse;

	// Got it all
	return info;
}

bool CAddressBookWindowState::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	// Check for pre-v2.0 format
	if (::VersionTest(vers_prefs, VERS_2_0_0_A_1) < 0)
	{
		cdstring temp(static_cast<char*>(txt));
		return SetInfo_Old(temp, vers_prefs);
	}

	CWindowState::SetInfo(txt, vers_prefs);

	long temp;
	txt.get(temp);
	mSort = (ESortAddressBy) temp;
	txt.get(temp);
	mShow = (EShowAddressBy) temp;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	txt.get(mSplitChange);
#elif __dest_os == __win32_os || __dest_os == __linux_os
	txt.get(mAddressSize);
	txt.get(mGroupSize);
#else
#error __dest_os
#endif
	txt.get(mAddressShow);
	txt.get(mGroupShow);
	txt.get(mVisible);

	return true;
}

bool CAddressBookWindowState::SetInfo_Old(cdstring& info, NumVersion vers_prefs)
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
	mBounds.clear();
	mBounds.push_back(CScreenWindowSize(bounds));

	// Read address book info
	item = ::strtok(nil, ",");
	if (item)
		mSort = (ESortAddressBy) ::atoi(item);
	item = ::strtok(nil, ",");
	if (item)
		mShow = (EShowAddressBy) ::atoi(item);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	item = ::strtok(nil, ",");
	if (item)
		mSplitChange = ::atoi(item);
#elif __dest_os == __win32_os
	item = ::strtok(nil, ",");
	if (item)
		mAddressSize = ::atoi(item);
	item = ::strtok(nil, ",");
	if (item)
		mGroupSize = ::atoi(item);
#elif __dest_os == __linux_os
	//There is no old way to do this on linux
	//(since there wasn't a linux version before 2.0)
#else
#error __dest_os
#endif
	item = ::strtok(nil, ",");
	if (item)
		mAddressShow = ::atoi(item);
	item = ::strtok(nil, ",");
	if (item)
		mGroupShow = ::atoi(item);
	item = ::strtok(nil, ",");
	if (item)
		mVisible = ::atoi(item);

	return not_old_vers;
}

#pragma mark ____________________________New Address Book Window

// Default constructor
CNewAddressBookWindowState::CNewAddressBookWindowState()
{
	ResetRect();
	ResetColumns();

	mSort = cSortAddressLastName;
	mAddressSize = (long)(0x10000000 * 0.5);
	mGroupSize = 0;
	mAddressShow = true;
	mGroupShow = true;
	mSplitterSize = 0x10000000 / 2; // Splitter ratio of 0.5
}

// Build from window
CNewAddressBookWindowState::CNewAddressBookWindowState(const char* name, Rect* bounds, EWindowState state,
										CColumnInfoArray* col_info,
										ESortAddressBy sort,
										long address_size,
										long group_size,
										bool addressShow,
										bool groupShow,
										long splitter)
		: CTableWindowState(name, bounds, state, col_info)
{
	mSort = sort;
	mAddressSize = address_size;
	mGroupSize = group_size;
	mAddressShow = addressShow;
	mGroupShow = groupShow;
	mSplitterSize = splitter;
}

// Assignment with same type
void CNewAddressBookWindowState::_copy(const CWindowState& copy)
{
	CTableWindowState::_copy(copy);

	// Copy items in list
	mSort = dynamic_cast<const CNewAddressBookWindowState&>(copy).mSort;
	mAddressSize = dynamic_cast<const CNewAddressBookWindowState&>(copy).mAddressSize;
	mGroupSize = dynamic_cast<const CNewAddressBookWindowState&>(copy).mGroupSize;
	mAddressShow = dynamic_cast<const CNewAddressBookWindowState&>(copy).mAddressShow;
	mGroupShow = dynamic_cast<const CNewAddressBookWindowState&>(copy).mGroupShow;
	mSplitterSize = dynamic_cast<const CNewAddressBookWindowState&>(copy).mSplitterSize;
}

// Default constructor
void CNewAddressBookWindowState::ResetRect()
{
#if __dest_os != __mac_os && __dest_os != __mac_os_x
	Rect bounds;
	bounds.left = 386;
	bounds.top = 0;
	bounds.right = 620;
	bounds.bottom = 374;

	CScreenWindowSize temp(bounds);
	AddUniqueRect(temp);
#else
	CTableWindowState::ResetRect();
#endif
}

// Default constructor
void CNewAddressBookWindowState::ResetColumns(void)
{
	// Remove existing items
	CColumnInfoArray info;

	SColumnInfo newInfo;

	// Add default columns

	// Name, width = 250
	newInfo.column_type = eAddrColumnName;
	newInfo.column_width = 250;
	info.push_back(newInfo);

	CScreenColumnInfo temp(info);
	AddUniqueColumnInfo(temp);
}

bool CNewAddressBookWindowState::PartialCompare(const CWindowState& default_state)
{
	return (mSort == static_cast<const CNewAddressBookWindowState&>(default_state).mSort) &&
			(mAddressSize == static_cast<const CNewAddressBookWindowState&>(default_state).mAddressSize) &&
			(mGroupSize == static_cast<const CNewAddressBookWindowState&>(default_state).mGroupSize) &&
			(mAddressShow == static_cast<const CNewAddressBookWindowState&>(default_state).mAddressShow) &&
			(mGroupShow == static_cast<const CNewAddressBookWindowState&>(default_state).mGroupShow) &&
			(mSplitterSize == static_cast<const CNewAddressBookWindowState&>(default_state).mSplitterSize) &&
			CTableWindowState::PartialCompare(default_state);
}

bool CNewAddressBookWindowState::Merge(const CWindowState& istate)
{
	const CNewAddressBookWindowState& state = static_cast<const CNewAddressBookWindowState&>(istate);

	// Do inherited
	bool result = CTableWindowState::Merge(state);

	// Now copy across other items
	result = result ||
				(mSort != state.mSort) ||
				(mAddressSize != state.mAddressSize) ||
				(mGroupSize != state.mGroupSize) ||
				(mAddressShow != state.mAddressShow) ||
				(mGroupShow != state.mGroupShow) ||
				(mSplitterSize != state.mSplitterSize);

	mSort = state.mSort;
	mAddressSize = state.mAddressSize;
	mGroupSize = state.mGroupSize;
	mAddressShow = state.mAddressShow;
	mGroupShow = state.mGroupShow;

	// Only merge when non-zero
	if (state.mSplitterSize != 0)
		mSplitterSize = state.mSplitterSize;

	return result;
}

cdstring CNewAddressBookWindowState::GetInfo(void) const
{
	SColumnInfo::s_descriptors = cAdbkColumnDescriptors;
	cdstring info = CTableWindowState::GetInfo();
	info += cSpace;

	info += cdstring((long) mSort);
	info += cSpace;

	info += cdstring(mAddressSize);
	info += cSpace;
	info += cdstring(mGroupSize);
	info += cSpace;

	info += mAddressShow ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;
	info += mGroupShow ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;

	info += cdstring(mSplitterSize);

	// Got it all
	return info;
}

bool CNewAddressBookWindowState::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	SColumnInfo::s_descriptors = cAdbkColumnDescriptors;
	SColumnInfo::s_max = eAddrColumnMax;
	SColumnInfo::s_max_value = eAddrColumnName;
	CTableWindowState::SetInfo(txt, vers_prefs);

	long temp;
	txt.get(temp);
	mSort = (ESortAddressBy) temp;
	txt.get(mAddressSize);
	txt.get(mGroupSize);
	txt.get(mAddressShow);
	txt.get(mGroupShow);
	txt.get(mSplitterSize);

	return true;
}

bool CNewAddressBookWindowState::SetInfo_Old(cdstring& info, NumVersion vers_prefs)
{
	return true;
}

#pragma mark ____________________________Address Book manager Window

const char* cAdbkManagerColumnDescriptors[] =
	{"Name",
	 "Open",
	 "Resolve",
	 "Search",
	 NULL};

// Default constructor
CAdbkManagerWindowState::CAdbkManagerWindowState()
{
	ResetRect();
	ResetColumns();
	mHide = true;
}

// Build from window
CAdbkManagerWindowState::CAdbkManagerWindowState(const char* name, Rect* bounds, EWindowState state,
										CColumnInfoArray* col_info,
										bool hide,
										cdstrvect* twisted)
		: CTableWindowState(name, bounds, state, col_info),
		  mTwisted(*twisted)
{
	mHide = hide;
}

// Assignment with same type
void CAdbkManagerWindowState::_copy(const CWindowState& copy)
{
	CTableWindowState::_copy(copy);

	// Copy items in list
	mHide = dynamic_cast<const CAdbkManagerWindowState&>(copy).mHide;
	mTwisted = dynamic_cast<const CAdbkManagerWindowState&>(copy).mTwisted;
}

// Default constructor
void CAdbkManagerWindowState::ResetRect()
{
#if __dest_os != __mac_os && __dest_os != __mac_os_x
	Rect bounds;
	bounds.left = 250;
	bounds.top = 0;
	bounds.right = 520;
	bounds.bottom = 380;

	CScreenWindowSize temp(bounds);
	AddUniqueRect(temp);
#else
	CTableWindowState::ResetRect();
#endif
}

// Default constructor
void CAdbkManagerWindowState::ResetColumns(void)
{
	// Remove existing items
	CColumnInfoArray info;

	SColumnInfo newInfo;

	// Add default columns

	// Open, width = 16
	newInfo.column_type = eAdbkColumnOpen;
	newInfo.column_width = 16;
	info.push_back(newInfo);

	// Resolve, width = 16
	newInfo.column_type = eAdbkColumnResolve;
	newInfo.column_width = 16;
	info.push_back(newInfo);

	// Search, width = 16
	newInfo.column_type = eAdbkColumnSearch;
	newInfo.column_width = 16;
	info.push_back(newInfo);

	// Name, width = 208
	newInfo.column_type = eAdbkColumnName;
	newInfo.column_width = 208;
	info.push_back(newInfo);

	CScreenColumnInfo temp(info);
	AddUniqueColumnInfo(temp);
}

bool CAdbkManagerWindowState::PartialCompare(const CWindowState& default_state)
{
	return (mHide == static_cast<const CAdbkManagerWindowState&>(default_state).mHide) &&
			(mTwisted == static_cast<const CAdbkManagerWindowState&>(default_state).mTwisted) &&
			CTableWindowState::PartialCompare(default_state);
}

bool CAdbkManagerWindowState::Merge(const CWindowState& istate)
{
	const CAdbkManagerWindowState& state = static_cast<const CAdbkManagerWindowState&>(istate);

	// Do inherited
	bool result = CTableWindowState::Merge(state);

	// Now copy across other items
	result = result ||
				(mHide != state.mHide) ||
				(mTwisted != state.mTwisted);

	mHide = state.mHide;
	mTwisted = state.mTwisted;

	return result;
}

cdstring CAdbkManagerWindowState::GetInfo(void) const
{
	SColumnInfo::s_descriptors = cAdbkManagerColumnDescriptors;
	cdstring info = CTableWindowState::GetInfo();
	info += cSpace;

	info += mHide ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;

	// Copy twisted items as S-Expression
	cdstring twisted;
	twisted.CreateSExpression(mTwisted);
	twisted.ConvertFromOS();
	info += twisted;

	// Got it all
	return info;
}

bool CAdbkManagerWindowState::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	// Check for pre-v2.0 format
	if (::VersionTest(vers_prefs, VERS_2_0_0_A_1) < 0)
	{
		cdstring temp(static_cast<char*>(txt));
		return SetInfo_Old(temp, vers_prefs);
	}

	SColumnInfo::s_descriptors = cAdbkManagerColumnDescriptors;
	SColumnInfo::s_max = eAdbkColumnMax;
	SColumnInfo::s_max_value = eAdbkColumnOpen;
	CTableWindowState::SetInfo(txt, vers_prefs);

	txt.get(mHide);
	cdstring::ParseSExpression(txt, mTwisted, true);

	return true;
}

bool CAdbkManagerWindowState::SetInfo_Old(cdstring& info, NumVersion vers_prefs)
{
	char* txt = info;
	mName = ::strgetquotestr(&txt);

	// Bump past comma & space
	txt += 2;

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
	mBounds.clear();
	mBounds.push_back(CScreenWindowSize(bounds));

	// Get column info count
	CColumnInfoArray cinfo;
	item = ::strtok(nil, ",");
	int col_count = 0;
	if (item)
		col_count = ::atoi(item);

	// Read column info
	for(int i = 0; i < col_count; i++)
	{
		SColumnInfo col_info;
		col_info.SetInfo_Old(cAdbkManagerColumnDescriptors, eAdbkColumnMax, eAdbkColumnOpen, vers_prefs);

		cinfo.push_back(col_info);
	}

	mColumnMap.clear();
	mColumnMap.push_back(CScreenColumnInfo(cinfo));

	if ((item = ::strtok(nil, ",")) != nil)
		mHide = ::atoi(item);

	// Get remainder
	cdstring temp(::strtok(nil, cdstring::null_str));
	temp.ParseSExpression(mTwisted);

	return true;
}

#pragma mark ____________________________Address Search Window


const char* cAdbkSearchMethodDescriptors[] =
	{"Mulberry",
	 "LDAP",
	 "Whois++",
	 "Finger",
	 NULL};
const char* cAdbkSearchCriteriaDescriptors[] =
	{"is",
	 "starts",
	 "ends",
	 "contains",
	 NULL};

// Default constructor
CAdbkSearchWindowState::CAdbkSearchWindowState()
{
	ResetRect();
	ResetColumns();
	mHide = false;
	mMethod = eAdbkSearchMethodMulberry;
	mField = eAddrColumnName;
	mCriteria = eAdbkSearchCriteriaContains;
}

// Build from window
CAdbkSearchWindowState::CAdbkSearchWindowState(const char* name, Rect* bounds, EWindowState state,
										CColumnInfoArray* col_info,
										EAdbkSearchMethod method,
										EAddrColumn field,
										EAdbkSearchCriteria criteria,
										bool hide)
		: CTableWindowState(name, bounds, state, col_info)
{
	mMethod = method;
	mField = field;
	mCriteria = criteria;
	mHide = hide;
}

// Assignment with same type
void CAdbkSearchWindowState::_copy(const CWindowState& copy)
{
	CTableWindowState::_copy(copy);

	// Copy items in list
	mHide = dynamic_cast<const CAdbkSearchWindowState&>(copy).mHide;
	mMethod = dynamic_cast<const CAdbkSearchWindowState&>(copy).mMethod;
	mField = dynamic_cast<const CAdbkSearchWindowState&>(copy).mField;
	mCriteria = dynamic_cast<const CAdbkSearchWindowState&>(copy).mCriteria;
}

// Default constructor
void CAdbkSearchWindowState::ResetRect()
{
#if __dest_os != __mac_os && __dest_os != __mac_os_x
	Rect bounds;
	bounds.left = 236;
	bounds.top = 40;
	bounds.right = 620;
	bounds.bottom = 300;

	CScreenWindowSize temp(bounds);
	AddUniqueRect(temp);
#else
	CTableWindowState::ResetRect();
#endif
}

// Default constructor
void CAdbkSearchWindowState::ResetColumns(void)
{
	// Remove existing items
	CColumnInfoArray info;

	SColumnInfo newInfo;

	// Add default columns

	// Name, width = 160
	newInfo.column_type = eAddrColumnName;
	newInfo.column_width = 160;
	info.push_back(newInfo);

	// Name, width = 210
	newInfo.column_type = eAddrColumnEmail;
	newInfo.column_width = 210;
	info.push_back(newInfo);

	CScreenColumnInfo temp(info);
	AddUniqueColumnInfo(temp);
}

bool CAdbkSearchWindowState::PartialCompare(const CWindowState& default_state)
{
	return (mHide == static_cast<const CAdbkSearchWindowState&>(default_state).mHide) &&
			(mMethod == static_cast<const CAdbkSearchWindowState&>(default_state).mMethod) &&
			(mField == static_cast<const CAdbkSearchWindowState&>(default_state).mField) &&
			(mCriteria == static_cast<const CAdbkSearchWindowState&>(default_state).mCriteria) &&
			CTableWindowState::PartialCompare(default_state);
}

bool CAdbkSearchWindowState::Merge(const CWindowState& istate)
{
	const CAdbkSearchWindowState& state = static_cast<const CAdbkSearchWindowState&>(istate);

	// Do inherited
	bool result = CTableWindowState::Merge(state);

	// Now copy across other items
	result = result ||
				(mHide != state.mHide) ||
				(mMethod != state.mMethod) ||
				(mField != state.mField) ||
				(mCriteria != state.mCriteria);

	mHide = state.mHide;
	mMethod = state.mMethod;
	mField = state.mField;
	mCriteria = state.mCriteria;

	return result;
}

cdstring CAdbkSearchWindowState::GetInfo(void) const
{
	SColumnInfo::s_descriptors = cAdbkColumnDescriptors;
	cdstring info = CTableWindowState::GetInfo();
	info += cSpace;

	info += mHide ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;

	cdstring temp = cAdbkSearchMethodDescriptors[mMethod - 1];
	temp.quote();
	info += temp;
	info += cSpace;

	temp = cAdbkColumnDescriptors[mField - 1];
	temp.quote();
	info += temp;
	info += cSpace;

	temp = cAdbkSearchCriteriaDescriptors[mCriteria - 1];
	temp.quote();
	info += temp;

	// Got it all
	return info;
}

bool CAdbkSearchWindowState::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	// Check for pre-v2.0 format
	if (::VersionTest(vers_prefs, VERS_2_0_0_A_1) < 0)
	{
		cdstring temp(static_cast<char*>(txt));
		return SetInfo_Old(temp, vers_prefs);
	}

	SColumnInfo::s_descriptors = cAdbkColumnDescriptors;
	SColumnInfo::s_max = eAddrColumnMax;
	SColumnInfo::s_max_value = eAddrColumnName;
	CTableWindowState::SetInfo(txt, vers_prefs);

	txt.get(mHide);

	// Get method
	{
		cdstring item;
		txt.get(item);
		long i = ::strindexfind(item, cAdbkSearchMethodDescriptors, eAdbkSearchMethodMulberry - 1);
		mMethod = static_cast<EAdbkSearchMethod>(i + 1);
	}

	// Get field
	{
		cdstring item;
		txt.get(item);
		long i = ::strindexfind(item, cAdbkColumnDescriptors, eAddrColumnName - 1);
		mField = static_cast<EAddrColumn>(i + 1);
	}

	// Get criteria
	{
		cdstring item;
		txt.get(item);
		long i = ::strindexfind(item, cAdbkSearchCriteriaDescriptors, eAdbkSearchCriteriaContains - 1);
		mCriteria = static_cast<EAdbkSearchCriteria>(i + 1);
	}

	return true;
}

bool CAdbkSearchWindowState::SetInfo_Old(cdstring& info, NumVersion vers_prefs)
{
	char* txt = info;
	mName = ::strgetquotestr(&txt);

	// Bump past comma & space
	txt += 2;

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
	mBounds.clear();
	mBounds.push_back(CScreenWindowSize(bounds));

	// Get column info count
	CColumnInfoArray cinfo;
	item = ::strtok(nil, ",");
	int col_count = 0;
	if (item)
		col_count = ::atoi(item);

	// Read column info
	for(int i = 0; i < col_count; i++)
	{
		SColumnInfo col_info;
		col_info.SetInfo_Old(cAdbkColumnDescriptors, eAddrColumnMax, eAddrColumnName, vers_prefs);

		cinfo.push_back(col_info);
	}

	mColumnMap.clear();
	mColumnMap.push_back(CScreenColumnInfo(cinfo));

	if ((item = ::strtok(nil, ",")) != nil)
		mHide = ::atoi(item);

	// Get method
	item = ::strtok(nil, ",");
	if (item)
	{
		item = ::strgetquotestr(&item);
		long i = ::strindexfind(item, cAdbkSearchMethodDescriptors, eAdbkSearchMethodMulberry - 1);
		mMethod = static_cast<EAdbkSearchMethod>(i + 1);
	}

	// Get field
	item = ::strtok(nil, ",");
	if (item)
	{
		item = ::strgetquotestr(&item);
		long i = ::strindexfind(item, cAdbkColumnDescriptors, eAddrColumnName - 1);
		mField = static_cast<EAddrColumn>(i + 1);
	}

	// Get criteria
	item = ::strtok(nil, ",");
	if (item)
	{
		item = ::strgetquotestr(&item);
		long i = ::strindexfind(item, cAdbkSearchCriteriaDescriptors, eAdbkSearchCriteriaContains - 1);
		mCriteria = static_cast<EAdbkSearchCriteria>(i + 1);
	}

	return true;
}

#pragma mark ____________________________Find & Replace Window

// Default constructor
CFindReplaceWindowState::CFindReplaceWindowState()
{
	mCaseSensitive = false;
	mBackwards = false;
	mWrap = false;
	mEntireWord = false;
}

// Build from window
CFindReplaceWindowState::CFindReplaceWindowState(const char* name, Rect* bounds, EWindowState state,
		bool case_sensitive, bool backwards, bool wrap, bool entire_word)
		: CWindowState(name, bounds, state)
{
	mCaseSensitive = case_sensitive;
	mBackwards = backwards;
	mWrap = wrap;
	mEntireWord = entire_word;
}

// Assignment with same type
void CFindReplaceWindowState::_copy(const CWindowState& copy)
{
	CWindowState::_copy(copy);
	mCaseSensitive = dynamic_cast<const CFindReplaceWindowState&>(copy).mCaseSensitive;
	mBackwards = dynamic_cast<const CFindReplaceWindowState&>(copy).mBackwards;
	mWrap = dynamic_cast<const CFindReplaceWindowState&>(copy).mWrap;
	mEntireWord = dynamic_cast<const CFindReplaceWindowState&>(copy).mEntireWord;
}

bool CFindReplaceWindowState::PartialCompare(const CWindowState& default_state)
{
	return (mCaseSensitive == static_cast<const CFindReplaceWindowState&>(default_state).mCaseSensitive) &&
			(mBackwards == static_cast<const CFindReplaceWindowState&>(default_state).mBackwards) &&
			(mWrap == static_cast<const CFindReplaceWindowState&>(default_state).mWrap) &&
			(mEntireWord == static_cast<const CFindReplaceWindowState&>(default_state).mEntireWord) &&
			CWindowState::PartialCompare(default_state);
}

bool CFindReplaceWindowState::Merge(const CWindowState& istate)
{
	const CFindReplaceWindowState& state = static_cast<const CFindReplaceWindowState&>(istate);

	// Do inherited
	bool result = CWindowState::Merge(state);

	// Now copy across other items
	result = result ||
				(mCaseSensitive != state.mCaseSensitive) ||
				(mBackwards != state.mBackwards) ||
				(mWrap != state.mWrap) ||
				(mEntireWord != state.mEntireWord);

	mCaseSensitive = state.mCaseSensitive;
	mBackwards = state.mBackwards;
	mWrap = state.mWrap;
	mEntireWord = state.mEntireWord;

	return result;
}

cdstring CFindReplaceWindowState::GetInfo(void) const
{
	cdstring info = CWindowState::GetInfo();
	info += cSpace;

	// Copy rect as string
	info += mCaseSensitive ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;

	info += mBackwards ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;

	info += mWrap ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;

	info += mEntireWord ? cValueBoolTrue : cValueBoolFalse;

	// Got it all
	return info;
}

bool CFindReplaceWindowState::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	// Check for pre-v2.0 format
	if (::VersionTest(vers_prefs, VERS_2_0_0_A_1) < 0)
	{
		cdstring temp(static_cast<char*>(txt));
		return SetInfo_Old(temp, vers_prefs);
	}

	CWindowState::SetInfo(txt, vers_prefs);

	txt.get(mCaseSensitive);
	txt.get(mBackwards);
	txt.get(mWrap);
	txt.get(mEntireWord);

	return true;
}

bool CFindReplaceWindowState::SetInfo_Old(cdstring& info, NumVersion vers_prefs)
{
	char* txt = info;
	mName = ::strgetquotestr(&txt);

	// Bump past comma & space
	txt += 2;

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
	mBounds.clear();
	mBounds.push_back(CScreenWindowSize(bounds));

	mCaseSensitive = ::atoi(::strtok(nil, ","));
	mBackwards = ::atoi(::strtok(nil, ","));
	mWrap = ::atoi(::strtok(nil, ","));
	mEntireWord = ::atoi(::strtok(nil, ","));

	return true;
}

#pragma mark ____________________________Search Window

// Default constructor
CSearchWindowState::CSearchWindowState()
{
	mExpanded = false;
}

// Build from window
CSearchWindowState::CSearchWindowState(const char* name, Rect* bounds, EWindowState state, bool expanded)
		: CWindowState(name, bounds, state)
{
	mExpanded = expanded;
}

// Assignment with same type
void CSearchWindowState::_copy(const CWindowState& copy)
{
	CWindowState::_copy(copy);
	mExpanded = dynamic_cast<const CSearchWindowState&>(copy).mExpanded;
}

bool CSearchWindowState::PartialCompare(const CWindowState& default_state)
{
	return (mExpanded == static_cast<const CSearchWindowState&>(default_state).mExpanded) &&
			CWindowState::PartialCompare(default_state);
}

bool CSearchWindowState::Merge(const CWindowState& istate)
{
	const CSearchWindowState& state = static_cast<const CSearchWindowState&>(istate);

	// Do inherited
	bool result = CWindowState::Merge(state);

	// Now copy across other items
	result = result ||
				(mExpanded != state.mExpanded);

	mExpanded = state.mExpanded;

	return result;
}

cdstring CSearchWindowState::GetInfo(void) const
{
	cdstring info = CWindowState::GetInfo();
	info += cSpace;

	info += mExpanded ? cValueBoolTrue : cValueBoolFalse;

	// Got it all
	return info;
}

bool CSearchWindowState::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	// Check for pre-v2.0 format
	if (::VersionTest(vers_prefs, VERS_2_0_0_A_1) < 0)
	{
		cdstring temp(static_cast<char*>(txt));
		return SetInfo_Old(temp, vers_prefs);
	}

	CWindowState::SetInfo(txt, vers_prefs);

	txt.get(mExpanded);

	return true;
}

bool CSearchWindowState::SetInfo_Old(cdstring& info, NumVersion vers_prefs)
{
	char* txt = info;
	mName = ::strgetquotestr(&txt);

	// Bump past comma & space
	txt += 2;

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
	mBounds.clear();
	mBounds.push_back(CScreenWindowSize(bounds));

	mExpanded = ::atoi(::strtok(nil, ","));

	return true;
}

#pragma mark ____________________________Rules Window

const char* cRulesColumnDescriptors[] =
	{"Name",
	 "Open",
	 "Apply",
	 NULL};

// Default constructor
CRulesWindowState::CRulesWindowState()
{
	ResetRect();
	ResetColumns();
	mSplitPos = 150;
	mShowTriggers = false;
	mHide = true;
}

// Build from window
CRulesWindowState::CRulesWindowState(const char* name, Rect* bounds, EWindowState state,
										CColumnInfoArray* col_info,
										long split_pos, bool show_triggers, bool hide)
		: CTableWindowState(name, bounds, state, col_info)
{
	mSplitPos = split_pos;
	mShowTriggers = show_triggers;
	mHide = hide;
}

// Assignment with same type
void CRulesWindowState::_copy(const CWindowState& copy)
{
	CTableWindowState::_copy(copy);

	// Copy items in list
	mSplitPos = dynamic_cast<const CRulesWindowState&>(copy).mSplitPos;
	mShowTriggers = dynamic_cast<const CRulesWindowState&>(copy).mShowTriggers;
	mHide = dynamic_cast<const CRulesWindowState&>(copy).mHide;
}

// Default constructor
void CRulesWindowState::ResetRect()
{
#if __dest_os != __mac_os && __dest_os != __mac_os_x
	Rect bounds;
	bounds.left = 256;
	bounds.top = 40;
	bounds.right = 556;
	bounds.bottom = 252;

	CScreenWindowSize temp(bounds);
	AddUniqueRect(temp);
#else
	CTableWindowState::ResetRect();
#endif
}

// Default constructor
void CRulesWindowState::ResetColumns(void)
{
	// Remove existing items
	CColumnInfoArray info;

	SColumnInfo newInfo;

	// Add default columns

	// Name, width = 272
	newInfo.column_type = eRulesColumnName;
	newInfo.column_width = 272;
	info.push_back(newInfo);

	CScreenColumnInfo temp(info);
	AddUniqueColumnInfo(temp);
}

bool CRulesWindowState::PartialCompare(const CWindowState& default_state)
{
	return (mSplitPos == static_cast<const CRulesWindowState&>(default_state).mSplitPos) &&
			(mShowTriggers == static_cast<const CRulesWindowState&>(default_state).mShowTriggers) &&
			(mHide == static_cast<const CRulesWindowState&>(default_state).mHide) &&
			CTableWindowState::PartialCompare(default_state);
}

bool CRulesWindowState::Merge(const CWindowState& istate)
{
	const CRulesWindowState& state = static_cast<const CRulesWindowState&>(istate);

	// Do inherited
	bool result = CTableWindowState::Merge(state);

	// Now copy across other items
	result = result ||
				(mSplitPos != state.mSplitPos) ||
				(mShowTriggers != state.mShowTriggers) ||
				(mHide != state.mHide);

	mSplitPos = state.mSplitPos;
	mShowTriggers = state.mShowTriggers;
	mHide = state.mHide;

	return result;
}

cdstring CRulesWindowState::GetInfo(void) const
{
	SColumnInfo::s_descriptors = cRulesColumnDescriptors;
	cdstring info = CTableWindowState::GetInfo();
	info += cSpace;

	info += cdstring(mSplitPos);
	info += cSpace;

	info += mShowTriggers ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;

	info += mHide ? cValueBoolTrue : cValueBoolFalse;

	// Got it all
	return info;
}

bool CRulesWindowState::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	SColumnInfo::s_descriptors = cRulesColumnDescriptors;
	SColumnInfo::s_max = eRulesColumnMax;
	SColumnInfo::s_max_value = eRulesColumnName;
	CTableWindowState::SetInfo(txt, vers_prefs);

	txt.get(mSplitPos);
	txt.get(mShowTriggers);
	txt.get(mHide);

	return true;
}

bool CRulesWindowState::SetInfo_Old(cdstring& info, NumVersion vers_prefs)
{
	return true;
}

#pragma mark ____________________________Calendar Store Window

const char* cCalendarStoreColumnDescriptors[] =
	{"Name",
	 "Subscribe",
	 "Colour",
	 NULL};

// Default constructor
CCalendarStoreWindowState::CCalendarStoreWindowState()
{
	ResetRect();
	ResetColumns();
}

// Build from window
CCalendarStoreWindowState::CCalendarStoreWindowState(const char* name, Rect* bounds, EWindowState state,
										CColumnInfoArray* col_info,
										cdstrvect* twisted)
		: CTableWindowState(name, bounds, state, col_info),
		  mTwisted(*twisted)
{
}

// Assignment with same type
void CCalendarStoreWindowState::_copy(const CWindowState& copy)
{
	// Copy items in list
	mTwisted = dynamic_cast<const CCalendarStoreWindowState&>(copy).mTwisted;
}

// Default constructor
void CCalendarStoreWindowState::ResetRect()
{
#if __dest_os != __mac_os && __dest_os != __mac_os_x
	Rect bounds;
	bounds.left = 250;
	bounds.top = 0;
	bounds.right = 520;
	bounds.bottom = 380;

	CScreenWindowSize temp(bounds);
	AddUniqueRect(temp);
#else
	CTableWindowState::ResetRect();
#endif
}

// Default constructor
void CCalendarStoreWindowState::ResetColumns(void)
{
	// Remove existing items
	CColumnInfoArray info;

	SColumnInfo newInfo;

	// Add default columns

	// Subscribe, width = 16
	newInfo.column_type = eCalendarStoreColumnSubscribe;
	newInfo.column_width = 16;
	info.push_back(newInfo);

	// Name, width = 158
	newInfo.column_type = eCalendarStoreColumnName;
	newInfo.column_width = 158;
	info.push_back(newInfo);

	// Search, width = 48
	newInfo.column_type = eCalendarStoreColumnColour;
	newInfo.column_width = 48;
	info.push_back(newInfo);

	CScreenColumnInfo temp(info);
	AddUniqueColumnInfo(temp);
}

bool CCalendarStoreWindowState::PartialCompare(const CWindowState& default_state)
{
	return (mTwisted == static_cast<const CCalendarStoreWindowState&>(default_state).mTwisted) &&
			CTableWindowState::PartialCompare(default_state);
}

bool CCalendarStoreWindowState::Merge(const CWindowState& istate)
{
	const CCalendarStoreWindowState& state = static_cast<const CCalendarStoreWindowState&>(istate);

	// Do inherited
	bool result = CTableWindowState::Merge(state);

	// Now copy across other items
	result = result ||
				(mTwisted != state.mTwisted);

	mTwisted = state.mTwisted;

	return result;
}

cdstring CCalendarStoreWindowState::GetInfo(void) const
{
	SColumnInfo::s_descriptors = cCalendarStoreColumnDescriptors;
	cdstring info = CTableWindowState::GetInfo();
	info += cSpace;

	// Copy twisted items as S-Expression
	cdstring twisted;
	twisted.CreateSExpression(mTwisted);
	twisted.ConvertFromOS();
	info += twisted;

	// Got it all
	return info;
}

bool CCalendarStoreWindowState::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	SColumnInfo::s_descriptors = cCalendarStoreColumnDescriptors;
	SColumnInfo::s_max = eAdbkColumnMax;
	SColumnInfo::s_max_value = eAdbkColumnOpen;
	CTableWindowState::SetInfo(txt, vers_prefs);

	cdstring::ParseSExpression(txt, mTwisted, true);

	return true;
}

bool CCalendarStoreWindowState::SetInfo_Old(cdstring& info, NumVersion vers_prefs)
{
	return true;
}

#pragma mark ____________________________Calendar Window

// Default constructor
CCalendarWindowState::CCalendarWindowState()
{
	mType = NCalendarView::eViewMonth;
	mShowToDo = false;
	mYearLayout = NCalendarView::e3x4;
	mDayWeekRange = CDayWeekViewTimeRange::e24Hrs;
	mDayWeekScale = 0;
	mSummaryType = NCalendarView::eList;
	mSummaryRange = NCalendarView::eToday;
	mSplitterSize = 0x10000000 / 2; // Splitter ratio of 0.5
}

// Build from window
CCalendarWindowState::CCalendarWindowState(const char* name, Rect* bounds, EWindowState state,
						NCalendarView::EViewType type, bool show_todo,
						NCalendarView::EYearLayout year_layout,
						CDayWeekViewTimeRange::ERanges	dayweek_range,
						unsigned long day_week_scale,
						NCalendarView::ESummaryType summary_type,
						NCalendarView::ESummaryRanges summary_range,
						long splitter)
		: CWindowState(name, bounds, state)
{
	mType = type;
	mShowToDo = show_todo;
	mYearLayout = year_layout;
	mDayWeekRange = dayweek_range;
	mDayWeekScale = day_week_scale;
	mSummaryType = summary_type;
	mSummaryRange = summary_range;
	mSplitterSize = splitter;
}

// Assignment with same type
void CCalendarWindowState::_copy(const CWindowState& copy)
{
	mType = dynamic_cast<const CCalendarWindowState&>(copy).mType;
	mShowToDo = dynamic_cast<const CCalendarWindowState&>(copy).mShowToDo;
	mYearLayout = dynamic_cast<const CCalendarWindowState&>(copy).mYearLayout;
	mDayWeekRange = dynamic_cast<const CCalendarWindowState&>(copy).mDayWeekRange;
	mDayWeekScale = dynamic_cast<const CCalendarWindowState&>(copy).mDayWeekScale;
	mSummaryType = dynamic_cast<const CCalendarWindowState&>(copy).mSummaryType;
	mSummaryRange = dynamic_cast<const CCalendarWindowState&>(copy).mSummaryRange;
	mSplitterSize = dynamic_cast<const CCalendarWindowState&>(copy).mSplitterSize;
}

bool CCalendarWindowState::PartialCompare(const CWindowState& default_state)
{
	return (mType == static_cast<const CCalendarWindowState&>(default_state).mType) &&
			(mShowToDo == static_cast<const CCalendarWindowState&>(default_state).mShowToDo) &&
			(mYearLayout == static_cast<const CCalendarWindowState&>(default_state).mYearLayout) &&
			(mDayWeekRange == static_cast<const CCalendarWindowState&>(default_state).mDayWeekRange) &&
			(mDayWeekScale == static_cast<const CCalendarWindowState&>(default_state).mDayWeekScale) &&
			(mSummaryType == static_cast<const CCalendarWindowState&>(default_state).mSummaryType) &&
			(mSummaryRange == static_cast<const CCalendarWindowState&>(default_state).mSummaryRange) &&
			(mSplitterSize == static_cast<const CCalendarWindowState&>(default_state).mSplitterSize) &&
			CWindowState::PartialCompare(default_state);
}

bool CCalendarWindowState::Merge(const CWindowState& istate)
{
	const CCalendarWindowState& state = static_cast<const CCalendarWindowState&>(istate);

	// Do inherited
	bool result = CWindowState::Merge(state);

	// Now copy across other items
	result = result ||
				(mType != state.mType) ||
				(mShowToDo != state.mShowToDo) ||
				(mYearLayout != state.mYearLayout) ||
				(mDayWeekRange != state.mDayWeekRange) ||
				(mDayWeekScale != state.mDayWeekScale) ||
				(mSummaryType != state.mSummaryType) ||
				(mSummaryRange != state.mSummaryRange) ||
				(mSplitterSize != state.mSplitterSize);

	mType = state.mType;
	mShowToDo = state.mShowToDo;
	mYearLayout = state.mYearLayout;
	mDayWeekRange = state.mDayWeekRange;
	mDayWeekScale = state.mDayWeekScale;
	mSummaryType = state.mSummaryType;
	mSummaryRange = state.mSummaryRange;

	// Only merge when non-zero
	if (state.mSplitterSize != 0)
		mSplitterSize = state.mSplitterSize;

	return result;
}

const char* cViewTypeDescriptors[] = {"day", "work_week", "week", "month", "year", "summary", NULL};
const char* cYearLayoutDescriptors[] = {"1x12", "2x6", "3x4", "4x3", "6x2", "12x1", NULL};
const char* cDayWeekRangeDescriptors[] = {"24Hrs", "Morning", "Afternoon", "Evening", "Awake", "Work", NULL};
const char* cSummaryTypeDescriptors[] = {"List", NULL};
const char* cSummaryRangeDescriptors[] = {"Today", "SeveralDays", "ThisWeek", "SeveralWeeks", "ThisMonth", "SeveralMonths", "ThisYear", "SeveralYears", "All", NULL};

cdstring CCalendarWindowState::GetInfo(void) const
{
	cdstring info = CWindowState::GetInfo();
	info += cSpace;

	info += cViewTypeDescriptors[mType];
	info += cSpace;

	info += mShowToDo ? cValueBoolTrue : cValueBoolFalse;
	info += cSpace;

	info += cYearLayoutDescriptors[mYearLayout];
	info += cSpace;

	info += cDayWeekRangeDescriptors[mDayWeekRange];
	info += cSpace;

	info += cdstring((unsigned long)mDayWeekScale);
	info += cSpace;

	info += cSummaryTypeDescriptors[mSummaryType];
	info += cSpace;

	info += cSummaryRangeDescriptors[mSummaryRange];
	info += cSpace;

	info += cdstring(mSplitterSize);

	// Got it all
	return info;
}

bool CCalendarWindowState::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	CWindowState::SetInfo(txt, vers_prefs);

	{
		cdstring item;
		txt.get(item);
		unsigned long i = ::strindexfind(item, cViewTypeDescriptors, NCalendarView::eViewMonth);
		mType = static_cast<NCalendarView::EViewType>(i);
	}
	txt.get(mShowToDo);
	{
		cdstring item;
		txt.get(item);
		unsigned long i = ::strindexfind(item, cYearLayoutDescriptors, NCalendarView::e3x4);
		mYearLayout = static_cast<NCalendarView::EYearLayout>(i);
	}
	{
		cdstring item;
		txt.get(item);
		unsigned long i = ::strindexfind(item, cDayWeekRangeDescriptors, CDayWeekViewTimeRange::e24Hrs);
		mDayWeekRange = static_cast<CDayWeekViewTimeRange::ERanges>(i);
	}
	txt.get(mDayWeekScale);
	{
		cdstring item;
		txt.get(item);
		unsigned long i = ::strindexfind(item, cSummaryTypeDescriptors, NCalendarView::eList);
		mSummaryType = static_cast<NCalendarView::ESummaryType>(i);
	}
	{
		cdstring item;
		txt.get(item);
		unsigned long i = ::strindexfind(item, cSummaryRangeDescriptors, NCalendarView::eToday);
		mSummaryRange = static_cast<NCalendarView::ESummaryRanges>(i);
	}

	txt.get(mSplitterSize);

	return true;
}
