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

#ifdef PowerPlant_PCH
#include PowerPlant_PCH
#endif

#include "CCarbonMBAR.h"

#include "CMulberryCommon.h"

#include <LMenu.h>
#include <PP_Messages.h>

#include <ToolUtils.h>

// ---------------------------------------------------------------------------
//	¥ CCarbonMBAR							Constructor				  [public]
// ---------------------------------------------------------------------------

CCarbonMBAR::CCarbonMBAR(
	ResIDT	inMBARid)

	: LMenuBar(inMBARid)
{
	mDynamicItems = NULL;
	mDynamicItemsMap = NULL;

	LMenu	*theMenu = nil;
	while (FindNextMenu(theMenu))
		ProcessMenu(theMenu);
}

CCarbonMBAR::~CCarbonMBAR()
{
	mDynamicItems = NULL;
	delete mDynamicItemsMap;
	mDynamicItemsMap = NULL;
}

// ---------------------------------------------------------------------------
//	¥ FindKeyCommand												  [public]
// ---------------------------------------------------------------------------
//	Return the Command number corresponding to a keystroke
//		Returns cmd_Nothing if the keystroke is not a menu equivalent
//

CommandT CCarbonMBAR::FindKeyCommand(const EventRecord& inKeyEvent, SInt32& outMenuChoice) const
{
	// Do default action
	CommandT cmd = LMenuBar::FindKeyCommand(inKeyEvent, outMenuChoice);
	
	// Look for dynamic items menu
	if (HiWord(outMenuChoice) == kDynamicItemsID)
	{
		// Hilite corresponding main menu
		::HiliteMenu((*mDynamicItemsMap)[LoWord(outMenuChoice)]);
	}
	
	// Return as normal
	return cmd;
}

void CCarbonMBAR::ProcessMenu(LMenu* menu)
{
	// Look at each item
	MenuHandle menuH = menu->GetMacMenuH();
	bool was_dynamic = false;
	bool is_dynamic = false;
	for(int i = 1; i <= ::CountMenuItems(menuH); i++)
	{
		Style sty;
		::GetItemStyle(menuH, i, &sty);
		if (sty)
		{
			// Get Mercutio styles
			bool shift = (sty & extend);
			bool option = (sty & condense);
			is_dynamic = (sty & outline);
			
			// Reset item style
			::SetItemStyle(menuH, i, normal);
			
			// Now set modifiers
			int modifiers  = kMenuNoModifiers;
			if (shift)
				modifiers |= kMenuShiftModifier;
			if (option)
				modifiers |= kMenuOptionModifier;
			::SetMenuItemModifiers(menuH, i, modifiers);
		}
		else
			is_dynamic = false;
		
		// Get command key character
		CharParameter cmdKey;
		::GetItemCmd(menuH, i, &cmdKey);
		if (isalpha(cmdKey) && islower(cmdKey))
		{
			// Map Mercutio lowercase specials to glyph's and actual key codes
			// I don't know some of the keycodes so they're set to 0
			const short mappings[][2] = {
				{0x0B, 0x0D},	// return
				{0x04, 0x00},	// Enter
				{0x02, 0x09},	// Tab
				{0x00, 0x00},	// Num Lock
				{0x6F, 0x00},	// F1
				{0x70, 0x00},	// F2
				{0x71, 0x00},	// F3
				{0x72, 0x00},	// F4
				{0x73, 0x00},	// F5
				{0x74, 0x00},	// F6
				{0x75, 0x00},	// F7
				{0x76, 0x00},	// F8
				{0x77, 0x00},	// F9
				{0x78, 0x00},	// F10
				{0x79, 0x00},	// F11
				{0x7A, 0x00},	// F12
				{0x87, 0x00},	// F13
				{0x88, 0x00},	// F14
				{0x89, 0x00},	// F15
				{0x67, 0x00},	// Help
				{0x17, 0x00},	// Del
				{0x0A, 0x00},	// Forward Del
				{0x19, 0x00},	// Home
				{0x10, 0x00},	// End
				{0x62, 0x00},	// Page Up
				{0x6B, 0x00}	// Page Down
				};
			
			// Set appropriate glyph and key from mapping
			switch(cmdKey)
			{
			case 'a'...'z':
				::SetMenuItemKeyGlyph(menuH, i, mappings[cmdKey - 'a'][0]);
				::SetItemCmd(menuH, i, mappings[cmdKey - 'a'][1]);
				break;
			case 0x10:	// Up Arrow
				::SetMenuItemKeyGlyph(menuH, i, 0x68);
				break;
			case 0x11:	// Down Arrow
				::SetMenuItemKeyGlyph(menuH, i, 0x6A);
				break;
			case 0x12:	// Left Arrow
				::SetMenuItemKeyGlyph(menuH, i, 0x64);
				break;
			case 0x13:	// Right Arrow
				::SetMenuItemKeyGlyph(menuH, i, 0x65);
				break;
			default:;
			}
		}
		
		// Check for dynamic item and move it to hidden menu
		if (was_dynamic && is_dynamic)
			AddDynamicItem(menu, i--);
		was_dynamic = is_dynamic;
	}
}

void CCarbonMBAR::AddDynamicItem(LMenu* menu, SInt16 index)
{
	// Create dynamic menu if not already done
	if (!mDynamicItems)
	{
		mDynamicItems = new LMenu(kDynamicItemsID);
		InstallMenu(mDynamicItems, kInsertHierarchicalMenu);
		
		mDynamicItemsMap = new TArray<SInt16>;
	}
	
	MenuHandle oldH = menu->GetMacMenuH();
	MenuHandle newH = mDynamicItems->GetMacMenuH();

	// Create new item in dynamic menu with same command ID as old one
	SInt16 new_index = ::CountMenuItems(newH) + 1;
	mDynamicItems->InsertCommand("\p-", menu->CommandFromIndex(index), new_index);
	
	// Copy over menu item title
	cdstring title = ::GetMenuItemTextUTF8(oldH, index);
	::SetMenuItemTextUTF8(newH, new_index, title);
	
	// Copy over command ID
	MenuCommand cmdID;
	::GetMenuItemCommandID(oldH, index, &cmdID);
	::SetMenuItemCommandID(newH, new_index, cmdID);

	// Copy over command key
	CharParameter cmd;
	::GetItemCmd(oldH, index, &cmd);
	if (cmd)
	{
		::SetItemCmd(newH, new_index, cmd);
		
		// Copy over modifiers
		UInt8 modifiers  = kMenuNoModifiers;
		::GetMenuItemModifiers(oldH, index, &modifiers);
		::SetMenuItemModifiers(newH, new_index, modifiers);
		
		// Copy over glyph - not really required since menu is never visible
		SInt16 glyph;
		::GetMenuItemKeyGlyph(oldH, index, &glyph);
		::SetMenuItemKeyGlyph(newH, new_index, glyph);
	}
	
	// Should copy other items like text encoding, font id, icon, style etc
	// but I don't need these right now so will ignore them
	
	// Remove the old item
	menu->RemoveItem(index);
	
	// Add to main menu map to enable main meni hiliting of selected dynamic menu item
	mDynamicItemsMap->AddItem(menu->GetMenuID());
}
