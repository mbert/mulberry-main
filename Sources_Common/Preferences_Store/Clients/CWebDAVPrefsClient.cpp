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
	CWebDAVPrefsClient.cpp

	Author:			
	Description:	<describe the CWebDAVPrefsClient class here>
*/

#include "CWebDAVPrefsClient.h"

#include "CGeneralException.h"
#include "CINETCommon.h"
#include "CPasswordManager.h"
#include "CStatusWindow.h"
#include "CStreamFilter.h"
#include "CStreamUtils.h"
#include "CStringUtils.h"
#include "CURL.h"

#include "CHTTPAuthorizationBasic.h"
#include "CHTTPAuthorizationDigest.h"
#include "CHTTPDataString.h"
#include "CHTTPDefinitions.h"
#include "CHTTPRequestResponse.h"
#include "CWebDAVDefinitions.h"
#include "CWebDAVGet.h"
#include "CWebDAVLock.h"
#include "CWebDAVOptions.h"
#include "CWebDAVPropFind.h"
#include "CWebDAVPut.h"
#include "CWebDAVUnlock.h"

#include "XMLNode.h"

#include "ctrbuf.h"

#if __dest_os == __win32_os || __dest_os == __linux_os
#include "StValueChanger.h"
#endif

#include <memory>
#include <strstream>

using namespace prefsstore; 

CWebDAVPrefsClient::CWebDAVPrefsClient(COptionsProtocol* options_owner) :
	COptionsClient(options_owner)
{
	mOwner = options_owner;
	mPrivateAccount = NULL;

	// Create the TCP stream
	mStream = new CTCPStream;

	// Init instance variables
	InitWebDAVClient();
}

CWebDAVPrefsClient::CWebDAVPrefsClient(const CWebDAVPrefsClient& copy, COptionsProtocol* options_owner) :
	COptionsClient(copy, options_owner)
{
	mOwner = options_owner;
	mPrivateAccount = NULL;
	mStream = copy.mStream ? new CTCPStream(*copy.mStream) : NULL;

	// Stream will be copied by parent constructor

	// Init instance variables
	InitWebDAVClient();
}

CWebDAVPrefsClient::~CWebDAVPrefsClient()
{
}

void CWebDAVPrefsClient::InitWebDAVClient()
{
	mLogType = CLog::eLogHTTP;
}

// Create duplicate, empty connection
CINETClient* CWebDAVPrefsClient::CloneConnection()
{
	// Copy construct this
	return new CWebDAVPrefsClient(*this, GetOptionsOwner());

}

// Get default port
tcp_port CWebDAVPrefsClient::GetDefaultPort()
{
	// TCP/IP-based sub-classes must deal with this
	if ((GetAccount()->GetTLSType() == CINETAccount::eSSL) || (GetAccount()->GetTLSType() == CINETAccount::eSSLv3))
		return http::cHTTPServerPort_SSL;
	else
		return http::cHTTPServerPort;
}

#pragma mark ____________________________Start/Stop

// Start TCP
void CWebDAVPrefsClient::Open()
{
	// Do account reset
	Reset();
	
	// Do inherited
	COptionsClient::Open();
}

// Reset acount info
void CWebDAVPrefsClient::Reset()
{
	mBaseRURL = GetOptionsOwner()->GetOptionsAccount()->GetBaseRURL();
	if (mBaseRURL.empty())
		mBaseRURL = "/";
	else if (mBaseRURL[mBaseRURL.length() - 1] != '/')
		mBaseRURL += "/";
	
	// Must always start with /
	if (mBaseRURL[0UL] != '/')
	{
		cdstring temp(mBaseRURL);
		mBaseRURL = "/";
		mBaseRURL += temp;
	}
	mBaseRURL.EncodeURL('/');
	
	// Get absolute URL
	if ((GetAccount()->GetTLSType() == CINETAccount::eSSL) || (GetAccount()->GetTLSType() == CINETAccount::eSSLv3))
		mBaseURL = cHTTPSURLScheme;
	else
		mBaseURL = cHTTPURLScheme;
	
	mBaseURL += GetAccount()->GetServerIP();
	mBaseURL += mBaseRURL;
	
	SetServerType(eDAVUnknown);

	// Check for account details change
	if (!CheckCurrentAuthorization())
	{
		delete mAuthorization;
		mAuthorization = NULL;
	}
}

