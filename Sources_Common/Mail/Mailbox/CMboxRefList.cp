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


// CMboxRefList :: class to handle flat list of mbox references

#include "CMboxRefList.h"

#include "CINETCommon.h"
#include "CMailAccountManager.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMboxRef.h"

#include <algorithm>

// Default constructor
CMboxRefList::CMboxRefList(const char* name)
	: CTreeNodeList(name)
{
	mFlags.Set(eHierarchic, false);
	mFlags.Set(eAdjustDelete);			// Must remove directories with no children
}

// Add mailbox to list
CMboxNode* CMboxRefList::AddMbox(const CMboxNode* node)
{
	// Multiple threads can modify this object
	cdmutex::lock_cdmutex _lock(mMutex);

	// Determine type
	const CMbox* mbox = dynamic_cast<const CMbox*>(node);
	const CMboxRef* ref = dynamic_cast<const CMboxRef*>(node);
	const CWildcardMboxRef* wildref = dynamic_cast<const CWildcardMboxRef*>(node);
	if (wildref)
		ref = NULL;

	// Do not add if already present
	if (ContainsMbox(node))
		return NULL;

	CMboxRef* new_item = NULL;
	if (mbox)
		new_item = new CMboxRef(mbox);
	else if (ref)
		new_item = new CMboxRef(*ref);
	else if (wildref)
		new_item = new CWildcardMboxRef(*wildref);

	return static_cast<CMboxNode*>(AddNode(new_item));
}

// Does it contain matching mbox
CMboxRef* CMboxRefList::FindMbox(const CMboxNode* node) const
{
	unsigned long index;
	return (FindPos(node, index) ? static_cast<CMboxRef*>(at(index)) : NULL);
}

// Does it contain matching mbox
bool CMboxRefList::FindPos(const CMboxNode* node, unsigned long& pos) const
{
	// Multiple threads can modify this object
	cdmutex::lock_cdmutex _lock(mMutex);

	// Determine type
	const CMbox* mbox = dynamic_cast<const CMbox*>(node);
	const CMboxRef* ref = dynamic_cast<const CMboxRef*>(node);
	const CWildcardMboxRef* wildref = dynamic_cast<const CWildcardMboxRef*>(node);
	if (wildref)
		ref = NULL;

	// Does it already exist
	CMboxRef* found = NULL;
	pos = 0;
	for(const_iterator iter = begin(); !found && (iter != end()); iter++, pos++)
	{
		// NB Need to discriminate between CMboxRef and CWildcardMboxRef as they
		// have different operator== methods which are NOT virtual!
		const CMboxRef* ref_comp = dynamic_cast<const CMboxRef*>(*iter);
		const CWildcardMboxRef* wildref_comp = dynamic_cast<const CWildcardMboxRef*>(*iter);
		if (wildref_comp)
			ref_comp = NULL;
		if (mbox)
		{
			if (ref_comp && (*ref_comp == *mbox))
			{
				// Test whether change in directory state
				if (ref_comp->IsDirectory() ^ mbox->IsDirectory())
					found = NULL;
				else
					return true;
			}
			else if (wildref_comp && (*wildref_comp == *mbox))
				return true;
		}
		else if (ref)
		{
			if ((ref_comp && (*ref_comp == *ref)) ||
				(wildref_comp && (*dynamic_cast<const CMboxRef*>(node) == *ref)))
				return true;
		}
		else if (wildref)
		{
			if ((ref_comp && (*ref_comp == *wildref)) ||
				(wildref_comp && (*wildref_comp == *wildref)))
				return true;
		}
	}

	return false;
}

// Hierarchical rename
void CMboxRefList::RenamedMbox(const cdstring& old_name, const cdstring& new_name)
{
	// Multiple threads can modify this object
	cdmutex::lock_cdmutex _lock(mMutex);

	size_t old_name_len = old_name.length();

	CMboxRefList renames;

	// Look at every node!
	for(const_iterator iter = begin(); iter != end(); iter++)
	{
		// See if first part of name matches - do not do wildcards!
		if (!(*iter)->IsWildcard() && !::strncmp((*iter)->GetName(), old_name, old_name_len))
		{
			// Now check whether its a legitimate mailbox - either the mailbox itself
			// or a proper child
			if (((*iter)->GetName().length() == old_name_len) ||
				((*iter)->GetName()[old_name_len] == (*iter)->GetDirDelim()))
				renames.push_back(*iter);
		}
	}

	// Now rename the ones that match
	for(iterator iter = renames.begin(); iter != renames.end(); iter++)
	{
		cdstring rename = new_name;
		rename += &(*iter)->GetName().c_str()[old_name_len];
		CMailAccountManager::sMailAccountManager->RenameFavouriteItem(this, static_cast<CMboxRef*>(*iter), rename);
	}
}

