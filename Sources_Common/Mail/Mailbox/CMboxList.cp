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


// Source for CMboxList class

#include "CMboxList.h"

#include "CINETCommon.h"
#include "CMailAccount.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMboxRef.h"

#include <algorithm>

// Default constructor
CMboxList::CMboxList()
{
	InitMboxList();
}

// Default constructor
CMboxList::CMboxList(CMboxProtocol* proto) :
	CTreeNodeList((const char*) NULL)
{
	InitMboxList();
	mProtocol = proto;
	mFlags.Set(eOwnsNodes);
}

// Default constructor
CMboxList::CMboxList(CMboxProtocol* proto, const char* root) :
	CTreeNodeList(root)
{
	InitMboxList();

	mProtocol = proto;
	mFlags.Set(eOwnsNodes);
}

// Default destructor
CMboxList::~CMboxList()
{
	mProtocol = NULL;
}


// Default constructor
void CMboxList::InitMboxList()
{
	mProtocol = nil;
}

long CMboxList::GetHierarchyIndex() const
{
	return mProtocol->GetHierarchyIndex(this);
}

// Can this root be used for inferiors?
bool CMboxList::CanCreateInferiors() const
{
	// Count '%' & '*' wildcards
	const char* p = mName.c_str();
	int ctr = 0;
	while(*p)
	{
		if ((*p == *cWILDCARD) || (*p == *cWILDCARD_NODIR))
		{
			if (ctr) return false;
			ctr++;
		}
		p++;
	}

	if (ctr)
	{
		char c = *--p;
		if ((c != *cWILDCARD) && (c != *cWILDCARD_NODIR))
			return false;
		else
			return (*--p == GetDirDelim());
	}
	else if (!mName.empty())
	{
		return (*--p == GetDirDelim());
	}
	else
		return false;
}

// Needs hierarchy descovery
bool CMboxList::NeedsHierarchyDescovery() const
{
#if 0
	// Iterate over each mailbox to find one that has not expanded
	for(const_iterator iter = begin(); iter != end(); iter++)
	{
		if (!(*iter)->NoInferiors() && !(*iter)->HasExpanded())
			return true;
	}

	return false;
#else
	return true;
#endif
}

// Recursively search cache for mbox with requested name
CMbox* CMboxList::FindMbox(const char* mbox_name) const
{
	// Search all entries
	if (!mbox_name) return nil;

	for(const_iterator iter = begin(); iter != end(); iter++)
	{
		if ((*iter)->GetName() == mbox_name)
			return (CMbox*) *iter;
	}

	// Not found in list
	return nil;
}

// Recursively search cache for mbox with requested url
CMbox* CMboxList::FindMboxURL(const char* url) const
{
	// Search all entries
	if (!url || !*url) return nil;

	for(const_iterator iter = begin(); iter != end(); iter++)
	{
		cdstring mbox_url = ((CMbox*) *iter)->GetURL();
		if (mbox_url == url)
			return (CMbox*) *iter;
	}

	// Not found in list
	return nil;
}

// Return all mboxes matching pattern
void CMboxList::FindWildcard(const cdstring& pattern, CMboxList& list, bool no_dir) const
{
	// Do a quick test to see if we can reject the entire hierarchy rather than always
	// comparing against all mailboxes. The policy here is to try to match the root name
	// against the pattern provided no wildcards appear within both strings up to the
	// length of the root name.

	// Look for wildcards in root
	if (!::strpbrk(mName.c_str(), cWILDCARD_ALL))
	{
		size_t name_len = mName.length();

		// Look at first part of pattern
		const char* p = ::strpbrk(pattern.c_str(), cWILDCARD_ALL);

		// If no wildcard within root name length then do root length compare and reject
		if (!p || (p - pattern.c_str() >= name_len))
		{
			// If no match then reject this entire hierarchy
			if (::strncmp(mName.c_str(), pattern.c_str(), name_len))
				return;
		}
	}

	// Must compare against all mailboxes
	for(const_iterator iter = begin(); iter != end(); iter++)
	{
		if ((*iter)->GetName().PatternDirMatch(pattern, GetDirDelim()) &&
			!(no_dir && (*iter)->IsDirectory()))
			list.push_back(*iter);
	}
}


// Recursively search cache for mbox with requested name
long CMboxList::FetchIndexOf(const CMbox* mbox) const
{
	const_iterator found = std::find(begin(), end(), static_cast<const CTreeNode*>(mbox));

	if (found != end())
		return found - begin();
	else
		return -1;
}

// Close all mailboxes in the list (silently)
void CMboxList::CloseAll()
{
	// WARNING: if its the singleton list the items will be removed when closed
	// Need to protect against erase during this process by reverse iterating
	for(reverse_iterator riter = rbegin(); riter != rend(); riter++)
		static_cast<CMbox*>(*riter)->CloseSilent();
}

// Remove duplicate items
void CMboxList::RemoveDuplicates()
{
	// First sort it
	Sort();

	// Now remove duplicates
	iterator iter1 = begin();
	iterator iter2 = begin();
	if (iter2 != end())
	{
		iter2++;
		while(iter2 != end())
		{
			if (static_cast<CMbox*>(*iter1)->GetAccountName() == static_cast<CMbox*>(*iter2)->GetAccountName())
				iter2 = erase(iter2);
			else
				iter2++;
		}
	}
}

