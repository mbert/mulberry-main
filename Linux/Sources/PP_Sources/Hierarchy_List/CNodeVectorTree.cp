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


// Modified by CD to improve performance and stability

// Methods changed
//	CNodeVectorTree::AddLastChildNode
//	CNodeVectorTree::InsertSiblingNodes/InsertSiblingNodes	changed to create list collpased always

#include <cstddef>
#include "CNodeVectorTree.h"

#include <stddef.h>

// ---------------------------------------------------------------------------
//		 CNodeVectorTree
// ---------------------------------------------------------------------------
//	Default Constructor

CNodeVectorTree::CNodeVectorTree()
{
	mExposedNodes = 0;
}


// ---------------------------------------------------------------------------
//		 ~CNodeVectorTree
// ---------------------------------------------------------------------------
//	Destructor

CNodeVectorTree::~CNodeVectorTree()
{
	for(CNodeVector::iterator iter = mHierarchyArray.begin(); iter != mHierarchyArray.end(); iter++)
		delete *iter;
}

#pragma mark --- Accessors ---

// ---------------------------------------------------------------------------
//		 CountNodes
// ---------------------------------------------------------------------------
//	Return the number of nodes in a Tree

UInt32
CNodeVectorTree::CountNodes() const
{
	return mHierarchyArray.size();
}


// ---------------------------------------------------------------------------
//		 CountExposedNodes
// ---------------------------------------------------------------------------
//	Return the number of exposed nodes in a Tree

UInt32
CNodeVectorTree::CountExposedNodes() const
{
	return mExposedNodes;
}


// ---------------------------------------------------------------------------
//		 GetWideOpenIndex
// ---------------------------------------------------------------------------
//	Return the wide-open index of a node specified by its exposed index

UInt32
CNodeVectorTree::GetWideOpenIndex(
	UInt32	inExposedIndex) const
{
	UInt32		wideOpenIndex = 0;
	
	if ((inExposedIndex > 0) && (inExposedIndex <= mExposedNodes)) {
			// +++ Replace with binary search +++
	
#if 0
		SHierarchyNode	*theRow;
		while (mHierarchyArray.FetchItemAt(++wideOpenIndex, &theRow)) {
			if (theRow->exposedIndex == inExposedIndex) {
				break;
			}
		}
#else
		SHierarchyNode	*theRow = mHierarchyArray.at(++wideOpenIndex - 1);
		while(true)
		{
			// If current match then done!
			if (theRow->exposedIndex == inExposedIndex)
				break;
			
			// Look at sibling (which must also be visible)
			if (theRow->sibling && (theRow->sibling->exposedIndex <= inExposedIndex))
				// Bump to sibling
				theRow = theRow->sibling;
			
			// A child must be the target
			else
				theRow = theRow->firstChild;
		}
	
		return theRow->wideOpenIndex;
#endif
	}
	
	return wideOpenIndex;
}


// ---------------------------------------------------------------------------
//		 GetExposedIndex
// ---------------------------------------------------------------------------
//	Return the exposed index of a node specified by its wide-open index

UInt32
CNodeVectorTree::GetExposedIndex(
	UInt32	inWideOpenIndex) const
{
	UInt32		exposedIndex = 0;
	
	if (ValidRow(inWideOpenIndex))
	{
		SHierarchyNode* theRow = mHierarchyArray.at(inWideOpenIndex - 1);
		exposedIndex = theRow->exposedIndex;
	}
	
	return exposedIndex;
}


// ---------------------------------------------------------------------------
//		 GetParentIndex
// ---------------------------------------------------------------------------
//	Return the wide-open index of the parent of a Node

UInt32
CNodeVectorTree::GetParentIndex(
	UInt32	inWideOpenIndex) const
{
	UInt32	parentIndex = 0;
	
	if (ValidRow(inWideOpenIndex))
	{
		SHierarchyNode* theRow = mHierarchyArray.at(inWideOpenIndex -1);
		if (theRow->parent != nil)
			parentIndex = theRow->parent->wideOpenIndex;
	}

	return parentIndex;
}


