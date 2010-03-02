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


// CAddressBook.cp

// Class that encapsulates address book functionality (local and remote).
// This is an abstract base class and should be derived for each type
// of address book (eg local, remote IMSP-style, remote-LDAP etc)

#include "CAddressBook.h"

#include "CAddressBookManager.h"
#include "CAdbkProtocol.h"
#include "CCharSpecials.h"
#include "CLog.h"
#include "CMailControl.h"
#include "CPreferences.h"
#include "CStringUtils.h"

#include "CVCardStoreXML.h"					// Share XML defintions with calendar store
#include "CVCardMapper.h"

#include "CVCardAddressBook.h"
#include "CVCardVCard.h"

#include <algorithm>
#include <memory>
#include <strstream>

#include "XMLDocument.h"
#include "XMLNode.h"
#include "XMLObject.h"

using namespace vcardstore;

cdmutex CAddressBook::_mutex;										// Used for locks

// This constructs the address book manager root
CAddressBook::CAddressBook()
{
	mProtocol = NULL;
	mParent = NULL;
	mChildren = new CAddressBookList;				// Root must always have list
	mChildren->set_delete_data(false);
	SetFlags(eIsProtocol, true);
	SetFlags(eIsDirectory, true);
	SetFlags(eHasExpanded, true);
	mSize = ULONG_MAX;
	mLastSync = 0;
	mACLs = NULL;
	mRefCount = 0;
	mVCardAdbk = NULL;
}

// This constructs the root of a protocol
CAddressBook::CAddressBook(CAdbkProtocol* proto)
{
	mProtocol = proto;
	mParent = &CAddressBookManager::sAddressBookManager->GetRoot();
	mChildren = NULL;
	SetFlags(eIsProtocol, true);
	SetFlags(eIsDirectory, true);
	SetFlags(eHasExpanded, true);
	mName = proto->GetAccountName();
	mShortName = mName;
	mSize = ULONG_MAX;
	mLastSync = 0;
	mACLs = NULL;
	mVCardAdbk = NULL;
}

// This constructs an actual node
CAddressBook::CAddressBook(CAdbkProtocol* proto, CAddressBook* parent, bool is_adbk, bool is_dir, const cdstring& name)
{
	mProtocol = proto;
	mParent = parent;
	mChildren = NULL;				// Root must always have list
	SetFlags(eIsAdbk, is_adbk);
	SetFlags(eIsDirectory, is_dir);
	mName = name;
	mSize = ULONG_MAX;
	mLastSync = 0;
	mACLs = NULL;
	mRefCount = 0;
	mVCardAdbk = NULL;

	SetShortName();
}

// Destructor
CAddressBook::~CAddressBook()
{
	// Do not sync the store root
	if (mProtocol != NULL)
		CAddressBookManager::sAddressBookManager->SyncAddressBook(this, false);

	// Deactivate/delete all children
	Clear();
	ClearContents();
	
	delete mACLs;
	mACLs = NULL;
}

cdstring CAddressBook::GetAccountName(bool multi) const
{
	cdstring name;
	if (multi)
	{
		name = mProtocol->GetAccountName();
		name += cMailAccountSeparator;
	}
	name += GetName();
	return name;
}

void CAddressBook::CheckSize()
{
	GetProtocol()->SizeAdbk(this);
}

void CAddressBook::SyncNow() const
{
	time_t now = ::time(NULL);
	mLastSync = ::mktime(::gmtime(&now));
}

bool CAddressBook::IsCached() const
{
	// Cached if not disconnected or known to have 
	return !GetProtocol()->IsDisconnected() || mFlags.IsSet(eIsCached);
}

cdstring CAddressBook::GetURL(bool full) const
{
	cdstring ruri = GetName();
	ruri.EncodeURL(GetProtocol()->GetDirDelim());

	cdstring result;
	if (!IsDisplayHierarchy())
	{
		result = mProtocol->GetURL(full);
		result += "/";
	}
	result += ruri;
	return result;
}

void CAddressBook::AddChild(CAddressBook* child, bool sort)
{
	// Create if required
	if (mChildren == NULL)
		mChildren = new CAddressBookList;
	mChildren->push_back(child);
	child->mParent = this;
	
	// Setting a child means it has been expanded
	SetFlags(eHasExpanded, true);
	
	// Do sort if requested
	if (sort)
		SortChildren();
}

void CAddressBook::AddChildHierarchy(CAddressBook* child, bool sort)
{
	// Only do this if its the protocol root
	if (mParent != &CAddressBookManager::sAddressBookManager->GetRoot())
		return;

	// Break the name down into components
	cdstrvect names;
	const char* start = child->GetName().c_str();
	const char* end = ::strchr(start, mProtocol->GetDirDelim());
	while(end != NULL)
	{
		names.push_back(cdstring(start, end - start));
		start = end + 1;
		end = ::strchr(start, mProtocol->GetDirDelim());
	}

	// Find each path component
	CAddressBook* parent = this;
	cdstring path;
	for(cdstrvect::const_iterator iter1 = names.begin(); iter1 != names.end(); iter1++)
	{
		// Get path component
		if (!path.empty())
			path += mProtocol->GetDirDelim();
		path += *iter1;
		
		// Find the node
		bool found = false;
		for(CAddressBookList::iterator iter2 = parent->GetChildren()->begin(); iter2 != parent->GetChildren()->end(); iter2++)
		{
			if ((*iter2)->GetName() == path)
			{
				parent = *iter2;
				found = true;
				break;
			}
		}
		
		// If not found, create a directory and make it the new parent
		if (!found)
		{
			CAddressBook* adbk = new CAddressBook(mProtocol, parent, false, true, path);
			parent->AddChild(adbk, sort);
			parent = adbk;
		}
	}

	// Now add to actual parent
	parent->AddChild(child, sort);
}

