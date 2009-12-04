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


// Source for CHelpAttach class

#include "CContextMenu.h"

#include <JXTextMenu.h>

#include <cassert>

#pragma mark ____________________________CContextMenu


// O T H E R  M E T H O D S ____________________________________________________________________________

void CContextMenu::CreateContextMenu(CMainMenu::EContextMenu items)
{
	// If its already set, don't do it again
	// this allows derived classes to override a default context
	// menu set in their base class
	if (mContextMenu)
		return;

	assert(mOwner != NULL);

	// Create array of ontext menus
	JPtrArray<JXMenu> menus(JPtrArrayT::kForgetAll);
	CMainMenu::sMMenu->CreateContext(items, mOwner, menus);

	// Grab the first one
	assert(menus.GetElementCount() > 0);
	mContextMenu = static_cast<JXTextMenu*>(menus.FirstElement());

	// Listen to them all
	// NB Use of virtual base means that mOwner also ends up listening!
	for(unsigned int i = 1; i <= menus.GetElementCount(); i++)
		ListenTo(menus.NthElement(i));
}

void CContextMenu::ContextEvent(const JPoint& pt,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers)
{
	// Only if context set - not all widgets that have this
	// class mixed in need context menus
	if (mContextMenu)
	{
		assert(mOwner != NULL);

		mContextMenu->PopUp(mOwner, pt, buttonStates, modifiers);
	}
}
