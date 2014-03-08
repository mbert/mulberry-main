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


// Source for CMboxProtocol class

#define LOCAL_MBOX

#include "CMboxProtocol.h"
#include "CMboxClient.h"

#include "CActionManager.h"
#include "CConnectionManager.h"
#include "CIMAPClient.h"
#include "CINETCommon.h"
#include "CLocalClient.h"
#include "CLocalCommon.h"
#include "CMailAccount.h"
#include "CMailAccountManager.h"
#include "CMailControl.h"
#include "CMailRecord.h"
#include "CMboxRef.h"
#include "CMulberryApp.h"
#include "CPOP3Client.h"
#include "CPreferences.h"
#include "CProgress.h"
#include "CRFC822.h"
#include "CStringUtils.h"
#include "CServerView.h"
#include "C3PaneWindow.h"
#include "CTaskClasses.h"
#include "CUnicodeStdLib.h"
#include "CURL.h"

#include __stat_header
#ifndef __VCPP__
#include <unistd.h>
#endif

#include <algorithm>
#include <stdio.h>
#include <iterator>

//#include <UProfiler.h>

// __________________________________________________________________________________________________
// C L A S S __ C M B O X P R O T O C O L
// __________________________________________________________________________________________________

// Default constructor
CMboxProtocol::CMboxProtocol(CMailAccount* account)
	: CINETProtocol(account)
{
	// Must init these first
	mINBOX = NULL;
	mCloneOwner = NULL;
	mCurrent_mbox = NULL;
	mClient = NULL;

	// Do the rest
	mHierarchies.push_back(new CMboxList(this));								// Always have subscribed list
	mHierarchies.front()->SetFlag(CTreeNodeList::eHierarchic, GetMailAccount()->GetSubsHierarchic());
	mHierarchies.front()->SetFlag(CTreeNodeList::eExpanded, GetMailAccount()->GetSubsExpanded());
	mFlags.Set(eShowSubscribed, true);
	mFlags.Set(eShowUnsubscribed, true);

	// Only IMAP servers can disconnect
	SetFlags(eCanDisconnect, IsOfflineAllowed() &&
								(GetAccountType() == CINETAccount::eIMAP) &&
								GetMailAccount()->GetDisconnected());

	mRecorder = NULL;

	CreateClient();

} // CMboxProtocol::CMboxProtocol

// Copy constructor
CMboxProtocol::CMboxProtocol(const CMboxProtocol& copy, bool force_local, bool force_remote)
	: CINETProtocol(copy)
{
	// Init instance variables
	mINBOX = NULL;
	mCloneOwner = &copy;		// Do not destroy mailboxes on delete
	mAccount = NULL;			// Clones never have an account but rely on their owner's account
	mCurrent_mbox = NULL;

	mRecorder = NULL; 				// Only original proto uses this - clones do not

	// Copy client
	if (GetMailAccount()->IsIMAP() && (force_local || force_remote))
	{
		if (force_remote)
		{
			SetFlags(eIsOffline, false);
			SetFlags(eDisconnected, false);
			mClient = new CIMAPClient(this);
		}
		else
		{
			SetFlags(eIsOffline, true);
			SetFlags(eDisconnected, true);
			InitDisconnect();
			mClient = new CLocalClient(this);
			static_cast<CLocalClient*>(mClient)->SetRecorder(mRecorder);
		}
		CINETProtocol::mClient = mClient;
	}
	else
		CopyClient(copy);

} // CMboxProtocol::CMboxProtocol

// Default destructor
CMboxProtocol::~CMboxProtocol()
{
	// Delete lists
	delete mINBOX;
	mINBOX = NULL;
	for(CHierarchies::iterator iter = mHierarchies.begin(); iter != mHierarchies.end(); iter++)
		delete *iter;

	mSingletons.DeleteAll();

	mCloneOwner = NULL;
	mCurrent_mbox = NULL;

	// Delete client
	RemoveClient();

	// Delete recorder
	delete mRecorder;
	mRecorder = NULL;

} // CMboxProtocol::~CMboxProtocol

#pragma mark ____________________________WD processing

// Automate namespace descovery
void CMboxProtocol::InitNamespace()
{
	// Only if IMAP
	if (!GetMailAccount()->IsIMAP())
		return;

	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Get namespace structure from client
	SNamespace _names;
	mClient->_Namespace(&_names);

	// Now initialise
	CMailAccountManager::sMailAccountManager->InitNamespace(this, _names);
}

// Automate namespace descovery
void CMboxProtocol::AutoNamespace()
{
	// Only if IMAP
	if (!GetMailAccount()->IsIMAP())
		return;

	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Get namespace structure from client
	SNamespace _names;
	mClient->_Namespace(&_names);

	// Merge all namespaces into one
	cdstrpairvect names;
	for(int i = ePersonal; i <= ePublic; i++)
		names.insert(names.end(), _names.mItems[i].begin(), _names.mItems[i].end());

	// Now compare returned items with existing namespaces
	for(cdstrpairvect::const_iterator iter1 = names.begin(); iter1 != names.end(); iter1++)
	{
		// things to search
		cdstring name1 = (*iter1).first;
		cdstring name2 = (*iter1).first + (*iter1).second;

		// Look for matching WD
		const CDisplayItemList& wds = GetMailAccount()->GetWDs();

		// Try to sync with existing and keep lists
		bool found = false;
		for(CDisplayItemList::const_iterator iter2 = wds.begin(); !found && (iter2 != wds.end()); iter2++)
		{
			if (((*iter2).GetName() == name1) || ((*iter2).GetName() == name2))
				found = true;
		}

		// If not found manually add it to the list
		if (!found && name1.length())
		{
			if (name1[name1.length() - 1] != *(*iter1).second.c_str())
				name1 += *(*iter1).second.c_str();
			AddWD(CDisplayItem(name1), *(*iter1).second.c_str());
		}
	}
}

// Indicates WD is always flat
bool CMboxProtocol::AlwaysFlatWD() const
{
	// Local or POP3 have single hierarchy displayed flat
	return (IsOffline() && !IsDisconnected());
}

// Set the working directory prefix
void CMboxProtocol::SyncWDs()
{
	// Inform listeners of possible global change to accounts
	Broadcast_Message(eBroadcast_BeginListChange, NULL);

	const CDisplayItemList& wds = GetMailAccount()->GetWDs();

	// Try to sync with existing and keep lists
	for(CDisplayItemList::const_iterator iter = wds.begin(); iter != wds.end(); iter++)
	{
		// Create new hierarchy
		CMboxList* new_list = new CMboxList(this, (*iter).GetName());
		new_list->SetFlag(CTreeNodeList::eHierarchic, (*iter).IsHierarchic());
		new_list->SetFlag(CTreeNodeList::eExpanded, (*iter).IsExpanded());
		new_list->SetFlag(CTreeNodeList::eLoaded, false);		// Must do this because other flag changes may set it to true
		mHierarchies.push_back(new_list);

		// Broadcast addition
		Broadcast_Message(eBroadcast_NewList, new_list);
	}

	// Inform listeners of end of global change to accounts
	Broadcast_Message(eBroadcast_EndListChange, NULL);

} // CMboxProtocol::SyncWDs


// Set the working directory prefix
void CMboxProtocol::AddWD(const CDisplayItem& wd, char dir_delim)
{
	// Make sure search directories are properly terminated
	if (!dir_delim)
		dir_delim = GetMailAccount()->GetDirDelim();

	// Create new hierarchy
	CMboxList* new_list = new CMboxList(this, wd.GetName());
	new_list->SetDirDelim(GetMailAccount()->GetDirDelim());
	new_list->SetFlag(CTreeNodeList::eHierarchic, wd.IsHierarchic());
	new_list->SetFlag(CTreeNodeList::eExpanded, wd.IsExpanded());
	mHierarchies.push_back(new_list);
	GetMailAccount()->GetWDs().push_back(wd);

	// Broadcast addition
	Broadcast_Message(eBroadcast_NewList, new_list);

	// Make prefs as dirty
	CPreferences::sPrefs->mMailAccounts.SetDirty();

	// Now load it if logged in
	if (IsLoggedOn())
		LoadList();

} // CMboxProtocol::AddWD

// Rename the working directory prefix
void CMboxProtocol::RenameWD(unsigned long pos, const cdstring& new_name)
{
	// Find hierarchy
	CHierarchies::iterator found1 = mHierarchies.begin() + pos;

	// Change name in list and force refresh
	(*found1)->SetName(new_name);
	(*found1)->Refresh();

	// Change account entry
	CDisplayItemList::iterator found2 = GetMailAccount()->GetWDs().begin() + (pos - 1);
	(*found2).SetName(new_name);

	// Make prefs as dirty
	CPreferences::sPrefs->mMailAccounts.SetDirty();

	// Now load it if logged in
	if (IsLoggedOn())
		LoadList();

} // CMboxProtocol::RenameWD

// Set the working directory prefix
void CMboxProtocol::RemoveWD(unsigned long pos)
{
	// Find hierarchy
	CHierarchies::iterator found = mHierarchies.begin() + pos;

	// Broadcast removal
	Broadcast_Message(eBroadcast_RemoveList, *found);

	// Delete and remove it
	delete *found;
	mHierarchies.erase(found);
	GetMailAccount()->GetWDs().erase(GetMailAccount()->GetWDs().begin() + (pos - 1));

	// Make prefs as dirty
	CPreferences::sPrefs->mMailAccounts.SetDirty();

} // CMboxProtocol::RemoveWD

// Set the working directory prefix
void CMboxProtocol::MoveWD(unsigned long old_pos, unsigned long new_pos)
{
	// Find hierarchy
	CHierarchies::iterator found = mHierarchies.begin() + old_pos;
	CMboxList* list = *found;
	CDisplayItem wd = GetMailAccount()->GetWDs().at(old_pos - 1);

	// Broadcast removal
	Broadcast_Message(eBroadcast_RemoveList, list);

	// Remove it
	mHierarchies.erase(found);
	GetMailAccount()->GetWDs().erase(GetMailAccount()->GetWDs().begin() + (old_pos - 1));

	// Adjust count
	if (old_pos < new_pos)
		new_pos--;

	// Create new hierarchy
	mHierarchies.insert(mHierarchies.begin() + new_pos, list);
	GetMailAccount()->GetWDs().insert(GetMailAccount()->GetWDs().begin() + (new_pos - 1), wd);

	// Broadcast addition
	Broadcast_Message(eBroadcast_NewList, list);

	// Mark prefs as dirty
	CPreferences::sPrefs->mMailAccounts.SetDirty();

} // CMboxProtocol::RemoveWD

// Set flag state of list
void CMboxProtocol::SetFlagWD(CMboxList* list, CTreeNodeList::ETreeNodeListFlags flag, bool set)
{
	list->SetFlag(flag, set);

	// Determine if subscribed mbox
	unsigned long pos = GetHierarchyIndex(list);

	// Special if subscribed
	bool changed = false;
	if (pos)
	{
		// Only works for IMAP
		CDisplayItem& wd = GetMailAccount()->GetWDs().at(pos - 1);

		// Change state and mark dirty
		switch(flag)
		{
		case CTreeNodeList::eHierarchic:
			if (wd.IsHierarchic() ^ set)
			{
				wd.SetHierarchic(set);
				changed = true;
			}
			break;
		case CTreeNodeList::eExpanded:
			if (wd.IsExpanded() ^ set)
			{
				wd.SetExpanded(set);
				changed = true;
			}
			break;
		default:;
		}
	}
	else
	{
		// Change state and mark dirty
		switch(flag)
		{
		case CTreeNodeList::eHierarchic:
			if (GetMailAccount()->GetSubsHierarchic() ^ set)
			{
				GetMailAccount()->SetSubsHierarchic(set);
				changed = true;
			}
			break;
		case CTreeNodeList::eExpanded:
			if (GetMailAccount()->GetSubsExpanded() ^ set)
			{
				GetMailAccount()->SetSubsExpanded(set);
				changed = true;
			}
			break;
		default:;
		}
	}

	// Mark prefs as dirty if changed
	if (changed)
		CPreferences::sPrefs->mMailAccounts.SetDirty();
}

// Get WD that is the INBOX root
CMboxList* CMboxProtocol::GetINBOXRootWD() const
{
	if (!mINBOX)
		return NULL;

	cdstring wd_name = mINBOX->GetName();
	if (mINBOX->GetDirDelim())
		wd_name += mINBOX->GetDirDelim();

	// Look through all hierarchy lists
	for(CHierarchies::const_iterator iter = mHierarchies.begin(); iter != mHierarchies.end(); iter++)
	{
		if ((*iter)->GetName() == wd_name)
			return const_cast<CMboxList*>(*iter);
	}

	return NULL;
}

#pragma mark ____________________________Operations

void CMboxProtocol::CreateClient()
{
	RemoveClient();

	if (GetMailAccount()->IsIMAP())
	{
		if (CConnectionManager::sConnectionManager.IsConnected() || !CanDisconnect())
		{
			SetFlags(eIsOffline, false);
			SetFlags(eDisconnected, false);
			mClient = new CIMAPClient(this);
		}
		else
		{
			SetFlags(eIsOffline, true);
			SetFlags(eDisconnected, true);
			InitDisconnect();
			mClient = new CLocalClient(this);
			static_cast<CLocalClient*>(mClient)->SetRecorder(mRecorder);
		}
	}
	else if (GetMailAccount()->IsPOP3())
	{
		SetFlags(eIsOffline, true);
		SetFlags(eDisconnected, false);
		InitOffline();
		mClient = new CPOP3Client(this);
	}
	else if (GetMailAccount()->IsLocal())
	{
		SetFlags(eIsOffline, true);
		SetFlags(eDisconnected, false);
		InitOffline();
		mClient = new CLocalClient(this);
	}
	else
		mClient = NULL;

	CINETProtocol::mClient = mClient;
}