#pragma mark ____________________________Login & Logout

void CWebDAVPrefsClient::Logon()
{
	// Do initialisation if not already done
	if (!Initialise(mServerAddr, mBaseRURL))
	{
		// Break connection with server
		CloseConnection();
		return;
	}
	
	// Recache user id & password after successful logon
	if (GetAccount()->GetAuthenticator().RequiresUserPswd())
	{
		CAuthenticatorUserPswd* auth = GetAccount()->GetAuthenticatorUserPswd();
		
		// Only bother if it contains something
		if (!auth->GetPswd().empty())
		{
			CPasswordManager::GetManager()->AddPassword(GetAccount(), auth->GetPswd());
		}
	}
}

void CWebDAVPrefsClient::Logoff()
{
	// Nothing to do for local
}

#pragma mark ____________________________Handle Errors

// Descriptor for object error context
const char*	CWebDAVPrefsClient::INETGetErrorDescriptor() const
{
	return "Options: ";
}

#pragma mark ____________________________Protocol

// Initialise capability flags to empty set
void CWebDAVPrefsClient::_InitCapability()
{
	// Init capabilities to empty set

	mAuthLoginAllowed = false;
	mAuthPlainAllowed = false;
	mAuthAnonAllowed = false;
	mSTARTTLSAllowed = false;

	mCapability = cdstring::null_str;
}

// Check version of server
void CWebDAVPrefsClient::_ProcessCapability()
{
	// Look for other capabilities

}

// Tickle to keep connection alive
void CWebDAVPrefsClient::_Tickle(bool force_tickle)
{
	// No HTTP protocol keep-alive
}

#pragma mark ____________________________Options

// Find all options below this key
void CWebDAVPrefsClient::_FindAllAttributes(const cdstring& entry)
{
	StINETClientAction action(this, "Status::IMSP::Getting", "Error::IMSP::OSErrGet", "Error::IMSP::NoBadGet");

	cdstring option = cWILDCARD;

	cdstring data;
	ReadData(entry, data);
	std::istrstream is(data.c_str());

	// Read lines from file
	while(!is.fail())
	{
		// Get a line - may end with any endl
		cdstring line;
		getline(is, line, 0);
		
		// Split into key & value
		char* key = ::strtok(line.c_str_mod(), "=");
		char* value = ::strtok(NULL, "\r\n");

		// Insert into map - ignore duplicates
		if (key && ::strpmatch(key, option))
		{
			if (!value)
				value = cdstring::null_str;

			// Insert
			std::pair<cdstrmap::iterator, bool> result = GetOptionsOwner()->GetMap()->insert(cdstrmap::value_type(key, value));

			// Does it exist already
			if (!result.second)
				// Replace existing
				(*result.first).second = value;
		}
	}
}

// Set all options in owmer map
void CWebDAVPrefsClient::_SetAllAttributes(const cdstring& entry)
{
	StINETClientAction action(this, "Status::IMSP::Setting", "Error::IMSP::OSErrSet", "Error::IMSP::NoBadSet");

	std::ostrstream os;
	
	for(cdstrmap::const_iterator iter = GetOptionsOwner()->GetMap()->begin(); iter != GetOptionsOwner()->GetMap()->end(); iter++)
	{
		os << (*iter).first;
		os.put('=');
		os << (*iter).second;
		os << net_endl;
	}
	os << std::ends;
	
	WriteData(entry, os.str());
}

// Delete the entire entry
void CWebDAVPrefsClient::_DeleteEntry(const cdstring& entry)
{
	// Cannot do this on flat file!
}

// Get option
void CWebDAVPrefsClient::_GetAttribute(const cdstring& entry, const cdstring& attribute)
{
	// Cannot do this on flat file!
}

// Set option
void CWebDAVPrefsClient::_SetAttribute(const cdstring& entry, const cdstring& attribute, const cdstring& value)
{
	// Cannot do this on flat file!
}

