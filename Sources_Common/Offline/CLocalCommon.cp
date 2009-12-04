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

#include "CLocalCommon.h"

#include "CConnectionManager.h"
#include "CErrorHandler.h"
#include "CGeneralException.h"
#include "CLog.h"
#include "CStringUtils.h"

#include "diriterator.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CMulberryApp.h"
#include "CStringResources.h"

#include "FullPath.h"

#include "cdstring.h"
#include "cdustring.h"

#if __dest_os == __mac_os_x
#include "MoreFilesX.h"
#endif

#include "MyCFString.h"
#endif

#if __dest_os == __win32_os
#include <WIN_LStream.h>
#endif

#include <errno.h>
#include <strstream>

#include __stat_header
#include <unistd.h>
#if __dest_os == __linux_os

#include <fcntl.h>
#include <jFileUtil.h>
#include <jDirUtil.h>
#else
#endif

// Globals
extern const char* cHexChar;
extern const char cFromHex[];

#if __dest_os == __mac_os || __dest_os == __mac_os_x
static const unsigned long cMaxFnameLen = 31;
static const unsigned long cMaxSafeFnameLen = 255;		// Now handle long file names on OS X
#else
static const unsigned long cMaxFnameLen = 255;
static const unsigned long cMaxSafeFnameLen = 255;
#endif


bool IsRelativePath(const char* path)
{
	// Determine absolute or relative path
	// Mac OS: prefixed by ':'
	// Win32: prefixed by '\' but NOT '\\' which is a UNC path
	// UNIX/Mac OS X: not prefixed by '/'

#if __dest_os == __mac_os
	return (*path == os_dir_delim);
#elif __dest_os == __win32_os
	return (*path == os_dir_delim) && (path[1] != os_dir_delim);
#else
	return (*path != os_dir_delim);
#endif
}

bool ConvertPath(cdstring& path, bool to_relative)
{
	if (path.empty())
		return false;

	if (to_relative)
	{
		// Get CWD
		const cdstring& cwd = CConnectionManager::sConnectionManager.GetCWD();

		// Path must start with CWD
		if (!::strncmp(path, cwd, cwd.length()))
		{
			cdstring temp = &path.c_str()[cwd.length() - 1];
			path = temp;
			return true;
		}
		else
			return false;
	}
	else
	{
		// Path must be relative
		if (IsRelativePath(path))
		{
			// Get CWD
			cdstring cwd = CConnectionManager::sConnectionManager.GetApplicationCWD();
			cwd += &path.c_str()[1];
			path = cwd;
			return true;
		}
		else
			return false;
	}
}

#if __dest_os == __win32_os
int __mkdir(const char* path, int mode)
{
	int temp = mkdir_utf8(path, mode);
	os_errno = ::GetLastError();
	return temp;
}
#endif

int chkdir(const char* path)
{
	// Directory must exist
	bool exists = direxists(path);
	if (!exists && (__mkdir(path, S_IRWXU) != 0))
	{
		int err_no = os_errno;
		CLOG_LOGTHROW(CGeneralException, err_no);
		throw CGeneralException(err_no);

		return 0;
	}
	else
		return exists ? 0 : 1;
}

bool direxists(const char* path)
{
	// Must strip any os_dir_delim from end for this test
	cdstring temp(path);
	if (temp[temp.length() - 1] == os_dir_delim)
		temp[temp.length() - 1] = 0;

	struct stat info;
	return (::stat_utf8(temp, &info) == 0) && S_ISDIR(info.st_mode);
}

bool dirreadable(const char* path)
{
	return ::direxists(path) && (::access_utf8(path, R_OK) == 0);
}

bool dirreadwriteable(const char* path)
{
	return ::direxists(path) && (::access_utf8(path, R_OK | W_OK) == 0);
}

bool fileexists(const char* path)
{
	struct stat info;
	return (::stat_utf8(path, &info) == 0) && S_ISREG(info.st_mode);
}

bool filereadable(const char* path)
{
	return ::fileexists(path) && (::access_utf8(path, R_OK) == 0);;
}

bool filereadwriteable(const char* path)
{
	return ::fileexists(path) && (::access_utf8(path, R_OK | W_OK) == 0);;
}

void addtopath(cdstring& path, const cdstring& name)
{
	char os[2] = {os_dir_delim, 0};
	if (!path.compare_end(os))
		path += os_dir_delim;
	path += name;
}

