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
	CHTTPSession.h

	Author:			
	Description:	<describe the CHTTPSession class here>
*/

#ifndef CHTTPSession_H
#define CHTTPSession_H

#include "cdstring.h"

namespace http {

class CHTTPAuthorization;
class CHTTPRequestResponse;

class CHTTPSession
{
public:
	CHTTPSession();
	CHTTPSession(const cdstring& host);
	virtual ~CHTTPSession();

	bool HasAuthorization() const
	{
		return mAuthorization != NULL;
	}
	const CHTTPAuthorization* GetAuthorization() const
	{
		return mAuthorization;
	}

	virtual void WriteHeaderToStream(std::ostream& os, const CHTTPRequestResponse* request) const;

	// Connection items
	void SetHost(const cdstring& host);
	const cdstring& GetHost() const
	{
		return mHost;
	}

	virtual void SendRequest(CHTTPRequestResponse* request);
	virtual void HandleHTTPError(CHTTPRequestResponse* request) = 0;
	virtual void DisplayHTTPError(CHTTPRequestResponse* request) = 0;

protected:
	enum EConnectionState
	{
		eClosed = 0,
		eOpen
	};

	CHTTPAuthorization* 	mAuthorization;			// Current authorization details
	EConnectionState		mConnectionState;
	
	// Connection items
	cdstring				mHost;

	virtual bool IsConnectionOpen() const
	{
		return mConnectionState == eOpen;
	}
	virtual void NeedConnection();
	virtual void OpenConnection();
	virtual void CloseConnection();
	virtual void OpenSession() = 0;
	virtual void CloseSession() = 0;
	virtual void DoRequest(CHTTPRequestResponse* request) = 0;
};

}	// namespace http

#endif	// CHTTPSession_H
