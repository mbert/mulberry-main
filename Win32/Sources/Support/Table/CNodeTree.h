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


// Header for CNodeTree class

#ifndef __CNODETREE__MULBERRY__
#define __CNODETREE__MULBERRY__

#include "CNodeVectorTree.h"

// Classes

class CNodeTree : public CNodeVectorTree {

public:
					CNodeTree();
	virtual 		~CNodeTree();
	
	virtual UInt32	SiblingPosition(UInt32 inWideOpenIndex);
	virtual UInt32	SiblingIndex(UInt32 parentIndex, UInt32 position);

};

#endif
