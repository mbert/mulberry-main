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


// Source for CHelpAttach class

#include "CHelpAttach.h"
#include "CBalloonApp.h"



// __________________________________________________________________________________________________
// C L A S S __ C H E L P A T T A C H
// __________________________________________________________________________________________________

LPane* CHelpAttach::sHelpPane = nil;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CHelpAttach::CHelpAttach(short strId, short index)
		: LAttachment(msg_ShowHelp, true)
{
	// Check for availablity of Balloon Help
	mHasBalloonHelp = UEnvironment::HasGestaltAttribute(gestaltHelpMgrAttr,gestaltHelpMgrPresent);

	// Cache the resource info
	mStrId = strId;
	mIndex = index;
}

CHelpAttach::CHelpAttach(LStream *inStream)
		: LAttachment(inStream)
{
	// Check for availablity of Balloon Help
	mHasBalloonHelp = UEnvironment::HasGestaltAttribute(gestaltHelpMgrAttr,gestaltHelpMgrPresent);

	// Always force to our message
	mMessage = msg_ShowHelp;

	// Cache the resource info
	*inStream >> mStrId;
	*inStream >> mIndex;
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Show help balloon
void CHelpAttach::ExecuteSelf(MessageT inMessage, void *ioParam)
{
#if PP_Target_Classic
	// Only do if help is present and Balloons are on
	if (!mHasBalloonHelp || !::HMGetBalloons())
	{
		sHelpPane = nil;
		return;
	}

	// Check for same pane and same balloon
	if (((LPane*) ioParam == sHelpPane) && ::HMIsBalloon() && SameBalloon(ioParam))
		return;

	// Remove any old balloon
	if (::HMIsBalloon())
		::HMRemoveBalloon();

	// Fill in Help manager record
	HMMessageRecord	aHelpMsg;
	FillHMRecord(aHelpMsg, ioParam);

	// Get hot rect so that balloon help automatically removes the balloon if it
	// moves out of the pane
	Rect hotRect;
	((LPane*) ioParam)->CalcPortFrameRect(hotRect);
	((LPane*) ioParam)->PortToGlobalPoint(topLeft(hotRect));
	((LPane*) ioParam)->PortToGlobalPoint(botRight(hotRect));

	// Set tip of balloon to centre of pane
	Point tip = {(hotRect.top + hotRect.bottom)/2,
					(hotRect.left + hotRect.right)/2};

	// Show the balloon
	OSErr err =::HMShowBalloon(&aHelpMsg, tip, &hotRect, nil, 0, 0, kHMRegularWindow);

	// If there was an error set the cached pane to nil to force the balloon
	// to be redrawn the next time AdjustCursor is called. This is neccessary because
	// sometimes the mouse is moving too quick for help manager and balloon fails to draw.
	sHelpPane = (err ? nil : (LPane*) ioParam);
#endif
}

// Fill in the HMMessageRecord
#if !PP_Target_Carbon
void CHelpAttach::FillHMRecord(HMMessageRecord	&theHelpMsg, void *ioParam)
{
#if PP_Target_Classic
	// Fill in Help manager record
	theHelpMsg.hmmHelpType = khmmStringRes;
	theHelpMsg.u.hmmStringRes.hmmResID = mStrId;
	theHelpMsg.u.hmmStringRes.hmmIndex = mIndex;

	mCurrentIndex = theHelpMsg.u.hmmStringRes.hmmIndex;
#endif
}

// Will display same balloon?
bool CHelpAttach::SameBalloon(void *ioParam)
{
	// Save for compare
	short current = mCurrentIndex;

	// Find out what str index will be used now
	HMMessageRecord	aHelpMsg;
	FillHMRecord(aHelpMsg, ioParam);

	// Compare
	return (mCurrentIndex == current);
}
#endif

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C H E L P P A N E A T T A C H
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CHelpPaneAttach::CHelpPaneAttach(short strId, short index_enabled, short index_disabled)
		: CHelpAttach(strId, index_enabled)
{
	// Cache the resource info
	mEnabledIndex = index_enabled;
	mDisabledIndex = index_disabled;
}

CHelpPaneAttach::CHelpPaneAttach(LStream *inStream)
		: CHelpAttach(inStream)
{
	// Cache the resource info
	mEnabledIndex = mIndex;
	*inStream >> mDisabledIndex;
}


// O T H E R  M E T H O D S ____________________________________________________________________________

#if !PP_Target_Carbon
// Fill in the HMMessageRecord
void CHelpPaneAttach::FillHMRecord(HMMessageRecord	&theHelpMsg, void *ioParam)
{
	// Fill in Help manager record
	theHelpMsg.hmmHelpType = khmmStringRes;
	theHelpMsg.u.hmmStringRes.hmmResID = mStrId;

	// Select help string based on pane state
	if (((LPane*) ioParam)->IsEnabled())
		theHelpMsg.u.hmmStringRes.hmmIndex = mEnabledIndex;
	else
		theHelpMsg.u.hmmStringRes.hmmIndex = mDisabledIndex;


	mCurrentIndex = theHelpMsg.u.hmmStringRes.hmmIndex;
}
#endif

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C H E L P P A N E A T T A C H
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CHelpControlAttach::CHelpControlAttach(short strId, short index_enabled_on, short index_enabled_off, short index_disabled)
		: CHelpAttach(strId, index_enabled_on)
{
	// Cache the resource info
	mEnabledOnIndex = index_enabled_on;
	mEnabledOffIndex = index_enabled_off;
	mDisabledIndex = index_disabled;
}

CHelpControlAttach::CHelpControlAttach(LStream *inStream)
		: CHelpAttach(inStream)
{
	// Cache the resource info
	mEnabledOnIndex = mIndex;
	*inStream >> mEnabledOffIndex;
	*inStream >> mDisabledIndex;
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Fill in the HMMessageRecord
#if !PP_Target_Carbon
void CHelpControlAttach::FillHMRecord(HMMessageRecord	&theHelpMsg, void *ioParam)
{
	// Fill in Help manager record
	theHelpMsg.hmmHelpType = khmmStringRes;
	theHelpMsg.u.hmmStringRes.hmmResID = mStrId;

	// Select help string based on pane state
	if (((LControl*) ioParam)->IsEnabled())
	{
		if (((LControl*) ioParam)->GetValue())
			theHelpMsg.u.hmmStringRes.hmmIndex = mEnabledOnIndex;
		else
			theHelpMsg.u.hmmStringRes.hmmIndex = mEnabledOffIndex;
	}
	else
		theHelpMsg.u.hmmStringRes.hmmIndex = mDisabledIndex;


	mCurrentIndex = theHelpMsg.u.hmmStringRes.hmmIndex;
}
#endif