int count_dir_contents(const char* path)
{
	// Count entire directory and contents

	// Must be a directory
	if (!::direxists(path))
		return -1;

	long ctr = 0;
	diriterator iter(path);
	const char* p = NULL;
	while(iter.next(&p))
	{
#if __dest_os == __mac_os_x || __dest_os == __win32_os || __dest_os == __linux_os
		// Ignore '.' and '..' on Win32/linux
		if ((p[0] == '.') && ((p[1] == 0) || ((p[1] == '.') && (p[2] == 0))))
			continue;
#endif

		ctr++;
	}

	return ctr;
}

int delete_dir(const char* path)
{
	// Delete entire directory and contents

	// Must be a directory
	if (!::direxists(path))
		return -1;

	// Make sure diriterator stack class is destroyed before we attempt
	// to delete the root directory, as diriterator may be holding it open
	{
		long ctr = 0;
		diriterator iter(path);
		const char* p = NULL;
		while(iter.next(&p))
		{
#if __dest_os == __mac_os_x || __dest_os == __win32_os || __dest_os == __linux_os
			// Ignore '.' and '..' on Win32/linux
			if ((p[0] == '.') && ((p[1] == 0) || ((p[1] == '.') && (p[2] == 0))))
				continue;
#endif

			// Get full path
			cdstring fpath = path;
			fpath += p;

			if (iter.is_dir())
			{
				// Delete the sub-directory and contents
				if (::delete_dir(fpath) > 0)
					return 1;
			}
			else
			{
				// Delete the file
				if (::remove_utf8(fpath))
					return 1;
			}
		}
	}

	// Delete the directory itself
	return ::rmdir_utf8(path);
}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#if __dest_os == __mac_os_x

OSErr __path2fsr(const char* path, FSRef* fref);

OSErr __path2fss(const char* path, FSSpec* spec)
{
	FSRef fref;
	OSStatus err = __path2fsr(path, &fref);
	if (err == noErr)
	{
		err = ::FSRefMakeFSSpec(&fref, spec);
	}
	else if (err == fnfErr)
	{
		// Split path
		cdstring parent(path);
		if (::strrchr(parent.c_str(), os_dir_delim) != NULL)
		{
			cdstring child(::strrchr(parent.c_str(), os_dir_delim) + 1);
			*::strrchr(parent.c_str_mod(), os_dir_delim) = 0;
			FSRef parentRef;
			if (__path2fsr(parent, &parentRef) == noErr)
			{
				FSCatalogInfo theInfo;
				err = FSGetCatalogInfo(&parentRef, kFSCatInfoVolume + kFSCatInfoNodeID, &theInfo, NULL, NULL, NULL);
				
				if (err == noErr)
				{
					LStr255 thePName(child);
					err = FSMakeFSSpec(theInfo.volume, theInfo.nodeID, thePName, spec);
				}
			}
		}
	}

	return err;
}

OSErr __path2fsr(const char* path, FSRef* fref)
{
	Boolean isdir;
	OSStatus err = ::FSPathMakeRef((const unsigned char*)path, fref, &isdir);
	return err;
}

int moverename_file(const char * old_name, const char * new_name)
{
	return ::rename_utf8(old_name, new_name);
}

#else

// Special hack to allow move or rename under Mac OS
int moverename_file(const char * old_name, const char * new_name)
{
	FSSpec					old_spec, new_spec;
	OSErr					ioResult;

	if (((ioResult = __path2fss(old_name, &old_spec)) != 0) && (ioResult != notAFileErr)) /* mm 980416 */
		return(__io_error);

	if ((ioResult = __path2fss(new_name, &new_spec)) != 0 && ioResult != fnfErr)
		return(__io_error);

	if (old_spec.vRefNum != new_spec.vRefNum)
		return(__io_error);

	if (!ioResult)
		return(__io_error);

	return moverename_file(old_spec, new_spec);
}


#endif

#ifndef __MSL__
enum __io_results 
{
	__no_io_error,
	__io_error,
	__io_EOF         								/*- mm 961031 -*/
};
#endif

