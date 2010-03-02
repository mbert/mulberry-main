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


//	A CollapsableTree that uses a DynamicArray to store pointers to
//	Node objects

#ifndef __CNODEVECTORTREE__MULBERRY__
#define __CNODEVECTORTREE__MULBERRY__

#include <WIN_LCollapsableTree.h>

struct SHierarchyNode {
	SHierarchyNode	*parent;
	SHierarchyNode	*firstChild;
	SHierarchyNode	*sibling;
	UInt32			wideOpenIndex;
	UInt32			exposedIndex;
	UInt32			nestingLevel;
	Boolean			collapsable;			
	Boolean			expanded;
};

typedef std::vector<SHierarchyNode*> CNodeVector;

// ---------------------------------------------------------------------------

class	CNodeVectorTree : public LCollapsableTree {
public:
						CNodeVectorTree();
	virtual				~CNodeVectorTree();
	
	virtual UInt32		CountNodes() const;
	virtual UInt32		CountExposedNodes() const;
	
	virtual UInt32		GetWideOpenIndex(
								UInt32			inExposedIndex) const;
	virtual UInt32		GetExposedIndex(
								UInt32			inWideOpenIndex) const;
	
	virtual UInt32		GetParentIndex(
								UInt32			inWideOpenIndex) const;
	virtual UInt32		GetNestingLevel(
								UInt32			inWideOpenIndex) const;
	virtual Boolean		IsCollapsable(
								UInt32			inWideOpenIndex) const;
	virtual void		MakeCollapsable(
								UInt32			inWideOpenIndex,
								Boolean			inCollapsable);
	virtual Boolean		IsExpanded(
								UInt32			inWideOpenIndex) const;
								
	virtual UInt32		CountAllDescendents(
								UInt32			inWideOpenIndex) const;
	virtual UInt32		CountExposedDescendents(
								UInt32			inWideOpenIndex) const;
	
	virtual UInt32		InsertSiblingNodes(
								UInt32			inHowMany,
								UInt32			inAfterIndex,
								Boolean			inCollapsable);
	virtual void		InsertChildNodes(
								UInt32			inHowMany,
								UInt32			inParentIndex,
								Boolean			inCollapsable);
	virtual UInt32		AddLastChildNode(
								UInt32			inParentIndex,
								Boolean			inCollapsable);
	virtual void		RemoveNode(
								UInt32			inWideOpenIndex,
								UInt32			&outTotalRemoved,
								UInt32			&outExposedRemoved);
	virtual void		RemoveChildren(
								UInt32			inWideOpenIndex,
								UInt32			&outTotalRemoved,
								UInt32			&outExposedRemoved);

	virtual UInt32		CollapseNode(
								UInt32			inWideOpenIndex);
	virtual UInt32		DeepCollapseNode(
								UInt32			inWideOpenIndex);
	
	virtual UInt32		ExpandNode(
								UInt32			inWideOpenIndex);
	virtual UInt32		DeepExpandNode(
								UInt32			inWideOpenIndex);
	
	virtual void	Clear(void);

	virtual Boolean		HasSibling(
								UInt32			inWideOpenIndex);
	virtual Boolean		HasParent(
								UInt32			inWideOpenIndex);
	virtual Boolean		HasChildren(
								UInt32			inWideOpenIndex);
	
	virtual UInt32	GetSiblingIndex(UInt32 inWideOpenIndex);
	virtual UInt32	SiblingPosition(UInt32 inWideOpenIndex);
	virtual UInt32	SiblingIndex(UInt32 parentIndex, UInt32 position);

protected:
	CNodeVector		mHierarchyArray;
	UInt32			mExposedNodes;

	void				AdjustNestedExposedIndexes(
								SHierarchyNode	*inNode,
								UInt32			&ioExposedIndex);
	
	bool				ValidRow(UInt32			inWideOpenIndex) const;
};

#endif
