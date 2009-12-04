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
#include "CMulberryApp.h"

#include <LaunchServices.h>

CIconRef::CIconRef()
{
	mIcons = NULL;
}

CIconRef::CIconRef(OSType creator, OSType type)
{
	mIcons = NULL;
	
	LoadIcons(creator, type);
}

CIconRef::~CIconRef()
{
	if (mIcons != NULL)
		::ReleaseIconRef(mIcons);
}

const CIconRef* CIconRef::LoadIcons(const PPx::FSObject& fsspec) const
{
	if (mIcons == NULL)
	{
		SInt16 dummy;
		if (::GetIconRefFromFileInfo(&fsspec.UseRef(), 0, NULL, kFSCatInfoNone, NULL, kIconServicesNormalUsageFlag, &mIcons, &dummy) != noErr)
			mIcons = NULL;
	}
	
	return this;
}

const CIconRef* CIconRef::LoadIcons(OSType creator, OSType type) const
{
	if (::GetIconRef(kOnSystemDisk, creator, type, &mIcons) != noErr)
		mIcons = NULL;
	
	return this;
}

const CIconRef* CIconRef::LoadIcons(const cdstring& type, const cdstring& subtype) const
{
	return this;
}

#pragma mark -

CIconRef*	CDesktopIcons::sGenericUnknown = new CIconRef();
CIconRef*	CDesktopIcons::sGenericFolder = new CIconRef(kSystemIconsCreator, kGenericFolderIcon);
CIconRef*	CDesktopIcons::sGenericDocument = new CIconRef(kSystemIconsCreator, kGenericDocumentIcon);
CIconRef*	CDesktopIcons::sGenericApplication = new CIconRef(kSystemIconsCreator, kGenericApplicationIcon);
CDesktopIcons::CIconRefMap		CDesktopIcons::sIconCache;

// Get suite from desktop
const CIconRef* CDesktopIcons::GetDesktopIcons(OSType creator, OSType type)
{
	// Look for existing cached item and return that
	const CIconRef* result = GetMatch(GetMapKey(creator, type));
	
	// Now try to create a new one
	if (result == NULL)
	{
		// Create new icon ref
		std::auto_ptr<CIconRef> icons(new CIconRef);
		icons->LoadIcons(creator, type);
		
		// Try to load items and add to cache if successful
		if (icons->GetIconRef() != NULL)
		{
			result = icons.release();
			sIconCache.insert(CIconRefMap::value_type(GetMapKey(creator, type), result));
		}
	}

	if (result != NULL)
		return result;

	if (type == 'APPL')
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
		std::auto_ptr<CIconRef> icons(new CIconRef);
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

cdstring CDesktopIcons::GetMapKey(OSType creator, OSType type)
{
	cdstring temp((const char*)&creator, 4);
	temp.append((const char*)&type, 4);
	return temp;
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

void CDesktopIcons::GetAPPLName(OSType creator, unsigned char* name)
{
	// Init as empty first
	*name = 0;

	// Only register help on OS X as Carbon on classic seems to recognise the Guide file
	if (UEnvironment::IsRunningOSX())
	{
		CFURLRef appURL;
		if (::LSFindApplicationForInfo(creator, NULL, NULL, NULL, &appURL) == noErr)
		{
			// Get name of app from URL
			CFStringRef nameApp;
			if (::LSCopyDisplayNameForURL(appURL, &nameApp) == noErr)
			{
				cdstring appl;
				appl.reserve(256);
				if (::CFStringGetCString(nameApp, appl.c_str_mod(), 256, kCFStringEncodingUTF8))
				{
					LStr255 applName(appl.c_str());
					::PLstrcpy(name, applName);
				}
			}
		}
	}
	else
	{
		FSSpec spec;
		if (GetAPPLSpec(creator, &spec) == noErr)
			::PLstrcpy(name, spec.name);
	}
}

OSErr CDesktopIcons::GetAPPLSpec(OSType creator, FSSpec* spec)
{
	// Iterate over all mounted volumes
	HParamBlockRec hpb;
	short index = 1;
	OSErr err;
	Str255 name;

	// Zero to start
	*name = '\0';

	do
	{
		hpb.volumeParam.ioNamePtr = 0;
		hpb.volumeParam.ioVRefNum = 0;
		hpb.volumeParam.ioVolIndex = index++;
		err = ::PBHGetVInfoSync(&hpb);

		if (!err)
		{
			// Get database refnum
			DTPBRec dtpb;
			short DTRefNum;

			dtpb.ioNamePtr = NULL;
			dtpb.ioVRefNum = hpb.volumeParam.ioVRefNum;
			if (::PBDTGetPath(&dtpb) != noErr)
				continue;
			DTRefNum = dtpb.ioDTRefNum;

			// Get APPL info
			dtpb.ioNamePtr = name;
			dtpb.ioDTRefNum = DTRefNum;
			dtpb.ioIndex = 0;
			dtpb.ioFileCreator = creator;
			if ((err = ::PBDTGetAPPLSync(&dtpb)) == noErr)
			{
				// Make FSSpec
				return ::FSMakeFSSpec(hpb.volumeParam.ioVRefNum, dtpb.ioAPPLParID, name, spec);
			}
			
			// May not be in currently selected desktop database
			else if (err == afpItemNotFound)
				err = noErr;
		}
	} while (!err);

	return err;
}

void CDesktopIcons::GetTypeList(OSType creator, LArray& list)
{
	// Iterate over all mounted volumes
	HParamBlockRec hpb;
	short index = 1;
	OSErr err;

	// Zero list at start
	list.RemoveItemsAt(list.GetCount(), LArray::index_First);

	do
	{
		hpb.volumeParam.ioNamePtr = 0;
		hpb.volumeParam.ioVRefNum = 0;
		hpb.volumeParam.ioVolIndex = index++;
		err = ::PBHGetVInfoSync(&hpb);

		if (!err)
		{
			// Get database refnum
			DTPBRec dtpb;
			short DTRefNum;

			dtpb.ioNamePtr = NULL;
			dtpb.ioVRefNum = hpb.volumeParam.ioVRefNum;
			if (::PBDTGetPath(&dtpb) != noErr)
				continue;
			DTRefNum = dtpb.ioDTRefNum;

			// Get type info
			dtpb.ioDTRefNum = DTRefNum;
			dtpb.ioIndex = 1;
			dtpb.ioTagInfo = 0;
			dtpb.ioFileCreator = creator;
			if (::PBDTGetIconInfoSync(&dtpb) == noErr)
			{
				// This creator exists in this database so get all types
				do
				{
					// Check for duplicates
					if (list.FetchIndexOf(&dtpb.ioFileType) == LArray::index_Bad)
						list.InsertItemsAt(1, LArray::index_Last, &dtpb.ioFileType);
					dtpb.ioIndex++;
					err = ::PBDTGetIconInfoSync(&dtpb);
				} while (!err);
				return;
			}
		}
	} while (!err);
}