// ---------------------------------------------------------------------------
//		 GetNestingLevel
// ---------------------------------------------------------------------------
//	Return the nesting level of a Node
//
//	Top level nodes are at nesting level 0

UInt32
CNodeVectorTree::GetNestingLevel(
	UInt32	inWideOpenIndex) const
{
	UInt32	nestingLevel = 0;

	if (ValidRow(inWideOpenIndex))
	{
		SHierarchyNode* theRow = mHierarchyArray.at(inWideOpenIndex -1);
		nestingLevel = theRow->nestingLevel;
	}

	return nestingLevel;
}


// ---------------------------------------------------------------------------
//		 IsCollapsable
// ---------------------------------------------------------------------------
//	Return whether a Node is collapsable (can have children)

bool
CNodeVectorTree::IsCollapsable(
	UInt32	inWideOpenIndex) const
{
	bool	collapsable = false;

	if (ValidRow(inWideOpenIndex))
	{
		SHierarchyNode* theRow = mHierarchyArray.at(inWideOpenIndex -1);
		collapsable = theRow->collapsable;
	}

	return collapsable;
}


// ---------------------------------------------------------------------------
//		 MakeCollapsable
// ---------------------------------------------------------------------------
//	Set the collapsable flag for a Node
//
//	Setting a Node that has children to be not collapsable is not allowed.

void
CNodeVectorTree::MakeCollapsable(
	UInt32	inWideOpenIndex,
	bool	inCollapsable)
{
	if (ValidRow(inWideOpenIndex))
	{
		SHierarchyNode* theRow = mHierarchyArray.at(inWideOpenIndex -1);
	
		if (!inCollapsable && (theRow->firstChild != nil)) {
			//SignalPStr_("\pCan't make Node with children not collapsable");
			
		} else {
			theRow->collapsable = inCollapsable;
		}
	}
}


// ---------------------------------------------------------------------------
//		 IsExpanded
// ---------------------------------------------------------------------------
//	Return whether a Node is expanded (rather than collapsed)

bool
CNodeVectorTree::IsExpanded(
	UInt32	inWideOpenIndex) const
{
	bool	expanded = true;

	if (ValidRow(inWideOpenIndex))
	{
		SHierarchyNode* theRow = mHierarchyArray.at(inWideOpenIndex -1);
		expanded = theRow->expanded;
	}

	return expanded;
}


// ---------------------------------------------------------------------------
//		 CountAllDescendents
// ---------------------------------------------------------------------------
//	Return the total number of descendents of a Node

UInt32
CNodeVectorTree::CountAllDescendents(
	UInt32	inWideOpenIndex) const
{
	UInt32	descendents = 0;
		
	if (ValidRow(inWideOpenIndex))
	{
		SHierarchyNode* theNode = mHierarchyArray.at(inWideOpenIndex -1);
	
		if (theNode->firstChild != nil) {	// Node has some descendents
		
				// In our flattened array of Nodes, the descendents of a
				// Node are all the nodes between that Node and the Node's
				// sibling (or the sibling of its closest ancestor that
				// has a sibling).
		
			SHierarchyNode	*prevSibling = theNode;
			while ((prevSibling != nil) && (prevSibling->sibling == nil)) {
				prevSibling = prevSibling->parent;
			}
			
			if ((prevSibling != nil) && (prevSibling->sibling != nil)) {
				descendents = prevSibling->sibling->wideOpenIndex -
								inWideOpenIndex - 1;
			
			} else {
				descendents = CountNodes() - inWideOpenIndex;
			}
		}
	}

	return descendents;
}


// ---------------------------------------------------------------------------
//		 CountExposedDescendents
// ---------------------------------------------------------------------------
//	Return the number of exposed descendents of a Node

