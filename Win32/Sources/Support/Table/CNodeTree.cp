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


// Source for CNodeTree class

#include "CNodeTree.h"

// __________________________________________________________________________________________________
// C L A S S __ CNodeTree
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CNodeTree::CNodeTree()
{
}

// Default destructor
CNodeTree::~CNodeTree()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Find index of this sibling in parent
UInt32 CNodeTree::SiblingPosition(UInt32 inWideOpenIndex)
{
	UInt32 index = 0;

	// Get parent node
	SHierarchyNode *child;
	child = mHierarchyArray.at(inWideOpenIndex - 1);
	
	if (child->parent)
	{
		SHierarchyNode *parent = child->parent;
		SHierarchyNode *next_child = parent->firstChild;
		index++;
		while(next_child && (next_child != child))
		{
			next_child = next_child->sibling;
			index++;
		}
	}
	
	return index;
}

// Find wide-open index of nth sibling in parent
UInt32 CNodeTree::SiblingIndex(UInt32 parentIndex, UInt32 position)
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
