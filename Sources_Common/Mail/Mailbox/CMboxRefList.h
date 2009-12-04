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

#ifndef __CMBOXREFLIST__MULBERRY__
#define __CMBOXREFLIST__MULBERRY__

#include "CTreeNodeList.h"

#include "cdmutex.h"

class CMbox;
class CMboxRef;

class CMboxRefList : public CTreeNodeList
{
public:
	CMboxRefList() {}
	explicit CMboxRefList(const char* name);
	explicit CMboxRefList(bool owns_nodes)
		: CTreeNodeList(owns_nodes) {}
	virtual ~CMboxRefList() {}

	CMboxNode* AddMbox(const CMboxNode* node);					// Add copy of mailbox to list
	bool ContainsMbox(const CMboxNode* node) const				// Does it contain matching mbox
		{ return FindMbox(node) != nil; }
	CMboxRef* FindMbox(const CMboxNode* node) const;			// Return matching mbox
	bool FindPos(const CMboxNode* node, unsigned long& pos) const;				// Return matching mbox index
	void RenamedMbox(const cdstring& old_name, const cdstring& new_name);		// Hierarchical rename
	void RemoveMbox(CMboxNode* node,
						bool delete_it,
						bool resolve = false);					// Remove the mailbox from the list (with delete?)

	unsigned long ResolveIndexOf(const CMbox* mbox) const;		// Fetch matching index

	void SyncList(const cdstrpairvect& items);

protected:
	mutable cdmutex		mMutex;

	virtual CTreeNode*	MakeHierarchy(const cdstring& name, const CTreeNode* child);
	virtual void		MakeHierarchic(CTreeNode* node, bool hier);

	virtual CompareNodePP GetCompareFN() const
		{ return CompareMboxRef; }
	static bool	CompareMboxRef(const CTreeNode* node1, const CTreeNode* node2);	// Comparison function for mailbox references
};

#endif
