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
	CCalendarStoreManager.h

	Author:			
	Description:	<describe the CCalendarStoreManager class here>
*/

#ifndef CCalendarStoreManager_H
#define CCalendarStoreManager_H

#include "CBroadcaster.h"

#include "CCalendarProtocol.h"

#include "CICalendar.h"

class CIdentity;

namespace calstore {

class CCalendarStoreNode;

class CCalendarStoreManager : public CBroadcaster
{
public:
	// Messages for broadcast
	enum
	{
		eBroadcast_NewCalendarAccount = 'cana',
		eBroadcast_InsertCalendarAccount = 'cani',
		eBroadcast_RemoveCalendarAccount = 'cara',
		eBroadcast_InsertNode = 'cain',
		eBroadcast_RemoveNode = 'carn',
		eBroadcast_DeleteNode = 'cadn',
		eBroadcast_SubscribeNode = 'casn',
		eBroadcast_UnsubscribeNode = 'caun',
		eBroadcast_ChangedNode = 'cach'
	};

	// Stack class to do activate/deactivate
	class StNodeActivate
	{
	public:
		StNodeActivate(CCalendarStoreNode* node)
		{
			mNode = node;
			if (mNode != NULL)
				sCalendarStoreManager->ActivateNode(mNode);
		}
		~StNodeActivate()
		{
			if (mNode != NULL) sCalendarStoreManager->DeactivateNode(mNode);
		}
	private:
		CCalendarStoreNode*	mNode;
	};

	static CCalendarStoreManager* sCalendarStoreManager;

	CCalendarStoreManager();
	virtual ~CCalendarStoreManager();

	void UpdateWindows();
	void SyncAccounts();	// Sync account changes

	void AddProtocol(CCalendarProtocol* proto);				// Add a new calendar remote protocol
	void RemoveProtocol(CCalendarProtocol* proto);			// Remove an calendar remote protocol
	CCalendarProtocol* GetProtocol(const cdstring& name);	// Get protocol by name
	CCalendarProtocolList& GetProtocolList()				// Get protocol list
		{ return mProtos; }
	bool HasMultipleProtocols() const
	{
		return mProtos.size() > 1;
	}
	unsigned long GetProtocolCount() const					// Number of protocols
		{ return mProtoCount; }

	void StartProtocol(CCalendarProtocol* proto, bool silent = true);	// Start protocol
	void UpdateProtocol(CCalendarProtocol* proto);						// Update protocol
	void StopProtocol(CCalendarProtocol* proto);						// Stop protocol

	long GetProtocolIndex(const CCalendarProtocol* proto) const;		// Get index of protocol
	bool FindProtocol(const CCalendarProtocol* proto, unsigned long& pos) const;

	void MoveProtocol(long old_index, long new_index);	// Move protocol

	CCalendarStoreNode& GetRoot()
	{
		return mRoot;
	}

	CCalendarStoreNodeList& GetNodes()
	{
		return *mRoot.GetChildren();
	}

	const iCal::CICalendarList& GetActiveCalendars() const
	{
		return mActiveCalendars;
	}

	const iCal::CICalendarList& GetSubscribedCalendars() const
	{
		return mSubscribedCalendars;
	}

	const iCal::CICalendarList& GetReceivableCalendars() const
	{
		return mReceivableCalendars;
	}
	
	const CCalendarStoreNode* GetNode(const iCal::CICalendar* cal) const;
	const CCalendarStoreNode* GetNode(const cdstring& cal) const;
	const CCalendarStoreNode* GetNodeByRemoteURL(const cdstring& url) const;
	int32_t	GetActiveIndex(const iCal::CICalendar* cal) const;
	uint32_t GetCalendarColour(const iCal::CICalendar* cal) const;
	uint32_t GetCalendarColour(const CCalendarStoreNode* node) const;

	const CIdentity* GetTiedIdentity(const iCal::CICalendar* cal) const;

	// Managing the store
	CCalendarStoreNode* NewCalendar(CCalendarProtocol* proto, const cdstring& name, bool directory, const cdstring& remote_url = cdstring::null_str);
	void RenameCalendar(CCalendarStoreNode* node, const cdstring& new_name);
	void DeleteCalendar(CCalendarStoreNode* node);
	void SaveAllCalendars();		
	
	// Web Calendars
	CCalendarProtocol* GetWebCalendarProtocol() const
	{
		return mWebProto;
	}
	void NewWebCalendar(const cdstring& url);
	void CreateWebAccount();

	// Managing nodes
	void SubscribeNode(CCalendarStoreNode* node, bool subs, bool no_prefs_update = false);
	void ChangeNodeColour(CCalendarStoreNode* node, uint32_t colour);
	void ActivateNode(CCalendarStoreNode* node);
	void DeactivateNode(CCalendarStoreNode* node);

	// User prompts
	iCal::CICalendar* PickCalendar(const iCal::CICalendarComponent* comp) const;

	// O F F L I N E
	void	GoOffline(bool force, bool sync, bool fast);
	void	DoOfflineSync(bool fast);
	void	GoOnline(bool sync);

	// Sleep controls
	void	Suspend();
	void	Resume();

protected:	
	CCalendarProtocol*			mLocalProto;
	CCalendarProtocol*			mWebProto;
	CCalendarProtocolList		mProtos;
	unsigned long				mProtoCount;
	CCalendarStoreNode			mRoot;
	iCal::CICalendarList		mActiveCalendars;
	iCal::CICalendarList		mReceivableCalendars;
	iCal::CICalendarList		mSubscribedCalendars;
	
	uint32_t GetColourFromIndex(uint32_t index) const;
	
};

}	// namespace calstore

#endif	// CCalendarStoreManager_H
