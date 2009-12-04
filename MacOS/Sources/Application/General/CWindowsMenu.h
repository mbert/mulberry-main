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


// MENU's

// Classes

class LMenu;
class LWindow;
class LArray;

class CWindowsMenu
{
private:
	static LMenu*			sMenu;
	static LArray*			sWindowList;
	static unsigned long	sNumServers;
	static bool				sDirty;

public:

						CWindowsMenu();
	virtual 			~CWindowsMenu();

	static void			AddWindow(LWindow* theWindow, bool server = false);
	static void			RemoveWindow(LWindow* theWindow);
	static void			RenamedWindow();
	static LWindow*		GetWindow(SInt16 menu_num, SInt16 item_num);

	static void			UpdateMenuList();
};

#endif
