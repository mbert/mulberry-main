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


// Header for COptionsMap class

#ifndef __COPTIONSMAP__MULBERRY__
#define __COPTIONSMAP__MULBERRY__

#include "cdstring.h"
#include "CACL.h"
#include "CAddressAccount.h"
#include "CCalendarAccount.h"
#include "CConnectionManager.h"
#include "CFavouriteItem.h"
#include "CIdentity.h"
#include "CINETAccount.h"
#include "CMailAccount.h"
#include "CMailNotification.h"
#include "CManageSIEVEAccount.h"
#include "CMIMEMap.h"
#include "COptionsAccount.h"
#include "CSearchStyle.h"
#include "CSMTPAccount.h"
#ifdef __use_speech
#include "CSpeechSynthesis.h"
#endif
#include "CWindowStates.h"

#include "templs.h"

typedef std::vector<RGBColor> CColourList;

// Classes

class CNotification;
class CPreferenceItem;

#if __dest_os == __win32_os
struct _RGBColor
{
	_RGBColor() : _c(0) {}
	_RGBColor(RGBColor c) : _c(c) {}
	_RGBColor(const _RGBColor& copy) : _c(copy._c) {}

	_RGBColor& operator=(RGBColor c)
		{ _c = c; return *this; }

	operator RGBColor() const { return _c; }

	RGBColor _c;
};
#else
typedef RGBColor	_RGBColor;
#endif

class COptionsMap
{

public:
	COptionsMap(void);
	virtual ~COptionsMap(void);

	// Title of this map
	virtual void SetTitle(const cdstring& title)
		{ mTitle = title; }
	virtual const cdstring& GetTitle() const
		{ return mTitle; }

	// Read/Write map
	virtual bool ReadMap(void) = 0;
	virtual bool WriteMap(bool verify) = 0;
	virtual bool IsEmpty() const = 0;

	virtual bool WriteEmptyEnums() const = 0;

	// Write with single key
	virtual bool WriteValue(const cdstring& key, const cdstring& value, bool quote = true, bool convert = true);
	virtual bool WriteValue(const cdstring& key, const cdstrvect& value);
	virtual bool WriteValue(const cdstring& key, const cdstrmap& value);
	virtual bool WriteValue(const cdstring& key, const cdstruint32map& value);
	virtual bool WriteValue(const cdstring& key, const cdstrset& value);
	virtual bool WriteValue(const cdstring& key, const CPreferenceItem& value);
#if __dest_os == __win32_os
	virtual bool WriteValue(const cdstring& key, const CStringArray& value);
#endif
	virtual bool WriteValue(const cdstring& key, char value);
	virtual bool WriteValue(const cdstring& key, short value);
	virtual bool WriteValue(const cdstring& key, long value);
	virtual bool WriteValue(const cdstring& key, int value);
	virtual bool WriteValue(const cdstring& key, bool value);
	virtual bool WriteValue(const cdstring& key, boolvector& value);
	virtual bool WriteValue(const cdstring& key, OSType value);
	virtual bool WriteValue(const cdstring& key, const SStyleTraits& value);
	virtual bool WriteValue(const cdstring& key, const _RGBColor& value);
	virtual bool WriteValue(const cdstring& key, const CColourList& value);
	virtual bool WriteValue(const cdstring& key, const CMailAccountList& value)
		{ return WriteValue(key, (const CINETAccountList&) value); }
	virtual bool WriteValue(const cdstring& key, const CSMTPAccountList& value)
		{ return WriteValue(key, (const CINETAccountList&) value); }
	virtual bool WriteValue(const cdstring& key, const COptionsAccountList& value)
		{ return WriteValue(key, (const CINETAccountList&) value); }
	virtual bool WriteValue(const cdstring& key, const CAddressAccountList& value)
		{ return WriteValue(key, (const CINETAccountList&) value); }
	virtual bool WriteValue(const cdstring& key, const CManageSIEVEAccountList& value)
		{ return WriteValue(key, (const CINETAccountList&) value); }
	virtual bool WriteValue(const cdstring& key, const CCalendarAccountList& value)
		{ return WriteValue(key, (const CINETAccountList&) value); }
	virtual bool WriteValue(const cdstring& key, const CINETAccountList& value);
	virtual bool WriteValue(const cdstring& key, const CNotification& value);
	virtual bool WriteValue(const cdstring& key, const CMailNotificationList& value);
	virtual bool WriteValue(const cdstring& key, const CFavouriteItemList& value);
	virtual bool WriteValue(const cdstring& key, const CSearchStyleList& value);
	virtual bool WriteValue(const cdstring& key, const CIdentityList& value);
	virtual bool WriteValue(const cdstring& key, const CTiedIdentityList& value);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	virtual bool WriteValue(const cdstring& key, const STextTraitsRecord& value);
#elif __dest_os == __win32_os
	virtual bool WriteValue(const cdstring& key, const SLogFont& value);
#elif __dest_os == __linux_os
	virtual bool WriteValue(const cdstring& key, const SFontInfo& value);
#else
#error __dest_os
#endif
#ifdef __use_speech
	virtual bool WriteValue(const cdstring& key, const CMessageSpeakVector& value);
#endif
	virtual bool WriteValue(const cdstring& key, const CMIMEMapVector& value);
	virtual bool WriteValue(const cdstring& key, const SACLStyleList& value);
	virtual bool WriteValue(const cdstring& key, const CWindowState& value);
	virtual bool WriteValue(const cdstring& key, const CWindowStateArray& value);
	virtual bool WriteValue(const cdstring& key, const CConnectionManager::CConnectOptions& value);
	virtual bool WriteValue(const cdstring& key, const CConnectionManager::CDisconnectOptions& value);

