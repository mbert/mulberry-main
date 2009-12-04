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


// Source for CMbox class

#include "CMbox.h"

#include "CFilterManager.h"
#include "CIMAPCommon.h"
#include "CINETClient.h"
#include "CLocalMessage.h"
#include "CMailAccountManager.h"
#include "CMailControl.h"
#include "CMboxProtocol.h"
#include "CMessageThread.h"
#include "CPreferences.h"
#include "CSearchItem.h"
#include "CStatusWindow.h"
#include "CStringUtils.h"
#include "CTaskClasses.h"

#include "CURL.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
#include "CWaitCursor.h"
#endif

#include <algorithm>

#include "cdomutex.h"

#pragma mark ____________________________CMboxOpen

CMbox::mbox_mutex CMbox::CMboxOpen::_mutex;

// Default constructor
CMbox::CMboxOpen::CMboxOpen()
{
	mMsgMailer = NULL;
	mMessages = NULL;
	mSortedMessages = NULL;
	mNumberCached = 0;
	mCacheIncrement = 0;
	mCheckRecent = 0;
	mMboxRecent = 0;
	mViewMode = eViewMode_All;
	mAutoViewMode = false;
	mSortBy = cSortMessageNumber;
	mShowBy = cShowMessageAscending;
	mFirstNew = ULONG_MAX;
	mRefCount = 0;
	
	mPartialCount = 0;
	mPartialStart = 0;
}

CMbox::CMboxOpen::~CMboxOpen()
{
}

#pragma mark ____________________________CMboxStatus

// Default constructor
CMbox::CMboxStatus::CMboxStatus()
{
	mNumberExists = 0;
	mNumberRecent = 0;
	mNumberUnseen = 0;
	mSize = ULONG_MAX;
	mUIDValidity = 0;
	mUIDNext = 0;
	mLastSync = 0;
	mAllowedFlags = NMessage::eAllPermanent;
	mACLs = NULL;
	mQuotas = NULL;
}

#pragma mark ____________________________CMbox

CMbox::mbox_mutex CMbox::_smutex;

// Default constructor - private so should never be called
CMbox::CMbox()
{
	InitMbox();
}

// Copy constructor
CMbox::CMbox(const CMbox& copy)
{
	InitMbox();

	mMailer = copy.mMailer;
	mDirDelim = copy.mDirDelim;
	SetName(copy.GetName());
	mMboxList = copy.mMboxList;
	mFlags = copy.mFlags;

	// To be safe, never copy these flags
	SetFlags(eOpen, false);
	SetFlags(eFullOpen, false);
	SetFlags(eReadOnly, false);
	SetFlags(eExamine, false);
	SetFlags(eNoCache, false);
	SetFlags(eDeleteOnClose, false);
}

// Construct with protocol and name
CMbox::CMbox(CMboxProtocol* mailer, const char* itsFullName, char itsDirDelim, CMboxList* itsMboxList, bool subscribed)
{
	InitMbox();
	SetProtocol(mailer);

	// Set dir delim before setting name as it is required for short name
	mDirDelim = itsDirDelim;

	// Look for directory
	if (itsFullName[::strlen(itsFullName) - 1] == mDirDelim)
	{
		// Use name without trailing dir delim
		cdstring temp_name(itsFullName, ::strlen(itsFullName) - 1);
		SetName(temp_name);

		// Mark it as \NoSelect and not \NoInferiors to indicate a directory
		SetFlags(eNoSelect);
		SetFlags(eNoInferiors, false);
	}
	else
		// Copy name and do other inits
		SetName(itsFullName);
	mMboxList = itsMboxList;

	SetFlags(eSubscribed, subscribed);

	if (subscribed)
		// Sync subscribed favourite
		CMailAccountManager::sMailAccountManager->ChangeFavouriteItem(this, CMailAccountManager::eFavouriteSubscribed, true);
}

// Default destructor
CMbox::~CMbox()
{
	// Kill any open connection
	if (mOpenInfo)
	{
		mMailer->EndConnection(mOpenInfo->mMsgMailer);

		// Make sure protocols really know we are no longer current
		if (mMailer->GetCurrentMbox() == this)
			mMailer->ClearCurrentMbox();
		if (mOpenInfo->mMsgMailer->GetCurrentMbox() == this)
			mOpenInfo->mMsgMailer->ClearCurrentMbox();
	 
	 	// Clean-up items
		if (IsFullOpen())
			mOpenInfo->mSortedMessages->DeleteFakes();
		delete mOpenInfo->mSortedMessages;
		delete mOpenInfo->mMessages;
		delete mOpenInfo;
	}

	delete mStatusInfo;
	
	mMailer = NULL;
	mMboxList = NULL;
	mOpenInfo = NULL;
	mStatusInfo = NULL;
}

void CMbox::InitMbox()
{
	mMailer = NULL;
	mWDLevel = -1;
	mMboxList = NULL;
	//mFlags = eNoInferiors;	// <-- this is difficult because it excludes creation of sub-hierarchies
								// on servers that accept it.
	mFlags = eNone;				// Changed so that creation is allowed, but my fail on some servers
	mDirDelim = 0;

	mStatusInfo = NULL;
	mOpenInfo = NULL;
}

// Add it to list
CMbox* CMbox::AddMbox()
{
	// Try and do best match for list if none specified
	if (!mMboxList)
		mMboxList = GetProtocol()->FindMatchingList(this);

	// Check for singletons
	if (mMboxList)
		return mMailer->AddMbox(this);		// WARNING: may delete this
	else
	{
		mMailer->AddSingleton(this);
		return this;
	}
}

void CMbox::SetProtocol(CMboxProtocol* proto)
{
	mMailer = proto;

	if (mMailer)
	{
		SetFlags(eLocalMbox, mMailer->IsOffline());
		SetFlags(eCachedMbox, mMailer->IsOffline() && !mMailer->IsDisconnected());
	}
}

// Get account relative name of mailbox
cdstring CMbox::GetAccountName(bool multi) const
{
	cdstring acct;

	if (multi)
	{
		acct += mMailer->GetAccountName();
		acct += cMailAccountSeparator;
	}
	acct += mName;

	return acct;
}

// Get URL of mailbox
cdstring CMbox::GetURL(bool full) const
{
	cdstring url = mMailer->GetURL(full);
	url += cURLHierarchy;
	cdstring name = mName;
	name.EncodeURL(mDirDelim);		// Must encode
	url += name;

	return url;
}

// Get identity associated with mailbox
const CIdentity* CMbox::GetTiedIdentity() const
{
	const CIdentity* result = NULL;

	// Try identity for this one first
	result = CPreferences::sPrefs->mTiedMailboxes.GetValue().GetTiedMboxIdentity(this);
	
	// Try inherited ones next
	if (!result && CPreferences::sPrefs->mTiedMboxInherit.GetValue())
	{
		// Get account name and dir char
		cdstring acctname = GetAccountName();
		
		char delim = GetDirDelim();
		
		// Chop at dir delim character
		if (delim)
		{
			// Loop up the hierarchy looking for a tied identity
			while(!result && (::strrchr(acctname.c_str(), delim) != NULL))
			{
				*::strrchr(acctname.c_str_mod(), delim) = 0;
				result = CPreferences::sPrefs->mTiedMailboxes.GetValue().GetTiedMboxIdentity(acctname);
			}
		}
	}
	
	return result;
}

// Set list flags only
void CMbox::SetListFlags(EFlags new_flags)
{
	// Clear existing list flags
	SetFlags((EFlags) (eNoInferiors | eNoSelect | eMarked | eUnMarked | eHasInferiors), false);

	// Set new flags
	SetFlags(new_flags);
}

// Set subscribed state
void CMbox::SetSubscribed(bool subs)
{
	SetFlags(eSubscribed, subs);

	//mMailer->MboxChanged(this);

} // CMbox::SetSubscribed

// Get WD prefix
const cdstring&	CMbox::GetWD() const
{
	return (mMboxList && (mMboxList->GetHierarchyIndex() > 0)) ? mMboxList->GetName() : cdstring::null_str;
}

// Update allowed flags status
void CMbox::DetermineAllowedFlags()
{
	// Only if status exists
	if (!HasStatus())
		return;

	// Policy:
	//
	// If READ-ONLY then use [PERMANENTFLAGS] response (already setup after SELECT)
	// If READ_WRITE then use MYRIGHTS (on non-ACL server this will default to all on)

	// If READ-ONLY turn all flags off
	if (!IsReadOnly())
	{
		NMessage::EFlags flags = NMessage::eNone;

		// Special case support of labels/mdnsent
		NMessage::EFlags mdnsent = ((GetAllowedFlags() & NMessage::eMDNSent) != 0) ? NMessage::eMDNSent : NMessage::eNone;
		NMessage::EFlags labels = ((GetAllowedFlags() & NMessage::eLabels) != 0) ? NMessage::eLabels : NMessage::eNone;

		// Check MYRIGHTS
		CheckMyRights();

		// Determine flags from rights
		SACLRight myrights = GetMyRights();

		// s - \Seen allowed
		if (myrights.HasRight(CMboxACL::eMboxACL_Seen))
			flags = static_cast<NMessage::EFlags>(flags | NMessage::eSeen);

		// w - \Answered \Flagged \Draft $MDNSent labels allowed
		if (myrights.HasRight(CMboxACL::eMboxACL_Write))
		{
			if (IsLocalMbox())
				flags = static_cast<NMessage::EFlags>(flags | NMessage::eAnswered | NMessage::eFlagged | NMessage::eDraft |
																mdnsent | NMessage::ePartial | NMessage::eError | labels);
			else
				flags = static_cast<NMessage::EFlags>(flags | NMessage::eAnswered | NMessage::eFlagged | NMessage::eDraft | mdnsent | labels);
		}

		// d - \Deleted allowed
		if (myrights.HasRight(CMboxACL::eMboxACL_Delete))
			flags = static_cast<NMessage::EFlags>(flags | NMessage::eDeleted);

		SetAllowedFlags(flags);
	}
}

// Set open somewhere state
void CMbox::SetOpenSomewhere(bool open)
{
	// Make sure status of all similar mailboxes are sync'd

	// Look for mailbox in lists
	CMboxList list;
	GetProtocol()->FindMbox(GetName(), list);

	// Update status of others to this one
	for(CMboxList::iterator iter = list.begin(); iter != list.end(); iter++)
		static_cast<CMbox*>(*iter)->SetFlags(eOpenSomewhere, open);;
}

// Clear new recent pseudo flag from all messages
void CMbox::ClearCheckRecent()
{
	// Clear recent bit in all messages if cached
	if (IsFullOpen())
	{
		for(CMessageList::iterator iter = mOpenInfo->mSortedMessages->begin(); iter != mOpenInfo->mSortedMessages->end(); iter++)
		{
			if ((*iter)->IsCheckRecent())
				(*iter)->ClearCheckRecent();
		}
	}
}

// Clear smart address flag from all messages
void CMbox::ClearSmartAddress()
{
	// Clear recent bit in all messages if cached
	if (IsFullOpen())
	{
		for(CMessageList::iterator iter = mOpenInfo->mSortedMessages->begin(); iter != mOpenInfo->mSortedMessages->end(); iter++)
			(*iter)->ResetSmart();
	}
}

// Copy status information from another mailbox
void CMbox::UpdateStatus(const CMbox* copy)
{
	// This is called after a check on another mailbox and is used to sync
	// new message counters in all mailboxes

	// Must not do self
	if (this == copy)
		return;

	// This mailbox CANNOT be open - only the original mailbox checked can ever be open

	// Force status info
	InitStatusInfo();

	// Copy relevant info from other mailbox
	SetNumberFound(copy->GetNumberFound());
	SetNumberRecent(copy->GetNumberViewRecent());	// Use view recent in case mailbox is open
	SetNumberUnseen(copy->GetNumberUnseen());
}

// Are there any new messages
bool CMbox::AnyNew() const
{
	unsigned long new_unseen = GetNumberUnseen();
	unsigned long new_recent = GetNumberRecent();
	unsigned long new_new = GetMboxRecent();
	bool is_new;

	// Process differently depending on open state of mailbox (this is an IMAPism)
	if (IsOpen())
	{
		// Use user determined concept of 'new' messages
		if (CPreferences::sPrefs->mUnseenNew.GetValue() && CPreferences::sPrefs->mRecentNew.GetValue())
			is_new = (new_unseen > 0) && (new_new > 0);
		else if (CPreferences::sPrefs->mUnseenNew.GetValue())
			is_new = (new_unseen > 0);
		else if (CPreferences::sPrefs->mRecentNew.GetValue())
			is_new = (new_new > 0);
	}
	else
	{
		// Use user determined concept of 'new' messages
		if (CPreferences::sPrefs->mUnseenNew.GetValue() && CPreferences::sPrefs->mRecentNew.GetValue())
			is_new = (new_unseen > 0) && (new_recent > 0);
		else if (CPreferences::sPrefs->mUnseenNew.GetValue())
			is_new = (new_unseen > 0);
		else if (CPreferences::sPrefs->mRecentNew.GetValue())
			is_new = (new_recent > 0);
	}
	
	return is_new;
}

#pragma mark ____________________________ACLS

// Get user's rights from server
void CMbox::CheckMyRights()
{
	// If its local then it must be cached, and must have logged in protocol
	if (IsLocalMbox() && !IsCachedMbox() || !mMailer->IsLoggedOn())
		return;

	// Force status info
	InitStatusInfo();

	if (IsOpen())
		mOpenInfo->mMsgMailer->MyRights(this);
	else
		mMailer->MyRights(this);
}

// Add ACL to list
void CMbox::AddACL(const CMboxACL* acl)
{
	// If its local then it must be cached, and must have logged in protocol
	if (IsLocalMbox() && !IsCachedMbox() || !mMailer->IsLoggedOn())
		return;

	InitStatusInfo();

	// Create list if it does not exist
	if (mStatusInfo)
	{
		if (!mStatusInfo->mACLs)
			mStatusInfo->mACLs = new CMboxACLList;

		// Add
		mStatusInfo->mACLs->push_back(*acl);
	}
}

