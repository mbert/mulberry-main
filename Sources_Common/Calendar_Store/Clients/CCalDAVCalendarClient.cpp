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
	CCalDAVCalendarClient.cpp

	Author:			
	Description:	<describe the CCalDAVCalendarClient class here>
*/

#include "CCalDAVCalendarClient.h"

#include "CCalendarAccount.h"
#include "CCalendarProtocol.h"
#include "CCalendarStoreNode.h"
//#include "CDisplayItem.h"
#include "CGeneralException.h"
#include "CStatusWindow.h"
#include "CStreamFilter.h"
#include "CStreamUtils.h"
#include "CURL.h"

#include "CHTTPAuthorizationBasic.h"
#include "CHTTPAuthorizationDigest.h"
#include "CHTTPDataString.h"
#include "CHTTPDefinitions.h"
#include "CHTTPRequestResponse.h"
#include "CWebDAVDefinitions.h"
#include "CWebDAVDelete.h"
#include "CWebDAVGet.h"
#include "CWebDAVMakeCollection.h"
#include "CWebDAVPropFind.h"
#include "CWebDAVPut.h"
#include "CCalDAVMakeCalendar.h"
#include "CCalDAVMultigetReport.h"
#include "CCalDAVReportParser.h"
#include "CCalDAVSchedule.h"
#include "CCalDAVScheduleParser.h"

#include "CICalendar.h"
#include "CICalendarSync.h"

#include "ctrbuf.h"

#if __dest_os == __win32_os || __dest_os == __linux_os
#include "StValueChanger.h"
#endif

#include <memory>
#include <strstream>

using namespace calstore; 

// sub-collections
//#define ORACLE_FIX_2	1

// no locking
#define ORACLE_FIX_3	1

// no multiget report
#define NO_MULTIGET_REPORT	1

#define OLD_STYLE_CALENDAR_DATA	1

#define NO_PRINCIPAL_REPORT	1

CCalDAVCalendarClient::CCalDAVCalendarClient(CCalendarProtocol* owner) :
	CWebDAVCalendarClient(owner)
{
	// Init instance variables
	InitCalDAVClient();
}

CCalDAVCalendarClient::CCalDAVCalendarClient(const CCalDAVCalendarClient& copy, CCalendarProtocol* owner) :
	CWebDAVCalendarClient(copy, owner)
{
	// Stream will be copied by parent constructor

	// Init instance variables
	InitCalDAVClient();
}

CCalDAVCalendarClient::~CCalDAVCalendarClient()
{
}

void CCalDAVCalendarClient::InitCalDAVClient()
{
	mCachedInboxOutbox = false;
}

// Create duplicate, empty connection
CINETClient* CCalDAVCalendarClient::CloneConnection()
{
	// Copy construct this
	return new CCalDAVCalendarClient(*this, GetCalendarProtocol());

}

#pragma mark ____________________________Protocol

// For CalDAV we need to do this twice - once on the base URI passed in, and then,
// if we know access control is supported, a second time on the principal resource
// which we know must indicate caldav capability levels.
bool CCalDAVCalendarClient::Initialise(const cdstring& host, const cdstring& base_uri)
{
	cdstring actual_base_uri = base_uri;
	if (actual_base_uri.length() == 0)
		actual_base_uri = "/";
		
	if (CWebDAVCalendarClient::Initialise(host, actual_base_uri) && HasDAVACL())
	{
		bool no_preport = false;
#ifdef NO_PRINCIPAL_REPORT
		no_preport = (GetCalendarProtocol()->GetAccount()->GetName().find("nopreport") != cdstring::npos);
#endif
		if (no_preport)
			return true;
		else
		{
			if (GetSelfPrincipalResource(actual_base_uri, mPrincipalURI))
			{
				bool result = CWebDAVCalendarClient::Initialise(host, mPrincipalURI);
				if (result)
				{
					// Get details from principal resource
					_GetPrincipalDetails(mPrincipalURI, true);
				}
				return result;
			}
		}
	}
	
	return false;
}

