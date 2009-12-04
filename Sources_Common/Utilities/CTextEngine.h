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


// CTextEngine.h	- class to do various text processing actions

#ifndef __CTEXTENGINE__MULBERRY__
#define __CTEXTENGINE__MULBERRY__

#include "cdstring.h"

class CTextEngine
{
public:
	
	static const char* WrapLines(const char* text,
									unsigned long length,
									unsigned long wrap_len,
									bool flowed);
	static const char* UnwrapLines(const char* text,
									unsigned long length);
	static const char* QuoteLines(const char* text,
									unsigned long length,
									unsigned long wrap_len,
									const cdstring& prefix,
									const cdstrvect* recognise = NULL,
									bool original_flowed = false);
	static const char* UnquoteLines(const char* text,
									unsigned long length,
									const char* prefix);

	static void RemoveSigDashes(char* text);

	static const char* StripQuotedLines(const char* text, const cdstrvect& quotes);

	static void SetSpacesPerTab(long spaces)
		{ sSpacesPerTab = spaces; }

private:
	static long	sSpacesPerTab;
	static cdstring sCurrentPrefix;

	static long GetPrefixDepth(const char*& text, long& remaining, const cdstrvect& matches);
	static void AddPrefix(std::ostream& out, const cdstring& prefix, long prefix_length, long num_add, long depth, long& prefixed);

	// Always static, no object
	CTextEngine() {}
	~CTextEngine() {}
};

#endif