// Set ACL on server
void CMbox::SetACL(CMboxACL* acl)
{
	// If its local then it must be cached, and must have logged in protocol
	if (IsLocalMbox() && !IsCachedMbox() || !mMailer->IsLoggedOn())
		return;

	InitStatusInfo();

	// Try to set on server
	try
	{
		mMailer->SetACL(this, acl);

		if (mStatusInfo)
		{
			// Create list if it does not exist
			if (!mStatusInfo->mACLs)
				mStatusInfo->mACLs = new CMboxACLList;

			// Search for existing ACL
			CMboxACLList::iterator found = std::find(mStatusInfo->mACLs->begin(), mStatusInfo->mACLs->end(), *acl);

			// Add if not found
			if (found == mStatusInfo->mACLs->end())
				mStatusInfo->mACLs->push_back(*acl);
			else
				// Replace existing
				*found = *acl;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Silent failure
	}
}

// Set ACL on server
void CMbox::DeleteACL(CMboxACL* acl)
{
	// If its local then it must be cached, and must have logged in protocol
	if (IsLocalMbox() && !IsCachedMbox() || !mMailer->IsLoggedOn())
		return;

	InitStatusInfo();

	// Try to delete on server
	try
	{
		mMailer->DeleteACL(this, acl);

		if (mStatusInfo && mStatusInfo->mACLs)
		{
			// Search for existing ACL
			CMboxACLList::iterator found = std::find(mStatusInfo->mACLs->begin(), mStatusInfo->mACLs->end(), *acl);

			// Remove it
			if (found != mStatusInfo->mACLs->end())
				mStatusInfo->mACLs->erase(found);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Silent failure
	}
}

// Reset all acls
void CMbox::ResetACLs(const CMboxACLList* acls)
{
	// If its local then it must be cached, and must have logged in protocol
	if (IsLocalMbox() && !IsCachedMbox() || !mMailer->IsLoggedOn())
		return;

	InitStatusInfo();

	try
	{
		// Delete all on server
		if (mStatusInfo->mACLs)
		{
			while(mStatusInfo->mACLs->size())
			{
				mMailer->DeleteACL(this, &mStatusInfo->mACLs->front());
				mStatusInfo->mACLs->erase(mStatusInfo->mACLs->begin());
			}
		}
		
		// Now add all the new ones
		for(CMboxACLList::const_iterator iter = acls->begin(); iter != acls->end(); iter++)
			SetACL(const_cast<CMboxACL*>(static_cast<const CMboxACL*>(&(*iter))));
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Silent failure
	}
}

// Get ACLs from server
void CMbox::CheckACLs()
{
	// If its local then it must be cached, and must have logged in protocol
	if (IsLocalMbox() && !IsCachedMbox() || !mMailer->IsLoggedOn())
		return;

	InitStatusInfo();

	// Save existing list in case of failure
	CMboxACLList* save = (mStatusInfo->mACLs ? new CMboxACLList(*mStatusInfo->mACLs) : NULL);

	try
	{
		// Delete everything in existing list
		if (mStatusInfo->mACLs)
			mStatusInfo->mACLs->clear();

		mMailer->GetACL(this);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Replace failed list with old one
		if (save)
			*mStatusInfo->mACLs = *save;
		delete save;

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}

	delete save;
}

#pragma mark ____________________________Quotas

// Add unique quotaroot to list
void CMbox::AddQuotaRoot(const char* txt)
{
	// If its local then it must be cached, and must have logged in protocol
	if (IsLocalMbox() && !IsCachedMbox() || !mMailer->IsLoggedOn())
		return;

	if (!mStatusInfo)
		return;

	// Create list if it does not exist
	if (!mStatusInfo->mQuotas)
		mStatusInfo->mQuotas = new cdstrvect;

	// Try to find it in existing list
	cdstring temp(txt);
	cdstrvect::iterator found = std::find(mStatusInfo->mQuotas->begin(), mStatusInfo->mQuotas->end(), temp);

	// Add if not found
	if (found == mStatusInfo->mQuotas->end())
		mStatusInfo->mQuotas->push_back(temp);
}

// Get list of quotaroots relevant to this mailbox
CQuotaRootList* CMbox::GetQuotas()
{
	// Create list to return
	CQuotaRootList* list = new CQuotaRootList;

	// If its local then it must be cached, and must have logged in protocol
	if (IsLocalMbox() && !IsCachedMbox() || !mMailer->IsLoggedOn())
		return list;

	if (mStatusInfo && mStatusInfo->mQuotas)
	{
		// Iterate over each entry and add copies of matches to list
		for(cdstrvect::const_iterator iter = mStatusInfo->mQuotas->begin(); iter != mStatusInfo->mQuotas->end(); iter++)
		{
			CQuotaRoot* root = mMailer->FindQuotaRoot(*iter);
			if (root)
				list->push_back(*root);
		}
	}

	return list;
}

// Refresh quota list
void CMbox::CheckQuotas()
{
	// If its local then it must be cached, and must have logged in protocol
	if (IsLocalMbox() && !IsCachedMbox() || !mMailer->IsLoggedOn())
		return;

	// Force status info
	InitStatusInfo();

	mMailer->GetQuotaRoot(this);
}

#pragma mark ____________________________Mailbox Ops

// Create mailbox
void CMbox::Create()
{
	// Create it on server
	mMailer->CreateMbox(this);

} // CMbox::Create

// Open mailbox
void CMbox::Open(CMboxProtocol* proto, bool update, bool examine, bool full)
{
	mbox_lock _lock(CMboxOpen::_mutex, this);

	// Open it on server if not already opened
	if (!IsOpen())
	{
		try
		{
			// Indicate in Open command
			SetFlags(eBeingOpened, true);

			// Tell main server to deselect
			mMailer->OpeningMbox(this);		// Must do before state change

			// Force status
			InitStatusInfo();

			// Create open object
			mOpenInfo = new CMboxOpen;

			// Create message lists (do before login as check thread may attempt to access them)
			mOpenInfo->mMessages = (full ? new CMessageList : NULL);
			mOpenInfo->mSortedMessages = (full ? new CMessageList(true) : NULL);
			mOpenInfo->mSortBy = cSortMessageNumber;
			mOpenInfo->mShowBy = cShowMessageAscending;

			// Look for connection to reuse
			if (proto)
				mOpenInfo->mMsgMailer = proto;
			else
				// Create new server connection
				//mOpenInfo->mMsgMailer = mMailer->CloneConnection();
				mOpenInfo->mMsgMailer = static_cast<CMboxProtocol*>(mMailer->NewConnection());
			if (!mOpenInfo->mMsgMailer)
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}
			
			mOpenInfo->mMsgMailer->Open();
			mOpenInfo->mMsgMailer->Logon();

			// Update state after preparing the mOpenInfo structure and opening server
			SetOpen(true);
			SetFullOpen(full);

			// Open via server
			mOpenInfo->mMsgMailer->OpenMbox(this, update || full, examine);

			// Now find all unseen/deleted/recent messages if updating
			if (update)
				DoInitialSearch();

			// Determine allowed flags if full open
			if (full)
				DetermineAllowedFlags();

			// Always process new messages
			if (!IsSynchronising() && update)
				mOpenInfo->mMsgMailer->ProcessCheckMbox(this);

			// No longer in Open command
			SetFlags(eBeingOpened, false);

		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			SetFullOpen(false);
			SetOpen(false);
			SetFlags(eBeingOpened, false);

			// Race condition can result in Close already done so check that we still have this
			if (mOpenInfo)
			{
				// Only remove proto if new
				if (!proto)
					mMailer->EndConnection(mOpenInfo->mMsgMailer);

				// Make sure protocols really know we are no longer current
				if (mMailer->GetCurrentMbox() == this)
					mMailer->ClearCurrentMbox();
				if (mOpenInfo->mMsgMailer->GetCurrentMbox() == this)
					mOpenInfo->mMsgMailer->ClearCurrentMbox();
			 
			 	// CLean-up other items
				if (mOpenInfo->mSortedMessages)
					mOpenInfo->mSortedMessages->DeleteFakes();
				delete mOpenInfo->mSortedMessages;
				delete mOpenInfo->mMessages;
				delete mOpenInfo;
				mOpenInfo = NULL;
			}

			CLOG_LOGRETHROW;
			throw;
		}

		CMailControl::MboxReset(this);
	}
	else
	{
		// Bump up reference count
		mOpenInfo->mRefCount++;
		
		// Don't use this code for now
#if 0
		// Check for change to full state
		if (!IsFullOpen() && full)
		{
			// Force into full open state

			// Create message lists (do before login as check thread may attempt to access them)
			mOpenInfo->mMessages = new CMessageList;
			mOpenInfo->mSortedMessages = new CMessageList(true);

			SetFullOpen(full);

			// Open via server
			mOpenInfo->mMsgMailer->OpenMbox(this, update || full, examine);

			// Now find all unseen/deleted/recent messages if updating
			if (update)
				DoInitialSearch();

			// Determine allowed flags if full open
			if (full)
				DetermineAllowedFlags();

			// Always process new messages
			if (!IsSynchronising() && update)
				mOpenInfo->mMsgMailer->ProcessCheckMbox(this);
		}
#endif
	}

} // CMbox::Open

// Reopen mailbox
void CMbox::Reopen()
{
	// Open it again if not already opened but open info exists
	if (!IsOpen() && mOpenInfo)
	{
		try
		{
			// Update state
			SetOpen(true);
			SetFullOpen(true);

			// Open via server
			mOpenInfo->mMsgMailer->OpenMbox(this);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Just throw - caller will handle
			CLOG_LOGRETHROW;
			throw;
		}

		CMailControl::MboxReset(this);
	}
}

// Recover from closed connection
void CMbox::Recover()
{
	// Force mailbox cache to be cleared for now
	// Ultimately we should do a UID sync on cached data
	UncacheMessages();

	// Recover clone state - we must still have an open cache, otherwise throw
	if (!mOpenInfo || !mOpenInfo->mMsgMailer)
	{
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
	}


	// Reset some state info
	SetCheckRecent(0);
	SetMboxRecent(0);
	SetFirstNew(ULONG_MAX);

	try
	{
		// Indicate in Open command
		SetFlags(eBeingOpened, true);

		// Clear out the cached data to force a proper cache reload and take into account
		// changes to the size of the mailbox whilst it was disconnected
		mStatusInfo->mNumberExists = 0;
		mOpenInfo->mSortedMessages->DeleteFakes();
		mOpenInfo->mMessages->DeleteAll();
		SetNumberCached(0);

		// This effectively opens the mailbox
		mOpenInfo->mMsgMailer->RecoverClone();

		// Now find all unseen/deleted/recent messages if fully open
		if (IsFullOpen())
			DoInitialSearch();

		SetFlags(eBeingOpened, false);

		// Force new mail check filter if unseen count non-zero
		if (GetNumberUnseen() && CPreferences::sPrefs && CPreferences::sPrefs->GetFilterManager())
			CPreferences::sPrefs->GetFilterManager()->NewMailMailbox(this, 0);

		// Force update of server and mailbox panes
		CMailControl::MboxRefresh(this);
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		SetFlags(eBeingOpened, false);
	}

} // CMbox::Reopen

// Open it if already open!
bool CMbox::OpenIfOpen()
{
	mbox_lock _lock(CMboxOpen::_mutex, this);

	// Test open state
	bool was_open = IsOpen();
	
	// If its currently open, then open again and bump ref count
	if (was_open)
		// Bump up reference count
		mOpenInfo->mRefCount++;
	
	return was_open;
}

// Close
void CMbox::Close(bool grab_proto, bool no_check, bool force, bool allow_punt, CMboxProtocol** reuse)
{
	mbox_lock _lock(CMboxOpen::_mutex, this);

	// Don't bother if not open
	if (!IsOpen())
		return;

	// Must not close if not safe to do so unless being forced
	if (!force && GetMsgProtocol()->_get_mutex().is_locked())
		return;

	// Only close if open in a single thread
	if (mOpenInfo->mRefCount)
	{
		// Decrease ref count
		mOpenInfo->mRefCount--;

		// Can never reuse the current protocol because someone else is using it
		if (reuse)
			*reuse = NULL;

		return;
	}

	// Close it on server if already opened
	try
	{
		// Do hack to broadcast logoff
		mOpenInfo->mMsgMailer->Broadcast_Message(CMboxProtocol::eBroadcast_MailMboxLogoff, this);

		// Clean up the UI
		CMailControl::CleanUpMboxClose(this);

		if (force)
		{
			// Update state
			SetFullOpen(false);
			SetOpen(false);

			mOpenInfo->mMsgMailer->Forceoff();
		}
		else
		{
			// Do punt on close if required and can set seen flag and not examined
			if (allow_punt && HasAllowedFlag(NMessage::eSeen) &&
				!mFlags.IsSet(eExamine) && mFlags.IsSet(ePuntOnClose) &&
				(mOpenInfo->mMsgMailer->GetCurrentMbox() == this))
				PuntUnseen();

#if 0	// Old code
			if (!grab_proto)
			{
				// Fully close the mbox as the protocol is not being reused
				mOpenInfo->mMsgMailer->CloseMbox(this);
				if (reuse)
					*reuse = NULL;
			}
			else
			{
				// Only deselect it from the protocol which will be reused
				mOpenInfo->mMsgMailer->ForgetMbox(this);
				if (reuse)
					*reuse = mOpenInfo->mMsgMailer;
			}
#endif

			// Use new connection caching scheme

			// Pretend that it is not fully open BEFORE closing it. This will prevent
			// attempts to resort the mailbox if new mail indication arrives during an
			// IMAP EXAMINE/CLOSE operation.
			SetFullOpen(false);

			// Only deselect it from the protocol which will be cached/reused
			mOpenInfo->mMsgMailer->ForgetMbox(this);

			if (!grab_proto)
			{
				// Give connection back to cache
				mMailer->EndConnection(mOpenInfo->mMsgMailer);
				if (reuse)
					*reuse = NULL;
			}
			else
			{
				if (reuse)
					*reuse = mOpenInfo->mMsgMailer;
			}
			
			// Update state
			SetOpen(false);

#if 0			// No longer need this with connection caching
			// Only logoff if not grabbing proto
			if (!grab_proto)
			{
				// Logoff and close as its no longer needed
				mOpenInfo->mMsgMailer->Logoff();
				mOpenInfo->mMsgMailer->Close();
			}
#endif
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Always force state closed
		SetFullOpen(false);
		SetOpen(false);
	}

	// Throw out items

	// Make sure protocols really know we are no longer current
	if (mMailer->GetCurrentMbox() == this)
		mMailer->ClearCurrentMbox();
	if ((GetMsgProtocol() != NULL) && (GetMsgProtocol()->GetCurrentMbox() == this))
		GetMsgProtocol()->ClearCurrentMbox();
 
	// Only delete proto if not grabbing
#if 0	// Never do this with connection caching
	if (!grab_proto)
		delete mOpenInfo->mMsgMailer;
#endif
	if (mOpenInfo->mSortedMessages)
		mOpenInfo->mSortedMessages->DeleteFakes();
	delete mOpenInfo->mSortedMessages;
	delete mOpenInfo->mMessages;
	delete mOpenInfo;
	mOpenInfo = NULL;

	// Recent flags always reset on close
	SetNumberRecent(0);
	SetMboxRecent(0);
	SetCheckRecent(0);

	if (!IsSynchronising())
	{
		// Force update of cabinet
		mMailer->ProcessCheckMbox(this);

		// Always do server display update
		CMailControl::MboxUpdate(this);
	}

	// Always turn off synchronise mode on close
	SetFlags(eSynchronising, false);

	// If delete on close do delete (watch out for dangling pointers!)
	if (mFlags.IsSet(eDeleteOnClose))
	{
		// Remove the mailbox using an always queued task to ensure pending access to this object does not
		// cause a stale reference crash as removal will delete this.
		CRemoveSingletonMailboxTask* task = new CRemoveSingletonMailboxTask(this);
		task->Go();
	}

} // CMbox::Close

// Can call close
bool CMbox::SafeToClose() const
{
	return !CMboxOpen::_mutex.is_locked(this);
}

// Program initiated abort of command in progress
// Use switch to allow or disallow connection recovery
// This is important when the abort is caused by a user cancel (e.g. search)
// and the mailbox was previously open.
void CMbox::Abort(bool allow_recover)
{
	if (mOpenInfo)
		mOpenInfo->mMsgMailer->Abort(true, allow_recover && mOpenInfo->mRefCount);
}

// Rename
void CMbox::Rename(const char* new_name)
{

	// Check subscribed
	bool was_subscribed = IsSubscribed();
	if (was_subscribed)
		// Unsubscribe on server only as it will be repositioned in list later
		mMailer->UnsubscribeMbox(this, false);

	// Rename it on server
	mMailer->RenameMbox(this, new_name);

	if (was_subscribed)
		// Subscribe on server only as it will be repositioned in list later
		mMailer->SubscribeMbox(this, false);

	// If it was open must do reselect
	if (IsOpen())
	{
		// Fake closure and reopen
		SetFullOpen(false);
		SetOpen(false);
		Reopen();
	}

} // CMbox::Rename

// Rename
void CMbox::MoveMbox(const CMbox* dir, bool sibling)
{
	cdstring new_name;

	// Must have item
	if (!dir)
		return;

	// Check for sibling
	if (sibling)
	{
		// Rename this mailbox by taking its short name and appending to name of WD
		new_name = dir->GetName();
		char* p = (dir->GetDirDelim() ? ::strrchr(new_name.c_str_mod(), dir->GetDirDelim()) : NULL);
		if (p)
			*++p = 0;
		else
			new_name = cdstring::null_str;
		new_name += GetShortName();
	}
	else
	{
		// Destination must support inferiors
		if (dir->NoInferiors())
			return;

		// Rename this mailbox by taking its short name and appending to name of directory
		new_name = dir->GetName();
		if (dir->GetDirDelim())
			new_name += dir->GetDirDelim();
		new_name += GetShortName();
	}

	// Check for existing duplicate
	if (dir->GetProtocol()->FindMbox(new_name))
		return;

	// Check for same server
	if (GetProtocol() == dir->GetProtocol())
	{
		// Just rename
		Rename(new_name.c_str());
	}
	else
	{
		// May need to add trailing directory separator
		if (IsDirectory())
			new_name += dir->GetDirDelim();

		// Do create on new server and then copy
		CMbox* mbox = NULL;

		// Logon to server - must be successful
		if (!dir->GetProtocol()->IsLoggedOn())
		{
			CMailAccountManager::sMailAccountManager->StartProtocol(dir->GetProtocol());

			if (!dir->GetProtocol()->IsLoggedOn())
				return;
		}

		// Create the mailbox
		try
		{
			mbox = new CMbox(dir->GetProtocol(), new_name, dir->GetDirDelim(), dir->GetMboxList());
			mbox = mbox->AddMbox();
			if (!mbox)
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}
			mbox->Create();

		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// If created it was added to the server list, therefore just remove it from server list
			// DO NOT delete it as create may have failed because of duplicate mailbox and
			// the original mailbox will be deleted!
			if (mbox)
				mbox->Remove();

			// Throw up
			CLOG_LOGRETHROW;
			throw;
		}

		// Do copy if selectable
		if (!NoSelect())
			CopyMbox(mbox);

		// Do recursive move if sub-hierarchies
		if (IsHierarchy())
		{
			// Must be expanded
			if (!HasExpanded())
			{
				// Do deep expand off this root mailbox
				GetProtocol()->LoadSubList(this, true);
				SetFlags(eHasExpanded);
			}

			// Now iterate over children doing move
			CMbox* child = GetFirstChild();
			while(child)
			{
				child->MoveMbox(mbox, false);
				child = child->GetNextSibling();
			}
		}
	}

} // CMbox::MoveMbox

// Delete
void CMbox::Delete()
{
	// Check subscribed
	bool was_subscribed = IsSubscribed();
	if (was_subscribed)
		// Unsubscribe on server only as it will be removed from list later
		mMailer->UnsubscribeMbox(this, false);

	// Delete it on server (IMPORTANT this will also delete this object)
	mMailer->DeleteMbox(this);

} // CMbox::Delete

// Remove
void CMbox::Remove()
{
	// Just remove it from server list (IMPORTANT this will also delete this object)
	mMailer->RemoveMbox(this);

} // CMbox::Remove

// Subscribe
void CMbox::Subscribe(bool update)
{
	// Subscribe to it on server if not already
	if (!IsSubscribed() && ::strcmpnocase(mName, cINBOX))
		mMailer->SubscribeMbox(this, update);

} // CMbox::Subscribe

// Unsubscribe
void CMbox::Unsubscribe(bool update)
{
	// Unsubscribe from it on server if not already
	if (IsSubscribed() && ::strcmpnocase(mName, cINBOX))
		mMailer->UnsubscribeMbox(this, update);

} // CMbox::Unsubscribe

// Rebuild from cache
void CMbox::Rebuild()
{
	// Don't do if open for now - eventually we need to reset the message cache
	// as well when open
	if (!IsOpen())
		mMailer->RebuildMbox(this);
}

// Synchronise to local
void CMbox::Synchronise(bool fast, bool partial, unsigned long size)
{
	// Tell protocol to synchronise it
	ulvector uids;
	mMailer->SynchroniseRemote(this, fast, partial, size, uids);
}

// Clear disconnected cache
void CMbox::ClearDisconnect()
{
	ulvector temp;
	mMailer->ClearDisconnect(this, temp);

	// Clear out disconnected info
	if (!GetProtocol()->IsDisconnected() && IsFullOpen())
	{
		// Clear list of full & partial UIDs
		mOpenInfo->mFullLocal.clear();
		mOpenInfo->mPartialLocal.clear();

		// Clear flags on all messages
		for(CMessageList::iterator iter = mOpenInfo->mMessages->begin(); iter != mOpenInfo->mMessages->end(); iter++)
			(*iter)->GetFlags().Set(NMessage::eFullLocal | NMessage::ePartialLocal, false);

	}
}

// Switch into disconnected mode
void CMbox::SwitchDisconnect(CMboxProtocol* local)
{
	// See if going into disconnected mode (i.e. local != NULL)
	if (local)
	{
		// Set local flag
		SetFlags(NMbox::eLocalMbox);

		// See if it exists locally
		if (local->TestMbox(this))
			SetFlags(NMbox::eCachedMbox);
	}
	else
		// Remove disconnected flags
		SetFlags(static_cast<NMbox::EFlags>(NMbox::eLocalMbox | NMbox::eCachedMbox), false);

	// Always reset status info
	if (HasStatus())
	{
		delete mStatusInfo;
		mStatusInfo = NULL;
	}
}

// Get the first child mailbox of this one
CMbox* CMbox::GetFirstChild() const
{
	return (GetMboxList() != NULL) ? static_cast<CMbox*>(GetMboxList()->GetFirstChild(this)) : NULL;
}

// Get next sibling of this mailbox
CMbox* CMbox::GetNextSibling() const
{
	return (GetMboxList() != NULL) ? static_cast<CMbox*>(GetMboxList()->GetNextSibling(this)) : NULL;
}

// Sync with full and partial uids
void CMbox::SyncDisconnectedMessage(CMessage* msg)
{
	// See if this is remote and not disconnected
	if (GetMsgProtocol()->CanDisconnect() && !GetMsgProtocol()->IsDisconnected())
	{
		// Check the message UID
		unsigned long uid = msg->GetUID();

		// Look for full local
		if (std::binary_search(mOpenInfo->mFullLocal.begin(), mOpenInfo->mFullLocal.end(), uid))
			msg->GetFlags().Set(NMessage::eFullLocal);

		// Look for partial local
		else if (std::binary_search(mOpenInfo->mPartialLocal.begin(), mOpenInfo->mPartialLocal.end(), uid))
			msg->GetFlags().Set(NMessage::ePartialLocal);
	}
}

#pragma mark ____________________________Search Ops

// Get Recent/Seen/Deleted flags from server
void CMbox::DoInitialSearch()
{
	// Don't bother if its empty
	if (!GetNumberFound())
		return;

	// Do search for recent
	// NB Don't need to do search as recent messages are ALWAYS at the end
	// Also set the mbox recent flag
	if (GetNumberRecent())
	{
		// Poke into messages
		unsigned long start = GetNumberFound() - GetNumberRecent() + 1;
		unsigned long stop = GetNumberFound();
		for(unsigned long num = start; num <= stop; num++)
			GetMessage(num)->GetFlags().Set(NMessage::eRecent, true);
	}

	// To cope with broken IMAP servers that do not implement SEARCH we muct attempt failure recovery here
	try
	{
		// First turn off error reporting on the protocol
		GetMsgProtocol()->SetNoErrorAlert(true);

		// Do the unseen/deleted caching using SEARCH
		DoInitialSearchUsingSearch();

		// Turn on error reporting on the protocol
		GetMsgProtocol()->SetNoErrorAlert(false);
	}
	
	// Catch NO/BAD response and ignore the error
	catch(CINETClient::CINETException& ex)
	{
		CLOG_LOGCATCH(CINETException&);

		// Turn on error reporting on the protocol
		if (GetMsgProtocol() != NULL)
			GetMsgProtocol()->SetNoErrorAlert(false);
	}
	
	// Any other exception must propogate out
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Turn on error reporting on the protocol
		if (GetMsgProtocol() != NULL)
			GetMsgProtocol()->SetNoErrorAlert(false);

		CLOG_LOGRETHROW;
		throw;
	}

	// Now look in any disconnected cache for full or partial messages,
	// but only if not being synchronised
	if (GetMsgProtocol()->CanDisconnect() && !GetMsgProtocol()->IsDisconnected() && !IsSynchronising())
		GetMsgProtocol()->GetDisconnectedMessageState(this, mOpenInfo->mFullLocal, mOpenInfo->mPartialLocal);
}

