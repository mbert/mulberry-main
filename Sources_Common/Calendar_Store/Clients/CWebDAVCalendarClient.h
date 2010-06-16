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
	CWebDAVCalendarClient.h

	Author:			
	Description:	<describe the CWebDAVCalendarClient class here>
*/

#ifndef CWebDAVCalendarClient_H
#define CWebDAVCalendarClient_H

#include "CCalendarClient.h"
#include "CWebDAVPropFindParser.h"
#include "CWebDAVSession.h"

#include "XMLNode.h"

namespace calstore {

class CWebDAVCalendarClient : public CCalendarClient, public http::webdav::CWebDAVSession
{
public:
	CWebDAVCalendarClient(CCalendarProtocol* owner);
	CWebDAVCalendarClient(const CWebDAVCalendarClient& copy, CCalendarProtocol* owner);
	virtual ~CWebDAVCalendarClient();

private:
			void	InitWebDAVClient();

public:
	virtual CINETClient*	CloneConnection();			// Create duplicate, empty connection

	virtual void	Open();									// Open account
	virtual void	Reset();								// Reset account

	virtual void	Logon();								// Logon to server
	virtual void	Logoff();								// Logoff from server
	
protected:
	cdstring		mHostURL;
	cdstring		mBaseURL;
	cdstring		mBaseRURL;
	cdstring		mPrincipalURI;
	cdstrmap		mLockTokens;
	cdstring		mAuthUniqueness;

	virtual tcp_port GetDefaultPort();						// Get default port;

	virtual void	_Reset(const cdstring& baseRURL);		// Setup paths for this base URL

	// P R O T O C O L
	virtual void	_InitCapability();					// Initialise capability flags to empty set
	virtual void	_ProcessCapability();				// Handle capability response
	virtual void	_Tickle(bool force_tickle);			// Do tickle

	// C A L E N D A R
	virtual void _ListCalendars(CCalendarStoreNode* root);
	
	virtual void _CreateCalendar(const CCalendarStoreNode& node);
	virtual void _DeleteCalendar(const CCalendarStoreNode& node);
	virtual void _RenameCalendar(const CCalendarStoreNode& node, const cdstring& node_new);
	virtual bool _TestCalendar(const CCalendarStoreNode& node);
	virtual bool _TouchCalendar(const CCalendarStoreNode& node);
	virtual void _LockCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal);
	virtual void _UnlockCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal);
	virtual bool _CheckCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal);
	virtual bool _CalendarChanged(const CCalendarStoreNode& node, iCal::CICalendar& cal);
	virtual void _UpdateSyncToken(const CCalendarStoreNode& node, iCal::CICalendar& cal);
	virtual void _SizeCalendar(CCalendarStoreNode& node);

	virtual void _ReadFullCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal, bool if_changed=false);
	virtual void _WriteFullCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal);

	virtual bool _CanUseComponents() const;
	virtual void _TestFastSync(const CCalendarStoreNode& node);
	virtual void _FastSync(const CCalendarStoreNode& node, iCal::CICalendar& cal, cdstrmap& changed, cdstrset& removed, cdstring& synctoken);
	virtual void _GetComponentInfo(const CCalendarStoreNode& node, iCal::CICalendar& cal, cdstrmap& comps);
	virtual void _AddComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component);
	virtual void _ChangeComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component);
	virtual void _RemoveComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component);
	virtual void _RemoveComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstring& rurl);
	virtual void _ReadComponents(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstrvect& rurls);
	virtual iCal::CICalendarComponent* _ReadComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstring& rurl);

	// ACL related
	virtual void	_SetACL(CCalendarStoreNode& node, CACL* acl);					// Set acl on server
	virtual void	_DeleteACL(CCalendarStoreNode& node, CACL* acl);				// Delete acl on server
	virtual void	_GetACL(CCalendarStoreNode& node);								// Get all acls for calendar from server
	virtual void	_ListRights(CCalendarStoreNode& node, CACL* acl);				// Get allowed rights for user
	virtual void	_MyRights(CCalendarStoreNode& node);							// Get current user's rights to calendar

	// Schedule related
	virtual void	_GetScheduleInboxOutbox(const CCalendarStoreNode& node, cdstring& inboxURI, cdstring& outboxURI);
	virtual void	_Schedule(const cdstring& outboxURI,
							  const cdstring& originator,
							  const cdstrvect& recipients,
							  const iCal::CICalendar& cal,
							  iCal::CITIPScheduleResultsList& results);
	virtual void	_GetFreeBusyCalendars(cdstrvect& calendars);
	virtual void	_SetFreeBusyCalendars(const cdstrvect& calendars);

