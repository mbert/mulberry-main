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

#include "CDayEvent.h"

// ---------------------------------------------------------------------------
//	CDayEvent														  [public]
/**
	Default constructor */

CDayEvent::CDayEvent(JXContainer* enclosure,
						const HSizingOption hSizing, const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h) :
	CCalendarEventBase(enclosure, hSizing, vSizing, x, y, w, h)
{
	mColumnOffset = 0;
	mColumnTotal = 1;
	mSpanToEdge = false;
}


CDayEvent* CDayEvent::Create(JXContainer* enclosure, const JRect& frame)
{
	CDayEvent* result = new CDayEvent(enclosure, kFixedLeft, kFixedTop, frame.left, frame.top, frame.width(), frame.height());
	try
	{
		result->OnCreate();
	}
	catch (...)
	{									// FinishCreate failed. View is
		delete result;					//   in an inconsistent state.
		throw;							//   Delete it and rethrow.
	}

	return result;
}