UInt32
CNodeVectorTree::CountExposedDescendents(
	UInt32	inWideOpenIndex) const
{
	UInt32	exposedDescendents = 0;
		
	if (ValidRow(inWideOpenIndex))
	{
		SHierarchyNode* theNode = mHierarchyArray.at(inWideOpenIndex -1);
	
		if ( (theNode->exposedIndex > 0) &&
			 theNode->expanded &&
			 (theNode->firstChild != nil) ) {
		
				// Node is exposed, expanded, and has descendents.
				// In our flattened array of Nodes, the descendents of a
				// Node are all the nodes between that Node and the Node's
				// sibling (or the sibling of its closest ancestor that
				// has a sibling).
		
			SHierarchyNode	*prevSibling = theNode;
			while ((prevSibling != nil) && (prevSibling->sibling == nil)) {
				prevSibling = prevSibling->parent;
			}
			
			if ((prevSibling != nil) && (prevSibling->sibling != nil)) {
				exposedDescendents = prevSibling->sibling->exposedIndex -
										theNode->exposedIndex - 1;
			
			} else {
				exposedDescendents = mExposedNodes - theNode->exposedIndex;
			}
		}
	}

	return exposedDescendents;
}

#pragma mark --- Insert/Delete Nodes ---

// ---------------------------------------------------------------------------
//		 InsertSiblingNodes
// ---------------------------------------------------------------------------
//	Add nodes to a Tree as siblings of the specified node
//
//	If inAfterIndex is 0, Nodes are inserted at the beginning of the Tree.
//	If inAfterIndex too big, Nodes are inserted at the end of the Tree,
//		but at the top level
//
//	Returns the wide open index of the first inserted node

UInt32
CNodeVectorTree::InsertSiblingNodes(
	UInt32	inHowMany,
	UInt32	inAfterIndex,				// WideOpen index
	bool	inCollapsable)
{
		// Determine where to insert the new Nodes

	SHierarchyNode	*parent = nil;
	SHierarchyNode	*afterSibling = nil;
	SHierarchyNode	*beforeSibling = nil;
	UInt32			wideOpenIndex;
	UInt32			exposedIndex;
	UInt32			nestingLevel;
	UInt32			lastIndex = CountNodes();
	UInt32			afterIndex;
		
	if ((inAfterIndex == 0) || (lastIndex == 0)) {
										// Insert at the beginning
		beforeSibling = (lastIndex ? mHierarchyArray.at(0) : nil);
		wideOpenIndex = 1;
		exposedIndex = 1;
		nestingLevel = 0;
		afterIndex = 0;
	
	} else if (inAfterIndex > 0) {
		if (inAfterIndex <= mHierarchyArray.size())
			afterSibling = mHierarchyArray.at(inAfterIndex - 1);
		
		if (afterSibling != nil) {		// Insert after this sibling
			parent = afterSibling->parent;
			beforeSibling = afterSibling->sibling;
			wideOpenIndex = afterSibling->wideOpenIndex +
							CountAllDescendents(afterSibling->wideOpenIndex) + 1;
			exposedIndex = afterSibling->exposedIndex;
			if (exposedIndex > 0) {
				exposedIndex += CountExposedDescendents(afterSibling->wideOpenIndex) + 1;
			}
			nestingLevel = afterSibling->nestingLevel;
			afterIndex = wideOpenIndex - 1;
		
		} else {						// Insert at the end at the top level
			wideOpenIndex = lastIndex + 1;
			exposedIndex = mExposedNodes + 1;
			nestingLevel = 0;
			afterIndex = lastIndex;
			
										// Sibling of new rows will be the
										//   last Node at the top level.
										//   We find this Node by starting
										//   with the last Node and following
										//   the parent links.
			afterSibling = mHierarchyArray.at(lastIndex - 1);
			while (afterSibling->parent != nil) {
				afterSibling = afterSibling->parent;
			}
		}
	}

										// Make space for Nodes in Array
	mHierarchyArray.insert(mHierarchyArray.begin() + afterIndex, inHowMany, nil);
	
		// Create Node objects for new items and store pointers to them in
		// the Hierarchy Array. Do this in reverse order so that the
		// sibling field can be set properly.
		
	SHierarchyNode	*sibling = beforeSibling;
	SHierarchyNode	*theNode;
	
	for (UInt32 i = inHowMany; i >= 1; i--) {
		theNode = new SHierarchyNode;
		theNode->parent = parent;
		theNode->firstChild = nil;
		theNode->sibling = sibling;
		theNode->wideOpenIndex = wideOpenIndex + i - 1;
		theNode->exposedIndex = 0;
		if (exposedIndex > 0) {
			theNode->exposedIndex = exposedIndex + i - 1;
		}
		theNode->nestingLevel = nestingLevel;
		theNode->collapsable = inCollapsable;
		theNode->expanded = false;				// Changed to keep collapsed on insert
		
		mHierarchyArray.at(afterIndex + i - 1) = theNode;
		
		sibling = theNode;
	}
	
	if (afterSibling != nil) {
		afterSibling->sibling = theNode;
	}
		
										// Adjust indexes for all Nodes
										//   after the inserted ones
	lastIndex = CountNodes();
	for (UInt32 j = afterIndex + inHowMany + 1; j <= lastIndex; j++) {
		theNode = mHierarchyArray.at(j - 1);
		
		theNode->wideOpenIndex = j;
		
		if ((exposedIndex > 0) && (theNode->exposedIndex > 0)) {
			theNode->exposedIndex += inHowMany;
		}
	}
	
	if (exposedIndex > 0) {
		mExposedNodes += inHowMany;
	}
	
	return afterIndex + 1;
}


