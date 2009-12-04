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


// Source for CTreeNodeList class

#include "CTreeNodeList.h"

#include "CINETCommon.h"
#include "CMailAccount.h"
#include "CMbox.h"
#include "CMboxRef.h"
#include "CMboxRefList.h"
#include "CTreeNode.h"
#include "cdstring.h"

#include <algorithm>
#include <typeinfo>

#pragma mark ____________________________Specializations

#if 0
typedef bool (*voidCompare)(const void*, const void*);
void stable_sort (CTreeNodeList::iterator first, CTreeNodeList::iterator last, CompareNodePP comp);
void stable_sort (CTreeNodeList::iterator first, CTreeNodeList::iterator last, CompareNodePP comp)
{
	stable_sort(reinterpret_cast<vector<void*>::iterator>(first),
					reinterpret_cast<vector<void*>::iterator>(last),
					reinterpret_cast<voidCompare>(comp));
}

CTreeNodeList::iterator upper_bound (CTreeNodeList::iterator first, CTreeNodeList::iterator last, const CTreeNode*& value, CompareNodePP comp);
CTreeNodeList::iterator upper_bound (CTreeNodeList::iterator first, CTreeNodeList::iterator last, const CTreeNode*& value, CompareNodePP comp)
{
	return
		reinterpret_cast<CTreeNodeList::iterator>(upper_bound(
				reinterpret_cast<vector<void*>::iterator>(first),
				reinterpret_cast<vector<void*>::iterator>(last),
				reinterpret_cast<const void*&>(value),
				reinterpret_cast<voidCompare>(comp)));
}
#endif

#pragma mark ____________________________CTreeNodeList

// Default constructor
CTreeNodeList::CTreeNodeList()
{
	InitTreeNodeList();

	// Created without name => does not own nodes
	mFlags.Set(eOwnsNodes, false);

	// Must not broadcast changes
	Stop_Broadcasting();
}

// Default constructor
CTreeNodeList::CTreeNodeList(const char* name)
{
	InitTreeNodeList();

	mFlags.Set(eOwnsNodes);
	mName = name;
}

CTreeNodeList::CTreeNodeList(bool owns_nodes)
{
	InitTreeNodeList();

	mFlags.Set(eOwnsNodes, owns_nodes);

	// Must not broadcast changes
	Stop_Broadcasting();
}

// Default destructor
CTreeNodeList::~CTreeNodeList()
{
	// Delete all mailboxes if owned
	if (mFlags.IsSet(eOwnsNodes))
		DeleteAll();
}


// Default constructor
void CTreeNodeList::InitTreeNodeList()
{
	mFlags.Set(eOwnsNodes | eHierarchic | eVisible);

	mDoDescovery = true;

	mDirDelim = 0;
}

cdstring CTreeNodeList::GetRootName() const
{
	cdstring rname = GetName();

	// Look for roots ending with wildcards
	if (rname.length() > 0)
	{
		char& c = rname[rname.length() - 1];

		if ((c == *cWILDCARD) || (c == *cWILDCARD_NODIR))
			c = 0;
	}

	return rname;
}

void CTreeNodeList::SetFlag(ETreeNodeListFlags flag, bool set)
{
	bool do_sort = false;
	bool do_adjust = false;
	bool do_visible = false;

	// Check for specific changes
	switch(flag)
	{
	case eSorted:
		do_sort = (set && !mFlags.IsSet(eSorted));
		break;
	case eHierarchic:
		do_adjust = (set ^ mFlags.IsSet(eHierarchic));
		break;
	case eVisible:
		do_visible = (set ^ mFlags.IsSet(eVisible));

		// Must broadcast before state change
		if (do_visible && !set)
			Broadcast_Message(eBroadcast_HideList, this);
		break;
	default:;
	}

	// Set flags themselves
	mFlags.Set(flag, set);

	if (do_sort)
		Sort();

	if (do_adjust)
		AdjustDirectories();

	if (do_visible && set)
		Broadcast_Message(eBroadcast_ShowList, this);
}

