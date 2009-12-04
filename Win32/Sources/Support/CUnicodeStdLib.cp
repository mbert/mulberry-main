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


// CUnicodeUtils.cpp : implementation file
//

#include "CUnicodeStdLib.h"

#include "cdustring.h"

#include __stat_header
#include <direct.h>
#include <errno.h>
#include <unistd.h>
//#include <extras.h>
//#include <time_api.h>

#ifdef __VCPP__
#define _MSL_CDECL
typedef unsigned short mode_t;
#endif

FILE* fopen_utf8(const char* name, const char* mode)
{
	cdustring name_utf16(name);
	cdustring mode_utf16(mode);
	
	return _wfopen(name_utf16.c_str(), mode_utf16.c_str());
}

int remove_utf8(const char* name)
{
	cdustring name_utf16(name);

	return _wremove(name_utf16.c_str());
}

int rename_utf8(const char* old_name, const char* new_name)
{
	cdustring old_name_utf16(old_name);
	cdustring new_name_utf16(new_name);

	return _wrename(old_name_utf16.c_str(), new_name_utf16.c_str());
}

/*
 *	int mkdir(const char *path)
 *
 */
int mkdir_utf8(const char *path)
{
	cdustring utf16(path);
	return _wmkdir(utf16.c_str());
}

/*
 *	int stat(char *path, struct stat *buf)
 *
 *		Returns information about a file.
 */

int _MSL_CDECL stat_utf8(const char *path_utf8, struct stat *buf) _MSL_CANT_THROW
{
	cdustring utf16(path_utf8);
	return _wstat(utf16.c_str(), (struct _stat32*)buf);
} 

/* check accessibility of a file */
int _MSL_CDECL access_utf8(const char *path, int mode) _MSL_CANT_THROW
{
	DWORD Attributes;
	
	if (path == NULL || path == "")
	{
		errno = ENOENT;
		return -1;
	}
	if (strlen(path) > MAX_PATH)
	{
		errno = ENAMETOOLONG;
		return -1;
	}

	cdustring utf16(path);

	Attributes = GetFileAttributesW(utf16.c_str());
	if (Attributes == -1)
	{
		errno = ENOENT;
		return -1;
	}
	
	/*	Test all the flags */
	if (mode & W_OK)
	{	/* can we write? */
		if (Attributes & FILE_ATTRIBUTE_READONLY)
		{
			errno = EACCES;
			return -1;
		}
	}
	if (mode & R_OK)
	{	/* can we read? */
		/* cannot fail ... ? */
	}
	if (mode & X_OK)
	{	/* can we execute? */
		DWORD type;
		if (!GetBinaryTypeW(utf16.c_str(), &type))
		{
			errno = EACCES;
			return -1;
		}
	}

	/* All tests passed, or mode is F_OK (and we already know it exists) */
	return 0;
}

/*
 *	int rmdir_utf8(const char *path)
 *
 *		Removes a directory (must be empty).
 */
int _MSL_CDECL rmdir_utf8(const char *path) _MSL_CANT_THROW
{
	cdustring utf16(path);
	return _wrmdir(utf16.c_str());
}