// ---------------------------------------------------------------------------
//		 InsertChildNodes
// ---------------------------------------------------------------------------
//	Add nodes to a Tree as the first children of the specified node

void
CNodeVectorTree::InsertChildNodes(
	UInt32	inHowMany,
	UInt32	inParentIndex,				// WideOpen index
	bool	inCollapsable)
{
	if (!ValidRow(inParentIndex))
	{
		//SignalPStr_("\pInvalid Parent Index");
		return;
	}

	SHierarchyNode	*parent = mHierarchyArray.at(inParentIndex - 1);
										// Make space for Nodes in Array
	mHierarchyArray.insert(mHierarchyArray.begin() + inParentIndex, inHowMany, nil);
	
		// Create Node objects for new items and store pointers to them in
		// the Hierarchy Array. Do this in reverse order so that the
		// sibling field can be set properly.
		
	SHierarchyNode	*theNode;
	SHierarchyNode	*sibling = parent->firstChild;
	UInt32			exposedIndex = 0;
	if ((parent->exposedIndex > 0) && parent->expanded) {
		exposedIndex = parent->exposedIndex;
	}
	UInt32			nestingLevel = parent->nestingLevel + 1;
	
	for (UInt32 i = inHowMany; i >= 1; i--) {
		theNode = new SHierarchyNode;
		theNode->parent = parent;
		theNode->firstChild = nil;
		theNode->sibling = sibling;
		theNode->wideOpenIndex = parent->wideOpenIndex + i;
		theNode->exposedIndex = 0;
		if (exposedIndex > 0) {
			theNode->exposedIndex = exposedIndex + i;
		}
		theNode->nestingLevel = nestingLevel;
		theNode->collapsable = inCollapsable;
		theNode->expanded = false;				// Changed to keep collapsed on insert
		
		mHierarchyArray.at(inParentIndex + i - 1)  = theNode;
		
		sibling = theNode;
	}
	
	parent->firstChild = theNode;
	parent->collapsable = true;
	
										// Adjust indexes for all Nodes
										//   after the inserted ones
	UInt32	lastIndex = CountNodes();
	for (UInt32 j = inParentIndex + inHowMany + 1; j <= lastIndex; j++) {
		theNode = mHierarchyArray.at(j - 1);
		
		theNode->wideOpenIndex = j;
		
		if ((exposedIndex > 0) && (theNode->exposedIndex > 0)) {
			theNode->exposedIndex += inHowMany;
		}
	}
	
	if (exposedIndex > 0) {
		mExposedNodes += inHowMany;
	}
}
	

