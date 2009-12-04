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


// Source for CTextWidgetEventHandler class

#include "CTextWidgetEventHandler.h"

#include "CScrollWheelAttachment.h"

#include "CTextWidget.h"

void CTextWidgetEventHandler::InstallEventHandler(EventTargetRef inTarget)
{
	EventTypeSpec events[] = { { kEventClassTextInput,	kEventTextInputUnicodeForKeyEvent },
							   { kEventClassTextInput,	kEventTextInputOffsetToPos },
							   { kEventClassTextInput,	kEventTextInputPosToOffset },
							   { kEventClassTextInput,	kEventTextInputGetSelectedText } };

	mHandler.Install(inTarget, 4, events, this, &CTextWidgetEventHandler::HandleEvent);
}

// Special draw
OSStatus CTextWidgetEventHandler::HandleEvent(EventHandlerCallRef inCallRef, EventRef inEventRef)
{
	OSStatus result = eventNotHandledErr;

	UInt32 eclass = GetEventClass(inEventRef);
	UInt32 ekind = GetEventKind(inEventRef);

	if (eclass == kEventClassTextInput)
	{
		// Look for Text Widget target
		CTextWidget* tw = dynamic_cast<CTextWidget*>(LCommander::GetTarget());
		if (tw == this)
		{
	  		// Do process
			result = tw->ProcessKeyEvent(inCallRef, inEventRef);
			if (result == eventNotHandledErr)
			{
				result = ::CallNextEventHandler(inCallRef, inEventRef);
			}
		}
	}

	return result;
}