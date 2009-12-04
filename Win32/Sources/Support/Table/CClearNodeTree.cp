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


// Source for CNodeArrayTree class

#include "CClearNodeTree.h"

// __________________________________________________________________________________________________
// C L A S S __ CClearNodeTree
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CClearNodeTree::CClearNodeTree()
{
}

// Default destructor
CClearNodeTree::~CClearNodeTree()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

void CClearNodeTree::Clear(void)
{
	// Delete nodes in array
	for(CNodeVector::iterator iter = mHierarchyArray.begin(); iter != mHierarchyArray.end(); iter++)
		delete *iter;

	// Remove all rows
	mHierarchyArray.clear();

	mExposedNodes = 0;
}
