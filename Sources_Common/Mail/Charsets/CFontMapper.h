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


// Header for CFontMapper classes

#ifndef __CFONTAMAPPER__MULBERRY__
#define __CFONTAMAPPER__MULBERRY__

#include "CCharSets.h"
#include "CWindowStates.h"

#include "cdstring.h"

// Classes
class char_stream;

class CFontDescriptor
{
public:
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	STextTraitsRecord	mTraits;
#elif __dest_os == __win32_os
	SLogFont			mTraits;
#else
#error __dest_os
#endif

	CFontDescriptor();
	CFontDescriptor(const CFontDescriptor& copy);
	CFontDescriptor(const char* name, unsigned long size)
		{ Reset(name, size); }
	void Reset(const char* name, unsigned long size);

	int operator==(const CFontDescriptor& comp) const				// Compare with same type
		{ return mTraits == comp.mTraits; }

	// Read/write prefs
	cdstring GetInfo(void) const;
	bool SetInfo(char_stream& info, NumVersion vers_prefs);
};

typedef map<cdstring, CFontDescriptor> CFontMapping;

class CFontMapper : public CFontMapping
{
public:
	CFontMapper() {}
	CFontMapper(const CFontMapper& copy) : CFontMapping(copy) {}
	virtual ~CFontMapper() {}
	
	CFontMapper& operator=(const CFontMapper& copy)					// Assignment with same type
		{ return static_cast<CFontMapper&>(CFontMapping::operator=(copy)); }

	//int operator==(const CFontMapper& comp) const						// Compare with same type
	//	{ return CFontMapping::operator==(comp); }

	// Add items
	void Add(ECharset charset, const char* name, unsigned long size);

	// Lookup
	const CFontDescriptor& GetCharsetFontDescriptor(ECharset charset) const;
	CFontDescriptor& GetCharsetFontDescriptor(ECharset charset);

	// Read/write prefs
	cdstring GetInfo(void) const;
	bool SetInfo(char_stream& info, NumVersion vers_prefs);	
};

#endif
