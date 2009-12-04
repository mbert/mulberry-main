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


// Header for CFullFileStream class

#ifndef __CFULLFILESTREAM__MULBERRY__
#define __CFULLFILESTREAM__MULBERRY__

#if __dest_os == __win32_os
#include <WIN_LFileStream.h>
#elif __dest_os == __linux_os
#include "UNX_LFileStream.h"
#endif

// Classes

class CFullFileStream : public LFileStream {

public:
					CFullFileStream();
#if __dest_os == __mac_os || __dest_os == __mac_os_x
					CFullFileStream(const PPx::FSObject &inFileSpec);
#elif __dest_os == __win32_os
					CFullFileStream(LPCTSTR inFileName, UINT flags);
#elif __dest_os == __linux_os
					CFullFileStream(const char* inFileName, UInt32 flags);
#endif
	virtual			~CFullFileStream();

	virtual SInt16			OpenResourceFork(short inPrivileges);
	virtual void			CloseResourceFork();

	virtual ExceptionCode	GetResourceBytes(void *outBuffer, long& inNumBytes);
	virtual ExceptionCode	PutResourceBytes(const void *inBuffer, long& inNumBytes);

	virtual void SetRsrcMarker(SInt32 inOffset, EStreamFrom inFromWhere);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	virtual SInt32	GetLength() const;
#elif __dest_os == __win32_os || __dest_os == __linux_os
	virtual UInt32	GetLength() const;
#else
#error __dest_os
#endif
	virtual SInt32	GetMarker() const;

};

#endif
