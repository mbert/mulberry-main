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
	CWebDAVSession.h

	Author:			
	Description:	<describe the CWebDAVSession class here>
*/

#ifndef CWebDAVSession_H
#define CWebDAVSession_H

#include "CHTTPSession.h"

using namespace http; 

namespace http {

namespace webdav {

class CWebDAVSession: public CHTTPSession
{
public:
	enum EDAVVersion
	{
		eDAVUnknown = 0,
		
		// Base DAV
		eDAV1 =						1L << 0,
		eDAV2 =						1L << 1,
		eDAV2bis =					1L << 2,
		
		// Extensions
		eDAVACL =					1L << 16,
		eCALDAVaccess =				1L << 17,
		eCALDAVsched =				1L << 18,
			
		// Private Extensions
		eCyrusoftInheritable =		1L << 19
		
	};

	CWebDAVSession();
	CWebDAVSession(const cdstring& host);
	virtual ~CWebDAVSession();

	bool Initialised() const
	{
		return mInitialised;
	}
	virtual bool Initialise(const cdstring& host, const cdstring& base_uri);

	bool HasDAVVersion(EDAVVersion version) const
	{
		return (mVersion & version) != 0;
	}

	bool HasDAV() const
	{
		return mVersion != eDAVUnknown;
	}

	bool HasDAVLocking() const
	{
		return HasDAVVersion(eDAV2) || HasDAVVersion(eDAV2bis);
	}

	bool HasDAVACL() const
	{
		return HasDAVVersion(eDAVACL);
	}

protected:
	bool			mInitialised;
	unsigned long	mVersion;

	virtual CHTTPAuthorization* GetAuthorization(bool first_time, const cdstrvect& www_authenticate) = 0;
	virtual void SetServerType(unsigned long type) = 0;
	virtual void SetServerDescriptor(const cdstring& txt) = 0;
	virtual void SetServerCapability(const cdstring& txt) = 0;
};

}	// namespace webdav

}	// namespace http

#endif	// CWebDAVSession_H