// ---------------------------------------------------------------------------
//		 AddLastChildNode
// ---------------------------------------------------------------------------
//	Add a single Node as the last child of the specified node

UInt32
CNodeVectorTree::AddLastChildNode(
	UInt32	inParentIndex,				// WideOpen index
	bool	inCollapsable)
{
	if (inParentIndex == 0) {			// Put at end at the top level
		return InsertSiblingNodes(1, -1L, inCollapsable);
	}
	
	if (!ValidRow(inParentIndex))
	{
		//SignalPStr_("\pInvalid Parent Index");
		return 0;
	}

	SHierarchyNode	*parent = mHierarchyArray.at(inParentIndex - 1);
	
	if (parent->firstChild == nil) {	// Parent has no children, so new
										//   node will be its first child
		InsertChildNodes(1, inParentIndex, inCollapsable);
		return inParentIndex + 1;
		
	} else {							// Find last child by following
										//   children's sibling chain
		SHierarchyNode	*child = parent->firstChild;
		while (child->sibling != nil) {
			child = child->sibling;
		}
		
		return InsertSiblingNodes(1, child->wideOpenIndex, inCollapsable);
	}
}
	

// ---------------------------------------------------------------------------
//		 RemoveNode
// ---------------------------------------------------------------------------
//	Remove a single Node (and all its descendents) from the Tree
//
//	Return the number of exposed nodes deleted by the removal

void
CNodeVectorTree::RemoveNode(
	UInt32	inWideOpenIndex,
	UInt32	&outTotalRemoved,
	UInt32	&outExposedRemoved)
{
	if (!ValidRow(inWideOpenIndex))
	{
		//SignalPStr_("\pInvalid index for Node to Remove");
		return;
	}

	SHierarchyNode	*removedNode = mHierarchyArray.at(inWideOpenIndex - 1);

		// Deleting this node deletes all its descendent nodes
	
	outTotalRemoved = CountAllDescendents(inWideOpenIndex) + 1;
	outExposedRemoved = 0;
	if (removedNode->exposedIndex > 0) {
		outExposedRemoved = CountExposedDescendents(inWideOpenIndex) + 1;
	}
		
		// Some other Node points to the Removed Node (unless we are
		// removing the first Node). That Node is either its parent
		// (if it is the first child) or its previous sibling. We must
		// change that Node to point to the Removed Node's next sibling.

	SHierarchyNode	*previousSibling = nil;
	if (ValidRow(inWideOpenIndex - 1))
		previousSibling = mHierarchyArray.at(inWideOpenIndex - 2);
										// Node before this one could be an
										//   ancestor of the previous sibling.
										//   Move up the parent chain find
										//   the previous sibling.
	while ((previousSibling != nil) && (previousSibling->sibling != removedNode)) {
		previousSibling = previousSibling->parent;
	}
	
	if (previousSibling != nil) {		// Removed Node has a previous sibling
		previousSibling->sibling = removedNode->sibling;
	} else if (removedNode->parent != nil) {
										// Remove Node is a first child
		removedNode->parent->firstChild = removedNode->sibling;
	}
	
		// Delete Node objects for Removed Node and all its descendents.
	
	UInt32	lastToDelete = inWideOpenIndex + outTotalRemoved - 1;
	for (UInt32 i = inWideOpenIndex; i <= lastToDelete; i++) {
		SHierarchyNode	*nodeToDelete ;
		nodeToDelete = mHierarchyArray.at(i - 1);
		delete nodeToDelete;
	}
	
		// Remove pointers to deleted Nodes from the Hierarchy array
		
	mHierarchyArray.erase(mHierarchyArray.begin() + (inWideOpenIndex - 1), mHierarchyArray.begin() + (inWideOpenIndex - 1 + outTotalRemoved));
	
		// Adjust the indexes of the Nodes after the deleted one
	
	UInt32	lastNode = CountNodes();
	for (UInt32 j = inWideOpenIndex; j <= lastNode; j++) {
		SHierarchyNode	*theNode;
		theNode = mHierarchyArray.at(j - 1);
		theNode->wideOpenIndex = j;
		
		if (theNode->exposedIndex != 0) {
			theNode->exposedIndex -= outExposedRemoved;
		}
	}
	
	mExposedNodes -= outExposedRemoved;
}

