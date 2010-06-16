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
	CWebDAVCalendarClient.cpp

	Author:			
	Description:	<describe the CWebDAVCalendarClient class here>
*/

#include "CWebDAVCalendarClient.h"

#include "CCalendarAccount.h"
#include "CCalendarProtocol.h"
#include "CCalendarStoreNode.h"
//#include "CDisplayItem.h"
#include "CGeneralException.h"
#include "CPasswordManager.h"
#include "CStatusWindow.h"
#include "CStreamFilter.h"
#include "CStreamUtils.h"
#include "CURL.h"

#include "CHTTPAuthorizationBasic.h"
#include "CHTTPAuthorizationDigest.h"
#include "CHTTPDataString.h"
#include "CHTTPDefinitions.h"
#include "CHTTPRequestResponse.h"
#include "CWebDAVACL.h"
#include "CWebDAVDefinitions.h"
#include "CWebDAVDelete.h"
#include "CWebDAVGet.h"
#include "CWebDAVLock.h"
#include "CWebDAVMakeCollection.h"
#include "CWebDAVMove.h"
#include "CWebDAVOptions.h"
#include "CWebDAVPrincipalMatch.h"
#include "CWebDAVPropFind.h"
#include "CWebDAVPut.h"
#include "CWebDAVUnlock.h"

#include "CICalendar.h"
#include "CICalendarSync.h"

#include "ctrbuf.h"

#include "XMLNode.h"

#if __dest_os == __win32_os || __dest_os == __linux_os
#include "StValueChanger.h"
#endif

#include <memory>
#include <strstream>

using namespace calstore; 

CWebDAVCalendarClient::CWebDAVCalendarClient(CCalendarProtocol* owner) :
	CCalendarClient(owner)
{
	// Create the TCP stream
	mStream = new CTCPStream;

	// Init instance variables
	InitWebDAVClient();
}

CWebDAVCalendarClient::CWebDAVCalendarClient(const CWebDAVCalendarClient& copy, CCalendarProtocol* owner) :
	CCalendarClient(copy, owner)
{
	// Stream will be copied by parent constructor

	// Init instance variables
	InitWebDAVClient();
}

CWebDAVCalendarClient::~CWebDAVCalendarClient()
{
}

void CWebDAVCalendarClient::InitWebDAVClient()
{
	mLogType = CLog::eLogHTTP;
}

// Create duplicate, empty connection
CINETClient* CWebDAVCalendarClient::CloneConnection()
{
	// Copy construct this
	return new CWebDAVCalendarClient(*this, GetCalendarProtocol());

}

// Get default port
tcp_port CWebDAVCalendarClient::GetDefaultPort()
{
	// TCP/IP-based sub-classes must deal with this
	if ((GetAccount()->GetTLSType() == CINETAccount::eSSL) ||
		(GetAccount()->GetTLSType() == CINETAccount::eSSLv3))
		return http::cHTTPServerPort_SSL;
	else
		return http::cHTTPServerPort;
}

#pragma mark ____________________________Start/Stop

// Start TCP
void CWebDAVCalendarClient::Open()
{
	// Do account reset
	Reset();
	
	// Do inherited
	CCalendarClient::Open();
}

// Reset account info
void CWebDAVCalendarClient::Reset()
{
	_Reset(GetCalendarProtocol()->GetCalendarAccount()->GetBaseRURL());
	
	SetServerType(eDAVUnknown);
	
	// Check for account details change
	if (!CheckCurrentAuthorization())
	{
		delete mAuthorization;
		mAuthorization = NULL;
	}
}

// Reset account info
void CWebDAVCalendarClient::_Reset(const cdstring& baseRURL)
{
	mBaseRURL = baseRURL;
	if (mBaseRURL.empty())
		mBaseRURL = "/";
	else if (mBaseRURL[(cdstring::size_type)(mBaseRURL.length() - 1)] != '/')
		mBaseRURL += "/";
	
	// Must always start with /
	if (mBaseRURL[(cdstring::size_type)0] != '/')
	{
		cdstring temp(mBaseRURL);
		mBaseRURL = "/";
		mBaseRURL += temp;
	}
	mBaseRURL.EncodeURL('/');
	
	cdstring temp(mBaseRURL);
	temp.erase(temp.length() -1);
	GetCalendarProtocol()->GetStoreRoot()->SetName(temp);
	
	// Get absolute URL
	if ((GetAccount()->GetTLSType() == CINETAccount::eSSL) ||
		(GetAccount()->GetTLSType() == CINETAccount::eSSLv3))
		mBaseURL = cHTTPSURLScheme;
	else
		mBaseURL = cHTTPURLScheme;
	
	mBaseURL += GetAccount()->GetServerIP();
	mHostURL = mBaseURL;
	mBaseURL += mBaseRURL;
	
}

#pragma mark ____________________________Login & Logout

