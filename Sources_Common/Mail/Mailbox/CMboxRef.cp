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


// CMboxRef : class to handle URL references to mailboxes

#include "CMboxRef.h"

#include "CMailAccountManager.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CSMTPAccountManager.h"

using namespace NMboxRef;

#pragma mark ____________________________CMboxRef

CMboxRef::CMboxRef(const char* acct_name, char dir, bool directory)
{
	mDirDelim = dir;
	SetName(acct_name);
	mFlags.Set(eIsDirectory, directory);
}

CMboxRef::CMboxRef(const CMbox* mbox)
{
	mDirDelim = mbox->GetDirDelim();
	SetName(mbox->GetAccountName());
	mWDLevel = -1;
	mFlags.Set(eIsDirectory, false);
}

CMboxRef::CMboxRef(const CMboxRef& copy)
	: CMboxNode(copy)
{
	mName = copy.GetName();
	mFlags = copy.mFlags;
	mAccountName = copy.mAccountName;

	// Cache pointer to mailbox name
	SetShortName();
}

// Assignment with same type
CMboxRef& CMboxRef::operator=(const CMboxRef& copy)
{
	if (this != &copy)
	{
		CMboxNode::operator=(copy);

		mName = copy.GetName();
		mFlags = copy.mFlags;
		mAccountName = copy.mAccountName;

		// Cache pointer to mailbox name
		SetShortName();
	}

	return *this;
}

// Assignment with mbox
CMboxRef& CMboxRef::operator=(const CMbox& mbox)
{
	SetName(mbox.GetAccountName());
	mWDLevel = -1;
	mDirDelim = mbox.GetDirDelim();

	return *this;
}

// Compare with same type
int CMboxRef::operator==(const CMboxRef& comp) const
{
	return (mName == comp.mName) &&
			(mAccountName == comp.mAccountName);
}

// Compare with mbox
int CMboxRef::operator==(const CMbox& mbox) const
{
	return GetAccountName() == mbox.GetAccountName();
}

void CMboxRef::SetName(const char* acct)
{
	// Name is full account name
	mName = acct;

	// Get account name (if any)
	const char* p = ::strchr(acct, cMailAccountSeparator);
	if (p && (*p != '*'))
		mAccountName.assign(acct, p - acct);

	// Cache pointer to mailbox name
	SetShortName();
}

// Set pointer to short name
void CMboxRef::SetShortName()
{
	// Determine last directory break
	const char* p = mDirDelim ? ::strrchr(mName.c_str(), mDirDelim) : NULL;
	const char* q = ::strchr(mName.c_str(), cMailAccountSeparator);
	if (p && (p > q))
		mShortName = ++p;
	else if (q && (*q != '*'))
		mShortName = ++q;
	else
		mShortName = mName.c_str();

} // CTreeNode::SetShortName

// Get account relative name of mailbox
cdstring CMboxRef::GetAccountName(bool multi) const
{
	cdstring acct;

	if (multi || mAccountName.empty())
		acct = mName;
	else
		acct = mName.c_str() + mAccountName.length() + 1;

	return acct;
}

CMbox* CMboxRef::ResolveMbox(bool force) const
{
	// Check whether resolve failed in the past
	if (mFlags.IsSet(eFailedResolve))
		return NULL;

	// Check regular mailboxes first
	cdstring acct_name = GetAccountName();
	CMbox* found = CMailAccountManager::sMailAccountManager ?
					CMailAccountManager::sMailAccountManager->FindMboxAccount(acct_name.c_str()) : NULL;

	// If the account part of the mailbox name is empty, check SMTP senders
	if (!found && GetAccountOnlyName().empty())
		found = CSMTPAccountManager::sSMTPAccountManager ?
					CSMTPAccountManager::sSMTPAccountManager->FindMbox(GetShortName()) : NULL;
	
	// May need to create singleton (only if not an SMTP account)
	if (!found && force && !GetAccountOnlyName().empty())
	{
		// Find protocol
		CMboxProtocol* proto = CMailAccountManager::sMailAccountManager->GetProtocol(acct_name);

		// Only do if logged in

		// Need to protect against re-entrancy here as a server window update during
		// a network call may result in nested calls to IMAP server - BAD!

		if (proto && proto->IsLoggedOn() && !proto->_get_mutex().is_locked())
		{
			// Create new unsubscribed mbox with auto add
			found = new CMbox(proto, GetAccountName(false), GetDirDelim(), nil);

			// Find mailbox list that could match this
			CMboxList* mbox_list = proto->FindMatchingList(found);

			if (mbox_list)
			{
				// Must be loaded, otherwise treat as logged off
				if (!mbox_list->IsLoaded())
					return NULL;

				CMbox* temp_found = NULL;

				// Check whether mailbox is in an undescovered hierarchy
				if (mbox_list->DoesDescovery())
				{
					// Now try to load it
					proto->LoadMbox(mbox_list, found->GetName());

					// Now see again if it exists
					temp_found = CMailAccountManager::sMailAccountManager->FindMboxAccount(acct_name.c_str());

				}

				// No need to create
				delete found;

				// Check if reolve failed completely
				if (!temp_found)
					const_cast<CMboxRef*>(this)->mFlags.Set(eFailedResolve);

				// Just return what we got, even if not found
				return temp_found;
			}

			if (!found->AddMbox())
				found = NULL;
		}
	}

	return found;
}

void CMboxRef::ResolveMbox(CMboxList& mboxes, bool dynamic, bool no_dir) const
{
	// Resolve one then do the rest!
	CMbox* found = ResolveMbox();
	if (found)
		found->GetProtocol()->FindMbox(found->GetName(), mboxes, no_dir);
}

#pragma mark ____________________________CWildcardMboxRef

CWildcardMboxRef::CWildcardMboxRef(const char* acct_name, char dir)
{
	mDirDelim = dir;
	SetName(acct_name);
	mFlags.Set(eIsDirectory, false);
}

// Assignment with same type
CWildcardMboxRef& CWildcardMboxRef::operator=(const CWildcardMboxRef& copy)
{
	if (this != &copy)
		CMboxRef::operator=(copy);

	return *this;
}

// Compare with same type
int CWildcardMboxRef::operator==(const CWildcardMboxRef& comp) const
{
	return CMboxRef::operator==(comp);
}

// Compare with mbox
int CWildcardMboxRef::operator==(const CMbox& mbox) const
{
	cdstring mbox_name = mbox.GetAccountName();

	// Try match on full pattern
	if (!mName.empty())
		return mbox_name.PatternDirMatch(mName, mbox.GetDirDelim());
	else
		return 0;
}

// Resolve into all matching mailboxes
void CWildcardMboxRef::ResolveMbox(CMboxList& mboxes, bool dynamic, bool no_dir) const
{
	CMailAccountManager::sMailAccountManager->FindWildcard(GetAccountName(), mboxes, dynamic, no_dir);
}
