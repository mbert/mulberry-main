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


// Source for CBalloonDialog class


#include "CBalloonDialog.h"
#include "CHelpAttach.h"
#include "CMulberryApp.h"

#include <LThread.h>
#include <UEventMgr.h>

// ===========================================================================
//		¥ CBalloonDialog Class
// ===========================================================================

// ---------------------------------------------------------------------------
//		¥ CBalloonDialog
// ---------------------------------------------------------------------------
//	Constructor

CBalloonDialog::CBalloonDialog(ResIDT inDialogResID, LCommander	*inSuper, bool blocking)
	: StDialogHandler(inDialogResID, inSuper)
{
	mCritical = false;
	mBlocking = blocking;

	// Force application's periodics off
	CMulberryApp::sApp->ErrorPause(true);

	// If previous dispatcher is our type then make sure it knows
	// we're the next one so that if dispatchers get deleted out of
	// sequence we can maintain a consistent dispatcher chain
	mSaveNextDispatcher = NULL;
	CBalloonDialog* dlog = dynamic_cast<CBalloonDialog*>(mSaveDispatcher);
	if (dlog)
		dlog->mSaveNextDispatcher = dlog;
	
	// Set sleep time to zero when blocking
	if (blocking)
		mSleepTime = 0;
}


// ---------------------------------------------------------------------------
//		¥ ~CBalloonDialog
// ---------------------------------------------------------------------------
//	Destructor

CBalloonDialog::~CBalloonDialog()
{
	// Make sure dispatcher chain remains consistent when we delete this one
	if (mSaveNextDispatcher)
		static_cast<CBalloonDialog*>(mSaveNextDispatcher)->mSaveDispatcher = mSaveDispatcher;

	// Force application's periodics on
	CMulberryApp::sApp->ErrorPause(false);
}

// Starting dialog
void CBalloonDialog::StartDialog(void)
{
	// Make sure window is visible
	if (!mDialog->IsVisible())
		mDialog->Show();
}

// Ending dialog
void CBalloonDialog::EndDialog(void)
{
	// Nothing to do here at the moment ... maybe some day?
}

// Ending dialog
MessageT CBalloonDialog::DoModal(void)
{
	// Run modal loop waiting for OK or Cancel message
	StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = DoDialog();

		if ((hitMessage == msg_OK) || (hitMessage == msg_Cancel))
			return hitMessage;
	}
}

// Copy of parent code but turns off periodicals if required
MessageT CBalloonDialog::DoDialog()
{
	EventRecord macEvent;

	if (IsOnDuty()) {
		UEventMgr::GetMouseAndModifiers(macEvent);
		AdjustCursor(macEvent);
	}

	SetUpdateCommandStatus(false);
	mMessage = msg_Nothing;

	Boolean gotEvent = ::WaitNextEvent(everyEvent, &macEvent,
										mSleepTime, mMouseRgn);

		// Let Attachments process the event. Continue with normal
		// event dispatching unless suppressed by an Attachment.

	if (LEventDispatcher::ExecuteAttachments(msg_Event, &macEvent)) {
		if (gotEvent) {
			DispatchEvent(macEvent);
		} else if (!mBlocking) {
			UseIdleTime(macEvent);
		}
	}

									// Repeaters get time after every event
	if (!mBlocking)
		LPeriodical::DevoteTimeToRepeaters(macEvent);

									// Update status of menu items
	if (IsOnDuty() && GetUpdateCommandStatus()) {
		UpdateMenus();
	}

	return mMessage;
}

Boolean CBalloonDialog::HandleKeyPress(const EventRecord &inKeyEvent)
{
	// Swallow all key presses to prevent them being passed up to super-commander
	return true;
}

// Handle cursor motion for help balloons
void CBalloonDialog::AdjustCursor(const EventRecord& inMacEvent)
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

// ---------------------------------------------------------------------------
//		¥ CBalloonDialog
// ---------------------------------------------------------------------------
//	Constructor

CCriticalBalloonDialog::CCriticalBalloonDialog(ResIDT inDialogResID, LCommander	*inSuper)
	: CBalloonDialog(inDialogResID, inSuper)
{
}


// ---------------------------------------------------------------------------
//		¥ ~CBalloonDialog
// ---------------------------------------------------------------------------
//	Destructor

CCriticalBalloonDialog::~CCriticalBalloonDialog()
{
}

// Handle case of critical operations
void CCriticalBalloonDialog::DispatchEvent(const EventRecord	&inMacEvent)
{
	switch (inMacEvent.what)
	{
		case mouseDown:
			AdjustCursor(inMacEvent);
			EventMouseDown(inMacEvent);
			break;

		case mouseUp:
			EventMouseUp(inMacEvent);
			break;

		case keyDown:
			EventKeyDown(inMacEvent);
			break;

		case autoKey:
			EventAutoKey(inMacEvent);
			break;

		case keyUp:
			EventKeyUp(inMacEvent);
			break;

		case diskEvt:
			if (!mCritical)
				EventDisk(inMacEvent);
			break;

		case updateEvt:
			if (mCritical)
				mDialog->UpdatePort();
			else
				EventUpdate(inMacEvent);
			break;

		case activateEvt:
			if (mCritical)
				mDialog->Activate();
			else
				EventActivate(inMacEvent);
			break;

		case osEvt:
			if (!mCritical)
				EventOS(inMacEvent);
			break;

		case kHighLevelEvent:
			if (!mCritical)
				EventHighLevel(inMacEvent);
			break;

		default:
			if (!mCritical)
				UseIdleTime(inMacEvent);
			break;
	}
}