void CCalDAVCalendarClient::_GetPrincipalDetails(const cdstring& puri, bool reset_home)
{
	cdstrmap result;
	xmllib::XMLNameList props;
	props.push_back(http::caldav::cProperty_calendar_home_set);
	// Only if scheduling supported
	if (HasDAVVersion(eCALDAVsched))
	{
		props.push_back(http::caldav::cProperty_scheduleinboxURL);
		props.push_back(http::caldav::cProperty_scheduleoutboxURL);
	}
	if (GetProperties(puri, props, result))
	{
		cdstrmap::const_iterator found = result.find(http::caldav::cProperty_calendar_home_set.FullName());
		if (found != result.end() and reset_home)
		{
			// May need to reset calendar-home path
			if (GetCalendarProtocol()->GetCalendarAccount()->GetBaseRURL().empty())
			{
				_Reset((*found).second);
			}
		}
		if (HasDAVVersion(eCALDAVsched))
		{
			found = result.find(http::caldav::cProperty_scheduleinboxURL.FullName());
			if (found != result.end())
				mCachedInbox = (*found).second;
			found = result.find(http::caldav::cProperty_scheduleoutboxURL.FullName());
			if (found != result.end())
				mCachedOutbox = (*found).second;
			mCachedInboxOutbox = true;
		}
		return;
	}
}

// Check version of server
void CCalDAVCalendarClient::_ProcessCapability()
{
	// Look for other capabilities
	CWebDAVCalendarClient::_ProcessCapability();
	GetCalendarProtocol()->SetHasScheduling(HasDAVVersion(eCALDAVsched));

}

void CCalDAVCalendarClient::_CreateCalendar(const CCalendarStoreNode& node)
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
		// Create CalDAV MKCALENDAR (overwrite not allowed)
		std::auto_ptr<http::caldav::CCalDAVMakeCalendar> request(new http::caldav::CCalDAVMakeCalendar(this, rurl));

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

bool CCalDAVCalendarClient::_CalendarChanged(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Checking", "Error::Calendar::OSErrCheckCalendar", "Error::Calendar::NoBadCheckCalendar", node.GetName());

	// Determine URL and lock
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');
	cdstring lock_token = GetLockToken(rurl);

	// Get current CTag
	cdstring ctag = GetProperty(rurl, lock_token, http::calendarserver::cProperty_getctag);
	
	// Changed if ctags are different
	return ctag.empty() || (ctag != cal.GetETag());
}

void CCalDAVCalendarClient::_UpdateSyncToken(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Checking", "Error::Calendar::OSErrCheckCalendar", "Error::Calendar::NoBadCheckCalendar", node.GetName());

	// Determine URL and lock
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');
	cdstring lock_token = GetLockToken(rurl);

	// Get current CTag
	cdstring ctag = GetProperty(rurl, lock_token, http::calendarserver::cProperty_getctag);
	
	cal.SetETag(ctag);
}