void CAddressBook::InsertChild(CAddressBook* child, uint32_t index, bool sort)
{
	// Do ordinary add if no children or insert at end
	if ((mChildren == NULL) || (index >= mChildren->size()))
		AddChild(child, sort);
	else
	{
		mChildren->insert(mChildren->begin() + index, child);
		child->mParent = this;
		
		// Do sort if requested
		if (sort)
			SortChildren();
	}
}

void CAddressBook::SortChildren()
{
	if (mChildren == NULL)
		return;
	
	std::sort(mChildren->begin(), mChildren->end(), sort_by_name);
}

bool CAddressBook::sort_by_name(const CAddressBook* s1, const CAddressBook* s2)
{
	// Put display hierarchies at the end
	if (s1->IsDisplayHierarchy() ^ s2->IsDisplayHierarchy())
	{
		return s2->IsDisplayHierarchy();
	}
	else
	{
		return ::strcmpnocase(s1->GetDisplayShortName(), s2->GetDisplayShortName()) < 0;
	}
}

CAddressBook* CAddressBook::FindNode(const cdstring& path, bool discover) const
{
	// Find top-level item matching last item in hierarchy
	if (mChildren)
	{
		for(CAddressBookList::iterator iter = mChildren->begin(); iter != mChildren->end(); iter++)
		{
			if (path == (*iter)->GetName())
				return *iter;
			if (path.compare_start((*iter)->GetName()))
			{
				// May need discovery
				if (discover && (*iter)->IsDirectory() && !(*iter)->HasExpanded())
					GetProtocol()->LoadSubList(*iter, false);					
				return (*iter)->FindNode(path, discover);
			}
		}
	}
	
	return NULL;
}

// Remove node from parent withtout deleting the node
void CAddressBook::RemoveFromParent()
{
	if (mParent)
	{
		CAddressBookList* list = mParent->GetChildren();
		CAddressBookList::iterator found = std::find(list->begin(), list->end(), this);
		if (found != list->end())
		{
			// NULL it out so that the node is not deleted, then erase
			*found = NULL;
			list->erase(found);
		}
		
		mParent = NULL;
	}
}

void CAddressBook::Clear()
{
	if (mChildren != NULL)
	{
		// Erase children
		delete mChildren;
		mChildren = NULL;
	}
}

uint32_t CAddressBook::GetRow() const
{
	// Look for sibling
	uint32_t row = GetParentOffset();
	const CAddressBook* parent = mParent;
	while(parent != NULL)
	{
		row += parent->GetParentOffset();
		parent = parent->mParent;
	}
	
	return row;
}

uint32_t CAddressBook::CountDescendants() const
{
	uint32_t result = 0;
	if (mChildren != NULL)
	{
		for(CAddressBookList::iterator iter = mChildren->begin(); iter != mChildren->end(); iter++)
			result += (*iter)->CountDescendants() + 1;
	}
	
	return result;
}

uint32_t CAddressBook::GetParentOffset() const
{
	uint32_t result = 0;
	if ((mParent != NULL) && mParent->HasInferiors())
	{
		result++;
		for(CAddressBookList::iterator iter = mParent->GetChildren()->begin(); iter != mParent->GetChildren()->end(); iter++)
		{
			if (*iter == this)
				break;
			
			result += (*iter)->CountDescendants() + 1;
		}
	}
	
	return result;
}

const CAddressBook* CAddressBook::GetSibling() const
{
	const CAddressBook* result = NULL;
	if ((mParent != NULL) && mParent->HasInferiors())
	{
		for(CAddressBookList::iterator iter = mParent->GetChildren()->begin(); iter != mParent->GetChildren()->end(); iter++)
		{
			if (*iter == this)
				break;
			
			result = *iter;
		}
	}
	
	return result;
}

void CAddressBook::GetInsertRows(uint32_t& parent_row, uint32_t& sibling_row) const
{
	// Get parent's row
	parent_row = (mParent ? mParent->GetRow() : 0);

	// Now get the sibling row
	uint32_t parent_offset = GetParentOffset();
	if ((parent_offset != 0) && (GetSibling() != NULL))
	{
		sibling_row = parent_row + parent_offset - GetSibling()->CountDescendants() - 1;
	}
	else
		sibling_row = 0;
}

// Set pointer to short name
void CAddressBook::SetShortName()
{
	// Determine last directory break
	if (GetProtocol()->GetDirDelim() != 0)
	{
		cdstrvect splits;
		mName.split(cdstring(GetProtocol()->GetDirDelim()), splits);
		if (splits.size() > 1)
		{
			if (splits.back().empty())
				splits.pop_back();
			mShortName = splits.back();
		}
		else
		{
			mShortName = splits.front();
		}
		
	}
	else
		mShortName = mName.c_str();
}

// Tell this and children to adjust names
void CAddressBook::NewName(const cdstring& name)
{
	// Adjust this one
	SetName(name);
	
	// Now iterate over children doing rename
	if (mChildren != NULL)
	{
		for(CAddressBookList::iterator iter = mChildren->begin(); iter != mChildren->end(); iter++)
		{
			(*iter)->ParentRenamed();
		}
	}
}

// Tell children to adjust names when parent moves
void CAddressBook::ParentRenamed()
{
	// Must have a parent
	if (mParent == NULL)
		return;

	// Adjust this node
	cdstring new_name;
	new_name = mParent->GetName();
	if (GetProtocol()->GetDirDelim())
		new_name += GetProtocol()->GetDirDelim();
	new_name += GetShortName();
	SetName(new_name);
	
	// Now iterate over children doing rename
	if (mChildren != NULL)
	{
		for(CAddressBookList::iterator iter = mChildren->begin(); iter != mChildren->end(); iter++)
		{
			(*iter)->ParentRenamed();
		}
	}
}