void CWebDAVCalendarClient::Logon()
{
	// HTTP calendars do not login
	if (GetAccount()->GetServerType() == CINETAccount::eHTTPCalendar)
		return;

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

void CWebDAVCalendarClient::Logoff()
{
	// Nothing to do for local
}

#pragma mark ____________________________Handle Errors

// Descriptor for object error context
const char*	CWebDAVCalendarClient::INETGetErrorDescriptor() const
{
	return "Calendar: ";
}

#pragma mark ____________________________Protocol

// Initialise capability flags to empty set
void CWebDAVCalendarClient::_InitCapability()
{
	// Init capabilities to empty set
	GetCalendarProtocol()->SetHasACL(false);

	mAuthLoginAllowed = false;
	mAuthPlainAllowed = false;
	mAuthAnonAllowed = false;
	mSTARTTLSAllowed = false;

	mCapability = cdstring::null_str;
}

// Check version of server
void CWebDAVCalendarClient::_ProcessCapability()
{
	// Look for other capabilities
	GetCalendarProtocol()->SetHasACL(HasDAVACL());

}

// Tickle to keep connection alive
void CWebDAVCalendarClient::_Tickle(bool force_tickle)
{
	// No HTTP protocol keep-alive
}

void CWebDAVCalendarClient::_ListCalendars(CCalendarStoreNode* root)
{
	// Node must be protocol or directory
	if (!root->IsProtocol() && !root->IsDirectory())
		return;

	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Listing", "Error::Calendar::OSErrListCalendars", "Error::Calendar::NoBadListCalendars");

	// Determine URL
	cdstring rurl = root->GetName();
	rurl.EncodeURL('/');
	if (root->IsProtocol())
		rurl = mBaseRURL;
	
	// Create WebDAV propfind
	xmllib::XMLNameList props;
	props.push_back(http::webdav::cProperty_getcontentlength);
	props.push_back(http::webdav::cProperty_getcontenttype);
	props.push_back(http::webdav::cProperty_resourcetype);
	props.push_back(http::webdav::cProperty_displayname);
	std::auto_ptr<http::webdav::CWebDAVPropFind> request(new http::webdav::CWebDAVPropFind(this, rurl, http::webdav::eDepth1, props));
	http::CHTTPOutputDataString dout;
	request->SetOutput(&dout);

	// Process it
	RunSession(request.get());

	// If its a 207 we want to parse the XML
	if (request->GetStatusCode() == http::webdav::eStatus_MultiStatus)
	{
		http::webdav::CWebDAVPropFindParser parser;
		parser.ParseData(dout.GetData());
		ListCalendars(root, parser);
	}
	else
	{
		HandleHTTPError(request.get());
		return;
	}
	
	// Always sort children after adding all of them
	root->SortChildren();
	root->SetHasExpanded(true);
}

void CWebDAVCalendarClient::ListCalendars(CCalendarStoreNode* root, const http::webdav::CWebDAVPropFindParser& parser)
{
	cdstring nodeURL = root->GetName();
	nodeURL.EncodeURL('/');
	if (root->IsProtocol())
		nodeURL = mBaseRURL;
	cdstring nodeURLNoSlash = nodeURL;
	if (nodeURLNoSlash.compare_end("/"))
		nodeURLNoSlash.erase(nodeURLNoSlash.length() - 1);
	else
	{
		nodeURLNoSlash = nodeURL;
		nodeURL += "/";
	}
	
	cdstring relBase = nodeURL;
	cdstring relBaseNoSlash = nodeURLNoSlash;

	// Look at each propfind result and determine type of calendar
	for(http::webdav::CWebDAVPropFindParser::CPropFindResults::const_iterator iter = parser.Results().begin(); iter != parser.Results().end(); iter++)
	{
		// Get child element name (decode URL)
		cdstring rpath((*iter)->GetResource());
	
		// Strip of path prefix
		if (rpath.compare_start(mHostURL))
			rpath.erase(0, mHostURL.length());

		// Ignore root
		if ((rpath == relBase) || (rpath == relBaseNoSlash))
			continue;
		rpath.DecodeURL();

		// Strip off trailing .ics
		if (rpath.compare_end(".ics"))
			rpath.erase(rpath.length() - 4);
		else if (rpath.compare_end("/"))
			rpath.erase(rpath.length() - 1);

		// Determine type of element
		bool is_dir = false;
		if ((*iter)->GetNodeProperties().count(http::webdav::cProperty_resourcetype.FullName()) != 0)
		{
			// Get the XML node for resourcetype
			const xmllib::XMLNode* node = (*(*iter)->GetNodeProperties().find(http::webdav::cProperty_resourcetype.FullName())).second;
			
			// Look at each child element
			for(xmllib::XMLNodeList::const_iterator iter2 = node->Children().begin(); iter2 != node->Children().end(); iter2++)
			{
				// Look for collection
				if ((*iter2)->CompareFullName(http::webdav::cProperty_collection))
					is_dir = true;
			}
		}

		// Create the new node and add to parent
		CCalendarStoreNode* node = new CCalendarStoreNode(GetCalendarProtocol(), root, is_dir, false, false, rpath);
		root->AddChild(node);
		
		// Grab the size if present
		if (!is_dir && ((*iter)->GetTextProperties().count(http::webdav::cProperty_getcontentlength.FullName()) != 0))
		{
			cdstring result = (*(*iter)->GetTextProperties().find(http::webdav::cProperty_getcontentlength.FullName())).second;
			long size = ::strtoul(result.c_str(), NULL, 10);
			node->SetSize(size);
		}

		if ((*iter)->GetTextProperties().count(http::webdav::cProperty_displayname.FullName()) != 0)
		{
			cdstring result = (*(*iter)->GetTextProperties().find(http::webdav::cProperty_displayname.FullName())).second;
			// Temporary FIX for getting GUIDs back from the server
			if (result.length() == 36 && result.c_str()[8] == '-')
				result = cdstring::null_str;
			node->SetDisplayName(result);
		}
	}
}

void CWebDAVCalendarClient::_CreateCalendar(const CCalendarStoreNode& node)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Creating", "Error::Calendar::OSErrCreateCalendar", "Error::Calendar::NoBadCreateCalendar", node.GetName());

	// Determine URL
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');

	// Use MKCOL for directories
	if (node.IsDirectory())
	{
		// Create WebDAV MKCOL (overwrite not allowed)
		std::auto_ptr<http::webdav::CWebDAVMakeCollection> request(new http::webdav::CWebDAVMakeCollection(this, rurl));

		// Process it
		RunSession(request.get());
	
		// Check response status
		switch(request->GetStatusCode())
		{
		case http::eStatus_OK:
		case http::eStatus_Created:
			break;
		default:
			HandleHTTPError(request.get());
			return;
		}
	}
	else
	{
		// Put empty data
		cdstring data;

		// Create WebDAV PUT
		std::auto_ptr<http::webdav::CWebDAVPut> request(new http::webdav::CWebDAVPut(this, rurl));
		http::CHTTPInputDataString din(data, "text/calendar; charset=utf-8");
		http::CHTTPOutputDataString dout;
		request->SetData(&din, &dout);

		// Process it
		RunSession(request.get());	
	
		// Check response status
		switch(request->GetStatusCode())
		{
		case http::eStatus_OK:
		case http::eStatus_Created:
			break;
		default:
			HandleHTTPError(request.get());
			return;
		}
	}
}