// Get Seen/Deleted flags from server
void CMbox::DoInitialSearchUsingSearch()
{
	// Do search for unseen
	{
		CSearchItem search_spec(CSearchItem::eUnseen);
		ulvector results;
		Search(&search_spec, &results, false, true);

		// Poke into messages
		for(ulvector::const_iterator iter = results.begin(); iter != results.end(); iter++)
			GetMessage(*iter)->GetFlags().Set(NMessage::eSeen, false);

		// Update internal counter
		SetNumberUnseen(results.size());

		// Make sure first new is available
		mOpenInfo->mFirstNew = 0;
		if (CPreferences::sPrefs->mUnseenNew.GetValue() && CPreferences::sPrefs->mRecentNew.GetValue())
		{
			// Scan for recent AND unseen
			for(ulvector::const_iterator iter = results.begin(); iter != results.end(); iter++)
			{
				CMessage* aMsg = GetMessage(*iter);
				if (aMsg->IsRecent())
				{
					if (!mOpenInfo->mFirstNew)
						mOpenInfo->mFirstNew = *iter;
					aMsg->GetFlags().Set(NMessage::eCheckRecent | NMessage::eMboxRecent);
				}
			}

			// Set counters to initial values
			SetCheckRecent(results.size());
			SetMboxRecent(results.size());
		}
		else
		{
			// Scan for recent
			unsigned long start = GetNumberFound() - GetNumberRecent() + 1;
			unsigned long stop = GetNumberFound();
			for(unsigned long num = start; num <= stop; num++)
			{
				CMessage* aMsg = GetMessage(num);
				aMsg->GetFlags().Set(NMessage::eCheckRecent | NMessage::eMboxRecent);
			}

			// Set counters to initial values
			SetCheckRecent(GetNumberRecent());
			SetMboxRecent(GetNumberRecent());
			
			// Set first new based on preference
			if (CPreferences::sPrefs->mUnseenNew.GetValue() && results.size())
				mOpenInfo->mFirstNew = results.front();
			else if (CPreferences::sPrefs->mRecentNew.GetValue() && (start <= stop))
				mOpenInfo->mFirstNew = start;
		}
	}

	// Do search for deleted
	{
		CSearchItem search_spec(CSearchItem::eDeleted);
		ulvector results;
		Search(&search_spec, &results, false, true);

		// Poke into messages
		for(ulvector::const_iterator iter = results.begin(); iter != results.end(); iter++)
			GetMessage(*iter)->GetFlags().Set(NMessage::eDeleted, true);
	}
}