void CAddressBook::MoveAddressBook(const CAddressBook* dir, bool sibling)
{
#ifdef TODO
#endif
}

// Copy this address book into another one
void CAddressBook::CopyAddressBook(CAddressBook* node)
{
#ifdef TODO
#endif
}

// Copy this address book's items into another one
void CAddressBook::CopyAddressBookContents(CAddressBook* node)
{
#ifdef TODO
#endif
}

// Switch into disconnected mode
void CAddressBook::TestDisconnectCache()
{
	if (mChildren != NULL)
	{
		// Test each child recursively
		for(CAddressBookList::iterator iter = mChildren->begin(); iter != mChildren->end(); iter++)
			(*iter)->TestDisconnectCache();
	}

	// See if it exists locally
	if (IsAdbk())
		SetFlags(eIsCached, mProtocol->TestAdbk(this));
}

#pragma mark ____________________________Opening/Closing

void CAddressBook::SetName(const cdstring& name)
{
	// Remove from manager's cache
	if (mName.length())
		CAddressBookManager::sAddressBookManager->SyncAddressBook(this, false);

	mName = name;
	SetShortName();
	//CAddressBookManager::sAddressBookManager->RefreshAddressBook(this);

	// Add to manager's cache
	CAddressBookManager::sAddressBookManager->SyncAddressBook(this, true);
}

// Determine whether to open based on ref count
bool CAddressBook::OpenCount()
{
	// Lock global mutex
	cdmutex::lock_cdmutex _lock(_mutex);
	
	// If ref count is 0, then open required - also bump ref count
	return mRefCount++ == 0;
}

// Determine whether to close based on ref cunt
bool CAddressBook::CloseCount()
{
	// Lock global mutex
	cdmutex::lock_cdmutex _lock(_mutex);
	
	// Bump down ref count, if ref count is now 0, then close required
	return --mRefCount == 0;
}

// Open visual address book from source
void CAddressBook::Open()
{
	// Bump reference count and open only if not already done
	if (!OpenCount())
		return;

	try
	{
		// NB We might have a vCard adbk if we've done searches whilst the address book is closed.
		// If that is the case we want to discard everything and start from scratch.
		if (mVCardAdbk != NULL)
			mVCardAdbk->Clear();
		else
			mVCardAdbk = new vCard::CVCardAddressBook();

		// Must remove any cached address lookups
		mAddresses.clear();
		mGroups.clear();

		// Now open
		mProtocol->OpenAdbk(this);

		// Mark as open
		SetFlags(eOpen);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
		
		delete mVCardAdbk;
		mVCardAdbk = NULL;
	}
}

// Close visual address book
void CAddressBook::Close()
{
	// Only if already open
	if (!IsOpen())
		return;

	// Bump reference count and close only if not open elsewhere
	if (!CloseCount())
		return;

	mProtocol->CloseAdbk(this);

	// Clean up the UI
	CMailControl::AddressBookClosed(this);

	SetFlags(eOpen, false);
	
	ClearContents();

	delete mVCardAdbk;
	mVCardAdbk = NULL;
}

// Rename address book
void CAddressBook::Rename(cdstring& new_name)
{
	mProtocol->RenameAdbk(this, new_name);

	// Remove from manager's cache
	CAddressBookManager::sAddressBookManager->SyncAddressBook(this, false);
	CPreferences::sPrefs->ChangeAddressBookOpenOnStart(this, false);
	CPreferences::sPrefs->ChangeAddressBookLookup(this, false);
	CPreferences::sPrefs->ChangeAddressBookSearch(this, false);

	mName = new_name;

	// Add to manager's cache
	CAddressBookManager::sAddressBookManager->SyncAddressBook(this, true);
	CPreferences::sPrefs->ChangeAddressBookOpenOnStart(this, IsSearch());
	CPreferences::sPrefs->ChangeAddressBookLookup(this, IsLookup());
	CPreferences::sPrefs->ChangeAddressBookSearch(this, true);
}

// Clear addresses
void CAddressBook::ClearContents()
{
	mAddresses.clear();
	mGroups.clear();
	if (mVCardAdbk != NULL)
		mVCardAdbk->Clear();
}

// Delete address book
void CAddressBook::Delete()
{
	// Close first
	Close();

	// Remove from manager's cache
	CAddressBookManager::sAddressBookManager->SyncAddressBook(this, false);
	CPreferences::sPrefs->ChangeAddressBookOpenOnStart(this, false);
	CPreferences::sPrefs->ChangeAddressBookLookup(this, false);
	CPreferences::sPrefs->ChangeAddressBookSearch(this, false);

	// Now delete on server
	mProtocol->DeleteAdbk(this);

	// Remove from manager
	//CAddressBookManager::sAddressBookManager->RemoveAddressBook(this);
}

// Empty address book
void CAddressBook::Empty()
{
	// Delete and recreate
	mProtocol->DeleteAdbk(this);
	mProtocol->CreateAdbk(this);
	
	ClearContents();
}

// Synchronise to local
void CAddressBook::Synchronise(bool fast)
{
	// Tell protocol to synchronise it
	mProtocol->SynchroniseRemote(this, fast);
}

// Clear disconnected cache
void CAddressBook::ClearDisconnect()
{
	mProtocol->ClearDisconnect(this);
}

#ifdef _TODO
// Copy addresses and groups to another address book
void CAddressBook::CopyAll(CAddressBook* adbk)
{
	// Copy all addresses
	CAddressList addrs;
	addrs.set_delete_data(false);
	for(CAddressList::iterator iter = mAddresses.begin(); iter != mAddresses.end(); iter++)
		addrs.push_back(new CAdbkAddress(*static_cast<CAdbkAddress*>(*iter)));
	adbk->AddAddress(&addrs);

	// Copy all groups
	CGroupList grps;
	grps.set_delete_data(false);
	for(CGroupList::iterator iter = mGroups.begin(); iter != mGroups.end(); iter++)
		grps.push_back(new CGroup(*static_cast<CGroup*>(*iter)));
	adbk->AddGroup(&grps);
}

