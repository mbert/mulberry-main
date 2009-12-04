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

/*******************************************************************************\
|																				|
|	HCmdButtonAttachment.cp 	©1997-1998 John C. Daub.  All rights reserved.	|
|																				|
|	See the file "HCmdButtonAttachment README" for full details, instructions,	|
|	changes, licensing agreement, etc.  Due to the important information		|
|	included in that file, if you did not receive a copy of it, please contact	|
|	the author for a copy immediately, before using this code.					|
|																				|
|	John C. Daub						<mailto:hsoi@eden.com>					|
|	<http://www.eden.com/~hsoi/>		<http://www.eden.com/~hsoi/prog.html>	|
|																				|
\*******************************************************************************/


#ifdef PowerPlant_PCH
#include PowerPlant_PCH
#endif


#include "HCmdButtonAttachment.h"

#include <PP_KeyCodes.h>
#include <LView.h>

#ifndef __EVENTS__
#include <Events.h>
#endif

#ifndef __CONTROLS__
#include <Controls.h>
#endif

#ifndef __TEXTUTILS__
#include <TextUtils.h>
#endif


#if ( __PowerPlant__ < 0x01608000 ) // version 1.6/CW11

//=============================================================================
//	¥ CreateFromStream							[static, public]
//=============================================================================
//	Used when registering the class.  Obsolete in more recent versions of
//	PowerPlant, so we conditionally compile it in and out.  If you are using
//	PowerPlant 1.6 (CW11) or higher, you should use the RegisterClass_() macro
//	instead to register your classes.

HCmdButtonAttachment*
HCmdButtonAttachment::CreateFromStream(
	LStream		*inStream )
{
	return (new HCmdButtonAttachment(inStream));
}

#endif


//=============================================================================
//	¥ HCmdButtonAttachment						[public]
//=============================================================================
//	Parameterized constructor

HCmdButtonAttachment::HCmdButtonAttachment(
	PaneIDT		inControlID,
	UInt8		inSpecifiedKey,
	UInt32		inDelay,
	bool		inDrawShortcut,
	MessageT	inMessage,
	bool		inExecuteHost)
		: LAttachment( inMessage, inExecuteHost ),
		mOriginalTitle("\p"),
		mMungedTitle("\p")
{
	mControl = nil;
	mControlID = inControlID;
	mDrawShortcut = inDrawShortcut;
	mUseSpecifiedKey = (inSpecifiedKey != 0);	// if you specify a key, we'll assume you want
												// to use that key.  if you want to use the
												// first letter of the control's descriptor,
												// assign zero to inSpecifiedKey
	mSpecifiedKey = inSpecifiedKey;
	mFlipped = false;
//	mWhenToDraw = 0;							// only usefully initialized at the end of the
												// init routine, so skip it here and save some
												// code size.
	mDelay = inDelay;
	
	InitCmdButtonAttachment();
}


//=============================================================================
//	¥ HCmdButtonAttachment						[public]
//=============================================================================
//	LStream constructor

HCmdButtonAttachment::HCmdButtonAttachment(
	LStream		*inStream )
		: LAttachment(inStream),
		mOriginalTitle("\p"),
		mMungedTitle("\p")
{

	mControl = nil;
	mFlipped = false;
//	mWhenToDraw = 0;						// see above comment for why we don't init this
	
	*inStream >> mControlID;
	
	*inStream >> mUseSpecifiedKey;
	
	// for ease of editing in Constructor, the "Specified key" field
	// is a string of length 1, but since it's a Pascal-style string,
	// 2 bytes are actually stored.  So, we'll read and swallow one dummy
	// character for the length byte.
	
	UInt8 dummy;
	*inStream >> dummy;
	SignalIf_(dummy == 0); // if the string is zero length, that's bad
	*inStream >> mSpecifiedKey;
	
	*inStream >> mDelay;
	*inStream >> mDrawShortcut;
	
	InitCmdButtonAttachment();
}


//=============================================================================
//	¥ ~HCmdButtonAttachment						[public, virtual]
//=============================================================================
//	Destructor

HCmdButtonAttachment::~HCmdButtonAttachment()
{
	// nothing
}