void CCalDAVCalendarClient::ListCalendars(CCalendarStoreNode* root, const http::webdav::CWebDAVPropFindParser& parser)
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
		if ((rpath == relBase) or (rpath == relBaseNoSlash))
			continue;
		rpath.DecodeURL();
		
		// Determine type of element
		bool is_dir = false;
		bool is_cal = false;
		bool is_inbox = false;
		bool is_outbox = false;
		if ((*iter)->GetNodeProperties().count(http::webdav::cProperty_resourcetype.FullName()) != 0)
		{
			bool is_col = false;

			// Get the XML node for resourcetype
			const xmllib::XMLNode* node = (*(*iter)->GetNodeProperties().find(http::webdav::cProperty_resourcetype.FullName())).second;
			
			// Look at each child element
			for(xmllib::XMLNodeList::const_iterator iter2 = node->Children().begin(); iter2 != node->Children().end(); iter2++)
			{
				// Look for collection
				if ((*iter2)->CompareFullName(http::webdav::cProperty_collection))
					is_col = true;
				
				// Look for calendar
				if ((*iter2)->CompareFullName(http::caldav::cProperty_caldavcalendar))
					is_cal = true;
				else if ((*iter2)->CompareFullName(http::caldav::cProperty_scheduleinbox))
				{
					is_cal = true;
					is_inbox = true;
				}
				if ((*iter2)->CompareFullName(http::caldav::cProperty_scheduleoutbox))
				{
					is_cal = true;
					is_outbox = true;
				}
				
			}
			
			// Only allow calendar if also a collection
			is_cal = is_cal && is_col;
			
			// Directory if a collection and not a calendar
			is_dir = is_col && !is_cal;
		}

		// Only add a node that is a normal collection or a calendar - ignore everything else
		// Also ignore the outbox - it clutters the display
		if ((is_dir or is_cal) and not is_outbox)
		{
			// Create the new node and add to parent
			CCalendarStoreNode* node = new CCalendarStoreNode(GetCalendarProtocol(), root, is_dir, is_inbox, is_outbox, rpath);
			root->AddChild(node);
			
			// Cannot determine the size of the calendar collection without another PROPFIND, so ignore for now
			
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
}

void CCalDAVCalendarClient::_ReadFullCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal, bool if_changed)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Reading", "Error::Calendar::OSErrReadCalendar", "Error::Calendar::NoBadReadCalendar", node.GetName());

	// Policy:
	//
	// Do PROPFIND Depth 1 to get all enclosed items
	// Extract HREFs for each item
	// Get each item found and parse into calendar one at a time whilst caching HREF
	
	// Create WebDAV propfind
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');
	xmllib::XMLNameList props;
	props.push_back(http::webdav::cProperty_getcontentlength);
	props.push_back(http::webdav::cProperty_getcontenttype);
	props.push_back(http::webdav::cProperty_resourcetype);
	props.push_back(http::webdav::cProperty_getetag);
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
		ReadCalendarComponents(node, parser, cal);
	}
	else
	{
		HandleHTTPError(request.get());
		return;
	}
}

void CCalDAVCalendarClient::ReadCalendarComponents(const CCalendarStoreNode& node, const http::webdav::CWebDAVPropFindParser& parser, iCal::CICalendar& cal)
{
	// Get all the component urls
	cdstrmap comps;
	GetCalendarComponents(node, parser, cal, comps, false);

	// Get list of hrefs
	cdstrvect hrefs;
	for(cdstrmap::const_iterator iter = comps.begin(); iter != comps.end(); iter++)
	{
		hrefs.push_back((*iter).first);
	}

	// Run the calendar-multiget report
	ReadCalendarComponents(node, hrefs, cal);
}

void CCalDAVCalendarClient::ReadCalendarComponents(const CCalendarStoreNode& node, const cdstrvect& hrefs, iCal::CICalendar& cal)
{
	bool no_report = false;
#ifdef NO_MULTIGET_REPORT
	no_report = (GetCalendarProtocol()->GetAccount()->GetName().find("noreport") != cdstring::npos);
#endif
	if (no_report)
	{
		for(cdstrvect::const_iterator iter = hrefs.begin(); iter != hrefs.end(); iter++)
		{
			// Read in the component
			ReadCalendarComponent(*iter, cal);
		}
	}
	else
	{
		// Don't bother with this if empty (actually spec requires at least one href)
		if (hrefs.empty())
			return;

		// Run the calendar-multiget report
		cdstring rurl = node.GetName();
		rurl.EncodeURL('/');

		// Create WebDAV REPORT
		bool old_style = false;
#ifdef OLD_STYLE_CALENDAR_DATA
		old_style = (GetCalendarProtocol()->GetAccount()->GetName().find("oldstyle") != cdstring::npos);
#endif
		std::auto_ptr<http::caldav::CCalDAVMultigetReport> request(new http::caldav::CCalDAVMultigetReport(this, rurl, hrefs, old_style));
		http::CHTTPOutputDataString dout;
		request->SetOutput(&dout);

		// Process it
		RunSession(request.get());

		// Check response status
		switch(request->GetStatusCode())
		{
		case http::webdav::eStatus_MultiStatus:
			// Do default action
			break;
		default:
			// Handle error and exit here
			HandleHTTPError(request.get());
			return;
		}

		http::caldav::CCalDAVReportParser parser(cal);
		parser.ParseData(dout.GetData());
	}
}