// Synchronise to local
void CAddressBook::Synchronise(bool fast)
{
	// Tell protocol to synchronise it
	mProtocol->SynchroniseRemote(this, fast);
}

// Clear disconnected cache
void CAddressBook::ClearDisconnect()
{
	mProtocol->ClearDisconnect(this);
}

// Switch into disconnected mode
void CAddressBook::SwitchDisconnect(CAdbkProtocol* local)
{
	// See if going into disconnected mode (i.e. local != NULL)
	if (local)
	{
		// Set local flag
		SetFlags(CAddressBook::eLocalAdbk);

		// See if it exists locally
		if (local->TestAdbk(this))
			SetFlags(CAddressBook::eCachedAdbk);
	}
	else
		// Remove disconnected flags
		SetFlags(static_cast<CAddressBook::EFlags>(CAddressBook::eLocalAdbk | CAddressBook::eCachedAdbk), false);
}
#endif

CAdbkAddress* CAddressBook::FindAddress(const char* name)
{
	// Look through address list for match
	for(CAddressList::iterator iter = mAddresses.begin(); iter != mAddresses.end(); iter++)
	{
		if ((*iter)->GetName() == name)
			return static_cast<CAdbkAddress*>(*iter);
	}

	return NULL;
}

CAdbkAddress* CAddressBook::FindAddress(const CAddress* addr)
{
	// Look through address list for match
	for(CAddressList::iterator iter = mAddresses.begin(); iter != mAddresses.end(); iter++)
	{
		if (**iter == *addr)
			return static_cast<CAdbkAddress*>(*iter);
	}

	return NULL;
}

CAdbkAddress* CAddressBook::FindAddressEntry(const char* entry)
{
	CAddressList::iterator found = std::find_if(mAddresses.begin(), mAddresses.end(), CAdbkAddress::same_entry_str(entry));
	if (found != mAddresses.end())
		return static_cast<CAdbkAddress*>(*found);

	return NULL;
}

CGroup* CAddressBook::FindGroup(const char* name)
{
	// Look through group list for match
	for(CGroupList::iterator iter = mGroups.begin(); iter != mGroups.end(); iter++)
	{
		if ((*iter)->GetName() == name)
			return *iter;
	}
	
	return NULL;
}

CGroup* CAddressBook::FindGroupEntry(const char* entry)
{
	CGroupList::iterator found = std::find_if(mGroups.begin(), mGroups.end(), CGroup::same_entry_str(entry));
	if (found != mGroups.end())
		return static_cast<CGroup*>(*found);

	return NULL;
}

void CAddressBook::AddAddress(CAddress* addr, bool sorted)
{
	CAddressList addrs;
	addrs.set_delete_data(false);
	addrs.push_back(addr);
	AddAddress(&addrs, sorted);
}

// Add addresses to list or delete them
void CAddressBook::AddAddress(CAddressList* addrs, bool sorted)
{
	// Always update the internal cache and map the address to a vCard
	for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
	{
		if (IsOpen())
		{
			// Update internal cache
			if (sorted)
				mAddresses.push_back_sorted(*iter);
			else
				mAddresses.push_back(*iter);
			
			// Map to vCard
			std::auto_ptr<vCard::CVCardVCard> vcard(vcardstore::GenerateVCard(GetVCardAdbk()->GetRef(), static_cast<CAdbkAddress*>(*iter), true));
			GetVCardAdbk()->AddNewVCard(vcard.release(), true);
		}
	}

	mProtocol->AddAddress(this, addrs);

	// Do change notification
	CMailControl::AddressAdded(this, addrs);

	// Awlays delete data if not needed
	for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
	{
		if (!IsOpen())
			delete *iter;
	}
}

// Add unique addresses from list
void CAddressBook::AddUniqueAddresses(CAddressList& add)
{
	CAddressList unique;
	unique.set_delete_data(false);

	// Count unique items first
	for(CAddressList::iterator iter = add.begin(); iter != add.end(); iter++)
	{
		if (!mAddresses.IsDuplicate(*iter))
			unique.push_back(new CAdbkAddress(**iter));
	}

	// Add unique items
	if (unique.size())
		AddAddress(&unique);
}

void CAddressBook::UpdateAddress(CAddress* addr, bool sorted)
{
	CAddressList addrs;
	addrs.set_delete_data(false);
	addrs.push_back(addr);
	UpdateAddress(&addrs, sorted);
}

void CAddressBook::UpdateAddress(CAddressList* addrs, bool sorted)
{
	// Always map the address to a vCard
	for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
	{
		// Change the existing vcard
		vcardstore::ChangeVCard(GetVCardAdbk(), static_cast<CAdbkAddress*>(*iter));
	}
	
	// Change it
	mProtocol->ChangeAddress(this, addrs);

	// Do change notification
	CMailControl::AddressChanged(this, addrs);
}

// Address changed
void CAddressBook::UpdateAddress(CAddressList* old_addrs, CAddressList* new_addrs, bool sorted)
{
	// This is strange: the protocol needs to have the old address data in order to do the actual delete,
	// since it keys off the data and not the pointer. The internal address book uses the pointer of the
	// original item (now the new item). So we must get the use of old_addr and new_addr correct.
	
	// Switch pointer to old items to pointers to new items
	for(CAddressList::iterator iter = old_addrs->begin(); iter != old_addrs->end(); iter++)
	{
		// Find new one
		CAddressList::iterator found = std::find_if(new_addrs->begin(), new_addrs->end(), CAdbkAddress::same_entry(static_cast<CAdbkAddress*>(*iter)));
		if (found != new_addrs->end())
		{
			// Get index of old one and replace with new one
			unsigned long index = mAddresses.FetchIndexOf(*iter);
			if (index != 0)
				mAddresses.at(index - 1) = *found;
		}
	}
	
	// Now do actual change
	UpdateAddress(new_addrs, sorted);
	
	// Update the entries in the old ones to match those of the new ones
	// This is needed with IMSP/LocalAdbk which use the full name as the key
	int ctr = 0;
	for(CAddressList::iterator iter = old_addrs->begin(); iter != old_addrs->end(); iter++, ctr++)
		static_cast<CAdbkAddress*>(*iter) ->SetEntry(static_cast<CAdbkAddress*>(new_addrs->at(ctr))->GetEntry());
}

