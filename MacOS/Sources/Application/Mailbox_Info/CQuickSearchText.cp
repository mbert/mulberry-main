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


// Source for CQuickSearchText class

#include "CQuickSearchText.h"

// Constructor from stream
CQuickSearchText::CQuickSearchText(LStream *inStream)
		: CTextFieldX(inStream)
{
}

// Default destructor
CQuickSearchText::~CQuickSearchText()
{
}

// Handle non-editable case
Boolean CQuickSearchText::HandleKeyPress(const EventRecord& inKeyEvent)
{
	UInt16		theKey = inKeyEvent.message & charCodeMask;

	// Broadcast if required
	switch (theKey)
	{
	case char_Return:
	case char_Enter:
		BroadcastMessage(mValueMessage + 1, this);
		SelectAll();
		break;
	case char_Tab:
		BroadcastMessage(mValueMessage + 2, this);
		SelectAll();
		break;
	default:
		if (!UKeyFilters::IsActionKey(inKeyEvent.message))
		{
			BroadcastMessage(mValueMessage, this);
		}
		break;
	}

	if ((theKey == '\r') || (theKey == '\t'))
	{
		return true;
	}
	else
		return CTextFieldX::HandleKeyPress(inKeyEvent);
}