// Unset option with this key
void CWebDAVPrefsClient::_DeleteAttribute(const cdstring& entry, const cdstring& attribute)
{
	// Cannot do this on flat file!
}

#pragma mark ____________________________Utils

cdstring CWebDAVPrefsClient::GetETag(const cdstring& rurl, const cdstring& lock_token)
{
	cdstring result = GetProperty(rurl, lock_token, http::webdav::cProperty_getetag);

	// Handle server bug: ETag value MUST be quoted per HTTP/1.1 §3.11
	if (!result.empty() && !result.isquoted())
		result.quote(true);

	return result;
}

cdstring CWebDAVPrefsClient::GetProperty(const cdstring& rurl, const cdstring& lock_token, const xmllib::XMLName& property)
{
	cdstring result;

	// Create WebDAV propfind
	xmllib::XMLNameList props;
	props.push_back(property);
	std::auto_ptr<http::webdav::CWebDAVPropFind> request(new http::webdav::CWebDAVPropFind(this, rurl, http::webdav::eDepth0, props));
	http::CHTTPOutputDataString dout;
	request->SetOutput(&dout);

	// Process it
	RunSession(request.get());

	// If its a 207 we want to parse the XML
	if (request->GetStatusCode() == http::webdav::eStatus_MultiStatus)
	{
		http::webdav::CWebDAVPropFindParser parser;
		parser.ParseData(dout.GetData());

		// Look at each propfind result and determine type of calendar
		cdstring decoded_rurl = rurl;
		decoded_rurl.DecodeURL();
		for(http::webdav::CWebDAVPropFindParser::CPropFindResults::const_iterator iter = parser.Results().begin(); iter != parser.Results().end(); iter++)
		{
			// Get child element name (decode URL)
			cdstring name((*iter)->GetResource());
			name.DecodeURL();
		
			// Must match rurl
			if (name.compare_end(decoded_rurl))
			{
				if ((*iter)->GetTextProperties().count(property.FullName()) != 0)
				{
					result = (*(*iter)->GetTextProperties().find(property.FullName())).second;
					break;
				}
			}
		}
	}
	else
	{
		HandleHTTPError(request.get());
	}

	return result;
}

cdstring CWebDAVPrefsClient::LockResource(const cdstring& rurl, unsigned long timeout, bool lock_null)
{
	cdstring result;

	// Lock owner is auth principle @ server ip
	CAuthenticator* acct_auth = GetAccount()->GetAuthenticator().GetAuthenticator();
	cdstring owner = acct_auth->GetActualUID();
	owner += "@";
	owner += GetAccount()->GetServerIP();

	// Create WebDAV LOCK (5 minutes should be enough)
	std::auto_ptr<http::webdav::CWebDAVLock> request(new http::webdav::CWebDAVLock(this, rurl, http::webdav::eDepth0, http::webdav::CWebDAVLock::eExclusive, owner,
																				timeout, lock_null ? http::webdav::CWebDAVLock::eResourceMustNotExist : http::webdav::CWebDAVLock::eResourceMustExist));

	// Process it
	RunSession(request.get());	
	
	// Check response status
	switch(request->GetStatusCode())
	{
	case http::eStatus_OK:
	case http::eStatus_NoContent:
		result = request->GetLockToken();
		break;
	case http::eStatus_PreconditionFailed:
		if (lock_null)
		{
			// Special code to incidate existing resource when trying to do lock-null
			result = "?";
			break;
		}
		
		// Fall through for regular error handling
	default:
		// Handle error and exit here
		HandleHTTPError(request.get());
		return result;
	}
	
	return result;
}
	
void CWebDAVPrefsClient::UnlockResource(const cdstring& rurl, const cdstring& lock_token)
{
	// Create WebDAV UNLOCK
	std::auto_ptr<http::webdav::CWebDAVUnlock> request(new http::webdav::CWebDAVUnlock(this, rurl, lock_token));

	// Process it
	RunSession(request.get());	
	
	// Check response status
	switch(request->GetStatusCode())
	{
	case http::eStatus_OK:
	case http::eStatus_NoContent:
		// Do default action
		break;
	default:
		// Handle error and exit here
		HandleHTTPError(request.get());
		return;
	}
}