// Add mailbox to list (unsorted)
CTreeNode* CTreeNodeList::AddNode(CTreeNode* node)
{
	return AddNode(node, GetInsertPosition(node));
}

// Add mailbox to list (unsorted)
CTreeNode* CTreeNodeList::AddNode(CTreeNode* node, unsigned long index)
{
	return AddNode(node, begin() + index);
}

// Add mailbox to list (unsorted)
CTreeNode* CTreeNodeList::AddNode(CTreeNode* node, iterator pos)
{
	// Do not add directories when list is flat
	if (!IsHierarchic() && node->IsDirectory())
	{
		delete node;
		node = NULL;
		return node;
	}

	// Tweak for memory consumption peformance
	if (capacity() == size())
	{
		// Must keep iterator in sync with changes
		difference_type old_pos = pos - begin();
		reserve(size() + 1024);
		pos = begin() + old_pos;
	}

	// Special for first item
	pos = insert(pos, node);

	// Must do directory adjustment
	if (mFlags.IsSet(eLoaded))
	{
		if (IsHierarchic())
		{
			char last_dir = 0;

			// Stack start and lowest expand already cached
			cdstring stack = mStack;
			size_t stack_len = stack.length();
			unsigned long level = 0;
			const char* previous = cdstring::null_str;
			bool hijack = false;	// Indicates hijacking of sub-hierarchy!
			iterator prev = end();	// Node to hijack

			// Adjust to previous mbox
			if (pos != begin())
			{
				prev = pos - 1;
				stack = (*prev)->GetName();
				last_dir = (*prev)->GetDirDelim();
				if (!last_dir) last_dir = '\r';
				char* p = ::strrchr(stack.c_str_mod(), last_dir);
				if (p)
					*++p = 0;
				else
					stack = cdstring::null_str;
				stack_len = stack.length();
				level = (*prev)->GetWDLevel();
				previous = (*prev)->GetName().c_str();

				// Check for hijacking
				if (node->GetName() == previous)
					hijack = true;
				else
				{
					// Check for additional trailing delimiter
					cdstring temp = previous;
					temp += (*prev)->GetDirDelim();
					if (node->GetName() == temp)
					{
						hijack = true;
						node->SetName(previous);
					}
				}
			}

			// If hijacked replace current node's flags
			if (hijack)
			{
				unsigned long change_pos = prev - begin();

				// Determine type
				if (dynamic_cast<CMbox*>(*prev))
				{
					// Remove existing IMAP flags (only) first then add in new ones
					dynamic_cast<CMbox*>(*prev)->SetFlags(eIMAPFlags, false);
					dynamic_cast<CMbox*>(*prev)->SetFlags(static_cast<NMbox::EFlags>(dynamic_cast<CMbox*>(node)->GetFlags() & NMbox::eIMAPFlags));
				}
				else if (dynamic_cast<CMboxRef*>(*prev))
				{
					// Check for directory -> mailbox change
					dynamic_cast<CMboxRef*>(*prev)->SetDirectory(node->IsDirectory());
				}

				// Delete new one and remove it
				delete node;
				node = NULL;
				erase(pos);

				// Reset to the one doing the hijack
				node = *prev;

				// Tell visual rep of change
				SBroadcastChangeNode info;
				info.mList = this;
				info.mIndex = change_pos;
				Broadcast_Message(eBroadcast_ChangeNode, &info);
			}
			else
			{
				// Adjust structure
				size_t num_insert = AdjustNodeStructure(pos, stack, stack_len, level, previous, last_dir, typeid(*this) == typeid(CMboxRefList));

				pos -= num_insert;

				// Make sure previous mailbox knows it has children
				if (pos != begin())
				{
					iterator prev = pos - 1;
					if ((*prev)->GetWDLevel() < (*pos)->GetWDLevel())
						(*prev)->SetHasInferiors(true);
				}

				// Broadcast change
				SBroadcastAddNode info;
				info.mList = this;
				info.mIndex = pos - begin();
				info.mNumber = num_insert + 1;
				Broadcast_Message(eBroadcast_AddNode, &info);
			}
		}
		else if (!node->IsDirectory())
		{
			// Reset WD level to 0
			node->SetWDLevel(0);

			// Never has inferiors
			node->SetHasInferiors(false);
			MakeHierarchic(node, true);

			// Broadcast change
			SBroadcastAddNode info;
			info.mList = this;
			info.mIndex = pos - begin();
			info.mNumber = 1;
			Broadcast_Message(eBroadcast_AddNode, &info);
		}
	}

	return node;
}

