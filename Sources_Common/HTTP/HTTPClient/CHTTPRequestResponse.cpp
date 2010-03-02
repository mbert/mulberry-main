/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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
	CHTTPRequestResponse.cpp

	Author:			
	Description:	<describe the CHTTPRequestResponse class here>
*/

#include "CHTTPRequestResponse.h"

#include "CHTTPAuthorization.h"
#include "CHTTPDefinitions.h"
#include "CHTTPSession.h"

#include <cstdlib>
#include <strstream>

using namespace http; 
using namespace webdav; 
using namespace caldav; 
using namespace carddav; 

const char* cMethodMap[] = 
{
	cRequestOPTIONS, cRequestGET, cRequestHEAD, cRequestPOST, cRequestPUT, cRequestDELETE, cRequestTRACE, cRequestCONNECT,
	cRequestMKCOL, cRequestMOVE, cRequestCOPY, cRequestPROPFIND, cRequestPROPPATCH, cRequestLOCK, cRequestUNLOCK,
	cRequestREPORT, cRequestACL, cRequestMKCALENDAR, cRequestMKADBK
};

CHTTPRequestResponse::CHTTPRequestResponse(const CHTTPSession* session, ERequestMethod method, const cdstring& ruri) :
	mHeaders(case_insensitive_cdstring())
{
	InitResponse();
	mSession = session;
	mMethod = method;
	mURI = ruri;
}
CHTTPRequestResponse::CHTTPRequestResponse(const CHTTPSession* session, ERequestMethod method, const cdstring& ruri, const cdstring& etag, bool etag_match) :
	mHeaders(case_insensitive_cdstring())
{
	InitResponse();
	mSession = session;
	mMethod = method;
	mURI = ruri;
	mETag = etag;
	mETagMatch = etag_match;
}

CHTTPRequestResponse::~CHTTPRequestResponse()
{
}

void CHTTPRequestResponse::InitResponse()
{
	mSession = NULL;
	mRequestData = NULL;
	mResponseData = NULL;
	mMethod = eRequest_GET;
	mETagMatch = false;
	mStatusCode = eStatus_Unknown;
	mConnectionClose = false;
	mContentLength = 0;
	mChunked = false;
	mCompleted = false;
}

const char* CHTTPRequestResponse::GetMethod() const
{
	return cMethodMap[mMethod];
}

bool CHTTPRequestResponse::HasRequestData() const
{
	return (mRequestData != NULL);
}

std::istream* CHTTPRequestResponse::GetRequestDataStream()
{
	return mRequestData != NULL ? mRequestData->GetStream() : NULL;
}

std::ostream* CHTTPRequestResponse::GetResponseDataStream()
{
	return mResponseData != NULL ? mResponseData->GetStream() : NULL;
}

cdstring CHTTPRequestResponse::GetRequestHeader()
{
	std::ostrstream sout;
	GenerateRequestHeader(sout);
	sout << std::ends;
	
	cdstring result;
	result.steal(sout.str());
	return result;
}

void CHTTPRequestResponse::GenerateRequestHeader(std::ostream& os)
{
	// This will be overridden by sub-classes that add headers - those classes should
	// call this class's implementation to write out the basic set of headers
	WriteHeaderToStream(os);

	// Empty line
	os << net_endl;
}

void CHTTPRequestResponse::WriteHeaderToStream(std::ostream& os)
{
	// Write Request-line
	os << cMethodMap[mMethod] << " " << mURI << " HTTP/1.1" << net_endl;
	
	// Write host
	os << cHeaderHost << cHeaderDelimiter << mSession->GetHost() << net_endl;
	
	// Do ETag matching
	if (!mETag.empty())
	{
		os << (mETagMatch ? cHeaderIfMatch : cHeaderIfNoneMatch) << cHeaderDelimiter << mETag << net_endl;
	}

	// Do session global headers
	mSession->WriteHeaderToStream(os, this);
	
	// Check for content
	WriteContentHeaderToStream(os);
}

void CHTTPRequestResponse::WriteContentHeaderToStream(std::ostream& os)
{
	// Check for content
	if (HasRequestData())
	{
		os << cHeaderContentLength << cHeaderDelimiter << mRequestData->GetContentLength() << net_endl;
		os << cHeaderContentType << cHeaderDelimiter << mRequestData->GetContentType() << net_endl;
	}
}

void CHTTPRequestResponse::ClearResponse()
{
	mETagMatch = false;
	mStatusCode = eStatus_Unknown;
	mStatusReason.clear();
	mHeaders.clear();
	mConnectionClose = false;
	mContentLength = 0;
	mChunked = false;
	mCompleted = false;
	
	if (mResponseData != NULL)
		mResponseData->Clear();
}

