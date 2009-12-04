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

#include "MyCFString.h"

#include <PPxPrefix.h>

using namespace PPx;

MyCFString::MyCFString(
	const char*			inCString,
	CFStringEncoding	inEncoding,
	CFAllocatorRef		inAllocator)
{
	CFStringRef	ref = ::CFStringCreateWithCString(inAllocator,
							inCString, inEncoding);
	
	// Use empty string if error occurs
	if (ref == NULL)
		ref = ::CFStringCreateWithCString(inAllocator, "", inEncoding);

	PPx_ThrowIfCFCreateFailed_(ref, "CFStringCreateWithCString");
	
	AttachRef(ref, PPx::retain_No);
}

MyCFString&
MyCFString::operator = (
	const CFString&	inSource)
{
	AssignObject(inSource);
	return *this;
}

void
MyCFString::GetString(
	cdstring&		outString,
	CFStringEncoding	inEncoding) const
{
	CFIndex	byteLength = GetLength() > 0 ? GetByteLength(PPx::cfRange_All, inEncoding) : 0;
	
	if (byteLength > 0) {
		outString.reserve(byteLength);
		UInt8*	buffer = reinterpret_cast<UInt8 *>(outString.c_str_mod());
		CFRange	allRange = { 0, GetLength() };
		
		::CFStringGetBytes(UseRef(), allRange, inEncoding, 0, false,
							buffer, byteLength, &byteLength);
	
	} else {
		outString.clear();
	}
}


cdstring
MyCFString::GetString(
	CFStringEncoding	inEncoding) const
{
	cdstring outString;
	CFIndex	byteLength = GetLength() > 0 ? GetByteLength(PPx::cfRange_All, inEncoding) : 0;
	
	if (byteLength > 0) {
		outString.reserve(byteLength);
		UInt8*	buffer = reinterpret_cast<UInt8 *>(outString.c_str_mod());
		CFRange	allRange = { 0, GetLength() };
		
		::CFStringGetBytes(UseRef(), allRange, inEncoding, 0, false,
							buffer, byteLength, &byteLength);
	
	} else {
		outString.clear();
	}
	
	return outString;
}
