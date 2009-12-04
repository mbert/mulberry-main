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
	CHTTPRequestResponse.h

	Author:			
	Description:	<describe the CHTTPRequestResponse class here>
*/

#ifndef CHTTPRequestResponse_H
#define CHTTPRequestResponse_H

#include <stdint.h>
#include <exception>
#include <istream>
#include <map>
#include <ostream>
#include "cdstring.h"

#include "CHTTPData.h"
#include "CHTTPDefinitions.h"


namespace http {

class CHTTPSession;

class CHTTPRequestResponse
{
public:
	enum ERequestMethod
	{
		// HTTP Requests
		eRequest_OPTIONS,
		eRequest_GET,
		eRequest_HEAD,
		eRequest_POST,
		eRequest_PUT,
		eRequest_DELETE,
		eRequest_TRACE,
		eRequest_CONNECT,
		
		// WebDAV requests
		eRequest_MKCOL,
		eRequest_MOVE,
		eRequest_COPY,
		eRequest_PROPFIND,
		eRequest_PROPPATCH,
		eRequest_LOCK,
		eRequest_UNLOCK,
		
		// WebDAV Version extension
		eRequest_REPORT,
		
		// WebDAV ACL extension
		eRequest_ACL,
		
		// CalDAV requests
		eRequest_MKCALENDAR,
		
		// CardDAV requests
		eRequest_MKADBK
	};

	class CHTTPResponseException : public std::exception
	{
	public:
		CHTTPResponseException(const char* error)
		{
			mError = error;
		}
		~CHTTPResponseException() throw() {}

	    virtual const char* what () const throw()
	    {
	    	return mError.c_str();
	    }

	private:
		cdstring mError;
	};

	CHTTPRequestResponse(const CHTTPSession* session, ERequestMethod method, const cdstring& ruri);
	CHTTPRequestResponse(const CHTTPSession* session, ERequestMethod method, const cdstring& ruri, const cdstring& etag, bool etag_match);
	virtual ~CHTTPRequestResponse();

	void SetSession(const CHTTPSession* session)
	{
		mSession = session;
	}
	const CHTTPSession* GetSession() const
	{
		return mSession;
	}

	const char* GetMethod() const;

	void SetRURI(const cdstring& ruri)
	{
		mURI = ruri;
	}
	const cdstring& GetRURI() const
	{
		return mURI;
	}

	void SetETag(const cdstring& etag, bool etag_match)
	{
		mETag = etag;
		mETagMatch = etag_match;
	}

	void QueuedForSending(const CHTTPSession* session)
	{
		mSession = session;
	}
	
	void SetData(CHTTPInputData* request_data, CHTTPOutputData* response_data)
	{
		mRequestData = request_data;
		mResponseData = response_data;
	}

	virtual bool	 HasRequestData() const;
	virtual std::istream* GetRequestDataStream();
	virtual std::ostream* GetResponseDataStream();
	
	void StartRequestData()
	{
		if (mRequestData != NULL)
			mRequestData->Start();
	}
	void StopRequestData()
	{
		if (mRequestData != NULL)
			mRequestData->Stop();
	}

	void StartResponseData()
	{
		if (mResponseData != NULL)
			mResponseData->Start();
	}
	void StopResponseData()
	{
		if (mResponseData != NULL)
			mResponseData->Stop();
	}

	cdstring GetRequestHeader();
	void GenerateRequestHeader(std::ostream& os);
	void ParseResponseHeader(std::istream& is, std::ostream* log);
	void ClearResponse();

	uint32_t GetStatusCode() const
	{
		return mStatusCode;
	}
	const cdstring& GetStatusReason() const
	{
		return mStatusReason;
	}

	bool GetConnectionClose() const
	{
		return mConnectionClose;
	}

	virtual uint32_t GetContentLength() const
	{
		return mContentLength;
	}
	virtual bool GetChunked() const
	{
		return mChunked;
	}

	void SetComplete()
	{
		mCompleted = true;
	}
	bool GetCompleted() const
	{
		return mCompleted;
	}

	const cdstrmultimapcasei& GetResponseHeaders() const
	{
		return mHeaders;
	}

	const cdstring& GetResponseHeader(const cdstring& hdr) const;
	
	cdstrvect& GetResponseHeaders(const cdstring& hdr, cdstrvect& hdrs) const;
	
	void DoRequestSync();

	bool IsRedirect() const;

protected:
	const CHTTPSession*	mSession;
	CHTTPInputData*		mRequestData;
	CHTTPOutputData*	mResponseData;
	
	// Request items
	ERequestMethod			mMethod;
	cdstring				mURI;
	cdstring				mETag;
	bool					mETagMatch;

	// Response items
	uint32_t				mStatusCode;
	cdstring				mStatusReason;
	cdstrmultimapcasei		mHeaders;
	bool					mConnectionClose;
	uint32_t				mContentLength;
	bool					mChunked;
	
	bool 					mCompleted;
	
	virtual void WriteHeaderToStream(std::ostream& os);
	virtual void WriteContentHeaderToStream(std::ostream& os);

private:
	void InitResponse();

	void ParseStatusLine(cdstring& line);
	bool ReadFoldedLine(std::istream& is, cdstring& line1, cdstring& line2, std::ostream* log) const;

	void CacheHeaders();
};

}	// namespace http

#endif	// CHTTPRequestResponse_H