// Delete the node from the list
void CTreeNodeList::RemoveNode(CTreeNode* node, bool delete_it)
{
	iterator found = std::find(begin(), end(), node);

	if (found != end())
	{
		unsigned long pos = found - begin();
		unsigned long level = (*found)->GetWDLevel();
		bool deleted = false;

		// NB Never delete children - just convert to directory
		iterator child = found + 1;
		if ((child != end()) && ((*child)->GetWDLevel() > level))
		{
#if 0
			// Must delete all children
			while((child != end()) && ((*child)->GetWDLevel() > level))
			{
				if (delete_it)
					delete *child;
				child++;
			}

			// Delete before erase
			if (delete_it)
				delete *found;

			// Erase it and all its children
			erase(found, child);
			
			deleted = true;
#endif
			CMboxRef* ref = dynamic_cast<CMboxRef*>(*found);
			CMbox* mbox = dynamic_cast<CMbox*>(*found);
			if (mbox)
				mbox->SetFlags(NMbox::eNoSelect, true);
			else
				ref->SetDirectory(true);
		}
		else
		{
			// Delete before erase
			if (delete_it)
				delete *found;

			// Erase it and all its children
			erase(found);
			
			deleted = true;
		}

		// Check for change to hierarchy
		if (deleted)
		{
			if (pos)
			{
				CTreeNode* prev = at(pos - 1);
				if (prev->GetWDLevel() < level)
				{
					if ((pos < size()) && (at(pos)->GetWDLevel() != level) || (pos == size()))
						prev->SetHasInferiors(false);
				}

				// Must adjust for directories without children
				if (mFlags.IsSet(eAdjustDelete))
				{
					while(prev->IsDirectory() && (prev->GetWDLevel() < level) && !prev->HasInferiors())
					{
						// Get WD level of previous
						level = prev->GetWDLevel();

						// delete and erase previous
						if (delete_it)
							delete prev;
						erase(--found);

						// Get the next one up (finish if no more)
						if (!--pos)
							break;

						prev = at(pos - 1);
						if (prev->GetWDLevel() < level)
						{
							if ((pos < size()) && (at(pos)->GetWDLevel() != level) || (pos == size()))
								prev->SetHasInferiors(false);
						}
					}
				}
			}

			// Tell visual rep of change
			SBroadcastDeleteNode info;
			info.mList = this;
			info.mIndex = pos;
			Broadcast_Message(eBroadcast_DeleteNode, &info);
		}
		else
		{
			// Tell visual rep of change
			SBroadcastChangeNode info;
			info.mList = this;
			info.mIndex = pos;
			Broadcast_Message(eBroadcast_ChangeNode, &info);
		}
	}
}

// Mailbox changed - just notify
void CTreeNodeList::ChangedNode(CTreeNode* node)
{
	// Must exist in this list
	iterator found = std::find(begin(), end(), node);
	if (found == end())
		return;

	// Tell visual rep of change
	SBroadcastChangeNode info;
	info.mList = this;
	info.mIndex = found - begin();
	Broadcast_Message(eBroadcast_ChangeNode, &info);
}