#define io_result(ioResult) ((ioResult == noErr) ? (int) __no_io_error : (int) __io_error)
int moverename_file(const PPx::FSObject& old_spec, const PPx::FSObject& new_spec)
{
	// Be intelligent about the rename or move:
	// If source and destination directories are the same => rename
	// otherwise try move (but cannot change name)

	if (old_spec.GetParentDirID() == new_spec.GetParentDirID())
	{
		PPx::FSObject temp(old_spec);
		OSErr err = noErr;
		try
		{
			temp.Rename(new_spec.GetName());
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
			
			err = 1;
		}

		return(io_result(err));
	}
	
	// See if moving without name change
	else if (old_spec.GetName() == new_spec.GetName())
	{
		FSRef parent;
		new_spec.GetParent(parent);
		OSErr err = ::FSMoveObject(&old_spec.UseRef(), &parent, NULL);

		return(io_result(err));
	}
	else
	{
		// Rename it in the current directory
		PPx::FSObject temp(old_spec);
		OSErr err = noErr;
		try
		{
			temp.Rename(new_spec.GetName());
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
			
			err = 1;
		}

		int result = io_result(err);
		if (result)
			return result;
		
		// Move it to the new directory
		FSRef parent;
		new_spec.GetParent(parent);
		err = ::FSMoveObject(&temp.UseRef(), &parent, NULL);

		return(io_result(err));
	}
}
#elif __dest_os == __win32_os
int moverename_file(const char * old_name, const char * new_name)
{
	int temp = ::rename_utf8(old_name, new_name);
	os_errno = ::GetLastError();
	return temp;
}
#else
int moverename_file(const char * old_name, const char * new_name)
{
	return ::rename_utf8(old_name, new_name);
}
#endif

void report_file_error(const char* fname, int err_no)
{
#ifdef __MULBERRY		// CErrorHandler not linked in Admin Tool
	CErrorHandler::PutOSErrAlertRsrcTxt("Alerts::General::FileError", err_no, fname);
#endif
}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
cdstring GetFullPath(const FSSpec* spec)
{
#if __dest_os == __mac_os
	short fullPathLength;
	Handle fullPath;
	OSErr err = ::FSpGetFullPath(spec, &fullPathLength, &fullPath);
	if (err == noErr)
	{
		cdstring path(*fullPath, fullPathLength);
		::DisposeHandle(fullPath);
		return path;
	}
#else
	cdstring path;
	path.reserve(1024);
	FSRef fref;
	OSErr err = ::FSpMakeFSRef(spec, &fref);
	if (err == noErr)
	{
		err = ::FSRefMakePath(&fref, (unsigned char*) path.c_str_mod(), 1024);
		if (!err)
			return path;
	}
#endif

	return cdstring::null_str;
}
cdstring GetFullPath(const FSRef* ref)
{
	cdstring path;
	path.reserve(1024);
	OSErr err = ::FSRefMakePath(ref, (unsigned char*) path.c_str_mod(), 1024);
	if (err == noErr)
		return path;

	return cdstring::null_str;
}
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
// Disk cache means file info not yet updated => flush the volume buffer
void FlushLocalVolume(const char* path)
{
	FSSpec spec;
	OSErr err = __path2fss(path, &spec);
	if (err != noErr)
	{
		CLOG_LOGTHROW(CGeneralException, err);
		throw CGeneralException(err);
	}

	ParamBlockRec pb;
	pb.fileParam.ioNamePtr = NULL;
	pb.fileParam.ioVRefNum = spec.vRefNum;
	err = PBFlushVolSync(&pb);
	if (err != noErr)
	{
		CLOG_LOGTHROW(CGeneralException, err);
		throw CGeneralException(err);
	}
}
#endif

#if __dest_os == __mac_os
// Need this because some versions of the InterfaceLib do not
// a have PBXGetVolInfoSync method
extern pascal OSErr MyPBXGetVolInfoSync(XVolumeParamPtr paramBlock);
extern "C" {char __msl_system_has_new_file_apis(void);}

#include <time.mac.h>					/*- mm 970514 -*/

#endif

