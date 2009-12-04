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


	// Source for CMIMEMap class

#include "CMIMEMap.h"

#include "CAttachment.h"
#include "CAttachmentList.h"
#include "char_stream.h"
#include "CPreferences.h"
#include "CStringUtils.h"

extern const char* cValueSpace;

CMIMEMap::CMIMEMap()
{
	// Common init
	InitMIMEMap();
}

CMIMEMap::CMIMEMap(const char* type, const char* subtype,
					OSType fileCreator, OSType fileType,
					const char* fileSuffix, EAppLaunch appLaunch)
{
	// Common init
	InitMIMEMap();

	// Set parameters
	mMIMEType = type;
	mMIMESubtype = subtype;
	mFileCreator = fileCreator;
	mFileType = fileType;
	mFileSuffix = fileSuffix;
	mAppLaunch = appLaunch;
}

void CMIMEMap::_copy(const CMIMEMap& copy)
{
	// Common init
	InitMIMEMap();

	// Duplicate items
	mMIMEType = copy.mMIMEType;
	mMIMESubtype = copy.mMIMESubtype;
	mFileCreator = copy.mFileCreator;
	mFileType = copy.mFileType;
	mFileSuffix = copy.mFileSuffix;
	mAppLaunch = copy.mAppLaunch;
}

void CMIMEMap::InitMIMEMap(void)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mFileCreator = '????';
	mFileType = '????';
#else
	mFileCreator = 0x3f3f3f3f;
	mFileType = 0x3f3f3f3f;
#endif
	mAppLaunch = eAppLaunchAsk;
}

// Compare with same type
int CMIMEMap::operator==(const CMIMEMap& other) const
{
	if ((mMIMEType != other.mMIMEType) ||
		(mMIMESubtype != other.mMIMESubtype) ||
		(mFileCreator != other.mFileCreator) ||
		(mFileType != other.mFileType) ||
		(mFileSuffix != other.mFileSuffix) ||
		(mAppLaunch != other.mAppLaunch))
		return false;

	return true;
}

// Compare with MIME type/subtype
int CMIMEMap::operator==(const char* mime) const
{
	cdstring compare = mMIMEType;
	compare += "/";
	compare += mMIMESubtype;
	return (compare == mime);
}

// Compare with same
int CMIMEMap::operator<(const CMIMEMap& comp) const
{
	int result = ::strcmp(mMIMEType, comp.mMIMEType);
	if (result)
		return (result < 0);

	result = ::strcmp(mMIMESubtype, comp.mMIMESubtype);
	if (result)
		return (result < 0);

	if (mFileCreator != comp.mFileCreator)
		return (mFileCreator < comp.mFileCreator);

	if (mFileType != comp.mFileType)
		return (mFileType < comp.mFileType);

	result = ::strcmp(mFileSuffix, comp.mFileSuffix);
	if (result)
		return (result < 0);

	return 0;
}

cdstring CMIMEMap::GetInfo(void) const
{
	cdstring all;

	cdstring info = mMIMEType;
	::strlower((char*) info.c_str());
	info.quote(true);
	all += info;
	all += cValueSpace;

	info = mMIMESubtype;
	::strlower((char*) info.c_str());
	info.quote(true);
	all += info;
	all += cValueSpace;

	info = cdstring((char*) &mFileCreator, 4);
	info.quote(true);
	all += info;
	all += cValueSpace;

	info = cdstring((char*) &mFileType, 4);
	info.quote(true);
	all += info;
	all += cValueSpace;

	info = mFileSuffix;
	info.quote(true);
	all += info;
	all += cValueSpace;

	info = cdstring((long) mAppLaunch);
	all += info;

	// Put it all together
	return all;
}

void CMIMEMap::SetInfo(char_stream& txt)
{
	txt.get(mMIMEType);
	txt += 2;
	txt.get(mMIMESubtype);
	txt += 2;
	cdstring temp;
	temp = txt.get();
	if (temp.length() == 4)
		::memcpy(&mFileCreator, temp.c_str(), 4);
	else
		mFileCreator = 0;
	txt += 2;
	temp = txt.get();
	if (temp.length() == 4)
		::memcpy(&mFileType, temp.c_str(), 4);
	else
		mFileType = 0;
	txt += 2;
	txt.get(mFileSuffix);
	txt += 2;
	mAppLaunch = (EAppLaunch) ::atol(txt.get());
}

// Find from MIME type/subtype
const CMIMEMap* CMIMEMap::Find(const char* mime)
{
	// Brute force approach
	const CMIMEMapVector* list = &CPreferences::sPrefs->mMIMEMappings.GetValue();
	const CMIMEMap* found = nil;
	for(CMIMEMapVector::const_iterator iter = list->begin(); iter != list->end(); iter++)
	{
		if (*iter == mime)
		{
			found = &(*iter);
			break;
		}
	}

	return found;
}

// Find from MIME type/subtype
const CMIMEMap* CMIMEMap::Find(OSType creator, OSType type)
{
	// Brute force approach
	const CMIMEMapVector* list = &CPreferences::sPrefs->mMIMEMappings.GetValue();
	const CMIMEMap* found = nil;
	for(CMIMEMapVector::const_iterator iter = list->begin(); iter != list->end(); iter++)
	{
		if ((iter->GetFileCreator() == creator) &&
			(iter->GetFileType() == type))
		{
			found = &(*iter);
			break;
		}
	}

	return found;
}

// Get launch mode for MIME type
EAppLaunch CMIMEMap::FindAppLaunch(const CAttachment& attach)
{
	// Get type/subtype text
	cdstring type;

	// Special for AppleDouble
	if (attach.IsMultipart() && attach.IsApplefile() && attach.GetParts())
		type = CMIMESupport::GenerateContentHeader(attach.GetParts()->back(), false, lendl, false);
	else
		type = CMIMESupport::GenerateContentHeader(&attach, false, lendl, false);

	const CMIMEMap* found = Find(type);

	if (found)
		return found->GetAppLaunch();
	else
		return CPreferences::sPrefs->mAppLaunch.GetValue();
}