void CCalDAVCalendarClient::GetCalendarComponents(const CCalendarStoreNode& node, const http::webdav::CWebDAVPropFindParser& parser, iCal::CICalendar& cal, cdstrmap& compinfo, bool last_path)
{
	CURL base(mHostURL + node.GetName(), true);
	
	// Look at each propfind result and determine type of calendar
	for(http::webdav::CWebDAVPropFindParser::CPropFindResults::const_iterator iter = parser.Results().begin(); iter != parser.Results().end(); iter++)
	{
		// Get child element name (decode URL)
		CURL name((*iter)->GetResource(), true);
	
		// Ignore if base
		if (base.Equal(name) || base.EqualRelative(name))
			continue;
		
		// Verify that its a text/calendar type
		if ((*iter)->GetTextProperties().count(http::webdav::cProperty_getcontenttype.FullName()) != 0)
		{
			// Get content-type
			cdstring result = (*(*iter)->GetTextProperties().find(http::webdav::cProperty_getcontenttype.FullName())).second;
			
			// Strip down to actual value
			result.erase(result.find(';'));
			result.trimspace();
			
			// Now look for text/calendar
			if (result.compare("text/calendar") != 0)
				continue;
		}
		
		// Get ETag for item
		cdstring etag;
		if ((*iter)->GetTextProperties().count(http::webdav::cProperty_getetag.FullName()) != 0)
		{
			etag = (*(*iter)->GetTextProperties().find(http::webdav::cProperty_getetag.FullName())).second;
			
			// Handle server bug: ETag value MUST be quoted per HTTP/1.1 3.11
			if (!etag.isquoted())
				etag.quote(true);
		}

		// Store the component
		compinfo.insert(cdstrmap::value_type(name.ToString(last_path ? CURL::eLastPath : CURL::eRelative), etag));
	}
}

iCal::CICalendarComponent* CCalDAVCalendarClient::ReadCalendarComponent(const cdstring& rurl, iCal::CICalendar& cal)
{
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
		return NULL;
	}

	// Get last segment of RURL path
	cdstring last_path(rurl);
	if (rurl.find('/') != cdstring::npos)
		last_path.assign(rurl, rurl.rfind('/') + 1, cdstring::npos);
	
	// Update ETag
	cdstring etag;
	if (request->GetNewETag() != NULL)
	{
		etag = *request->GetNewETag();

		// Handle server bug: ETag value MUST be quoted per HTTP/1.1 3.11
		if (!etag.isquoted())
			etag.quote(true);
	}
	else
		etag = cdstring::null_str;

	// Read calendar component(s) from file
	cdstring data = dout.GetData();
	std::istrstream is(data.c_str());
	return cal.ParseComponent(is, last_path, etag);
}

void CCalDAVCalendarClient::_WriteFullCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Writing", "Error::Calendar::OSErrWriteCalendar", "Error::Calendar::NoBadWriteCalendar", node.GetName());

	// Policy:
	//
	// Write each component group to server if it has changed or is new
	// We do not attempt to delete components on the server if they are not present locally

	// No longer write timezones as those are included with each respective event/to-do etc as needed	
	//WriteCalendarComponents(node, cal, cal.GetVTimezone());

	WriteCalendarComponents(node, cal, cal.GetVEvents());
	WriteCalendarComponents(node, cal, cal.GetVToDos());
	WriteCalendarComponents(node, cal, cal.GetVJournals());
	WriteCalendarComponents(node, cal, cal.GetVFreeBusy());
}