// Convert stat time to a timezone invariant time
time_t GetDefinitiveFileTime(time_t time, const char* fpath)
{
	// What a mess! Different file systems report different times
	// dending on the timezone and daylight savings.
	// We have to have file system specific code to convert the
	// file system specific date-time offset to UTC under all
	// conditions.

	bool stat_time_adjust = true;

#if __dest_os == __mac_os
	if (__msl_system_has_new_file_apis())
	{
		FSRef theRef;
		OSErr err = __msl_path2fsr(fpath, &theRef);

		if (err == noErr)
		{
			FSCatalogInfo theInfo;
			
			err = ::FSGetCatalogInfo(&theRef, kFSCatInfoNodeFlags + kFSCatInfoVolume + kFSCatInfoParentDirID +
												kFSCatInfoNodeID + kFSCatInfoCreateDate + kFSCatInfoContentMod +
												kFSCatInfoAccessDate + kFSCatInfoFinderInfo + kFSCatInfoDataSizes,
												&theInfo, NULL, NULL, NULL);
			
			if (err == noErr)
				return theInfo.contentModDate.lowSeconds + _mac_msl_epoch_offset_ + 3600;
		}

		return 0;
	}
	else
	{
		// HFS stores absolute local dates, i.e. the value retured by stat
		// is the same no matter what timezone or daylight savings is in effect
		
		// HFS+ stores UTC dates, but the File Manager converts this to local
		// dates relative to the current timezone. But (oops) it does not account
		// for daylight savings

		typedef std::pair<cdstring, bool> TFileSystemTimeUTC;
		typedef std::vector<TFileSystemTimeUTC> CFileSystemTimeUTCArray;
		static CFileSystemTimeUTCArray sFileSystemTimeUTC;

		// Check on daylight savings
		// We always have to do this because DST may change while Mulberry is open
		int dst_offset = 0;
		{
			// This depends on OS version:
			//   8.1 does not need DST offset
			//   8.5 and above does!
			long os_response;
			if ((::Gestalt(gestaltSystemVersion, &os_response) == noErr) &&
				((os_response & 0x0000FFFF) >= 0x0850))
			{
				MachineLocation loc;
				ReadLocation(&loc);
				
				dst_offset = (loc.u.dlsDelta ? 1 : 0) * 3600;
			}
		}

		// Compare up to first ':'
		const char* p = ::strchr(fpath, ':');
		int comp_len = 0;
		if (p)
			comp_len = (p - fpath);
		else
			comp_len = ::strlen(fpath);

		// Check for cached file system time UTC info
		bool found = false;
		for(CFileSystemTimeUTCArray::const_iterator iter = sFileSystemTimeUTC.begin();
			iter != sFileSystemTimeUTC.end(); iter++)
		{
			if (!::strncmp((*iter).first, fpath, comp_len))
			{
				stat_time_adjust = (*iter).second;
				found = true;
				
				// Adjust for daylight savings
				if (stat_time_adjust)
					time += dst_offset;
				break;
			}
		}

		// If not found => must get file system info directly
		if (!found)
		{
			// Store the volume name
			cdstring vol_name(fpath, comp_len);

			// Get FSSpec for full path name
			FSSpec fspec;
			c2pstr((char*) fpath);
			OSErr err = ::FSMakeFSSpec(0, 0, (unsigned char*) fpath, &fspec);
			p2cstr((unsigned char*) fpath);

			bool is_hfs_plus = false;
			
			// Is PBXGetVolInfoSync available
			long response;
			if ((::Gestalt(gestaltFSAttr, &response) == noErr) &&
				((response & (1L << gestaltFSSupports2TBVols)) != 0))
			{
				XVolumeParam hpb;
				hpb.ioCompletion = NULL;
				hpb.ioXVersion = 0;
				hpb.ioNamePtr = NULL;
				hpb.ioVRefNum = fspec.vRefNum;
				hpb.ioVolIndex = 0;

				err = ::MyPBXGetVolInfoSync(&hpb);
				
				is_hfs_plus = (hpb.ioVSigWord == kHFSPlusSigWord);
			}
			else
			{
				HParamBlockRec hpb;
				hpb.volumeParam.ioCompletion = NULL;
				hpb.volumeParam.ioNamePtr = NULL;
				hpb.volumeParam.ioVRefNum = fspec.vRefNum;
				hpb.volumeParam.ioVolIndex = 0;

				err = ::PBHGetVInfoSync(&hpb);
				
				is_hfs_plus = (hpb.volumeParam.ioVSigWord == kHFSPlusSigWord);
			}

			if (err == noErr)
			{
				// Check for HFS+
				stat_time_adjust = is_hfs_plus;
				
				// Cache value in array
				sFileSystemTimeUTC.push_back(CFileSystemTimeUTCArray::value_type(vol_name, stat_time_adjust));
			}
				
			// Adjust for daylight savings
			if (stat_time_adjust)
				time += dst_offset;
		}
	}
#else
	// Convert local time from stat into UTC
	stat_time_adjust = true;
#endif

	if (stat_time_adjust)
		// Just convert local time into UTC
		return ::mktime(::gmtime(&time));
	else
		// Just return local time
		return time;
}

