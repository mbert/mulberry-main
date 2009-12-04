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
|	HCmdButtonAttachment.h	 	й1997-1998 John C. Daub.  All rights reserved.	|
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

#ifndef _H_HCmdButtonAttachment
#define _H_HCmdButtonAttachment
#pragma once

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

#include <PP_Prefix.h>
#include <PP_Messages.h>
#include <LAttachment.h>
#include <LPeriodical.h>
#include <LString.h>
#include <LStream.h>
#include <LControl.h>

#ifndef __TYPES__
#include <MacTypes.h>
#endif



class HCmdButtonAttachment : public LAttachment,
								public LPeriodical {

public:

			enum { class_ID = 'HCmd' };
		
	#if ( __PowerPlant__ < 0x01608000 ) // version 1.6/CW11
		static	HCmdButtonAttachment*	CreateFromStream( LStream *inStream );
	#endif
		
									HCmdButtonAttachment( 	PaneIDT		inPaneID,
															UInt8		inSpecifiedKey = 0,
															UInt32		inDelay = 0,
															bool		inDrawShortcut = true,
															MessageT	inMessage = msg_KeyPress,
															bool		inExecuteHost = true );
									HCmdButtonAttachment( LStream		*inStream );
		virtual						~HCmdButtonAttachment();

		virtual	void				SpendTime( const EventRecord &inMacEvent);	

		virtual	bool				IsCommandKeyPressed();	
		virtual	void				MungeTitle( UInt8 inHotKey );
		virtual	UInt8				FindHotKey( bool inForceUpper = true );


protected:
				
		virtual void				ExecuteSelf(MessageT inMessage, void *ioParam);
		
		virtual	LControl*			GetControl() const;
		virtual	void				SetControl( LControl *inControl );
		
		virtual	PaneIDT				GetControlPaneID() const;
		virtual	void				SetControlPaneID( const PaneIDT inControlPaneID );
		
		virtual	bool				GetDrawShortcut() const;
		virtual	void				SetDrawShortcut( const bool inDrawShortcut );
		
		virtual	bool				GetUseSpecifiedKey() const;
		virtual	bool				GetUseFirstTitleChar() const;
		virtual	void				SetUseSpecifiedKey( const bool inUseSpecifiedKey );
		virtual	void				SetUseFirstTitleChar( const bool inUseFirstTitleChar );

		virtual	UInt8				GetHotKey() const;
		virtual	void				SetHotKey( const UInt8 inHotKey );
		
		virtual	bool				GetShowKeyEquiv() const;
		virtual	void				SetShowKeyEquiv( const bool inShowKeyEquiv );
		
		virtual	void				GetOriginalTitle( Str255 outOriginalTitle );
		virtual	void				SetOriginalTitle( ConstStr255Param inTitle );
		
		virtual	void				GetMungedTitle( Str255 outMungedTitle );
		virtual	void				SetMungedTitle( ConstStr255Param inTitle );
		
		virtual	UInt32				GetWhenToDraw() const;
		virtual	void				SetWhenToDraw( const UInt32 inWhenToDraw );
		virtual	void				SetWhenToDraw();
		
		virtual	UInt32				GetDelay() const;
		virtual	void				SetDelay( const UInt32 inDelay );
	
private:

				void				InitCmdButtonAttachment();
			
		// defensive programming.  No copy constructor, no assignment operator
		// and no default constructor (since LAttachment doesn't have them)
		
									HCmdButtonAttachment();
									HCmdButtonAttachment( const HCmdButtonAttachment &inOriginal);
			HCmdButtonAttachment&	operator=( const HCmdButtonAttachment &inOriginal );

				LControl			*mControl;			// pointer to our control
				PaneIDT				mControlID;			// pane ID of the control
				Boolean				mDrawShortcut;		// draw the shortcut or not
				Boolean				mUseSpecifiedKey;	// true = use specified key
														// false = use first char of descriptor

				UInt8				mSpecifiedKey;		// user selected hot key
				Boolean				mFlipped;			// showing the cmd equivs or not?

				LStr255				mOriginalTitle;		// original control title
				LStr255				mMungedTitle;		// modified control title
			
				UInt32				mWhenToDraw;		// buffers our count for delay drawing
				UInt32				mDelay;				// how many ticks to wait before we
														// display the cmd equiv
			

};	


//=============================================================================
//		еее Inlines
//=============================================================================