void CAddressBook::RemoveAddress(CAddress* addr)
{
	CAddressList addrs;
	addrs.set_delete_data(false);
	addrs.push_back(addr);
	RemoveAddress(&addrs);
}

void CAddressBook::RemoveAddress(CAddressList* addrs)
{
	mProtocol->RemoveAddress(this, addrs);

	if (IsOpen())
	{
		// Always update the vCards
		for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
		{
			// Remove the vCard
			GetVCardAdbk()->RemoveCardByKey(static_cast<const CAdbkAddress*>(*iter)->GetEntry());
		}
		
		mAddresses.RemoveAddress(addrs);
	}

	mProtocol->RemovalOfAddress(this);

	// Do change notification
	CMailControl::AddressRemoved(this, addrs);
}

void CAddressBook::AddGroup(CGroup* grp, bool sorted)
{
	CGroupList grps;
	grps.set_delete_data(false);
	grps.push_back(grp);
	AddGroup(&grps, sorted);
}

// Add group to group list.
void CAddressBook::AddGroup(CGroupList* grps, bool sorted)
{
	mProtocol->AddGroup(this, grps);

	for(CGroupList::const_iterator iter = grps->begin(); iter != grps->end(); iter++)
	{
		if (IsOpen())
		{
			if (sorted)
				mGroups.push_back_sorted(*iter);
			else
				mGroups.push_back(*iter);
		}
		else
			delete *iter;
	}

	// Do change notification
	CMailControl::GroupAdded(this, grps);
}

// Add unique groups from list
void CAddressBook::AddUniqueGroups(CGroupList& add)
{
	CGroupList unique;
	unique.set_delete_data(false);

	// Count unique items first
	for(CGroupList::const_iterator iter = add.begin(); iter != add.end(); iter++)
	{
		if (!mGroups.IsDuplicate(*iter))
			unique.push_back(new CGroup(**iter));
	}

	// Add unique items
	if (unique.size())
		AddGroup(&unique);
}

void CAddressBook::UpdateGroup(CGroup* grp, bool sorted)
{
	CGroupList grps;
	grps.set_delete_data(false);
	grps.push_back(grp);
	UpdateGroup(&grps, sorted);
}

void CAddressBook::UpdateGroup(CGroupList* grps, bool sorted)
{
	// Remove it then add it
	mProtocol->ChangeGroup(this, grps);

	// Do change notification
	CMailControl::GroupChanged(this, grps);
}

// Address changed
void CAddressBook::UpdateGroup(CGroupList* old_grps, CGroupList* new_grps, bool sorted)
{
	// Switch pointer to old items to pointers to new items
	for(CGroupList::iterator iter = old_grps->begin(); iter != old_grps->end(); iter++)
	{
		// Find new one
		CGroupList::iterator found = std::find_if(new_grps->begin(), new_grps->end(), CGroup::same_entry(*iter));
		if (found != new_grps->end())
		{
			// Get index of old one and replace with new one
			unsigned long index = mGroups.FetchIndexOf(*iter);
			if (index != 0)
				mGroups.at(index - 1) = *found;
		}
	}
	
	// Now do actual change
	UpdateGroup(new_grps, sorted);
}

void CAddressBook::RemoveGroup(CGroup* grp)
{
	CGroupList grps;
	grps.set_delete_data(false);
	grps.push_back(grp);
	RemoveGroup(&grps);
}

void CAddressBook::RemoveGroup(CGroupList* grps)
{
	mProtocol->RemoveGroup(this, grps);
	if (IsOpen())
		mGroups.RemoveGroup(grps);

	mProtocol->RemovalOfAddress(this);

	// Do change notification
	CMailControl::GroupRemoved(this, grps);
}

// Give this address a unique entry
void CAddressBook::MakeUniqueEntry(CAdbkAddress* addr) const
{
	// If it has an entry we're done
	bool done = false;
	while(!done)
	{
		// Generate time stamp
		char time_str[256];
		::snprintf(time_str, 256, "Mulberry_%04x", ::time(NULL) + (time_t) ::clock());

		// Look for match
		done = true;
		for(CAddressList::const_iterator iter = mAddresses.begin(); iter != mAddresses.end(); iter++)
		{
			if (static_cast<CAdbkAddress*>(*iter)->GetEntry() == time_str)
			{
				done = false;
				break;
			}
		}

		// Add if its unique
		if (done)
			addr->SetEntry(time_str);
	}
}

#pragma mark ____________________________Read/Write

#define ANON				"Anonymous"
#define	GROUP_HEADER		"Group:"
#define	NEW_GROUP_HEADER	"Grp:"

void CAddressBook::ImportAddresses(char* txt)
{
	// Do to each line
	char* p = txt;
	char* line = p;
	while(*p)
	{
		// Punt to end of line/string
		while(*p && (*p != '\r') && (*p != '\n'))
			p++;
		
		// Terminate if not at end
		if (*p)
			*p++ = 0;

		// Import whole line
		ImportAddress(line, true, NULL, NULL);
		
		// Punt over line ends
		while(*p && ((*p == '\r') || (*p == '\n')))
			p++;
		
		// Reset line start
		line = p;
	}
}

