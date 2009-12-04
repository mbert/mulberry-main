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



/*
Version History
---------------
1.1.1	30/1/01		Fixed UNIVERSAL_INTERFACES_VERSION in CHelpMenuAttachment::GetHelpMenuHandle()
1.1		6/1/01		Fixed memory leak in CHelpMenuAttachment::GetHelpMenuHandleFromInterfaceLib().
1.0.3	4/11/00		Carbonised. Many thanks to Steve Splonskowski <splons@criticalpath.com>
1.0.2	16/4/00		Fixed dtor to remove mHelpMenu from the menu
						bar before deleting it
1.0.1	28/1/99		Corrected documentation error
1.0		31/3/98		First public release

How to use:
-----------

Use Constructor to create a menu and assign command numbers to it
in the usual way. Don't start the menu with a separator item: the
Help Manager will do that for you.

In your app's Initialize() override, 
**NOT** its constructor as previously stated, call
	AddAttachment( new CHelpMenuAttachment( theResID ) );
where theResID is the resource ID of your menu.

Add cases in your ObeyCommand() methods to handle the command numbers
as you wish.
When the user chooses an item from the help menu, the current
LCommander target will get a ProcessCommand in the usual way
and pass it along the command chain.

Carbon
------
HMGetHelp() is only available in CarbonLib 1.1 and Universal headers 3.4.
The new method GetHelpMenuHandle() conditionalises around this.

Submenus
--------
There's a bug with the Menu Manager and submenus in the Help menu,
so if you want submenus you'll have to do a little extra work.
If you don't need submenus in your help menu, you can ignore this section.

NB This ONLY relates to submenus in the Help menu. Submenus in regular
menus work just fine as they are.

For more detail, see the Apple Sample Code "MyHelpMenu", which is
available on the ADC Reference Library CDs and online:
<http://developer.apple.com/samplecode/Sample_Code/Help_Systems/MyHelpMenu.htm>

The bug is that if the user selects from a submenu of the help menu, 
MenuSelect() thinks that a disabled item was chosen and returns 0, 
so we have to use MenuChoice() to find out what was really chosen, 
and respond accordingly. 

The easiest way that I can see to do this in PowerPlant is to override
ClickMenuBar() in your application object along the following lines.
You will need to define constant(s) for the ID(s) of your submenu(s).

void
CMyApp::ClickMenuBar( const EventRecord	&inMacEvent )
{
	StUnhiliteMenu	unhiliter;			// Destructor unhilites menu title
	SInt32			menuChoice;
	CommandT		menuCmd = LMenuBar::GetCurrentMenuBar()->
								MenuCommandSelection(inMacEvent, menuChoice);
	
	// **************** begin fix ****************
	
	if( menuCmd==cmd_Nothing ) {
		menuChoice = ::MenuChoice();
		// see if it's our submenu ID
		if (HiWord(menuChoice) == kMySubMenuID ) {
			menuCmd = LMenuBar::GetCurrentMenuBar()->
						FindCommand( HiWord(menuChoice), LoWord(menuChoice) );
		}
	}
	
	// **************** end fix ******************
	
	if (menuCmd != cmd_Nothing) {
		if (LCommander::GetTarget() != nil) {
			LCommander::SetUpdateCommandStatus(true);
			LCommander::GetTarget()->ProcessCommand(menuCmd, &menuChoice);
		}
	}
}


NB this code is off the top of my head and untested, so treat it with
caution.


Conditions of Use
-----------------

This source code is freeware, but not in the public domain. Sailmaker
Software Limited retains all rights to it.
Redistribution is permitted, but only in an unmodified form.

Comments and questions are welcomed at freeware@sailmaker.co.uk

This source code is distributed AS IS in the hope that it may be useful,
however it is your responsibility to determine its fitness for your
particular purpose and you use it entirely at your own risk.
Sailmaker Software Ltd hereby disclaims all warranties relating to this
software, whether express or implied, including without limitation any
implied warranties of merchantability or fitness for a particular purpose. 
Sailmaker Software Ltd will not be liable for any special, incidental,
consequential, indirect or similar damages due to loss of data or any other
reason, even if Sailmaker Software Ltd or an agent of its has been advised of
the possibility of such damages.  In no event shall Sailmaker Software Ltd be
liable for any damages, regardless of the form of the claim.  The person
using the software bears all risk as to the quality and performance of the
software.

US Government: 
Government End Users:  If you are acquiring this software on behalf of any unit
or agency of the United States Government, the following provisions apply.  The
Government agrees: (i) if this software is supplied to the Department of Defense
(DoD), the software is classified as "Commercial Computer Software" and the
Government is acquiring only "restricted rights" in the software, its
documentation and fonts as that term is defined in Clause 252.227-7013(c)(1) of
the DFARS; and (ii) if the software is supplied to any unit or agency of the
United States Government other than DoD, the Government's rights in the software,
its documentation and fonts will be as defined in Clause 52.227-19(c)(2) of the
FAR or, in the case of NASA, in Clause 18-52.227-86(d) of the NASA Supplement to
the FAR.

So there.
*/