unsigned long hash_fun1(const char* c, unsigned long n);
unsigned long hash_fun1(const char* c, unsigned long n)
{
    const char* d = c;
    unsigned long h = 0; 
	unsigned long prime = 1073741827;

    for (unsigned long i = 0; i < n; ++i, ++d)
        h = 613 * h + *d;

    return (h % prime); 
}

void hash_encode_name(std::ostrstream& out, const char* name, unsigned long n);
void hash_encode_name(std::ostrstream& out, const char* name, unsigned long n)
{
	// Get hash
	unsigned long hash = hash_fun1(name, n);
	unsigned short hasher = (hash & 0xFFFF);
	
	// Get maximum length of filename less extension and hash bytes
	const unsigned long cMaxFname = cMaxFnameLen - 8;
	n = std::min(n, cMaxFname);
	
	// Output maximum filename length
	while(n--)
	{
		// Make characters safe for filenames in local file system
		// Best approach - use only alphanumeric plus selected punctuation
		if (isalnum(*name) || (*name == '-') || (*name == '_'))
			out.put(*name++);
		else
		{
			out.put('_');
			name++;
		}
	}
	
	// Now output hexencoded hash

	out.put(cHexChar[hasher >> 12]);
	out.put(cHexChar[(hasher >> 8) & 0x000F]);
	out.put(cHexChar[(hasher >> 4) & 0x000F]);
	out.put(cHexChar[hasher & 0x000F]);
}

void safe_encode_name(std::ostrstream& out, const char* name, unsigned long n, bool single_level);
void safe_encode_name(std::ostrstream& out, const char* name, unsigned long n, bool single_level)
{
	// Output maximum filename length
	while(n--)
	{
		switch(*name)
		{
		case os_dir_delim:
			if (single_level)
			{
				out.put(os_dir_escape);
				out.put(os_dir_escape);
			}
			else
				out.put(*name);
			name++;
			break;

		case os_dir_escape:
			out.put(os_dir_escape);
			out.put(*name++);
			break;

		default:
			out.put(*name++);
		}
	}
}

cdstring LocalFileName(const cdstring& name, char dir_delim, bool hash_encode, bool single_level)
{
	std::ostrstream out;
	const char* p = name;
	const char* q = p;

	// Scan looking for directory delimiter in name
	while(*p)
	{
		if (*p == dir_delim)
		{
			// Output current section - hash encode if required
			if (hash_encode)
				hash_encode_name(out, q, p - q);
			else
				safe_encode_name(out, q, p - q, single_level);

			p++;
			q = p;

			// Output the local dir delim in place of remote one
			if (*p)
				out.put(os_dir_delim);
		}
		else
			p++;
	}

	// Output remainder
	if (p - q)
	{
		if (hash_encode)
			hash_encode_name(out, q, p - q);
		else
			safe_encode_name(out, q, p - q, single_level);
	}

	out << std::ends;

	p = out.str();
	out.freeze(false);
	return p;
}

// Make file name safe for use on local OS
void MakeSafeFileName(cdstring& name)
{
	char* p = name.c_str_mod();
	unsigned long count = 0;
	while(*p)
	{
		// Check string length
		if (count++ >= cMaxSafeFnameLen)
		{
			// Too long - terminate here and exit loop
			*p = 0;
			break;
		}

		// alnum is OK
		if (isalnum(*p))
			p++;
		else
		{
			switch(*p)
			{
			// Invalid characters - force to underscore
			case os_dir_delim:
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			case ':':
#elif __dest_os == __win32_os
			case '/':
			case ':':
			case '*':
			case '?':
			case '\"':
			case '<':
			case '>':
			case '|':
			case '\t':
			case '\r':
			case '\n':
#endif
				*p++ = '_';
				break;
				
			// Pass other characters through
			default:
				p++;
				break;
			}
		}
	}
}

void TempFileSpecAttachments(fspectype& ftemp, const cdstring& name)
{
	TempFileSpec(ftemp, name, true, false);
}

void TempFileSpecSecurity(fspectype& ftemp, const cdstring& name)
{
	TempFileSpec(ftemp, name, false, true);
}

