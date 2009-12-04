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


// Source for CClockEventHandler class

#include "CClockEventHandler.h"

#include "LClock.h"

void CClockEventHandler::InstallMLTEEventHandler(EventTargetRef inTarget)
{
	EventTypeSpec events[] = { { kEventClassKeyboard,	kEventRawKeyDown } };

	mHandler.Install(inTarget, 1, events, this, &CClockEventHandler::HandleEvent);
}

// Special draw
OSStatus CClockEventHandler::HandleEvent(EventHandlerCallRef inCallRef, EventRef inEventRef)
{
	OSStatus result = eventNotHandledErr;

	UInt32 eclass = GetEventClass(inEventRef);
	UInt32 ekind = GetEventKind(inEventRef);

	if ((eclass == kEventClassKeyboard) && (ekind == kEventRawKeyDown))
	{
		// Look for MLTE target
		LClock* clock = dynamic_cast<LClock*>(LCommander::GetTarget());
		if (clock != NULL)
		{
			clock->FocusDraw();

	  		// Do pre-process, then do post-process
			result = ::CallNextEventHandler(inCallRef, inEventRef);
		}
	}

	return result;
}