void CMboxProtocol::CopyClient(const CINETProtocol& copy)
{
	// NB This is called in the constructor and cannot rely on virtual functions
	// being called for this object!
	CMboxClient* copy_it = static_cast<const CMboxProtocol&>(copy).mClient;
	if (dynamic_cast<CIMAPClient*>(copy_it))
		mClient = new CIMAPClient(static_cast<const CIMAPClient&>(*copy_it), this);
	else if (dynamic_cast<CPOP3Client*>(copy_it))
		mClient = new CPOP3Client(static_cast<const CPOP3Client&>(*copy_it), this);
	else if (dynamic_cast<CLocalClient*>(copy_it))
		mClient = new CLocalClient(static_cast<const CLocalClient&>(*copy_it), this);
	else
		mClient = NULL;

	CINETProtocol::mClient = mClient;
}

void CMboxProtocol::RemoveClient()
{
	delete mClient;
	mClient = NULL;
	CINETProtocol::mClient = NULL;
}

// Create duplicate, empty connection
CINETProtocol* CMboxProtocol::CloneConnection()
{
	// Copy construct this
	return new CMboxProtocol(*this);

} // CMboxProtocol::CloneConnection

// Set into synchronising mode
void CMboxProtocol::SetSynchronising()
{
	// Just remove recorder
	if (mRecorder)
	{
		// Remove from local client
		CLocalClient* client = dynamic_cast<CLocalClient*>(mClient);
		if (client)
			client->SetRecorder(NULL);

		delete mRecorder;
		mRecorder = NULL;
	}
}

// Set up default mboxes etc
void CMboxProtocol::InitProtocol()
{
	// Make sure name is properly set
	SetDescriptor(GetAccount()->GetServerIP());

	// Only IMAP servers can disconnect
	SetFlags(eCanDisconnect, IsOfflineAllowed() &&
								(GetAccountType() == CINETAccount::eIMAP) &&
								GetMailAccount()->GetDisconnected());

	// Replace INBOX first
	delete mINBOX;
	mINBOX = NULL;

	// Add default INBOX to list (no WD, no add) - IMAP & POP3 only
	if (GetAccountType() != CINETAccount::eLocal)
	{
		mINBOX = new CMbox(this, cINBOX, GetMailAccount()->GetDirDelim(), NULL, false);
		mINBOX->SetWDLevel(0);
	}

	// Throw out existing
	ClearCache();

	// Redo WDs
	SyncWDs();
}

void CMboxProtocol::SetAccount(CINETAccount* account)
{
	if (!IsCloned())
	{
		// Do inherited
		CINETProtocol::SetAccount(account);
		
		// Check disconnected state
		SetFlags(eCanDisconnect, IsOfflineAllowed() &&
									(GetAccountType() == CINETAccount::eIMAP) &&
									GetMailAccount()->GetDisconnected());
	}
}

// Open connection to protocol server
void CMboxProtocol::Open()
{
	CINETProtocol::Open();
}

// Get index of hierarchy in list
long CMboxProtocol::GetHierarchyIndex(const CMboxList* list) const
{
	CHierarchies::const_iterator found = std::find(mHierarchies.begin(), mHierarchies.end(), list);
	if (found != mHierarchies.end())
		return found - mHierarchies.begin();
	else
		return -1;
}

// Clear out the cache of mboxes
void CMboxProtocol::ClearCache()
{
	// Dispose of the list elements if not cloned
	if (!IsCloned())
	{
		// Inform listeners of possible global change to accounts
		Broadcast_Message(eBroadcast_BeginListChange, NULL);

		for(CHierarchies::iterator iter = mHierarchies.begin(); iter != mHierarchies.end(); )
		{
			// Delete all contents
			(*iter)->DeleteAll();

			// Delete hierarchy if not Subscribed
			if (iter != mHierarchies.begin())
			{
				Broadcast_Message(eBroadcast_RemoveList, *iter);
				iter = mHierarchies.erase(iter);
				continue;
			}
			
			iter++;
		}

		// Inform listeners of end of global change to accounts
		Broadcast_Message(eBroadcast_EndListChange, NULL);
	}

	// Clear current items
	mCurrent_mbox = NULL;

} // CMboxProtocol::ClearCache

// Close connection to protocol server
void CMboxProtocol::Close()
{
	try
	{
		// Do inherited
		CINETProtocol::Close();

		// Clear current items
		mCurrent_mbox = NULL;

	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Clean up and throw up

		// Clear current items
		mCurrent_mbox = NULL;

		CLOG_LOGRETHROW;
		throw;
	}

} // CMboxProtocol::Close

// Logon to server
void CMboxProtocol::Logon()
{
	if (!IsOpenAllowed())
		return;

	bool was_logged_on = IsLoggedOn();

	// Do inherited
	CINETProtocol::Logon();

	// Do not reset current mbox for clone
	if (!IsCloned())
		mCurrent_mbox = NULL;

	// Do visual updates
	if (!was_logged_on)
		Broadcast_Message(eBroadcast_MailLogon, this);

	// Attempt hierarchy character descovery - not if disconnected as that will get
	// the local OS character NOT the proper server character
	if (!GetMailAccount()->GetDirDelim() && !IsDisconnected())
	{
		FindAllMbox(NULL);

		// Reset INBOX
		if (mINBOX)
			mINBOX->SetDirDelim(GetMailAccount()->GetDirDelim());
	}

	// Attempt init namespace if requested (not for clones)
	if (!IsCloned() && GetMailAccount()->GetInitNamespace())
		InitNamespace();

	// Attempt auto namespace descovery if requested (not for clones)
	else if (!IsCloned() && GetMailAccount()->GetAutoNamespace())
		AutoNamespace();

	// Look for server-side disconnected support
	if (!IsCloned() && !IsOffline() &&
		GetMailAccount()->GetDisconnected() &&
		!GetHasDisconnected())
	{
		// Force disocnnected off
		GetMailAccount()->SetDisconnected(false);
		SetFlags(eCanDisconnect, false);
	}

} // CMboxProtocol::Logon

// Logoff server
void CMboxProtocol::Logoff()
{
	if (IsLoggedOn())
	{
		// If disconnected then this is a good time to sync any
		// disconnected mailbox changes
		if (!IsCloned() && IsDisconnected())
			DumpMailboxes(true);

		// Do visual updates before actually logoff as windows etc
		// may need to get information from open mailboxes
		Broadcast_Message(eBroadcast_MailLogoff, this);

		// Do inherited
		CINETProtocol::Logoff();

		// Close all mailboxes
		if (!IsCloned())
		{
			if (mINBOX)
				mINBOX->CloseSilent();
			for(CHierarchies::iterator iter = mHierarchies.begin(); iter != mHierarchies.end(); iter++)
				(*iter)->CloseAll();
			mSingletons.CloseAll();
		}

		mCurrent_mbox = NULL;
	}

} // CMboxProtocol::Logoff

// Forced off server
void CMboxProtocol::Forceoff()
{
	// Do inherited
	CINETProtocol::Forceoff();

	// Close all mailboxes
	if (!IsCloned())
	{
		if (mINBOX)
			mINBOX->CloseSilent();
		for(CHierarchies::iterator iter = mHierarchies.begin(); iter != mHierarchies.end(); iter++)
			(*iter)->CloseAll();
		mSingletons.CloseAll();
	}

	mCurrent_mbox = NULL;

	// Do visual updates
	Broadcast_Message(eBroadcast_MailLogoff, this);

} // CINETProtocol::Forceoff

// Recover clone state
void CMboxProtocol::RecoverClone()
{
	// Do not do error recovery since we are already trying to recover
	bool recovery = GetNoRecovery();
	SetNoRecovery(true);

	try
	{
		// Just reselect the existing mailbox
		CMbox* reselect = mCurrent_mbox;
		mCurrent_mbox = NULL;
		SetCurrentMbox(reselect, false, reselect->IsExamine());
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Reset recovery
		SetNoRecovery(recovery);

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}

	// Reset recovery
	SetNoRecovery(recovery);
}

#pragma mark ____________________________Mbox List related

// Keep mbox list sorted
void CMboxProtocol::SetSorted(bool keep_sorted)
{
	for(CHierarchies::iterator iter = mHierarchies.begin(); iter != mHierarchies.end(); iter++)
 		(*iter)->SetFlag(CTreeNodeList::eSorted, keep_sorted);
}

// Sort list now
void CMboxProtocol::Sort()
{
	for(CHierarchies::iterator iter = mHierarchies.begin(); iter != mHierarchies.end(); iter++)
 		(*iter)->Sort();
}

// Load mbox list from server
void CMboxProtocol::Refresh()
{
	// Only if not cloned
	if (IsCloned())
		return;

	// Force lists to load
	long i = 0;
	for(CHierarchies::iterator iter = mHierarchies.begin(); iter != mHierarchies.end(); iter++, i++)
	{
		// Skip based on show value
		if (!ShowSubscribed() && !i)
			continue;
		if (!ShowUnsubscribed() && i)
			continue;

 		(*iter)->Refresh();
 	}

 	// Now reload
 	LoadList();
}

void CMboxProtocol::SetShowSubscribed(bool show, bool clear)
{
	// Only if changed
	if (ShowSubscribed() == show)
		return;

	mFlags.Set(eShowSubscribed, show);

	// Remove mailboxes if required
	if (!ShowSubscribed() && clear)
		mHierarchies.front()->DeleteAll();

	// Try to load - will do so if not already loaded
	else if (ShowSubscribed())
		LoadList();
}

void CMboxProtocol::SetShowUnsubscribed(bool show, bool clear)
{
	// Only if changed
	if (ShowUnsubscribed() == show)
		return;

	mFlags.Set(eShowUnsubscribed, show);

	// Remove mailboxes if required
	if (!ShowUnsubscribed() && clear)
	{
		for(CHierarchies::iterator iter = mHierarchies.begin() + 1; iter != mHierarchies.end(); iter++)
			(*iter)->DeleteAll();
	}

	// Try to load - will do so if not already loaded
	else if (ShowUnsubscribed())
		LoadList();
}

// Clear mbox list after account change
void CMboxProtocol::ClearList()
{
	// Only if not cloned
	if (IsCloned())
		return;

	// Inform listeners of possible global change to lists
	Broadcast_Message(eBroadcast_BeginListUpdate, NULL);

	// Now find the mailboxes
	long i = 0;
	for(CHierarchies::iterator iter = mHierarchies.begin(); iter != mHierarchies.end(); iter++, i++)
	{
		// Skip based on show value
		if (!ShowSubscribed() && !i)
			continue;
		if (!ShowUnsubscribed() && i)
			continue;

		// Force refresh next time through
 		(*iter)->Refresh();

		bool had_items = (*iter)->size();

		// Remove all existing mailboxes
	 	(*iter)->DeleteAll();

	 	// Have to fake subscribed node delete in order to remove subscribed hierarchy
	 	if ((iter == mHierarchies.begin()) && had_items)
	 	{
			// Tell visual rep of change
			CTreeNodeList::SBroadcastDeleteNode info;
			info.mList = *iter;
			info.mIndex = 0;
			(*iter)->Broadcast_Message(CTreeNodeList::eBroadcast_DeleteNode, &info);
	 	}
	}

	// Refresh mbox list
	SyncMbox();

	// Inform listeners of possible global change to lists
	Broadcast_Message(eBroadcast_EndListUpdate, NULL);
}

// Load mbox list from server
void CMboxProtocol::LoadList(bool deep)
{
	// Only if not cloned
	if (IsCloned())
		return;

	// Make sure list broadcast update is reset on error
	try
	{
		// Inform listeners of possible global change to lists
		Broadcast_Message(eBroadcast_BeginListUpdate, NULL);

		// Check whether connected or not
		if (!GetMailAccount()->IsIMAP() || CConnectionManager::sConnectionManager.IsConnected())
			LoadRemoteList(deep);
		else
			ReadMailboxes();

		// Special check for non-selectable INBOX - this will cause
		// an explicit list to get real flag state
		if (GetINBOX() && GetINBOX()->NoSelect())
			TestMbox(GetINBOX());
		
		// Inform listeners of possible global change to lists
		Broadcast_Message(eBroadcast_EndListUpdate, NULL);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Inform listeners of possible global change to lists
		Broadcast_Message(eBroadcast_EndListUpdate, NULL);

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}
}

