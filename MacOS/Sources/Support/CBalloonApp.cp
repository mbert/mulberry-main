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


// Source for CBalloonApp class


#include "CBalloonApp.h"
#include "CHelpAttach.h"


// ===========================================================================
//		¥ CBalloonApp Class
// ===========================================================================

// ---------------------------------------------------------------------------
//		¥ CBalloonApp
// ---------------------------------------------------------------------------
//	Constructor

CBalloonApp::CBalloonApp()
{
}


// ---------------------------------------------------------------------------
//		¥ ~CBalloonApp
// ---------------------------------------------------------------------------
//	Destructor

CBalloonApp::~CBalloonApp()
{
}

// Handle cursor motion for help balloons
void CBalloonApp::AdjustCursor(const EventRecord& inMacEvent)
{
	bool		useArrow = true;	// Assume cursor will be the Arrow

									// Find out where the mouse is
	WindowPtr	macWindowP;
	Point		globalMouse = inMacEvent.where;
	WindowPartCode	part = ::MacFindWindow(globalMouse, &macWindowP);
	
	mMouseRgn.Clear();				// Start with an empty mouse region
	
	if (macWindowP != nil) {		// Mouse is inside a window
		
		LWindow*	theWindow = LWindow::FetchWindowObject(macWindowP);
		
		if ( (theWindow != nil)  &&			// Mouse is inside an active
			 theWindow->IsActive()  &&		//   and enabled PowerPlant
			 theWindow->IsEnabled() ) {		//   window
			 
			useArrow = false;
			Point	portMouse = globalMouse;
			theWindow->GlobalToPortPoint(portMouse);

			if (part == inContent) {
				theWindow->AdjustContentMouse(portMouse, inMacEvent, mMouseRgn);

			} else {
				theWindow->AdjustStructureMouse(part, inMacEvent, mMouseRgn);
			}

// ----------------
// Start of my bit
// ----------------
			// Find the top pane under the mouse
			LPane* hitPane = theWindow->FindDeepSubPaneContaining(portMouse.h, portMouse.v);

			// Execute the CHelpAttach if the pane exists
			if (hitPane)
				hitPane->ExecuteAttachments(msg_ShowHelp, (void*) hitPane);
// ----------------
// End of my bit
// ----------------

		}
	}
	
	if (mMouseRgn.IsEmpty()) {		// No Pane set the mouse region

		mMouseRgn = ::GetGrayRgn();	// Gray region is desktop minus menu bar
		
									// Add bounds of main device so mouse
									//   region includes the menu bar
		GDHandle	mainGD = ::GetMainDevice();
		mMouseRgn += (**mainGD).gdRect;
		
									// Exclude structure regions of all
									//   active windows
		UWindows::ExcludeActiveStructures(mMouseRgn);
	}
	
	
	if (useArrow) {					// Window didn't set the cursor
		UCursor::SetArrow();		// Default cursor is the arrow
	}
}
