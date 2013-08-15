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


// diriterator.cp - handles scanning directory for a file

#include "diriterator.h"

#ifdef __MULBERRY
#include "CGeneralException.h"
#include "CLocalCommon.h"
#include "CLog.h"
#endif

#if __dest_os == __mac_os
#include "FullPath.h"
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "MoreFilesX.h"
#endif

#include "cdustring.h"

#pragma mark ____________________________diriterator

diriterator::diriterator(const char* path)
{
	_init(path, false, NULL);
}

diriterator::diriterator(const char* path, bool dir)
{
	_init(path, dir, NULL);
}

diriterator::diriterator(const char* path, const char* extension)
{
	_init(path, false, extension);
}

diriterator::diriterator(const char* path, bool dir, const char* extension)
{
	_init(path, dir, extension);
}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
diriterator::diriterator(const FSSpec& spec, bool dir, const char* extension)
{
	_init(spec, dir, extension);
	mCreator = 0;
	mType = 0;
}

diriterator::diriterator(const FSRef& fref, bool dir, const char* extension)
{
	_init(fref, dir, extension);
	mCreator = 0;
	mType = 0;
}

diriterator::diriterator(const char* path, OSType creator, OSType type, const char* extension)
{
	_init(path, false, extension);
	mCreator = creator;
	mType = type;
}

diriterator::diriterator(const char* path, bool dir, OSType creator, OSType type, const char* extension)
{
	_init(path, dir, extension);
	mCreator = creator;
	mType = type;
}
#endif

diriterator::~diriterator()
{
#if __dest_os == __mac_os
#elif __dest_os == __mac_os_x
	if (mIterator != NULL)
		::FSCloseIterator(mIterator);
#elif __dest_os == __win32_os
	if (mSearch != INVALID_HANDLE_VALUE)
	{
		::FindClose(mSearch);
		mSearch = INVALID_HANDLE_VALUE;
	}
#elif __dest_os == __linux_os
	if (mDir)
	{
		::closedir(mDir);
		mDir = NULL;
	}
#endif
}