void CCalDAVCalendarClient::WriteCalendarComponents(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponentDB& components)
{
	for(iCal::CICalendarComponentDB::const_iterator iter = components.begin(); iter != components.end(); iter++)
		WriteCalendarComponent(node, cal, *(*iter).second);
}

void CCalDAVCalendarClient::WriteCalendarComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component)
{
	// We do not write recurrence instances, only non-recurring or master instance components.
	// Writing the master will result in the instances being added automatically.
	if (!component.CanGenerateInstance())
		return;

	// Check whether it is new or existing
	if (component.GetRURL().empty())
		AddComponent(node, cal, component);
	else
		ChangeComponent(node, cal, component);
}

bool CCalDAVCalendarClient::_CanUseComponents() const
{
	// Can handle components separately
	return true;
}

void CCalDAVCalendarClient::_GetComponentInfo(const CCalendarStoreNode& node, iCal::CICalendar& cal, cdstrmap& comps)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Reading", "Error::Calendar::OSErrReadCalendar", "Error::Calendar::NoBadReadCalendar", node.GetName());

	// Policy:
	//
	// Do PROPFIND Depth 1 to get all enclosed items
	// Extract HREFs for each item
	// Get each item found and parse into calendar one at a time whilst caching HREF
	
	// Create WebDAV propfind
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');
	xmllib::XMLNameList props;
	props.push_back(http::webdav::cProperty_getcontentlength);
	props.push_back(http::webdav::cProperty_getcontenttype);
	props.push_back(http::webdav::cProperty_resourcetype);
	props.push_back(http::webdav::cProperty_getetag);
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
		GetCalendarComponents(node, parser, cal, comps, true);
	}
	else
	{
		HandleHTTPError(request.get());
		return;
	}
}

void CCalDAVCalendarClient::_AddComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Adding", "Error::Calendar::OSErrAddingToCalendar", "Error::Calendar::NoBadAddingToCalendar", node.GetName());

	// Create new resource via lock NULL
	AddComponent(node, cal, component);
}

void CCalDAVCalendarClient::_ChangeComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Changing", "Error::Calendar::OSErrChangingCalendar", "Error::Calendar::NoBadChangingCalendar", node.GetName());

	// We will assume that we always overwrite the server even if the server copy has changed
	ChangeComponent(node, cal, component);
}

void CCalDAVCalendarClient::_RemoveComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component)
{
	// Must have valid component URL
	if (component.GetRURL().empty())
		return;

	// Do removal
	_RemoveComponent(node, cal, component.GetRURL());
}

void CCalDAVCalendarClient::_RemoveComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstring& comp_rurl)
{
	// NB We really ought to check whether the item has changed before we do the delete (ETag comparison),
	// but we will ignore that for now

	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Removing", "Error::Calendar::OSErrRemoveFromCalendar", "Error::Calendar::NoBadRemoveFromCalendar", node.GetName());

	// Must have valid component URL
	if (comp_rurl.empty())
		return;

	// Determine URL
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');
	rurl += comp_rurl;

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

// Read single component from server
void CCalDAVCalendarClient::_ReadComponents(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstrvect& rurls)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Reading", "Error::Calendar::OSErrReadCalendar", "Error::Calendar::NoBadReadCalendar", node.GetName());

	// Determine URLs
	cdstrvect hrefs;
	cdstring root_url = node.GetName();
	root_url.EncodeURL('/');
	for(cdstrvect::const_iterator iter = rurls.begin(); iter != rurls.end(); iter++)
	{
		cdstring rurl = root_url;
		rurl += *iter;
		hrefs.push_back(rurl);
	}

	// Read it all in	
	return ReadCalendarComponents(node, hrefs, cal);
}


// Read single component from server
iCal::CICalendarComponent* CCalDAVCalendarClient::_ReadComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstring& comp_rurl)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Reading", "Error::Calendar::OSErrReadCalendar", "Error::Calendar::NoBadReadCalendar", node.GetName());

	// Determine URL
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');
	rurl += comp_rurl;

	// Read it in	
	return ReadCalendarComponent(rurl, cal);
}

