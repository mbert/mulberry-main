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


// Header for CApplyRulesMenu class

#ifndef __CAPPLYRULESMENU__MULBERRY__
#define __CAPPLYRULESMENU__MULBERRY__

#include "CListener.h"

#include "templs.h"

// Classes

class CApplyRulesMenu : public CListener
{
private:
	static CMenu		sApplyRulesMenu;
	static bool			sApplyRulesReset;

public:
	enum
	{
		eApply_All = 0,
		eApply_Separator,
		eApply_First
	};

	static CApplyRulesMenu*	sApplyRules;

						CApplyRulesMenu();
	virtual 			~CApplyRulesMenu();

	void				ListenTo_Message(long msg, void* param);	// Respond to list changes

 	static CMenu*		GetMenu()
		{ return &sApplyRulesMenu; }

	static void			DirtyMenuList(void)
		{ sApplyRulesReset = true; }
	static void			ResetMenuList(void);


private:
	static void			Reset(CMenu* main_menu);
};

#endif