void CWebDAVCalendarClient::_DeleteCalendar(const CCalendarStoreNode& node)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Deleting", "Error::Calendar::OSErrDeleteCalendar", "Error::Calendar::NoBadDeleteCalendar", node.GetName());

	// Determine URL
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');

	// Create WebDAV DELETE
	std::auto_ptr<http::webdav::CWebDAVDelete> request(new http::webdav::CWebDAVDelete(this, rurl));

	// Process it
	RunSession(request.get());
	
	// Check response status
	switch(request->GetStatusCode())
	{
	case http::eStatus_OK:
	case http::eStatus_Accepted:
	case http::eStatus_NoContent:
		break;
	case http::webdav::eStatus_MultiStatus:
		break;
	default:
		HandleHTTPError(request.get());
		return;
	}
}

void CWebDAVCalendarClient::_RenameCalendar(const CCalendarStoreNode& node, const cdstring& node_new)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Renaming", "Error::Calendar::OSErrRenameCalendar", "Error::Calendar::NoBadRenameCalendar", node.GetName());

	// Determine URLs (new one must be absolute URL)
	cdstring rurl_old = node.GetName();
	rurl_old.EncodeURL('/');
	cdstring absurl_new = GetRURL(node_new, node.IsDirectory(), true);

	// Create WebDAV MOVE (overwrite not allowed)
	std::auto_ptr<http::webdav::CWebDAVMove> request(new http::webdav::CWebDAVMove(this, rurl_old, absurl_new, false));

	// Process it
	RunSession(request.get());
	
	// Check response status
	switch(request->GetStatusCode())
	{
	case http::eStatus_OK:
	case http::eStatus_Created:
	case http::eStatus_NoContent:
		break;
	case http::webdav::eStatus_MultiStatus:
		break;
	default:
		HandleHTTPError(request.get());
		return;
	}
}

bool CWebDAVCalendarClient::_TestCalendar(const CCalendarStoreNode& node)
{
	// Do PROPFIND Depth 0 on the uri - if we get get multistatus response then the item is OK

	// Determine URL
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');

	// Create WebDAV propfind
	xmllib::XMLNameList props;
	props.push_back(http::webdav::cProperty_getcontentlength);
	props.push_back(http::webdav::cProperty_getcontenttype);
	props.push_back(http::webdav::cProperty_resourcetype);
	std::auto_ptr<http::webdav::CWebDAVPropFind> request(new http::webdav::CWebDAVPropFind(this, rurl, http::webdav::eDepth1, props));
	http::CHTTPOutputDataString dout;
	request->SetOutput(&dout);

	// Process it
	RunSession(request.get());

	// If its a 207 we want to parse the XML
	if (request->GetStatusCode() == http::webdav::eStatus_MultiStatus)
	{
		return true;
	}
	else
		return false;
}

bool CWebDAVCalendarClient::_TouchCalendar(const CCalendarStoreNode& node)
{
	if (!_TestCalendar(node))
	{
		_CreateCalendar(node);
		return true;
	}
	else
		return false;
}

void CWebDAVCalendarClient::_LockCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Need to have capability cached before we can use locking since it may not be available
	if (!Initialised())
		RunSession(NULL);
	
	// Must support locking
	if (!HasDAVLocking())
		return;

	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Checking", "Error::Calendar::OSErrCheckCalendar", "Error::Calendar::NoBadCheckCalendar", node.GetName());

	// Determine URL
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');

	// Lock if not already locked
	LockToken(rurl, 300, false);
}

void CWebDAVCalendarClient::_UnlockCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Need to have capability cached before we can use locking since it may not be available
	if (!Initialised())
		RunSession(NULL);
	
	// Must support locking
	if (!HasDAVLocking())
		return;

	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Checking", "Error::Calendar::OSErrCheckCalendar", "Error::Calendar::NoBadCheckCalendar", node.GetName());

	// Determine URL
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');

	// See if already locked
	UnlockToken(rurl);
}

bool CWebDAVCalendarClient::_CheckCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Checking", "Error::Calendar::OSErrCheckCalendar", "Error::Calendar::NoBadCheckCalendar", node.GetName());

	bool result = false;

	// Determine URL
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');

	// Do LOCK request (5 minute timeout)
	cdstring lock_token = LockResource(rurl, 300);
	
	// Get current ETag
	cdstring etag = GetETag(rurl, lock_token);
	
	// If etags are not the same do sync operation, unless a total replace is ocurring
	if ((etag != cal.GetETag()) && (!cal.GetETag().empty()) && !cal.IsTotalReplace())
	{
		// Get current one from the server
		iCal::CICalendar cal_server;
		_ReadFullCalendar(node, cal_server);
		
		// Do sync
		iCal::CICalendarSync sync(cal, cal_server);
		sync.Sync();
		
		result = true;
	}
	
	// Now unlock
	UnlockResource(rurl, lock_token);
	
	return result;
}

bool CWebDAVCalendarClient::_CalendarChanged(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Checking", "Error::Calendar::OSErrCheckCalendar", "Error::Calendar::NoBadCheckCalendar", node.GetName());

	// Determine URL and lock
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');
	cdstring lock_token = GetLockToken(rurl);

	// Get current ETag
	cdstring etag = GetETag(rurl, lock_token);
	
	// Changed if etags are different
	return etag != cal.GetETag();
}

