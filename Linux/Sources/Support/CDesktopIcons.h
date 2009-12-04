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

#include "cdstring.h"

// Classes
class JXImage;

class CIconRef
{
public:
	CIconRef();
	CIconRef(const cdstring& ext);
	~CIconRef();
					
	const CIconRef* LoadIcons(const cdstring& fsspec) const;
	const CIconRef* LoadIconsExt(const cdstring& ext) const;
	const CIconRef* LoadIcons(const cdstring& type, const cdstring& subtype) const;

	JXImage*	GetIconRef() const
	{
		return mIcon;
	}
	void SetIconRef(JXImage* icons)
	{
		mIcon = icons;
	}

private:
	mutable JXImage*	mIcon;
};

class CDesktopIcons
{
public:
	static const CIconRef*	GetUnknownIcons(void)
	{
		return sGenericUnknown;
	}

	static const CIconRef* GetFolderIcons(void)
	{
		return sGenericFolder;
	}

	static const CIconRef*	GetDesktopIcons(const cdstring& ext);
	static const CIconRef*	GetDesktopIconsFromMIME(const cdstring& type, const cdstring& subtype);

private:
	typedef	std::map<cdstring, const CIconRef*> CIconRefMap;

	static CIconRef*			sGenericUnknown;
	static CIconRef*			sGenericFolder;
	static CIconRef*			sGenericDocument;
	static CIconRef*			sGenericApplication;
	static CIconRefMap			sIconCache;

	static cdstring GetMapKey(const cdstring& type, const cdstring& subtype);
	static const CIconRef*	GetMatch(const cdstring& key);
};

#endif
