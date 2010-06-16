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
	CLocalCalendarClient.h

	Author:			
	Description:	<describe the CLocalCalendarClient class here>
*/

#ifndef CLocalCalendarClient_H
#define CLocalCalendarClient_H

#include "CCalendarClient.h"

class CCalendarRecord;

namespace calstore {

class CLocalCalendarClient: public CCalendarClient
{
public:
	CLocalCalendarClient(CCalendarProtocol* owner);
	CLocalCalendarClient(const CLocalCalendarClient& copy, CCalendarProtocol* owner);
	virtual ~CLocalCalendarClient();

	virtual CINETClient*	CloneConnection();			// Create duplicate, empty connection

	virtual void	SetRecorder(CCalendarRecord* recorder)
		{ mRecorder = recorder; }

	virtual void	Open();									// Start
	virtual void	Reset();								// Reset account
protected:
	virtual void	CheckCWD();								// Check CWD
public:
	virtual void	Close();								// Release TCP
	virtual void	Abort();								// Program initiated abort
	virtual void	Forceoff();								// Forced close

	virtual void	Logon();								// Logon to server
	virtual void	Logoff();								// Logoff from server
	
	virtual void	_Tickle(bool force_tickle);			// Do tickle

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
	bool				mCaching;						// Use extra cache files
	CCalendarRecord*	mRecorder;						// Recording object
	unsigned long		mRecordID;						// Recording ID
	cdstring	mCWD;		// Path to main calendar hierarchy
	
	virtual bool IsCaching() const;

	// H A N D L E  E R R O R
	virtual const char*	INETGetErrorDescriptor() const;			// Descriptor for object error context

	cdstring MapName(const CCalendarStoreNode& node) const;
	cdstring MapName(const cdstring& node, bool is_dir) const;
	cdstring MapCacheName(const CCalendarStoreNode& node) const;
	cdstring MapCacheName(const cdstring& node, bool is_dir) const;
	
	void ListCalendars(CCalendarStoreNode* root, const cdstring& path);

private:
			void	InitLocalClient();

};

}	// namespace calstore

#endif	// CCalendarClientLocal_H
