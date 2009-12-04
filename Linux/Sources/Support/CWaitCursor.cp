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

#include "CWaitCursor.h"
#include "CMulberryApp.h"

#include <JXCursor.h>
#include <JXDisplay.h>

unsigned long CWaitCursor::sBusy = 0;

void CWaitCursor::StartBusyCursor()
{
	// Set the busy cursor
	CMulberryApp::sApp->DisplayBusyCursor();

	// Bump reference count
	sBusy++;
}

void CWaitCursor::StopBusyCursor()
{
	// Bump down reference count
	if (!--sBusy)
	{
		// Reset cursor once ref count gets back to zero
		// This now has to be done by hand since busy processing removes
		// events from the event queue that would have changed the cursor
		JPtrArray<JXDisplay>* disp_list = const_cast<JPtrArray<JXDisplay>*>(CMulberryApp::sApp->GetDisplayList());
		const JSize count = disp_list->GetElementCount();
		for (JIndex i=1; i<=count; i++)
			{
			(disp_list->NthElement(i))->DisplayCursorInAllWindows(kJXDefaultCursor);
			}
	}
}
