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
	CHTTPSession.cpp

	Author:			
	Description:	<describe the CHTTPSession class here>
*/

#include "CHTTPSession.h"

#include "CHTTPAuthorization.h"
#include "CHTTPRequestResponse.h"
#include "CLog.h"

#include <sstream>

using namespace http; 

CHTTPSession::CHTTPSession()
{
	mAuthorization = NULL;
	mConnectionState = eClosed;
}

CHTTPSession::CHTTPSession(const cdstring& host)
{
	mAuthorization = NULL;
	mHost = host;
	mConnectionState = eClosed;
}

CHTTPSession::~CHTTPSession()
{
	delete mAuthorization;
	mAuthorization = NULL;
}

// Do session global headers
void CHTTPSession::WriteHeaderToStream(std::ostream& os, const CHTTPRequestResponse* request) const
{
	if (HasAuthorization())
		GetAuthorization()->GenerateAuthorization(os, request);
}

void CHTTPSession::SetHost(const cdstring& host)
{
	if (mHost != host)
	{
		mHost = host;
		
		// Always clear out authorization when host changes
		delete mAuthorization;
		mAuthorization = NULL;
	}
}

void CHTTPSession::NeedConnection()
{
	if (!IsConnectionOpen())
		OpenConnection();
}

void CHTTPSession::OpenConnection()
{
	if (mConnectionState != eOpen)
	{
		OpenSession();
		mConnectionState = eOpen;
	}
}

void CHTTPSession::CloseConnection()
{
	if (mConnectionState != eClosed)
	{
		CloseSession();
		mConnectionState = eClosed;
	}
}

void CHTTPSession::SendRequest(CHTTPRequestResponse* request)
{
	try
	{
		// First need a connection
		NeedConnection();
		
		// Now do the connection
		DoRequest(request);
		
		// Check the final connection state and close if that's what the server wants
		if (request->GetConnectionClose())
			CloseConnection();
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
		mConnectionState = eClosed;
		
		CLOG_LOGRETHROW;
		throw;
	}
}
