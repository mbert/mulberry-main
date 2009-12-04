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


// CUnicodeStdLib.h : header file
//

#ifndef __CUnicodeStdLib__MULBERRY__
#define __CUnicodeStdLib__MULBERRY__

#ifdef __VCPP__
#define _MSL_CANT_THROW
#endif

#include "stdio.h"

FILE* 	fopen_utf8(const char* name, const char* mode) _MSL_CANT_THROW;
int 	remove_utf8(const char* name) _MSL_CANT_THROW;
int 	rename_utf8(const char* old_name, const char* new_name) _MSL_CANT_THROW;

int		mkdir_utf8(const char* path) _MSL_CANT_THROW;
int		stat_utf8(const char* path, struct stat* buf) _MSL_CANT_THROW;
int		access_utf8(const char* path, int mode) _MSL_CANT_THROW;
int		rmdir_utf8(const char *path) _MSL_CANT_THROW;

#endif
