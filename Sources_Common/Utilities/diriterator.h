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


// diriterator : iterator toscan a directory

#ifndef __DIRITERATOR__MULBERRY__
#define __DIRITERATOR__MULBERRY__

#include "cdstring.h"

#if __dest_os == __linux_os
#include <sys/types.h>
#include <dirent.h>
#endif

class diriterator
{
public:
	diriterator(const char* path);
	diriterator(const char* path, bool dir);
	diriterator(const char* path, const char* extension);
	diriterator(const char* path, bool dir, const char* extension);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	diriterator(const FSSpec& spec, bool dir = false, const char* extension = NULL);
	diriterator(const FSRef& fref, bool dir = false, const char* extension = NULL);
	diriterator(const char* path, OSType creator, OSType type, const char* extension = NULL);
	diriterator(const char* path, bool dir, OSType creator, OSType type, const char* extension = NULL);
#endif
	~diriterator();

	bool next(const char** name);

	const char* get() const
		{ return mCurrent.c_str(); }
		
	bool is_dir() const
		{ return mCurrentDir; }

	void set_return_hidden_files(bool hidden)
		{ mReturnHiddenFiles = hidden; }
	bool get_return_hidden_files() const
		{ return mReturnHiddenFiles; }

private:
	cdstring mPath;
	bool mReturnDir;
	bool mReturnHiddenFiles;
	cdstring mExtension;

	cdstring mCurrent;
	bool mCurrentDir;

#if __dest_os == __mac_os
	CInfoPBRec mCpb;
	FSSpec mSpec;
	long mDirID;
	OSType mCreator;
	OSType mType;
	bool mWorking;
#elif __dest_os == __mac_os_x
	FSIterator	mIterator;
	OSType mCreator;
	OSType mType;
#elif __dest_os == __win32_os
	WIN32_FIND_DATA mFileData;
	HANDLE mSearch;
#elif __dest_os == __linux_os
	DIR* mDir;
#endif

	void _init(const char* path, bool dir, const char* extension);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	void _init(const FSSpec& spec, bool dir, const char* extension);
	void _init(const FSRef& fref, bool dir, const char* extension);
#endif
};

#endif