void CWebDAVCalendarClient::_UpdateSyncToken(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Checking", "Error::Calendar::OSErrCheckCalendar", "Error::Calendar::NoBadCheckCalendar", node.GetName());

	// Determine URL and lock
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');
	cdstring lock_token = GetLockToken(rurl);

	// Get current ETag
	cdstring etag = GetETag(rurl, lock_token);
	
	cal.SetETag(etag);
}

void CWebDAVCalendarClient::_SizeCalendar(CCalendarStoreNode& node)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Sizing", "Error::Calendar::OSErrSizeCalendar", "Error::Calendar::NoBadSizeCalendar", node.GetName());

	// Need to have capability cached before we can decide appropriate method to use
	if (!Initialised())
		RunSession(NULL);

	// Use PROPFIND if DAV supported
	if (HasDAV())
		SizeCalendar_DAV(node);
	else
		SizeCalendar_HTTP(node);
}

void CWebDAVCalendarClient::SizeCalendar_DAV(CCalendarStoreNode& node)
{
	// Create WebDAV propfind
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');
	xmllib::XMLNameList props;
	props.push_back(http::webdav::cProperty_getcontentlength);
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

		// Look at each propfind result and determine size of calendar
		for(http::webdav::CWebDAVPropFindParser::CPropFindResults::const_iterator iter = parser.Results().begin(); iter != parser.Results().end(); iter++)
		{
			// Get child element name (decode URL)
			cdstring name((*iter)->GetResource());
			name.DecodeURL();
		
			// Must match rurl
			if (name.compare_end(rurl))
			{
				if ((*iter)->GetTextProperties().count(http::webdav::cProperty_getcontentlength.FullName()) != 0)
				{
					cdstring result = (*(*iter)->GetTextProperties().find(http::webdav::cProperty_getcontentlength.FullName())).second;
					long size = ::strtoul(result.c_str(), NULL, 10);
					node.SetSize(size);
					break;
				}
			}
		}
	}
	else
	{
		HandleHTTPError(request.get());
		return;
	}
}

void CWebDAVCalendarClient::SizeCalendar_HTTP(CCalendarStoreNode& node)
{
	// Create WebDAV HEAD
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');
	std::auto_ptr<http::webdav::CWebDAVGet> request(new http::webdav::CWebDAVGet(this, rurl, true));
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

	// Grab size from response
	node.SetSize(request->GetContentLength());
}

void CWebDAVCalendarClient::_ReadFullCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal, bool if_changed)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Reading", "Error::Calendar::OSErrReadCalendar", "Error::Calendar::NoBadReadCalendar", node.GetName());

	// Determine URL
	cdstring rurl;
	if (!node.GetRemoteURL().empty() && (GetPrivateAccount() != NULL))
	{
		CURL parsed(node.GetRemoteURL());
		
		// Get account
		CINETAccount* acct = GetPrivateAccount();
		if (acct == NULL)
			return;

		// Set server address
		acct->SetServerIP(parsed.Server());
		
		// Handle authentication
		if (!parsed.User().empty())
		{
			acct->GetAuthenticatorUserPswd()->SetUID(parsed.User());
		}

		// Set TLS type
		if ((parsed.SchemeType() == CURL::eHTTP) || (parsed.SchemeType() == CURL::eWebcal))
			acct->SetTLSType(CINETAccount::eNoTLS);
		else
			acct->SetTLSType(CINETAccount::eSSL);
		
		rurl = parsed.Path();
		
		// The base is also the URL we want to access - this is important
		// as we test the base URL for OPTIONS
		mBaseRURL = rurl;
		
		// Bypass initialisation
		mInitialised = true;
		SetHost(parsed.Server());
	}
	else
	{
		rurl = node.GetName();
		rurl.EncodeURL('/');
	}

	// Create WebDAV GET
	std::auto_ptr<http::webdav::CWebDAVGet> request(new http::webdav::CWebDAVGet(this, rurl));
	if (if_changed)
		request->SetETag(cal.GetETag(), false);
	http::CHTTPOutputDataString dout;
	request->SetData(&dout);

	// Process it
	RunSession(request.get());
	
	// Check response status
	bool changed = true;
	switch(request->GetStatusCode())
	{
	case http::eStatus_OK:
		// Do default action
		break;
	case http::eStatus_NotModified:
		// Nothing more to do
		changed = false;
		break;
	default:
		// Handle error and exit here
		HandleHTTPError(request.get());
		return;
	}

	if (changed)
	{
		// Read calendar from file
		cdstring data = dout.GetData();
		std::istrstream is(data.c_str());
		cal.Clear();
		cal.Parse(is);
		
		// Update ETag
		if (request->GetNewETag() != NULL)
		{
			cdstring temp(*request->GetNewETag());

			// Handle server bug: ETag value MUST be quoted per HTTP/1.1 3.11
			if (!temp.isquoted())
				temp.quote(true);
			
			cal.SetETag(temp);
		}
		else
			cal.SetETag(cdstring::null_str);
		
		// Check read-only status
		std::auto_ptr<http::webdav::CWebDAVOptions> optrequest(new http::webdav::CWebDAVOptions(this, rurl));

		// Process it
		RunSession(optrequest.get());
		
		
		// Check response status
		switch(optrequest->GetStatusCode())
		{
		case http::eStatus_OK:
		case http::eStatus_NoContent:
			// Do default action
			break;
		case eStatus_NotImplemented:
			// Ignore failure
			return;
		default:
			// Handle error and exit here
			HandleHTTPError(optrequest.get());
			return;
		}

		// Look for PUT
		cal.SetReadOnly(!optrequest->IsAllowed(http::cRequestPUT));
	}
}