void CWebDAVPrefsClient::OpenSession()
{
	// Init TCP
	mStream->TCPOpen();

	// Set server TCP info if not already
	LookupServer();

	// Look for SSL and turn on here
	if (mOwner && ((GetAccount()->GetTLSType() == CINETAccount::eSSL) || (GetAccount()->GetTLSType() == CINETAccount::eSSLv3)))
	{
		mStream->TLSSetTLSOn(true, GetAccount()->GetTLSType());
		
		// Check for client cert
		if (GetAccount()->GetUseTLSClientCert())
		{
			// Try to load client certificate
			if (!DoTLSClientCertificate())
			{
				CLOG_LOGTHROW(unsigned long, 1UL);
				throw 1UL;
			}
		}
	}
	else
		mStream->TLSSetTLSOn(false);

	// Make connection with server
	mStream->TCPStartConnection();

	// Write to log file
	if (mAllowLog)
	{
		mLog.LogEntry("        <-------- BEGIN HTTP CONNECTION -------->");
		cdstring str("Server: ");
		str += mServerAddr;
		mLog.AddEntry(str);
	}
}

void CWebDAVPrefsClient::CloseSession()
{
	// Break connection with server
	mStream->TCPCloseConnection();

	// Write to log file
	if (mAllowLog)
	{
		mLog.LogEntry("        <-------- END HTTP CONNECTION -------->");
	}
}

void CWebDAVPrefsClient::RunSession(CHTTPRequestResponse* request)
{
	// Do initialisation if not already done
	if (!Initialised())
	{
		if (!Initialise(mServerAddr, mBaseRURL))
		{
			// Break connection with server
			CloseConnection();
			return;
		}
		
		// Recache user id & password after successful logon
		if (GetAccount()->GetAuthenticator().RequiresUserPswd())
		{
			CAuthenticatorUserPswd* auth = GetAccount()->GetAuthenticatorUserPswd();
			
			// Only bother if it contains something
			if (!auth->GetPswd().empty())
			{
				CPasswordManager::GetManager()->AddPassword(GetAccount(), auth->GetPswd());
			}
		}
	}
	
	// Server, base uri may change due to redirection
	StValueChanger<cdstring> _preserve1(mServerAddr, mServerAddr);
	StValueChanger<cdstring> _preserve2(mBaseRURL, mBaseRURL);

	// Loop over redirects upto 5 times
	uint32_t ctr = 5;
	while(ctr--)
	{
		// Run a session
		DoSession(request);

		// Check for auth failure if none before
		if ((request != NULL) && (request->IsRedirect()))
		{
			// Get new location
			cdstring location = request->GetResponseHeader(cHeaderLocation);
			if (!location.empty())
			{
				CURL parsed(location);
				if (((parsed.SchemeType() == CURL::eHTTP) || (parsed.SchemeType() == CURL::eHTTPS)) && !parsed.Server().empty() && !parsed.Path().empty())
				{
					// Get new server and base RURL
					bool different_server = (mServerAddr != parsed.Server());
					mServerAddr = parsed.Server();
					mBaseRURL = parsed.Path();
					
					// Use new host in this session
					if (different_server)
					{
						if (GetPrivateAccount() != NULL)
							GetPrivateAccount()->SetServerIP(parsed.Server());
						SetHost(mServerAddr);
					}
					
					// Reset the request with new info
					request->SetRURI(parsed.Path());
					request->ClearResponse();

					// Write to log file
					if (mAllowLog)
					{
						mLog.LogEntry("        <-------- HTTP REDIRECT -------->");
						cdstring str("Location: ");
						str += location;
						mLog.AddEntry(str);
					}

					// Recyle through loop
					continue;
				}
			}
		}

		// Exit when redirect does not occur
		break;
	}
}