// Load mbox list from server
void CMboxProtocol::LoadRemoteList(bool deep)
{
	// Now find the mailboxes
	long i = 0;
	for(CHierarchies::iterator iter = mHierarchies.begin(); iter != mHierarchies.end(); iter++, i++)
	{
		// Skip based on show value
		if (!ShowSubscribed() && !i)
			continue;
		if (!ShowUnsubscribed() && i)
			continue;

		if (!(*iter)->IsLoaded())
		{
			// Check for any open mailboxes and transfer to singleton list
			for(CMboxList::iterator iter2 = (*iter)->begin(); iter2 != (*iter)->end();)
			{
				// Check open state
				if (static_cast<CMbox*>(*iter2)->IsOpen())
				{
					// Add as singleton and remove from WD
					AddSingleton(static_cast<CMbox*>(*iter2));
					iter2 = (*iter)->erase(iter2);
					continue;
				}
				
				iter2++;
			}

			// Cache current size
			unsigned long old_size = (*iter)->size();

			// Remove all existing mailboxes
		 	(*iter)->DeleteAll();

			// Turn off sorting during big load
			(*iter)->SetFlag(CTreeNodeList::eSorted, false);

			if (i)
			{
				if ((*iter)->IsRootName((*iter)->GetDirDelim()))
				{
					// Look for roots ending with wildcards
					char c = (*iter)->GetName()[(*iter)->GetName().length() - 1];

					if (c == *cWILDCARD)
					{
						// Ends in '*' wildcard: just do full LIST
						(*iter)->SetRoot((*iter)->GetName());
						FindAllMbox(*iter);
					}
					else if (c == *cWILDCARD_NODIR)
					// Ends in '%' wildcard: do LIST and then hierarchy descovery
					{
						cdstring new_root = (*iter)->GetName();

						// Force it to go deep if required
						if (deep)
							const_cast<cdstring&>((*iter)->GetName()).c_str_mod()[(*iter)->GetName().length() - 1] = *cWILDCARD;

						// Hierarchy descovery - boo!
						(*iter)->SetRoot(new_root);
						FindAllMbox(*iter);

						// Only do subhierarchy if hierarchies actually exist and have not already been flagged
						if (!deep && (*iter)->GetDirDelim() && (*iter)->NeedsHierarchyDescovery())
						{
							new_root += (*iter)->GetDirDelim();
							new_root += cWILDCARD_NODIR;

							// Hierarchy descovery - boo!
							(*iter)->SetRoot(new_root);
							FindAllMbox(*iter);
						}
					}
					else
					// Does not end in wildcard: do full hierarchy descovery
					{
						cdstring new_root = (*iter)->GetName();
						new_root += (deep ? cWILDCARD : cWILDCARD_NODIR);

						// Hierarchy descovery - boo!
						(*iter)->SetRoot(new_root);
						FindAllMbox(*iter);

						// Only do subhierarchy if hierarchies actually exist
						if (!deep && (*iter)->GetDirDelim() && (*iter)->NeedsHierarchyDescovery())
						{
							new_root += (*iter)->GetDirDelim();
							new_root += cWILDCARD_NODIR;

							// Hierarchy descovery - boo!
							(*iter)->SetRoot(new_root);
							FindAllMbox(*iter);
						}
					}

					// Must reset to starting root
					(*iter)->SetRoot((*iter)->GetName());
				}
				// Check for '%' special
				else if (((*iter)->GetName().length() == 1) &&
							(*(*iter)->GetName().c_str() == *cWILDCARD_NODIR))
				{
					cdstring new_root = (*iter)->GetName();

					// Hierarchy descovery - boo!
					(*iter)->SetRoot(new_root);
					FindAllMbox(*iter);

					// Only do subhierarchy if hierarchies actually exist
					if ((*iter)->GetDirDelim() && (*iter)->NeedsHierarchyDescovery())
					{
						new_root += (*iter)->GetDirDelim();
						new_root += cWILDCARD_NODIR;

						// Hierarchy descovery - boo!
						(*iter)->SetRoot(new_root);
						FindAllMbox(*iter);
					}

					// Must reset to starting root
					(*iter)->SetRoot((*iter)->GetName());
				}
				else
				{
					(*iter)->SetRoot((*iter)->GetName());
					FindAllMbox(*iter);
				}
			}
			else
			{
				FindAllSubsMbox(*iter);
				unsigned long new_size = (*iter)->size();

				// Broadcast change to Subscribed to make sure its hidden/shown
				if (!old_size && new_size)
					Broadcast_Message(eBroadcast_NewList, *iter);
				else if (old_size && !new_size)
					Broadcast_Message(eBroadcast_RemoveList, *iter);
			}

			// Sort now
			(*iter)->SetFlag(CTreeNodeList::eSorted, true);

			// Adjust directories
			(*iter)->AdjustDirectories();
		}
	}

	// Refresh mbox list
	SyncMbox();
}

// Load mbox list from server
void CMboxProtocol::LoadSubList(const CMbox* mbox, bool deep)
{
	// Only if not cloned and not disconnected
	if (IsCloned() || IsDisconnected())
		return;

	// Make sure list broadcast update is reset on error
	try
	
	{
		// Inform listeners of possible global change to lists
		Broadcast_Message(eBroadcast_BeginListUpdate, NULL);

		// Check whether connected or not
		if (!GetMailAccount()->IsIMAP() || CConnectionManager::sConnectionManager.IsConnected())
			LoadRemoteSubList(mbox, deep);

		// Inform listeners of possible global change to lists
		Broadcast_Message(eBroadcast_EndListUpdate, NULL);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Inform listeners of possible global change to lists
		Broadcast_Message(eBroadcast_EndListUpdate, NULL);

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}
}

// Load mbox list from server
void CMboxProtocol::LoadRemoteSubList(const CMbox* mbox, bool deep)
{
	// Now find the mailboxes
	CMboxList* list = mbox->GetMboxList();
	if (!list)
		return;

	// Only makes sense to do subhierarchy if one exists
	if (list->GetDirDelim() && list->IsRootName(list->GetDirDelim()))
	{
		cdstring new_root = mbox->GetName() + list->GetDirDelim();	// Cannot be '\0' within this 'if (...IsRootName...)'
		if (deep)
			// Deep expansion => use '*'
			new_root += cWILDCARD;
		else
		{
			// One-level expansion => use '%.%'
			new_root += cWILDCARD_NODIR;
			new_root += list->GetDirDelim();	// Cannot be '\0' within this 'if (...IsRootName...)'
			new_root += cWILDCARD_NODIR;
		}

		// Hierarchy descovery - boo!
		list->SetRoot(new_root);
		FindAllMbox(list);

		// Must reset to starting root
		list->SetRoot(mbox->GetName() + list->GetDirDelim());	// Cannot be '\0' within this 'if (...IsRootName...)'
	}
	else
	{
		cdstring new_root = mbox->GetName();
		if (list->GetDirDelim())
			new_root += list->GetDirDelim();
		list->SetRoot(new_root);
		FindAllMbox(list);
	}

	// Refresh mbox list
	SyncMbox();
}

// Load mbox list from server
void CMboxProtocol::DeepSubList(CMboxList* list)
{
	// Only if not cloned
	if (IsCloned())
		return;

	// Look at every mailbox in the list and deep expand if required
	for(CMboxList::iterator iter = list->begin(); iter != list->end(); iter++)
	{
		CMbox* mbox = static_cast<CMbox*>(*iter);
		if (mbox->HasInferiors() && !mbox->HasExpanded())
		{
			// WARNING: list inserts done during this loop
			// iterators maybe invalidated, so must use index instead
			unsigned long saved_index = iter - list->begin();

			LoadRemoteSubList(mbox, true);
			mbox->SetFlags(NMbox::eHasExpanded);
			
			// Now restore iterator
			iter = list->begin() + saved_index;
		}
	}
}

// Load mbox list from server
void CMboxProtocol::LoadMbox(CMboxList* list, const cdstring& mbox_name)
{
	// Only if not cloned
	if (IsCloned())
		return;

	// Make sure list broadcast update is reset on error
	try
	{
		// Inform listeners of possible global change to lists
		Broadcast_Message(eBroadcast_BeginListUpdate, NULL);

		// Only makes sense to do subhierarchy if one exists
		if (list->GetDirDelim() && list->IsRootName(list->GetDirDelim()))
		{
			list->SetRoot(mbox_name);
			FindAllMbox(list);
		}

		// Refresh mbox list
		SyncMbox();

		// Inform listeners of possible global change to lists
		Broadcast_Message(eBroadcast_EndListUpdate, NULL);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Inform listeners of possible global change to lists
		Broadcast_Message(eBroadcast_EndListUpdate, NULL);

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}
}


// Sync various mbox flags
void CMboxProtocol::SyncMbox()
{
	// Look at each notifier
	for(CMailNotificationList::const_iterator iter = CPreferences::sPrefs->mMailNotification.GetValue().begin();
			iter != CPreferences::sPrefs->mMailNotification.GetValue().end(); iter++)
	{
		unsigned long index = (*iter).GetFavouriteIndex(CPreferences::sPrefs);
		SyncFlag(CPreferences::sPrefs->mFavourites.GetValue().at(index).GetItems(), NMbox::eAutoCheck);
	}
	SyncFlag(CPreferences::sPrefs->mFavourites.GetValue().at(CMailAccountManager::eFavouriteCopyTo).GetItems(), NMbox::eCopyTo);
	SyncFlag(CPreferences::sPrefs->mFavourites.GetValue().at(CMailAccountManager::eFavouriteAppendTo).GetItems(), NMbox::eAppendTo);
	SyncFlag(CPreferences::sPrefs->mFavourites.GetValue().at(CMailAccountManager::eFavouritePuntOnClose).GetItems(), NMbox::ePuntOnClose);
	SyncFlag(CPreferences::sPrefs->mFavourites.GetValue().at(CMailAccountManager::eFavouriteAutoSync).GetItems(), NMbox::eAutoSync);
	SyncFlag(CPreferences::sPrefs->mFavourites.GetValue().at(CMailAccountManager::eFavouriteSubscribed).GetItems(), NMbox::eSubscribed);
}

// Refresh list of mailboxes
void CMboxProtocol::SyncFlag(const cdstrpairvect& mboxes, NMbox::EFlags flag)
{
	// Only if not cloned
	if (IsCloned())
		return;

	// Look at all items in list
	for(cdstrpairvect::const_iterator iter1 = mboxes.begin(); iter1 != mboxes.end(); iter1++)
	{
		CMboxList mboxlist;

		// Look for wildcards
		if (::strpbrk((*iter1).first.c_str(), cWILDCARD_ALL))
		{
			const char* p = ::strchr((*iter1).first.c_str(), cMailAccountSeparator);
			if (!p)
				return;

			// Get account name and bump past delimiter
			cdstring acct_name((*iter1).first, 0, p - (*iter1).first.c_str());
			cdstring mbox_name(++p);

			// Try to match account
			if (!GetAccountName().PatternMatch(acct_name))
				continue;

			// Find list of matching mailboxes
			FindWildcard(mbox_name, mboxlist, false);
		}
		else
			// Find it in this protocol
			FindMboxAccount((*iter1).first, mboxlist, true);

		// Now set flag in all items found
		for(CMboxList::iterator iter2 = mboxlist.begin(); iter2 != mboxlist.end(); iter2++)
			static_cast<CMbox*>(*iter2)->SetFlags(flag, true);
	}
}

void CMboxProtocol::AddSingleton(CMbox* mbox)
{
	// Add to list and set flag for delete
	mSingletons.push_back(mbox);

	mbox->SetFlags(NMbox::eDeleteOnClose);
}

void CMboxProtocol::RemoveSingleton(CMbox* mbox)
{
	// Look for it in the list
	CMboxList::iterator found = std::find(mSingletons.begin(), mSingletons.end(), mbox);

	if (found != mSingletons.end())
		// Just remove it - it will delete itself
		mSingletons.erase(found);
	
	// Now delete it
	delete mbox;
}

// Do possible auto create of this mailbox
void CMboxProtocol::AutoCreate(const cdstring& mbox_name, bool subscribe)
{
	const char* p = mbox_name.c_str();

	// Check for matching accounts
	if (::strncmp(p, GetAccountName(), GetAccountName().length()) != 0)
		return;
	p += GetAccountName().length();
	if (*p != cMailAccountSeparator)
		return;
	p++;

	// Look it up in the list
	if (FindMbox(p))
		return;

	// Try to find a matching hierarchy
	for(CHierarchies::const_iterator iter = mHierarchies.begin(); iter != mHierarchies.end(); iter++)
	{
		// Look for matching root name
		if ((*iter)->IsRootName((*iter)->GetDirDelim()))
		{
			cdstring root = (*iter)->GetRootName();

			if (::strncmp(p, root.c_str(), root.length()) == 0)
			{
				CMbox* newMbox = NULL;
				try
				{
					// Create object
					newMbox = new CMbox(this, p, (*iter)->GetDirDelim(), *iter);
					newMbox = newMbox->AddMbox();
					if (!newMbox)
					{
						CLOG_LOGTHROW(CGeneralException, -1);
						throw CGeneralException(-1);
					}
					
					// Create on server
					newMbox->Create();
					
					// Subscribe if required
					if (subscribe)
						newMbox->Subscribe();
				}
				catch (...)
				{
					CLOG_LOGCATCH(...);

					// Do not allow this to stop logon processing
					if (newMbox)
						newMbox->Remove();
				}

				// Processing is done!
				break;
			}
		}
	}
}

#pragma mark ____________________________Locating items

// Recursively search cache for mbox with requested name, optionally
// returning an open if if any
//
// mbox_name: full name of mailbox to find - can be NULL
// to_open (false): try to return one that is open if there are multiple
//                  CMbox's for the same mailbox
//
// return:    the mailbox or NULL

CMbox* CMboxProtocol::FindMbox(const char* mbox_name, bool to_open) const
{
	// Search all entries
	if (!mbox_name) return NULL;

	// Try INBOX
	if (mINBOX && !::strcmpnocase(mbox_name, cINBOX))
		return mINBOX;

	// Try primary list
	CMbox* found = NULL;
	CMbox* found_open = NULL;
	for(CHierarchies::const_iterator iter = mHierarchies.begin(); !found_open && (iter != mHierarchies.end()); iter++)
	{
		// Try to find mailbox in current hierarchy
		CMbox* found_it = (*iter)->FindMbox(mbox_name);
		
		// Was one found
		if (found_it)
		{
			// If we've not found one before, remember this one
			if (!found)
				found = found_it;

			// Remember it if its open or we don't care if its openw
			if (!to_open || found_it->IsOpen())
				found_open = found_it;
		}
	}

	// Try singletons (must be open if a singleton)
	if (!found)
		found = mSingletons.FindMbox(mbox_name);

	// Return whatever it got
	return found_open ? found_open : found;
}