#pragma mark ____________________________Schedule

// Get Scheduling Inbox/Outbox URIs
void CCalDAVCalendarClient::_GetScheduleInboxOutbox(const CCalendarStoreNode& node, cdstring& inboxURI, cdstring& outboxURI)
{
	// Policy:
	//
	// Assume we've authenticated at least once so our principal is known and auth sent to the server with the requests.
	//
	// Determine the principal collections set for a suitable URI (/).
	//
	// Run a principal-match report for self returning the inbox/outbox URIs.
	
	// Only if scheduling supported
	if (!HasDAVVersion(eCALDAVsched))
		return;
	
	// Use cached values
	if (mCachedInboxOutbox)
	{
		inboxURI = mCachedInbox;
		outboxURI = mCachedOutbox;
		return;
	}

	// Get <DAV:principal-collection-set> property on root URI

	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Listing", "Error::Calendar::OSErrListCalendars", "Error::Calendar::NoBadListCalendars");

	// Get calendar-home-set from principal resource
	_GetPrincipalDetails(mPrincipalURI);
	inboxURI = mCachedInbox;
	outboxURI = mCachedOutbox;
}

// Run scheduling request
void CCalDAVCalendarClient::_Schedule(const cdstring& outboxURI,
									  const cdstring& originator,
									  const cdstrvect& recipients,
									  const iCal::CICalendar& cal,
									  iCal::CITIPScheduleResultsList& results)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Scheduling", "Error::Calendar::OSErrScheduling", "Error::Calendar::NoBadScheduling", outboxURI);

	// Now POST the resource

	// Write calendar file to stream
	std::ostrstream os;
	cal.Generate(os);
	os << std::ends;
	cdstring data;
	data.steal(os.str());

	// Create CalDAV POST (Schedule)
	std::auto_ptr<http::caldav::CCalDAVSchedule> request(new http::caldav::CCalDAVSchedule(this, outboxURI, originator, recipients));
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
	case http::eStatus_NoContent:
		// Do default action
		break;
	default:
		// Handle error and exit here
		HandleHTTPError(request.get());
		return;
	}

	http::caldav::CCalDAVScheduleParser parser(results);
	parser.ParseData(dout.GetData());
}

void CCalDAVCalendarClient::_GetFreeBusyCalendars(cdstrvect& calendars)
{
	// Start UI action
	StINETClientAction _action(this, "Status::Calendar::Scheduling", "Error::Calendar::OSErrScheduling", "Error::Calendar::NoBadScheduling");
	
	// Determine URL
	cdstring rurl = mCachedInbox;
	
	calendars = GetHrefListProperty(rurl, http::caldav::cProperty_schedulefreebusyset);
}

void CCalDAVCalendarClient::_SetFreeBusyCalendars(const cdstrvect& calendars)
{
	
}

#pragma mark ____________________________Others

void CCalDAVCalendarClient::AddComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component)
{
	// Create new resource via lock NULL
	
	// Policy:
	//
	// Generate new unique URL for component
	// Get LOCK NULL on that - if it exists, regenerate unique URL
	// Once we have the lock, PUT it to create it
	// Then unlock
	
	cdstring rurl;
	cdstring lock_token;
	while(true)
	{
		// Generate unqiue RURL
		const_cast<iCal::CICalendarComponent&>(component).GenerateRURL();

		// Determine URL
		rurl = node.GetName();
		rurl.EncodeURL('/');
		rurl += component.GetRURL();

#ifndef ORACLE_FIX_3
		// Do LOCK NULL request (5 minute timeout)
		lock_token = LockResource(rurl, 300, true);

		if (lock_token != "?")
#endif
			break;
	}
	
	// Make sure we got the lock
#ifndef ORACLE_FIX_3
	if (lock_token.empty())
		return;
	
	// Add lock token to list
	mLockTokens.insert(cdstrmap::value_type(rurl, lock_token));
#endif

	// Now PUT the resource
	WriteComponent(node, cal, component, true);

	// Now unlock
#ifndef ORACLE_FIX_3
	UnlockToken(rurl);
#endif
}

