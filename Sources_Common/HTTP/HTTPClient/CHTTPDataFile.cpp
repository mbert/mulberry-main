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
	CHTTPInputFile.cpp

	Author:			
	Description:	<describe the CHTTPDataFile class here>
*/

#include "CHTTPDataFile.h"

#include "cdfstream.h"

#include <sys/stat.h>

using namespace http; 

CHTTPInputDataFile::CHTTPInputDataFile(const cdstring& str, const cdstring& content_type)
{
	// Cache file name
	mFname = str;

	// Determine size of stream
	struct stat finfo;
	if (::stat_utf8(mFname.c_str(), &finfo) == 0)
	{
		mContentLength = finfo.st_size;
	}
	mContentType = content_type;
}

CHTTPInputDataFile::~CHTTPInputDataFile()
{
}

void CHTTPInputDataFile::Start()
{
	// Create an input file stream
	mStream = new cdifstream(mFname.c_str());
	mOwnsStream = true;
}

void CHTTPInputDataFile::Stop()
{
	// Delete the fstream now as we are done with it and do not need to keep it open
	Clean();
}

CHTTPOutputDataFile::CHTTPOutputDataFile(const cdstring& str)
{
	// Cache file name
	mFname = str;
}


CHTTPOutputDataFile::~CHTTPOutputDataFile()
{
}

void CHTTPOutputDataFile::Start()
{
	// Create an output file stream
	mStream = new cdofstream(mFname.c_str());
	mOwnsStream = true;
}

void CHTTPOutputDataFile::Stop()
{
	// Delete the fstream now as we are done with it and do not need to keep it open
	Clean();
}

void CHTTPOutputDataFile::Clear()
{
	// Throw out existing data and start from scratch
	delete mStream;
	mStream = new cdofstream(mFname.c_str());
}