// Recursively search cache for mbox with requested account name
CMbox* CMboxProtocol::FindMboxAccount(const char* acct) const
{
	// Search all entries
	if (!acct) return NULL;

	// Check that URL protocols match
	cdstring proto_acct = GetAccountName();
	proto_acct += cMailAccountSeparator;
	if (::strncmp(acct, proto_acct, proto_acct.length()) != 0)
		return NULL;

	// Point past account name
	const char* mbox_name = acct + proto_acct.length();

	// Try INBOX
	if (mINBOX && !::strcmpnocase(mbox_name, cINBOX))
		return mINBOX;

	// Try lists
	CMbox* found = NULL;
	for(CHierarchies::const_iterator iter = mHierarchies.begin(); !found && (iter != mHierarchies.end()); iter++)
		found = (*iter)->FindMbox(mbox_name);

	// Try singletons (must be open if a singleton)
	if (!found)
		found = mSingletons.FindMbox(mbox_name);

	// Return whatever it got
	return found;
}

// Recursively search cache for all mboxes with requested account name
void CMboxProtocol::FindMboxAccount(const char* acct, CMboxList& list, bool no_dir) const
{
	// Search all entries
	if (!acct) return;

	// Check that account names match
	cdstring proto_acct = GetAccountName();
	proto_acct += cMailAccountSeparator;
	if (::strncmp(acct, proto_acct, proto_acct.length()) != 0)
		return;

	// Point past account name
	const char* mbox_name = acct + proto_acct.length();

	// Now get matching items
	FindMbox(mbox_name, list, no_dir);
}

// Recursively search cache for mbox with requested url
CMbox* CMboxProtocol::FindMboxURL(const char* url) const
{
	// Search all entries
	if (!url) return NULL;

	// Check that URL protocols match
	cdstring proto_url = GetURL();
	if (::strncmp(url, proto_url, proto_url.length()) != 0)
		return NULL;

	// Try primary list
	CMbox* found = NULL;
	for(CHierarchies::const_iterator iter = mHierarchies.begin(); !found && (iter != mHierarchies.end()); iter++)
		found = (*iter)->FindMboxURL(url);

	// Try singletons (must be open if a singleton)
	if (!found)
		found = mSingletons.FindMboxURL(url);

	// Return whatever it got
	return found;
}

// Return all mboxes with this name
void CMboxProtocol::FindMbox(const char* mbox_name, CMboxList& list, bool no_dir) const
{
	// Search all entries
	if (!mbox_name) return;

	// Try INBOX
	if (mINBOX && !::strcmpnocase(mbox_name, cINBOX))
		list.push_back(mINBOX);

	// Look in all hierarchies
	for(CHierarchies::const_iterator iter = mHierarchies.begin(); iter != mHierarchies.end(); iter++)
	{
		CMbox* found = (*iter)->FindMbox(mbox_name);
		if (found && !(no_dir && found->IsDirectory()))
			list.push_back(found);
	}

	// Try singletons (must be open if a singleton)
	{
		CMbox* found = mSingletons.FindMbox(mbox_name);
		if (found && !(no_dir && found->IsDirectory()))
			list.push_back(found);
	}
}

// Recursively search cache for mbox with requested name that is open
CMbox* CMboxProtocol::FindOpenMbox(const char* mbox_name) const
{
	// Search all entries
	if (!mbox_name) return NULL;

	// Try INBOX - can only be one open
	if (mINBOX && !::strcmpnocase(mbox_name, cINBOX))
		return (mINBOX->IsOpen() ? mINBOX : NULL);

	// Try lists
	CMbox* found = NULL;
	for(CHierarchies::const_iterator iter = mHierarchies.begin(); !found && (iter != mHierarchies.end()); iter++)
	{
		found = (*iter)->FindMbox(mbox_name);
		if (found && !found->IsOpen())
			found = NULL;
	}

	// Try singletons (must be open if a singleton)
	if (!found)
		found = mSingletons.FindMbox(mbox_name);

	// Return whatever it got
	return found;
}

// Return all mboxes matching pattern
void CMboxProtocol::FindWildcard(const cdstring& pattern, CMboxList& list, bool dynamic, bool no_dir)
{
	// Check whether connected or not
	// Don't do dynamic search in disconnected mode as we can't scan directories because of
	// mailbox name hashing and non-sync'd mailboxes
	if (!GetMailAccount()->IsIMAP() || CConnectionManager::sConnectionManager.IsConnected())
	{
		// Dynamic:
		if (dynamic && IsLoggedOn())
		{
			// Used to hold search results
			CMboxList temp;

			// Determine if root pattern in use
			list.SetName(pattern);
			if (!list.IsRootName(list.GetDirDelim()))
			{
				// Do dynamic list search
				FindAllMbox(&temp);
			}
			else
			{
				// Get nearest match on WD
				CMboxList* root_list = NULL;
				CMbox* root_mbox = NULL;
				if (FindRoot(list.GetRootName(), root_list, root_mbox))
				{
					if (root_list)
					{
						// Force deep LIST
						root_list->Refresh();
						LoadList(true);
					}
					else
						// Do LIST of root name
						LoadSubList(root_mbox, true);
				}
				else
				{
					// No matching WD so do list anyways
					temp.SetName(pattern);
					temp.SetRoot(pattern);
					FindAllMbox(&temp);
				}
			}

			// Add search results to protocol
			if (temp.size())
			{
				// See if mailbox already exists and use that, otherwise add to singletons
				for(CMboxList::iterator iter = temp.begin(); iter != temp.end(); iter++)
				{
					CMbox* found = FindMbox((*iter)->GetName());
					if (!found)
						// Add as a new singleton
						AddSingleton(static_cast<CMbox*>(*iter));
					else
					{
						// Replace with the one found
						delete *iter;
						*iter = found;
					}
				}
			}

			// Now fall through and do static list
		}
	}

	// Try INBOX
	if (mINBOX)
	{
		cdstring temp(cINBOX);
		if (temp.PatternDirMatch(pattern, mINBOX->GetDirDelim()))
			list.push_back(mINBOX);
	}

	// Look in all hierarchies
	for(CHierarchies::const_iterator iter = mHierarchies.begin(); iter != mHierarchies.end(); iter++)
		(*iter)->FindWildcard(pattern, list, no_dir);

	// Try singletons (must be open if a singleton)
	mSingletons.FindWildcard(pattern, list, no_dir);
}

// Find search hierarchy that matches mbox
CMboxList* CMboxProtocol::FindMatchingList(const CMbox* mbox) const
{
	// Look in all hierarchies, except subscribed
	for(CHierarchies::const_iterator iter = mHierarchies.begin() + 1; iter != mHierarchies.end(); iter++)
	{
		if ((*iter)->IsRootName((*iter)->GetDirDelim()))
		{
			cdstring root_name = (*iter)->GetRootName();
			if (::strncmp(root_name.c_str(), mbox->GetName(), root_name.length()) == 0)
				return *iter;
		}
	}

	return NULL;
}

bool CMboxProtocol::FindRoot(const cdstring& mbox_name, CMboxList*& root_list, CMbox*& root_mbox)
{
	// Scan each WD for match
	for(CHierarchies::iterator iter = mHierarchies.begin(); iter != mHierarchies.end(); iter++)
	{
		if (mbox_name == (*iter)->GetRootName())
		{
			root_list = *iter;
			return true;
		}
	}

	// Try to find mailbox
	root_mbox = FindMbox(mbox_name);
	if (root_mbox && root_mbox->GetMboxList())
		return true;

	return false;
}

// Get index of a WD
unsigned long CMboxProtocol::FetchIndexOf(const CMboxList* list) const
{
	// Search all entries - adjust for missing INBOX
	unsigned long pos = mINBOX ? 2 : 1;

	// Bump down if no Subscribed
	if (!mHierarchies.front()->size())
		pos--;

	// Look in all hierarchies
	for(CHierarchies::const_iterator iter = mHierarchies.begin(); iter != mHierarchies.end(); iter++)
	{
		if (list == *iter)
			return pos;

		// Bump past hierarchy title
		pos++;

		// Bump past list
		pos += (*iter)->size();
	}

	return 0;
}

// Return all mboxes with this name
unsigned long CMboxProtocol::FetchIndexOf(const CMbox* mbox, bool adjust_single) const
{
	// Try INBOX
	if (mINBOX && (mbox == mINBOX))
		return 1;

	// Search all other entries adjusting for missing INBOX
	unsigned long pos = mINBOX ? 1 : 0;

	// Look in all hierarchies
	bool flat_wd = (adjust_single || AlwaysFlatWD()) && (mHierarchies.size() == 2);
	bool first = true;
	for(CHierarchies::const_iterator iter = mHierarchies.begin(); iter != mHierarchies.end(); iter++)
	{
		// Subscribed NEVER present if empty
		// Bump past hierarchy title
		if ((!first || (*iter)->size()) && (first || !flat_wd))
			pos++;

		long found = (*iter)->FetchIndexOf(mbox);
		if (found >= 0)
			return pos + ++found;

		// Bump past primary list
		pos += (*iter)->size();

		first = false;
	}

	return 0;
}

// Return all mboxes with this name
void CMboxProtocol::FetchIndexOf(const CMbox* mbox, ulvector& indices, unsigned long offset, bool adjust_single) const
{
	// Find matching list of items
	CMboxList mboxlist;
	FindMbox(mbox->GetName(), mboxlist);

	// Now lookup all the positions
	for(CMboxList::const_iterator iter = mboxlist.begin(); iter != mboxlist.end(); iter++)
	{
		unsigned long index = FetchIndexOf(static_cast<const CMbox*>(*iter), adjust_single);
		if (index)
			indices.push_back(offset + index);
	}
	return;
}

// Count all items in visual hierarchy
unsigned long CMboxProtocol::CountHierarchy(bool adjust_single) const
{
	// Take missing INBOX into account
	unsigned long count = mINBOX ? 1 : 0;
	bool first = true;
	bool flat_wd = (adjust_single || AlwaysFlatWD()) && (mHierarchies.size() == 2);

	// Add totals in each hierarchy plus one for hierarchy itself
	for(CHierarchies::const_iterator iter = mHierarchies.begin(); iter != mHierarchies.end(); iter++)
	{
		// Do not do Subscribed if empty
		if (!first || (*iter)->size())
		{
			if (first || !flat_wd)
				count++;
			count += (*iter)->size();
		}

		first = false;
	}

	return count;
}

#pragma mark ____________________________Mbox releated

// Add mbox to list
CMbox* CMboxProtocol::AddMbox(CMbox* mbox)
{
	// Only if not cloned
	if (IsCloned())
		return NULL;

	// Find out where to put mbox
	if (mbox->GetMboxList())
		return static_cast<CMbox*>(mbox->GetMboxList()->AddNode(mbox));
	else
		return NULL;
}

// Create a new mbox on server
void CMboxProtocol::CreateMbox(CMbox* mbox)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Do create action
	mClient->_CreateMbox(mbox);

} // CMboxProtocol::NewMbox

// Touch mbox on server
void CMboxProtocol::TouchMbox(CMbox* mbox)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Do create action
	mClient->_TouchMbox(mbox);
	
	// If disconnected set flag
	if (IsDisconnected())
		// Now convert to local mbox
		mbox->SetFlags(NMbox::eCachedMbox);

} // CMboxProtocol::NewMbox

// Test mbox on server
bool CMboxProtocol::TestMbox(CMbox* mbox)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return false;

	// Do create action
	return mClient->_TestMbox(mbox);

} // CMboxProtocol::NewMbox

// Rebuild mbox cache
void CMboxProtocol::RebuildMbox(CMbox* mbox)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Rebuild it
	mClient->_RebuildMbox(mbox);
}

// Open mailbox on server
void CMboxProtocol::OpenMbox(CMbox* mbox, bool update, bool examine)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// See whether requested box is already in list
	unsigned long index = 0;
	if (IsCloned())
		index = 1;
	else if (!mINBOX || (mbox != mINBOX))
	{
		for(CHierarchies::iterator iter = mHierarchies.begin(); (index < 0) && (iter != mHierarchies.end()); iter++)
			index = (*iter)->FetchIndexOf(mbox);
	}
	bool new_mbox = (index == -1);

	// Disconnected touch
	if (IsDisconnected())
		TouchMbox(mbox);

	// Do open command
	mClient->_OpenMbox(mbox);

	// Force it current (this will do selecting)
	if (mCurrent_mbox == mbox)
		mCurrent_mbox = NULL;
	SetCurrentMbox(mbox, update, examine);

	// Append mailbox to list if new
	if (!IsCloned() && new_mbox)
		AddMbox(mbox);

} // CMboxProtocol::OpenMbox

// Mailbox about to be opened elsewhere
void CMboxProtocol::OpeningMbox(CMbox* mbox)
{
	// Force it not to be current
	if (mCurrent_mbox == mbox)
	{
		// Must block
		cdmutex::lock_cdmutex _lock(_mutex);

		// Verify its still active because mutex wait could have resulted in closure
		if (!IsLoggedOn())
			return;

		mClient->_Deselect(mCurrent_mbox);
		mCurrent_mbox = NULL;
	}

} // CMboxProtocol::OpeningMbox

// Close existing mailbox
void CMboxProtocol::CloseMbox(CMbox* mbox)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Do close
	mClient->_CloseMbox(mbox);

	// Make sure current does not dangle
	if (mCurrent_mbox==mbox)
		mCurrent_mbox = NULL;

} // CMboxProtocol::CloseMbox

// Set UID validity
void CMboxProtocol::SetUIDValidity(unsigned long uidv)
{
	mClient->_SetUIDValidity(uidv);
}

// Set UID next
void CMboxProtocol::SetUIDNext(unsigned long uidn)
{
	mClient->_SetUIDNext(uidn);
}

// Set Last sync time
void CMboxProtocol::SetLastSync(unsigned long sync)
{
	mClient->_SetLastSync(sync);
}