void CCalDAVCalendarClient::ChangeComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component)
{
	// Determine URL
#ifndef ORACLE_FIX_3
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');
	rurl += component.GetRURL();

	// Lock it
	bool locked = LockToken(rurl, 300);
#endif

	// Now PUT the resource
	WriteComponent(node, cal, component, false);

	// Now unlock
#ifndef ORACLE_FIX_3
	if (locked)
		UnlockToken(rurl);
#endif
}

void CCalDAVCalendarClient::WriteComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component, bool new_item)
{
	// Determine URL
	cdstring rurl = node.GetName();
	rurl.EncodeURL('/');
	rurl += component.GetRURL();
	cdstring lock_token = GetLockToken(rurl);

	// Now PUT the resource

	// Write calendar file to stream
	std::ostrstream os;
	cal.GenerateOne(os, component);
	os << std::ends;
	cdstring data;
	data.steal(os.str());

	// Create WebDAV PUT
	std::auto_ptr<http::webdav::CWebDAVPut> request(new http::webdav::CWebDAVPut(this, rurl, lock_token));
	http::CHTTPInputDataString din(data, "text/calendar; charset=utf-8");
	http::CHTTPOutputDataString dout;
	
	// Use lock if present, otherwise ETag
	request->SetData(&din, &dout, new_item);

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

	// Get new location
	cdstring location = request->GetResponseHeader(cHeaderLocation);
	if (!location.empty())
	{
		// Check for "redirect" on PUT
		
		// We will assume that we only get redirected to another resource in the same collection
		cdstrvect splits;
		location.split("/", splits);
		cdstring new_path = splits.back();
		if (new_path != component.GetRURL())
		{
			const_cast<iCal::CICalendarComponent&>(component).SetRURL(new_path);
			rurl = node.GetName();
			rurl.EncodeURL('/');
			rurl += component.GetRURL();
		}
	}

	// Update ETag
	if (request->GetNewETag() != NULL)
	{
		cdstring temp(*request->GetNewETag());

		// Handle server bug: ETag value MUST be quoted per HTTP/1.1 3.11
		if (!temp.isquoted())
			temp.quote(true);
		
		const_cast<iCal::CICalendarComponent&>(component).SetETag(temp);
	}
	else
		const_cast<iCal::CICalendarComponent&>(component).SetETag(GetETag(rurl));
	
	const_cast<iCal::CICalendarComponent&>(component).SetChanged(false);
}

void CCalDAVCalendarClient::SizeCalendar_DAV(CCalendarStoreNode& node)
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

void CCalDAVCalendarClient::SizeCalendar_HTTP(CCalendarStoreNode& node)
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

// Return the encoded url for the node
cdstring CCalDAVCalendarClient::GetRURL(const CCalendarStoreNode* node, bool abs) const
{
	if (node->IsProtocol())
		return abs ? mBaseURL : mBaseRURL;
	else
		return GetRURL(node->GetName(), node->IsDirectory(), abs);
}

// Return the encoded url for the node
cdstring CCalDAVCalendarClient::GetRURL(const cdstring& name, bool directory, bool abs) const
{
	// Determine URL
	cdstring rurl = (abs ? mBaseURL : mBaseRURL);
	if (name.c_str()[0] == '/')
		rurl = "";
	cdstring temp(name);
	temp.EncodeURL('/');
	rurl += temp;

	// CalDAV calendars are collections with no special extensions and should be treated as directories
	if (rurl[rurl.length() - 1] != '/')
		rurl += "/";
	
#ifdef ORACLE_FIX_2
	// Oracle server currently uses separate collections with the calendar
	if (!directory && (mBaseURL.find("caldavlinux.oracle.com") != cdstring::npos))
		rurl += "events/";
#endif
	
	return rurl;
}