void CWebDAVCalendarClient::_WriteFullCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Determine URL
	cdstring rurl;
	if (!node.GetRemoteURL().empty() && (GetPrivateAccount() != NULL))
	{
		CURL parsed(node.GetRemoteURL());
		
		// Get account
		CINETAccount* acct = GetPrivateAccount();
		if (acct == NULL)
			return;

		// Set server address
		acct->SetServerIP(parsed.Server());
		
		// Set TLS type
		if ((parsed.SchemeType() == CURL::eHTTP) || (parsed.SchemeType() == CURL::eWebcal))
			acct->SetTLSType(CINETAccount::eNoTLS);
		else
			acct->SetTLSType(CINETAccount::eSSL);
		
		rurl = parsed.Path();
		
		// The base is also the URL we want to access - this is important
		// as we test the base URL for OPTIONS
		mBaseRURL = rurl;
	}
	else
	{
		rurl = node.GetName();
		rurl.EncodeURL('/');
	}

	// Need to have capability cached before we can decide appropriate method to use
	if (!Initialised())
		RunSession(NULL);

	// Use LOCK/HEAD(getetag)/sync/PUT/UNLOCK transaction sequence if DAV2 supported
	if (HasDAVLocking())
		WriteFullCalendar_Lock(node, cal);
	else
		WriteFullCalendar_Put(node, cal);
}

void CWebDAVCalendarClient::WriteFullCalendar_Put(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstring& lock_token)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Writing", "Error::Calendar::OSErrWriteCalendar", "Error::Calendar::NoBadWriteCalendar", node.GetName());

	// Write calendar file to stream
	std::ostrstream os;
	cal.Generate(os);
	os << std::ends;
	cdstring data;
	data.steal(os.str());

	// Determine URL
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');

	// Create WebDAV PUT
	std::auto_ptr<http::webdav::CWebDAVPut> request(new http::webdav::CWebDAVPut(this, rurl, lock_token));
	http::CHTTPInputDataString din(data, "text/calendar; charset=utf-8");
	http::CHTTPOutputDataString dout;
	
	// Use lock if present, otherwise ETag
	if (lock_token.empty())
		request->SetData(&din, &dout, cal.GetETag());
	else
		// Write existing resource
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

	// Can now remove recorded items as they have been sync'd
	cal.ClearRecording();
	cal.SetTotalReplace(false);

	// Update ETag
	if (request->GetNewETag() != NULL)
	{
		cdstring temp(*request->GetNewETag());

		// Handle server bug: ETag value MUST be quoted per HTTP/1.1 3.11
		if (!temp.isquoted())
			temp.quote(true);
		
		cal.SetETag(temp);
	}
	else
		cal.SetETag(GetETag(rurl));
}

void CWebDAVCalendarClient::WriteFullCalendar_Lock(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Writing", "Error::Calendar::OSErrWriteCalendar", "Error::Calendar::NoBadWriteCalendar", node.GetName());

	// Determine URL
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');

	// Do LOCK request (5 minute timeout)
	cdstring lock_token = LockResource(rurl, 300);
	
	// Get current ETag
	cdstring etag = GetETag(rurl, lock_token);
	
	// If etags are not the same do sync operation, unless a total replace is ocurring
	if ((etag != cal.GetETag()) && (!cal.GetETag().empty()) && !cal.IsTotalReplace())
	{
		// Get current one from the server
		iCal::CICalendar cal_server;
		_ReadFullCalendar(node, cal_server);
		
		// Do sync
		iCal::CICalendarSync sync(cal, cal_server);
		sync.Sync();
	}

	// The etag on the server MUST match the etag we have stored or the stored one is empty
	WriteFullCalendar_Put(node, cal, lock_token);
	
	// Now unlock
	UnlockResource(rurl, lock_token);
}

bool CWebDAVCalendarClient::_CanUseComponents() const
{
	// Only handles entire calendar files
	return false;
}

void CWebDAVCalendarClient::_TestFastSync(const CCalendarStoreNode& node)
{
	// Does nothing in this implementation
}

void CWebDAVCalendarClient::_FastSync(const CCalendarStoreNode& node, iCal::CICalendar& cal, cdstrmap& changed, cdstrset& removed, cdstring& synctoken)
{
	// Does nothing in this implementation
}

void CWebDAVCalendarClient::_GetComponentInfo(const CCalendarStoreNode& node, iCal::CICalendar& cal, cdstrmap& comps)
{
	// Does nothing in this implementation
}

void CWebDAVCalendarClient::_AddComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component)
{
	// Does nothing in this implementation
}

void CWebDAVCalendarClient::_ChangeComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component)
{
	// Does nothing in this implementation
}

void CWebDAVCalendarClient::_RemoveComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component)
{
	// Does nothing in this implementation
}

void CWebDAVCalendarClient::_RemoveComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstring& rurl)
{
	// Does nothing in this implementation
}

void CWebDAVCalendarClient::_ReadComponents(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstrvect& rurls)
{
	// Does nothing in this implementation
}

iCal::CICalendarComponent* CWebDAVCalendarClient::_ReadComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstring& rurl)
{
	// Does nothing in this implementation
	return NULL;
}

#pragma mark ____________________________ACLs

// Set acl on server
void CWebDAVCalendarClient::_SetACL(CCalendarStoreNode& node, CACL* acl)
{
	WriteFullACL(node);
}

// Delete acl on server
void CWebDAVCalendarClient::_DeleteACL(CCalendarStoreNode& node, CACL* acl)
{
	WriteFullACL(node);
}

// Write full set of ACLs
void CWebDAVCalendarClient::WriteFullACL(CCalendarStoreNode& node)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::ChangingACL", "Error::Calendar::OSErrChangingACL", "Error::Calendar::NoBadChangingACL", node.GetName());

	// Determine URL
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');

	// Create WebDAV ACL
	std::auto_ptr<http::webdav::CWebDAVACL> request(new http::webdav::CWebDAVACL(this, rurl, node.GetACLs()));

	// Process it
	RunSession(request.get());
	
	// Check response status
	switch(request->GetStatusCode())
	{
	case http::eStatus_OK:
	case http::eStatus_Created:
	case http::eStatus_NoContent:
		break;
	default:
		HandleHTTPError(request.get());
		return;
	}
}

