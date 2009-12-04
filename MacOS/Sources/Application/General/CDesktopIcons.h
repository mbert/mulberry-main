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

#ifndef __CDESKTOPICONS__MULBERRY__
#define __CDESKTOPICONS__MULBERRY__

#include <Icons.h>

#include <map>

// Classes

class CIconRef
{
public:
	CIconRef();
	CIconRef(OSType creator, OSType type);
	~CIconRef();
					
	const CIconRef* LoadIcons(const PPx::FSObject& fsspec) const;
	const CIconRef* LoadIcons(OSType creator, OSType type) const;
	const CIconRef* LoadIcons(const cdstring& type, const cdstring& subtype) const;

	IconRef	GetIconRef() const
	{
		return mIcons;
	}
	void SetIconRef(IconRef icons)
	{
		mIcons = icons;
	}

private:
	mutable IconRef	mIcons;
};

class CDesktopIcons
{
public:
	static const CIconRef*	GetUnknownIcons(void)
	{
		return sGenericUnknown;
	}

	static const CIconRef*	GetFolderIcons(void)
	{
		return sGenericFolder;
	}

	static const CIconRef*	GetDesktopIcons(OSType creator, OSType type);
	static const CIconRef*	GetDesktopIconsFromMIME(const cdstring& type, const cdstring& subtype);

	static void				GetAPPLName(OSType creator, unsigned char* name);
	static OSErr			GetAPPLSpec(OSType creator, FSSpec* spec);

	static void				GetTypeList(OSType creator, LArray& list);

private:
	typedef	std::map<cdstring, const CIconRef*> CIconRefMap;

	static CIconRef*	sGenericUnknown;
	static CIconRef*	sGenericFolder;
	static CIconRef*	sGenericDocument;
	static CIconRef*	sGenericApplication;
	static CIconRefMap	sIconCache;

	static cdstring GetMapKey(OSType creator, OSType type);
	static cdstring GetMapKey(const cdstring& type, const cdstring& subtype);
	static const CIconRef*	GetMatch(const cdstring& key);
};

#endif