void CAddressBook::ImportAddress(char* txt, bool add, CAdbkAddress** raddr, CGroup** rgrp, bool add_entry)
{
	char* p = txt;

	cdstring adl;
	cdstring name;
	cdstring whole_name;
	cdstring eaddr;
	cdstring calendar;
	cdstring company;
	cdstring address;
	cdstring phone_work;
	cdstring phone_home;
	cdstring fax;
	cdstring url;
	cdstring notes;

	// ADL will be at start
	char q = SkipTerm(&p, adl);
	adl.ConvertToOS();

	// Check for group
	if (((adl == NEW_GROUP_HEADER) || (adl == GROUP_HEADER)) && (q != '\r') && (q != '\n') && q)
	{
		if (adl == NEW_GROUP_HEADER)
		{
			// Nick-name is next
			q = SkipTerm(&p, adl);
			adl.ConvertToOS();

			// Do not allow anonymous
			if (adl == ANON)
				adl = cdstring::null_str;
		}
		else
			adl = cdstring::null_str;

		// Name is next
		q = SkipTerm(&p, name);
		name.ConvertToOS();

		// Do not allow anonymous
		if (name == ANON)
			name = cdstring::null_str;

		// Create new group
		CGroup* grp  = new CGroup(name, adl);
		if (add_entry)
			grp->SetEntry(name);

		// Add all addresses to group
		while(q && (q != '\r') && (q != '\n'))
		{
			q = SkipTerm(&p, eaddr);
			eaddr.ConvertToOS();

			if (!eaddr.empty())
				grp->GetAddressList().push_back(eaddr);
		}
		
		// Add it or return it
		if (add)
			GetGroupList()->push_back(grp);
		else if (rgrp)
			*rgrp = grp;
	}

	// Got address
	else if ((q != '\r') && (q != '\n') && q)
	{
		// Check for anonymous
		if (adl == ANON)
			adl = cdstring::null_str;

		// Name is next
		q = SkipTerm(&p, name);
		name.ConvertToOS();

		// Check for anonymous
		if (name != ANON)
		{
			// Process into first & last names

			// Look for comma
			const char* comma = ::strchr(name.c_str(), ',');

			if (comma)
			{
				const char* r = comma + 1;

				// Strip leading space
				r += ::strspn(r, SPACE);

				// Copy first names first with trailing space
				whole_name = r;
				whole_name += SPACE;

				// Copy last name up to comma
				whole_name += cdstring(name, 0, comma - name.c_str());
			}

			// Name in correct order - just copy
			else
				whole_name = name;
		}

		short ctr = 2;
		while ((q != '\r') && (q != '\n') && q)
		{
			switch(ctr++)
			{
			case 2:
				q = SkipTerm(&p, eaddr);
				eaddr.ConvertToOS();
				break;
			case 3:
				q = SkipTerm(&p, company);
				company.ConvertToOS();
				break;
			case 4:
				q = SkipTerm(&p, address);

				// Must unescape address
				address.FilterOutEscapeChars();
				address.ConvertToOS();
				break;
			case 5:
				q = SkipTerm(&p, phone_work);
				phone_work.ConvertToOS();
				break;
			case 6:
				q = SkipTerm(&p, phone_home);
				phone_home.ConvertToOS();
				break;
			case 7:
				q = SkipTerm(&p, fax);
				fax.ConvertToOS();
				break;
			case 8:
				q = SkipTerm(&p, url);

				// Must unescape URL
				url.FilterOutEscapeChars();
				url.ConvertToOS();

				// If no more then last item was notes
				if ((q == '\r') || (q == '\n') || !q)
				{
					notes = url;
					url = cdstring::null_str;
				}
				break;
			case 9:
				q = SkipTerm(&p, notes);

				// Must unescape notes
				notes.FilterOutEscapeChars();
				notes.ConvertToOS();
				break;
			default:
				// Step till line end or string end
				while ((*p) && (*p != '\r') && (*p != '\n')) p++;
				q = *p;
				if (*p == '\r') p++;
				if (*p == '\n') p++;
			}
		}


		CAdbkAddress* addr = new CAdbkAddress(NULL, eaddr, whole_name, adl, calendar, company, address,
													phone_work, phone_home, fax, url, notes);
		if (add_entry)
			addr->SetEntry(whole_name);
		
		// Add it or return it
		if (add)
			GetAddressList()->push_back(addr);
		else if (raddr)
			*raddr = addr;
	}
}

// Read and set bounds from file
char CAddressBook::SkipTerm(char** txt, cdstring& copy)
{
	const char* p = *txt;

	// Step up to terminator
	while ((**txt) && (**txt != '\t') && (**txt != '\r') && (**txt != '\n')) (*txt)++;

	char q = **txt;

	// null terminate and advance
	if ((**txt == '\r') || (**txt == '\n'))
	{
		**txt = '\0';
		(*txt)++;
		while((**txt) && ((**txt == '\r') || (**txt == '\n')))
			(*txt)++;
	}
	else if (**txt == '\t')
	{
		**txt = '\0';
		(*txt)++;
	}

	// Copy tokenised item
	copy = p;

	return q;
}

