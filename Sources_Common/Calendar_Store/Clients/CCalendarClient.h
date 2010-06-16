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
	CCalendarClient.h

	Author:			
	Description:	abstract base class for a calendar store 'driver'
*/

#ifndef CCalendarClient_H
#define CCalendarClient_H

#include "CINETClient.h"

#include "CCalendarProtocol.h"
#include "CITIPScheduleResults.h"

#include "cdstring.h"

namespace iCal
{
	class CICalendar;
}
namespace calstore {

class CCalendarStoreNode;

class CCalendarClient : public CINETClient
{
public:
	CCalendarClient(CCalendarProtocol* owner) :
		CINETClient(owner) {}
	CCalendarClient(const CCalendarClient& copy, CCalendarProtocol* owner)
			: CINETClient(copy, owner) {}
	virtual ~CCalendarClient() {}

	virtual void _ListCalendars(CCalendarStoreNode* root) = 0;
	
	virtual void _CreateCalendar(const CCalendarStoreNode& node) = 0;
	virtual void _DeleteCalendar(const CCalendarStoreNode& node) = 0;
	virtual void _RenameCalendar(const CCalendarStoreNode& node, const cdstring& node_new) = 0;
	virtual bool _TestCalendar(const CCalendarStoreNode& node) = 0;
	virtual bool _TouchCalendar(const CCalendarStoreNode& node) = 0;
	virtual void _LockCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal) = 0;
	virtual void _UnlockCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal) = 0;
	virtual bool _CheckCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal) = 0;
	virtual bool _CalendarChanged(const CCalendarStoreNode& node, iCal::CICalendar& cal) = 0;
	virtual void _UpdateSyncToken(const CCalendarStoreNode& node, iCal::CICalendar& cal) = 0;
	virtual void _SizeCalendar(CCalendarStoreNode& node) = 0;

	virtual void _ReadFullCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal, bool if_changed=false) = 0;
	virtual void _WriteFullCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal) = 0;

	virtual bool _CanUseComponents() const = 0;
	virtual void _TestFastSync(const CCalendarStoreNode& node) = 0;
	virtual void _FastSync(const CCalendarStoreNode& node, iCal::CICalendar& cal, cdstrmap& changed, cdstrset& removed, cdstring& synctoken) = 0;
	virtual void _GetComponentInfo(const CCalendarStoreNode& node, iCal::CICalendar& cal, cdstrmap& comps) = 0;
	virtual void _AddComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component) = 0;
	virtual void _ChangeComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component) = 0;
	virtual void _RemoveComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component) = 0;
	virtual void _RemoveComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstring& rurl) = 0;
	virtual void _ReadComponents(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstrvect& rurls) = 0;
	virtual iCal::CICalendarComponent* _ReadComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstring& rurl) = 0;

	// ACL related
	virtual void	_SetACL(CCalendarStoreNode& node, CACL* acl) = 0;				// Set acl on server
	virtual void	_DeleteACL(CCalendarStoreNode& node, CACL* acl) = 0;			// Delete acl on server
	virtual void	_GetACL(CCalendarStoreNode& node) = 0;							// Get all acls for calendar from server
	virtual void	_ListRights(CCalendarStoreNode& node, CACL* acl) = 0;			// Get allowed rights for user
	virtual void	_MyRights(CCalendarStoreNode& node) = 0;						// Get current user's rights to calendar

	// Schedule related
	virtual void	_GetScheduleInboxOutbox(const CCalendarStoreNode& node, cdstring& inboxURI, cdstring& outboxURI) = 0;
	virtual void	_Schedule(const cdstring& outboxURI,
							  const cdstring& originator,
							  const cdstrvect& recipients,
							  const iCal::CICalendar& cal,
							  iCal::CITIPScheduleResultsList& results) = 0;
	virtual void	_GetFreeBusyCalendars(cdstrvect& calendars) = 0;
	virtual void	_SetFreeBusyCalendars(const cdstrvect& calendars) = 0;

protected:
	CCalendarProtocol*	GetCalendarProtocol() const							// Return type-cast owner
		{ return static_cast<CCalendarProtocol*>(mOwner); }

};

}	// namespace calstore

#endif	// CCalendarClient_H
