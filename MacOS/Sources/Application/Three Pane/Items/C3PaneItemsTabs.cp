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


// C3PaneItemsTabs

#include "C3PaneItemsTabs.h"

#include "CCommands.h"
#include "CContextMenu.h"
#include "CResources.h"
#include "C3PaneItems.h"

#include <LControlImp.h>

enum
{
	menuChoice_MoveLeft = 1,
	menuChoice_MoveRight,
	menuChoice_MoveStart,
	menuChoice_MoveEnd,
	menuChoice_Separator1,
	menuChoice_Close,
	menuChoice_CloseAll,
	menuChoice_CloseOthers,
	menuChoice_Separator2,
	menuChoice_Rename,
	menuChoice_Separator3,
	menuChoice_Lock,
	menuChoice_LockAll,
	menuChoice_Dynamic,
	menuChoice_Separator4,
	menuChoice_Save,
	menuChoice_Restore
};

C3PaneItemsTabs::C3PaneItemsTabs(LStream *inStream) : CTabsX(inStream)
{
	mOwner = NULL;
	mMenuChoice = ::GetMenu(MENU_3PaneTabbedItems);
	::InsertMenu(mMenuChoice, hierMenu);
}
 
Boolean
C3PaneItemsTabs::IsHitBy(
	SInt32	inHorizPort,
	SInt32	inVertPort)
{
	return PointIsInFrame(inHorizPort, inVertPort);
}

void C3PaneItemsTabs::Click(SMouseDownEvent &inMouseDown)
{
	// Look for context menu click
	if (CContextMenuProcessAttachment::IsCMMEvent(inMouseDown.macEvent))
	{
		bool in_button = false;
		long btn = 0;
		if (IsEnabled())
		{
			for(btn = 1; btn <= GetMaxValue(); btn++)
			{
				StRegion rgn;
				::GetControlRegion(GetControlImp()->GetMacControl(), btn, rgn);

				if (rgn.Contains(inMouseDown.whereLocal))
				{
					in_button = true;
					break;
				}
			}
		}

		// Do menu Select
		Point startPt = inMouseDown.wherePort;
		PortToGlobalPoint(startPt);
		if (in_button && (btn > 1))
		{
			::EnableMenuItem(mMenuChoice, menuChoice_MoveLeft);
			::EnableMenuItem(mMenuChoice, menuChoice_MoveStart);
		}
		else
		{
			::DisableMenuItem(mMenuChoice, menuChoice_MoveLeft);
			::DisableMenuItem(mMenuChoice, menuChoice_MoveStart);
		}
		if (in_button && (btn < GetMaxValue()))
		{
			::EnableMenuItem(mMenuChoice, menuChoice_MoveRight);
			::EnableMenuItem(mMenuChoice, menuChoice_MoveEnd);
		}
		else
		{
			::DisableMenuItem(mMenuChoice, menuChoice_MoveRight);
			::DisableMenuItem(mMenuChoice, menuChoice_MoveEnd);
		}

		if (in_button)
		{
			// Determine if locked
			bool is_locked = mOwner && mOwner->IsSubstituteLocked(btn - 1);
			if (is_locked)
			{
				::DisableMenuItem(mMenuChoice, menuChoice_Close);
				::DisableMenuItem(mMenuChoice, menuChoice_Dynamic);
				::CheckMenuItem(mMenuChoice, menuChoice_Lock, true);
			}
			else
			{
				::EnableMenuItem(mMenuChoice, menuChoice_Close);
				::EnableMenuItem(mMenuChoice, menuChoice_Dynamic);
				::CheckMenuItem(mMenuChoice, menuChoice_Lock, false);
			}

			// Determine if dynamic
			bool is_dynamic = mOwner && mOwner->IsSubstituteDynamic(btn - 1);
			::CheckMenuItem(mMenuChoice, menuChoice_Dynamic, is_dynamic);
			
			::EnableMenuItem(mMenuChoice, menuChoice_CloseAll);
			::EnableMenuItem(mMenuChoice, menuChoice_CloseOthers);
			::EnableMenuItem(mMenuChoice, menuChoice_Rename);
			::EnableMenuItem(mMenuChoice, menuChoice_Lock);
			::EnableMenuItem(mMenuChoice, menuChoice_LockAll);
		}
		else
		{
			::DisableMenuItem(mMenuChoice, menuChoice_Close);
			::DisableMenuItem(mMenuChoice, menuChoice_CloseAll);
			::DisableMenuItem(mMenuChoice, menuChoice_CloseOthers);
			::DisableMenuItem(mMenuChoice, menuChoice_Rename);
			::DisableMenuItem(mMenuChoice, menuChoice_Lock);
			::DisableMenuItem(mMenuChoice, menuChoice_LockAll);
			::DisableMenuItem(mMenuChoice, menuChoice_Dynamic);
		}

		SInt32 result = ::PopUpMenuSelect(mMenuChoice, startPt.v, startPt.h, 0);

		// Handle menu result
		MenuChoice(result & 0x0000FFFF, btn);
	}
	
	// Look for shift-click to trigger a close
	else if (inMouseDown.macEvent.modifiers & shiftKey)
	{
		bool found = false;
		long btn = 0;
		for(btn = 1; btn <= GetMaxValue(); btn++)
		{
			StRegion rgn;
			::GetControlRegion(GetControlImp()->GetMacControl(), btn, rgn);

			if (rgn.Contains(inMouseDown.whereLocal))
			{
				found = true;
				break;
			}
		}

		if (found)
			// Broadcast change
			BroadcastMessage(msg_3PaneItemsTabsClose, &btn);
	}
	else
		// Do inherited
		CTabsX::Click(inMouseDown);
}

