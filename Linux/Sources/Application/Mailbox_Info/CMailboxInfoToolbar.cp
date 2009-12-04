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

#include "CCommands.h"
#include "CIconLoader.h"
#include "CMailboxToolbarPopup.h"

#include <JXImageWidget.h>

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxInfoToolbar::CMailboxInfoToolbar(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h)
	: CToolbar(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Default destructor
CMailboxInfoToolbar::~CMailboxInfoToolbar()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void CMailboxInfoToolbar::OnCreate()
{
	// Do inherited
	CToolbar::OnCreate();

	// Set 3pane state
	mIs3Pane = false;

	SetType(CToolbarManager::e1PaneMailbox);
	BuildToolbar();
}

void CMailboxInfoToolbar::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case eBroadcast_IconState:
		{
			JXImageWidget* icon = static_cast<JXImageWidget*>(GetItem(CToolbarManager::eToolbar_MailboxType));
			if (icon != NULL)
				icon->SetImage(CIconLoader::GetIcon(*reinterpret_cast<ResIDT*>(param), icon, mSmallIcons ? 16 : 32, 0x00CCCCCC), kFalse);
		}
		break;

	default:
		CToolbar::ListenTo_Message(msg, param);
		break;
	}
}		

CMailboxToolbarPopup* CMailboxInfoToolbar::GetCopyBtn() const
{
	return static_cast<CMailboxToolbarPopup*>(GetButton(CToolbarManager::eToolbar_CopyTo));
}