// Get all acls for calendar from server
void CWebDAVCalendarClient::_GetACL(CCalendarStoreNode& node)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::GetACL", "Error::Calendar::OSErrGetACL", "Error::Calendar::NoBadGetACL", node.GetName());

	// Create WebDAV propfind
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');
	CURL base(mHostURL + rurl, true);
	xmllib::XMLNameList props;
	props.push_back(http::webdav::cProperty_supported_privilege_set);
	props.push_back(http::webdav::cProperty_acl);
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

		// Look at each propfind result and determine size of calendar
		for(http::webdav::CWebDAVPropFindParser::CPropFindResults::const_iterator iter = parser.Results().begin(); iter != parser.Results().end(); iter++)
		{
			// Get child element name (decode URL)
			CURL name((*iter)->GetResource(), true);
		
			// Must match rurl
			if (base.Equal(name) || base.EqualRelative(name))
			{
				if ((*iter)->GetNodeProperties().count(http::webdav::cProperty_acl.FullName()) != 0)
				{
					// Get the XML node for current_user_privilege_set
					const xmllib::XMLNode* xmlnode = (*(*iter)->GetNodeProperties().find(http::webdav::cProperty_acl.FullName())).second;
					ParseACL(node, xmlnode);
					break;
				}
			}
		}
	}
	else
	{
		HandleHTTPError(request.get());
		return;
	}
}

// Get allowed rights for user
void CWebDAVCalendarClient::_ListRights(CCalendarStoreNode& node, CACL* acl)
{
}

// Get current user's rights to calendar
void CWebDAVCalendarClient::_MyRights(CCalendarStoreNode& node)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::MyRights", "Error::Calendar::OSErrMyRights", "Error::Calendar::NoBadMyRights", node.GetName());

	// Create WebDAV propfind
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');
	CURL base(mHostURL + rurl, true);
	xmllib::XMLNameList props;
	props.push_back(http::webdav::cProperty_supported_privilege_set);
	props.push_back(http::webdav::cProperty_current_user_privilege_set);
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

		// Look at each propfind result and determine size of calendar
		for(http::webdav::CWebDAVPropFindParser::CPropFindResults::const_iterator iter = parser.Results().begin(); iter != parser.Results().end(); iter++)
		{
			// Get child element name (decode URL)
			CURL name((*iter)->GetResource(), true);
		
			// Must match rurl
			if (base.Equal(name) || base.EqualRelative(name))
			{
				if ((*iter)->GetNodeProperties().count(http::webdav::cProperty_current_user_privilege_set.FullName()) != 0)
				{
					// Get the XML node for current_user_privilege_set
					const xmllib::XMLNode* xmlnode = (*(*iter)->GetNodeProperties().find(http::webdav::cProperty_current_user_privilege_set.FullName())).second;
					ParseMyRights(node, xmlnode);
					break;
				}
			}
		}
	}
	else
	{
		HandleHTTPError(request.get());
		return;
	}
}

#pragma mark ____________________________Schedule

// Get Scheduling Inbox/Outbox URIs
void CWebDAVCalendarClient::_GetScheduleInboxOutbox(const CCalendarStoreNode& node, cdstring& inboxURI, cdstring& outboxURI)
{
}

// Run scheduling request
void CWebDAVCalendarClient::_Schedule(const cdstring& outboxURI,
									  const cdstring& originator,
									  const cdstrvect& recipients,
									  const iCal::CICalendar& cal,
									  iCal::CITIPScheduleResultsList& results)

{
	
}

void CWebDAVCalendarClient::_GetFreeBusyCalendars(cdstrvect& calendars)
{
	
}

void CWebDAVCalendarClient::_SetFreeBusyCalendars(const cdstrvect& calendars)
{
	
}

#pragma mark ____________________________Utils

cdstring CWebDAVCalendarClient::GetETag(const cdstring& rurl, const cdstring& lock_token)
{
	cdstring result = GetProperty(rurl, lock_token, http::webdav::cProperty_getetag);

	// Handle server bug: ETag value MUST be quoted per HTTP/1.1 3.11
	if (!result.empty() && !result.isquoted())
		result.quote(true);

	return result;
}

cdstring CWebDAVCalendarClient::GetProperty(const cdstring& rurl, const cdstring& lock_token, const xmllib::XMLName& property)
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

void CWebDAVCalendarClient::TestProperty(const cdstring& rurl, const cdstring& lock_token, const xmllib::XMLName& property, TestPropertyPP callback, void* data)
{
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
				if ((*iter)->GetNodeProperties().count(property.FullName()) != 0)
				{
					if (!callback(*(*(*iter)->GetNodeProperties().find(property.FullName())).second, data))
						break;
				}
			}
		}
	}
	else
	{
		HandleHTTPError(request.get());
	}
}

cdstrvect CWebDAVCalendarClient::GetHrefListProperty(const cdstring& rurl, const xmllib::XMLName& propname)
{
	cdstrvect results;

	// Create WebDAV propfind
	xmllib::XMLNameList props;
	props.push_back(propname);
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

		// Look at each propfind result and extract any Hrefs
		cdstring decoded_rurl = rurl;
		decoded_rurl.DecodeURL();
		for(http::webdav::CWebDAVPropFindParser::CPropFindResults::const_iterator iter1 = parser.Results().begin(); iter1 != parser.Results().end(); iter1++)
		{
			// Get child element name (decode URL)
			cdstring name((*iter1)->GetResource());
			name.DecodeURL();
		
			// Must match rurl
			if (name.compare_end(decoded_rurl))
			{
				if ((*iter1)->GetNodeProperties().count(propname.FullName()) != 0)
				{
					const xmllib::XMLNode* hrefs = (*(*iter1)->GetNodeProperties().find(propname.FullName())).second;
					for(xmllib::XMLNodeList::const_iterator iter2 = hrefs->Children().begin(); iter2 !=  hrefs->Children().end(); iter2++)
					{
						// Make sure we got an Href
						if ((*iter2)->CompareFullName(http::webdav::cElement_href))
							results.push_back((*iter2)->Data());
					}
				}
			}
		}
	}
	else
	{
		HandleHTTPError(request.get());
		return results;
	}

	return results;
}