//=============================================================================
//	е GetControl							[protected, virtual]
//=============================================================================
//	Return a pointer to the LControl on which we function

inline
LControl*
HCmdButtonAttachment::GetControl() const
{
	return mControl;
}


//=============================================================================
//	е SetControl							[protected, virtual]
//=============================================================================
//	Set the control on which we function. nil can be a valid value

inline
void
HCmdButtonAttachment::SetControl(
	LControl *inControl )
{
	mControl = inControl;
}


//=============================================================================
//	е GetControlPaneID						[protected, virtual]
//=============================================================================
//	Return the PaneIDT of our control

inline
PaneIDT
HCmdButtonAttachment::GetControlPaneID() const
{
	// even if we don't have a control (mControl == nil), we should still
	// have a PaneIDT for it since all ctors require this be specified
	
	return mControlID;
}


//=============================================================================
//	е SetControlPaneID						[protected, virtual]
//=============================================================================
//	Set the PaneIDT of our control. Doesn't load the control, create it, get
// 	pointer to it. Just establishes the PaneIDT within the object.

inline
void
HCmdButtonAttachment::SetControlPaneID(
	const PaneIDT inControlPaneID )
{
	mControlID = inControlPaneID;
}


//=============================================================================
//	е GetDrawShortcut						[protected, virtual]
//=============================================================================
//	Return true if we will be drawing the shortcut in the control's title

inline
bool
HCmdButtonAttachment::GetDrawShortcut() const
{
	return mDrawShortcut;
}


//=============================================================================
//	е SetDrawShortcut						[protected, virtual]
//=============================================================================
//	Set if we will be drawing the shortcut in the control's title or not

inline
void
HCmdButtonAttachment::SetDrawShortcut(
	const bool inDrawShortcut )
{
	mDrawShortcut = inDrawShortcut;
}


//=============================================================================
//	е GetUseSpecifiedKey					[protected, virtual]
//=============================================================================
//	Returns true if we are to use the user-specified key. False, then we
//	would utilize the first character of the control's descriptor (title).

inline
bool
HCmdButtonAttachment::GetUseSpecifiedKey() const
{
	return mUseSpecifiedKey;
}

// As a convenience, we'll offer the converse

inline
bool
HCmdButtonAttachment::GetUseFirstTitleChar() const
{
	return !mUseSpecifiedKey;
}


//=============================================================================
//	е SetUseSpecifiedKey					[protected, virtual]
//=============================================================================
//	Set if we use a user specified key (true) or to use the first character
//	of the control's descriptor (false)

inline
void
HCmdButtonAttachment::SetUseSpecifiedKey(
	const bool inUseSpecifiedKey )
{
	mUseSpecifiedKey = inUseSpecifiedKey;
}

// As a convenience, we'll offer the converse

inline
void
HCmdButtonAttachment::SetUseFirstTitleChar(
	const bool inUseFirstTitleChar )
{
	mUseSpecifiedKey = !inUseFirstTitleChar;
}


//=============================================================================
//	е GetHotKey								[protected, virtual]
//=============================================================================
//	Return our hot-key. Even if it's not being used (i.e. they're using the
//	first char of the descriptor), we'll still return something (zero should
//	mean that we're not using a hot-key).

inline
UInt8
HCmdButtonAttachment::GetHotKey() const
{
	return mSpecifiedKey;
}


//=============================================================================
//	е SetHotKey								[protected, virtual]
//=============================================================================
//	Establish our (user specified) hot-key: e.g. "N" for "New", "O" for "Open".
//	We'll assume if you specify something, that you want to use it, so we'll
//	set data accordingly. However if you wish to "clear" this and use the
//	first character of the descriptor, then pass zero.

inline
void
HCmdButtonAttachment::SetHotKey(
	const UInt8 inHotKey )
{
	mSpecifiedKey = inHotKey;
	
	SetUseSpecifiedKey( (bool)inHotKey );
}


//=============================================================================
//	е GetShowKeyEquiv						[protected, virtual]
//=============================================================================
//	Return true if we will display the keyboard shortcut, false if not

inline
bool
HCmdButtonAttachment::GetShowKeyEquiv() const
{
	return mFlipped;
}