//=============================================================================
//	¥ InitCmdButtonAttachment					[private]
//=============================================================================
//	Private initializer

void
HCmdButtonAttachment::InitCmdButtonAttachment()
{
	// see who we are attached to
	
	LView	*theView = dynamic_cast<LView*>(mOwnerHost);
	
	// normally I would check for nil here, but it might be a legal
	// situation.... so we'll contend with it in other ways...
	
	if ( theView != nil ) {
	
		// get the control and save it for later (faster to cache this
		// pointer than to call FindPaneByID() all the time)
		
		SetControl( dynamic_cast<LControl*>(theView->FindPaneByID(GetControlPaneID())) );
		
		// again, no check for nil here... we have other ways.
	}
	
	// get the original control title
	
	if ( (GetControl() != nil) && GetDrawShortcut() ) {
		Str255 theTitle;
		GetControl()->GetDescriptor(theTitle);
		SetOriginalTitle(theTitle);
	}
	
	SetWhenToDraw();

	// we only need to repeat if we're going to draw or not
	
	if (GetDrawShortcut()) {
		StartRepeating();
	}
}


//=============================================================================
//	¥ ExecuteSelf								[protected, virtual]
//=============================================================================
//	Does the dirty work of making the "click" happen

void
HCmdButtonAttachment::ExecuteSelf(
	MessageT	inMessage,
	void		*ioParam )
{
#pragma unused (inMessage)

	// here's one way we deal with nil pointers
	
	if ( GetControl() == nil ) {
		InitCmdButtonAttachment();
	}
	
	// and now we hope we have something...
	
	if ( GetControl() != nil ) {
	
		// get the EventRecord
		EventRecord theMacEvent = *(static_cast<EventRecord*>(ioParam));

		// we only need worry if the cmdKey is held down
		
		if ( theMacEvent.modifiers & cmdKey ) {

			// find our our hot key.  we want to do this every time (instead of
			// just caching it) because titles could change on the fly, so we should
			// be responsive.
			
			UInt8	theHotKey = FindHotKey();

			// get the key pressed
			
			UInt8 theKeyPress = theMacEvent.message & charCodeMask;
		
			// see if things match up
			if ( LString::CompareIgnoringCase( &theHotKey, &theKeyPress, 1, 1 ) == 0 ) {			

				// we have a winner, so fake the click in the button
				
				GetControl()->SimulateHotSpotClick(kControlButtonPart);
			}
		}		
	}
}


//=============================================================================
//	¥ SpendTime									[public, virtual]
//=============================================================================
//	Used to modify the look of the control title, appending the cmd-key
//	shortcut to the title of the control

void
HCmdButtonAttachment::SpendTime(
	const EventRecord	&inMacEvent )
{
#pragma unused(inMacEvent)

	// have to put checks in here, lest risk crashing....
	
	if ( GetControl() == nil ) {
		InitCmdButtonAttachment();
	}
	
	if ( GetControl() == nil ) {
		return;
	}
		
	// we'll only do this if we're enabled (and visble), active, and if the cmd-key is
	// held down (this helps prevent modifying a control that wouldn't make sense
	// to modify (e.g. disabled), and also keeps controls in a not-active state
	// (e.g. backgrounded) from being modified)
	
	LControl *theControl = GetControl();
	
	if ( theControl->IsEnabled() &&
			theControl->IsActive() &&
			IsCommandKeyPressed() ) {
		
		// modify the title to have the shortcut appended to it

		UInt8	theHotKey = FindHotKey();		
		MungeTitle( theHotKey );

		// and if we already displaying the munged title, don't switch it
		// again, to reduce flicker
		
		if ( !GetShowKeyEquiv() ) {
			Str255 theTitle;
			GetMungedTitle(theTitle);
			theControl->SetDescriptor( theTitle );
			SetShowKeyEquiv( true );
		}

	} else {
	
		// no cmd-key down, ensure we're looking normal.  and again, only
		// do this if we need to, to reduce flicker

		if ( GetShowKeyEquiv() ) {
			Str255 theTitle;
			GetOriginalTitle( theTitle );
			theControl->SetDescriptor( theTitle );
			SetShowKeyEquiv( false );
		}
	}
}