#include "CHelpMenuAttachment.h"
#if !PP_Target_Carbon
#include <Balloons.h>
#endif

typedef OSErr (*HMGetHelpMenuHandleProcPtr)(MenuHandle*);


// constructor
CHelpMenuAttachment::CHelpMenuAttachment(	ResIDT inHelpMenuID )
	: LAttachment( msg_AnyMessage ),
	mHelpMenu(inHelpMenuID),
	mBaseHelpItems(0),
	mHelpMenuID(inHelpMenuID)
	{
	SetUpHelpMenu();
	}


// destructor
CHelpMenuAttachment::~CHelpMenuAttachment()
	{
	if( mHelpMenu.IsInstalled() )
		{
		LMenuBar *theMenuBar = LMenuBar::GetCurrentMenuBar();
		Assert_(theMenuBar);
		theMenuBar->RemoveMenu( &mHelpMenu );
		}
	}


// Load up our menu and insert it into the system Help menu.
// If we can't find the Help menu, we append our menu to the menu bar.
void
CHelpMenuAttachment::SetUpHelpMenu()
	{
	OSErr err = noErr;
	
	// get the balloon help menu handle
	MenuHandle balloonMenuH = GetHelpMenuHandle();

	// get our menu
	MenuHandle ourHelpMenuH = mHelpMenu.GetMacMenuH();
	ThrowIfNil_(ourHelpMenuH);
	
	// we start by installing it into the menu bar
	LMenuBar *theMenuBar = LMenuBar::GetCurrentMenuBar();
	Assert_(theMenuBar);
	theMenuBar->InstallMenu( &mHelpMenu, 0 );
	
	// did we get the balloon help menu?
	if ( (err==noErr) && (balloonMenuH!=nil) )
		{
		short ourItems = ::CountMenuItems(ourHelpMenuH);
		short itemOffset = mBaseHelpItems = ::CountMenuItems(balloonMenuH);
		
		for( SInt32 i = 1; i <= ourItems; i++)
			{
			// append the item
			::AppendMenu( balloonMenuH, "\p " );
			
			// copy all the attributes
			Str255 itemText;
			::GetMenuItemText( ourHelpMenuH, i, itemText );
			::SetMenuItemText( balloonMenuH, mBaseHelpItems + i, itemText );
			
			SInt16 iconIndex;
			::GetItemIcon( ourHelpMenuH, i, &iconIndex );
			::SetItemIcon( balloonMenuH, mBaseHelpItems + i, iconIndex );
			
			SInt16 markChar;
			::GetItemMark( ourHelpMenuH, i, &markChar );
			::SetItemMark( balloonMenuH, mBaseHelpItems + i, markChar );
			
			SInt16 cmdChar;
			::GetItemCmd( ourHelpMenuH, i, &cmdChar );
			::SetItemCmd( balloonMenuH, mBaseHelpItems + i, cmdChar );
			
			Style chStyle;
			::GetItemStyle( ourHelpMenuH, i, &chStyle );
			::SetItemStyle( balloonMenuH, mBaseHelpItems + i, chStyle );
			} // next i
		
		// now remove our own menu from the menu bar
		theMenuBar->RemoveMenu( &mHelpMenu );
		}
	else
		{
		// we don't have a system help menu, leave mHelpMenu in place
		}
	
	::InvalMenuBar();
	}
	

// Look for synthetic commands from our help menu and route them accordingly
void
CHelpMenuAttachment::ExecuteSelf(MessageT inMessage, void *ioParam)
	{
	mExecuteHost = true;
	switch (inMessage)
		{
		case msg_Event:
		case msg_DrawOrPrint:
		case msg_Click:
		case msg_AdjustCursor:
		case msg_KeyPress:
		case msg_PostAction:
			// we ignore all the above
			break;
		
		default:
			{
			MessageT newMessage = inMessage;
			if( IsHelpCommand(newMessage) )
				{
				if( newMessage != inMessage )
					{
					// don't process the old command
					mExecuteHost = false;
					// process the new command instead
					if( LCommander::GetTarget() )
						LCommander::GetTarget()->ProcessCommand(newMessage, ioParam);
					}
				else
					{
					// continue processing in the usual way
					mExecuteHost = true;
					}
				}
			}
			break;
		}
	
	}


