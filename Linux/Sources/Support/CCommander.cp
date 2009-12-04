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


// Source for CCommander class

#include "CCommander.h"

#include "CCommands.h"
#include "CMulberryApp.h"

#include "TPopupMenu.h"
#include "HPopupMenu.h"

#include <JXTextMenu.h>
#include <JXWindow.h>
#include <JXWindowDirector.h>

#include <algorithm>

CCommander* CCommander::sTarget = NULL;

void CCommander::CCmdUI::SetText(const cdstring& txt)
{
	// Strip & from text
	mText = txt;
	
	char* p = mText.c_str_mod();
	char* q = p;
	while(*p)
	{
		if (*p == '&')
		{
			p++;
			if (*p != '&')
				continue;
		}
		*q++ = *p++;
	}
	*q = 0;
}

CCommander::CCommander(CCommander* super)
{
	mSuperCommander = NULL;
	mLastSubTarget = NULL;
	SetSuperCommander(super);
}

CCommander::CCommander(JXContainer* superview)
{
	mSuperCommander = NULL;
	mLastSubTarget = NULL;

	// Locate a super commander in the superview hierarchy
	const JXContainer* parent = superview;
	while(parent && !dynamic_cast<const CCommander*>(parent))
		parent = parent->GetEnclosure();
	
	SetSuperCommander(const_cast<CCommander*>(dynamic_cast<const CCommander*>(parent)));

	// If its still null, get the window director and use that as the commander if possible
	if (!GetSuperCommander())
		SetSuperCommander(dynamic_cast<CCommander*>(superview->GetWindow()->GetDirector()));
}

CCommander::~CCommander()
{
	// Remove it as the target
	if (IsTarget())
	{
		if (GetSuperCommander())
			SetTarget(GetSuperCommander());
#ifdef __MULBERRY
		else
			SetTarget(CMulberryApp::sApp);
#endif
	}
	
	// Delete all sub-commanders
	while(mSubCommanders.size())
	{
		delete mSubCommanders.front();
		RemoveSubCommander(mSubCommanders.front());
	}

	SetSuperCommander(NULL);
	mLastSubTarget = NULL;
}

bool CCommander::IsOnDuty() const
{
	CCommander* chain = sTarget;
	while(chain)
	{
		if (chain == this)
			return true;
		chain = chain->GetSuperCommander();
	}
	
	return false;
}

void CCommander::MakeTarget()
{
	// Tell parent we are now the target
	if (GetSuperCommander())
		GetSuperCommander()->AddSubTarget(this);
	
	// Remove any sub-commander marked as last target
	mLastSubTarget = NULL;
}

void CCommander::AddSubTarget(CCommander* cmdr)
{
	// Tell parent we are now the target
	if (GetSuperCommander())
		GetSuperCommander()->AddSubTarget(this);
	
	// Remove any sub-commander marked as last target
	mLastSubTarget = cmdr;
}

CCommander* CCommander::GetSubTarget() const
{
	// Return this one if no sub-commander marked as sub-target
	if (mLastSubTarget == NULL)
		return const_cast<CCommander*>(this);

	// Verify that last sub target is still a sub-commander
	CCommanders::const_iterator found = std::find(mSubCommanders.begin(), mSubCommanders.end(), mLastSubTarget);
	if (found != mSubCommanders.end())
		// Get last sub target to return the value
		return mLastSubTarget->GetSubTarget();
	else
		// Have to return us
		return const_cast<CCommander*>(this);
}

void CCommander::SetSuperCommander(CCommander* cmdr)
{
	if (GetSuperCommander())
		GetSuperCommander()->RemoveSubCommander(this);

	mSuperCommander = cmdr;
	
	if (GetSuperCommander())
		GetSuperCommander()->AddSubCommander(this);
}

void CCommander::AddSubCommander(CCommander* sub)
{
	mSubCommanders.push_back(sub);
}

void CCommander::RemoveSubCommander(CCommander* sub)
{
	if (mLastSubTarget == sub)
		mLastSubTarget = NULL;
	mSubCommanders.erase(remove(mSubCommanders.begin(), mSubCommanders.end(), sub), mSubCommanders.end());
}

bool CCommander::ReceiveMenu(JBroadcaster* sender, const JBroadcaster::Message& message)
{
	if (message.Is(JXMenu::kNeedsUpdate))
	{
		// Iterate over every item in the menu
		JXTextMenu* menu = dynamic_cast<JXTextMenu*>(sender);

		// Do not do menus that are marked to not disable items as they will do their own enabling
		if (menu->GetUpdateAction() == JXMenu::kDisableNone)
			return false;

		// NB Menu count may change while updating a dynamic menu so always
		// get the current count for the loop terminator
		for(unsigned int i = 1; i <= menu->GetItemCount(); i++)
		{
			// Next item
			CCmdUI cmdui;
			cmdui.mMenu = menu;
			cmdui.mMenuIndex = i;
			
			const JString* cmd = NULL;
			if (menu->GetItemID(i, &cmd))
			{
				cmdui.mCmd = CCommand::CommandToCmd(cmd->GetCString());
				GetSubTarget()->UpdateCommand(cmdui.mCmd, &cmdui);
			}
			else
				GetSubTarget()->UpdateCommand(0, &cmdui);

			// Now update the menu item
			menu->SetItemEnable(i, cmdui.mEnable ? kTrue : kFalse);
			if (cmdui.mCheck)
				menu->CheckItem(i);
			if (cmdui.mText.length())
				menu->SetItemText(i, cmdui.mText);
		}
		return false;
	}
	else if(message.Is(JXMenu::kItemSelected))
	{
		SMenuCommandChoice menuchoice;
		menuchoice.mMenu = dynamic_cast<JXTextMenu*>(sender);
		menuchoice.mIndex = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();

		const JString* cmd = NULL;
		if (menuchoice.mMenu->GetItemID(menuchoice.mIndex, &cmd))
			return GetSubTarget()->ObeyCommand(CCommand::CommandToCmd(cmd->GetCString()), &menuchoice);
		else
			return GetSubTarget()->ObeyCommand(0, &menuchoice);
	}

	return false;
}

bool CCommander::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	// Subclass must override to set up command routing

	// Subclass must call inherited version to ensure super command gets to handle it
	return GetSuperCommander() ? GetSuperCommander()->HandleChar(key, modifiers) : false;
}

bool CCommander::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	// Subclass must override to set up command routing

	// Subclass must call inherited version to ensure super command gets to handle it
	return GetSuperCommander() ? GetSuperCommander()->ObeyCommand(cmd, menu) : false;
}

void CCommander::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	// Subclass must override to set up command routing

	// Subclass must call inherited version to ensure super command gets to handle it
	if (GetSuperCommander())
		GetSuperCommander()->UpdateCommand(cmd, cmdui);
}