//=============================================================================
//	¥ IsCommandKeyPressed							[public, virtual]
//=============================================================================
//	Looks to see if the command key is held down or not.  We have to use
//	GetKeys() to do this as modifiers don't generate events

bool
HCmdButtonAttachment::IsCommandKeyPressed()
{
	// see if the command key is held down
	
	if (::GetCurrentKeyModifiers() & cmdKey) {
	
		// if so, make sure we don't draw until mDelay has passed
		
		if ( ::TickCount() >= GetWhenToDraw() ) {
			return true;
		} else {
			return false;
		}
	} else {
	
		// it's not held down, so keep upping the ante on when to
		// draw so it all works right
		
		SetWhenToDraw();
		return false;
	}
}


//=============================================================================
//	¥ MungeTitle									[public, virtual]
//=============================================================================
//	Munge the title to display the command key equiv appended to the control
//	title.  This current will take "Fred" and make it "Fred cmdF".  Override
//	this routine to modify how the title is munged (prepend the key combo,
//	insert a hyphen between the cmd and the char, etc.)

void
HCmdButtonAttachment::MungeTitle(
	UInt8	inHotKey )
{	
	LStr255 theTitle;
	
	//GetOriginalTitle( theTitle );			// start with the original
	//theTitle += char_Space;					// add a space
	theTitle += char_Propeller;				// add the command key
	theTitle += inHotKey;					// and finally the given key
	
	// and set it internally
	
	SetMungedTitle( theTitle );
}


//=============================================================================
//	¥ FindHotKey									[public, virtual]
//=============================================================================
//	Based upon internal settings and configuration, determine the
//	magic key/character which we are to look for and act upon

UInt8
HCmdButtonAttachment::FindHotKey(
	bool		inForceUpper )
{
	UInt8	theHotKey = 0;
	
	if ( GetControl() != nil ) {
		if ( GetUseSpecifiedKey() ) {
			theHotKey = GetHotKey();
		} else {
		
			// get the button's title
			
			Str255	theTitle;
			GetControl()->GetDescriptor(theTitle);				
			theHotKey = theTitle[1];
		}

		// make sure the hot key char is uppercase, if desired
		
		if ( inForceUpper && ((theHotKey >= 'a') && (theHotKey <= 'z')) ) {
			::UppercaseText( (Ptr)&theHotKey, 1, smSystemScript );
		}
	}
	
	return theHotKey;
}



// the following functions are declared inline in the header file. The #pragma
// mark let's them show up in the CW IDE's function popup for ease of navigation
// and reference. :-)

#pragma mark HCmdButtonAttachment::GetControl
#pragma mark HCmdButtonAttachment::SetControl
#pragma mark HCmdButtonAttachment::GetControlPaneID
#pragma mark HCmdButtonAttachment::SetControlPaneID
#pragma mark HCmdButtonAttachment::GetDrawShortcut
#pragma mark HCmdButtonAttachment::SetDrawShortcut
#pragma mark HCmdButtonAttachment::GetUseSpecifiedKey
#pragma mark HCmdButtonAttachment::SetUseSpecifiedKey
#pragma mark HCmdButtonAttachment::GetUseFirstTitleChar
#pragma mark HCmdButtonAttachment::SetUseFirstTitleChar
#pragma mark HCmdButtonAttachment::GetHotKey
#pragma mark HCmdButtonAttachment::SetHotKey
#pragma mark HCmdButtonAttachment::GetShowKeyEquiv
#pragma mark HCmdButtonAttachment::SetShowkeyEquiv
#pragma mark HCmdButtonAttachment::GetOriginalTitle
#pragma mark HCmdButtonAttachment::SetOriginalTitle
#pragma mark HCmdButtonAttachment::GetMungedTitle
#pragma mark HCmdButtonAttachment::SetMungedTitle
#pragma mark HCmdButtonAttachment::GetDelay
#pragma mark HCmdButtonAttachment::SetDelay
#pragma mark HCmdButtonAttachment::GetWhenToDraw
#pragma mark HCmdButtonAttachment::SetWhenToDraw