void CMboxList::DumpHierarchy(std::ostream& out)
{
	// Write dir delim first
	out << GetDirDelim() << std::endl;

	// Must compare against all mailboxes
	for(const_iterator iter = begin(); iter != end(); iter++)
	{
		CMbox* mbox = static_cast<CMbox*>(*iter);

		// Write name then flags
		out << mbox->GetName() << std::endl;
		out << (mbox->GetFlags() &
					(NMbox::eNoSelect |
					 NMbox::eNoInferiors |
					 NMbox::eMarked |
					 NMbox::eUnMarked)) << std::endl;
	}

	// Always end with blank line
	out << std::endl;
}

void CMboxList::ReadHierarchy(std::istream& in, CMboxProtocol* local, bool add_items)
{
	// Read dir delim
	cdstring delim;
	getline(in, delim);
	if (add_items)
		SetDirDelim(*delim.c_str());

	while(true)
	{
		// Get mailbox name
		cdstring mbox_name;
		getline(in, mbox_name);

		// Check for end of hierarchy
		if (mbox_name.empty())
			break;

		// Get flags
		unsigned long flags;
		in >> flags;
		in.ignore();

		// Add as mailbox if required
		if (add_items)
		{
			CMbox* mbox = new CMbox(mProtocol, mbox_name, GetDirDelim(), this, GetHierarchyIndex() == 0);
			mbox->SetFlags(static_cast<NMbox::EFlags>(flags));

			// Now convert to local mbox
			mbox->SwitchDisconnect(local);

			// Add to its list - maybe deleted if duplicate
			mbox = mbox->AddMbox();
		}
	}
}

bool CMboxList::IsRootName(char dir) const
{
	// Count '%' & '*' wildcards
	const char* p = mName.c_str();
	int ctr = 0;
	while(*p)
	{
		if ((*p == *cWILDCARD) || (*p == *cWILDCARD_NODIR))
		{
			if (ctr) return false;
			ctr++;
		}
		p++;
	}

	if (ctr)
	{
		char c = *--p;
		return (c == *cWILDCARD) || (c == *cWILDCARD_NODIR);
	}
	else if (!mName.empty())
	{
		return (*--p == dir) || !dir;
	}
	else
		return false;
}

CTreeNode* CMboxList::MakeHierarchy(const cdstring& name, const CTreeNode* child)
{
	// Create it but don't add it
	CMbox* mbox = new CMbox(GetProtocol(), name, child->GetDirDelim(), static_cast<const CMbox*>(child)->GetMboxList(), mName.empty());

	// NB If making hierarchy there must be children so this must have expanded!
	mbox->SetFlags((NMbox::EFlags) (NMbox::eNoSelect | NMbox::eHasInferiors | NMbox::eHasExpanded));
	mbox->SetFlags(NMbox::eNoInferiors, false);

	return mbox;
}

void CMboxList::MakeHierarchic(CTreeNode* node, bool hier)
{
	// Mark it as expanded since it has children
	static_cast<CMbox*>(node)->SetFlags(NMbox::eHasExpanded, hier);
}

void CMboxList::Add(CMboxProtocol* proto, bool not_dirs)
{
	if (proto == NULL)
		return;

	// Add INBOX if present
	if (proto->GetINBOX())
		push_back(proto->GetINBOX());
	
	// Add each hierarchy
	for(CHierarchies::const_iterator iter = proto->GetHierarchies().begin(); iter != proto->GetHierarchies().end(); iter++)
	{
		// Add each mailbox in the hierarchy
		Add(*iter, not_dirs);
	}
}

void CMboxList::Add(CMboxList* list, bool not_dirs)
{
	if (list == NULL)
		return;

	// Add each mailbox in the hierarchy
	for(CMboxList::const_iterator iter = list->begin(); iter != list->end(); iter++)
	{
		CMbox* mbox = static_cast<CMbox*>(*iter);
		Add(mbox, not_dirs);
	}
}

void CMboxList::Add(CMboxRefList* list, bool not_dirs)
{
	if (list == NULL)
		return;

	for(CMboxRefList::iterator iter = list->begin(); iter != list->end(); iter++)
	{
		CMboxRef* mboxref = static_cast<CMboxRef*>(*iter);
		Add(mboxref, not_dirs);
	}
}

void CMboxList::Add(CMbox* mbox, bool not_dirs)
{
	if (mbox == NULL)
		return;

	// Add each mailbox in the hierarchy
	if (!(not_dirs && mbox->IsDirectory()))
		push_back(mbox);
}

void CMboxList::Add(CMboxRef* mbox_ref, bool not_dirs)
{
	if (mbox_ref == NULL)
		return;

	// Check specifically for wildcard
	if (mbox_ref->IsWildcard())
	{
		// Resolve to entire list
		static_cast<const CWildcardMboxRef*>(mbox_ref)->ResolveMbox(*this, true, not_dirs);
	}
	else if (!(not_dirs && mbox_ref->IsDirectory()))
	{
		// Just get a single match
		CMbox* found = static_cast<const CMboxRef*>(mbox_ref)->ResolveMbox(false);
		if (found)
			push_back(found);
	}
}