void CWebDAVPrefsClient::DoSession(CHTTPRequestResponse* request)
{
	// Do the request if present
	if (request != NULL)
	{
		// Handle delayed authorization
		bool first_time = true;
		while(true)
		{
			// Run the request actions - this will make any connection that is needed
			SendRequest(request);
			
			// Check for auth failure if none before
			if (request->GetStatusCode() == eStatus_Unauthorized)
			{
				// If we had authorization before, then chances are auth details are wrong - so delete and try again with new auth
				if (HasAuthorization())
				{
					delete mAuthorization;
					mAuthorization = NULL;
					
					// Display error so user knows why the prompt occurs again
					if (!first_time)
						DisplayHTTPError(request);
				}

				// Get authorization object (prompt the user) and redo the request
				cdstrvect hdrs;
				mAuthorization = GetAuthorization(first_time, request->GetResponseHeaders(cHeaderWWWAuthenticate, hdrs));
				
				// Check for auth cancellation
				if (mAuthorization == (CHTTPAuthorization*) -1)
				{
					mAuthorization = NULL;
				}
				else
				{
					first_time = false;
					
					request->ClearResponse();
					
					// Repeat the request loop with new authorization
					continue;
				}
			}
			
			// Recache user id & password after successful logon
			if (!first_time && GetAccount()->GetAuthenticator().RequiresUserPswd())
			{
				CAuthenticatorUserPswd* auth = GetAccount()->GetAuthenticatorUserPswd();

				// Only bother if it contains something
				if (!auth->GetPswd().empty())
				{
					CPasswordManager::GetManager()->AddPassword(GetAccount(), auth->GetPswd());
				}
			}

			// If we get here we are complete with auth loop
			break;
		}
	}

	// Now close it - eventually we will do keep-alive support

	// Break connection with server
	CloseConnection();
}

static const char* cType_HTTP = "HTTP";
static const char* cType_DAV1 = "DAV Level 1";
static const char* cType_DAV2 = "DAV Level 2";
static const char* cType_DAV2bis = "DAV Level 2bis";

void CWebDAVPrefsClient::SetServerType(unsigned long type)
{
	if (type & eDAV2bis)
		mOwner->SetType(cType_DAV2bis);
	else if (type & eDAV2)
		mOwner->SetType(cType_DAV2);
	else if (type & eDAV1)
		mOwner->SetType(cType_DAV1);
	else
		mOwner->SetType(cType_HTTP);
}

void CWebDAVPrefsClient::SetServerDescriptor(const cdstring& txt)
{
	mGreeting = txt;
}

void CWebDAVPrefsClient::SetServerCapability(const cdstring& txt)
{
	_ProcessCapability();
	mCapability = txt;
}

CHTTPAuthorization* CWebDAVPrefsClient::GetAuthorization(bool first_time, const cdstrvect& www_authenticate)
{
	// Loop while trying to authentciate
	CAuthenticator* acct_auth = GetAccount()->GetAuthenticator().GetAuthenticator();

	// Pause busy cursor before we do the prompt
	bool paused = INETPauseAction(true);
	
	// Loop while waiting for successful login with optional user prompt
	if (CMailControl::PromptUser(acct_auth, GetAccount(), GetAccount()->IsSecure(),
									false, false, true, false, false, false, first_time))
	{
		// Restart busy cursor after prompt
		if (paused)
			paused = INETPauseAction(false);
		
		switch(GetAccount()->GetAuthenticatorType())
		{
		case CAuthenticator::eNone:			// Actually ANONYMOUS!
			return NULL;

		case CAuthenticator::ePlainText:
		{
			CAuthenticatorUserPswd* auth = GetAccount()->GetAuthenticatorUserPswd();
			mAuthUniqueness = GetAccount()->GetAuthenticator().GetUniqueness();
			return new CHTTPAuthorizationBasic(auth->GetUID(), auth->GetPswd());
		}

		case CAuthenticator::eSSL:
			return NULL;					// Already authenticated via SSL - no additional authorization required

		// These ones do AUTHENTICATE processing via plugin
		case CAuthenticator::ePlugin:
			// Look for DIGEST-MD5
			if (GetAccount()->GetAuthenticator().GetSASLID() == "DIGEST-MD5")
			{
				CAuthenticatorUserPswd* auth = GetAccount()->GetAuthenticatorUserPswd();
				mAuthUniqueness = GetAccount()->GetAuthenticator().GetUniqueness();
				return new CHTTPAuthorizationDigest(auth->GetUID(), auth->GetPswd(), www_authenticate);
			}
			else
				return NULL;					// Currently not supported with http
		}
	}

	// Restart busy cursor after prompt
	if (paused)
		paused = INETPauseAction(false);
	
	return (CHTTPAuthorization*) -1;
}