// Make mbox current
void CMboxProtocol::SetCurrentMbox(CMbox* mbox, bool update, bool examine)
{
	// Allow selection if already selected to ensure '* n Recent' checking works
	// If its already current then do nothing
	// If not examining but currently examined then must try full select to get read-write access
	if ((mCurrent_mbox == mbox) && (examine || !mbox->IsExamine()))
		return;

	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	bool is_msg_proto = (mbox->GetMsgProtocol() == this);

	try
	{
		// Mark it as current
		mCurrent_mbox = mbox;

		// Issue SELECT/EXAMINE call
		mFlags.Set(eWasUpdated, false);
		mClient->_SelectMbox(mbox, examine);
		mbox->SetFlags(NMbox::eExamine, examine);

		// No need to do this anymore as mbox updated automatically when EXISTS etc arrive
		// Get new envelopes
		if (update && !mFlags.IsSet(eWasUpdated))
			UpdateMbox(mbox);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Force selection off after error

		// Only do if this is not deleted
		if (mbox->GetMsgProtocol() || !is_msg_proto)
			mCurrent_mbox = NULL;

		CLOG_LOGRETHROW;
		throw;
	}

}

// Deselect mbox
void CMboxProtocol::ForgetMbox(CMbox* mbox)
{
	// Force it not to be current
	if (mCurrent_mbox == mbox)
	{
		// Must block
		cdmutex::lock_cdmutex _lock(_mutex);

		// Verify its still active because mutex wait could have resulted in closure
		if (!IsLoggedOn())
			return;

		// Do not do error recovery since we are no longer interested in the mailbox
		bool recovery = GetNoRecovery();
		SetNoRecovery(true);

		// Do without errors appearing on screen as the user
		// is not really interested in failures during close
		bool old_error_alert = GetNoErrorAlert();
		SetNoErrorAlert(true);

		try
		{
			mClient->_Deselect(mCurrent_mbox);
			mCurrent_mbox = NULL;
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);

			mCurrent_mbox = NULL;

			// Reset recovery
			SetNoRecovery(recovery);
		}
		SetNoErrorAlert(old_error_alert);
	}
}

// Read in new envelopes
void CMboxProtocol::UpdateMbox(CMbox* mbox)
{
	// Set flag
	mFlags.Set(eWasUpdated, true);

	// This method is currently only called in situations where the mailbox is already
	// SELECT'ed, so this call is NOT required
#if 0
	// Make it current (does not cause a check)
	SetCurrentMbox(mbox, false, mbox->IsExamine());
#endif

	// Fetch envelopes of new messages
	mbox->LoadMessages();

} // CMboxProtocol::UpdateMbox

// Check mbox for new messages
long CMboxProtocol::CheckMbox(CMbox* mbox, bool fast)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return 0;

	long num_new = 0;
	bool is_new = false;

	// Will get delta or total depending on open status
	unsigned long old_unseen = mbox->GetNumberUnseen();
	unsigned long old_recent = mbox->GetNumberRecent();
	unsigned long old_new = mbox->GetMboxRecent();

	// Issue CHECK call
	mClient->_CheckMbox(mbox, fast);

	// Process new cabinet
	ProcessCheckMbox(mbox);

	unsigned long new_unseen = mbox->GetNumberUnseen();
	unsigned long new_recent = mbox->GetNumberRecent();
	unsigned long new_new = mbox->GetMboxRecent();

	// Process differently depending on open state of mailbox (this is an IMAPism)
	if (mbox->IsOpen())
	{
		// Use user determined concept of 'new' messages
		if (CPreferences::sPrefs->mUnseenNew.GetValue() && CPreferences::sPrefs->mRecentNew.GetValue())
		{
			// Use check recent flag
			num_new = std::min(new_unseen - old_unseen, mbox->GetCheckRecent());
			mbox->SetCheckRecent(0);
		}
		else if (CPreferences::sPrefs->mUnseenNew.GetValue())
			num_new = new_unseen - old_unseen;
		else if (CPreferences::sPrefs->mRecentNew.GetValue())
		{
			// Use check recent flag
			num_new = mbox->GetCheckRecent();
			mbox->SetCheckRecent(0);
		}
	}
	else
	{
		// Use user determined concept of 'new' messages
		if (CPreferences::sPrefs->mUnseenNew.GetValue() && CPreferences::sPrefs->mRecentNew.GetValue())
			num_new = std::min(new_unseen - old_unseen, new_recent - old_recent);
		else if (CPreferences::sPrefs->mUnseenNew.GetValue())
			num_new = new_unseen - old_unseen;
		else if (CPreferences::sPrefs->mRecentNew.GetValue())
			num_new = new_recent - old_recent;
	}

	// Now find all matching mailboxes in all hierarchies on main protocol
	// (this one may be a clone if mailbox is open)
	CMboxList mboxlist;
	mbox->GetProtocol()->FindMbox(mbox->GetName(), mboxlist);

	// Set status in all lists
	for(CMboxList::iterator iter = mboxlist.begin(); iter != mboxlist.end(); iter++)
		static_cast<CMbox*>(*iter)->UpdateStatus(mbox);

	return (num_new > 0) ? num_new : 0;

} // CMboxProtocol::CheckMbox

// Process a mailbox check change
void CMboxProtocol::ProcessCheckMbox(CMbox* mbox)
{
	// Only if cabinet processing on
	if (mbox->IsNoCabinet())
		return;

	bool is_new = mbox->AnyNew();
	bool was_new = CMailAccountManager::sMailAccountManager->IsFavouriteItem(CMailAccountManager::eFavouriteNew, mbox);

	// Now update cabinet if state change
	if (was_new ^ is_new)
	{
		// Need to ensure that server window is only updated at a safe time so use
		// a sync work queue item. This will force the cabinet change to wait until
		// idle time when nothing else is using the shared resource.
		CAddMailboxToNewMessagesTask* task = new CAddMailboxToNewMessagesTask(mbox, is_new);
		task->Go();
	}
}

// Read in size
void CMboxProtocol::MailboxSize(CMbox* mbox)
{
	// Must block only if not already
	// Its possible for the size check to take a while forcing a server
	// window redraw that causes another check. We must stop recursion.
	if (_mutex.try_lock())
	{
		try
		{
			// Get mailbox size - only if its been opened
			// This might get called BEFORE the protocol is opened during startup
			if (!IsNotOpen())
				mClient->_MailboxSize(mbox);

			_mutex.release();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			_mutex.release();
			CLOG_LOGRETHROW;
			throw;
		}
	}
}

// Does server do copies?
bool CMboxProtocol::DoesMailboxSize() const
{
	return mClient->_DoesMailboxSize();
}

// Search for messages
void CMboxProtocol::SearchMbox(CMbox* mbox, const CSearchItem* spec, ulvector* results, bool uids)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Make it current (causes an update)
	// Only SELECTed mailboxes can be searched, at least until SCAN works
	// For now do an EXAMINE to prevent multi-access problems
	SetCurrentMbox(mbox, false, true);

	// Send SEARCH message to server
	mClient->_SearchMbox(spec, results, uids);

}

// Clear (EXPUNGE) mbox
void CMboxProtocol::ExpungeMbox(CMbox* mbox, bool closing)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn() || (GetCurrentMbox() != mbox))
		return;

	// Make it current (causes an update)
	//SetCurrentMbox(mbox, true, mbox->IsExamine());

	// Send EXPUNGE message to server
	bool did_close = mClient->_ExpungeMbox(closing);
	
	// Make sure current does not dangle if it was closed
	if (closing && did_close && (mCurrent_mbox == mbox))
		mCurrent_mbox = NULL;

} // CMboxProtocol::ExpungeMbox

// Delete mbox
void CMboxProtocol::DeleteMbox(CMbox* mbox, bool remove)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Need name later after object delete
	cdstring acct_name = mbox->GetAccountName();

	// Send DELETE message to server
	mClient->_DeleteMbox(mbox);

	// Remove from lists if requested
	if (remove)
	{
		// Now find all matching mailboxes in all hierarchies
		CMboxList mboxlist;
		FindMbox(mbox->GetName(), mboxlist);

		// Remove from all lists
		for(CMboxList::iterator iter = mboxlist.begin(); iter != mboxlist.end(); iter++)
			RemoveMbox(static_cast<CMbox*>(*iter));

		// Must delete any cabinet references
		CMailAccountManager::sMailAccountManager->RemovedMailbox(acct_name);
	}

} // CMboxProtocol::DeleteMbox

// Remove mbox from list
void CMboxProtocol::RemoveMbox(CMbox* mbox)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Make sure current does not dangle
	if (mCurrent_mbox == mbox)
		mCurrent_mbox = NULL;

	// Delete it from the list (IMPORTANT: this will delete the mbox object)
	if (!IsCloned())
	{
		if (mbox->GetMboxList())
			mbox->GetMboxList()->RemoveNode(mbox, true);
		else
			delete mbox;
	}

} // CMboxProtocol::DeleteMbox

// Rename mbox
void CMboxProtocol::RenameMbox(CMbox* mbox,const char* mbox_new)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Need name later after object change
	cdstring old_acct_name = mbox->GetAccountName();

	// Send RENAME message to server
	mClient->_RenameMbox(mbox, mbox_new);

	// Now find all matching mailboxes in all hierarchies
	CMboxList mboxlist;
	FindMbox(mbox->GetName(), mboxlist);

	// Rename in all lists
	for(CMboxList::iterator iter = mboxlist.begin(); iter != mboxlist.end(); iter++)
	{
		// Force list to rename it and all children
		if (static_cast<CMbox*>(*iter)->GetMboxList())
			static_cast<CMbox*>(*iter)->GetMboxList()->NodeRenamed(*iter, mbox_new);
	}

	// Must rename any cabinet references
	CMailAccountManager::sMailAccountManager->RenamedMailbox(old_acct_name, mbox->GetAccountName());

} // CMboxProtocol::RenameMbox

// Subscribe mbox
void CMboxProtocol::SubscribeMbox(CMbox* mbox, bool update)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Send SUBSCRIBE MAILBOX message to server
	mClient->_SubscribeMbox(mbox);

	// Don't go any further if not required
	if (!update)
		return;

	// Create a copy (will add to list as subscribed)
	CMbox* subs = new CMbox(*mbox);
	subs->SetMboxList(mHierarchies.at(0));
	subs->SetFlags(NMbox::eSubscribed);
	subs = AddMbox(subs);

	// NB subs may be deleted now if it was a directory and the subscribed list was in flat mode

	// Now find all matching mailboxes in all hierarchies
	CMboxList mboxlist;
	FindMbox(mbox->GetName(), mboxlist);

	// Set flag in all lists
	for(CMboxList::iterator iter = mboxlist.begin(); iter != mboxlist.end(); iter++)
		static_cast<CMbox*>(*iter)->SetFlags(NMbox::eSubscribed);

	// Sync subscribed favourite if it was added
	if (subs)
		CMailAccountManager::sMailAccountManager->ChangeFavouriteItem(mbox, CMailAccountManager::eFavouriteSubscribed, true);
}

// Unsubscribe mbox
void CMboxProtocol::UnsubscribeMbox(CMbox* mbox, bool update)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Send UNSUBSCRIBE MAILBOX message to server
	mClient->_UnsubscribeMbox(mbox);

	// Don't go any further if not required
	if (!update)
		return;

	// Sync subscribed favourite
	CMailAccountManager::sMailAccountManager->ChangeFavouriteItem(mbox, CMailAccountManager::eFavouriteSubscribed, false);

	// Now find all matching mailboxes in all hierarchies
	CMboxList mboxlist;
	FindMbox(mbox->GetName(), mboxlist);

	// Set flag in all lists
	for(CMboxList::iterator iter = mboxlist.begin(); iter != mboxlist.end(); iter++)
		static_cast<CMbox*>(*iter)->SetFlags(NMbox::eSubscribed, false);

	// First matching mailbox must be in subscribed but check anyway
	if (mboxlist.size() && (static_cast<CMbox*>(mboxlist.front())->GetMboxList()->GetHierarchyIndex() == 0))
	{
		CMbox* old_subs = static_cast<CMbox*>(mboxlist.front());

		// Check if currently open
		bool is_open = old_subs->IsOpen();
		if (is_open)
			// Transfer to singleton list and mark for delete on close
			AddSingleton(old_subs);

		// Just delete it from subscribed list (do not do mbox delete if singleton)
		mHierarchies.at(0)->RemoveNode(old_subs, !is_open);
	}

} // CMboxProtocol::UnsubscribeMbox

// Find all subscribed mboxes
void CMboxProtocol::FindAllSubsMbox(CMboxList* mboxes)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Send FIND MAILBOX * message to server
	mClient->_FindAllSubsMbox(mboxes);

} // CMboxProtocol::FindAllSubsMbox

// Find all unsubscribed mboxes
void CMboxProtocol::FindAllMbox(CMboxList* mboxes)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Send FIND ALL.MAILBOX * message to server
	mClient->_FindAllMbox(mboxes);

} // CMboxProtocol::FindAllMbox

// Append to mbox
void CMboxProtocol::StartAppend(CMbox* mbox)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Disconnected touch
	if (IsDisconnected())
		TouchMbox(mbox);

	mClient->_StartAppend(mbox);
}

// Append to mbox
void CMboxProtocol::StopAppend(CMbox* mbox)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	mClient->_StopAppend(mbox);
}

// Append to mbox
void CMboxProtocol::AppendMbox(CMbox* mbox, CMessage* theMsg, unsigned long& new_uid, bool dummy_files, bool doMRU)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// If mbox is currently selected make sure it has read-write access
	if ((mbox == mCurrent_mbox) && mbox->IsReadOnly())
		SetCurrentMbox(mbox, false, mbox->IsExamine());

	// Disconnected touch
	if (IsDisconnected())
		TouchMbox(mbox);

	// Send APPEND message to server
	mClient->_AppendMbox(mbox, theMsg, new_uid, dummy_files);

	// Add to MRU list
	if (doMRU)
	{
		if (theMsg->GetMbox())
			// If message has mailbox its a copy operation across different servers
			CMailAccountManager::sMailAccountManager->AddMRUCopyTo(mbox);
		else
			// Plain append
			CMailAccountManager::sMailAccountManager->AddMRUAppendTo(mbox);
	}
} // CMboxProtocol::AppendMbox

