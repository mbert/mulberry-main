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
	CHTTPData.h

	Author:			
	Description:	<describe the CHTTPData class here>
*/

#ifndef CHTTPData_H
#define CHTTPData_H

#include <stdint.h>
#include <iostream>
#include "cdstring.h"

namespace http
{

class CHTTPData
{
public:
	CHTTPData()
	{
		mStream = NULL;
		mOwnsStream = false;
	}
	virtual ~CHTTPData()
	{
		Clean();
	}

	virtual void Start() {}
	virtual void Stop() {}
	
protected:
	std::ios*		mStream;
	bool		mOwnsStream;

	void Clean()
	{
		if (mOwnsStream && (mStream != NULL))
		{
			delete mStream;
			mStream = NULL;
		}
	}
};

class CHTTPInputData : public CHTTPData
{
public:
	CHTTPInputData() {}
	virtual ~CHTTPInputData() {}

	std::istream* GetStream() const
	{
		return dynamic_cast<std::istream*>(mStream);
	}

	uint32_t GetContentLength()
	{
		return mContentLength;
	}
	const cdstring& GetContentType()
	{
		return mContentType;
	}

protected:
	uint32_t	mContentLength;
	cdstring	mContentType;
};

class CHTTPOutputData : public CHTTPData
{
public:
	CHTTPOutputData() {}
	virtual ~CHTTPOutputData() {}

	std::ostream* GetStream() const
	{
		return dynamic_cast<std::ostream*>(mStream);
	}
	
	virtual void Clear() = 0;
};

}	// namespace http

#endif	// CHTTPData_H
