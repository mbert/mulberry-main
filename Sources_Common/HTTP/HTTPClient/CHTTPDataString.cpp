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

/* 
	CHTTPInputString.cpp

	Author:			
	Description:	<describe the CHTTPInputDataString class here>
*/

#include "CHTTPDataString.h"

#include <strstream>

using namespace http; 

CHTTPInputDataString::CHTTPInputDataString(const cdstring& str, const cdstring& content_type)
{
	// Create an input string stream
	std::istrstream* temp = new std::istrstream(str.c_str());
	//std::ios* temp2 = temp;
	mStream = temp;
	mOwnsStream = true;
	mContentLength = str.length();
	mContentType = content_type;
}

CHTTPInputDataString::CHTTPInputDataString(char* str, const cdstring& content_type)
{
	// Cache the string which we must delete
	mCachedData.steal(str);

	// Create an input string stream
	std::istrstream* temp = new std::istrstream(mCachedData.c_str());
	//std::ios* temp2 = temp;
	mStream = temp;
	mOwnsStream = true;
	mContentLength = mCachedData.length();
	mContentType = content_type;
}


CHTTPInputDataString::~CHTTPInputDataString()
{
}

void CHTTPInputDataString::Start()
{
	// Always rewind to start in case it needs to be read multiple times
	mStream->clear();
	dynamic_cast<std::istrstream*>(mStream)->seekg(0);
}

CHTTPOutputDataString::CHTTPOutputDataString()
{
	// Create stream that uses the string
	mStream = new std::ostrstream;
	mOwnsStream = true;
}


CHTTPOutputDataString::~CHTTPOutputDataString()
{
}

cdstring CHTTPOutputDataString::GetData() const
{
	*dynamic_cast<std::ostrstream*>(mStream) << std::ends;
	cdstring result;
	result.steal(dynamic_cast<std::ostrstream*>(mStream)->str());
	return result;
}

void CHTTPOutputDataString::Clear()
{
	// Throw out existing data and start from scratch
	delete mStream;
	mStream = new std::ostrstream;
}