// ---------------------------------------------------------------------------
//		 RemoveNode
// ---------------------------------------------------------------------------
//	Remove a single Node's descendents from the Tree
//
//	Return the number of exposed nodes deleted by the removal

void
CNodeVectorTree::RemoveChildren(
	UInt32	inWideOpenIndex,
	UInt32	&outTotalRemoved,
	UInt32	&outExposedRemoved)
{
	if (!ValidRow(inWideOpenIndex))
	{
		//SignalPStr_("\pInvalid index for Node to Remove");
		return;
	}

	SHierarchyNode	*removedNode = mHierarchyArray.at(inWideOpenIndex - 1);

		// Deleting this node deletes all its descendent nodes
	
	outTotalRemoved = CountAllDescendents(inWideOpenIndex);
	outExposedRemoved = 0;
	if (removedNode->exposedIndex > 0) {
		outExposedRemoved = CountExposedDescendents(inWideOpenIndex);
	}
		
		// Adjust removed nodes indices
	removedNode->firstChild = nil;

		// Delete Node objects for Removed Node and all its descendents.
	if (outTotalRemoved)
	{
		UInt32	lastToDelete = inWideOpenIndex + outTotalRemoved;
		for (UInt32 i = inWideOpenIndex + 1; i <= lastToDelete; i++) {
			SHierarchyNode	*nodeToDelete ;
			nodeToDelete = mHierarchyArray.at(i - 1);
			delete nodeToDelete;
		}
		
			// Remove pointers to deleted Nodes from the Hierarchy array
		mHierarchyArray.erase(mHierarchyArray.begin() + inWideOpenIndex, mHierarchyArray.begin() + (inWideOpenIndex + outTotalRemoved));
	}

		// Adjust the indexes of the Nodes after the deleted one
	
	UInt32	lastNode = CountNodes();
	for (UInt32 j = inWideOpenIndex + 1; j <= lastNode; j++) {
		SHierarchyNode	*theNode;
		theNode = mHierarchyArray.at(j - 1);
		theNode->wideOpenIndex = j;
		
		if (theNode->exposedIndex != 0) {
			theNode->exposedIndex -= outExposedRemoved;
		}
	}
	
	mExposedNodes -= outExposedRemoved;
}

#pragma mark --- Collapse/Expand Nodes ---

// ---------------------------------------------------------------------------
//		 CollapseNode
// ---------------------------------------------------------------------------
//	Collapse a Node in the Tree, concealing its child Nodes
//
//	Return the number of nodes concealed by the collapse

UInt32
CNodeVectorTree::CollapseNode(
	UInt32	inWideOpenIndex)
{
	if (!ValidRow(inWideOpenIndex))
	{
		//SignalPStr_("\pInvalid index for Node to Collapse");
		return 0;
	}
	
	UInt32		nodesConcealed = 0;
	
	SHierarchyNode	*collapsedNode = mHierarchyArray.at(inWideOpenIndex - 1);
	
	if (collapsedNode->expanded) {		// Node is currently expanded
		
		if (collapsedNode->exposedIndex > 0) {
										// Node is exposed
			nodesConcealed = CountExposedDescendents(inWideOpenIndex);
			
			if (nodesConcealed > 0) {
										// Conceal all descendent Nodes
				UInt32	lastToConceal = collapsedNode->wideOpenIndex +
									CountAllDescendents(inWideOpenIndex);						
				for (UInt32 i = collapsedNode->wideOpenIndex + 1;
					 i <= lastToConceal; i++) {
					SHierarchyNode	*descendent;
					descendent = mHierarchyArray.at(i - 1);
					descendent->exposedIndex = 0;
				}		
				
										// Adjust exposed indexes for all
										//   Nodes after the concealed ones
				UInt32	currentCount = CountNodes();
				for (UInt32 j = lastToConceal + 1; j <= currentCount; j++) {
					SHierarchyNode	*theNode;
					theNode = mHierarchyArray.at(j - 1);
					
					if (theNode->exposedIndex > 0) {
						theNode->exposedIndex -= nodesConcealed;
					}
				}
				
				mExposedNodes -= nodesConcealed;
			}
		}
		collapsedNode->expanded = false;
	}
	
	return nodesConcealed;
}