bool CWebDAVPrefsClient::CheckCurrentAuthorization() const
{
	return mAuthUniqueness == GetAccount()->GetAuthenticator().GetUniqueness();
}

void CWebDAVPrefsClient::DoRequest(CHTTPRequestResponse* request)
{
	// Write request headers
	{
		cdstring hdrs = request->GetRequestHeader();
		mStream->write(hdrs.c_str(), hdrs.length());

		// Write to log file
		if (mAllowLog)
		{
			mLog.LogEntry("        <-------- BEGIN HTTP REQUEST -------->");
			hdrs.ConvertEndl();
			mLog.AddEntry(hdrs);
		}
	}

	// Write the data
	WriteRequestData(request);

	// Flush all request data
	*mStream << std::flush;
	
	// Blank line in log between 
	if (mAllowLog)
	{
		mLog.AddEntry("        <-------- BEGIN HTTP RESPONSE -------->");
	}

	// Now get the response header
	while(true)
	{
		{
			// Do CRLF -> lendl conversion for log
			std::auto_ptr<CStreamFilter> filter(mAllowLog ? new CStreamFilter(new crlf_filterbuf(lendl), mLog.GetLog()) : NULL);
			request->ParseResponseHeader(*mStream, filter.get());
		}

		// Check for a WebDAV HTTP 102 response which means the client should continue to wait
		// for the final response
		if (request->GetStatusCode() == http::webdav::eStatus_Processing)
		{
			// Clear stream and response so we start over with a new response result etc
			request->ClearResponse();
			
			// Just continue to wait for real response
			continue;
		}
		
		// Now get the data
		ReadResponseData(request);
		break;
	}

	// Trailer in log 
	if (mAllowLog)
	{
		mLog.AddEntry("        <-------- END HTTP RESPONSE -------->");
	}
}

void CWebDAVPrefsClient::WriteRequestData(CHTTPRequestResponse* request)
{
	// Tell data we are using it
	request->StartRequestData();
	
	// Write the data if any present
	if (request->HasRequestData() && (request->GetRequestDataStream() != NULL))
	{
		unsigned long stream_length = ::StreamLength(*request->GetRequestDataStream());
		::StreamCopy(*request->GetRequestDataStream(), *mStream, 0, stream_length);

		// Write to log file
		if (mAllowLog)
		{
			// Do CRLF -> lendl conversion for log
			CStreamFilter filter(new crlf_filterbuf(lendl), mLog.GetLog());
			::StreamCopy(*request->GetRequestDataStream(), filter, 0, stream_length);
		}
	}

	// Tell data we are done using it
	request->StopRequestData();
}

void CWebDAVPrefsClient::ReadResponseData(CHTTPRequestResponse* request)
{
	// Tell data we are using it
	request->StartResponseData();

	// Special behaviour for chunked
	if (request->GetChunked())
	{
		// Read chunks of data until no more are left
		unsigned long chunk_size = 0;
		while((chunk_size = ReadResponseDataChunked(request)) != 0)
		{
			ReadResponseDataLength(request, chunk_size);
		}
		
		// Read empty line
		INETGetLine();
	}
	else
	{
		ReadResponseDataLength(request, request->GetContentLength());
	}

	// Tell data we are done using it
	request->StopResponseData();
}

void CWebDAVPrefsClient::ReadResponseDataLength(CHTTPRequestResponse* request, unsigned long read_length)
{
	// Do CRLF -> lendl conversion for log
	std::auto_ptr<CStreamFilter> filter(mAllowLog ? new CStreamFilter(new crlf_filterbuf(lendl), mLog.GetLog()) : NULL);

	// Create network status item for % progress counter
	CNetworkAttachProgress progress;

	long length = read_length;
	if (request->GetResponseDataStream() != NULL)
		mStream->gettostream(*request->GetResponseDataStream(), filter.get(), &length, &progress);
	else
	{
		// Create counting stream which allows us to simply discard the data read in
		ctrbuf ctr;
		std::ostream octr(&ctr);

		mStream->gettostream(octr, filter.get(), &length, &progress);
	}
}