//=============================================================================
//	е SetShowKeyEquiv						[protected, virtual]
//=============================================================================
//	Pass true if you wish to display the keyboard shortcut with the control.
//	Be careful however... the way we display the shortcut is to modify the
//	control's descriptor. If the control doesn't have a descriptor (e.g. icon
//	button), then we could be in trouble. You can still use HCmdButtonAttachment
//	for non-descriptor-having controls, just can't print the key equiv.

inline
void
HCmdButtonAttachment::SetShowKeyEquiv(
	const bool inShowKeyEquiv )
{
	mFlipped = inShowKeyEquiv;
}


//=============================================================================
//	е GetOriginalTitle						[protected, virtual]
//=============================================================================
//	Return the original, unmagled title (should be the descriptor of the control)

inline
void
HCmdButtonAttachment::GetOriginalTitle(
	Str255 outOriginalTitle )
{
	LString::CopyPStr(mOriginalTitle, outOriginalTitle );
}


//=============================================================================
//	е SetOriginalTitle						[protected, virtual]
//=============================================================================
//	Set the original title (i.e. title displayed when modifier keys are not
//	depressed) to the given Pascal-style string.

inline
void
HCmdButtonAttachment::SetOriginalTitle(
	ConstStr255Param inTitle )
{
	mOriginalTitle = inTitle;
}


//=============================================================================
//	е GetMungedTitle						[protected, virtual]
//=============================================================================
//	Return the munged title (title with the cmd-key shortcut added to it)

inline
void
HCmdButtonAttachment::GetMungedTitle(
	Str255 outMungedTitle )
{
	LString::CopyPStr(mMungedTitle, outMungedTitle);
}


//=============================================================================
//	е SetMungedTitle						[protected, virtual]
//=============================================================================
//	Set the munged title (i.e. control's title + modifier keys)

inline
void
HCmdButtonAttachment::SetMungedTitle(
	ConstStr255Param inTitle )
{
	mMungedTitle = inTitle;
}


//=============================================================================
//	The following four methods: Get/SetWhenToDraw, Get/SetDelay are related
//	in purpose. HCmdButtonAttachment allows one to specify if the control
//	should display the keyboard shortcut or not (nice for letting your users
//	know it's there as well as what it is). But as a cosmetic addition, one
//	can delay the time until this shortcut is displayed... i.e. wait a second
//	and the show it (can help to reduce flicker and such for power users that
//	just bounce everywhere quickly,  or just make you look cool ;-)
//
//	So these 4 methods help in this regard. WhenToDraw establishes when we
//	actually are to draw, in a value compared to ::TickCount(). When
//	::TickCount() is greater than or equal to our "WhenToDraw" value, then
//	we shall display the keyboard shortcut. The Delay is how many ticks we
//	are to wait from time of key depression until we display the shortcut.
//	And the modifier key must be held down for that total length of time...
//	letting go of the key resets the counter.
//=============================================================================

//=============================================================================
//	е GetDelay								[protected, virtual]
//=============================================================================
//	Return a number (in ticks) that represents how much time we wait before
//	we display the keyboard shortcut (after the modifier key is depressed and
//	held down)

inline
UInt32
HCmdButtonAttachment::GetDelay() const
{
	return mDelay;
}


//=============================================================================
//	е SetDelay								[protected, virtual]
//=============================================================================
//	Establish the amount of time (in ticks) that we are to wait before drawing
//	the keyboard shortcut

inline
void
HCmdButtonAttachment::SetDelay(
	const UInt32 inDelay )
{
	mDelay = inDelay;
}


//=============================================================================
//	е GetWhenToDraw							[protected, virtual]
//=============================================================================
//	Return a number (in ticks, and compared against ::TickCount()) that
//	says when we are to draw/display the key shortcut.

inline
UInt32
HCmdButtonAttachment::GetWhenToDraw() const
{
	return mWhenToDraw;
}


//=============================================================================
//	е SetWhenToDraw							[protected, virtual]
//=============================================================================
//	Set the time (in ticks) to draw at.

inline
void
HCmdButtonAttachment::SetWhenToDraw(
	const UInt32 inWhenToDraw )
{
	mWhenToDraw = inWhenToDraw;
}

//	Often when to draw will be just a function of ::TickCount() + GetDelay()
//	so we'll provide a no-parameter variant of this routine that does just
//	that (saves you a step)

inline
void
HCmdButtonAttachment::SetWhenToDraw()
{
	mWhenToDraw = ::TickCount() + GetDelay();
}





#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif

#endif // #ifndef _H_HCmdButtonAttachment
