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


// CClipboard

#ifndef __CCLIPBOARD__MULBERRY__
#define __CCLIPBOARD__MULBERRY__

#include <X11/X.h>

class JXDisplay;
class cdstring;

class CClipboard
{
public:
	static void CopyToPrimaryClipboard(JXDisplay* display, const char* txt);
	static void CopyToSecondaryClipboard(JXDisplay* display, const char* txt);

	static void GetPrimaryClipboard(JXDisplay* display, cdstring& txt);
	static void GetSecondaryClipboard(JXDisplay* display, cdstring& txt);

	static bool HasPrimaryText(JXDisplay* display);
	static bool HasSecondaryText(JXDisplay* display);

private:
	CClipboard() {}
	~CClipboard() {}

	static void CopyToClipboard(JXDisplay* display, const char* txt, Atom clip);
	static void GetClipboard(JXDisplay* display, cdstring& txt, Atom clip);

	static bool TestText(JXDisplay* display, Atom clip, Time time, Atom& textType);
};

#endif
