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


// CDesktopIcons.cp

#include "CDesktopIcons.h"

#include "CLog.h"

CIconRef::CIconRef()
{
	mIcon = NULL;
}

CIconRef::CIconRef(const cdstring& ext)
{
	mIcon = NULL;
	
	LoadIconsExt(ext);
}

CIconRef::~CIconRef()
{
	if (mIcon != NULL)
		::DestroyIcon(mIcon);
}

const CIconRef* CIconRef::LoadIcons(const cdstring& fsspec) const
{
	if (mIcon == NULL)
	{
		cdustring utf16(fsspec);
		SHFILEINFO finfo;
		if (::SHGetFileInfo(utf16.c_str(), 0, &finfo, sizeof(finfo), SHGFI_ICON | SHGFI_SMALLICON) != 0)
		{
			mIcon = finfo.hIcon;
		}
	}
	
	return this;
}

const CIconRef* CIconRef::LoadIconsExt(const cdstring& ext) const
{
	mIcon = NULL;
	
	return this;
}

const CIconRef* CIconRef::LoadIcons(const cdstring& type, const cdstring& subtype) const
{
	return this;
}

#pragma mark -

CIconRef*	CDesktopIcons::sGenericUnknown = new CIconRef();
CIconRef*	CDesktopIcons::sGenericFolder = new CIconRef(".fol");
CIconRef*	CDesktopIcons::sGenericDocument = new CIconRef(".txt");
CIconRef*	CDesktopIcons::sGenericApplication = new CIconRef(".exe");
CDesktopIcons::CIconRefMap	CDesktopIcons::sIconCache;

// Get suite from desktop
const CIconRef* CDesktopIcons::GetDesktopIcons(const cdstring& ext)
{
	// Look for existing cached item and return that
	const CIconRef* result = GetMatch(ext);
	
	// Now try to create a new one
	if (result == NULL)
	{
		// Create new icon ref
		auto_ptr<CIconRef> icons(new CIconRef(ext));
		
		// Try to load items and add to cache if successful
		if (icons->GetIconRef() != NULL)
		{
			result = icons.release();
			sIconCache.insert(CIconRefMap::value_type(ext, result));
		}
	}

	if (result != NULL)
		return result;

	if (ext == ".exe")
	{
		return sGenericApplication;
	}
	else
	{
		return sGenericDocument;
	}
}

// Get suite from desktop
const CIconRef* CDesktopIcons::GetDesktopIconsFromMIME(const cdstring& type, const cdstring& subtype)
{
	// Look for existing cached item and return that
	const CIconRef* result = GetMatch(GetMapKey(type, subtype));
	
	// Now try to create a new one
	if (result == NULL)
	{
		// Create new icon ref
		auto_ptr<CIconRef> icons(new CIconRef);
		icons->LoadIcons(type, subtype);
		
		// Try to load items and add to cache if successful
		if (icons->GetIconRef() != NULL)
		{
			result = icons.release();
			sIconCache.insert(CIconRefMap::value_type(GetMapKey(type, subtype), result));
		}
	}

	if (result != NULL)
		return result;

	return sGenericDocument;
}
cdstring CDesktopIcons::GetMapKey(const cdstring& type, const cdstring& subtype)
{
	cdstring temp(type);
	temp += '/';
	temp += subtype;
	return temp;
}

const CIconRef* CDesktopIcons::GetMatch(const cdstring& key)
{
	// Check all suites in cache for match
	CIconRefMap::const_iterator found = sIconCache.find(key);
	if (found != sIconCache.end())
		return (*found).second;
	else
		return NULL;
}