// Mailbox changed - resort
void CTreeNodeList::NodeRenamed(CTreeNode* node, const cdstring& new_name)
{
	// This is only called as a result of a rename so do
	// full hierarchical rename here.

	cdstring old_name = node->GetName();
	size_t old_name_len = old_name.length();

	// Now rename the one root node
	node->SetName(new_name);

	// Iterate over all child nodes and change their names
	iterator found = std::find(begin(), end(), node);
	if (found == end())
		return;

	// If not hierarchic append a dir delim for comparison
	if (!IsHierarchic())
	{
		old_name += mDirDelim;
		old_name_len++;
	}

	while(++found != end())
	{
		// Different if hierarchic or not
		if (IsHierarchic())
		{
			// See if WD level is one down
			if ((*found)->GetWDLevel() > node->GetWDLevel())
			{
				cdstring rename = new_name;
				rename += &(*found)->GetName().c_str()[old_name_len];
				(*found)->SetName(rename);
			}
			else
				// No more children - break out of loop
				break;
		}
		else
		{
			// Do string compare of old name
			if (!::strncmp((*found)->GetName(), old_name, old_name_len))
			{
				cdstring rename = new_name;
				rename += &(*found)->GetName().c_str()[old_name_len];
				(*found)->SetName(rename);
			}
			else
				// No more children - break out of loop
				break;
		}
	}

	// Note at this point the names of all the nodes have already been changed

	// As a quick fix do : resort and list reset
	Sort();
	AdjustDirectories();

#if 0
	// Remove from list
	RemoveNode(node, false);

	// Add changed node back into list
	AddNode(node);
#endif
}

// Get parent
CTreeNode* CTreeNodeList::GetParentNode(const CTreeNode* node)
{
	// Can only do if hierarchic and sorted
	if (!IsHierarchic() || !IsSorted())
		return NULL;

	// Must exist in this list
	iterator found = std::find(begin(), end(), node);
	if (found == end())
		return NULL;

	// Check WD Level
	unsigned long wd_level = node->GetWDLevel();

	if (wd_level)
	{
		// Step back looking for next lower WD
		while(((*--found)->GetWDLevel() != wd_level - 1) && (found != begin())) {}

		// Got it
		return *found;
	}
	else
		return NULL;
}

// Get the first child
CTreeNode* CTreeNodeList::GetFirstChild(const CTreeNode* node)
{
	// Can only do if hierarchic and sorted
	if (!IsHierarchic() || !IsSorted())
		return NULL;

	// Must exist in this list
	iterator found = std::find(begin(), end(), node);
	if (found == end())
		return NULL;

	// See if next item has lower WD
	if ((++found != end()) && ((*found)->GetWDLevel() == node->GetWDLevel() + 1))
		return *found;
	else
		return NULL;
}

// Get next sibling
CTreeNode* CTreeNodeList::GetNextSibling(const CTreeNode* node)
{
	// Can only do if hierarchic and sorted
	if (!IsHierarchic() || !IsSorted())
		return NULL;

	// Must exist in this list
	iterator found = std::find(begin(), end(), node);
	if (found == end())
		return NULL;

	// Check WD Level
	unsigned long wd_level = node->GetWDLevel();

	if (wd_level)
	{
		// Step forward looking for same WD
		while((++found != end()) && ((*found)->GetWDLevel() >= wd_level))
		{
			if ((*found)->GetWDLevel() == wd_level)
				return *found;
		}
	}

	// Failed to find
	return NULL;
}

// Get index (starts at 1)
unsigned long CTreeNodeList::FetchIndexOf(const CTreeNode* node) const
{
	const_iterator found = std::find(begin(), end(), node);

	if (found != end())
		return (found - begin()) + 1;
	else
		return 0;
}

// Delete all mailboxes from the list
void CTreeNodeList::DeleteAll()
{
	// Delete all in list
	for(iterator iter = begin(); iter != end(); iter++)
		delete *iter;

	// Empty list
	clear();

	// Set flag
	mFlags.Set(eLoaded, false);

	// Force visual update
	Broadcast_Message(eBroadcast_ResetList, this);
}

// Sort mailboxes in list
void CTreeNodeList::Sort()
{
	// Sort all
	std::stable_sort(begin(), end(), GetCompareFN());
}