// SetViewMode : changes the view mode for view filtering
// returns bool : true if a change to the sort list occurred (i.e. need to do viual update)
// mode : the new mode to set
bool CMbox::SetViewMode(EViewMode mode)
{
	// Only change if there's a real change
	if (!IsFullOpen() || (mode == mOpenInfo->mViewMode))
		return false;

	// Update mode
	mOpenInfo->mViewMode = mode;

	// Remove existing sorted items
	mOpenInfo->mSortedMessages->DeleteFakes();

	// Always transfer sort results based on new mode
	switch(mOpenInfo->mViewMode)
	{
	case eViewMode_All:
	case eViewMode_AllMatched:
		// Copy all messages into sorted list and sort
		mOpenInfo->mSortedMessages->insert(mOpenInfo->mSortedMessages->begin(), mOpenInfo->mMessages->size(), 0);
		std::copy(mOpenInfo->mMessages->begin(), mOpenInfo->mMessages->end(), mOpenInfo->mSortedMessages->begin());
		mOpenInfo->mSortedMessages->SortDirty();
		break;

	case eViewMode_ShowMatch:
		// Copy matching into list
		for(ulvector::const_iterator iter = mOpenInfo->mViewSearchResults.begin(); iter != mOpenInfo->mViewSearchResults.end(); iter++)
			mOpenInfo->mSortedMessages->push_back(mOpenInfo->mMessages->at(*iter - 1));
		mOpenInfo->mSortedMessages->SortDirty();
		break;
	}

	// Always resort messages
	ReSort();

	// Changed if resorted
	return true;
}


// SetViewSearch : changes the search criteria used for view filtering
// returns bool: true if sort list was refreshed (i.e. need to reset visual display)
// spec : new search criteria to use - mbox will own this and delete it itself!
bool CMbox::SetViewSearch(const CSearchItem* spec, bool update)
{
	bool result = false;	// Indicates change to sort list occurred

	if (!mOpenInfo)
		return false;

	// Clear results and update current spec if not updating
	if (!update)
	{
		mOpenInfo->mViewSearchResults.clear();
		if (spec)
			mOpenInfo->mViewCurrent = *spec;
		else
			mOpenInfo->mViewCurrent.clear();
	}

	// If spec do search and save results
	Search(spec, &mOpenInfo->mViewSearchResults);

	// Now, if search changed may need to redo sort list
	switch(mOpenInfo->mViewMode)
	{
	case eViewMode_ShowMatch:
		{
			// Fake change to view mode to force reset
			EViewMode old = mOpenInfo->mViewMode;
			mOpenInfo->mViewMode = eViewMode_All;
			SetViewMode(old);
			result = true;
			break;
		}
	default:;
	}

	return result;
}

// Search for messages
// spec : search specification
// results : vector to store message number results
// no_flags : if true, message flags are not updated (default = false)
void CMbox::Search(const CSearchItem* spec, ulvector* results, bool uids, bool no_flags)
{
	InitStatusInfo();

	// Use internal results list if none specified
	if (!results)
	{
		results = &mStatusInfo->mSearchResults;

		// Clear any previous results
		results->clear();
	}

	// Only bother with search if something to search for
	if (spec)
	{
		// Check it on suitable server
		if (IsOpen())
			mOpenInfo->mMsgMailer->SearchMbox(this, spec, results, uids);
		else
			mMailer->SearchMbox(this, spec, results, uids);
	}

	// Always sort search results into ascending order
	std::sort(results->begin(), results->end());

	// Change search flags on messages
	if (!no_flags)
		SetSearchFlags(*results);
}

// Transfer existing search results and reset view mode
bool CMbox::OpenSearch()
{
	bool result = TransferSearch();

	// Force change in view mode if not already
	if (!result && mOpenInfo)
	{
		mOpenInfo->mViewMode = eViewMode_All;
		SetViewMode(eViewMode_ShowMatch);
		result = true;
	}

	return result;
}

// Transfer existing search results to view results and set flags
bool CMbox::TransferSearch()
{
	bool result = false;

	if (!mOpenInfo)
		return false;

	// Clear existing search spec
	mOpenInfo->mViewCurrent.clear();

	// Transfer search results to view results
	mOpenInfo->mViewSearchResults = mStatusInfo->mSearchResults;
	SetSearchFlags(mOpenInfo->mViewSearchResults);

	// Now, if search changed may need to redo sort list
	if (mOpenInfo->mViewMode == eViewMode_ShowMatch)
	{
		// Fake change to view mode to force reset
		EViewMode old = mOpenInfo->mViewMode;
		mOpenInfo->mViewMode = eViewMode_All;
		SetViewMode(old);
		result = true;
	}

	return result;
}

// Set search flag for messages that match
void CMbox::SetSearchFlags(const ulvector& matches)
{
	if (IsFullOpen())
	{
		ulvector::const_iterator index_iter = matches.begin();
		unsigned long index = 1;
		for(CMessageList::iterator iter = mOpenInfo->mMessages->begin(); iter != mOpenInfo->mMessages->end(); iter++, index++)
		{
			// Look for matching index
			if ((index_iter != matches.end()) && (index == *index_iter))
			{
				(*iter)->ChangeFlags(NMessage::eSearch, true);
				index_iter++;
			}
			else
				(*iter)->ClearSearch();
		}
	}
}

#pragma mark ____________________________Sorting Ops

// Determine if sorting is valid
bool CMbox::ValidSort(ESortMessageBy sort_by)
{
	// NB When there is server sorting, this should ask the server what type of sorting it can do

	// If all messages cached then sort is always valid and sort by number always valid
	if (!mOpenInfo || (sort_by == cSortMessageNumber) || FullyCached())
		return true;
	
	// Check for server sort
	else if (mOpenInfo->mMsgMailer->DoesSort(sort_by))
		return true;

	// Check for server threading
	else if ((sort_by == cSortMessageThread) && mOpenInfo->mMsgMailer->DoesThreading(cThreadMessageAny))
		return true;

	// Do caching
	else if (GetSortCache() >= GetNumberMessages() - GetNumberCached())
	{
		// Do cache all automatcally
		CacheAllMessages();
		return true;
	}
	else
		return false;
}

// Set sorting
bool CMbox::SortBy(ESortMessageBy sort_by)
{
	if (!mOpenInfo)
		return false;

	bool resort = (mOpenInfo->mSortBy != sort_by);
	mOpenInfo->mSortBy = sort_by;
	return resort ? ReSort() : true;
}

// Reset sorting
bool CMbox::ReSort()
{
	bool external = false;
	
	if (!IsFullOpen())
		return false;

	// Check to see whether there is a change in size due to sorting
	// This can happen when fake messages are inserted into threads and must be
	// signalled back to the caller to allow the caller to refresh any mailbox window display
	unsigned long old_size = mOpenInfo->mSortedMessages->size();

	// Don't do this anymore to allow 'current' sort mode to work
	// Local threading has been made 'safe' wrt message caching
	// 'Full Sort' will have triggered a cache all
	//
	// Force message caching here if required by client-side sort
	//if (!ValidSort(mOpenInfo->mSortBy))
	//	CacheAllMessages();

	// Check for server-side sort/threading and handle differently
	if (mOpenInfo->mMsgMailer->DoesSort(mOpenInfo->mSortBy))
	{
		// May need to use view results as search criteria to maintain semi-dynamic view
		CSearchItem input_window;
		bool do_sort = (GetNumberFound() > 0);
		if (do_sort && (GetViewMode() == eViewMode_ShowMatch))
		{
			// Add search results as search item
			CSearchItem temp(CSearchItem::eNumber, mOpenInfo->mViewSearchResults);
			input_window = temp;
			
			// Don't do sort command if results are empty
			do_sort = mOpenInfo->mViewSearchResults.size();
		}

		// Don't let server reverse because it does not do implicit sequence reversal
		ulvector results;
		if (do_sort)
			mOpenInfo->mMsgMailer->Sort(mOpenInfo->mSortBy, cShowMessageAscending,
										(GetViewMode() != eViewMode_ShowMatch) ? NULL : &input_window,
										&results, false);
		
		// Do global reverse
		if (mOpenInfo->mShowBy == cShowMessageDescending)
			std::reverse(results.begin(), results.end());

		// Manually re-order messages
		mOpenInfo->mSortedMessages->DeleteFakes();
		for(ulvector::const_iterator iter = results.begin(); iter != results.end(); iter++)
			mOpenInfo->mSortedMessages->push_back(mOpenInfo->mMessages->at(*iter - 1));
		mOpenInfo->mSortedMessages->SortDirty();
		external = true;
	}
	else if ((mOpenInfo->mSortBy == cSortMessageThread) && mOpenInfo->mMsgMailer->DoesThreading(cThreadMessageAny))
	{
		// May need to make fixed search results into a search criteria
		CSearchItem input_window;
		bool do_thread = (GetNumberFound() > 0);
		if (do_thread && (GetViewMode() == eViewMode_ShowMatch))
		{
			// Add search results as search item
			CSearchItem temp(CSearchItem::eNumber, mOpenInfo->mViewSearchResults);
			input_window = temp;
			
			// Don't do thread command if results are empty
			do_thread = mOpenInfo->mViewSearchResults.size();
		}

		bool use_references = mOpenInfo->mMsgMailer->DoesThreading(cThreadMessageReferences);

		// Don't let server reverse because it does not do implicit sequence reversal
		threadvector results;
		if (do_thread)
			mOpenInfo->mMsgMailer->Thread(use_references ? cThreadMessageReferences : cThreadMessageSubject,
											(GetViewMode() != eViewMode_ShowMatch) ? NULL : &input_window,
											&results, false);
		
		// Create a CMessageThread tree based on results - might be reversed
		mOpenInfo->mSortedMessages->DeleteFakes();
		CMessageThread::ThreadResults(this, results, mOpenInfo->mSortedMessages, mOpenInfo->mShowBy == cShowMessageDescending);
		mOpenInfo->mSortedMessages->SortDirty();

		external = true;
	}
	else
	{
		// Make sure References header is cached for all messages when doing thread sorting
		if (mOpenInfo->mSortBy == cSortMessageThread)
			LoadReferences();
	}

	// Now do new sort - NB won't do it if external
	mOpenInfo->mSortedMessages->SetSortBy(mOpenInfo->mSortBy, mOpenInfo->mShowBy, external);
	
	// Set flag to indicate external sorting
	SetFlags(eExternalSort, external);

	// Indicate change in size of sorted message list - forces mailbox window update
	return mOpenInfo->mSortedMessages->size() != old_size;
}


// Remove sorted message
bool CMbox::SortRemove(CMessage* msg)
{
	if (!mOpenInfo)
		return false;

	CMessage* replace = NULL;
	bool removed_item = true;

	// Remove a message from the sorted list
	// This has to do special stuff of thread sorting only
	
	if (mOpenInfo->mSortBy == cSortMessageThread)
	{
		// Check thread status of message being removed
		CMessage* parent = const_cast<CMessage*>(msg->GetThreadParent());
		CMessage* child = const_cast<CMessage*>(msg->GetThreadChild());
		CMessage* previous = const_cast<CMessage*>(msg->GetThreadPrevious());
		CMessage* next = const_cast<CMessage*>(msg->GetThreadNext());
		
		// No child => nothing to promote or splice - just let it be removed
		if (!child)
		{
			// Reset parent or sibling node
			if (previous)
				previous->SetThreadNext(next);
			else if (parent)
				parent->SetThreadChild(next);
			if (next)
				next->SetThreadPrevious(previous);
		}

		// Single child or children below root => promote/splice child to parents place
		else if (parent || !child->GetThreadNext())
		{
			// Move cur's children into cur's place
			if (previous)
				previous->SetThreadNext(child);
			else if (parent)
				parent->SetThreadChild(child);

			// Make cur's first child point to previous
			child->SetThreadPrevious(previous);

			// Make cur's parent the new parent of cur's children
			do
			{
				child->ThreadPromote();
				child->SetThreadParent(parent);
			} while(child->GetThreadNext() && (child = const_cast<CMessage*>(child->GetThreadNext())));

			// Make the cur's last child point to cur's next sibling
			child->SetThreadNext(next);
			if (next)
				next->SetThreadPrevious(child);
		}
		
		// Must be at the root with multiple children
		else
		{
			// Create a fake
			replace = new CMessage((CMbox*) NULL);
			replace->MakeFake();

			// Insert in current's place
			replace->SetThreadInfo(0, NULL, child, previous, next);
			
			// Make sure others know about this change
			if (previous)
				previous->SetThreadNext(replace);
			if (next)
				next->SetThreadPrevious(replace);
			
			// Make cur's parent the new parent of cur's children
			do
			{
				child->SetThreadParent(replace);
			} while(child->GetThreadNext() && (child = const_cast<CMessage*>(child->GetThreadNext())));

			// Make sure its signalled as a replacement
			removed_item = false;
		}
	}
	
	if (removed_item)
	{
		// Check to see if parent is a fake and we're the only child
		// Also must not have children which will be promoted
		if (msg->GetThreadParent() &&
			msg->GetThreadParent()->IsFake() &&
			!msg->GetThreadPrevious() &&
			!msg->GetThreadNext() &&
			!msg->GetThreadChild())
		{
			// Remove the parent
			CMessage* parent = const_cast<CMessage*>(msg->GetThreadParent());
			mOpenInfo->mSortedMessages->RemoveMessage(parent);
			
			// Adjust parent siblings now that it is gone
			if (parent->GetThreadPrevious())
				parent->GetThreadPrevious()->SetThreadNext(parent->GetThreadNext());
			if (parent->GetThreadNext())
				parent->GetThreadNext()->SetThreadPrevious(parent->GetThreadPrevious());

			// Delete the parent
			delete parent;
		}
		mOpenInfo->mSortedMessages->RemoveMessage(msg);
	}
	else
		mOpenInfo->mSortedMessages->ReplaceMessage(msg, replace);
		
	return removed_item;
}

// Set sorting method
void CMbox::ShowBy(EShowMessageBy show_by)
{
	if (!mOpenInfo)
		return;

	bool resort = (mOpenInfo->mShowBy != show_by);
	mOpenInfo->mShowBy = show_by;
	if (resort)
		ReSort();
}

#pragma mark ____________________________Message Ops

bool CMbox::FullyCached() const
{
	// Depends on view mode
	if (IsFullOpen())
	{
		switch(mOpenInfo->mViewMode)
		{
		case eViewMode_All:
		case eViewMode_AllMatched:
			// See if number cached is same as total in mailbox
			return (GetNumberCached() == GetNumberFound());

		case eViewMode_ShowMatch:
			// Count number cached in sorted list
			for(CMessageList::const_iterator iter = mOpenInfo->mSortedMessages->begin(); iter != mOpenInfo->mSortedMessages->end(); iter++)
			{
				if (!(*iter)->IsFullyCached())
					return false;
			}
			return true;
		}
	}

	return true;
}

