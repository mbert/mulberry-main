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

#ifndef __CWINDOW__MULBERRY__
#define __CWINDOW__MULBERRY__

#include <JXWindowDirector.h>
#include <JXWindow.h>

#include "CCommander.h"
#include "CWindowStatus.h"
#include "CMainMenu.h"

class JXDirector;
class JXTextMenu;

class CWindow : public CCommander,		// Commander must be first so it gets destroyed last
				public JXWindowDirector,
				public CWindowStatus
{
 public:
	CWindow(JXDirector* owner);

	void Show()
		{ GetWindow()->Show(); }

	//Override OnCreate with window creation code. 
	virtual void OnCreate() {}
	virtual JBoolean Close();

	CMainMenu::MenusArray& GetMenus()
		{ return mainMenus; }
	CMainMenu::MenuId GetMainMenuID(JXTextMenu* menu) const;

 protected:
	CMainMenu::MenusArray mainMenus;

	//Override to do any actions that should be done at close.  Return false
	//if you don't want the window to actually close
	virtual bool AttemptClose();

	virtual void Receive(JBroadcaster* sender, const Message& message);
	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	// The array to hold the main menubar and menus
	void CreateMainMenu(JXContainer *wnd, unsigned short which, bool listen = true);
};

#endif
