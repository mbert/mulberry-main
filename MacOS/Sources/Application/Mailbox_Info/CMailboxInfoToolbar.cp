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


// Source for CMailboxInfoToolbar class

#include "CMailboxInfoToolbar.h"

#include "CMailboxToolbarPopup.h"

#include <LIconControl.h>

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxInfoToolbar::CMailboxInfoToolbar()
{
}

// Constructor from stream
CMailboxInfoToolbar::CMailboxInfoToolbar(LStream *inStream)
		: CToolbar(inStream)
{
}

// Default destructor
CMailboxInfoToolbar::~CMailboxInfoToolbar()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void CMailboxInfoToolbar::FinishCreateSelf()
{
	// Do inherited
	CToolbar::FinishCreateSelf();

	// Set 3pane state
	mIs3Pane = false;

	SetType(CToolbarManager::e1PaneMailbox);
	BuildToolbar();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, GetPaneID());
}

void CMailboxInfoToolbar::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case eBroadcast_IconState:
	{
		LIconControl* icon = static_cast<LIconControl*>(GetItem(CToolbarManager::eToolbar_MailboxType));
		if (icon != NULL)
			icon->SetResourceID(*reinterpret_cast<ResIDT*>(param));
		break;
	}

	default:
		CToolbar::ListenTo_Message(msg, param);
		break;
	}
}		

CMailboxToolbarPopup* CMailboxInfoToolbar::GetCopyBtn() const
{
	return static_cast<CMailboxToolbarPopup*>(GetButton(CToolbarManager::eToolbar_CopyTo));
}
