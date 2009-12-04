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
	CWebDAVLock.h

	Author:			
	Description:	<describe the CWebDAVLock class here>
*/

#ifndef CWebDAVLock_H
#define CWebDAVLock_H

#include "CWebDAVRequestResponse.h"

using namespace http; 

namespace http {

namespace webdav {

class CWebDAVLock: public CWebDAVRequestResponse
{
public:
	enum EWebDAVLockScope
	{
		eExclusive = 0,
		eShared
	};

	enum EResourceExists
	{
		eResourceMustExist,
		eResourceMustNotExist,
		eResourceMayExist
	};

	CWebDAVLock(CWebDAVSession* session, const cdstring& ruri, EWebDAVDepth depth,
					EWebDAVLockScope scope, const cdstring& owner, unsigned long timeout, EResourceExists exists = eResourceMustExist);
	virtual ~CWebDAVLock();

	cdstring GetLockToken() const;

protected:
	EWebDAVDepth		mDepth;
	EWebDAVLockScope	mScope;
	cdstring			mOwner;
	unsigned long		mTimeout;

	virtual void WriteHeaderToStream(std::ostream& os);

	void	InitRequestData();

	void	GenerateXML(std::ostream& os);
};

}	// namespace webdav

}	// namespace http

#endif	// CWebDAVLock_H
