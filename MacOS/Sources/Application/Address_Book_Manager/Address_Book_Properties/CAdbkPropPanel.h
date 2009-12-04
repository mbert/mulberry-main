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


// Header for CAdbkPropPanel class

#ifndef __CADBKPROPPANEL__MULBERRY__
#define __CADBKPROPPANEL__MULBERRY__

#include "CAddressBook.h"


// Constants

// Classes

class CAdbkPropPanel : public LView
{
public:
					CAdbkPropPanel() {}
					CAdbkPropPanel(LStream *inStream): LView(inStream) {}
	virtual 		~CAdbkPropPanel() {}

	virtual void	SetAdbkList(CAddressBookList* adbk_list) = 0;	// Set adbk list - pure virtual
	virtual void	ApplyChanges(void) = 0;					// Force update of values

protected:
	CAddressBookList*	mAdbkList;

};

#endif