void C3PaneItemsTabs::MenuChoice(SInt32 choice, long btn)
{
	switch(choice)
	{
	case menuChoice_MoveLeft:
		{
			std::pair<unsigned long, unsigned long> p(btn, btn - 1);
			BroadcastMessage(msg_3PaneItemsTabsMove, &p);
		}
		break;
	case menuChoice_MoveRight:
		{
			std::pair<unsigned long, unsigned long> p(btn, btn + 1);
			BroadcastMessage(msg_3PaneItemsTabsMove, &p);
		}
		break;
	case menuChoice_MoveStart:
		{
			std::pair<unsigned long, unsigned long> p(btn, 1);
			BroadcastMessage(msg_3PaneItemsTabsMove, &p);
		}
		break;
	case menuChoice_MoveEnd:
		{
			std::pair<unsigned long, unsigned long> p(btn, GetMaxValue());
			BroadcastMessage(msg_3PaneItemsTabsMove, &p);
		}
		break;
	case menuChoice_Close:
		BroadcastMessage(msg_3PaneItemsTabsClose, &btn);
		break;
	case menuChoice_CloseAll:
		BroadcastMessage(msg_3PaneItemsTabsCloseAll, &btn);
		break;
	case menuChoice_CloseOthers:
		BroadcastMessage(msg_3PaneItemsTabsCloseOthers, &btn);
		break;
	case menuChoice_Rename:
		BroadcastMessage(msg_3PaneItemsTabsRename, &btn);
		break;
	case menuChoice_Lock:
		BroadcastMessage(msg_3PaneItemsTabsLock, &btn);
		break;
	case menuChoice_LockAll:
		BroadcastMessage(msg_3PaneItemsTabsLockAll, &btn);
		break;
	case menuChoice_Dynamic:
		BroadcastMessage(msg_3PaneItemsTabsDynamic, &btn);
		break;
	case menuChoice_Save:
		BroadcastMessage(msg_3PaneItemsTabsSave, &btn);
		break;
	case menuChoice_Restore:
		BroadcastMessage(msg_3PaneItemsTabsRestore, &btn);
		break;
	}
}
