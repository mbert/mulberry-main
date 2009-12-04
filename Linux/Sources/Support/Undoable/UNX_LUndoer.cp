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


//	Attachment for implementing Undo
//
//	An LUndoer object can be attached to a LCommander. The Undoer will
//	store the last Action object posted to the Commander, enable and
//	set the text for the "undo" menu item, and respond to the "undo"
//	command by telling the Action object to Undo/Redo itself.


#include "UNX_LUndoer.h"

#include "CLog.h"
#include "CXStringResources.h"

#include "UNX_LAction.h"

#include <JXTextMenu.h>

// ---------------------------------------------------------------------------
//		 LUndoer
// ---------------------------------------------------------------------------
//	Default Constructor

LUndoer::LUndoer()
{
	mAction = nil;
}


// ---------------------------------------------------------------------------
//		 ~LUndoer
// ---------------------------------------------------------------------------
//	Destructor

LUndoer::~LUndoer()
{
	try
	{
	PostAction(nil);					// Will delete current Action
}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Fail silently
	}
}


// ---------------------------------------------------------------------------
//		 PostAction
// ---------------------------------------------------------------------------
//	A new Action has been posted to the host Commander

void
LUndoer::PostAction(LAction	*inAction)
{
	ExceptionCode	actionFailure = noErr;
	
	if ((inAction == nil)  ||
		((inAction != nil) && inAction->IsPostable())) {

										// Save old Action
		LAction	*oldAction = mAction;
		mAction = inAction;
		
		bool	deleteOldAction = false;

		// Finalize the old action
		if (oldAction != nil)
		{
			try
			{		
				oldAction->Finalize();
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

			}
			
			deleteOldAction = true;
		}

		// Do the new action
		if (mAction != nil)
		{
			try
			{
				mAction->Redo();
			}
			
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Failed to "Do" the newly posted Action. Finalize
				// and delete the new Action, then check to see if the
				// old Action can still be undone/redone. If so, keep
				// the old Action as the last undoable Action.

				try
				{
					mAction->Finalize();
				}
				catch (...)
				{
					CLOG_LOGCATCH(...);

				}
				
				delete mAction;
				mAction = nil;
				
				if ((oldAction != nil) &&
					 (oldAction->CanUndo() || oldAction->CanRedo()) )
				{	 
					mAction = oldAction;
					deleteOldAction = false;
				}
			}
		}
		
		if (deleteOldAction)
		{
			delete oldAction;
		}
		
	}
	else
	{							// A non-postable Action
		try
		{
			inAction->Redo();
		}
		
		catch (...)
		{
			CLOG_LOGCATCH(...);

		}
		
		try
		{
			inAction->Finalize();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

		}
			
		delete inAction;
	}

	
}


// ---------------------------------------------------------------------------
//		 ToggleAction
// ---------------------------------------------------------------------------
//	Undo/Redo the Action associated with this Undoer

void LUndoer::ToggleAction(void)
{
	if (mAction != nil)
	{			// Shouldn't be nil, but let's be safe
		if (mAction->CanUndo())
			mAction->Undo();
		else if (mAction->CanRedo())
			mAction->Redo();
	}
}


// ---------------------------------------------------------------------------
//		 FindUndoStatus
// ---------------------------------------------------------------------------
//	Enable/disable and set the text for the "undo" menu item

void LUndoer::FindUndoStatus(JXTextMenu* menu, JIndex item)
{
	cdstring txt;
	bool enabled = false;

	// Check for action
	if (mAction != NULL)
	{
		// Get menu string from action and enable menu item
		cdstring dummy;
		if (mAction->CanRedo())
		{
			enabled = true;
			menu->EnableItem(item);
			mAction->GetDescription(txt, dummy);
		}
		else if (mAction->CanUndo())
		{
			enabled = true;
			menu->EnableItem(item);
			mAction->GetDescription(dummy, txt);
		}
	}

	if (!enabled)
	{
		// Use default menu string and disable menu item
		menu->DisableItem(item);
		txt.FromResource("UI::General::Undo");
	}

	// Check for accelarator key and add to item
	menu->SetItemText(item, txt.c_str());
}
