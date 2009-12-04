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
	CHTTPDataFile.h

	Author:			
	Description:	<describe the CHTTPDataFile class here>
*/

#ifndef CHTTPDataFile_H
#define CHTTPDataFile_H

#include "CHTTPData.h"

#include "cdstring.h"

namespace http {

class CHTTPInputDataFile: public CHTTPInputData
{
public:
	CHTTPInputDataFile(const cdstring& fname, const cdstring& content_type);
	virtual ~CHTTPInputDataFile();

	virtual void Start();
	virtual void Stop();

protected:
	cdstring	mFname;
};

class CHTTPOutputDataFile: public CHTTPOutputData
{
public:
	CHTTPOutputDataFile(const cdstring& fname);
	virtual ~CHTTPOutputDataFile();

	virtual void Start();
	virtual void Stop();

	virtual void Clear();

protected:
	cdstring	mFname;
};

}	// namespace http

#endif	// CHTTPDataFile_H