unsigned long CMboxProtocol::GetMessageLocalUID(unsigned long uid)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return 0;

	// Fetch bits
	return mClient->_GetMessageLocalUID(uid);
}

// Get message header text from server
void CMboxProtocol::FetchItems(const ulvector& nums, bool uids, EFetchItems items)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Fetch bits
	mClient->_FetchItems(nums, uids, items);

} // CMboxProtocol::ReadHeader

// Get messages header text from server
void CMboxProtocol::ReadHeaders(const ulvector& nums, bool uids, const cdstring& hdrs)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Read it
	mClient->_ReadHeaders(nums, uids, hdrs);
}

// Get message header text from server
void CMboxProtocol::ReadHeader(CMbox* mbox, CMessage* msg)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn() || (GetCurrentMbox() != mbox))
		return;

	// Make it current (causes an update)
	//SetCurrentMbox(mbox, true, mbox->IsExamine());

	// Read it
	mClient->_ReadHeader(msg);

} // CMboxProtocol::ReadHeader

// Get attachment data into stream
void CMboxProtocol::ReadAttachment(CMbox* mbox, unsigned long msg_num, CAttachment* attach, LStream* aStream, bool peek, unsigned long count, unsigned long start)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn() || (GetCurrentMbox() != mbox))
		return;

	// Make it current (causes an update)
	//SetCurrentMbox(mbox, true, mbox->IsExamine());

	// Read it
	mClient->_ReadAttachment(msg_num, attach, aStream, peek, count, start);

} // CMboxProtocol::ReadHeader

// Copy raw attachment data into stream
void CMboxProtocol::CopyAttachment(CMbox* mbox, unsigned long msg_num, CAttachment* attach, costream& aStream, bool peek, unsigned long count, unsigned long start)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn() || (GetCurrentMbox() != mbox))
		return;

	// Make it current (causes an update)
	//SetCurrentMbox(mbox, true, mbox->IsExamine());

	// Copy it
	mClient->_CopyAttachment(msg_num, attach, &aStream, peek, count, start);

} // CMboxProtocol::ReadHeader

// Change UID on server
void CMboxProtocol::RemapUID(CMbox* mbox, unsigned long local_uid, unsigned long new_uid)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Change UID
	mClient->_RemapUID(local_uid, new_uid);

}

// Map local to remote UIDs
void CMboxProtocol::MapLocalUIDs(CMbox* mbox, const ulvector& uids, ulvector* missing, ulmap* local_map)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Map UIDs
	mClient->_MapLocalUIDs(uids, missing, local_map);
}

// Change flags on server
void CMboxProtocol::SetFlagMessage(CMbox* mbox, const ulvector& nums, bool uids, NMessage::EFlags flags, bool set)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn() || (GetCurrentMbox() != mbox))
		return;

	// Make it current (causes an update)
	//SetCurrentMbox(mbox, true, mbox->IsExamine());

	// Change flags
	mClient->_SetFlag(nums, uids, flags, set);

} // CMboxProtocol::SetFlagMessage

// Copy message to mailbox
void CMboxProtocol::CopyMessage(CMbox* mbox_from, const ulvector& nums, bool uids, CMbox* mbox_to, ulmap& copy_uids, bool doMRU)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn() || (GetCurrentMbox() != mbox_from))
		return;

	// Make it current (causes an update)
	//SetCurrentMbox(mbox_from, true, mbox_from->IsExamine());

	// Destination must exist in disconnected mode
	if (IsDisconnected())
		TouchMbox(mbox_to);

	// Unmark it as deleted
	mClient->_CopyMessage(nums, uids, mbox_to, copy_uids);

	// Add to MRU list
	if (doMRU)
		CMailAccountManager::sMailAccountManager->AddMRUCopyTo(mbox_to);

} // CMboxProtocol::CopyMessage

// Copy message to mailbox
void CMboxProtocol::CopyMessage(CMbox* mbox, unsigned long msg_num, bool uids, costream& aStream, unsigned long count, unsigned long start)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn() || (GetCurrentMbox() != mbox))
		return;

	// Make it current (causes an update)
	//SetCurrentMbox(mbox, true, mbox->IsExamine());

	// Unmark it as deleted
	mClient->_CopyMessage(msg_num, uids, &aStream, count, start);

} // CMboxProtocol::CopyMessage

// Does server do copies?
bool CMboxProtocol::DoesCopy() const
{
	//return mClient->_DoesCopy();
	return true;
}

// Do message expunge
void CMboxProtocol::ExpungeMessage(const ulvector& nums, bool uids)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Send EXPUNGE message to server
	mClient->_ExpungeMessage(nums, uids);
}

// Does server handle copy?
bool CMboxProtocol::DoesExpungeMessage() const
{
	return mClient->_DoesExpungeMessage();
}

#pragma mark ____________________________SORT/THREAD

// Does server-side sorting
bool CMboxProtocol::DoesSort(ESortMessageBy sortby) const
{
	return mClient->_DoesSort(sortby);
}

// Do server-side sort
void CMboxProtocol::Sort(ESortMessageBy sortby, EShowMessageBy show_by, const CSearchItem* search, ulvector* results, bool uids)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Send command to server
	mClient->_Sort(sortby, show_by, search, results, uids);
}
	
// Does server-side threading
bool CMboxProtocol::DoesThreading(EThreadMessageBy threadby) const
{
	return mClient->_DoesThreading(threadby);
}

// Do server-side thread
void CMboxProtocol::Thread(EThreadMessageBy threadby, const CSearchItem* search, threadvector* results, bool uids)
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Send command to server
	mClient->_Thread(threadby, search, results, uids);
}

#pragma mark ____________________________ACLs

// Set new value on server
void CMboxProtocol::SetACL(CMbox* mbox, CACL* acl)
{
	// Do nothing if not available
	if (!UsingACLs())
		return;

	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Send command to server
	// Mbox will take care of adding/replacing in list if successful
	mClient->_SetACL(mbox, acl);
}

// Delete value on server
void CMboxProtocol::DeleteACL(CMbox* mbox, CACL* acl)
{
	// Do nothing if not available
	if (!UsingACLs())
		return;

	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Send command to server
	// Mbox will take care of adding/replacing in list if successful
	mClient->_DeleteACL(mbox, acl);
}

// Get values on server
void CMboxProtocol::GetACL(CMbox* mbox)
{
	// Do nothing if not available
	if (!UsingACLs())
		return;

	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Send command to server
	// Mbox will take care of recovery
	mClient->_GetACL(mbox);
}

// List rights on server
void CMboxProtocol::ListRights(CMbox* mbox, CACL* acl)
{
	// Do nothing if not available
	if (!UsingACLs())
		return;

	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Send command to server
	// Mbox will take care of recovery
	mClient->_ListRights(mbox, acl);
}

// List user's rights on server
void CMboxProtocol::MyRights(CMbox* mbox)
{
	// Do nothing if not available
	if (!UsingACLs())
		return;

	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Send command to server
	// Mbox will take care of recovery
	mClient->_MyRights(mbox);
}

#pragma mark ____________________________Quotas

// Add unique quotaroot to list (copies original)
void CMboxProtocol::AddQuotaRoot(CQuotaRoot* root)
{
	// Do nothing if not available
	if (!UsingQuotas())
		return;

	// Try to find it in existing list
	CQuotaRootList::iterator found = std::find(mRoots.begin(), mRoots.end(), *root);

	// If found replace it with copy
	if (found != mRoots.end())
		*found = *root;
	else
		// Just add copy to list
		mRoots.push_back(*root);
}

// Add unique quotaroot to list (copies original)
CQuotaRoot* CMboxProtocol::FindQuotaRoot(const char* txt)
{
	// Do nothing if not available
	if (!UsingQuotas())
		return NULL;

	// Create dummy for lookup
	CQuotaRoot dummy(txt);

	// Try to find it in existing list
	CQuotaRootList::iterator found = std::find(mRoots.begin(), mRoots.end(), dummy);

	// If found return it
	if (found != mRoots.end())
		return &(*found);
	else
		return NULL;
}

// Set new value on server
void CMboxProtocol::SetQuota(CQuotaRoot* root)
{
	// Do nothing if not available
	if (!UsingQuotas())
		return;

	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Just send command to server
	// Server will take care of adding/replacing item in mRoots
	mClient->_SetQuota(root);
}

// Get value from server
void CMboxProtocol::GetQuota(CQuotaRoot* root)
{
	// Do nothing if not available
	if (!UsingQuotas())
		return;

	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Just send command to server
	// Server will take care of replacing item in mRoots
	mClient->_GetQuota(root);
}

// Get CMboxProtocol roots for a mailbox
void CMboxProtocol::GetQuotaRoot(CMbox* mbox)
{
	// Do nothing if not available
	if (!UsingQuotas())
		return;

	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Just send command to server
	// Server will take care of replacing item in mRoots
	mClient->_GetQuotaRoot(mbox);
}

#pragma mark ____________________________Disconnected

void CMboxProtocol::InitDisconnect()
{
	// Do inherited to setup directory
	CINETProtocol::InitDisconnect();

	// Now create recorder
	if (!mRecorder && !IsCloned())
	{
		mRecorder = new CMailRecord;
		mRecorder->SetDescriptor(mOfflineCWD + "Record.mbr");
		mRecorder->SetFullRecord();
	}
	if (mRecorder)
		mRecorder->Open();
}

const cdstring& CMboxProtocol::GetOfflineDefaultDirectory()
{
	static cdstring name("Mailboxes");
	return name;
}

void CMboxProtocol::GoOffline(bool force, bool sync_list)
{
	// Check that offline directory exists
	if (mOfflineCWD.empty())
		InitOffline();

	// Dump current mailbox list to disk if not being forced off
	if (!force)
		DumpMailboxes(sync_list);

	// Now do inherited connection switch
	CINETProtocol::GoOffline();

	// If not being dumped, read previous list from disk
	// Do this AFTER changing to local protocol
	if (!sync_list)
	{
		ClearList();
		ReadMailboxes();
	}
	else
		SwitchMailboxes(this);
}

void CMboxProtocol::GoOnline(bool updating_only)
{
	// Reset mailbox state
	SwitchMailboxes(NULL);

	// Now do inherited connection switch
	CINETProtocol::GoOnline();
}

void CMboxProtocol::SynchroniseRemote(CMbox* mbox, bool fast, bool partial, unsigned long size, const ulvector& uids)
{
	// Only if possible
	if (!CanDisconnect())
		return;

	// For each mailbox being synchronised do this:
	//
	// Clone this but force it to local
	// Create a temp mailbox object and give it the local proto
	// Sync mailbox from this proto to temp one

	// Create local clone
	std::auto_ptr<CMboxProtocol> clone(new CMboxProtocol(*this, true));
	clone->SetSynchronising();

	try
	{
		// Open with faked logged on status
		clone->Open();
		clone->SetState(eINETLoggedOn);

		// Create copy of mailbox
		std::auto_ptr<CMbox> temp(new CMbox(*mbox));

		// Local mailbox is given local protocol
		temp->SetProtocol(clone.get());

		// Make sure local item exists
		clone->TouchMbox(temp.get());

		// Now sync them both
		SyncRemote(mbox, temp.get(), fast, partial, size, uids);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		clone->SetState(eINETLoggedOff);
		clone->Close();
		CLOG_LOGRETHROW;
		throw;
	}

	clone->SetState(eINETLoggedOff);
	clone->Close();
}