void TempFileSpec(fspectype& ftemp, const cdstring& name, bool attachment, bool security)
{
	// Generate a suitable name
	cdstring new_name = name;

	// Get appropriate temp directory path
	cdstring temp_path = CConnectionManager::sConnectionManager.GetTempDirectory();
	if (attachment)
		temp_path = CConnectionManager::sConnectionManager.GetViewAttachmentDirectory();
	if (security)
		temp_path = CConnectionManager::sConnectionManager.GetSecurityTempDirectory();
	
#if __dest_os == __mac_os || __dest_os == __mac_os_x

	// Get parent FSRef
	MyCFString cfstr_parent(temp_path);
	PPx::FSObject parent(cfstr_parent);

	// Convert to child
	MyCFString cfstr(new_name);
	ftemp = PPx::FSObject(parent.UseRef(), cfstr);
	
	unsigned long ctr = 0;
	while(ftemp.Exists())
	{
		// Add counter to end of file name but preserve any extension
		cdstring name_prefix = name;
		if (::strrchr(name_prefix.c_str(), '.'))
			*::strrchr(name_prefix.c_str_mod(), '.') = 0;
		cdstring name_suffix = ::strrchr(name.c_str(), '.');

		cdstring temp = name_prefix;
		temp += cdstring(++ctr);
		temp += name_suffix;

		cfstr = MyCFString(temp);
		ftemp = PPx::FSObject(parent.UseRef(), cfstr);
	}
#elif __dest_os == __win32_os
	// Locate temp directory and create full path
	cdstring tmp(temp_path);
	::addtopath(tmp, new_name);

	unsigned long ctr = 0;
	while(true)
	{
		DWORD attrs = ::GetFileAttributes(tmp.win_str());
		if (attrs != 0xFFFFFFFF)
		{
			// Add counter to end of file name but preserve any extension
			cdstring name_prefix = tmp;
			if (::strrchr(name_prefix.c_str(), '.'))
				*::strrchr(name_prefix.c_str_mod(), '.') = 0;
			cdstring name_suffix = ::strrchr(tmp.c_str(), '.');

			tmp = name_prefix;
			tmp += cdstring(++ctr);
			tmp += name_suffix;
		}
		else
		{
			DWORD err = ::GetLastError();
			if (err == ERROR_FILE_NOT_FOUND)
			{
				ftemp = tmp;
				break;
			}
			else
			{
				CLOG_LOGTHROW(CGeneralException, err);
				throw CGeneralException(err);
			}
		}
	}

#else
	// On unix use ~/.mulberry/Temporary Files as the default
	if (ftemp.empty())
		ftemp = temp_path;
	::addtopath(ftemp, new_name);

	unsigned long ctr = 0;
	while(true)
	{
		// Test to see whether it already exists
		if (::access_utf8(ftemp, F_OK))
			break;
		else
		{
			// Add counter to end of file name but preserve any extension
			cdstring name_prefix = ftemp;
			if (::strrchr(name_prefix.c_str(), '.'))
				*::strrchr(name_prefix.c_str_mod(), '.') = 0;
			cdstring name_suffix = ::strrchr(ftemp.c_str(), '.');

			ftemp = name_prefix;
			ftemp += cdstring(++ctr);
			ftemp += name_suffix;
		}
	}
#endif
}

// InsertPathIntoStringWithQuotes
//   Insert a path name into a string (unix command) and ensure that the path is properly
//   quoted in the resulting string.
//
// result  <- resulting command with quoted path in it
// cmd     -> the command which assumes a %s substitution
// path    -> unquoted path name to substitute in
//
// Notes:
// This function will work whether the %s in the command already has
// quotes around it or not.
//
// History:
// 17-Sep-2002 cd: created initial implementation
//

void InsertPathIntoStringWithQuotes(cdstring& result, const cdstring& cmd, const cdstring& path)
{
	// Look for %s in cmd
	const char* p = ::strstr(cmd.c_str(), "%s");
	if (!p)
	{
		result = cmd;
		return;
	}
	
	// Check whether %s already surrounded by quotes
	bool quoted = (p[-1] == '\"') && (p[2] == '\"') ||
					(p[-1] == '\'') && (p[2] == '\'');
	
	// Force path to be quoted if quotes not around %s
	cdstring actual_path(path);
	if (!quoted)
		actual_path.quote();
	
	// Form the command using the quoted path
	size_t result_reserve = cmd.length() + actual_path.length();
	result.reserve(result_reserve);
	::snprintf(result.c_str_mod(), result_reserve, cmd.c_str(), actual_path.c_str());
}