protected:
	// H A N D L E  E R R O R
	virtual const char*	INETGetErrorDescriptor() const;			// Descriptor for object error context

	virtual void OpenSession();
	virtual void CloseSession();
	void RunSession(CHTTPRequestResponse* request);
	void DoSession(CHTTPRequestResponse* request);

	virtual void SetServerType(unsigned long type);
	virtual void SetServerDescriptor(const cdstring& txt);
	virtual void SetServerCapability(const cdstring& txt);

	virtual CHTTPAuthorization* GetAuthorization(bool first_time, const cdstrvect& www_authenticate);
			bool CheckCurrentAuthorization() const;
	virtual void DoRequest(CHTTPRequestResponse* request);
	virtual void WriteRequestData(CHTTPRequestResponse* request);
	virtual void ReadResponseData(CHTTPRequestResponse* request);
	virtual void ReadResponseDataLength(CHTTPRequestResponse* request, unsigned long read_length);
	virtual unsigned long ReadResponseDataChunked(CHTTPRequestResponse* request);

	virtual void HandleHTTPError(CHTTPRequestResponse* request);
	virtual void DisplayHTTPError(CHTTPRequestResponse* request);

	virtual void ListCalendars(CCalendarStoreNode* root, const http::webdav::CWebDAVPropFindParser& parser);
	
	virtual void SizeCalendar_DAV(CCalendarStoreNode& node);
	virtual void SizeCalendar_HTTP(CCalendarStoreNode& node);

	void WriteFullCalendar_Put(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstring& lock_token = cdstring::null_str);
	void WriteFullCalendar_Lock(const CCalendarStoreNode& node, iCal::CICalendar& cal);

	typedef bool (*TestPropertyPP)(const xmllib::XMLNode& node, void* data);

	cdstring GetETag(const cdstring& rurl, const cdstring& lock_token = cdstring::null_str);
	cdstring GetProperty(const cdstring& rurl, const cdstring& lock_token, const xmllib::XMLName& property);
	void TestProperty(const cdstring& rurl, const cdstring& lock_token, const xmllib::XMLName& property, TestPropertyPP callback, void* data);
	cdstrvect GetHrefListProperty(const cdstring& rurl, const xmllib::XMLName& propname);
	bool GetProperties(const cdstring& rurl, const xmllib::XMLNameList& props, cdstrmap& results);
	bool GetSelfProperties(const cdstring& rurl, const xmllib::XMLNameList& props, cdstrmap& results);
	bool GetSelfHrefs(const cdstring& rurl, cdstrvect& results);
	bool GetSelfPrincipalResource(const cdstring& rurl, cdstring& result);

	cdstring LockResource(const cdstring& rurl, unsigned long timeout, bool lock_null = false);
	void UnlockResource(const cdstring& rurl, const cdstring& lock_token);

	virtual cdstring GetRURL(const CCalendarStoreNode* node, bool abs = false) const;
	virtual cdstring GetRURL(const cdstring& name, bool directory, bool abs = false) const;
	cdstring GetLockToken(const cdstring& rurl) const;
	bool	 LockToken(const cdstring& rurl, unsigned long timeout, bool lock_null = false);
	void	 UnlockToken(const cdstring& rurl);
	
	void	WriteFullACL(CCalendarStoreNode& node);
	void	ParseACL(CCalendarStoreNode& node, const xmllib::XMLNode* aclnode);
	void	ParseMyRights(CCalendarStoreNode& node, const xmllib::XMLNode* rightsnode);
};

}	// namespace calstore

#endif	// CCalendarClientLocal_H
