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

#ifndef MyCFString_H
#define MyCFString_H

#include <SysCFString.h>

#include "cdstring.h"

class MyCFString: public PPx::CFString
{
public:
	MyCFString() {}
	MyCFString(
			CFStringRef	inStringRef,
			bool		inRetain = true) :
		PPx::CFString(inStringRef, inRetain) {}
	MyCFString(
			CFMutableStringRef	inStringRef,
			bool				inRetain) :
		PPx::CFString(inStringRef, inRetain) {}
	MyCFString( const PPx::CFString& inSource ) :
		PPx::CFString( inSource ) {}

	explicit			MyCFString(
								const char*			inCString,
								CFStringEncoding	inEncoding = PPx::encoding_System,
								CFAllocatorRef		inAllocator = nil);

	virtual ~MyCFString() {}

	MyCFString&			operator = ( const PPx::CFString& inSource );

	void				GetString(
								cdstring&		outString,
								CFStringEncoding	inEncoding = PPx::encoding_System) const;
	cdstring			GetString(CFStringEncoding	inEncoding = kCFStringEncodingUTF8) const;

};

#endif	// MyCFString_H