// Expunge
void CMbox::Expunge(bool closing)
{
	if (!IsFullOpen())
		return;

	// Do punt unseen action here if closing since the expunge action will actually close the mailbox here
	if (closing && HasAllowedFlag(NMessage::eSeen) && !mFlags.IsSet(eExamine) && mFlags.IsSet(ePuntOnClose))
		PuntUnseen();
	
	// Expunge it on server
	mOpenInfo->mMsgMailer->ExpungeMbox(this, closing);

	// If closing, must update message count
	if (closing)
	{
		unsigned long deleted_removed = 0;
		unsigned long unseen_removed = 0;

		// Count number of deleted and deleted and unseen messages
		for(CMessageList::const_iterator iter = mOpenInfo->mMessages->begin(); iter != mOpenInfo->mMessages->end(); iter++)
		{
			// If deleted increment counter
			if ((*iter)->IsDeleted())
			{
				deleted_removed++;
				if ((*iter)->IsUnseen())
					unseen_removed++;
			}
		}

		// Now adjust counters
		SetNumberFound(GetNumberFound() - deleted_removed);
		SetNumberUnseen(GetNumberUnseen() - unseen_removed);
	}

} // CMbox::Expunge

// Check
// NB This can be called by different thread
long CMbox::Check(bool fast, bool silent)
{
	// Resolve to open mailbox so that open connection is used
	CMbox* open_mbox = GetProtocol()->FindOpenMbox(GetName());
	if (open_mbox && (open_mbox != this))
	{
		// Get the open one to check itself using the open connection
		// That will update the status of this and any others
		return open_mbox->Check(fast, silent);
	}

	// Must create status info at this point
	InitStatusInfo();

	// Clear new recent pseudo flag on all messages
	ClearCheckRecent();
	SetCheckRecent(0);

	// Local mailboxes requiring locking here
	if (IsLocalMbox())
		CMbox::_smutex.acquire(this);

	// Check it on suitable server
	if (OpenIfOpen())
	{
		// Can now clear local lock as we have an open lock
		if (IsLocalMbox())
			CMbox::_smutex.release(this);
		
		// Have now grabbed a ref count on open state
		long result = 0;
		
		try
		{
			if (silent)
				mOpenInfo->mMsgMailer->SetNoErrorAlert(true);
			result = mOpenInfo->mMsgMailer->CheckMbox(this, fast);
			if (silent)
				mOpenInfo->mMsgMailer->SetNoErrorAlert(false);

			// Close to set ref count back
			Close();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			if (silent)
				mOpenInfo->mMsgMailer->SetNoErrorAlert(false);

			// Close to set ref count back
			Close();
			
			if (!silent)
			{
				CLOG_LOGRETHROW;
				throw;
			}
		}

		return result;
	}
	else
	{
		long result = 0;
		
		try
		{
			if (silent)
				mMailer->SetNoErrorAlert(true);
			result = mMailer->CheckMbox(this, fast);
			if (silent)
				mMailer->SetNoErrorAlert(false);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			if (silent)
				mMailer->SetNoErrorAlert(false);

			// Can now clear local lock after doing check
			if (IsLocalMbox())
				CMbox::_smutex.release(this);

			if (!silent)
			{
				CLOG_LOGRETHROW;
				throw;
			}
		}
		
		// Can now clear local lock after doing check
		if (IsLocalMbox())
			CMbox::_smutex.release(this);
		
		return result;
	}

} // CMbox::Check

void CMbox::ProcessCheck()
{
	// Don't do this whilst mailbox is being opened or synchronised
	if (!IsBeingOpened() && !IsSynchronising())
		GetProtocol()->ProcessCheckMbox(this);
}

// Get its size
void CMbox::CheckSize()
{
	// Must create status info at this point
	InitStatusInfo();

	// If its local then it must be cached
	if (IsLocalMbox() && !IsCachedMbox())
		return;

	// Check whether this is meaningful
	if (IsOpen() ? !mOpenInfo->mMsgMailer->DoesMailboxSize() : !mMailer->DoesMailboxSize())
	{
		// Fake check
		bool was_open = IsOpen();
		if (!was_open)
			Open(NULL, false, true);

		// Ensure that temp message list is deleted BEFORE the mailbox is closed
		{
			// Cache any uncached messages
			CMessageList cached;
			cached.SetOwnership(false);
			for(CMessageList::iterator iter = mOpenInfo->mMessages->begin(); iter != mOpenInfo->mMessages->end(); iter++)
			{
				if ((*iter)->CacheMessage())
					cached.push_back(*iter);
			}

			// Forcibly cache message size
			ulvector seq_all;
			seq_all.push_back(0);
			if (GetNumberFound())
				mOpenInfo->mMsgMailer->FetchItems(seq_all, false, CMboxProtocol::eSize);

			// Now get the total size of the mailbox
			unsigned long size = 0;
			for(CMessageList::iterator iter = mOpenInfo->mMessages->begin(); iter != mOpenInfo->mMessages->end(); iter++)
			{
				unsigned long msg_size = (*iter)->GetSize();
				if (0xFFFFFFFE - size < msg_size)
				{
					size = 0xFFFFFFFE;
					break;
				}

				size += msg_size;
			}

			// Tell mailbox the size
			SetSize(size);

			// Uncache any cached messages
			for(CMessageList::iterator iter = cached.begin(); iter != cached.end(); iter++)
				(*iter)->UncacheMessage();
		}

		// Close if previously open - don't bother punting again
		if (!was_open)
			CloseSilent();
	}
	else
	{
		// Do check on server
		if (IsOpen())
			mOpenInfo->mMsgMailer->MailboxSize(this);
		else
			mMailer->MailboxSize(this);
	}
}

void CMbox::ChangeUIDValidity(unsigned long uidv)
{
	SetUIDValidity(uidv);
	if (mOpenInfo)
		mOpenInfo->mMsgMailer->SetUIDValidity(uidv);
}

void CMbox::ChangeUIDNext(unsigned long uidn)
{
	SetUIDNext(uidn);
	if (mOpenInfo)
		mOpenInfo->mMsgMailer->SetUIDNext(uidn);
}

unsigned long CMbox::GetLastSync()
{
	// Determine last sync from disconnected mbox only if not local
	if (!IsLocalMbox() &&
		(!mStatusInfo || !mStatusInfo->mLastSync))
	{
		if (mOpenInfo)
			mOpenInfo->mMsgMailer->GetLastSync(this);
		else
			mMailer->GetLastSync(this);
	}

	return (mStatusInfo ? mStatusInfo->mLastSync : 0);
}

void CMbox::ChangeLastSync(unsigned long sync)
{
	SetLastSync(sync);
	if (mOpenInfo)
		mOpenInfo->mMsgMailer->SetLastSync(sync);
}

#pragma mark ____________________________Message Cache

unsigned long CMbox::GetLOCache() const
{
	return IsLocalMbox() ? CPreferences::sPrefs->mLLoCache.GetValue() : CPreferences::sPrefs->mRLoCache.GetValue();
}

void CMbox::SetLOCache(unsigned long value) const
{
	if (IsLocalMbox())
		CPreferences::sPrefs->mLLoCache.SetValue(value, false);
	else
		CPreferences::sPrefs->mRLoCache.SetValue(value, false);
}

unsigned long CMbox::GetHICache() const
{
	return IsLocalMbox() ? CPreferences::sPrefs->mLHiCache.GetValue() : CPreferences::sPrefs->mRHiCache.GetValue();
}

void CMbox::SetHICache(unsigned long value) const
{
	if (IsLocalMbox())
		CPreferences::sPrefs->mLHiCache.SetValue(value, false);
	else
		CPreferences::sPrefs->mRHiCache.SetValue(value, false);
}

bool CMbox::GetUseHICache() const
{
	return IsLocalMbox() ? CPreferences::sPrefs->mLUseHiCache.GetValue() : CPreferences::sPrefs->mRUseHiCache.GetValue();
}

unsigned long CMbox::GetSortCache() const
{
	return IsLocalMbox() ? CPreferences::sPrefs->mLSortCache.GetValue() : CPreferences::sPrefs->mRSortCache.GetValue();
}

// Force all messages into cache
void CMbox::CacheAllMessages()
{
	// Only if some to cache
	if (!GetNumberFound())
		return;

	// Do this by faking a cache load with parameters set to load the entire mailbox!

	// Save copies of cache parameters
	unsigned long lo_tide = GetLOCache();
	unsigned long hi_tide = GetHICache();
	unsigned long increment = GetCacheIncrement();

	// Set cache parameters to do entire cache
	SetLOCache(GetNumberFound());
	SetHICache(GetNumberFound());
	SetCacheIncrement(GetNumberFound());

	try
	{
		// Now bring all into cache
		CacheMessage(1);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Reset cache parameters
		SetLOCache(lo_tide);
		SetHICache(hi_tide);
		
		// Only if still open
		SetCacheIncrement(increment);
		
		CLOG_LOGRETHROW;
		throw;
	}

	// Reset cache parameters
	SetLOCache(lo_tide);
	SetHICache(hi_tide);
	SetCacheIncrement(increment);
}

// Force message into cache
void CMbox::CacheMessage(unsigned long msg_num,	unsigned long count)
{
	if (!IsFullOpen())
		return;

	// Check whether it exists already
	CMessage* aMsg = GetMessage(msg_num);

	// Reverse lookup sorted number
	unsigned long start_msg = std::find(mOpenInfo->mSortedMessages->begin(), mOpenInfo->mSortedMessages->end(), aMsg) -
								mOpenInfo->mSortedMessages->begin() + 1;

	unsigned long lo_tide = GetLOCache();
	unsigned long hi_tide = GetHICache();
	unsigned long increment = GetCacheIncrement();
	if (increment == 0)
		increment = IsLocalMbox() ? CPreferences::sPrefs->mLCacheIncrement.GetValue() : CPreferences::sPrefs->mRCacheIncrement.GetValue();
	if (increment == 0)
		increment = IsLocalMbox() ? 50 : 20;

	// How many do we want to cache?
	// Want to have at least lo-tide, at most hi-tide
	unsigned long num_cached = GetNumberCached();
	unsigned long num_to_cache = 0;
	unsigned long cache_end = mOpenInfo->mSortedMessages->size();

	// Use auto count if present
	if (count)
		num_to_cache = count;
	else if (num_cached < lo_tide)
		num_to_cache = std::max(increment, lo_tide - num_cached);
	else
		num_to_cache = increment;

	// Adjust starting position
	bool backward_adjust = false;
	if (!count && (start_msg + num_to_cache - 1 > cache_end))
	{
		if (cache_end > num_to_cache)
			start_msg = cache_end - num_to_cache + 1;
		else
			start_msg = 1;
		backward_adjust = true;
	}

	// Determine range for caching - use sorted messages
	ulvector fetch_nums;
	ulvector sorted_nums;
	fetch_nums.reserve(num_to_cache);	// Pre-allocate in case of large mbox
	sorted_nums.reserve(num_to_cache);	// Pre-allocate in case of large mbox
	for(unsigned long num = start_msg; (num < start_msg + num_to_cache) && (num <= cache_end); num++)
	{
		// Check if message already cached and add to list if not
		CMessage* aMsg = GetMessage(num, true);
		if (aMsg->CacheMessage())
		{
			fetch_nums.push_back(aMsg->GetMessageNumber());
			sorted_nums.push_back(num);
		}
	}

	// Special may be scrolling back, in which case fetch_nums contains only one
	if (!backward_adjust && !count && (fetch_nums.size() == 1))
	{
		// Look backwards
		for(unsigned long num = start_msg - 1; (num > start_msg - num_to_cache) && (num > 0); num--)
		{
			// Check if message already cached and add to list if not
			CMessage* aMsg = GetMessage(num, true);
			if (aMsg->CacheMessage())
			{
				fetch_nums.insert(fetch_nums.begin(), aMsg->GetMessageNumber());
				sorted_nums.insert(sorted_nums.begin(), num);
			}
		}
	}

	// Boo! Can only uncache if NOT sorted by message number. Otherwise all hell breaks loose
	// since the uncached messages are sorted out of their original positions causing the
	// mailbox window to jump between caching alternate sets of messages.

	// Now check to see whether caching this number will cause overflow
	if ((GetSortBy() == cSortMessageNumber) &&
		GetUseHICache() &&
		hi_tide && (fetch_nums.size() + num_cached > hi_tide))
	{
		// Determine number to remove
		unsigned long num_to_remove = fetch_nums.size() + num_cached - hi_tide;

		// First uncache messages in open list but not in sorted list
		if (mOpenInfo->mMessages->size() > mOpenInfo->mSortedMessages->size())
		{
			ulvector full;
			ulvector sorted;
			ulvector difference;

			// Generate full list
			full.reserve(mOpenInfo->mMessages->size());
			for(unsigned long i = 1; i <= mOpenInfo->mMessages->size(); i++)
				full.push_back(i);

			sorted.reserve(mOpenInfo->mSortedMessages->size());
			for(unsigned long i = 0; i < mOpenInfo->mSortedMessages->size(); i++)
				sorted.push_back(mOpenInfo->mSortedMessages->at(i)->GetMessageNumber());
			std::sort(sorted.begin(), sorted.end());

			// Get difference between sorted and full list
			difference.insert(difference.begin(), mOpenInfo->mMessages->size() - mOpenInfo->mSortedMessages->size(), 0);
			std::set_difference(full.begin(),
								full.end(),
								sorted.begin(),
								sorted.end(),
								difference.begin());

			// Remove any in full but not in sorted that are cached
			for(ulvector::iterator iter = difference.begin();
					num_to_remove && (iter != difference.end()); iter++)
			{
				if (mOpenInfo->mMessages->at(*iter)->IsCached())
				{
					UncacheMessage(*iter);
					num_to_remove--;
				}
			}
		}

		// Only bother if more to be removed
		if (num_to_remove)
		{
			// Determine cached messages furthest away from new set
			typedef std::multimap<long, unsigned long> mmapulul;
			mmapulul mmap;
			long average = (sorted_nums.back() + sorted_nums.front()) / 2;
			unsigned long index = 1;
			for(CMessageList::const_iterator iter = mOpenInfo->mSortedMessages->begin(); iter != mOpenInfo->mSortedMessages->end(); iter++, index++)
			{
				if ((*iter)->IsCached())
					mmap.insert(mmapulul::value_type(::abs(average - (long) index), (*iter)->GetMessageNumber()));
			}

			// Now remove required number from end of map
			for(mmapulul::reverse_iterator riter = mmap.rbegin(); (num_to_remove > 0) && (riter != mmap.rend()); riter++, num_to_remove--)
				UncacheMessage((*riter).second);
		}
	}

	// Now reload
	if (fetch_nums.size())
	{
		try
		{
			mOpenInfo->mMsgMailer->FetchItems(fetch_nums, false, CMboxProtocol::eSummary);
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);

			// Recover from failure to cache
			if (IsFullOpen())
			{
				for(ulvector::const_iterator iter = fetch_nums.begin(); iter != fetch_nums.end(); iter++)
				{
					CMessage* msg = GetMessage(*iter);
					if (msg)
						msg->ValidateCache();
				}
			}

			CLOG_LOGRETHROW;
			throw;
		}
		mOpenInfo->mSortedMessages->SortDirty();

		// Resort if local sorting
		if (!IsExternalSort())
			SortMessages();
	}

} // CMbox::CacheMessage

