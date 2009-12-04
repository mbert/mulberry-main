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


// Forward Header for CWindowStates class

#ifndef __CWINDOWSTATESFWD__MULBERRY__
#define __CWINDOWSTATESFWD__MULBERRY__

#include "cdstring.h"
#include "prefsvector.h"

#if __framework == __jx
#include <jColor.h>
#endif
// Type of window info
enum EWindowInfo
{
	eServerWindowInfo = 1,
	eMailboxWindowInfo,
	eAddressBookWindowInfo,
	eAdbkManagerWindowInfo,
	eAdbkSearchWindowInfo
};

enum EWindowState
{
	eWindowStateNormal = 1,
	eWindowStateMax,
	eWindowStateMin
};

// Column header info
enum EServerColumn
{
	eServerColumnFlags = 1,
	eServerColumnName,
	eServerColumnTotal,
	eServerColumnNew,
	eServerColumnUnseen,
	eServerColumnAutoCheck,
	eServerColumnSize,
	eServerColumnMax = eServerColumnSize
};

// Column header info
enum EMboxColumnType
{
	eMboxColumnTo = 1,
	eMboxColumnFrom,
	eMboxColumnReplyTo,
	eMboxColumnSender,
	eMboxColumnCc,
	eMboxColumnSubject,
	eMboxColumnThread,
	eMboxColumnDateSent,
	eMboxColumnDateReceived,
	eMboxColumnSize,
	eMboxColumnFlags,
	eMboxColumnNumber,
	eMboxColumnSmart,
	eMboxColumnAttachments,
	eMboxColumnParts,
	eMboxColumnMatch,
	eMboxColumnDisconnected,
	eMboxColumnMax = eMboxColumnDisconnected
};

// Column header info
enum ERulesColumn
{
	eRulesColumnName = 1,
	eRulesColumnMax = eRulesColumnName
};

// Column header info
enum EAdbkManagerColumn
{
	eAdbkColumnName = 1,
	eAdbkColumnOpen,
	eAdbkColumnResolve,
	eAdbkColumnSearch,
	eAdbkColumnMax = eAdbkColumnSearch
};

// Column header info
enum EAddrColumn
{
	eAddrColumnName = 1,
	eAddrColumnNameLastFirst,
	eAddrColumnNickName,
	eAddrColumnEmail,
	eAddrColumnCompany,
	eAddrColumnAddress,
	eAddrColumnPhoneWork,
	eAddrColumnPhoneHome,
	eAddrColumnFax,
	eAddrColumnURLs,
	eAddrColumnNotes,
	eAddrColumnMax = eAddrColumnNotes
};

// Column header info
enum EAdbkSearchMethod
{
	eAdbkSearchMethodMulberry = 1,
	eAdbkSearchMethodLDAP,
	eAdbkSearchMethodWhoispp,
	eAdbkSearchMethodFinger,
	eAdbkSearchMethodMax = eAdbkSearchMethodFinger
};

enum EAdbkSearchCriteria
{
	eAdbkSearchCriteriaIs = 1,
	eAdbkSearchCriteriaStarts,
	eAdbkSearchCriteriaEnds,
	eAdbkSearchCriteriaContains,
	eAdbkSearchCriteriaMax = eAdbkSearchCriteriaContains
};

// Column header info
enum ECalendarStoreColumn
{
	eCalendarStoreColumnName = 1,
	eCalendarStoreColumnSubscribe,
	eCalendarStoreColumnColour,
	eCalendarStoreColumnMax = eCalendarStoreColumnColour
};

struct SColumnInfo
{
	long	column_type;
	long	column_width;

	static const char** s_descriptors;
	static long s_max;
	static long s_max_value;

	// Compare with another
	int operator==(const SColumnInfo& comp) const
		{ return (column_type == comp.column_type) && (column_width == comp.column_width); }
	int operator!=(const SColumnInfo& comp) const
		{ return !operator==(comp); }

	cdstring GetInfo() const;
	bool SetInfo(char_stream& txt, NumVersion vers_prefs);
	bool SetInfo_Old(const char** descriptors, long max, long max_value, NumVersion vers_prefs);
};

typedef prefsvector<SColumnInfo> CColumnInfoArray;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
struct STextTraitsRecord
{
	TextTraitsRecord	traits;
	
	int operator==(const STextTraitsRecord& state) const	// Dummy for template - never used
		{ return (traits.size == state.traits.size) &&
					(::PLstrcmp(traits.fontName, state.traits.fontName) == 0); }
	int operator!=(const STextTraitsRecord& state) const	// Dummy for template - never used
		{ return !operator==(state); }
};
#elif __dest_os == __win32_os
struct SLogFont
{
	LOGFONT		logfont;
	
	int operator==(const SLogFont& state) const				// Dummy for template - never used
		{ return (logfont.lfHeight == state.logfont.lfHeight) &&
					(::lstrcmp(logfont.lfFaceName, state.logfont.lfFaceName) == 0); }
	int operator!=(const SLogFont& state) const				// Dummy for template - never used
		{ return !operator==(state); }
};
#elif __dest_os == __linux_os
struct SFontInfo
{
	cdstring fontname;
	JSize size;
	int operator==(const SFontInfo& state) const				// Dummy for template - never used
		{ return (size == state.size) &&
			 (fontname == state.fontname);
		}
	int operator!=(const SFontInfo& state) const				// Dummy for template - never used
		{ return !operator==(state); }
	
};
#else
	#error __dest_os
#endif

const short strike_through = 0x0080;

struct SStyleTraits
{
	short			style;
	RGBColor		color;

	// Compare with another
	int operator==(const SStyleTraits& traits) const
		{ return ((style == traits.style) &&
#if __dest_os == __mac_os || __dest_os == __mac_os_x
					(color.red == traits.color.red) &&
					(color.green == traits.color.green) &&
					(color.blue == traits.color.blue)); }
#elif __dest_os == __win32_os
					(color == traits.color)); }
#elif __dest_os == __linux_os
(color == traits.color));}
#endif
	int operator!=(const SStyleTraits& traits) const
		{ return !operator==(traits); }
};

struct SStyleTraits2
{
	short			style;
	RGBColor		color;
	bool			usecolor;
	RGBColor		bkgcolor;
	bool			usebkgcolor;
	cdstring		name;

	// Compare with another
	int operator==(const SStyleTraits2& traits) const
		{ return ((style == traits.style) && (name == traits.name) &&
					(usecolor == traits.usecolor) && (usebkgcolor == traits.usebkgcolor) &&
#if __dest_os == __mac_os || __dest_os == __mac_os_x
					(color.red == traits.color.red) &&
					(color.green == traits.color.green) &&
					(color.blue == traits.color.blue) &&
					(bkgcolor.red == traits.bkgcolor.red) &&
					(bkgcolor.green == traits.bkgcolor.green) &&
					(bkgcolor.blue == traits.bkgcolor.blue)); }
#elif __dest_os == __win32_os || __dest_os == __linux_os
					(color == traits.color) && (bkgcolor == traits.bkgcolor)); }
#endif
	int operator!=(const SStyleTraits2& traits) const
		{ return !operator==(traits); }

	cdstring GetInfo(void) const;
	bool SetInfo(char_stream& txt, NumVersion vers_prefs);
};

typedef prefsptrvector<SStyleTraits2> CStyleTraits2List;
 
#endif