unsigned long CWebDAVPrefsClient::ReadResponseDataChunked(CHTTPRequestResponse* request)
{
	while(true)
	{
		const char* line = INETGetLine();
		if (line && *line)
		{
			return ::strtoul(line, NULL, 16);
		}
	}
	
	return 0;
}

void CWebDAVPrefsClient::HandleHTTPError(CHTTPRequestResponse* request)
{
	DisplayHTTPError(request);
}

void CWebDAVPrefsClient::DisplayHTTPError(CHTTPRequestResponse* request)
{
	// Fake response
	mLastResponse.code = cTagNO;
	mLastResponse.tag_msg = request->GetStatusReason();

	CINETException fake_ex(CINETException::err_NoResponse);
	INETDisplayError(fake_ex, mErrorID, mNoBadID);
}

// Return the encoded url for the node
cdstring CWebDAVPrefsClient::GetRURL(const cdstring& name, bool directory, bool abs) const
{
	// Determine URL
	cdstring rurl = (abs ? mBaseURL : mBaseRURL);
	cdstring temp(name);
	temp.EncodeURL('/');
	rurl += temp;
	if (directory)
	{
		if (rurl[rurl.length() - 1] != '/')
			rurl += "/";
	}
	else
	{
		rurl += ".mbp";
	}
	
	return rurl;
}

// Get lock token for this rurl
cdstring CWebDAVPrefsClient::GetLockToken(const cdstring& rurl) const
{
	cdstrmap::const_iterator found = mLockTokens.find(rurl);
	if (found != mLockTokens.end())
		return (*found).second;
	else
		return cdstring::null_str;
}

bool CWebDAVPrefsClient::LockToken(const cdstring& rurl, unsigned long timeout, bool lock_null)
{
	// See if already locked
	cdstrmap::const_iterator found = mLockTokens.find(rurl);
	if (found != mLockTokens.end())
		return false;
	else
	{
		cdstring lock_token = LockResource(rurl, timeout, lock_null);
		mLockTokens.insert(cdstrmap::value_type(rurl, lock_token));
		return true;
	}
}

void CWebDAVPrefsClient::UnlockToken(const cdstring& rurl)
{
	// See if already locked
	cdstrmap::const_iterator found = mLockTokens.find(rurl);
	if (found != mLockTokens.end())
	{
		cdstring lock_token = (*found).second;
		mLockTokens.erase(rurl);
		UnlockResource(rurl, lock_token);
	}
}

void CWebDAVPrefsClient::WriteData(const cdstring& entry, const cdstring& data)
{
	// Determine URL
	cdstring rurl = GetRURL(entry, false);

	// Create WebDAV PUT
	std::auto_ptr<http::webdav::CWebDAVPut> request(new http::webdav::CWebDAVPut(this, rurl));
	http::CHTTPInputDataString din(data, "text/plain; charset=utf-8");
	http::CHTTPOutputDataString dout;
	
	// Write resource
	request->SetData(&din, &dout, false);

	// Process it
	RunSession(request.get());
	
	// Check response status
	switch(request->GetStatusCode())
	{
	case http::eStatus_OK:
	case http::eStatus_Created:
	case http::eStatus_NoContent:
		// Do default action
		break;
	default:
		// Handle error and exit here
		HandleHTTPError(request.get());
		return;
	}

}

void CWebDAVPrefsClient::ReadData(const cdstring& entry, cdstring& data)
{
	cdstring rurl = GetRURL(entry, false);

	// Create WebDAV GET
	std::auto_ptr<http::webdav::CWebDAVGet> request(new http::webdav::CWebDAVGet(this, rurl));
	http::CHTTPOutputDataString dout;
	request->SetData(&dout);

	// Process it
	RunSession(request.get());
	
	
	// Check response status
	switch(request->GetStatusCode())
	{
	case http::eStatus_OK:
		// Do default action
		break;
	default:
		// Handle error and exit here
		HandleHTTPError(request.get());
		return;
	}

	// Read calendar from file
	data = dout.GetData();
}