// Return true if ioCommand is a synthetic command for our help menu,
// and replace ioCommand with the real command for the menu item.
Boolean
CHelpMenuAttachment::IsHelpCommand(CommandT& ioCommand)
	{
	Boolean result = false;
	ResIDT   menuID;
	SInt16    normalizedMenuItem;
	
	// IsSyntheticCommand only returns true for positive menu IDS
	if (LCommander::IsSyntheticCommand(ioCommand, menuID, normalizedMenuItem))
		{
		// is it our help menu, sitting directly in the menu bar?
		if(menuID == mHelpMenuID)
			{
			ioCommand = mHelpMenu.CommandFromIndex(normalizedMenuItem);
			result = true;
			}
		}
	else
		{
		// this is a surrogate for IsSyntheticCommand that handles the
		// negative ID of the help menu
		menuID = HiWord(ioCommand * -1);
		normalizedMenuItem = LoWord(ioCommand * -1);
		
		// is it our help menu?
		if (menuID == kHMHelpMenuID)
			{
			normalizedMenuItem -= mBaseHelpItems;
			ioCommand = mHelpMenu.CommandFromIndex(normalizedMenuItem);
			result = true;
			}
		}
	
	return result;
}

// get the help menu regardless of platform, headers, etc
MenuHandle 		
CHelpMenuAttachment::GetHelpMenuHandle()
	{
	MenuHandle result = nil;
	OSErr err = noErr;
	
	// This is a bit messy because HMGetHelpMenu was only introduced in 
	// CarbonLib 1.1 and Universal Headers 3.4.
	// Bear with me...
	#if PP_Target_Carbon
		// See if building against Universal Headers 3.4 ie Carbon 1.1 APIs.
		// For some reason Apple use 0X0335 for Universal Headers 3.4.
		#if UNIVERSAL_INTERFACES_VERSION >= 0x0335
			if( CFM_AddressIsResolved_(::HMGetHelpMenu) )
				{
				err = ::HMGetHelpMenu(&result, nil);
				}
		    else
		    	{
		    	result = GetHelpMenuHandleFromInterfaceLib();
		    	}
		    Assert_(result);
		#else 
		// hmm, building with Universal Headers 3.3 ie Carbon 1.0.x APIs
		    	{
		    	result = GetHelpMenuHandleFromInterfaceLib();
		    	}
		#endif
	#else // not Carbon, do it the old way
		err = ::HMGetHelpMenuHandle(&result);
	#endif	
	
	Assert_(err == noErr);
	
	return result;
	}


// pull HMGetHelpMenuHandle out of InterfaceLib manually
MenuHandle 		
CHelpMenuAttachment::GetHelpMenuHandleFromInterfaceLib()
	{
	MenuHandle result = nil;
	CFragConnectionID theConn = NULL;
	Ptr theFnPtr = NULL;
	if (   
		// Can we connnect to the shared lib?
		::GetSharedLibrary(
						"\pInterfaceLib",
	                       kPowerPCCFragArch,
	                       kReferenceCFrag,
	                       &theConn,
	                       NULL, NULL) == noErr
        // can we find HMGetHelpMenuHandle in it?
        && ::FindSymbol(
						theConn,
						"\pHMGetHelpMenuHandle",
						&theFnPtr,
						NULL) == noErr
		&& theFnPtr )
		{
		OSErr err = noErr;
		err = ( *(HMGetHelpMenuHandleProcPtr)(theFnPtr) )( &result );
		Assert_(err==noErr);
		err = ::CloseConnection(&theConn);
		Assert_(err==noErr);
		}
	else
		{
		// running on Mac OS X but built with Universal Headers 3.3
		SignalStringLiteral_("Please rebuild with Universal Headers 3.4 to get the Mac OS X help menu.");
		}
	return result;
	}

void CHelpMenuAttachment::CommandStatus()
{
	// Only bother if its not installed (i.e. on OS X)
	MenuHandle menuH = GetHelpMenuHandle();
	if (!menuH || mHelpMenu.IsInstalled())
		return;

	// Loop over each item in the menu and do command status
	SInt16 menuItem = 0;
	CommandT theCommand = 0;
	LCommander*	theTarget = LCommander::GetTarget();
	while(mHelpMenu.FindNextCommand(menuItem, theCommand))
	{
									// Don't change menu item state for
									//   special commands (all negative
									//   values except cmd_UseMenuItem)
		if (theCommand > 0) {

									// Ask Target if command is enabled,
									//   if the menu item should be marked,
									//   and if the name should be changed
			Boolean isEnabled   = false;
			Boolean usesMark    = false;
			UInt16		mark;
			Str255		itemName;
			itemName[0] = 0;

			if (theTarget != nil) {
				theTarget->ProcessCommandStatus(theCommand, isEnabled,
										usesMark, mark, itemName);
			}

				// Adjust the state of each menu item as needed.
				// Also designate as "used" the Menu containing an
				// enabled item.

			if (isEnabled) {
				::EnableMenuItem(menuH, mBaseHelpItems + menuItem);
			} else {
				::DisableMenuItem(menuH, mBaseHelpItems + menuItem);
			}

			if (usesMark) {
				::SetItemMark(menuH, mBaseHelpItems + menuItem, (SInt16) mark);
			}

			if (itemName[0] > 0) {
				::SetMenuItemText(menuH, mBaseHelpItems + menuItem, itemName);
			}

		}
	}
}
