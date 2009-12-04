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


// Header for common local definitions

#ifndef __CLOCALCOMMON__MULBERRY__
#define __CLOCALCOMMON__MULBERRY__

#include "cdstring.h"
#include <stdio.h>
#include <time.h>

#include "CUnicodeStdLib.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include <PPxFSObject.h>
typedef PPx::FSObject fspectype;
#else
typedef cdstring fspectype;
#endif
typedef const fspectype* ffspec;


bool IsRelativePath(const char* path);
bool ConvertPath(cdstring& path, bool to_relative);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
cdstring GetFullPath(const FSSpec* spec);
cdstring GetFullPath(const FSRef* ref);
void FlushLocalVolume(const char* path);

#if __dest_os == __mac_os_x
OSErr FSRefMakeFSSpec(const FSRef *ref, FSSpec *spec);
OSErr __path2fss(const char* path, FSSpec* spec);
#endif
#endif

// This is a horrible hack because of incorrect proto for mkdir in Win32 MSL
#if __dest_os == __win32_os
int __mkdir(const char* path, int mode);
#else
#define __mkdir mkdir
#endif

int chkdir(const char* path);
bool direxists(const char* path);
bool dirreadable(const char* path);
bool dirreadwriteable(const char* path);
bool fileexists(const char* path);
bool filereadable(const char* path);
bool filereadwriteable(const char* path);
void addtopath(cdstring& path, const cdstring& name);

int count_dir_contents(const char* path);
int delete_dir(const char* path);

int moverename_file(const char * old_name, const char * new_name);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
int moverename_file(const PPx::FSObject& old_spec, const PPx::FSObject& new_spec);
#endif

void report_file_error(const char* fname, int err_no);

time_t GetDefinitiveFileTime(time_t time, const char* fpath);

class LStream;

cdstring LocalFileName(const cdstring& name, char dir_delim, bool hash_encode, bool single_level = false);

void MakeSafeFileName(cdstring& name);

void TempFileSpecAttachments(fspectype& ftemp, const cdstring& name);
void TempFileSpecSecurity(fspectype& ftemp, const cdstring& name);
void TempFileSpec(fspectype& ftemp, const cdstring& name, bool attachment, bool security);

void InsertPathIntoStringWithQuotes(cdstring& result, const cdstring& cmd, const cdstring& path);

class StRemoveFile
{
public:
	StRemoveFile()
		{ }
	StRemoveFile(const char* filename)
		{ mFileName = filename; }
	~StRemoveFile()
		{ if (!mFileName.empty()) ::remove_utf8(mFileName); }
	
	void release()
		{ mFileName = cdstring::null_str; }

	void set(const char* filename)
		{ mFileName = filename; }
private:
	cdstring mFileName;
};

#if __dest_os == __mac_os || __dest_os == __mac_os_x
class StRemoveFileSpec
{
public:
	StRemoveFileSpec()
		{ }
	StRemoveFileSpec(PPx::FSObject* filespec)
		{ mFileSpec = *filespec; }
	~StRemoveFileSpec()
		{ if (mFileSpec.IsValid())
			try
			{
				mFileSpec.Delete();
			}
			catch(...)
			{
				
			}
		}

	void release()
		{ mFileSpec.Invalidate(); }

	void set(PPx::FSObject* filespec)
		{ mFileSpec = *filespec; }
private:
	PPx::FSObject mFileSpec;
};
#else
class StRemoveFileSpec
{
public:
	StRemoveFileSpec()
		{ }
	StRemoveFileSpec(const char* filename)
		{ mFileName = filename; }
	~StRemoveFileSpec()
		{ if (!mFileName.empty()) ::remove_utf8(mFileName); }
	
	void release()
		{ mFileName = cdstring::null_str; }

	void set(const char* filename)
		{ mFileName = filename; }
private:
	cdstring mFileName;
};
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
const OSType cMulberryCreator = 'Mlby';
const OSType cMailboxType = 'Mbox';
const OSType cMailboxCacheType = 'Mcac';
const OSType cMailboxIndexType = 'Mind';
const OSType cMailboxRecordType = 'Mrec';
const OSType cPrefFileType = 'Pref';
const OSType cAddressBookFileType = 'AdBk';

#if __dest_os == __mac_os
extern long _fcreator, _ftype;

class StCreatorType
{
public:
	StCreatorType(long creator, long type)
		{ mSavedCreator = _fcreator; _fcreator = creator;
		  mSavedType = _ftype; _ftype = type;
		  mDoReset = true; }
	~StCreatorType()
		{ Reset(); }
	void Reset()
		{ if (mDoReset)
			{ _fcreator = mSavedCreator; _ftype = mSavedType; mDoReset = false; } }
private:
	long mSavedCreator;
	long mSavedType;
	bool mDoReset;
};

#elif __dest_os == __mac_os_x

class StCreatorType
{
public:
	StCreatorType(long creator, long type)
		{ }
	~StCreatorType()
		{ }
	void Reset()
		{ }
};
#endif
#endif

#endif