// Force message into cache
void CMbox::CacheMessage(const ulvector& nums, bool uids)
{
	ulvector actual_nums;
	
	// Only bother with ones that are not cached
	for(ulvector::const_iterator iter = nums.begin(); iter != nums.end(); iter++)
	{
		CMessage* msg = uids ? GetMessageUID(*iter) : GetMessage(*iter);
		if (!msg || msg->CacheMessage())
			actual_nums.push_back(*iter);
	}

	// Now reload
	if (actual_nums.size())
	{
		try
		{
			mOpenInfo->mMsgMailer->FetchItems(actual_nums, uids, CMboxProtocol::eSummary);
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);

			// Recover from failure to cache
			for(ulvector::const_iterator iter = actual_nums.begin(); iter != actual_nums.end(); iter++)
			{
				CMessage* msg = GetMessage(*iter);
				if (msg)
					msg->ValidateCache();
			}
			
			CLOG_LOGRETHROW;
			throw;
		}
	}
}

// Force message into cache
void CMbox::CacheUIDs(const ulvector& nums)
{
	// Must have message list
	if (!mOpenInfo)
		return;

	mOpenInfo->mMsgMailer->FetchItems(nums, false, CMboxProtocol::eUID);
}

// Force messages from cache
void CMbox::UncacheMessages()
{
	// Must have message list
	if (!IsFullOpen())
		return;

	// Remove all windows
	for(CMessageList::const_iterator iter = mOpenInfo->mMessages->begin(); iter != mOpenInfo->mMessages->end(); iter++)
	{
		CMailControl::MessageRemoved(*iter);

		// Now uncache it
		(*iter)->UncacheMessage();
	}

} // CMbox::UncacheMessages

// Force a message from cache
void CMbox::UncacheMessage(unsigned long msg_num)
{
	// Must have message list
	if (!IsFullOpen())
		return;

	CMessage* aMsg = mOpenInfo->mMessages->at(msg_num - 1);
	if (aMsg->IsCached())
	{
		// Must remove any window
		CMailControl::MessageRemoved(aMsg);
		aMsg->UncacheMessage();
	}

} // CMbox::UncacheMessage

// Load messages from server
void CMbox::LoadMessages()
{
	// Only do if fully open
	if (!IsFullOpen())
		return;

	// Determine number to add
	long diff = mStatusInfo->mNumberExists - mOpenInfo->mMessages->size();

	// Determine whether initial load or load due to check
	bool check_load = (mOpenInfo->mFirstNew != ULONG_MAX);

	// Always add to more recent count
	if (check_load)
	{
		SetCheckRecent(GetCheckRecent() + diff);
		SetMboxRecent(GetMboxRecent() + diff);
	}

	// Determine whether new messages will match if matching
	if (GetViewMode() == eViewMode_ShowMatch)
	{
		// Update match for NEW items only
		// Do not allow removal of existing match items if they no longer match

		// Create enough space for incoming messages
		mOpenInfo->mMessages->reserve(mOpenInfo->mMessages->size() + diff);
		
		// These will be cached when required
		unsigned long first_new = mOpenInfo->mMessages->size() + 1;
		unsigned long index = first_new;
		ulvector new_nums;
		new_nums.reserve(diff);
		for(long i = 0; i < diff; i++)
		{
			CMessage* aMsg = CreateMessage();
			aMsg->SetMessageNumber(index);
			new_nums.push_back(index);
			mOpenInfo->mMessages->push_back(aMsg);

			// If new arrival mark with flags
			if (check_load)
				aMsg->GetFlags().Set(NMessage::eCheckRecent | NMessage::eMboxRecent, true);
		}

		// See if current search item is set to All => Last Search Results
		if (mOpenInfo->mViewCurrent.GetType() != CSearchItem::eAll)
		{
			// Now update match on new set
			CSearchItemList* item_list = new CSearchItemList;
			item_list->push_back(new CSearchItem(CSearchItem::eNumber, new_nums));
			item_list->push_back(new CSearchItem(mOpenInfo->mViewCurrent));

			// Create the new combined spec
			CSearchItem new_spec(CSearchItem::eAnd, item_list);

			// Now redo view match as an update
			SetViewSearch(&new_spec, true);
		}
	}
	else
	{
		// Determine if new messages should automatically be cached
		bool auto_cache = false;
		unsigned long cache_count = diff;
		if (FullyCached())
			auto_cache = true;
		else if (!mOpenInfo->mMessages->size())
		{
			auto_cache = !mFlags.IsSet(eBeingOpened);
			cache_count = 0;
		}
		else if (mOpenInfo->mMessages->size() < 10)
			auto_cache = true;
		else
		{
			// Look at last few messages, cache if they are also cached
			long weight = 0;
			long ctr = 5;
			for(CMessageList::const_reverse_iterator riter = static_cast<const CMessageList*>(mOpenInfo->mMessages)->rbegin();
				ctr && (riter != static_cast<const CMessageList*>(mOpenInfo->mMessages)->rend()); riter++, ctr--)
			{
				if ((*riter)->IsFullyCached())
					weight += ctr;
			}

			if (weight > 7)
				auto_cache = true;
		}

		// Turn off caching if flag is not set
		auto_cache = auto_cache && !IsNoCache();

		// These will be cached when required
		if (diff > 0)
		{
			// Create enough space for incoming messages
			mOpenInfo->mMessages->reserve(mOpenInfo->mMessages->size() + diff);
			mOpenInfo->mSortedMessages->reserve(mOpenInfo->mSortedMessages->size() + diff);
			
			unsigned long first_new = mOpenInfo->mMessages->size() + 1;
			unsigned long index = first_new;
			for(long i = 0; i < diff; i++, index++)
			{
				CMessage* aMsg = CreateMessage();
				aMsg->SetMessageNumber(index);
				mOpenInfo->mMessages->push_back(aMsg);
				mOpenInfo->mSortedMessages->push_back(aMsg);
				mOpenInfo->mSortedMessages->SortDirty();

				// If new arrival mark with flags
				if (check_load)
					aMsg->GetFlags().Set(NMessage::eCheckRecent | NMessage::eMboxRecent, true);
			}

			// Force full message caching here if required by client-side sort
			if (!ValidSort(mOpenInfo->mSortBy))
				CacheAllMessages();

			// Otherwise only bring into cache if required
			else if (auto_cache)
				CacheMessage(first_new, cache_count);
			
			// Must do a search to check for unseen/deleted counts when load caused by check
			else if (check_load)
				SearchCount(first_new, diff);
		}
	}
}

// Search messages for unseen/delete
void CMbox::SearchCount(unsigned long start, unsigned long ctr)
{
	// For now ignore inputs and do entire mailbox search.
	// Eventually this should search only the new messages and add to the existing counts.

	// Unseen search
	{
		CSearchItem search_spec(CSearchItem::eUnseen);
		ulvector results;
		Search(&search_spec, &results, false, true);

		// Poke into messages
		for(ulvector::const_iterator iter = results.begin(); iter != results.end(); iter++)
			GetMessage(*iter)->GetFlags().Set(NMessage::eSeen, false);

		// Update internal counter
		SetNumberUnseen(results.size());
	}

	// Deleted search
	{
		CSearchItem search_spec(CSearchItem::eDeleted);
		ulvector results;
		Search(&search_spec, &results, false, true);

		// Poke into messages
		for(ulvector::const_iterator iter = results.begin(); iter != results.end(); iter++)
			GetMessage(*iter)->GetFlags().Set(NMessage::eDeleted, true);
	}
}

// Load all references headers (needed for local threading)
void CMbox::LoadReferences()
{
	if (!IsFullOpen())
		return;

	// Assume messages are already cached (valid if local sorting)
	// Find ones without references alreday cached
	ulvector nums;
	for(CMessageList::const_iterator iter = mOpenInfo->mMessages->begin(); iter != mOpenInfo->mMessages->end(); iter++)
	{
		if ((*iter)->GetEnvelope() && !(*iter)->GetEnvelope()->HasReferences())
		{
			if ((*iter)->HasHeader())
			{
				// Extract from full header
				cdstring temp;
				(*iter)->GetHeaderField("References", temp);
				(*iter)->GetEnvelope()->SetReferences(temp);
			}
			else
				nums.push_back((*iter)->GetMessageNumber());
		}
	}
	
	// Don't bother if nothing to read in
	if (nums.empty())
		return;

	// Read in appropriate header
	mOpenInfo->mMsgMailer->ReadHeaders(nums, false, "References");
	
	// Transfer to references cached value
	for(CMessageList::const_iterator iter = mOpenInfo->mMessages->begin(); iter != mOpenInfo->mMessages->end(); iter++)
	{
		if ((*iter)->GetEnvelope() && !(*iter)->GetEnvelope()->HasReferences())
		{
			// Extract from partial header
			cdstring temp;
			(*iter)->GetHeaderField("References", temp);
			(*iter)->GetEnvelope()->SetReferences(temp);
			
			// Wipe out header which we don't need
			(*iter)->SetHeader(NULL);
		}
	}
}

#pragma mark ____________________________Message Operations

// Start multiple append
void CMbox::StartAppend()
{
	// Append message on appropriate server
	if (IsOpen())
		mOpenInfo->mMsgMailer->StartAppend(this);
	else
		mMailer->StartAppend(this);
}

// Stop multiple append
void CMbox::StopAppend()
{
	// Append message on appropriate server
	if (IsOpen())
		mOpenInfo->mMsgMailer->StopAppend(this);
	else
		mMailer->StopAppend(this);
}

// Append the specified message on the server
void CMbox::AppendMessage(CMessage* msg, unsigned long& new_uid, bool dummy_files, bool doMRU)
{
	// Grab open lock on mailbox
	bool did_open = OpenIfOpen();

	// Check to see if open connection should be used
	bool append_open = did_open && (mMailer->IsOffline() || !msg->GetBody()->TestOwnership(this));

	// Append message on appropriate server
	try
	{
		if (append_open)
		{
			mOpenInfo->mMsgMailer->AppendMbox(this, msg, new_uid, dummy_files, doMRU);

			// Force fast check to bring into cache
			if (!IsSynchronising())
				Check(true);
		}
		else
		{
			// Check that it is logged in before doing this
			// Logon to server - must be successful
			if (!mMailer->IsLoggedOn())
			{
				CMailAccountManager::sMailAccountManager->StartProtocol(mMailer);

				if (!mMailer->IsLoggedOn())
				{
					// Must throw so that any additional actions are cancelled
					CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}
			}

			mMailer->AppendMbox(this, msg, new_uid, dummy_files, doMRU);
		}
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
		
		// Now close mailbox if we had a lock
		if (did_open)
			Close();

		CLOG_LOGRETHROW;
		throw;		
	}

	// Now close mailbox if we had a lock
	if (did_open)
		Close();

	// No longer do this - after draft append message is always deleted
	// After append to SMTP queue message must remain intact
#if 0
	// Clear message cache as its unlikely to be needed after append
	msg->ClearMessage();
#endif

} // CMbox::AppendMessage

// Get the requested message
CMessage* CMbox::GetMessage(unsigned long msg_num, bool sorted)
{
	CMessage* theMsg = NULL;
	if (IsFullOpen())
	{
		if (sorted)
			theMsg = mOpenInfo->mSortedMessages->at(msg_num - 1);
		else
			theMsg = mOpenInfo->mMessages->at(msg_num - 1);
	}
	return theMsg;

} // CMbox::GetMessage

bool msg_uid_compare(const CMessage* msg1, const CMessage* msg2);
bool msg_uid_compare(const CMessage* msg1, const CMessage* msg2)
{
	return msg1->GetUID() < msg2->GetUID();
}

// Get the requested message
CMessage* CMbox::GetMessageUID(unsigned long uid)
{
	if (!IsFullOpen())
		return NULL;

	// Have to do this the hard way because UIDs may not all be cached
	for(CMessageList::iterator iter = mOpenInfo->mMessages->begin(); iter != mOpenInfo->mMessages->end(); iter++)
	{
		if ((*iter)->GetUID() == uid)
			return *iter;
		else if ((*iter)->GetUID() > uid)
			break;
	}
	
	return NULL;
}

// Get the requested message
CMessage* CMbox::GetMessageLocalUID(unsigned long uid)
{
	if (!mOpenInfo)
		return NULL;

	unsigned long msg_index = mOpenInfo->mMsgMailer->GetMessageLocalUID(uid);
	
	if (msg_index)
		return GetMessage(msg_index);
	else
		return NULL;
}

// Get the requested message and force into cache
CMessage* CMbox::GetCacheMessage(unsigned long msg_num, bool sorted)
{
	if (!IsFullOpen())
		return NULL;

	// Get it from lists
	CMessage* theMsg = NULL;
	if (sorted)
	{
		if (msg_num - 1 < mOpenInfo->mSortedMessages->size())
			theMsg = mOpenInfo->mSortedMessages->at(msg_num - 1);
	}
	else
		theMsg = mOpenInfo->mMessages->at(msg_num - 1);

	// Try caching (use actual message num)
	if (!theMsg->GetEnvelope())
		CacheMessage(theMsg->GetMessageNumber());

	return theMsg;

} // CMbox::GetMessage