// Compare nodes in list (true if node1 < node2)
bool CTreeNodeList::CompareNode(const CTreeNode* node1, const CTreeNode* node2)
{
	// Test for same mailbox first
	if (node1 == node2)
		return false;

	// Do comparison but float dir delim chars to top
	return CompareNodeNames(node1->GetName().c_str(), node2->GetName().c_str(), node1->GetDirDelim());
}

// Compare nodes in list (true if node1 < node2)
bool CTreeNodeList::CompareNodeNames(const char* node1, const char* node2, char delim)
{
	// Test for same mailbox first
	if (node1 == node2)
		return false;

	// Do comparison but float dir delim chars to top
	unsigned char dir = delim;
	if (!dir) dir = '\r';

#if !__POWERPC__

	const unsigned char * p1 = (unsigned char *) node1;
	const unsigned char * p2 = (unsigned char *) node2;
	unsigned char c1, c2;
	unsigned char cc1= 0;
	unsigned char cc2 = 0;
	bool case_off = false;

	while ((c1 = tolower(*p1)) == (c2 = tolower(*p2)))
	{
		if (!c1)
			// If equal do case sensitive compare
			return (cc1 < cc2);

		if (!case_off && (*p1 != *p2))
		{
			cc1 = *p1++;
			cc2 = *p2++;
			case_off = true;
		}
		else if (case_off && (c1 == dir))
			return (cc1 < cc2);
		else
		{
			p1++;
			p2++;
		}
	}

#else

	const unsigned char * p1 = (unsigned char *) node1 - 1;
	const unsigned char * p2 = (unsigned char *) node2 - 1;
	unsigned long c1, c2;
	unsigned char cc1= 0;
	unsigned char cc2 = 0;
	bool case_off = false;

	while ((c1 = ::tolower(*++p1)) == (c2 = ::tolower(*++p2)))
	{
		if (!c1)
			// If equal do case sensitive compare
			return (cc1 < cc2);

		if (!case_off && (*p1 != *p2))
		{
			cc1 = *p1;
			cc2 = *p2;
			case_off = true;
		}
		else if (case_off && (c1 == dir))
			return (cc1 < cc2);
		else
		{
		}
	}

#endif

	if (c1 == dir)
		return (c2 ? true : (case_off ? (cc1 < cc2) : false));
	else if (c2 == dir)
		return (c1 ? false : (case_off ? (cc1 < cc2) : true));
	else
		return c1 < c2;
}

// Get insert position of mbox
CTreeNodeList::iterator CTreeNodeList::GetInsertPosition(const CTreeNode* node)
{
	iterator pos;

	// Determine position based on sorting
	if (mFlags.IsSet(eSorted))
	{
		// Find new sorted insert position
		// Use upper bound so that insertion will hijack the sub-hierarchy
		// of any existing similarly named node
		pos = std::upper_bound(begin(), end(), node, GetCompareFN());
	}
	else
	{
		// Find new insert position
		pos = end();
	}

	return pos;
}

// Adjust directories for changes to list
void CTreeNodeList::AdjustDirectories()
{
	// Only bother if something found
	if (size())
	{
		if (IsHierarchic())
			AdjustHierarchy();
		else
			AdjustFlat();
	}
	else if (IsHierarchic())
	{
		// Must calculate stack even if empty
		char dir = GetDirDelim();
		if (!dir) dir = '\r';
		CalculateStackStart(dir);
	}

	// Set flag
	mFlags.Set(eLoaded, true);

	// Force visual update
	Broadcast_Message(eBroadcast_ResetList, this);
}

// Adjust to flat list
void CTreeNodeList::AdjustFlat()
{
	// Process all
	for(iterator iter = begin(); iter != end(); )
	{
		// Remove any directories
		if ((*iter)->IsDirectory())
		{
			delete *iter;
			iter = erase(iter);
			continue;
		}
		else
		{
			// Reset WD level to 0
			(*iter)->SetWDLevel(0);

			// Never has inferiors and always expanded
			(*iter)->SetHasInferiors(false);
			MakeHierarchic(*iter, true);
		}
		
		iter++;
	}
}

