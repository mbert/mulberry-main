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
File:		CScrollWheelAttachment.h
Contact:	Richard Buckle, Sailmaker Software Ltd
			<mailto:richardb@sailmaker.co.uk>
			<http://www.sailmaker.co.uk/>
Version:	1.1.1
Purpose:	Attachment to add Carbon scroll wheel support to windows
Status:		Public domain
*/

/*
*********** Change history  *************
1.1.1	Fixes to CScrollWheelAttachment::DoScrollEvent.
		
		Pass CScrollWheelAttachment::HandleScroll to
		mScrollHandler.Install() to keep CodeWarrior Pro 8 happy.
		
1.1		mScrollWheelFactor now defaults to 1, for compatibility with
		USBOverdrive etc.
		Previously, we recommended kScrollWheelFactor = 3.
		However, drivers such as USBOverdrive are now available
		for Mac OS X, so we should allow them to set the scroll
		wheel factor and leave kScrollWheelFactor at 1. 
		See also <http://www.usboverdrive.com/trouble.html#scroll>
		
		Parameterised the option to scroll background windows,
		since many people feel it is against current UI principles.
		It is now controlled by sAllowScrollBackgroundWindows,
		which defaults to false.
		Use the ctor or GetAllowScrollBackgroundWindows() and
		SetAllowScrollBackgroundWindows() to control this.
		
1.0.2	Conditionalised for correct behaviour on PowerPlant 2.2
1.0.1	No longer requires modifications to LScroller and LScrollerView.
1.0.1	Respects the magnitude of the mouse wheel delta -- it can
		exceed +/-1 if you really thrash the mouse wheel.
1.0.1	Put Carbon-specific code inside #if PP_Target_Carbon blocks.

*********** Licensing  *************
This code is placed "as is" in the public domain in the hope that it may be useful.
You may use and modify it free of charge, however it comes with no warranty whatsoever.
It is your responsibility to determine its fitness for use and no liability, whether 
express or implied, will be accepted.
I would however appreciate being advised of any bug fixes or enhancements that 
you make.

*********** Credits  *************
The core code was generously provided by Matthew Vosburgh. 
I then added LScrollerView support and wrapped it as an attachment.

*********** Requirements  *************
PowerPlant version of CW7 or later.

*********** Usage *************
0. 	The changes to LScroller and LScrollerView that version 1.0 required are no longer
	necessary. If you made these changes, you can keep them or undo them as you see fit.

1.	Add the following files to your project:
	CScrollWheelAttachment.cp
	LEventHandler.cp
	
2.	In your applications's Initialize() override, call:
	AddAttachment( new CScrollWheelAttachment );
	
3.	If you are brave, have a good set of asbestos underwear,
	and want to experiment with scrolling background windows, call:
	AddAttachment( new CScrollWheelAttachment(true) );
	instead.

*********** Remarks *************
It seems to me that there is an awful lot of common code between LScroller and LScrollerView
that could profitably be factored up into a common abstract base class.
*/

#pragma once

#include <LAttachment.h>
#if PP_Target_Carbon
	#include <TEventHandler.h>
#endif	

class	CScrollWheelAttachment : public LAttachment {
public:
	static CScrollWheelAttachment* sScrollWheelAttachment;

	// Instance methods
					CScrollWheelAttachment( bool inAllowScrollBackgroundWindows = false,
											SInt32 inScrollWheelFactor = 1
											);
	virtual			~CScrollWheelAttachment();
	
	// Accessors
	SInt32			GetScrollWheelFactor() 
									{return mScrollWheelFactor;}
	void			SetScrollWheelFactor(SInt32 inVal) 
									{mScrollWheelFactor = inVal;}
	bool			GetAllowScrollBackgroundWindows() 
									{return mAllowScrollBackgroundWindows;}
	void			SetAllowScrollBackgroundWindows(bool inVal) 
									{mAllowScrollBackgroundWindows = inVal;}

#if PP_Target_Carbon
	OSStatus		HandleScroll(
									EventHandlerCallRef	inCallRef,
									EventRef			inEventRef);
#endif

protected:
	// Implementation
#if PP_Target_Carbon
	bool			DoScrollEvent(
									Boolean isVertical, 
									SInt32 delta, 
									Point mouseLoc,
									UInt32	inModifiers);
									
	LPane*			FindScrollerFromHitPane( const LPane* inPane );
#endif	

private:
	// Members
#if PP_Target_Carbon
	TEventHandler<CScrollWheelAttachment>	mScrollHandler;
#endif	
	
	bool			mAllowScrollBackgroundWindows;
	SInt32			mScrollWheelFactor;
	
	// Disallowed methods
	CScrollWheelAttachment( CScrollWheelAttachment& );
	CScrollWheelAttachment& operator= (const CScrollWheelAttachment&);
};
