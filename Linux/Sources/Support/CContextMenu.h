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


// CContextMenu

#ifndef __CCONTEXTMENU__MULBERRY__
#define __CCONTEXTMENU__MULBERRY__

#include <JBroadcaster.h>

#include "CMainMenu.h"

class CContextMenu : virtual public JBroadcaster
{
public:
		CContextMenu(JXContainer* owner)
			{ mOwner = owner; mContextMenu = NULL; }
	
protected:
	JXContainer*	mOwner;
	JXTextMenu*		mContextMenu;

	virtual void	CreateContextMenu(CMainMenu::EContextMenu items);

	virtual void 	ContextEvent(const JPoint& pt,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);
private:
	CContextMenu();
};

#endif