void CHTTPRequestResponse::ParseResponseHeader(std::istream& is, std::ostream* log)
{
	// First line must be status
	cdstring line;
	getline(is, line);
	if (is.fail())
		throw CHTTPResponseException("failed to read status line of response");
	if (log != NULL)
		*log << line << os_endl;

	ParseStatusLine(line);

	// Read lines of text to form the complete set of headers
	cdstring line1;
	cdstring line2;
	bool got_blank = false;

	while(!is.fail() && ReadFoldedLine(is, line1, line2, log))
	{
		// Empty line => end of headers
		if (line1.empty())
		{
			got_blank = true;
			break;
		}

		// Look for header delimiter
		cdstring::size_type pos1 = line1.find_first_of(':');
		if (pos1 == cdstring::npos)
			throw CHTTPResponseException("illegal header line in response");
		
		// Copy header
		cdstring header(line1, 0, pos1);
		
		// Get value
		cdstring::size_type pos2 = line1.find_first_not_of("\t ", pos1 + 1);
		if (pos2 == cdstring::npos)
			pos2 = pos1 + 1;

		cdstring value(line1, pos2);
		if (value.empty())
			throw CHTTPResponseException("missing header value in response");
		
		// Add to multimap
		mHeaders.insert(std::multimap<cdstring, cdstring>::value_type(header, value));
	}
	
	// Validate
	if (!got_blank)
		throw CHTTPResponseException("missing header blank line in response");
	
	// Now cache some useful header values
	CacheHeaders();
}

void CHTTPRequestResponse::ParseStatusLine(cdstring& line)
{
	const char* p = line.c_str();
	
	// Must have 'HTTP/' version at start
	if ((*p++ != 'H') ||
		(*p++ != 'T') ||
		(*p++ != 'T') ||
		(*p++ != 'P') ||
		(*p++ != '/'))
		throw CHTTPResponseException("status line incorrect at start");
	
	// Must have version '1.1 '
	if ((*p++ != '1') ||
		(*p++ != '.') ||
		(*p++ != '1') ||
		(*p++ != ' '))
		throw CHTTPResponseException("incorrect http version in status line");
	
	// Must have three digits followed by nothing or one space
	if (!isdigit(p[0]) || !isdigit(p[1]) || !isdigit(p[2]) || (p[3] != ' ') && (p[3] != 0))
		throw CHTTPResponseException("invalid status response code syntax");
	
	// Read in the status code
	mStatusCode = (*p++ - '0') * 100;
	mStatusCode += (*p++ - '0') * 10;
	mStatusCode += (*p++ - '0');
	p++;
	
	// Remainder is reason
	mStatusReason = p;
}

bool CHTTPRequestResponse::ReadFoldedLine(std::istream& is, cdstring& line1, cdstring& line2, std::ostream* log) const
{
	// If line2 already has data, transfer that into line1
	if (!line2.empty() || !line1.empty())
		line1 = line2;
	else
	{
		// Fill first line
		getline(is, line1);
		if (is.fail())
			return false;

		if (log != NULL)
			*log << line1 << os_endl;
	}

	// Terminate on blank line which is end of headers
	if (line1.empty())
		return true;

	// Now loop looking ahead at the next line to see if it is folded
	while(true)
	{
		// Get next line
		getline(is, line2);
		if (is.fail())
			return true;

		if (log != NULL)
			*log << line2 << os_endl;
		
		// Does it start with a space => folded
		if (!line2.empty() && isspace(line2[(cdstring::size_type)0]))
			// Copy folded line (without space) to current line and cycle for more
			line1.append(line2, 1, line2.length() - 1);
		else
			// Not folded - just exit loop
			break;
	}

	return true;
}

void CHTTPRequestResponse::CacheHeaders()
{
	// Connection
	if (mHeaders.count(cHeaderConnection))
	{
		cdstring value = (*mHeaders.find(cHeaderConnection)).second;
		mConnectionClose = (value.compare(cHeaderConnectionClose, true) == 0);
	}
	// Content-Length
	if (mHeaders.count(cHeaderContentLength))
	{
		cdstring value = (*mHeaders.find(cHeaderContentLength)).second;
		mContentLength = ::strtoul(value.c_str(), NULL, 10);
	}
	
	// Transfer encoding
	if (mHeaders.count(cHeaderTransferEncoding))
	{
		cdstring value = (*mHeaders.find(cHeaderTransferEncoding)).second;
		mChunked = (value == cHeaderTransferEncodingChunked);
	}
}

const cdstring& CHTTPRequestResponse::GetResponseHeader(const cdstring& hdr) const
{
	if (mHeaders.count(hdr))
	{
		return (*mHeaders.find(hdr)).second;
	}
	else
		return cdstring::null_str;
}

cdstrvect& CHTTPRequestResponse::GetResponseHeaders(const cdstring& hdr, cdstrvect& hdrs) const
{
	if (mHeaders.count(hdr))
	{
		for(cdstrmultimapcasei::const_iterator iter = mHeaders.lower_bound(hdr); iter != mHeaders.upper_bound(hdr); iter++)
			hdrs.push_back((*iter).second);
	}

	return hdrs;
}

bool CHTTPRequestResponse::IsRedirect() const
{
	// Only some methods can do redirect
	switch(mMethod)
	{
	// HTTP allowed
	case eRequest_GET:
	case eRequest_HEAD:
		// Only these are allowed
		return (mStatusCode == eStatus_MovedPermanently) ||
				(mStatusCode == eStatus_Found) ||
				(mStatusCode == eStatus_TemporaryRedirect);
	
	// WebDAV/CalDAV allowed
	case eRequest_MKCOL:
	case eRequest_MOVE:
	case eRequest_COPY:
	case eRequest_PROPFIND:
	case eRequest_PROPPATCH:
	case eRequest_LOCK:
	case eRequest_UNLOCK:
	case eRequest_ACL:
	case eRequest_MKCALENDAR:
		// Only these are allowed
		return (mStatusCode == eStatus_MovedPermanently) ||
				(mStatusCode == eStatus_Found);
	
	default:
		// Nothing else can redirect
		return false;
	}
}
