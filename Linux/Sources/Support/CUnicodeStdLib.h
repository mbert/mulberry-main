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

// Mac OS X can handle utf8 file names

#define fopen_utf8		fopen
#define remove_utf8		remove
#define rename_utf8		rename

#define mkdir_utf8		mkdir
#define stat_utf8		stat
#define access_utf8		access
#define rmdir_utf8		rmdir

#endif
