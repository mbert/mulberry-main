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
	CCalendarProtocol.h

	Author:			
	Description:	<describe the CCalendarProtocol class here>
*/

#ifndef CCalendarProtocol_H
#define CCalendarProtocol_H

#include "CINETProtocol.h"

#include "CCalendarAccount.h"
#include "CCalendarStoreNode.h"
#include "CITIPScheduleResults.h"

#include "ptrvector.h"

namespace xmllib 
{
class XMLDocument;
class XMLNode;
};

namespace iCal 
{
class CICalendar;
class CICalendarComponent;
class CICalendarPeriod;
};

class CCalendarAccount;
class CCalendarRecord;

namespace calstore {

class CCalendarClient;

class CCalendarProtocol : public CINETProtocol
{

public:
	// Flags
	enum ECalendarFlags
	{
		eHasACL					= 1L << 16,
		eACLDisabled			= 1L << 17,
		eHasScheduling			= 1L << 18,
		eHasSync				= 1L << 19,
		eDidSyncTest			= 1L << 20
	};

	// Messages for broadcast
	enum
	{
		eBroadcast_ClearList = 'cpcl',			// param = CCalendarProtocol*
		eBroadcast_RefreshList = 'cprl',		// param = CCalendarProtocol*
		eBroadcast_ClearSubList = 'cpcs',		// param = CCalendarStoreNode*
		eBroadcast_RefreshSubList = 'cprs',		// param = CCalendarStoreNode*
		eBroadcast_RefreshNode = 'cprn'			// param = CCalendarStoreNode*
	};

	CCalendarProtocol(CINETAccount* acct);
	CCalendarProtocol(const CCalendarProtocol& copy,
							bool force_local = false,
							bool force_remote = false);
	virtual ~CCalendarProtocol();

	virtual void	CreateClient();
	virtual void	CopyClient(const CINETProtocol& copy);
	virtual void	RemoveClient();

	virtual const CCalendarAccount* GetCalendarAccount() const
		{ return static_cast<const CCalendarAccount*>(GetAccount()); }
	virtual CCalendarAccount* GetCalendarAccount()
		{ return static_cast<CCalendarAccount*>(GetAccount()); }

	bool IsLocalCalendar() const
	{
		return GetAccount()->GetServerType() == CINETAccount::eLocalCalendar;
	}
	bool IsWebCalendar() const
	{
		return GetAccount()->GetServerType() == CINETAccount::eHTTPCalendar;
	}
	bool IsWebDAVCalendar() const
	{
		return GetAccount()->GetServerType() == CINETAccount::eWebDAVCalendar;
	}
	bool IsComponentCalendar() const
	{
		return GetAccount()->GetServerType() == CINETAccount::eCalDAVCalendar;
	}
	bool IsACLCalendar() const
	{
		return GetHasACL();
	}

	char GetDirDelim() const
	{
		return mDirDelim;
	}

	CCalendarStoreNode* GetStoreRoot()
	{
		return &mStoreRoot;
	}
	CCalendarStoreNode* GetNode(const cdstring& cal, bool parent = false) const;
	CCalendarStoreNode* GetNodeByRemoteURL(const cdstring& url) const;
	CCalendarStoreNode* GetParentNode(const cdstring& cal) const;

	void	AddWD(const cdstring& wd);							// Add the working directory prefix
	void	RenameWD(CCalendarStoreNode& node,
					 const cdstring& new_name);					// Rename the working directory
	void	RemoveWD(CCalendarStoreNode& node);			// Remove the working directory prefix

	// Calendar lists
	void	LoadList();											// Load calendar list from server
	void	LoadSubList(CCalendarStoreNode* node, bool deep);	// Load calendar list from server
	void	SyncList();											// Sync calendars with prefs options
	void	RefreshList();										// Refresh calendar list from server
	void	RefreshSubList(CCalendarStoreNode* node);			// Refresh calendar list from server
	void	ListChanged();										// List was changed in some way

	// Account
	virtual void SetAccount(CINETAccount* account);
	virtual void DirtyAccount();

	// Offline
			bool	IsDisconnectedCache() const
			{
				return IsDisconnected() || (mCacheClient != NULL);
			}
	virtual void	InitDisconnect();
	virtual const cdstring& GetOfflineDefaultDirectory();
	virtual void	GoOffline();
	virtual void	GoOnline();
			void	SynchroniseDisconnect(bool fast);
			bool	DoPlayback();
	virtual void	SetSynchronising();						// Set into synchronising mode
			bool	HasDisconnectedCalendars();

	virtual void	Open();								// Open connection to protocol server
	virtual void	Close();							// Close connection to protocol server
	virtual void	Logon();							// Logon to protocol server
	virtual void	Logoff();							// Logoff from protocol server