// Get previous message
CMessage* CMbox::GetPrevMessage(CMessage* aMsg, bool sorted, bool cache)
{
	if (!IsFullOpen())
		return NULL;

	CMessage* msg = (sorted) ?
						mOpenInfo->mSortedMessages->GetPrevMessage(aMsg) :
						mOpenInfo->mMessages->GetPrevMessage(aMsg);

	// Try caching (use actual message num)
	if (cache && msg && !msg->GetEnvelope())
		CacheMessage(msg->GetMessageNumber());

	return msg;
}

// Get next message
CMessage* CMbox::GetNextMessage(CMessage* aMsg, bool sorted, bool cache)
{
	if (!IsFullOpen())
		return NULL;

	CMessage* msg = (sorted) ?
						mOpenInfo->mSortedMessages->GetNextMessage(aMsg) :
						mOpenInfo->mMessages->GetNextMessage(aMsg);

	// Try caching (use actual message num)
	if (cache && msg && !msg->GetEnvelope())
		CacheMessage(msg->GetMessageNumber());

	return msg;
}

// Get next message with flag set/unset
CMessage* CMbox::GetNextFlagMessage(CMessage* aMsg, NMessage::EFlags set_flag, NMessage::EFlags unset_flag, bool sorted, bool ascending_seq, bool cache)
{
	if (!IsFullOpen())
		return NULL;

	CMessageList* list = sorted ? mOpenInfo->mSortedMessages : mOpenInfo->mMessages;
	bool must_search = false;
	bool forward = ascending_seq;

	// Special case for Recent, Seen and Deleted flag processing
	// Those flags are automatically loaded when the mailbox is
	// opened but the message is not marked as cached.
	// We can avoid an extra search on open and when tabbing to a new message with the following code
	bool need_cache = true;
	if (!(set_flag & ~(NMessage::eRecent | NMessage::eSeen | NMessage::eDeleted)) &&
		!(unset_flag & ~(NMessage::eRecent | NMessage::eSeen | NMessage::eDeleted)) &&
		IsFullOpen())
		need_cache = false;

	// Get index of starting message
	unsigned long index = list->FetchIndexOf(aMsg);

	// First scan existing cached messages for a match
	while(true)
	{
		// Get next message (suing its index) in sorted order
		if (forward)
			index = list->GetNextMessageIndex(index);
		else
			index = list->GetPrevMessageIndex(index);

		aMsg = (index > 0) ? list->at(index - 1) : NULL;

		if (!aMsg)
			break;

		// If its not cached, force into search mode
		if (need_cache && !aMsg->IsFullyCached())
		{
			must_search = true;
			break;
		}

		// Do flag test
		if (((set_flag == NMessage::eNone) || aMsg->GetFlags().IsSet(set_flag)) &&
			((unset_flag == NMessage::eNone) || aMsg->GetFlags().IsUnset(unset_flag)))
			break;
	}

	if (must_search)
	{
		// Search for messages with matching flags from the one choosen one onwards/backwards
		unsigned long sorted_index = GetMessageIndex(aMsg, sorted);
		ulvector sorted_sequence;
		ulvector results;

		// Create search item list and main search item
		CSearchItemList* search_list = new CSearchItemList;
		CSearchItem search_spec(CSearchItem::eAnd, search_list);

		// First create message number range
		{
			// Generate sequence of current message to last message, in sorted order
			sorted_sequence.reserve(list->size() - sorted_index + 1);
			for(unsigned long i = sorted_index; i <= list->size(); i++)
				sorted_sequence.push_back(list->at(i - 1)->GetMessageNumber());

			// Add sequence list
			search_list->push_back(new CSearchItem(CSearchItem::eNumber, sorted_sequence));
		}

		// Only bother if something left to search
		if (sorted_sequence.size())
		{
			// Now add list of flags that are set
			if (set_flag != NMessage::eNone)
			{
				if (set_flag & NMessage::eRecent)
					search_list->push_back(new CSearchItem(CSearchItem::eRecent));
				if (set_flag & NMessage::eAnswered)
					search_list->push_back(new CSearchItem(CSearchItem::eAnswered));
				if (set_flag & NMessage::eFlagged)
					search_list->push_back(new CSearchItem(CSearchItem::eFlagged));
				if (set_flag & NMessage::eDeleted)
					search_list->push_back(new CSearchItem(CSearchItem::eDeleted));
				if (set_flag & NMessage::eSeen)
					search_list->push_back(new CSearchItem(CSearchItem::eSeen));
				if (set_flag & NMessage::eDraft)
					search_list->push_back(new CSearchItem(CSearchItem::eDraft));
			}

			// Now add list of flags that are not set
			if (unset_flag != NMessage::eNone)
			{
				if (unset_flag & NMessage::eRecent)
					search_list->push_back(new CSearchItem(CSearchItem::eOld));
				if (unset_flag & NMessage::eAnswered)
					search_list->push_back(new CSearchItem(CSearchItem::eUnanswered));
				if (unset_flag & NMessage::eFlagged)
					search_list->push_back(new CSearchItem(CSearchItem::eUnflagged));
				if (unset_flag & NMessage::eDeleted)
					search_list->push_back(new CSearchItem(CSearchItem::eUndeleted));
				if (unset_flag & NMessage::eSeen)
					search_list->push_back(new CSearchItem(CSearchItem::eUnseen));
				if (unset_flag & NMessage::eDraft)
					search_list->push_back(new CSearchItem(CSearchItem::eUndraft));
			}

			Search(&search_spec, &results, false, true);
		}

		// If any results, get the first one (results have been sorted)
		if (results.size())
		{
			// Now search for first item in results that matches a sorted_sequence item
			aMsg = NULL;
			for(ulvector::const_iterator iter = sorted_sequence.begin(); !aMsg && (iter != sorted_sequence.end()); iter++)
			{
				if (std::binary_search(results.begin(), results.end(), *iter))
					aMsg = GetMessage(*iter);
			}
		}
		else
			aMsg = NULL;
	}

	// Try caching (use actual message num)
	if (cache && aMsg && !aMsg->GetEnvelope())
		CacheMessage(aMsg->GetMessageNumber());

	return aMsg;
}

// Read the specified message cache
void CMbox::ReadCache(CMessage* msg)
{
	ulvector nums;
	nums.push_back(msg->GetMessageNumber());

	// Read message size from server
	mOpenInfo->mMsgMailer->FetchItems(nums, false, CMboxProtocol::eSummary);

} // CMbox::ReadHeader

// Read the specified message size
void CMbox::ReadSize(CMessage* msg)
{
	// Read message size from server
	ulvector nums;
	nums.push_back(msg->GetMessageNumber());
	mOpenInfo->mMsgMailer->FetchItems(nums, false, CMboxProtocol::eSize);

} // CMbox::ReadHeader

// Read the specified message header
void CMbox::ReadHeader(unsigned long msg_num, bool sorted)
{
	ReadHeader(GetMessage(msg_num, sorted));

} // CMbox::ReadHeader

// Read the specified message header
void CMbox::ReadHeader(CMessage* msg)
{
	if (!msg->HasHeader())
		mOpenInfo->mMsgMailer->ReadHeader(this, msg);

} // CMbox::ReadHeader

// Read the specified message attachment
void CMbox::ReadAttachment(unsigned long msg_num, CAttachment* attach, LStream* aStream, bool peek, bool sorted)
{
	if (sorted)
	{
		CMessage* theMsg = GetMessage(msg_num, true);
		ReadAttachment(theMsg, attach, aStream, peek);
	}
	else
	{
		// Read message from server if not already cached
		CMessage* theMsg = GetMessage(msg_num);
		mOpenInfo->mMsgMailer->ReadAttachment(this, msg_num, attach, aStream, peek);
	}

} // CMbox::ReadAttachment

// Read the specified message header
void CMbox::ReadAttachment(CMessage* msg, CAttachment* attach, LStream* aStream, bool peek)
{
	unsigned long msg_num = GetMessageIndex(msg);
	if (msg_num)
		ReadAttachment(msg_num, attach, aStream, peek);

} // CMbox::ReadAttachment

// Read the specified message attachment
void CMbox::CopyAttachment(unsigned long msg_num, CAttachment* attach, costream* aStream, bool peek, unsigned long count, unsigned long start, bool sorted)
{
	if (sorted)
	{
		CMessage* theMsg = GetMessage(msg_num, true);
		CopyAttachment(theMsg, attach, aStream, peek, count, start);
	}
	else
	{
		// Read message from server
		mOpenInfo->mMsgMailer->CopyAttachment(this, msg_num, attach, *aStream, peek, count, start);
	}

}

// Read the specified message header
void CMbox::CopyAttachment(CMessage* msg, CAttachment* attach, costream* aStream, bool peek, unsigned long count, unsigned long start)
{
	// Iterate up with a submessage
	if (msg->IsSubMessage())
		// Copy the attachment
		CopyAttachment(msg->GetOwner(), attach ? attach : msg->GetBody()->GetParent(), aStream, peek, count, start);
	else
	{
		unsigned long msg_num = GetMessageIndex(msg);
		if (msg_num)
			CopyAttachment(msg_num, attach, aStream, peek, count, start);
	}
}

// Remove the specified message from mailbox
void CMbox::RemoveMessage(unsigned long msg_num)
{
	// Determine message or missing status
	CMessage* theMsg = NULL;
	bool new_expunge = false;
	if ((msg_num <= GetNumberFound()) && (msg_num > mOpenInfo->mMessages->size()))		// test for newly arrived message that is not yet cached
		new_expunge = true;
	else
		theMsg = GetMessage(msg_num);

	// Adjust flag counters
	if (theMsg)
	{
		if (IsFullOpen() && theMsg->IsMboxRecent())
			mOpenInfo->mMboxRecent--;
		if (IsFullOpen() && theMsg->IsCheckRecent())
			mOpenInfo->mCheckRecent--;
#if 0	// NO! Recent count is controlled solely by server responses - do NOT adjust elsewhere
		if (theMsg->IsRecent())
			mStatusInfo->mNumberRecent--;
#endif
		if (theMsg->IsUnseen())
			mStatusInfo->mNumberUnseen--;
	}
	mStatusInfo->mNumberExists--;

	// Remove from sorted list BEFORE server list to ensure correct number
	if (theMsg)
	{
		if (SortRemove(theMsg))
			// Remove any GUI message
			CMailControl::MessageRemoved(theMsg);
		else
		{
			// Always remove any GUI message
			CMailControl::MessageRemoved(theMsg);

			// Refresh any GUI message - but note its been replaced
			CMailControl::MessageChanged(theMsg, true);
		}
	}
	if (IsFullOpen() && !new_expunge)
		mOpenInfo->mMessages->erase(mOpenInfo->mMessages->begin() + (msg_num - 1));

	delete theMsg;

	// Must adjust search results
	for(ulvector::iterator iter = mStatusInfo->mSearchResults.begin(); iter != mStatusInfo->mSearchResults.end(); )
	{
		if (*iter > msg_num)
			*iter = *iter - 1;
		else if (*iter == msg_num)
		{
			iter = mStatusInfo->mSearchResults.erase(iter);
			continue;
		}
		
		iter++;
	}
	for(ulvector::iterator iter = mOpenInfo->mViewSearchResults.begin(); iter != mOpenInfo->mViewSearchResults.end(); )
	{
		if (*iter > msg_num)
			*iter = *iter - 1;
		else if (*iter == msg_num)
		{
			iter = mOpenInfo->mViewSearchResults.erase(iter);
			continue;
		}
		
		iter++;
	}

} // CMbox::RemoveMessage

// Remove the specified message from mailbox
void CMbox::RemoveMessageUID(unsigned long uid)
{
	// Adjust flag counters
	CMessage* theMsg = GetMessageUID(uid);
	if (theMsg)
	{
		if (IsFullOpen() && theMsg->IsMboxRecent())
			mOpenInfo->mMboxRecent--;
		if (IsFullOpen() && theMsg->IsCheckRecent())
			mOpenInfo->mCheckRecent--;
#if 0	// NO! Recent count is controlled solely by server responses - do NOT adjust elsewhere
		if (theMsg->IsRecent())
			mStatusInfo->mNumberRecent--;
#endif
		if (theMsg->IsUnseen())
			mStatusInfo->mNumberUnseen--;
	}
	mStatusInfo->mNumberExists--;

	// Remove any GUI message
	CMailControl::MessageRemoved(theMsg);

	// Remove from sorted list BEFORE server list to ensure correct number
	if (IsFullOpen())
	{
		if (theMsg)
			mOpenInfo->mSortedMessages->RemoveMessage(theMsg);
		mOpenInfo->mMessages->RemoveMessage(theMsg);
	}

	delete theMsg;
}

// Change the specified message uid on the server
void CMbox::RemapUID(unsigned long local_uid, unsigned long new_uid)
{
	mOpenInfo->mMsgMailer->RemapUID(this, local_uid, new_uid);
}

// Map from local to remote uids
void CMbox::MapLocalUIDs(const ulvector& uids, ulvector* missing, ulmap* local_map)
{
	mOpenInfo->mMsgMailer->MapLocalUIDs(this, uids, missing, local_map);
}

// Read the specified message flags
void CMbox::ReadMessageFlags(CMessage* msg)
{
	// Read message flags from server
	ulvector nums;
	nums.push_back(msg->GetMessageNumber());
	mOpenInfo->mMsgMailer->FetchItems(nums, false, CMboxProtocol::eFlags);

} // CMbox::ReadMessageFlags

// Change the specified message flags
void CMbox::SetFlagMessage(unsigned long msg_num, bool uids, NMessage::EFlags flags, bool set)
{
	ulvector nums(1, msg_num);

	SetFlagMessage(nums, uids, flags, set);

} // CMbox::SetFlagMessage

// Change the specified message flags
void CMbox::SetFlagMessage(const ulvector& nums, bool uids, NMessage::EFlags flags, bool set, bool sorted)
{
	// Map sorted nums to actual nums
	ulvector actual_nums;
	MapSorted(actual_nums, nums, !uids && sorted);

	// Can only change permanents
	flags = (NMessage::EFlags) (flags & GetAllowedFlags());

	// Only bother if something to set
	if (flags)
	{
		// Change flags on server
		mOpenInfo->mMsgMailer->SetFlagMessage(this, actual_nums, uids, flags, set);

		// Resort if required (only do when fully open - might be unseen punting on closed mailbox)
		if (IsFullOpen() && (mOpenInfo->mSortBy == cSortMessageFlags))
			ReSort();
	}

} // CMbox::SetFlagMessage