void CMboxProtocol::SyncRemote(CMbox* remote, CMbox* local, bool fast, bool partial, unsigned long size, const ulvector& uids)
{
	//StProfileSection profile("\pSyncRemote Profile", 200, 20);

	// Special tricks for fast mode:
	if (fast)
	{
		try
		{
			// Need to ensure that UIDValidity and UIDNext are up to date in each mailbox

			// See if open
			if (remote->IsOpen())
			{
				// UIDValidity is OK (as its returned in SELECT)
				// but UIDNext may be out of date (its sometimes returned
				// in SELECT but new message arrival will invalidate it anyway)
				
				// Set UID next to UID of last message plus one
				if (remote->GetNumberFound())
				{
					// Is last message cached
					if (!remote->GetMessage(remote->GetNumberFound())->IsFullyCached())
					{
						// Get the UID of the last message
						ulvector seq_end;
						seq_end.push_back(remote->GetNumberFound());
						remote->GetMsgProtocol()->FetchItems(seq_end, false, eUID);
					}
					remote->SetUIDNext(remote->GetMessage(remote->GetNumberFound())->GetUID() + 1);
				}
			}
			else
				// Do check (STATUS) if closed
				remote->Check();
			
			// Just do local check
			local->Check();
			
			// Compare UIDValidities and UIDNexts
			if ((remote->GetUIDValidity() == local->GetUIDValidity()) &&
				(remote->GetUIDNext() == local->GetUIDNext()))
			{
				// Everything the same => no need to sync, but update last sync time
				local->SetFlags(NMbox::eSynchronising);
				local->Open(NULL, false, false, false);
				time_t now = ::time(NULL);
				local->ChangeLastSync(::mktime(::gmtime(&now)));
				remote->SetLastSync(local->GetLastSync());
				local->CloseSilent();
				
				// All done
				return;
			}
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);
		}
	}
	
	// If fast then only sync new messages in remote store
	// If uids then only sync specific messages in remote store

	bool remote_open = remote->IsOpen();

	try
	{
		// Open them both
		remote->SetFlags(NMbox::eSynchronising);
		if (!remote_open)
		{
			// Check for reusable proto
			CMboxProtocol* reuse = NULL;
			if (CMailAccountManager::sMailAccountManager->ReuseProto() &&
				CMailAccountManager::sMailAccountManager->GetReusableProto())
			{
				reuse = CMailAccountManager::sMailAccountManager->GetReusableProto();
				
				// Check for matching servers
				if (reuse->GetCloneOwner() != remote->GetProtocol())
				{
					// Close unwanted
					try
					{
						reuse->Close();
						delete reuse;
						reuse = NULL;
						CMailAccountManager::sMailAccountManager->SetReusableProto(reuse);
					}
					catch (...)
					{
						CLOG_LOGCATCH(...);
					}
				}
			}
			remote->Open(reuse, false, true);
		}
		local->SetFlags(NMbox::eSynchronising);
		local->Open(NULL, false);

		// Compare UIDValidities
		if (remote->GetUIDValidity() != local->GetUIDValidity())
		{
			// Expunge all messages!
			if (local->GetNumberFound())
			{
				ulvector seq_all;
				seq_all.push_back(0);
				local->ExpungeMessage(seq_all, false);
			}

			// Now reset UIDValidity
			local->ChangeUIDValidity(remote->GetUIDValidity());
			
			// Set UIDNext to 0 to force reset of cache in fast mode
			local->ChangeUIDNext(0);
		}

		// Get local message uids first
		// NB Assumes that local and remote UIDs are the same
		ulvector seq_all;
		seq_all.push_back(0);
		if (local->GetNumberFound())
			local->GetMsgProtocol()->FetchItems(seq_all, false, static_cast<CMboxProtocol::EFetchItems>(eUID | eFlags));

		// Get local and partial uids - don't do zero UIDs
		// Do this twice - once to get count to reserve vector size, second to do actual vector insert
		ulvector local_uids;
		ulvector partial_uids;
		{
			unsigned long local_uids_size = 0;
			unsigned long partial_uids_size = 0;
			for(unsigned long i = 1; i <= local->GetNumberFound(); i++)
			{
				CMessage* msg = local->GetMessage(i);
				unsigned long msg_uid = msg->GetUID();
				if (msg_uid)
				{
					local_uids_size++;
					if (local->GetMessage(i)->IsPartial())
						partial_uids_size++;
				}
			}
			local_uids.reserve(local_uids_size);
			partial_uids.reserve(partial_uids_size);
			for(unsigned long i = 1; i <= local->GetNumberFound(); i++)
			{
				CMessage* msg = local->GetMessage(i);
				unsigned long msg_uid = msg->GetUID();
				if (msg_uid)
				{
					local_uids.push_back(msg_uid);
					if (local->GetMessage(i)->IsPartial())
						partial_uids.push_back(msg_uid);
				}
			}
		}

		// Get list of remote uids for processing
		bool got_remote_flags = false;
		ulvector remote_uids;
		if (uids.size())
			// Use specified remote uids
			remote_uids = uids;
		else
		{
			// Get last local uid
			unsigned long last_local_uid = fast && local_uids.size() ? local_uids.back() : 0;

			if (remote->GetNumberFound() && !last_local_uid)
			{
				remote->GetMsgProtocol()->FetchItems(seq_all, false, static_cast<CMboxProtocol::EFetchItems>(eUID | eFlags));
				got_remote_flags = true;
			}
			else if (remote->GetNumberFound())
			{
				ulvector uid_end;
				uid_end.push_back(last_local_uid+1);
				uid_end.push_back(0);
				remote->GetMsgProtocol()->FetchItems(uid_end, true, eUID);
			}

			// Get all remote uids greater than last local
			// Do count first to reserve vector size
			{
				unsigned long remote_uids_size = 0;
				for(unsigned long i = 1; i <= remote->GetNumberFound(); i++)
				{
					if (remote->GetMessage(i)->GetUID() > last_local_uid)
						remote_uids_size++;
				}
				remote_uids.reserve(remote_uids_size);
				for(unsigned long i = 1; i <= remote->GetNumberFound(); i++)
				{
					if (remote->GetMessage(i)->GetUID() > last_local_uid)
						remote_uids.push_back(remote->GetMessage(i)->GetUID());
				}
			}
		}

		// Reduce set of remote uids to those not in local

		// Remove messages from local
		ulvector remove;

		// Find ones in local not in remote if not fast and not selection only
		if (!fast && !uids.size())
			std::set_difference(local_uids.begin(), local_uids.end(), remote_uids.begin(), remote_uids.end(), std::back_inserter<ulvector>(remove));

		// If full cache, remove the partial ones so they get fully cached
		if (!partial && !size && partial_uids.size())
		{
			ulvector temp1;
			temp1.reserve(partial_uids.size());

			// Find partials in remote
			std::set_intersection(partial_uids.begin(), partial_uids.end(), remote_uids.begin(), remote_uids.end(), std::back_inserter<ulvector>(temp1));

			// Expunge these as well
			if (temp1.size())
			{
				ulvector temp2;
				temp2.reserve(remove.size() + temp1.size());
				std::set_union(remove.begin(), remove.end(), temp1.begin(), temp1.end(), std::back_inserter<ulvector>(temp2));
				remove = temp2;
			}
		}

		// Expunge the unwanted ones
		if (remove.size())
		{
			local->ExpungeMessage(remove, true);

			// Recalc local uids
			local_uids.clear();
			{
				unsigned long local_uids_size = 0;
				for(unsigned long i = 1; i <= local->GetNumberFound(); i++)
				{
					CMessage* msg = local->GetMessage(i);
					unsigned long msg_uid = msg->GetUID();
					if (msg_uid)
						local_uids_size++;
				}
				local_uids.reserve(local_uids_size);
				for(unsigned long i = 1; i <= local->GetNumberFound(); i++)
				{
					CMessage* msg = local->GetMessage(i);
					unsigned long msg_uid = msg->GetUID();
					if (msg_uid)
						local_uids.push_back(msg_uid);
				}
			}
		}

		// Sync remaining messages
		ulvector copy;
		{
			// Find ones in remote not in local
			copy.reserve((remote_uids.size() > local_uids.size()) ? remote_uids.size() - local_uids.size() : remote_uids.size());
			std::set_difference(remote_uids.begin(), remote_uids.end(), local_uids.begin(), local_uids.end(), std::back_inserter<ulvector>(copy));

			// Copy the new ones
			if (copy.size())
			{
				// Check for partial and set flag
				ulvector make_partial;
				if (partial)
				{
					// Find messages not of type text/*
					cdstring field("Content-Type");
					cdstring text("text/");
					CSearchItem search_spec(CSearchItem::eNot, new CSearchItem(CSearchItem::eHeader, field, text));
					ulvector results;
					remote->Search(&search_spec, &results, true, true);

					// Intersect with ones actually being copied
					make_partial.reserve(results.size());
					std::set_intersection(results.begin(), results.end(), copy.begin(), copy.end(), std::back_inserter<ulvector>(make_partial));
				}
				else if (size)
				{
					// Find messages over a particular size
					CSearchItem search_spec(CSearchItem::eLarger, size);
					ulvector results;
					remote->Search(&search_spec, &results, true, true);

					// Intersect with ones actually being copied
					make_partial.reserve(results.size());
					std::set_intersection(results.begin(), results.end(), copy.begin(), copy.end(), std::back_inserter<ulvector>(make_partial));
					
					// Make sure partial fetch count is set
					if (make_partial.size())
						remote->SetPartialCount(size);
				}

				// Make specific ones partial
				for(ulvector::const_iterator iter = make_partial.begin(); iter != make_partial.end(); iter++)
				{
					CMessage* msg = remote->GetMessageUID(*iter);
					msg->GetFlags().Set(NMessage::ePartial);
				}

				ulmap temp;
				remote->CopyMessage(copy, true, local, temp);

				// Recalc local uids
				local_uids.clear();
				{
					unsigned long local_uids_size = 0;
					for(unsigned long i = 1; i <= local->GetNumberFound(); i++)
					{
						CMessage* msg = local->GetMessage(i);
						unsigned long msg_uid = msg->GetUID();
						if (msg_uid)
							local_uids_size++;
					}
					local_uids.reserve(local_uids_size);
					for(unsigned long i = 1; i <= local->GetNumberFound(); i++)
					{
						CMessage* msg = local->GetMessage(i);
						unsigned long msg_uid = msg->GetUID();
						if (msg_uid)
							local_uids.push_back(msg_uid);
					}
				}

				// Update message status if mailbox open
				if (remote_open)
				{
					// Check for partial and reset flag
					for(ulvector::const_iterator iter = copy.begin(); iter != copy.end(); iter++)
					{
						CMessage* msg = remote->GetMessageUID(*iter);
						if (!msg->IsPartial())
							msg->GetFlags().Set(NMessage::eFullLocal);
					}

					// Check for partial and reset flag
					if (make_partial.size())
					{
						for(ulvector::const_iterator iter = make_partial.begin(); iter != make_partial.end(); iter++)
						{
							CMessage* msg = remote->GetMessageUID(*iter);
							msg->GetFlags().Set(NMessage::ePartial, false);
							msg->GetFlags().Set(NMessage::ePartialLocal);
						}
					}
				}
			}
		}

		// Now get all flags
		{
			// First subtract ones that were just copied from remote set
			ulvector flag_sync;
			flag_sync.reserve(remote_uids.size() - copy.size());
			std::set_difference(remote_uids.begin(), remote_uids.end(), copy.begin(), copy.end(), std::back_inserter<ulvector>(flag_sync));

			// Get all flags
			if (flag_sync.size())
			{
				// Only interested in specific ones
				if (!got_remote_flags)
					remote->GetMsgProtocol()->FetchItems(flag_sync, true, eFlags);
				local->GetMsgProtocol()->FetchItems(flag_sync, true, eFlags);

				// Now sync them
				ulvector set_seen;
				ulvector unset_seen;
				ulvector set_answered;
				ulvector unset_answered;
				ulvector set_flagged;
				ulvector unset_flagged;
				ulvector set_deleted;
				ulvector unset_deleted;
				ulvector set_draft;
				ulvector unset_draft;
				ulvector set_mdn;
				ulvector unset_mdn;

				// Will throw if a message is out of range
				try
				{
					// Need to generate map of UID -> msg # to improve performance
					ulmap remote_map;
					ulmap local_map;
					remote->GetUIDMap(remote_map);
					local->GetUIDMap(local_map);

					// Do twice, once to get vector reserve size, then to actually add to vector
					unsigned long set_seen_size = 0;
					unsigned long unset_seen_size = 0;
					unsigned long set_answered_size = 0;
					unsigned long unset_answered_size = 0;
					unsigned long set_flagged_size = 0;
					unsigned long unset_flagged_size = 0;
					unsigned long set_deleted_size = 0;
					unsigned long unset_deleted_size = 0;
					unsigned long set_draft_size = 0;
					unsigned long unset_draft_size = 0;
					unsigned long set_mdn_size = 0;
					unsigned long unset_mdn_size = 0;

					for(ulvector::const_iterator iter = flag_sync.begin(); iter != flag_sync.end(); iter++)
					{
						// Determine sequence numbers from map
						unsigned long remote_seq = 0;
						unsigned long local_seq = 0;
						
						ulmap::const_iterator remote_find = remote_map.find(*iter);
						if (remote_find != remote_map.end())
							remote_seq = (*remote_find).second;
						ulmap::const_iterator local_find = local_map.find(*iter);
						if (local_find != local_map.end())
							local_seq = (*local_find).second;

						// If either missing just skip it
						if (!remote_seq || !local_seq)
							continue;

						CMessage* remote_msg = remote->GetMessage(remote_seq);
						CMessage* local_msg = local->GetMessage(local_seq);

						// If either missing just skip it
						if (!remote_msg || !local_msg)
							continue;

						// Now check each flag and change those that are different
						if (remote_msg->IsUnseen() ^ local_msg->IsUnseen())
							(remote_msg->IsUnseen() ? unset_seen_size++ : set_seen_size++);
						if (remote_msg->IsAnswered() ^ local_msg->IsAnswered())
							(remote_msg->IsAnswered() ? set_answered_size++ : unset_answered_size++);
						if (remote_msg->IsFlagged() ^ local_msg->IsFlagged())
							(remote_msg->IsFlagged() ? set_flagged_size++ : unset_flagged_size++);
						if (remote_msg->IsDeleted() ^ local_msg->IsDeleted())
							(remote_msg->IsDeleted() ? set_deleted_size++ : unset_deleted_size++);
						if (remote_msg->IsDraft() ^ local_msg->IsDraft())
							(remote_msg->IsAnswered() ? set_draft_size++ : unset_draft_size++);
						if (remote_msg->IsMDNSent() ^ local_msg->IsMDNSent())
							(remote_msg->IsMDNSent() ? set_mdn_size++ : unset_mdn_size++);
					}

					set_seen.reserve(set_seen_size);
					unset_seen.reserve(unset_seen_size);
					set_answered.reserve(set_answered_size);
					unset_answered.reserve(unset_answered_size);
					set_flagged.reserve(set_flagged_size);
					unset_flagged.reserve(unset_flagged_size);
					set_deleted.reserve(set_deleted_size);
					unset_deleted.reserve(unset_deleted_size);
					set_draft.reserve(set_draft_size);
					unset_draft.reserve(unset_draft_size);
					set_mdn.reserve(set_mdn_size);
					unset_mdn.reserve(unset_mdn_size);

					for(ulvector::const_iterator iter = flag_sync.begin(); iter != flag_sync.end(); iter++)
					{
						// Determine sequence numbers from map
						unsigned long remote_seq = 0;
						unsigned long local_seq = 0;
						
						ulmap::const_iterator remote_find = remote_map.find(*iter);
						if (remote_find != remote_map.end())
							remote_seq = (*remote_find).second;
						ulmap::const_iterator local_find = local_map.find(*iter);
						if (local_find != local_map.end())
							local_seq = (*local_find).second;

						// If either missing just skip it
						if (!remote_seq || !local_seq)
							continue;

						CMessage* remote_msg = remote->GetMessage(remote_seq);
						CMessage* local_msg = local->GetMessage(local_seq);

						// If either missing just skip it
						if (!remote_msg || !local_msg)
							continue;

						// Now check each flag and change those that are different
						if (remote_msg->IsUnseen() ^ local_msg->IsUnseen())
							(remote_msg->IsUnseen() ? unset_seen : set_seen).push_back(remote_msg->GetUID());
						if (remote_msg->IsAnswered() ^ local_msg->IsAnswered())
							(remote_msg->IsAnswered() ? set_answered : unset_answered).push_back(remote_msg->GetUID());
						if (remote_msg->IsFlagged() ^ local_msg->IsFlagged())
							(remote_msg->IsFlagged() ? set_flagged : unset_flagged).push_back(remote_msg->GetUID());
						if (remote_msg->IsDeleted() ^ local_msg->IsDeleted())
							(remote_msg->IsDeleted() ? set_deleted : unset_deleted).push_back(remote_msg->GetUID());
						if (remote_msg->IsDraft() ^ local_msg->IsDraft())
							(remote_msg->IsAnswered() ? set_draft : unset_draft).push_back(remote_msg->GetUID());
						if (remote_msg->IsMDNSent() ^ local_msg->IsMDNSent())
							(remote_msg->IsMDNSent() ? set_mdn : unset_mdn).push_back(remote_msg->GetUID());
					}
				}
				catch (...)
				{
					CLOG_LOGCATCH(...);
				}

				// Now set/unset the flags
				if (set_seen.size())
					local->SetFlagMessage(set_seen, true, NMessage::eSeen, true);
				if (unset_seen.size())
					local->SetFlagMessage(unset_seen, true, NMessage::eSeen, false);
				if (set_answered.size())
					local->SetFlagMessage(set_answered, true, NMessage::eAnswered, true);
				if (unset_answered.size())
					local->SetFlagMessage(unset_answered, true, NMessage::eAnswered, false);
				if (set_flagged.size())
					local->SetFlagMessage(set_flagged, true, NMessage::eFlagged, true);
				if (unset_flagged.size())
					local->SetFlagMessage(unset_flagged, true, NMessage::eFlagged, false);
				if (set_deleted.size())
					local->SetFlagMessage(set_deleted, true, NMessage::eDeleted, true);
				if (unset_deleted.size())
					local->SetFlagMessage(unset_deleted, true, NMessage::eDeleted, false);
				if (set_draft.size())
					local->SetFlagMessage(set_draft, true, NMessage::eDraft, true);
				if (unset_draft.size())
					local->SetFlagMessage(unset_draft, true, NMessage::eDraft, false);
				if (set_mdn.size())
					local->SetFlagMessage(set_mdn, true, NMessage::eMDNSent, true);
				if (unset_mdn.size())
					local->SetFlagMessage(unset_mdn, true, NMessage::eMDNSent, false);
			}
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Close them both
		if (remote_open)
			remote->SetFlags(NMbox::eSynchronising, false);
		else
		{
			// Always close
			remote->CloseSilent();

			// Check for protocol reuse and always undo
			CMailAccountManager::sMailAccountManager->SetReusableProto(NULL);
		}
		local->CloseSilent();

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}

	// Clean up sync info
	local->ChangeUIDNext(remote->GetUIDNext());
	time_t now = ::time(NULL);
	local->ChangeLastSync(::mktime(::gmtime(&now)));
	remote->SetLastSync(local->GetLastSync());

	// Close them both
	if (remote_open)
		remote->SetFlags(NMbox::eSynchronising, false);
	else
	{
		// Check for protocol reuse
		if (CMailAccountManager::sMailAccountManager->ReuseProto())
		{
			// Reuse current proto
			CMboxProtocol* reuse = NULL;
			remote->CloseMboxOnly(&reuse, true);
			CMailAccountManager::sMailAccountManager->SetReusableProto(reuse);
		}
		else
			remote->CloseSilent();
	}
	local->CloseSilent();
}


