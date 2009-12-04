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


// Header for CAddressBookToolbar class

#ifndef __CADDRESSBOOKTOOLBAR__MULBERRY__
#define __CADDRESSBOOKTOOLBAR__MULBERRY__

#include "CToolbar.h"

// Classes
class LBevelButton;

class CAddressBookToolbar : public CToolbar
{
	friend class CAddressBookView;

public:
					CAddressBookToolbar();
	virtual 		~CAddressBookToolbar();

protected:
	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

#endif