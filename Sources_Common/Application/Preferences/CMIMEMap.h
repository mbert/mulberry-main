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


// Header for CMIMEMap class

#ifndef __CMIMEMAP__MULBERRY__
#define __CMIMEMAP__MULBERRY__

#include "cdstring.h"

enum EAppLaunch
{
	eAppLaunchAlways = 0,
	eAppLaunchAsk,
	eAppLaunchNever
};


// Declare vector
class CMIMEMap;
typedef std::vector<CMIMEMap> CMIMEMapVector;

class CAttachment;

class CMIMEMap
{
public:

	CMIMEMap();
	CMIMEMap(const char* type, const char* subtype,
				OSType fileCreator, OSType fileType,
				const char* fileSuffix, EAppLaunch appLaunch);
	CMIMEMap(const CMIMEMap& copy)
		{ _copy(copy); }
	~CMIMEMap() {}
	CMIMEMap& operator=(const CMIMEMap& copy)							// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	// Compare with another
	int operator==(const CMIMEMap& other) const;	// Compare with same type
	int operator!=(const CMIMEMap& other) const		// Compare with same type
					{ return !(*this == other); }
	int operator==(const char* mime) const;			// Compare with MIME type/subtype

	int operator<(const CMIMEMap& comp) const;		// Compare with same

	const char*	GetMIMEType(void) const
					{ return mMIMEType; }
	void		SetMIMEType(const char* type)
					{ mMIMEType = type; }

	const char*	GetMIMESubtype(void) const
					{ return mMIMESubtype; }
	void		SetMIMESubtype(const char* subtype)
					{ mMIMESubtype = subtype; }

	OSType		GetFileCreator(void) const
					{ return mFileCreator; }
	void		SetFileCreator(OSType creator)
					{ mFileCreator = creator; }

	OSType		GetFileType(void) const
					{ return mFileType; }
	void		SetFileType(OSType type)
					{ mFileType = type; }

	const char*	GetFileSuffix(void) const
					{ return mFileSuffix; }
	void		SetFileSuffix(const char* suffix)
					{ mFileSuffix = suffix; }

	EAppLaunch	GetAppLaunch(void) const
					{ return mAppLaunch; }
	void		SetAppLaunch(EAppLaunch appLaunch)
					{ mAppLaunch = appLaunch; }
	
	cdstring	GetInfo(void) const;
	void		SetInfo(char_stream& info);

	static const CMIMEMap* Find(const char* mime);						// Find from MIME type/subtype
	static const CMIMEMap* Find(OSType creator, OSType type);			// Find from MIME file creator & type
	static EAppLaunch FindAppLaunch(const CAttachment& attach);			// Get launch mode for MIME type

private:
	cdstring		mMIMEType;
	cdstring		mMIMESubtype;
	OSType			mFileCreator;
	OSType			mFileType;
	cdstring		mFileSuffix;
	EAppLaunch		mAppLaunch;

	void		InitMIMEMap(void);
	void		_copy(const CMIMEMap& copy);
};

#endif