// Do principal-match report with self on the passed in url
bool CWebDAVCalendarClient::GetProperties(const cdstring& rurl, const xmllib::XMLNameList& props, cdstrmap& results)
{
	// Create WebDAV propfind
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
		
		// Look at each principal-match result and return first one that is appropriate
		cdstring decoded_rurl = rurl;
		decoded_rurl.DecodeURL();
		for(http::webdav::CWebDAVPropFindParser::CPropFindResults::const_iterator iter1 = parser.Results().begin(); iter1 != parser.Results().end(); iter1++)
		{
			// Get child element name (decode URL)
			cdstring name((*iter1)->GetResource());
			name.DecodeURL();
			
			for(xmllib::XMLNameList::const_iterator iter2 = props.begin(); iter2 != props.end(); iter2++)
			{
				if ((*iter1)->GetNodeProperties().count((*iter2).FullName()) != 0)
				{
					const xmllib::XMLNode* hrefs = (*(*iter1)->GetNodeProperties().find((*iter2).FullName())).second;
					if (hrefs->Children().size() == 1)
					{
						// Make sure we got an Href
						if ((*hrefs->Children().begin())->CompareFullName(http::webdav::cElement_href))
							results.insert(cdstrmap::value_type((*iter2).FullName(), (*hrefs->Children().begin())->Data()));
					}
				}
			}
			
			// We'll take the first one, whatever that is
			break;
		}
	}
	else
	{
		HandleHTTPError(request.get());
		return false;
	}
	
	return true;
}

// Do principal-match report with self on the passed in url
bool CWebDAVCalendarClient::GetSelfProperties(const cdstring& rurl, const xmllib::XMLNameList& props, cdstrmap& results)
{
	// Create WebDAV principal-match
	std::auto_ptr<http::webdav::CWebDAVPrincipalMatch> request(new http::webdav::CWebDAVPrincipalMatch(this, rurl, props));
	http::CHTTPOutputDataString dout;
	request->SetOutput(&dout);

	// Process it
	RunSession(request.get());

	// If its a 207 we want to parse the XML
	if (request->GetStatusCode() == http::webdav::eStatus_MultiStatus)
	{
		http::webdav::CWebDAVPropFindParser parser;
		parser.ParseData(dout.GetData());

		// Look at each principal-match result and return first one that is appropriate
		cdstring decoded_rurl = rurl;
		decoded_rurl.DecodeURL();
		for(http::webdav::CWebDAVPropFindParser::CPropFindResults::const_iterator iter1 = parser.Results().begin(); iter1 != parser.Results().end(); iter1++)
		{
			// Get child element name (decode URL)
			cdstring name((*iter1)->GetResource());
			name.DecodeURL();
			//if ((parser.Results().size() > 1) && (name.find("/users/") == cdstring::npos))
			//	continue;
		
			for(xmllib::XMLNameList::const_iterator iter2 = props.begin(); iter2 != props.end(); iter2++)
			{
				if ((*iter1)->GetNodeProperties().count((*iter2).FullName()) != 0)
				{
					const xmllib::XMLNode* hrefs = (*(*iter1)->GetNodeProperties().find((*iter2).FullName())).second;
					if (hrefs->Children().size() == 1)
					{
						// Make sure we got an Href
						if ((*hrefs->Children().begin())->CompareFullName(http::webdav::cElement_href))
							results.insert(cdstrmap::value_type((*iter2).FullName(), (*hrefs->Children().begin())->Data()));
					}
				}
			}
			
			// We'll take the first one, whatever that is
			break;
		}
	}
	else
	{
		HandleHTTPError(request.get());
		return false;
	}
	
	return true;
}

// Do principal-match report with self on the passed in url
bool CWebDAVCalendarClient::GetSelfHrefs(const cdstring& rurl, cdstrvect& results)
{
	xmllib::XMLNameList props;
	props.push_back(http::webdav::cProperty_principal_URL);

	// Create WebDAV principal-match
	std::auto_ptr<http::webdav::CWebDAVPrincipalMatch> request(new http::webdav::CWebDAVPrincipalMatch(this, rurl, props));
	http::CHTTPOutputDataString dout;
	request->SetOutput(&dout);
	
	// Process it
	RunSession(request.get());
	
	// If its a 207 we want to parse the XML
	if (request->GetStatusCode() == http::webdav::eStatus_MultiStatus)
	{
		http::webdav::CWebDAVPropFindParser parser;
		parser.ParseData(dout.GetData());
		
		// Look at each principal-match result and return first one that is appropriate
		for(http::webdav::CWebDAVPropFindParser::CPropFindResults::const_iterator iter1 = parser.Results().begin(); iter1 != parser.Results().end(); iter1++)
		{
			// Get child element name (decode URL)
			cdstring name((*iter1)->GetResource());
			name.DecodeURL();
			
			results.push_back(name);
		}
	}
	else
	{
		HandleHTTPError(request.get());
		return false;
	}
	
	return true;
}