char* CAddressBook::ExportAddress(const CAdbkAddress* addr) const
{
	std::ostrstream out;

	// Write nick-name (adl)
	cdstring str;
	if (addr->GetADL().empty())
		str = ANON;
	else
		str = addr->GetADL();
	out << str;
	out.put('\t');

	// Write user name
	if (addr->GetName().empty())
		str = ANON;
	else
		str = addr->GetName();
	char whole_name[256];
	::strcpy(whole_name, str);

	// Find last name
	char* last_name = ::strrchr(whole_name, ' ');
	if (last_name)
	{
		// Tie it off and advance to last name
		*last_name = '\0';
		last_name++;
		if (::strlen(last_name) > 0)
		{
			str = last_name;
			out << str << ", ";
		}
	}

	// Do first name
	str = whole_name;
	out << str;
	out.put('\t');

	// Write email address mailbox@host
	str = addr->GetMailAddress();
	out << str;
	out.put('\t');

	// Write company
	str = addr->GetCompany();
	out << str;
	out.put('\t');

	// Write address
	str = addr->GetAddress(CAdbkAddress::eDefaultAddressType);
	if (!str.empty())
	{
		str.FilterInEscapeChars(cCEscapeChar);
		out << str;
	}
	out.put('\t');

	// Write phone work
	str = addr->GetPhone(CAdbkAddress::eWorkPhoneType);
	out << str;
	out.put('\t');

	// Write phone home
	str = addr->GetPhone(CAdbkAddress::eHomePhoneType);
	out << str;
	out.put('\t');

	// Write fax
	str = addr->GetPhone(CAdbkAddress::eFaxType);
	out << str;
	out.put('\t');

	// Write URL
	str = addr->GetURL();
	if (!str.empty())
	{
		str.FilterInEscapeChars(cCEscapeChar);
		out << str;
	}
	out.put('\t');

	// Write notes
	str = addr->GetNotes();
	if (!str.empty())
	{
		str.FilterInEscapeChars(cCEscapeChar);
		out << str;
	}

	out << os_endl << std::ends;
	return out.str();
}

char* CAddressBook::ExportGroup(const CGroup* grp) const
{
	std::ostrstream out;

	// Write group header
	cdstring str = NEW_GROUP_HEADER;
	out << str;
	out.put('\t');

	// Write group nick-name
	if (grp->GetNickName().empty())
		str = ANON;
	else
		str = grp->GetNickName();
	out << str;
	out.put('\t');

	// Write group name
	if (grp->GetName().empty())
		str = ANON;
	else
		str = grp->GetName();
	out << str;

	// Write each address out
	for(cdstrvect::const_iterator iter = grp->GetAddressList().begin(); iter != grp->GetAddressList().end(); iter++)
	{
		// Write TAB before address
		out.put('\t');

		// Write address
		str = *iter;
		out << str;
	}

	out << os_endl << std::ends;
	return out.str();
}

#pragma mark ____________________________Lookup

// Find nick-name
bool CAddressBook::FindNickName(const char* nick_name, CAdbkAddress*& addr, bool cache_only)
{
	bool result = false;

	// Try to get nick-name from list
	for(CAddressList::const_iterator iter = mAddresses.begin(); iter != mAddresses.end(); iter++)
	{
		if (::strcmpnocase(nick_name, (*iter)->GetADL())==0)
		{
			addr = dynamic_cast<CAdbkAddress*>(*iter);
			result = true;
			break;
		}
	}

	// Do remote lookup if not open
	if (!cache_only && !result && !IsOpen())
	{
		// Need to create vCard address book for cached data
		mVCardAdbk = new vCard::CVCardAddressBook();
		
		// Lookup
		mProtocol->ResolveAddress(this, nick_name, addr);

		// Resolveb from cache again
		result = CAddressBook::FindNickName(nick_name, addr, true);
	}

	return result;
}

// Find group nick-name
bool CAddressBook::FindGroupName(const char* grp_name, CGroup*& grp, bool cache_only)
{
	bool result = false;

	// Try to get nick-name from list
	for(CGroupList::const_iterator iter = mGroups.begin(); iter != mGroups.end(); iter++)
	{
		if (::strcmpnocase(grp_name, (*iter)->GetNickName())==0)
		{
			grp = *iter;
			result = true;
			break;
		}
	}

	// Do remote lookup if not open
	if (!cache_only && !result && !IsOpen())
	{
		// Need to create vCard address book for cached data
		mVCardAdbk = new vCard::CVCardAddressBook();
		
		// Lookup
		mProtocol->ResolveGroup(this, grp_name, grp);

		// Resolveb from cache again
		result = CAddressBook::FindGroupName(grp_name, grp, true);
	}

	return result;
}

// Do search
void CAddressBook::SearchAddress(const cdstring& name, CAdbkAddress::EAddressMatch match, const CAdbkAddress::CAddressFields& fields, CAddressList& addr_list)
{
	// Do remote lookup only if not open
	if (!IsOpen())
		mProtocol->SearchAddress(this, name, match, fields, addr_list);
	else
	{
		cdstring matchit(name);
		CAdbkAddress::ExpandMatch(match, matchit);

		// Iterate over all single addresses
		for(CAddressList::const_iterator iter = mAddresses.begin(); iter != mAddresses.end(); iter++)
		{
			bool result = false;

			CAdbkAddress* addr = dynamic_cast<CAdbkAddress*>(*iter);
			if (!addr)
				continue;
				
			// Do comparison of specific field
			result = addr->Search(matchit, fields);

			// Check for matching item
			if (result)
				// Add copy to list
				addr_list.push_back(new CAdbkAddress(*addr));
		}
	}
}


#pragma mark ____________________________ACLs

// Get user's rights from server
void CAddressBook::CheckMyRights()
{
	// Check for valid protocol & state
	if (!mProtocol->IsACLAdbk() || mProtocol->IsDisconnected())
		return;

	mProtocol->MyRights(this);
}

// Add ACL to list
void CAddressBook::AddACL(const CAdbkACL* acl)
{
	// Check for valid protocol & state
	if (!mProtocol->IsACLAdbk() || mProtocol->IsDisconnected())
		return;

	// Create list if it does not exist
	if (!mACLs)
		mACLs = new CAdbkACLList;

	// Add
	mACLs->push_back(*acl);
}