// ---------------------------------------------------------------------------
//		 DeepCollapseNode
// ---------------------------------------------------------------------------
//	Collapse a Node and all its descendent Nodes
//
//	Return the number of nodes concealed by the deep collapse

UInt32
CNodeVectorTree::DeepCollapseNode(
	UInt32	inWideOpenIndex)
{
	if (!ValidRow(inWideOpenIndex))
	{
		//SignalPStr_("\pInvalid index for Node to Collapse");
		return 0;
	}
										// Collapse this Node
	UInt32	nodesConcealed = CollapseNode(inWideOpenIndex);
	
	SHierarchyNode	*collapsedNode = mHierarchyArray.at(inWideOpenIndex - 1);
	
										// Collapse all descendents
	UInt32	lastDescendent = inWideOpenIndex +
								CountAllDescendents(inWideOpenIndex);
	for (UInt32 i = inWideOpenIndex + 1; i <= lastDescendent; i++) {
		CollapseNode(i);
	}
	
	return nodesConcealed;
}


// ---------------------------------------------------------------------------
//		 ExpandNode
// ---------------------------------------------------------------------------
//	Expand a Node in the Tree, revealing its child Nodes
//
//	Return the number of Nodes revealed by the expansion

UInt32
CNodeVectorTree::ExpandNode(
	UInt32	inWideOpenIndex)
{
	if (!ValidRow(inWideOpenIndex))
	{
		//SignalPStr_("\pInvalid index for Node to Expand");
		return 0;
	}

	UInt32			nodesRevealed = 0;
	
	SHierarchyNode	*expandedNode = mHierarchyArray.at(inWideOpenIndex - 1);
	
	if (!expandedNode->expanded) {		// Node is currently collapsed
		
		if (expandedNode->exposedIndex > 0) {
		
				// Node is exposed. Check each descendent. If expanding
				// this Node exposes the descendent, set its exposedIndex.
			
			UInt32	exposedIndex = expandedNode->exposedIndex + 1;
			AdjustNestedExposedIndexes(expandedNode->firstChild, exposedIndex);

			nodesRevealed = exposedIndex - expandedNode->exposedIndex - 1;
			
			if (nodesRevealed > 0) {
										// Adjust exposedIndex of all Nodes
										//   after the expanded ones										
				UInt32	nextIndex = inWideOpenIndex + 1 +
									CountAllDescendents(inWideOpenIndex);
				UInt32	lastIndex = CountNodes();
				for (UInt32 j = nextIndex; j <= lastIndex; j++) {
					SHierarchyNode	*theNode;
					theNode = mHierarchyArray.at(j - 1);
					
					if (theNode->exposedIndex > 0) {
						theNode->exposedIndex += nodesRevealed;
					}
				}
			
				mExposedNodes += nodesRevealed;
			}
		}
		expandedNode->expanded = true;
	}
	
	return nodesRevealed;
}


// ---------------------------------------------------------------------------
//		 DeepExpandNode
// ---------------------------------------------------------------------------
//	Expand a Node and all its descendent Nodes
//
//	Return the number of Nodes revealed by the expansion