// Delete the mailbox from the list
void CMboxRefList::RemoveMbox(CMboxNode* node, bool delete_it, bool resolve)
{
	// Multiple threads can modify this object
	cdmutex::lock_cdmutex _lock(mMutex);

	// Determine type
	CMbox* mbox = dynamic_cast<CMbox*>(node);
	CMboxRef* ref = dynamic_cast<CMboxRef*>(node);

	// Does it already exist
	if (mbox || resolve)
	{
		iterator found;
		for(found = begin(); found != end(); found++)
		{
			if ((mbox && (*dynamic_cast<CMboxRef*>(*found) == *mbox)) ||
				(ref && (*dynamic_cast<CMboxRef*>(*found) == *ref))) break;
		}

		// Remove it if found
		if (found != end())
			RemoveNode(*found, delete_it);
	}
	else
		RemoveNode(ref, delete_it);
}

// Get index (starts at 1)
unsigned long CMboxRefList::ResolveIndexOf(const CMbox* mbox) const
{
	// Multiple threads can modify this object
	cdmutex::lock_cdmutex _lock(mMutex);

	// Does it already exist
	unsigned long pos = 0;
	for(const_iterator iter = begin(); iter != end(); iter++)
	{
		pos++;
		if (*dynamic_cast<CMboxRef*>(*iter) == *mbox)
			return pos;
	}

	return 0;
}

void CMboxRefList::SyncList(const cdstrpairvect& items)
{
	// Multiple threads can modify this object
	cdmutex::lock_cdmutex _lock(mMutex);

	// Delete existing
	DeleteAll();

	// Add all
	for(cdstrpairvect::const_iterator iter = items.begin(); iter != items.end(); iter++)
	{
		// Look for wildcards
		if (::strpbrk((*iter).first.c_str(), cWILDCARD_ALL))
			push_back(new CWildcardMboxRef((*iter).first, *(*iter).second.c_str()));
		else
			push_back(new CMboxRef((*iter).first, *(*iter).second.c_str()));
	}

	// Sort if sorted
	if (IsSorted())
		Sort();

	AdjustDirectories();
}


CTreeNode* CMboxRefList::MakeHierarchy(const cdstring& name, const CTreeNode* child)
{
	CMboxRef* mbox_ref = new CMboxRef(name, child->GetDirDelim(), true);
	mbox_ref->SetHasInferiors(true);

	return mbox_ref;
}

void CMboxRefList::MakeHierarchic(CTreeNode* node, bool hier)
{
	// I do nothing!
}

// Compare nodes in list (true if node1 < node2)
bool CMboxRefList::CompareMboxRef(const CTreeNode* node1, const CTreeNode* node2)
{
	// Test for same mboxref first
	if (node1 == node2)
		return false;

	// Compare accounts first
	const char* acct1 = dynamic_cast<const CMboxRef*>(node1)->GetAccountOnlyName().c_str();
	const char* acct2 = dynamic_cast<const CMboxRef*>(node2)->GetAccountOnlyName().c_str();

	// If not same account do account ordering based on order in prefs
	if (::strcmp(acct1, acct2))
	{
		// Find order of accounts in prefs
		const CMboxProtocolList& proto_list = CMailAccountManager::sMailAccountManager->GetProtocolList();
		for(CMboxProtocolList::const_iterator iter = proto_list.begin(); iter != proto_list.end(); iter++)
		{
			const char* acct_name = (*iter)->GetAccountName().c_str();
			if (::strcmp(acct_name, acct1) == 0)
				return true;
			else if (::strcmp(acct_name, acct2) == 0)
				return false;
		}

		return (::strcmp(acct1, acct2) < 0);
	}

	// Look for wildcards
	bool wild1 = dynamic_cast<const CMboxRef*>(node1)->IsWildcard();
	bool wild2 = dynamic_cast<const CMboxRef*>(node2)->IsWildcard();

	// Float wildcards to top
	if (wild1 ^ wild2)
		return wild1;

	// Compare wildcards as strings
	if (wild1 && wild2)
		return (::strcmp(node1->GetName().c_str(), node2->GetName().c_str()) < 0);

	// At this point the nodes must be ordinary (non-wildcard) mailbox references in the same account

	// Punt past leading account name for comparison
	size_t acct_len = ::strlen(acct1);
	acct_len = (acct_len ? acct_len + 1 : 0);
	const char* p1 = node1->GetName().c_str() + acct_len;
	const char* p2 = node2->GetName().c_str() + acct_len;

	// Do inherited version of compare
	return CompareNodeNames(p1, p2, node1->GetDirDelim());
}