// Find messages with matching flags set/unset
void CMbox::MatchMessageFlags(NMessage::EFlags set_flag, NMessage::EFlags unset_flag, ulvector& matched, bool uids, bool sorted) const
{
	// If all messages cached then do search of in-memory cache
	// Also if doing a search for the Match flag, only
	if (IsFullOpen() &&
		((GetNumberCached() == GetNumberFound()) ||
		 (set_flag & NMessage::eSearch) ||
		 (unset_flag & NMessage::eSearch)))
	{
		// Do to each message
		unsigned long index = 1;
		const CMessageList* msg_list = (sorted ? mOpenInfo->mSortedMessages : mOpenInfo->mMessages);
		for(CMessageList::const_iterator iter = msg_list->begin(); iter != msg_list->end(); iter++, index++)
		{
			if (((set_flag == NMessage::eNone) || (*iter)->GetFlags().IsSet(set_flag)) &&
				((unset_flag == NMessage::eNone) || !(*iter)->GetFlags().IsSet(unset_flag)))
				matched.push_back(uids ? (*iter)->GetUID() : index);
		}
	}

	// Some messages are not cached => do server search for flags
	else if (IsFullOpen())
	{
		// Do search for specified flags
		CSearchItem search_spec(set_flag, unset_flag);
		const_cast<CMbox*>(this)->Search(&search_spec, &matched, uids, true);

		// Must map to sorted positions
		if (!uids && sorted)
		{
			// Have to do brute force inverse lookup of sorted messages!
			for(ulvector::iterator iter1 = matched.begin(); iter1 != matched.end(); iter1++)
			{
				// Replace unsorted index by sorted index (may be zero if message not in sorted view)
				const CMessage* msg = mOpenInfo->mMessages->at(*iter1 - 1);
				*iter1 = mOpenInfo->mSortedMessages->FetchIndexOf(msg);
			}

			// Remove any missing message indexes from sorted view
			unsigned long temp = 0;
			ulvector::iterator erase_start = std::remove_copy(matched.begin(), matched.end(), matched.begin(), temp);
			if (erase_start != matched.end())
				matched.erase(erase_start, matched.end());
		}
	}
}

// Punt unseen messages
void CMbox::PuntUnseen()
{
	// Check open status
	bool was_open = IsOpen();

	// Open it silently if not already (do not open fully)
	if (!was_open)
		Open(NULL, false, false, false);

	// Determine unseen messages

	// Do search for unseen
	CSearchItem search_spec(CSearchItem::eUnseen);
	ulvector unseens;
	Search(&search_spec, &unseens, false, true);

	// Set seen flag on all these
	if (unseens.size())
	{
		SetFlagMessage(unseens, false, NMessage::eSeen, true);
		SetNumberUnseen(0);
	}

	// Close if previously open - don't bother punting again
	if (!was_open)
		CloseNoPunt();
}

// Copy this mbox to another mbox
void CMbox::CopyMbox(CMbox* mbox_to)
{
	// Check open status
	bool was_open = IsOpen();

	try
	{
		// Must be open
		if (!was_open)
		{
			// Don't do message caching
			// The copy operation will cache messages only if needed
			SetFlags(eNoCache);

			// Open using EXAMINE to keep it read-only - i.e. no flag changes
			// Open fully if on different server since messages have to be read in when transferred
			Open(NULL, false, true, (mMailer != mbox_to->mMailer) || !mMailer->DoesCopy());
		}

		// Only bother if messages actually exist!
		if (GetNumberFound())
		{
			// Determine all messages
			ulvector copies(GetNumberFound());

			for(unsigned long i = 1; i <= GetNumberFound(); i++)
				copies[i - 1] = i;

			// Copy unsorted
			ulmap temp;
			CopyMessage(copies, false, mbox_to, temp);
		}

		// Close if previously not open - don't bother punting again
		if (!was_open)
		{
			CloseSilent();

			// Re-enable message caching
			SetFlags(eNoCache, false);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Clean up and throw up
		if (!was_open)
		{
			CloseSilent();

			// Re-enable message caching
			SetFlags(eNoCache, false);
		}

		CLOG_LOGRETHROW;
		throw;
	}
}

// Copy the specified message to another mailbox
void CMbox::CopyMessage(unsigned long msg_num, bool uids, CMbox* mbox_to)
{
	ulvector nums(1, msg_num);
	ulmap temp;

	CopyMessage(nums, uids, mbox_to, temp);

} // CMbox::CopyMessage

// Copy the specified message to another mailbox
void CMbox::CopyMessage(const ulvector& nums, bool uids, CMbox* mbox_to, ulmap& copy_uids, bool sorted)
{
	// Map sorted nums to actual nums
	ulvector actual_nums;
	MapSorted(actual_nums, nums, !uids && sorted);

	// Check mboxes are on the same server and that the server does its own copying
	if ((mMailer == mbox_to->mMailer) && mMailer->DoesCopy())
	{
		// Copy message to server
		mOpenInfo->mMsgMailer->CopyMessage(this, actual_nums, uids, mbox_to, copy_uids);
	}
	else
	{
		// Must do visual progress
		CStatusWindow::StStatusLock lock_status;

		// Start cursor for busy operation
		CWaitCursor wait;

		// Manually append to mailbox with text
		try
		{
			// Make sure required UIDs are cached so we can actually access the message
			if (uids)
			{
				// Look for missing UIDs
				ulvector force_uids;
				for(ulvector::const_iterator iter = actual_nums.begin(); iter != actual_nums.end(); iter++)
				{
					// Try to locate message via UID
					CMessage* copy_from = NULL;
					copy_from = const_cast<CMessage*>(GetMessageUID(*iter));
					
					// Not found => cache it
					if (!copy_from)
						force_uids.push_back(*iter);
				}
				
				// Try to load all the missing UIDs
				if (force_uids.size())
					mOpenInfo->mMsgMailer->FetchItems(force_uids, true, CMboxProtocol::eUID);
			}


			// Start multiple append operation
			mbox_to->StartAppend();

			// For all messages
			size_t total = actual_nums.size();
			size_t current = 1;
			for(ulvector::const_iterator iter = actual_nums.begin(); iter != actual_nums.end(); iter++, current++)
			{
				// Set status info
				lock_status.AllowChange();
				CStatusWindow::SetIMAPStatus2("Status::IMAP::CopyingProgress", current, total);

				// Get cached message
				CMessage* copy_from = NULL;
				if (uids)
					copy_from = const_cast<CMessage*>(GetMessageUID(*iter));
				else
					copy_from = GetMessage(*iter);

				// Must throw if no message is found
				if (!copy_from)
				{
					CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}

				bool clear_cache = copy_from->CacheMessage();

				// Clean up after exceptions
				try
				{
					// Must read in message cache from server in order to do copy
					copy_from->ReadCache();

					// Append new message to mbox
					unsigned long new_uid = 0;
					mbox_to->AppendMessage(copy_from, new_uid, false, !IsSynchronising());

					// Clean up
					if (clear_cache)
						copy_from->UncacheMessage();
				}
				catch (...)
				{
					CLOG_LOGCATCH(...);

					// Clean up and throw up
					if (clear_cache)
						copy_from->UncacheMessage();
					CLOG_LOGRETHROW;
					throw;
				}
			}

			// Stop multiple append
			mbox_to->StopAppend();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Clean up multiple append operation
			lock_status.AllowChange();
			mbox_to->StopAppend();
			CStatusWindow::SetIMAPStatus("Status::IDLE");

			CLOG_LOGRETHROW;
			throw;
		}
		lock_status.AllowChange();
		CStatusWindow::SetIMAPStatus("Status::IDLE");
	}

	// Force fast check on copied to mailbox to ensure its display is up to date
	mbox_to->Check(true, true);

} // CMbox::CopyMessage

// Read the specified message
void CMbox::CopyMessage(unsigned long msg_num, bool uids, costream* aStream, bool sorted)
{
	if (!uids && sorted)
	{
		CMessage* theMsg = GetMessage(msg_num, true);
		CopyMessage(theMsg, aStream);
	}
	else
	{
		// Read message from server
		mOpenInfo->mMsgMailer->CopyMessage(this, msg_num, uids, *aStream);
	}

}

// Read the specified message
void CMbox::CopyMessage(CMessage* msg, costream* aStream)
{
	// Check for sub-message
	if (msg->IsSubMessage())
		// Copy the attachment
		CopyAttachment(msg->GetOwner(), msg->GetBody()->GetParent(), aStream);
	else
	{
		unsigned long msg_num = GetMessageIndex(msg);
		if (msg_num)
			CopyMessage(msg_num, false, aStream);
	}
}

// Expunge the specified message
void CMbox::ExpungeMessage(unsigned long msg_num, bool uids)
{
	ulvector nums(1, msg_num);

	ExpungeMessage(nums, uids);
}

// Expunge the specified message
void CMbox::ExpungeMessage(const ulvector& nums, bool uids, bool sorted)
{
	// Check for UID expunge
	if (mOpenInfo->mMsgMailer->DoesExpungeMessage())
	{
		// Map sorted nums to actual nums
		ulvector actual_nums;
		MapSorted(actual_nums, nums, !uids && sorted);

		mOpenInfo->mMsgMailer->ExpungeMessage(actual_nums, uids);
	}
	else
	{
		// Find currently deleted messages on the server
		CSearchItem search_spec(CSearchItem::eDeleted);
		ulvector original;
		Search(&search_spec, &original, true, true);

		// Find ones in original that are not to be expunged
		ulvector no_expunge;
		std::set_difference(original.begin(), original.end(), nums.begin(), nums.end(), std::back_inserter<ulvector>(no_expunge));

		// Unset delete flag on those not being expunged
		if (no_expunge.size())
			SetFlagMessage(no_expunge, true, NMessage::eDeleted, false);

		// Now do normal expunge
		Expunge(false);

		// Reset deleted flag on those not expunged
		if (no_expunge.size())
			SetFlagMessage(no_expunge, true, NMessage::eDeleted, true);
	}
}

// Synchronise the specified message
void CMbox::SynchroniseMessage(const ulvector& nums, bool uids, bool full, bool partial, unsigned long size, bool sorted)
{
	// Map sequence numbers to uids
	ulvector actual_nums;
	if (uids)
		actual_nums = nums;
	else
		MapUIDs(nums, actual_nums, sorted);

	// Tell protocol to synchronise it
	mMailer->SynchroniseRemote(this, full, partial, size, actual_nums);
}

// Synchronise the specified message
void CMbox::ClearDisconnectMessage(const ulvector& nums, bool uids, bool sorted)
{
	// Map sequence numbers to uids
	ulvector actual_nums;
	if (uids)
		actual_nums = nums;
	else
		MapUIDs(nums, actual_nums, sorted);

	// Tell protocol to synchronise it
	mOpenInfo->mMsgMailer->ClearDisconnect(this, actual_nums);

	// Clear out disconnected info
	if (!GetProtocol()->IsDisconnected() && IsFullOpen())
	{
		// Clear flags on all messages
		for(ulvector::iterator iter = actual_nums.begin(); iter != actual_nums.end(); iter++)
		{
			CMessage* msg = GetMessageUID(*iter);
			if (msg)
			{
				ulvector* clearem = NULL;
				if (msg->GetFlags().IsSet(NMessage::eFullLocal))
					clearem = &mOpenInfo->mFullLocal;
				else if (msg->GetFlags().IsSet(NMessage::ePartialLocal))
					clearem = &mOpenInfo->mPartialLocal;
				if (clearem)
				{
					msg->GetFlags().Set(NMessage::eFullLocal | NMessage::ePartialLocal, false);
					ulvector::iterator found = std::lower_bound(clearem->begin(), clearem->end(), *iter);
					if ((found != clearem->end()) && (*found == *iter))
						clearem->erase(found);
				}
			}
		}
	}
}

// Map sorted message positions to message numbers
void CMbox::MapSorted(ulvector& actual_nums, const ulvector& nums, bool sorted)
{
	if (sorted)
	{
		// Map sorted list to actual list
		actual_nums.reserve(nums.size());	// Preallocate in case of large mbox
		for(ulvector::const_iterator iter = nums.begin(); iter != nums.end(); iter++)
		{
			// Must have valid message and message number (may be a fake)
			CMessage* theMsg = GetMessage(*iter, true);
			if (theMsg && theMsg->GetMessageNumber())
				actual_nums.push_back(theMsg->GetMessageNumber());
		}
	}
	else
		// Just copy existing list
		actual_nums = nums;

}

// Map sorted message numbers to message uids
void CMbox::MapUIDs(const ulvector& nums, ulvector& uids, bool sorted)
{
	uids.reserve(nums.size());	// Preallocate in case of large mbox

	// May need to read in UIDs for some messages
	ulvector read_seq;
	read_seq.reserve(nums.size());	// Preallocate in case of large mbox

	// Check for non-existent UIDs
	for(ulvector::const_iterator iter = nums.begin(); iter != nums.end(); iter++)
	{
		CMessage* msg = GetMessage(*iter, sorted);
		if (msg && !msg->GetUID() && !msg->IsFake())
			read_seq.push_back(msg->GetMessageNumber());
	}

	// Read in missing UIDs
	if (read_seq.size())
		mOpenInfo->mMsgMailer->FetchItems(read_seq, false, CMboxProtocol::eUID);

	// Map sorted list to actual list
	for(ulvector::const_iterator iter = nums.begin(); iter != nums.end(); iter++)
		uids.push_back(GetMessage(*iter, sorted) ? GetMessage(*iter, sorted)->GetUID() : 0);

} // CMbox::MapUIDs

// Map UIDs to sequence numbers
void CMbox::GetUIDMap(ulmap& map) const
{
	if (!IsFullOpen())
		return;

	for(CMessageList::iterator iter = mOpenInfo->mMessages->begin(); iter != mOpenInfo->mMessages->end(); iter++)
		map.insert(ulmap::value_type((*iter)->GetUID(), (*iter)->GetMessageNumber()));
}

// Create message suitable for this mailbox
CMessage* CMbox::CreateMessage()
{
	return mOpenInfo->mMsgMailer->IsOffline() ? new CLocalMessage(this) : new CMessage(this);
}
