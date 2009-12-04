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


// Header for CDynamicMenu class

#ifndef __CDYNAMICMENU__MULBERRY__
#define __CDYNAMICMENU__MULBERRY__

// Classes

namespace CDynamicMenu
{
	void CreateEditMenu(CMenu* menu, bool add_shared = true);
	void DestroyEditMenu(CMenu* menu);

	void CreateMailboxMenu(CMenu* menu, bool add_shared = true);
	void DestroyMailboxMenu(CMenu* menu);

	void CreateMessagesMenu(CMenu* menu, bool add_shared = true);
	void DestroyMessagesMenu(CMenu* menu);

	void CreateDraftMenu(CMenu* menu, bool add_shared = true);
	void DestroyDraftMenu(CMenu* menu);

	void CreateHelpMenu(CMenu* menu, bool add_shared = true);
	void DestroyHelpMenu(CMenu* menu);
	
	CMenu* GetCopytoSubMenu();
	CMenu* GetAppendToSubMenu();
}

#endif