// Set ACL on server
void CAddressBook::SetACL(CAdbkACL* acl)
{
	// Check for valid protocol & state
	if (!mProtocol->IsACLAdbk() || mProtocol->IsDisconnected())
		return;

	// Try to set on server
	try
	{
		mProtocol->SetACL(this, acl);

		// Create list if it does not exist
		if (!mACLs)
			mACLs = new CAdbkACLList;

		// Search for existing ACL
		CAdbkACLList::iterator found = std::find(mACLs->begin(), mACLs->end(), *acl);

		// Add if not found
		if (found == mACLs->end())
			mACLs->push_back(*acl);
		else
			// Replace existing
			*found = *acl;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Silent failure
	}
}

// Set ACL on server
void CAddressBook::DeleteACL(CAdbkACL* acl)
{
	// Check for valid protocol & state
	if (!mProtocol->IsACLAdbk() || mProtocol->IsDisconnected())
		return;

	// Try to delete on server
	try
	{
		mProtocol->DeleteACL(this, acl);

		// Search for existing ACL
		CAdbkACLList::iterator found = std::find(mACLs->begin(), mACLs->end(), *acl);

		// Remove it
		if (found != mACLs->end())
			mACLs->erase(found);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Silent failure
	}
}

// Get ACLs from server
void CAddressBook::CheckACLs()
{
	// Check for valid protocol & state
	if (!mProtocol->IsACLAdbk() || mProtocol->IsDisconnected())
		return;

	// Save existing list in case of failure
	CAdbkACLList* save = (mACLs ? new CAdbkACLList(*mACLs) : NULL);

	try
	{
		// Delete everything in existing list
		if (mACLs)
			mACLs->clear();

		mProtocol->GetACL(this);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Replace failed list with old one
		if (save)
			*mACLs = *save;
		delete save;

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}

	delete save;
}

#pragma mark ____________________________XML

void CAddressBook::WriteXML(xmllib::XMLDocument* doc, xmllib::XMLNode* parent, bool is_root) const
{
	xmllib::XMLNode* xmlnode = NULL;
	
	// root node just write children
	if (!is_root)
	{
		// Create new node
		xmlnode = new xmllib::XMLNode(doc, parent, cXMLElement_adbknode);

		// Set adbk attribute
		if (IsAdbk())
		{
			xmllib::XMLObject::WriteAttribute(xmlnode, cXMLAttribute_adbk, IsAdbk());
		}

		// Set directory attribute
		if (IsDirectory())
		{
			xmllib::XMLObject::WriteAttribute(xmlnode, cXMLAttribute_directory, IsDirectory());
			xmllib::XMLObject::WriteAttribute(xmlnode, cXMLAttribute_has_expanded, HasExpanded());
		}

		if (IsDisplayHierarchy())		
			xmllib::XMLObject::WriteAttribute(xmlnode, cXMLAttribute_displayhierachy, IsDisplayHierarchy());

		// Set name child node
		xmllib::XMLObject::WriteValue(doc, xmlnode, cXMLElement_name, GetName());
		
		// Set display name child node
		if (!mDisplayName.empty())
			xmllib::XMLObject::WriteValue(doc, xmlnode, cXMLElement_displayname, mDisplayName);
		
		// Set last sync child node
		if (mLastSync != 0)
			xmllib::XMLObject::WriteValue(doc, xmlnode, cXMLElement_lastsync, mLastSync);
	}
	else
		xmlnode = parent;

	// Do children if they exist
	if (GetChildren() != NULL)
	{
		for(CAddressBookList::const_iterator iter = GetChildren()->begin(); iter != GetChildren()->end(); iter++)
		{
			(*iter)->WriteXML(doc, xmlnode);
		}
	}
}

void CAddressBook::ReadXML(const xmllib::XMLNode* xmlnode, bool is_root)
{
	if (is_root)
	{
		// Get has expanded
		bool has_expanded = false;
		if (xmlnode->AttributeValue(cXMLAttribute_has_expanded, has_expanded))
		{
			SetHasExpanded(has_expanded);
		}
	}
	else
	{
		// Must have right type of node
		if (!xmlnode->CompareFullName(cXMLElement_adbknode))
			return;
		
		// Check attributes
		bool temp = false;
		xmllib::XMLObject::ReadAttribute(xmlnode, cXMLAttribute_adbk, temp);
		SetFlags(eIsAdbk, temp);

		temp = false;
		if (xmllib::XMLObject::ReadAttribute(xmlnode, cXMLAttribute_directory, temp))
		{
			SetFlags(eIsDirectory, temp);
			if (xmllib::XMLObject::ReadAttribute(xmlnode, cXMLAttribute_has_expanded, temp))
				SetHasExpanded(temp);
		}

		if (xmllib::XMLObject::ReadAttribute(xmlnode, cXMLAttribute_displayhierachy, temp))
			SetFlags(eIsDisplayHierarchy, temp);

		// Must have a name
		cdstring name;
		if (!xmllib::XMLObject::ReadValue(xmlnode, cXMLElement_name, name))
			return;
		SetName(name);

		// Get display name details
		xmllib::XMLObject::ReadValue(xmlnode, cXMLElement_displayname, mDisplayName);
		
		// Get last sync
		xmllib::XMLObject::ReadValue(xmlnode, cXMLElement_lastsync, mLastSync);
	}

	// Scan into directories
	if (IsDirectory())
	{
		for(xmllib::XMLNodeList::const_iterator iter = xmlnode->Children().begin(); iter != xmlnode->Children().end(); iter++)
		{
			// Check child name
			xmllib::XMLNode* child = *iter;
			if (child->CompareFullName(cXMLElement_adbknode))
			{
				// Create new cal store node
				CAddressBook* node = new CAddressBook(GetProtocol(), this);
				
				// Parse it
				node->ReadXML(child);
				
				// Add it to this one
				AddChild(node);
			}
		}
		
		// Always mark node as having been expanded
		SetHasExpanded(true);
		
		// Always sort the children after adding all of them
		SortChildren();
	}
}