void CMboxProtocol::ClearDisconnect(CMbox* mbox, const ulvector& uids)
{
	// The mailbox is either remote or local
	if (mbox->GetProtocol()->IsDisconnected())
	{
		// Only bother if cache actually exists
		if (!TestMbox(mbox))
			return;

		// Don't record the delete/expunge operation!
		static_cast<CLocalClient*>(mClient)->SetRecorder(NULL);
		try
		{
			if (uids.size())
				ExpungeMessage(const_cast<ulvector&>(uids), true);	// Mailbox must already be open at this point
			else
			{
				DeleteMbox(mbox, false);
				
				// Remove flag from mailbox
				mbox->SetFlags(NMbox::eCachedMbox, false);
			}
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			static_cast<CLocalClient*>(mClient)->SetRecorder(mRecorder);
			CLOG_LOGRETHROW;
			throw;
		}
		static_cast<CLocalClient*>(mClient)->SetRecorder(mRecorder);
	}
	else
	{
		// Create local clone
		std::auto_ptr<CMboxProtocol> clone(new CMboxProtocol(*this, true));
		clone->SetSynchronising();

		try
		{
			// Open with faked logged on status
			clone->Open();
			clone->SetState(eINETLoggedOn);

			// Create copy of mailbox
			std::auto_ptr<CMbox> temp(new CMbox(*mbox));

			// Local mailbox is given local protocol
			temp->SetProtocol(clone.get());

			// Only bother if local item exists
			if (clone->TestMbox(temp.get()))
			{
				if (uids.size())
				{
					CMbox* local = temp.get();
					local->SetFlags(NMbox::eSynchronising);
					local->Open(NULL, false);
					local->ExpungeMessage(const_cast<ulvector&>(uids), true);
					local->CloseSilent();

					// Remove
				}
				else
					clone->DeleteMbox(temp.get(), false);
			}
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			clone->SetState(eINETLoggedOff);
			clone->Close();
			CLOG_LOGRETHROW;
			throw;
		}

		clone->SetState(eINETLoggedOff);
		clone->Close();
	}
}

const char* cMailboxListName = "mailboxlist";
const char* cMailboxListTempName = "mailboxlist.temp";

void CMboxProtocol::DumpMailboxes(bool full_sync)
{
	// Actual name of mailboxlist file
	cdstring list_name = mOfflineCWD + cMailboxListName;

	// Temp name of mailboxlist file
	cdstring list_temp_name = mOfflineCWD + cMailboxListTempName;

	// Force to a full sync if the mailbox list file currently does not exist.
	// This takes care of sync'ing the list the first time a user disconnects.
	if (!::fileexists(list_name))
		full_sync = true;

	// For now only process when doing a full sync. In the future we will
	// do smart partial sync's of the list
	if (!full_sync)
		return;

	try
	{
		cdofstream dump(list_temp_name);

		// Write out each hierarchy
		for(CHierarchies::const_iterator iter = mHierarchies.begin(); iter != mHierarchies.end(); iter++)
		{
			// Make sure entire list gets cached (don't bother with subscribed or when disconnected)
			if (!IsDisconnected() && (iter != mHierarchies.begin()))
				DeepSubList(*iter);
			
			// Now write to stream
			(*iter)->DumpHierarchy(dump);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Delete temp file
		::remove_utf8(list_temp_name);
		
		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}

	// Check if it exists and delete
	if (::fileexists(list_name))
		::remove_utf8(list_name);

	// Rename temp to new
	::rename_utf8(list_temp_name, list_name);
}

void CMboxProtocol::ReadMailboxes()
{
	cdstring list_name = mOfflineCWD + cMailboxListName;
	cdifstream dump(list_name);
	if (dump.fail())
		return;

	// Check cached state of INBOX
	if (mINBOX)
		mINBOX->SwitchDisconnect(this);

	// Read in each hierarchy
	unsigned long i = 0;
	for(CHierarchies::const_iterator iter = mHierarchies.begin(); iter != mHierarchies.end(); iter++, i++)
	{
		// Skip based on show value
		if (!ShowSubscribed() && !i)
			continue;
		if (!ShowUnsubscribed() && i)
			continue;

		if (!(*iter)->IsLoaded())
		{
			// Cache current size
			unsigned long old_size = (*iter)->size();

			// Remove all existing mailboxes
		 	(*iter)->DeleteAll();

			// Turn off sorting during big load
			(*iter)->SetFlag(CTreeNodeList::eSorted, false);

			// Get mailboxes from list
			(*iter)->ReadHierarchy(dump, this);

			// Special for subscribed
			if (!i)
			{
				unsigned long new_size = (*iter)->size();

				// Broadcast change to Subscribed to make sure its hidden/shown
				if (!old_size && new_size)
					Broadcast_Message(eBroadcast_NewList, *iter);
				else if (old_size && !new_size)
					Broadcast_Message(eBroadcast_RemoveList, *iter);
			}

			// Must reset to starting root
			(*iter)->SetRoot((*iter)->GetName());

			// Sort now
			(*iter)->SetFlag(CTreeNodeList::eSorted, true);

			// Adjust directories
			(*iter)->AdjustDirectories();
		}
		else
			// Punt over items in list
			(*iter)->ReadHierarchy(dump, this, false);
	}

	// Refresh mbox list
	SyncMbox();
}

void CMboxProtocol::SwitchMailboxes(CMboxProtocol* local)
{
	// Check cached state of INBOX
	if (mINBOX)
		mINBOX->SwitchDisconnect(local);

	// Write out each hierarchy
	for(CHierarchies::const_iterator iter1 = mHierarchies.begin(); iter1 != mHierarchies.end(); iter1++)
	{
		// Must do each mailbox
		for(CMboxList::const_iterator iter2 = (*iter1)->begin(); iter2 != (*iter1)->end(); iter2++)
			static_cast<CMbox*>(*iter2)->SwitchDisconnect(local);
	}
}

bool CMboxProtocol::DoPlayback(CProgress* progress)
{
	// Ignore if no defined recorder
	if (!mRecorder)
		return true;

	// Create remote clone
	std::auto_ptr<CMboxProtocol> clone(new CMboxProtocol(*this, false, true));
	clone->SetSynchronising();

	// Prevent further recording
	mRecorder->SetNoRecord();

	bool result = mRecorder->Playback(clone.get(), this, progress);

	// Enable recording
	mRecorder->SetFullRecord();
	
	return result;
}

void CMboxProtocol::GetDisconnectedMessageState(CMbox* remote, ulvector& full, ulvector& partial)
{
	// Create local clone
	std::auto_ptr<CMboxProtocol> clone(new CMboxProtocol(*this, true));
	clone->SetSynchronising();
	clone->SetNoErrorAlert(true);
	clone->SetNoRecovery(true);

	try
	{
		// Open with faked logged on status
		clone->Open();
		clone->SetState(eINETLoggedOn);

		// Create copy of mailbox
		std::auto_ptr<CMbox> local(new CMbox(*remote));

		// Local mailbox is given local protocol
		local->SetProtocol(clone.get());

		// Make sure local item exists
		if (clone->TestMbox(local.get()))
		{
			local->SetFlags(NMbox::eSynchronising);
			local->Open(NULL, false);

			// Compare UIDs
			if (remote->GetUIDValidity() == local->GetUIDValidity())
			{
				// Need all UIDs and flags
				ulvector seq_all;
				seq_all.push_back(0);
				if (local->GetNumberFound())
					local->GetMsgProtocol()->FetchItems(seq_all, false, static_cast<CMboxProtocol::EFetchItems>(eUID | eFlags));

				// Optimise for large mailboxes:
				// Count the items in each list first, reserve each size then add to lists
				unsigned long full_count = 0;
				unsigned long partial_count = 0;

				for(unsigned long i = 1; i <= local->GetNumberFound(); i++)
				{
					CMessage* msg = local->GetMessage(i);
					if (msg->IsPartial())
						partial_count++;
					else
						full_count++;
				}
				
				// Reserve enough space for all possibilities - waste of memory but time is an isue with large mailboxes
				full.reserve(full_count);
				partial.reserve(partial_count);

				// test and add to appropriate list
				for(unsigned long i = 1; i <= local->GetNumberFound(); i++)
				{
					CMessage* msg = local->GetMessage(i);
					if (msg->IsPartial())
						partial.push_back(msg->GetUID());
					else
						full.push_back(msg->GetUID());
				}
			}

			local->CloseSilent();
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		clone->SetState(eINETLoggedOff);
		clone->Close();
		CLOG_LOGRETHROW;
		throw;
	}

	clone->SetState(eINETLoggedOff);
	clone->Close();
}

// Get last sync of remote mailbox
void CMboxProtocol::GetLastSync(CMbox* remote)
{

	// Create local clone
	std::auto_ptr<CMboxProtocol> clone(new CMboxProtocol(*this, true));
	clone->SetSynchronising();

	try
	{
		// Open with faked logged on status
		clone->Open();
		clone->SetState(eINETLoggedOn);

		// Create copy of mailbox
		std::auto_ptr<CMbox> local(new CMbox(*remote));

		// Local mailbox is given local protocol
		local->SetProtocol(clone.get());

		// Make sure local item exists
		if (clone->TestMbox(local.get()))
		{
			local->SetFlags(NMbox::eSynchronising);
			local->Check();

			// Get last sync time from local and give it to remote
			remote->SetLastSync(local->GetLastSync());
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		clone->SetState(eINETLoggedOff);
		clone->Close();
		CLOG_LOGRETHROW;
		throw;
	}

	clone->SetState(eINETLoggedOff);
	clone->Close();
}
