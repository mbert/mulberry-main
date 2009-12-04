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
File:		CScrollWheelAttachment.cp
Contact:	Richard Buckle, Sailmaker Software Ltd
			<mailto:richardb@sailmaker.co.uk>
			<http://www.sailmaker.co.uk/>
Version:	1.1.1
Purpose:	Attachment to add Carbon scroll wheel support to windows
Status:		Public domain
*/

// See CScrollWheelAttachment.h for important usage notes

#include "CScrollWheelAttachment.h"
#include <LScrollerView.h>
#include <LScroller.h>

#if PP_Target_Carbon && (__PowerPlant__	< 0x02208000)
	EventHandlerUPP	TEventHandler<CScrollWheelAttachment>::sHandlerUPP = 0;
#endif

CScrollWheelAttachment* CScrollWheelAttachment::sScrollWheelAttachment = NULL;

//
CScrollWheelAttachment::CScrollWheelAttachment(	bool inAllowScrollBackgroundWindows,
												SInt32 inScrollWheelFactor
												)
	:
	mScrollWheelFactor(inScrollWheelFactor),
	mAllowScrollBackgroundWindows(inAllowScrollBackgroundWindows)
	{
#if PP_Target_Carbon
	mScrollHandler.Install( ::GetApplicationEventTarget(), 
								kEventClassMouse, 
								kEventMouseWheelMoved, this, 
								&CScrollWheelAttachment::HandleScroll );
#endif
	
	sScrollWheelAttachment = this;
	}
	
//
CScrollWheelAttachment::~CScrollWheelAttachment()
	{
	sScrollWheelAttachment = NULL;
	}


#if PP_Target_Carbon
//
OSStatus
CScrollWheelAttachment::HandleScroll(
								EventHandlerCallRef	/*inCallRef*/,
								EventRef			inEvent)
	{
	OSStatus result = eventNotHandledErr;
	EventMouseWheelAxis	axis;
	SInt32	delta;
	Point	mouseLoc;
	UInt32	modifiers;
	bool	handledEvent = false;

	::GetEventParameter(inEvent, kEventParamMouseWheelAxis, typeMouseWheelAxis,
		NULL, sizeof(axis), NULL, &axis);
	
	::GetEventParameter(inEvent, kEventParamMouseWheelDelta, typeLongInteger,
		NULL, sizeof(delta), NULL, &delta);

	::GetEventParameter(inEvent, kEventParamMouseLocation, typeQDPoint,
		NULL, sizeof(mouseLoc), NULL, &mouseLoc);

	::GetEventParameter(inEvent, kEventParamKeyModifiers, typeUInt32,
		NULL, sizeof(modifiers), NULL, &modifiers);

	handledEvent = DoScrollEvent((axis == kEventMouseWheelAxisY), delta, mouseLoc, modifiers);

	if (handledEvent) 
		{result = noErr;}
	
	return result;
	}

// We can allow the scroll wheel to scroll background windows of our app.
// This is controlled by mAllowScrollBackgroundWindows.
// It is somewhat questionable UI, as it breaks the fundamental principle of
// consistency. "My Mom" might become rather confused by some background windows
// scrolling and others not.
// However, scrolling is such an innocuous, non-destructive process that I've
// decided to go with it.
// Maybe some day both Carbon and Cocoa apps will all support scrolling of 
// background windows.
bool
CScrollWheelAttachment::DoScrollEvent(
	Boolean	inIsVertical, 
	SInt32	inDelta, 
	Point	inMouseLoc,
	UInt32	/*inModifiers*/)
{

	WindowRef 		macHitWindow = nil;
	WindowPartCode	hitPart = ::FindWindow(inMouseLoc, &macHitWindow);
	LWindow* 		hitWindow = nil;
	bool			handledEvent = false;
	LPane*			paneToScroll = nil;
	
	if (macHitWindow && (hitPart == inContent))
		{
		hitWindow = LWindow::FetchWindowObject(macHitWindow);
		}
		
	if (	hitWindow 
			&& (
				GetAllowScrollBackgroundWindows() 
				|| UDesktop::WindowIsSelected(hitWindow)
				)
			)
		{
		hitWindow->GlobalToPortPoint(inMouseLoc);
		
		LPane*	hitPane = hitWindow->FindDeepSubPaneContaining(inMouseLoc.h, inMouseLoc.v);
		paneToScroll = FindScrollerFromHitPane(hitPane);
		}
		
	if (paneToScroll == nil)
		{
		// The mouse is not over a view that can scroll or it was not over the active window.
		// In this case, we'll start with the target and work out until we find a scroller.
		
		LCommander* theTarget = LCommander::GetTarget();
		LPane* theTargetPane = dynamic_cast<LPane*>(theTarget);
		while( theTargetPane == nil && theTarget != nil )
			{
			theTarget = theTarget->GetSuperCommander();
			theTargetPane = dynamic_cast<LPane*>(theTarget);
			}
		paneToScroll = FindScrollerFromHitPane(theTargetPane);
		}

	// Determine type of pane
	LView* viewToScroll = dynamic_cast<LView*>(paneToScroll);

	if( viewToScroll )
		{
		SPoint32 scrollUnit;
		viewToScroll->GetScrollUnit(scrollUnit);
		if (inIsVertical)
			{
			scrollUnit.h = 0;
			scrollUnit.v *= -inDelta * GetScrollWheelFactor();
			}
		else
			{
			scrollUnit.h *= -inDelta * GetScrollWheelFactor();
			scrollUnit.v = 0;
			}
		viewToScroll->ScrollPinnedImageBy(scrollUnit.h, scrollUnit.v, Refresh_Yes);
		handledEvent = true;
		}
	
	return handledEvent;
	}

// starting with inPane and walking up the containment heirarchy,
// look for a scrollable view.
// If none found, return nil.
// If inPane is nil, return nil.
LPane*			
CScrollWheelAttachment::FindScrollerFromHitPane( const LPane* inPane )
	{
	LPane* result = nil;
	const LScroller*		scroller = nil;
	const LScrollerView*	scrollerView = nil;
	const LPane* theTargetPane = inPane;
	
	while(theTargetPane != nil && result == nil)
	{
		scroller = dynamic_cast<const LScroller*>(theTargetPane);
		if (scroller)
		{
			result = scroller->GetScrollingView();
		}
		else
		{
			scrollerView = dynamic_cast<const LScrollerView*>(theTargetPane);
			if (scrollerView)
			{
				result = scrollerView->GetScrollingView();
			}
		}
		
		theTargetPane = theTargetPane->GetSuperView();
	}	
	
	return result;
	}
	
#endif // PP_Target_Carbon