UInt32
CNodeVectorTree::DeepExpandNode(
	UInt32	inWideOpenIndex)
{
	if (!ValidRow(inWideOpenIndex))
	{
		//SignalPStr_("\pInvalid index for Node to Expand");
		return 0;
	}
	
	SHierarchyNode	*expandedNode = mHierarchyArray.at(inWideOpenIndex - 1);
	
										// Expand all descendents
	UInt32	lastDescendent = inWideOpenIndex +
								CountAllDescendents(inWideOpenIndex);
	for (UInt32 i = inWideOpenIndex + 1; i <= lastDescendent; i++) {
		ExpandNode(i);
	}
	
	return ExpandNode(inWideOpenIndex);	// Expand this Node
}



// ---------------------------------------------------------------------------
//		 AdjustNestedExposedIndexes
// ---------------------------------------------------------------------------

void
CNodeVectorTree::AdjustNestedExposedIndexes(
	SHierarchyNode	*inNode,
	UInt32			&ioExposedIndex)
{
	while (inNode != nil) {
		inNode->exposedIndex = ioExposedIndex++;
		if (inNode->expanded) {
			AdjustNestedExposedIndexes(inNode->firstChild, ioExposedIndex);
		}
		inNode = inNode->sibling;
	}
}

bool CNodeVectorTree::ValidRow(UInt32	inWideOpenIndex) const
{
	return (inWideOpenIndex > 0) && (inWideOpenIndex <= mHierarchyArray.size());
}


void CNodeVectorTree::Clear(void)
{
	// Delete nodes in array
	for(CNodeVector::iterator iter = mHierarchyArray.begin(); iter != mHierarchyArray.end(); iter++)
		delete *iter;

	// Remove all rows
	mHierarchyArray.clear();

	mExposedNodes = 0;
}

bool CNodeVectorTree::HasSibling(UInt32 inWideOpenIndex)
{
	SHierarchyNode *node = mHierarchyArray.at(inWideOpenIndex - 1);
	
	return (node->sibling != nil);
}

bool CNodeVectorTree::HasParent(UInt32 inWideOpenIndex)
{
	SHierarchyNode *node = mHierarchyArray.at(inWideOpenIndex - 1);
	
	return (node->parent != nil);
}

bool CNodeVectorTree::HasChildren(UInt32 inWideOpenIndex)
{
	SHierarchyNode *node = mHierarchyArray.at(inWideOpenIndex - 1);
	
	return (node->firstChild != nil);
}

// Find index of sibling
UInt32 CNodeVectorTree::GetSiblingIndex(UInt32 inWideOpenIndex)
{
	// Get node
	SHierarchyNode* node = mHierarchyArray.at(inWideOpenIndex - 1);

	if (node->sibling)
		return node->sibling->wideOpenIndex;
	else
		return 0;
}

// Find index of this sibling in parent
UInt32 CNodeVectorTree::SiblingPosition(UInt32 inWideOpenIndex)
{
	UInt32 index = 0;

	// Get this node
	SHierarchyNode* child = mHierarchyArray.at(inWideOpenIndex - 1);

	// Get first sibling node
	SHierarchyNode* next_child = NULL;
	if (child->parent)
	{
		SHierarchyNode* parent = child->parent;
		next_child = parent->firstChild;
	}
	else
		next_child = mHierarchyArray.at(0);

	// Scan siblings for match
	index++;
	while(next_child && (next_child != child))
	{
		next_child = next_child->sibling;
		index++;
	}

	return index;
}

// Find wide-open index of nth sibling in parent
UInt32 CNodeVectorTree::SiblingIndex(UInt32 parentIndex, UInt32 position)
{
	// Get parent node
	SHierarchyNode *parent;
	parent = mHierarchyArray.at(parentIndex - 1);
	SHierarchyNode *next_child = parent->firstChild;
	UInt32 woRow = parent->wideOpenIndex;
	while(next_child && position)
	{
		woRow = next_child->wideOpenIndex;
		next_child = next_child->sibling;
		position--;
	}

	return woRow;
}
