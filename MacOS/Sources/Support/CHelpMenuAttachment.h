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


#pragma once

#include <LAttachment.h>
#include <LMenu.h>

class	CHelpMenuAttachment : public LAttachment {
public:
					CHelpMenuAttachment(ResIDT inHelpMenuID);
	virtual			~CHelpMenuAttachment();

	// get the help menu regardless of platform, headers, etc
	static MenuHandle 		GetHelpMenuHandle();
	// pull HMGetHelpMenuHandle out of InterfaceLib manually
	static MenuHandle 		GetHelpMenuHandleFromInterfaceLib();
	
			void	CommandStatus();

protected:
	// Load up our menu and insert it into the system Help menu.
	// If we can't find the Help menu, we append our menu to the menu bar.
	void			SetUpHelpMenu();
	
	// Return true if ioCommand is a synthetic command for our help menu,
	// and replace ioCommand with the real command for the menu item.
	Boolean			IsHelpCommand(CommandT& ioCommand);
	
	// Look for synthetic commands from our help menu and route them accordingly
	virtual void	ExecuteSelf(MessageT inMessage, void *ioParam);
	
private:
	// data members
	LMenu			mHelpMenu;		// used to map our help items to command numbers
	short			mBaseHelpItems; // the item number of the first of our help commands
	ResIDT			mHelpMenuID;	// used to test synthetic commands
	
	// disallowed methods
					CHelpMenuAttachment();
					CHelpMenuAttachment( const CHelpMenuAttachment &inOriginal );
	CHelpMenuAttachment&	operator=( const CHelpMenuAttachment &inOriginal );

};