	void CreateCalendar(const CCalendarStoreNode& node);
	void TouchCalendar(const CCalendarStoreNode& node);
	bool TestCalendar(const CCalendarStoreNode& node);
	void DeleteCalendar(const CCalendarStoreNode& node);
	void RenameCalendar(const CCalendarStoreNode& node, const cdstring& node_new);
	bool CheckCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal);
	void SizeCalendar(CCalendarStoreNode& node);

	void OpenCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal);
	void SyncFromServer(const CCalendarStoreNode& node, iCal::CICalendar& cal);
	void SyncFullFromServer(const CCalendarStoreNode& node, iCal::CICalendar& cal);
	void SyncComponentsFromServer(const CCalendarStoreNode& node, iCal::CICalendar& cal);
	void SyncComponentsFromServerSlow(const CCalendarStoreNode& node, iCal::CICalendar& cal);
	void SyncComponentsFromServerFast(const CCalendarStoreNode& node, iCal::CICalendar& cal);
	void CloseCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal);
	void CopyCalendar(const CCalendarStoreNode& node, iCal::CICalendar& newcal);
	void CopyCalendarContents(const CCalendarStoreNode& node, iCal::CICalendar& newcal);

	void ReadFullCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal, bool if_changed=false);
	void WriteFullCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal);

	void SubscribeFullCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal);
	void PublishFullCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal);

	void AddComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component);
	void ChangeComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component);
	void RemoveComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component);
	void RemoveComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstring& rurl);
	void RemoveAllComponents(const CCalendarStoreNode& node, iCal::CICalendar& cal);
	void ReadComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstring& rurl);

	// Status of ACL support
	void SetHasACL(bool has_acl)
		{ mFlags.Set(eHasACL, has_acl); }
	bool GetHasACL() const
		{ return mFlags.IsSet(eHasACL); }
	void SetACLDisabled(bool disabled)
		{ mFlags.Set(eACLDisabled, disabled); }
	bool GetACLDisabled() const
		{ return mFlags.IsSet(eACLDisabled); }
	bool UsingACLs() const							// Indicates that client can use ACLs
		{ return GetHasACL() && !GetACLDisabled(); }

	// ACL commands
	void SetACL(CCalendarStoreNode& node, CACL* acl);				// Set acl on server
	void DeleteACL(CCalendarStoreNode& node, CACL* acl);			// Delete acl on server
	void GetACL(CCalendarStoreNode& node);							// Get all acls for mailbox from server
	void ListRights(CCalendarStoreNode& node, CACL* acl);			// Get allowed rights for user
	void MyRights(CCalendarStoreNode& node);						// Get current user's rights to mailbox

	// Scheduling support
	void SetHasScheduling(bool has_schedule)
		{ mFlags.Set(eHasScheduling, has_schedule); }
	bool GetHasScheduling() const
		{ return mFlags.IsSet(eHasScheduling); }
	void GetScheduleInboxOutbox(cdstring& inboxURI, cdstring& outboxURI);
	void Schedule(const cdstring& outboxURI,
				  const cdstring& originator,
				  const cdstrvect& recipients,
				  const iCal::CICalendar& cal,
				  iCal::CITIPScheduleResultsList& results);
	void GetFreeBusyCalendars(cdstrvect& calendars);
	void SetFreeBusyCalendars(const cdstrvect& calendars);

	void SetHasSync(bool has_sync)
		{ mFlags.Set(eHasSync, has_sync); mFlags.Set(eDidSyncTest, true); }
	bool GetHasSync() const
		{ return mFlags.IsSet(eHasSync); }
	bool GetDidSyncTest() const
		{ return mFlags.IsSet(eDidSyncTest); }
	
protected:
	CCalendarClient*		mClient;							// Its client
	CCalendarClient*		mCacheClient;						// Its caching client
	bool					mCacheIsPrimary;					// Use the cache client ahead of the main one	
	CCalendarStoreNode		mStoreRoot;
	char					mDirDelim;				// Directory delimiter
	bool					mSyncingList;
	bool					mListedFromCache;

	CCalendarRecord*		mRecorder;							// Recorder for mail ops

	CCalendarStoreNode* SyncCalendarNode(const cdstring& node);		// Sync node by finding it

	// Offline
			void DumpCalendars();
			bool ReadCalendars(bool only_if_current=false);
			void RecoverCalendars();
	
			void SynchroniseRemote(CCalendarStoreNode& node, bool fast);

			void ClearDisconnect(CCalendarStoreNode& node);
};

typedef ptrvector<CCalendarProtocol> CCalendarProtocolList;

}	// namespace calstore

#endif	// CCalendarProtocol_H