// Calculate stack start and lowest expand
void CTreeNodeList::CalculateStackStart(char dir)
{
	// Adjust stack start based on root
	mStack = cdstring::null_str;
	if (IsRootName(dir))
	{
		// Stack starts at root (without wildcards)
		mStack = mName;
		char* p = mStack.length() ? &mStack.c_str_mod()[mStack.length() - 1] : cdstring::null_str.c_str_mod();
		char c = *p;
		if ((c == *cWILDCARD) || (c == *cWILDCARD_NODIR))
			*p = 0;

		if (c == *cWILDCARD)
			mDoDescovery = false;
		else if (c == *cWILDCARD_NODIR)
			mDoDescovery = true;
		else
			mDoDescovery = true;
	}
	else
		mDoDescovery = false;
}

// Adjust to hierarchic list
void CTreeNodeList::AdjustHierarchy()
{
	// Check for inferiors
	for(iterator iter = begin(); iter != end(); iter++)
	{
		// Can it have inferiors?
		if ((*iter)->IsWildcard())
			(*iter)->SetHasInferiors(false);

		else if ((*iter)->IsHierarchy() && (iter != (end() - 1)))
		{
			// Check name against next mbox
			const cdstring* node_name = &(*iter)->GetName();
			size_t len = node_name->length();

			const cdstring* next_node_name = &(*(iter+1))->GetName();

			// Check for duplicate nodes here and delete the second
			while(*next_node_name == *node_name)
			{
				erase(iter + 1);
				if (iter != (end() - 1))
					next_node_name = &(*(iter+1))->GetName();
				else
					next_node_name = &cdstring::null_str;
			}

			if (next_node_name->empty())
				continue;

			if ((::strncmp(*node_name, *next_node_name, len) == 0) &&
				((*next_node_name)[len] == (*iter)->GetDirDelim()))
				(*iter)->SetHasInferiors(true);
			else
				(*iter)->SetHasInferiors(false);
		}
		else if (iter != (end() - 1))
			(*iter)->SetHasInferiors(false);
	}

	// Adjust stack start based on root
	char dir = GetDirDelim();
	if (!dir) dir = '\r';
	CalculateStackStart(dir);
	cdstring stack = mStack;
	size_t stack_len = stack.length();
	unsigned long level = 0;
	const char* previous = cdstring::null_str;

	// Check that very first node is not equal to stack
	if (size())
	{
		bool remove = false;

		if (front()->IsDirectory())
		{
			cdstring temp = front()->GetName();
			temp += front()->GetDirDelim();
			if (temp == stack)
				remove = true;
		}
		else if (front()->GetName() == stack)
			remove = true;

		if (remove)
		{
			// Remove it from the list
			delete *begin();
			erase(begin());
		}
	}

	// Look at each node
	bool account_prefix = (typeid(*this) == typeid(CMboxRefList));
	char last_dir = dir;
	for(iterator iter = begin(); iter != end(); iter++)
	{
		AdjustNodeStructure(iter, stack, stack_len, level, previous, last_dir, account_prefix);

		// reset previous name if not wildcard
		if (!(*iter)->IsWildcard())
			previous = (*iter)->GetName();
	}
}

