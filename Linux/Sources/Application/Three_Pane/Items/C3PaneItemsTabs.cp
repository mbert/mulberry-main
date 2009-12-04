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

#include "CMultibitPopup.h"
#include "C3PaneItems.h"


enum
{
	menuChoice_MoveLeft = 1,
	menuChoice_MoveRight,
	menuChoice_MoveStart,
	menuChoice_MoveEnd,
	//menuChoice_Separator1,
	menuChoice_Close,
	menuChoice_CloseAll,
	menuChoice_CloseOthers,
	//menuChoice_Separator2,
	menuChoice_Rename,
	//menuChoice_Separator3,
	menuChoice_Lock,
	menuChoice_LockAll,
	menuChoice_Dynamic,
	//menuChoice_Separator4,
	menuChoice_Save,
	menuChoice_Restore
};

C3PaneItemsTabs::C3PaneItemsTabs(JXContainer* enclosure,
		const HSizingOption hSizing, const VSizingOption vSizing,
		const JCoordinate x, const JCoordinate y,
		const JCoordinate w, const JCoordinate h)
		: JXTabs(enclosure, hSizing, vSizing, x, y, w, h)
{
	mOwner = NULL;
	mPopup = new CMultibitPopup("", this, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 10, 10);
	mPopup->SetMenuItems("Move Left|Move Right|Move to Start|Move to End %l|Close|Close All|Close Others %l|Rename %l|Lock %b|Lock All|Dynamic %b %l|Save Current Set|Restore Last Set");
	mPopup->InitBits();
	mPopup->SetToHiddenPopupMenu(kTrue);
	mPopup->Hide();
	ListenTo(mPopup);
	
	mActiveTabs = false;
}
 
void C3PaneItemsTabs::OnRightClick(JIndex btn, const JPoint& pt, const JXButtonStates& buttonStates, const JXKeyModifiers& modifiers)
{
	bool in_button = (btn > 0) && HasActiveTabs();
	
	if (in_button && (btn > 1))
	{
		mPopup->EnableItem(menuChoice_MoveLeft);
		mPopup->EnableItem(menuChoice_MoveStart);
	}
	else
	{
		mPopup->DisableItem(menuChoice_MoveLeft);
		mPopup->DisableItem(menuChoice_MoveStart);
	}
	if (in_button && (btn < GetTabCount()))
	{
		mPopup->EnableItem(menuChoice_MoveRight);
		mPopup->EnableItem(menuChoice_MoveEnd);
	}
	else
	{
		mPopup->DisableItem(menuChoice_MoveRight);
		mPopup->DisableItem(menuChoice_MoveEnd);
	}

	if (in_button)
	{
		// Determine if locked
		bool is_locked = mOwner && mOwner->IsSubstituteLocked(btn - 1);
		if (is_locked)
		{
			mPopup->DisableItem(menuChoice_Close);
			mPopup->DisableItem(menuChoice_Dynamic);
		}
		else
		{
			mPopup->EnableItem(menuChoice_Close);
			mPopup->EnableItem(menuChoice_Dynamic);
		}
		mPopup->SetBit(menuChoice_Lock, is_locked);

		// Determine if dynamic
		bool is_dynamic = mOwner && mOwner->IsSubstituteDynamic(btn - 1);
		mPopup->SetBit(menuChoice_Dynamic, is_dynamic);

		//mPopup->EnableItem(menuChoice_Close);
		mPopup->EnableItem(menuChoice_CloseAll);
		mPopup->EnableItem(menuChoice_CloseOthers);
		mPopup->EnableItem(menuChoice_Rename);
		mPopup->EnableItem(menuChoice_Lock);
		mPopup->EnableItem(menuChoice_LockAll);
		//mPopup->EnableItem(menuChoice_Dynamic);
	}
	else
	{
		mPopup->DisableItem(menuChoice_Close);
		mPopup->DisableItem(menuChoice_CloseAll);
		mPopup->DisableItem(menuChoice_CloseOthers);
		mPopup->DisableItem(menuChoice_Rename);
		mPopup->DisableItem(menuChoice_Lock);
		mPopup->DisableItem(menuChoice_LockAll);
		mPopup->DisableItem(menuChoice_Dynamic);
	}

	mPopupIndex = btn;
	mPopup->PopUp(this, pt, buttonStates, modifiers);
}

void C3PaneItemsTabs::OnShiftClick(JIndex index)
{
	mOwner->OnTabsClose(index);
}

void C3PaneItemsTabs::Receive(JBroadcaster* sender, const Message& message)
{
	if (sender == mPopup && message.Is(JXMenu::kItemSelected))
	{
		const JXMenu::ItemSelected* is = dynamic_cast<const JXMenu::ItemSelected*>(&message);
		MenuChoice(is->GetIndex(), mPopupIndex);
	}
	else
		JXTabs::Receive(sender, message);
}

void C3PaneItemsTabs::MenuChoice(JIndex choice, JIndex btn)
{
	switch(choice)
	{
	case menuChoice_MoveLeft:
		{
			std::pair<unsigned long, unsigned long> p(btn, btn - 1);
			mOwner->OnTabsMove(p);
		}
		break;
	case menuChoice_MoveRight:
		{
			std::pair<unsigned long, unsigned long> p(btn, btn + 1);
			mOwner->OnTabsMove(p);
		}
		break;
	case menuChoice_MoveStart:
		{
			std::pair<unsigned long, unsigned long> p(btn, 1);
			mOwner->OnTabsMove(p);
		}
		break;
	case menuChoice_MoveEnd:
		{
			std::pair<unsigned long, unsigned long> p(btn, GetTabCount());
			mOwner->OnTabsMove(p);
		}
		break;
	case menuChoice_Close:
		mOwner->OnTabsClose(btn);
		break;
	case menuChoice_CloseAll:
		mOwner->OnTabsCloseAll();
		break;
	case menuChoice_CloseOthers:
		mOwner->OnTabsCloseOthers(btn);
		break;
	case menuChoice_Rename:
		mOwner->OnTabsRename(btn);
		break;
	case menuChoice_Lock:
		mOwner->OnTabsLock(btn);
		break;
	case menuChoice_LockAll:
		mOwner->OnTabsLockAll();
		break;
	case menuChoice_Dynamic:
		mOwner->OnTabsDynamic(btn);
		break;
	case menuChoice_Save:
		mOwner->OnTabsSave();
		break;
	case menuChoice_Restore:
		mOwner->OnTabsRestore();
		break;
	}
}
