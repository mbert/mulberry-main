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


// Source for C3PaneMailboxToolbar class

#include "C3PaneMailboxToolbar.h"

#include "CMailboxToolbarPopup.h"

BEGIN_MESSAGE_MAP(C3PaneMailboxToolbar, CToolbar)
	ON_WM_CREATE()
END_MESSAGE_MAP()

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneMailboxToolbar::C3PaneMailboxToolbar()
{
}

// Default destructor
C3PaneMailboxToolbar::~C3PaneMailboxToolbar()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int C3PaneMailboxToolbar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CToolbar::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetType(CToolbarManager::e3PaneMailbox);
	BuildToolbar();

	return 0;
}

CMailboxToolbarPopup* C3PaneMailboxToolbar::GetCopyBtn() const
{
	return static_cast<CMailboxToolbarPopup*>(GetButton(CToolbarManager::eToolbar_CopyTo));
}