size_t CTreeNodeList::AdjustNodeStructure(iterator& iter, cdstring& stack, size_t& stack_len, unsigned long& level,
											const char* previous, char& last_dir, bool account_prefix)
{
	size_t num_insert = 0;
	char dir = (*iter)->GetDirDelim();
	if (!dir) dir = '\r';

	// Special for wildcards
	if ((*iter)->IsWildcard())
	{
		(*iter)->SetWDLevel(0);
		last_dir = dir;
		return 0;
	}

	const cdstring& node_name = (*iter)->GetName();
	
	// Check match with current hierarchy
	if (::strncmp(stack.c_str(), node_name.c_str(), stack_len) == 0)
	{
		// Check for sub-hierarchy
		const char* rest = &node_name.c_str()[stack_len];

		// Get position of any hierarchy
		const char* rest_dir = ::strchr(rest, dir);

		// Take accounts into account
		if (account_prefix && (rest_dir <= ::strchr(rest, cMailAccountSeparator)))
			rest_dir = NULL;

		// Special check for hierarchy only
		if (rest_dir && !*(rest_dir + 1))
		{
			// Delete this item and get out
			delete *iter;
			iter = erase(iter) -1;
			last_dir = dir;
			return 0;
		}

		// Special check for duplicates
		if (!rest_dir && (::strcmp(rest, &previous[stack_len]) == 0))
		{
			// Delete this item and get out
			delete *iter;
			iter = erase(iter) -1;
			last_dir = dir;
			return 0;
		}

		while (rest_dir)
		{
			// Insert hierarchies
			const char* p = rest;
			size_t len = 0;
			while(*p != dir)
			{
				p++;
				len++;
			}
			stack += cdstring(rest, p - rest);

			// Push current onto stack if not already
			if (stack != previous)
			{
				// Tweak for memory consumption peformance
				if (capacity() == size())
				{
					// Must keep iterator in sync with changes
					difference_type old_pos = iter - begin();
					reserve(size() + 1024);
					iter = begin() + old_pos;
				}

				iter = insert(iter, MakeHierarchy(stack, *iter));
				num_insert++;
				(*iter)->SetWDLevel(level);
				if (!mDoDescovery)
					MakeHierarchic(*iter, true);
				iter++;
			}

			stack += dir;
			stack_len = stack.length();
			level++;

			// Adjust for next bit
			rest = ++p;

			rest_dir = ::strchr(rest, dir);
		}

		(*iter)->SetWDLevel(level);
		if (!mDoDescovery)
			MakeHierarchic(*iter, true);
	}
	else
	{
		bool done = false;
		while(!done)
		{
			// Roll back hierarchy stack
			((char*) stack.c_str())[stack_len - 1] = 0;
			char* p = ::strrchr(stack.c_str_mod(), last_dir);
			const char* r = ::strchr(stack.c_str(), cMailAccountSeparator);
			if (p && (!account_prefix || (p > r)))
			{
				*++p = 0;
				stack_len = stack.length();
				if (level)		// Don't allow level to go negative
					level--;
				done = (::strncmp(stack.c_str(), node_name.c_str(), stack_len) == 0);
			}
			else
			{
				stack = cdstring::null_str;
				stack_len = 0;
				if (level)		// Don't allow level to go negative
					level--;
				done = true;
			}
		}

		// See if additional 'fake hierarchy' nodes required
		const char* rest = &node_name.c_str()[stack_len];
		if (!account_prefix || (::strchr(rest, dir) > ::strchr(node_name, cMailAccountSeparator)))
		{
			while (::strchr(rest, dir))
			{
				// Insert hierarchies
				const char* p = rest;

				// Punt over account name at start
				if (account_prefix && !stack_len)
				{
					while(*p && (*p != cMailAccountSeparator))
						p++;
					if (*p == cMailAccountSeparator)
						p++;
				}

				// Find start of next hierachy level
				while(*p != dir)
					p++;
				stack += cdstring(rest, p - rest);

				// Push current onto stack if not already
				if (stack != previous)
				{
					// Tweak for memory consumption peformance
					if (capacity() == size())
					{
						// Must keep iterator in sync with changes
						difference_type old_pos = iter - begin();
						reserve(size() + 1024);
						iter = begin() + old_pos;
					}

					iter = insert(iter, MakeHierarchy(stack, *iter));
					num_insert++;
					(*iter)->SetWDLevel(level);
					if (!mDoDescovery)
						MakeHierarchic(*iter, true);
					iter++;
				}
				stack += dir;
				stack_len = stack.length();
				level++;

				// Adjust for next bit
				rest = ++p;
			}
		}
		
		// Set details on actual node
		(*iter)->SetWDLevel(level);
		if (!mDoDescovery)
			MakeHierarchic(*iter, true);
	}

	last_dir = dir;
	return num_insert;
}