bool diriterator::next(const char** name)
{
	// Always initialise to NULL
	*name = NULL;

#if __dest_os == __mac_os
	// Must still be active
	if (!mWorking)
		return false;

	// Loop because we may skip some
	while(true)
	{
		// Get info about next file in directory
		mCpb.dirInfo.ioFDirIndex++;
		mCpb.dirInfo.ioDrDirID = mDirID;
		OSErr err = ::PBGetCatInfoSync(&mCpb);
		if (err && (err != fnfErr))
		{
#ifdef __MULBERRY
			CLOG_LOGTHROW(CGeneralException, err);
			throw CGeneralException(err);
#else
			throw -1L;
#endif
		}
		if (err)
		{
			mWorking = false;
			return false;
		}

		// Get the current one
#ifdef __MULBERRY
		mCurrent = mCpb.hFileInfo.ioNamePtr;
#else
		mCurrent.assign((const char*) (mCpb.hFileInfo.ioNamePtr + 1), *mCpb.hFileInfo.ioNamePtr);
#endif

		// Test for directory
		mCurrentDir = mCpb.hFileInfo.ioFlAttrib & ioDirMask;

		// Ignore hidden files if not required
		if (!mReturnHiddenFiles &&
			(mCurrentDir && (mCpb.dirInfo.ioDrUsrWds.frFlags & kIsInvisible) ||
			 (mCpb.hFileInfo.ioFlFndrInfo.fdFlags & kIsInvisible)))
			continue;

		// Reject if dirs not wanted
		if (!mReturnDir && mCurrentDir)
			continue;

		// Always return directories regardless of name or type
		if (mCurrentDir)
			break;

		// Look for matching extension
#ifdef __MULBERRY
		if (mExtension.length() && !mCurrent.compare_end(mExtension))
			continue;
#else
		if (mExtension.length() && (mCurrent.compare(mCurrent.length() - mExtension.length(), std::string::npos, mExtension) != 0))
			continue;
#endif

		// Look for matching creator
		if (mCreator && (mCreator != mCpb.hFileInfo.ioFlFndrInfo.fdCreator))
			continue;

		// Look for matching type
		if (mType && (mType != mCpb.hFileInfo.ioFlFndrInfo.fdType))
			continue;

		// Found something - exit loop
		break;
	}
#elif __dest_os == __mac_os_x
	// Must still be active
	if (mIterator == NULL)
		return false;

	// Loop because we may skip some
	while(true)
	{
		// Get info about next file in directory
		ItemCount actualCount;
		FSCatalogInfo finfo;
		HFSUniStr255 fname;
		OSErr err = ::FSGetCatalogInfoBulk(mIterator, 1, &actualCount, NULL, kFSCatInfoNodeFlags | kFSCatInfoFinderInfo, &finfo, NULL, NULL, &fname);
		if (err && (err != errFSNoMoreItems))
		{
#ifdef __MULBERRY
			CLOG_LOGTHROW(CGeneralException, err);
			throw CGeneralException(err);
#else
			throw -1L;
#endif
		}
		if (err)
		{
			::FSCloseIterator(mIterator);
			mIterator = NULL;
			return false;
		}

		// Get the current one
		cdustring utf16;
		utf16.assign(fname.unicode, fname.length);
		mCurrent = utf16.ToUTF8();

		// Test for directory
		mCurrentDir = (finfo.nodeFlags & kFSNodeIsDirectoryMask) != 0;

		// Ignore hidden files if not required
		if (!mReturnHiddenFiles &&
			((mCurrentDir && (((FinderInfo*)&finfo.finderInfo)->folder.finderFlags & kIsInvisible)) ||
			 (((FinderInfo*)&finfo.finderInfo)->file.finderFlags & kIsInvisible)))
			continue;

		// Reject if dirs not wanted
		if (!mReturnDir && mCurrentDir)
			continue;

		// Always return directories regardless of name or type
		if (mCurrentDir)
			break;

		// Look for matching extension
		if (mExtension.length() && !mCurrent.compare_end(mExtension))
			continue;

		// Look for matching creator
		if (mCreator && (mCreator != ((FinderInfo*)&finfo.finderInfo)->file.fileCreator))
			continue;

		// Look for matching type
		if (mType && (mType != ((FinderInfo*)&finfo.finderInfo)->file.fileType))
			continue;

		// Found something - exit loop
		break;
	}
#elif __dest_os == __win32_os
	if (mSearch == INVALID_HANDLE_VALUE)
		return false;

	// Loop because we may skip some
	while(true)
	{
		// Get next item
		if (mSearch)
		{
			if (!::FindNextFile(mSearch, &mFileData))
			{
				if (::GetLastError() == ERROR_NO_MORE_FILES)
					return false;
				else
				{
					long err_last = ::GetLastError();
#ifdef __MULBERRY
					CLOG_LOGTHROW(CGeneralException, err_last);
					throw CGeneralException(err_last);
#else
					throw -1L;
#endif
				}
			}
		}
		else
		{
			cdstring search = mPath;
			search += os_dir_delim;
			search += "*";
#ifdef _UNICODE
			cdustring usearch(search);
			mSearch = ::FindFirstFile(usearch.c_str(), &mFileData);
#else
			mSearch = ::FindFirstFile(search.c_str(), &mFileData);
#endif
			if (mSearch == INVALID_HANDLE_VALUE)
				return false;
		}

		// Ignore '.', '..' and hidden items
		char p0 = mFileData.cFileName[0];
		char p1 = mFileData.cFileName[1];
		char p2 = mFileData.cFileName[2];
		if ((p0 == '.') && ((p1 == '\0') || ((p1 == '.') && (p2 == '\0'))))
			continue;

		// Ignore hidden files if not required
		if (!mReturnHiddenFiles && (mFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
			continue;

		// Get the current one
#ifdef _UNICODE
		mCurrent = mFileData.cFileName;
#else
		cdustring temp(mFileData.cFileName);
		mCurrent = temp.ToUTF8();
#endif

		// Test for directory
		mCurrentDir = mFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;

		// Reject if dirs not wanted
		if (!mReturnDir && mCurrentDir)
			continue;

		// Always return directories regardless of name or type
		if (mCurrentDir)
			break;

		// Look for matching extension
		if (mExtension.length() && !mCurrent.compare_end(mExtension))
			continue;

		// Found something - exit loop
		break;
	}

#elif __dest_os == __linux_os
	if (!mDir)
		return false;

	// Loop because we may skip some
	while(true)
	{
		struct dirent* entry;
#ifdef J_USE_READDIR_R
		cdstring buf;
		buf.reserve(sizeof(struct dirent) + _POSIX_PATH_MAX);
		struct dirent* data = (struct dirent*) buf.c_str_mod();

		entry = ::readdir_r(mDir, data);
#else
		entry = ::readdir(mDir);
#endif
		if (!entry)
			return false;	

		// Ignore '.' & '..'
		if (entry->d_name[0] == '.' && ((entry->d_name[1] == '\0') ||
			((entry->d_name[1] == '.') && (entry->d_name[2] == '\0'))))
			continue;

		// Ignore hidden files if not required
		if (!mReturnHiddenFiles && (entry->d_name[0] == '.'))
			continue;

		// Get the current one
		mCurrent = entry->d_name;

		// Test for directory
		cdstring fullpath = mPath;
		fullpath += os_dir_delim;
		fullpath += mCurrent;
		mCurrentDir = ::direxists(fullpath);

		// Reject if dirs not wanted
		if (!mReturnDir && mCurrentDir)
			continue;

		// Always return directories regardless of name or type
		if (mCurrentDir)
			break;

		// Look for matching extension
		if (mExtension.length() && !mCurrent.compare_end(mExtension))
			continue;

		// Found something - exit loop
		break;
	}
#endif

	// Return name found
	*name = mCurrent.c_str();
	return true;
}

void diriterator::_init(const char* path, bool dir, const char* extension)
{
	mPath = path;
	mReturnDir = dir;
	mReturnHiddenFiles = true;
	mExtension = extension;

	mCurrentDir = false;

#if __dest_os == __mac_os
	mWorking = false;

	FSSpec fspec;
	::FSpLocationFromFullPath(::strlen(path), path, &fspec);

	// Must check for directory - not file
	mCpb.hFileInfo.ioNamePtr = fspec.name;
	mCpb.hFileInfo.ioVRefNum = fspec.vRefNum;
	mCpb.dirInfo.ioDrDirID = fspec.parID;
	mCpb.dirInfo.ioFDirIndex = 0;

	if (::PBGetCatInfoSync(&mCpb) != noErr)
		return;
	if ((mCpb.hFileInfo.ioFlAttrib & ioDirMask) == 0)
		return;
	long dirID = mCpb.dirInfo.ioDrDirID;

	// Iterate over all files in directory looking for shared libraries
	mSpec = fspec;
	mSpec.parID = dirID;

	mCpb.hFileInfo.ioNamePtr = mSpec.name;
	mCpb.hFileInfo.ioVRefNum = mSpec.vRefNum;
	mCpb.dirInfo.ioFDirIndex = 0;
	mCpb.dirInfo.ioDrDirID = dirID;

	mDirID = dirID;
	mCreator = 0;
	mType = 0;
	mWorking = true;
#elif __dest_os == __mac_os_x
	FSRef fref;
	Boolean isdir;
	::FSPathMakeRef((const unsigned char*)path, &fref, &isdir);
	_init(fref, dir, extension);
#elif __dest_os == __win32_os
	mSearch = NULL;
#elif __dest_os == __linux_os
	mDir = ::opendir(mPath);
#endif
}


#if __dest_os == __mac_os || __dest_os == __mac_os_x
void diriterator::_init(const FSSpec& spec, bool dir, const char* extension)
{
#if __dest_os == __mac_os
	mReturnDir = dir;
	mReturnHiddenFiles = true;
	mExtension = extension;

	mCurrentDir = false;

	mWorking = false;

	// Must check for directory - not file
	mCpb.hFileInfo.ioNamePtr = const_cast<unsigned char*>(spec.name);
	mCpb.hFileInfo.ioVRefNum = spec.vRefNum;
	mCpb.dirInfo.ioDrDirID = spec.parID;
	mCpb.dirInfo.ioFDirIndex = 0;

	if (::PBGetCatInfoSync(&mCpb) != noErr)
		return;
	if ((mCpb.hFileInfo.ioFlAttrib & ioDirMask) == 0)
		return;
	long dirID = mCpb.dirInfo.ioDrDirID;

	mSpec = spec;
	mSpec.parID = dirID;

	mCpb.hFileInfo.ioNamePtr = mSpec.name;
	mCpb.hFileInfo.ioVRefNum = mSpec.vRefNum;
	mCpb.dirInfo.ioFDirIndex = 0;
	mCpb.dirInfo.ioDrDirID = dirID;

	mDirID = dirID;
	mCreator = 0;
	mType = 0;
	mWorking = true;
#else
	FSRef fref;
	::FSpMakeFSRef(&spec, &fref);
	_init(fref, dir, extension);
#endif
}

void diriterator::_init(const FSRef& fref, bool dir, const char* extension)
{
#if __dest_os == __mac_os
	FSSpec spec;
	::FSRefMakeFSSpec(&fref, &spec);
	_init(spec, dir, extension);
#else
	if (::FSOpenIterator(&fref, kFSIterateFlat, &mIterator) != noErr)
		mIterator = NULL;
	mCreator = 0;
	mType = 0;
#endif
}

#endif

