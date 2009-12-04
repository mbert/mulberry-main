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

CDayEvent::CDayEvent(const SPaneInfo &inPaneInfo) :
	CCalendarEventBase(inPaneInfo)
{
	mColumnOffset = 0;
	mColumnTotal = 1;
	mSpanToEdge = false;
}


CDayEvent* CDayEvent::Create(LView* parent, const HIRect& frame)
{
	SPaneInfo pane;
	pane.visible = true;
	pane.enabled = true;
	pane.userCon = 0L;
	pane.superView = parent;
	pane.bindings.left =
	pane.bindings.top =
	pane.bindings.right =
	pane.bindings.bottom = false;
	pane.paneID = 0;
	pane.width = frame.size.width;
	pane.height = frame.size.height;
	pane.left = frame.origin.x;
	pane.top = frame.origin.y;
	CDayEvent* result = new CDayEvent(pane);
	try
	{
		result->FinishCreate();
	}
	catch (...)
	{									// FinishCreate failed. View is
		delete result;					//   in an inconsistent state.
		throw;							//   Delete it and rethrow.
	}

	return result;
}

