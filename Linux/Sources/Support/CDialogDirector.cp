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


// Source for CDialogDirector class

#include "CDialogDirector.h"

#include "CResizeNotifier.h"

#include <JXWindow.h>
#include <jXGlobals.h>
#include <jAssert.h>

CDialogDirector::CDialogDirector(JXDirector* supervisor, const JBoolean modal)
	: JXDialogDirector(supervisor, modal)
{
	mClosed = kDialogContinue;
	mPendingResize = false;
}

void CDialogDirector::Receive(JBroadcaster*	sender, const Message&	message)
{
	if (message.Is(CResizeNotifier::kResized))
	{
		const CResizeNotifier::Resized* info =
			dynamic_cast<const CResizeNotifier::Resized*>(&message);
		BoundsResized(info->dw(), info->dh());
	}
	
	JXDialogDirector::Receive(sender, message);
}

void CDialogDirector::AdjustSize(const JSize w, const JSize h)
{
	mPendingResize = true;
	GetWindow()->ClearMinSize();
	GetWindow()->ClearMaxSize();
	GetWindow()->AdjustSize(w, h);
}

void CDialogDirector::BoundsResized(const JCoordinate dw, const JCoordinate dh)
{
	// Check for pending update on expand/collapse
	if (mPendingResize)
	{
		mPendingResize = false;

		// Lock window size
		GetWindow()->LockCurrentSize();
	}
}

int CDialogDirector::DoModal(bool do_create)
{
	// Create it and start dialog director
	if (do_create)
		OnCreate();
	BeginDialog();

	JXApplication* app = JXGetApplication();
	app->DisplayInactiveCursor();
	JXWindow* window = GetWindow();
	while (true)
	{
		// Handle an event
		app->HandleOneEventForWindow(window);

		// Allow extra event loop processing
		Continue();

		// Test for OK
		switch(mClosed)
		{
		case kDialogClosed_OK:
		case kDialogClosed_Btn3:
		case kDialogClosed_Btn4:
			// Just return - caller has to close the dialog
			return mClosed;

		case kDialogClosed_Cancel:
			// Close the dialog and return
			Close();
			return kDialogClosed_Cancel;

		default:;
		}
	}

	// Must have been cancelled some other way
	return kDialogClosed_Cancel;
}

void CDialogDirector::EndDialog(int btn)
{
	mClosed = btn;
	switch(mClosed)
	{
	case kDialogClosed_OK:
	case kDialogClosed_Btn3:
	case kDialogClosed_Btn4:
		// Close the dialog with success
		JXDialogDirector::EndDialog(kTrue);
		return;

	case kDialogClosed_Cancel:
		// Close the dialog with failure
		JXDialogDirector::EndDialog(kFalse);
		return;

	default:;
	}
}

// Same as super-class except that window is not locked if pending resize
void CDialogDirector::Activate()
{
	if (!IsActive())
		{
		assert( !itsModalFlag || itsOKButton != NULL );

		JXWindow* window = GetWindow();
		assert( window != NULL );
		window->SetCloseAction(JXWindow::kDeactivateDirector);
		window->ShouldFocusWhenShow(kTrue);

		JXDirector* supervisor = GetSupervisor();
		if (supervisor->IsWindowDirector())
			{
			JXWindowDirector* windowDir =
				dynamic_cast(JXWindowDirector*, supervisor);
			assert( windowDir != NULL );
			window->SetTransientFor(windowDir);
			}

		if (itsAutoGeomFlag)
			{
			window->PlaceAsDialogWindow();
			if (!mPendingResize)
				window->LockCurrentSize();
			}

		JXWindowDirector::Activate();
		if (IsActive())
			{
			itsCancelFlag = kTrue;		// so WM_DELETE_WINDOW => cancel
			if (itsModalFlag)
				{
				supervisor->Suspend();
				while (IsSuspended())
					{
					Resume();			// we need to be active
					}
				}
			}
		}
	else
		{
		JXWindowDirector::Activate();
		}
}

JBoolean CDialogDirector::Deactivate()
{
	if (!IsActive())
		{
		return kTrue;
		}

	if (Cancelled())
		{
		(GetWindow())->KillFocus();
		}

	if (JXWindowDirector::Deactivate())
		{
		const JBoolean success = JNegate(Cancelled());
		Broadcast(JXDialogDirector::Deactivated(success));
		(GetSupervisor())->Resume();
		if (mClosed == kDialogContinue)
			mClosed = success ? kDialogClosed_OK : kDialogClosed_Cancel;

		return kTrue;
		}
	else
		{
		return kFalse;
		}
}

void CDialogDirector::Continue()
{
	// This does nothing here  - subclasses can override
}