// Do principal-match report with self on the passed in url
bool CWebDAVCalendarClient::GetSelfPrincipalResource(const cdstring& rurl, cdstring& result)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Listing", "Error::Calendar::OSErrListCalendars", "Error::Calendar::NoBadListCalendars");
	
	cdstrvect hrefs = GetHrefListProperty(rurl, http::webdav::cProperty_current_user_principal);
	if (!hrefs.empty())
	{
		result = hrefs.front();
		return true;
	}
	
	hrefs = GetHrefListProperty(rurl, http::webdav::cProperty_principal_collection_set);
	if (hrefs.empty())
		return false;
	
	// For each principal collection find one that matches self
	for(cdstrvect::const_iterator iter = hrefs.begin(); iter != hrefs.end(); iter++)
	{
		cdstrvect results;
		if (GetSelfHrefs(*iter, results))
		{
			if (results.size() > 0)
			{
				result = results.front();
				return true;
			}
		}
	}
	
	return false;
}

cdstring CWebDAVCalendarClient::LockResource(const cdstring& rurl, unsigned long timeout, bool lock_null)
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
	
void CWebDAVCalendarClient::UnlockResource(const cdstring& rurl, const cdstring& lock_token)
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

void CWebDAVCalendarClient::OpenSession()
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

void CWebDAVCalendarClient::CloseSession()
{
	// Break connection with server
	mStream->TCPCloseConnection();

	// Write to log file
	if (mAllowLog)
	{
		mLog.LogEntry("        <-------- END HTTP CONNECTION -------->");
	}
}

void CWebDAVCalendarClient::RunSession(CHTTPRequestResponse* request)
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


void CWebDAVCalendarClient::DoSession(CHTTPRequestResponse* request)
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
					
					// Display error so user knows why the prompt occurs again - but not the first time
					// as we might have a digest re-auth.
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

void CWebDAVCalendarClient::SetServerType(unsigned long type)
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

void CWebDAVCalendarClient::SetServerDescriptor(const cdstring& txt)
{
	mGreeting = txt;
}

void CWebDAVCalendarClient::SetServerCapability(const cdstring& txt)
{
	_ProcessCapability();
	mCapability = txt;
}

CHTTPAuthorization* CWebDAVCalendarClient::GetAuthorization(bool first_time, const cdstrvect& www_authenticate)
{
	// Loop while trying to authentciate
	CAuthenticator* acct_auth = GetAccount()->GetAuthenticator().GetAuthenticator();

	// Pause busy cursor before we do the prompt
	bool paused = INETPauseAction(true);
	
	// Loop while waiting for successful login with optional user prompt
	if (CMailControl::PromptUser(acct_auth, GetAccount(), GetAccount()->IsSecure(),
									false, false, false, false, false, true, first_time))
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

bool CWebDAVCalendarClient::CheckCurrentAuthorization() const
{
	return mAuthUniqueness == GetAccount()->GetAuthenticator().GetUniqueness();
}

void CWebDAVCalendarClient::DoRequest(CHTTPRequestResponse* request)
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

void CWebDAVCalendarClient::WriteRequestData(CHTTPRequestResponse* request)
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

void CWebDAVCalendarClient::ReadResponseData(CHTTPRequestResponse* request)
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

void CWebDAVCalendarClient::ReadResponseDataLength(CHTTPRequestResponse* request, unsigned long read_length)
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

unsigned long CWebDAVCalendarClient::ReadResponseDataChunked(CHTTPRequestResponse* request)
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

void CWebDAVCalendarClient::HandleHTTPError(CHTTPRequestResponse* request)
{
	DisplayHTTPError(request);
}

void CWebDAVCalendarClient::DisplayHTTPError(CHTTPRequestResponse* request)
{
	// Fake response
	mLastResponse.code = cTagNO;
	mLastResponse.tag_msg = request->GetStatusReason();

	CINETException fake_ex(CINETException::err_NoResponse);
	INETDisplayError(fake_ex, mErrorID, mNoBadID);
}

// Return the encoded url for the node
cdstring CWebDAVCalendarClient::GetRURL(const CCalendarStoreNode* node, bool abs) const
{
	if (node->IsProtocol())
		return abs ? mBaseURL : mBaseRURL;
	else
		return GetRURL(node->GetName(), node->IsDirectory(), abs);
}

// Return the encoded url for the node
cdstring CWebDAVCalendarClient::GetRURL(const cdstring& name, bool directory, bool abs) const
{
	// Determine URL
	cdstring rurl = (abs ? mBaseURL : mBaseRURL);
	if (name.c_str()[0] == '/')
		rurl = "";
	cdstring temp(name);
	temp.EncodeURL('/');
	rurl += temp;
	if (directory)
	{
		if (rurl[(cdstring::size_type)(rurl.length() - 1)] != '/')
			rurl += "/";
	}
	else
	{
		rurl += ".ics";
	}
	
	return rurl;
}

// Get lock token for this rurl
cdstring CWebDAVCalendarClient::GetLockToken(const cdstring& rurl) const
{
	cdstrmap::const_iterator found = mLockTokens.find(rurl);
	if (found != mLockTokens.end())
		return (*found).second;
	else
		return cdstring::null_str;
}

bool CWebDAVCalendarClient::LockToken(const cdstring& rurl, unsigned long timeout, bool lock_null)
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

void CWebDAVCalendarClient::UnlockToken(const cdstring& rurl)
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

void CWebDAVCalendarClient::ParseACL(CCalendarStoreNode& node, const xmllib::XMLNode* aclnode)
{
	// Must have valid input
	if (aclnode == NULL)
		return;
	
	// Now parse each child <ace> node of the <acl> node
	for(xmllib::XMLNodeList::const_iterator iter = aclnode->Children().begin(); iter != aclnode->Children().end(); iter++)
	{
		xmllib::XMLNode* acenode = *iter;
		CCalendarACL acl;
		acl.SetRights(0);
		if (acl.ParseACE(acenode))
			node.AddACL(&acl);
	}
}

void CWebDAVCalendarClient::ParseMyRights(CCalendarStoreNode& node, const xmllib::XMLNode* rightsnode)
{
	// Parse privileges and add as my rights
	CCalendarACL acl;
	acl.SetRights(0);
	acl.ParsePrivilege(rightsnode, true);
	node.SetMyRights(acl.GetRights());
}
