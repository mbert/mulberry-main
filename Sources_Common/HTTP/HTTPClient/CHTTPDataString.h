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
	CHTTPDataString.h

	Author:			
	Description:	<describe the CHTTPDataString class here>
*/

#ifndef CHTTPDataString_H
#define CHTTPDataString_H

#include "CHTTPData.h"

#include "cdstring.h"

namespace http {

class CHTTPInputDataString: public CHTTPInputData
{
public:
	CHTTPInputDataString(const cdstring& str, const cdstring& content_type);
	CHTTPInputDataString(char* str, const cdstring& content_type);
	virtual ~CHTTPInputDataString();

	virtual void Start();

private:
	cdstring mCachedData;
};

class CHTTPOutputDataString: public CHTTPOutputData
{
public:
	CHTTPOutputDataString();
	virtual ~CHTTPOutputDataString();

	cdstring GetData() const;

	virtual void Clear();
};

}	// namespace http

#endif	// CHTTPDataString_H
