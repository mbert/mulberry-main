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


// Header for CWindowsMenu class

#ifndef __CWINDOWSMENU__MULBERRY__
#define __CWINDOWSMENU__MULBERRY__

#include <vector>
#include "CCommander.h"
#include <functional>

class JXWindowDirector;
class JXTextMenu;

class CWindowsMenu
{
	typedef std::vector<JXWindowDirector*> CWindowList;

private:
	static CWindowList		sWindowList;
	static long				sNumServers;
	static bool				sDirty;

	class AddWindowToMenu : public std::unary_function<void, JXWindowDirector*>
	{
	public:
		AddWindowToMenu(JXTextMenu* menu) : menu_(menu), shortcut_(0) {}
		inline void operator()(JXWindowDirector* w);
	private:
		JXTextMenu* menu_;
		short shortcut_;
		static const JCharacter shortcut_keys_[][7];
	};

public:
	
	CWindowsMenu() {}
	~CWindowsMenu() {}

	static void			Init();
	static void			AddWindow(JXWindowDirector* theWindow, bool server = false);
	static void			RemoveWindow(JXWindowDirector* theWindow);
	static void			RenamedWindow();
	static JXWindowDirector* GetWindow(SInt16 item_num);

	static void			UpdateMenu(JXTextMenu* menu);
	static void			UpdateMenuItem(CCommander::CCmdUI* cmdui);
};


#endif
