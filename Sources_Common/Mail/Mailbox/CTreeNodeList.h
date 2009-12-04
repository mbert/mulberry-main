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


// Header for CTreeNodeList class

#ifndef __CTREENODELIST__MULBERRY__
#define __CTREENODELIST__MULBERRY__

#include <vector>
#include "CBroadcaster.h"

#include "cdstring.h"
#include "SBitFlags.h"

// Classes

class CTreeNode;
typedef CTreeNode CMboxNode;
typedef bool (*CompareNodePP)(const CTreeNode* node1, const CTreeNode* node2);

class CTreeNodeList : public std::vector<CTreeNode*>, public CBroadcaster
{
public:
	// Messages for broadcast
	enum
	{
		eBroadcast_ResetList = 'tnrl',
		eBroadcast_ShowList = 'tnsl',
		eBroadcast_HideList = 'tnhl',
		eBroadcast_AddNode = 'tran',
		eBroadcast_ChangeNode = 'trch',
		eBroadcast_DeleteNode = 'trdn',
		eBroadcast_MoveNode = 'trcn'
	};

	// Structs used to convey broadcast information
	struct SBroadcastAddNode
	{
		CTreeNodeList* mList;
		long mIndex;
		long mNumber;
	};
	
	struct SBroadcastChangeNode
	{
		CTreeNodeList* mList;
		long mIndex;
	};

	struct SBroadcastDeleteNode
	{
		CTreeNodeList* mList;
		long mIndex;
	};

	struct SBroadcastMoveNode
	{
		CTreeNodeList* mList;
		long mOldIndex;
		long mNewIndex;
	};

	// Bit flags
	enum ETreeNodeListFlags
	{
		eOwnsNodes =	1L << 0,
		eSorted =		1L << 1,
		eHierarchic =	1L << 2,
		eVisible =		1L << 3,
		eExpanded =		1L << 4,
		eLoaded =		1L << 5,
		eAdjustDelete =	1L << 6
	};
		
			CTreeNodeList();
	explicit CTreeNodeList(const char* name);
	explicit CTreeNodeList(bool owns_nodes);
	virtual ~CTreeNodeList();

	virtual const cdstring& GetName() const
		{ return mName; }
	virtual cdstring GetRootName() const;
	virtual void SetName(const char* name)
		{ mName = name; }

	virtual bool IsRootName(char dir) const
		{ return false; }
	virtual bool DoesDescovery() const
		{ return mDoDescovery; }

	virtual const cdstring& GetRoot() const
		{ return mRoot; }
	virtual void SetRoot(const char* root)
		{ mRoot = root; }

	virtual char GetDirDelim() const
		{ return mDirDelim; }
	virtual void SetDirDelim(char delim)
		{ mDirDelim = delim; }

	virtual void Refresh()													// Force reload
		{ mFlags.Set(eLoaded, false); }
	virtual bool IsLoaded() const
		{ return mFlags.IsSet(eLoaded); }

	virtual CTreeNode*	AddNode(CTreeNode* node);								// Add node to list
	virtual CTreeNode*	AddNode(CTreeNode* node, unsigned long index);			// Add node to list
	virtual CTreeNode*	AddNode(CTreeNode* node, iterator pos);					// Add node to list
	virtual void		RemoveNode(CTreeNode* node, bool delete_it);			// Remove the node from the list (with delete?)
	virtual void		ChangedNode(CTreeNode* node);							// Node changed
	virtual void		NodeRenamed(CTreeNode* node, const cdstring& new_name);	// Node renamed

	virtual CTreeNode*	GetParentNode(const CTreeNode* node);					// Get parent
	virtual CTreeNode*	GetFirstChild(const CTreeNode* node);					// Get the first child
	virtual CTreeNode*	GetNextSibling(const CTreeNode* node);					// Get next sibling

	virtual unsigned long FetchIndexOf(const CTreeNode* node) const;			// Get index (starts at 1)

	virtual void		DeleteAll();										// Delete all items

	virtual void		SetFlag(ETreeNodeListFlags flag, bool set);

	virtual bool		IsSorted() const									// Is kept sorted
						{ return mFlags.IsSet(eSorted); }
	virtual void		Sort();												// Sort mailbox

	virtual bool		IsHierarchic() const								// Is kept hierarchic
						{ return mFlags.IsSet(eHierarchic); }

	virtual bool		IsVisible() const									// Is visible
						{ return mFlags.IsSet(eVisible); }

	virtual bool		IsExpanded() const									// Is expanded
						{ return mFlags.IsSet(eExpanded); }

	virtual void		AdjustDirectories();								// Adjust directories for added/removed mailbox

protected:
	cdstring		mName;
	cdstring		mRoot;
	cdstring		mStack;
	bool			mDoDescovery;
	SBitFlags		mFlags;
	char			mDirDelim;

	virtual CTreeNodeList::iterator GetInsertPosition(const CTreeNode* node);		// Get insert position of node

	virtual CompareNodePP GetCompareFN() const
		{ return CompareNode; }
	static bool	CompareNode(const CTreeNode* node1, const CTreeNode* node2);		// Comparison function for mailboxes
	static bool	CompareNodeNames(const char* node1, const char* node2, char delim);	// Comparison function for hierarchy names

	virtual CTreeNode*	MakeHierarchy(const cdstring& name, const CTreeNode* child) = 0;
	virtual void		MakeHierarchic(CTreeNode* node, bool hier) = 0;

	virtual void		AdjustFlat();
	virtual void		AdjustHierarchy();
	virtual size_t		AdjustNodeStructure(iterator& iter,
									cdstring& stack,
									size_t& stack_len,
									unsigned long& level,
									const char* previous,
									char& last_dir,
									bool account_prefix);

private:
	void InitTreeNodeList();
	void CalculateStackStart(char dir);
	
};

#endif