	// Read with single key
	virtual bool ReadValue(const cdstring& key, cdstring& value, NumVersion vers_prefs, bool convert = true);
	virtual bool ReadValue(const cdstring& key, cdstrvect& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, cdstrmap& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, cdstruint32map& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, cdstrset& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, CPreferenceItem& value, NumVersion vers_prefs);
#if __dest_os == __win32_os
	virtual bool ReadValue(const cdstring& key, CStringArray& value, NumVersion vers_prefs);
#endif
	virtual bool ReadValue(const cdstring& key, char& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, short& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, long& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, int& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, bool& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, boolvector& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, OSType& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, SStyleTraits& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, _RGBColor& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, CColourList& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, CMailAccountList& value, NumVersion vers_prefs)
		{ return ReadValue(key, (CINETAccountList&) value, vers_prefs); }
	virtual bool ReadValue(const cdstring& key, CSMTPAccountList& value, NumVersion vers_prefs)
		{ return ReadValue(key, (CINETAccountList&) value, vers_prefs); }
	virtual bool ReadValue(const cdstring& key, COptionsAccountList& value, NumVersion vers_prefs)
		{ return ReadValue(key, (CINETAccountList&) value, vers_prefs); }
	virtual bool ReadValue(const cdstring& key, CAddressAccountList& value, NumVersion vers_prefs)
		{ return ReadValue(key, (CINETAccountList&) value, vers_prefs); }
	virtual bool ReadValue(const cdstring& key, CManageSIEVEAccountList& value, NumVersion vers_prefs)
		{ return ReadValue(key, (CINETAccountList&) value, vers_prefs); }
	virtual bool ReadValue(const cdstring& key, CCalendarAccountList& value, NumVersion vers_prefs)
		{ return ReadValue(key, (CINETAccountList&) value, vers_prefs); }
	virtual bool ReadValue(const cdstring& key, CINETAccountList& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, CNotification& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, CMailNotificationList& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, CFavouriteItemList& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, CSearchStyleList& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, CIdentityList& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, CTiedIdentityList& value, NumVersion vers_prefs);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	virtual bool ReadValue(const cdstring& key, STextTraitsRecord& value, NumVersion vers_prefs);
#elif __dest_os == __win32_os
	virtual bool ReadValue(const cdstring& key, SLogFont& value, NumVersion vers_prefs);
#elif __dest_os == __linux_os
	virtual bool ReadValue(const cdstring& key, SFontInfo& value, NumVersion vers_prefs);
#else
#error __dest_os
#endif
#ifdef __use_speech
	virtual bool ReadValue(const cdstring& key, CMessageSpeakVector& value, NumVersion vers_prefs);
#endif
	virtual bool ReadValue(const cdstring& key, CMIMEMapVector& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, SACLStyleList& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, CWindowState& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, CWindowStateArray& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, CConnectionManager::CConnectOptions& value, NumVersion vers_prefs);
	virtual bool ReadValue(const cdstring& key, CConnectionManager::CDisconnectOptions& value, NumVersion vers_prefs);

	// Remove
	virtual void RemoveValue(const cdstring& key)
					{ RemoveKeyValue(key); }

	// Look for values under a key
	virtual bool EnumValue(int num, cdstring& key) = 0;
	virtual bool EnumKey(int num, cdstring& key) = 0;
	virtual bool EnumKeys(cdstrvect& found) = 0;

	// Section
	virtual void SetSection(const cdstring& section)
					{ mSection = section; mSectionStack.clear(); mSectionStack.push_back(section); }
	virtual void PushSection(const cdstring& sub_section);
	virtual void PopSection(void);
	virtual const cdstring& GetSection(void) const
					{ return mSection; }
	virtual void ClearSection(void) = 0;

	// Hierarchy separator
	virtual void SetSeparator(char separator)
					{ mSeparator = separator; }
	virtual char GetSeparator(void) const
					{ return mSeparator[(cdstring::size_type)0]; }

protected:
	cdstring		mTitle;
	cdstring		mSection;
	cdstrvect		mSectionStack;
	cdstring		mSeparator;

	virtual bool WriteKeyValue(const cdstring& key, const cdstring& value) = 0;
	virtual bool ReadKeyValue(const cdstring& key, cdstring& value) = 0;
	virtual bool RemoveKeyValue(const cdstring& key) = 0;
};

// Stack based class to handle sectioning
class StMapSection
{
public:
	StMapSection(COptionsMap* aMap, cdstring section) : mMap(aMap)
		{ mMap->PushSection(section); }
	~StMapSection()
		{ mMap->PopSection(); }
private:
	COptionsMap* mMap;
};

#endif
