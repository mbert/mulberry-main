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
//	An CMultiUndoer object can be attached to a LCommander. The Undoer will
//	store the last Action object posted to the Commander, enable and
//	set the text for the "undo" menu item, and respond to the "undo"
//	command by telling the Action object to Undo/Redo itself.

#include "CMultiUndoer.h"

#include <LAction.h>
#include <PP_Messages.h>
#include <PP_Resources.h>
#include <Resources.h>

// ---------------------------------------------------------------------------
//	¥ CMultiUndoer								Default Constructor		  [public]
// ---------------------------------------------------------------------------

CMultiUndoer::CMultiUndoer()
{
}


// ---------------------------------------------------------------------------
//	¥ CMultiUndoer								Stream Constructor		  [public]
// ---------------------------------------------------------------------------

CMultiUndoer::CMultiUndoer(
	LStream*		inStream)

	: LUndoer(inStream)
{
	mMessage = msg_AnyMessage;
}


// ---------------------------------------------------------------------------
//	¥ ~CMultiUndoer								Destructor				  [public]
// ---------------------------------------------------------------------------

CMultiUndoer::~CMultiUndoer()
{
	// Clear out all actions
	ClearActions();
}

void CMultiUndoer::ClearActions(bool redo_only)
{
	if (!redo_only)
	{
		for(CActionStack::iterator iter = mUndo.begin(); iter != mUndo.end(); iter++)
		{
			(*iter)->Finalize();
			delete *iter;
		}
		mUndo.clear();
	}
	for(CActionStack::iterator iter = mRedo.begin(); iter != mRedo.end(); iter++)
	{
		(*iter)->Finalize();
		delete *iter;
	}
	mRedo.clear();
}

// ---------------------------------------------------------------------------
//	¥ ExecuteSelf												   [protected]
// ---------------------------------------------------------------------------
//	Execute an Undoer Attachment

void
CMultiUndoer::ExecuteSelf(
	MessageT	inMessage,
	void*		ioParam)
{
	switch (inMessage) {

		case msg_PostAction:		// New Action to perform
			PostAction(static_cast<LAction*> (ioParam));
			SetExecuteHost(false);
			break;

		case msg_CommandStatus: {	// Enable and set text for "undo"
									//   menu item
			SCommandStatus *status = static_cast<SCommandStatus*> (ioParam);

			if (status->command == cmd_Undo) {
				FindUndoStatus(status, false);	// CD: Modified to support separate Undo/Redo
				SetExecuteHost(false);

			// CD: Added to support separate Undo/Redo
			} else if (status->command == cmd_Redo) {
				FindUndoStatus(status, true);
				SetExecuteHost(false);
			} else {
				SetExecuteHost(true);
			}
			break;
		}

		case cmd_Undo:				// Undo/Redo the Action
			UndoAction();
			SetExecuteHost(false);
			break;

		case cmd_Redo:				// Undo/Redo the Action
			RedoAction();
			SetExecuteHost(false);
			break;

		default:
			SetExecuteHost(true);
			break;
	}
}


// ---------------------------------------------------------------------------
//	¥ PostAction												   [protected]
// ---------------------------------------------------------------------------
//	A new Action has been posted to the host Commander

void
CMultiUndoer::PostAction(
	LAction*	inAction)
{
	ExceptionCode	actionFailure = noErr;

	if ((inAction == nil)  ||
		((inAction != nil) && inAction->IsPostable())) {

										// Save old Action
		LAction	*oldAction = (mUndo.size() != 0) ? mUndo.back() : NULL;
		LAction	*newAction = inAction;

		if (newAction != nil) {			// Do the new action
			try {
				newAction->Redo();
			}

			catch (const LException& inErr) {

				// Failed to "Do" the newly posted Action. Finalize
				// and delete the new Action, then check to see if the
				// old Action can still be undone/redone. If so, keep
				// the old Action as the last undoable Action.

				actionFailure = inErr.GetErrorCode();

				try {
					newAction->Finalize();
				}
				catch (...) { }

				delete newAction;
				newAction = nil;
			}
		}

		if (newAction != nil)
		{
			// Make sure old action gets committed
			if ((mUndo.size() != 0) && dynamic_cast<CMultiUndoerAction*>(mUndo.back()))
			{
				dynamic_cast<CMultiUndoerAction*>(mUndo.back())->Commit();
			}
			// Now add to undo stack
			mUndo.push_back(newAction);
			
			// Clear out the existing redo stack
			ClearActions(true);
		}

	} else {							// A non-postable Action
		try {
			inAction->Redo();
		}

		catch (ExceptionCode inErr) {
			actionFailure = inErr;
		}

		catch (const LException& inException) {
			actionFailure = inException.GetErrorCode();
		}

		try {
			inAction->Finalize();
		}

		catch (...) { }

		delete inAction;
	}

	if (actionFailure != noErr) {
		Throw_(actionFailure);
	}
}


// ---------------------------------------------------------------------------
//	¥ UndoAction												   [protected]
// ---------------------------------------------------------------------------
//	Undo the Action associated with this Undoer

void
CMultiUndoer::UndoAction()
{
	if (mUndo.size() != 0)
	{
		if (mUndo.back()->CanUndo())
		{
			// Make sure current action gets committed before being undone
			if ((mUndo.size() != 0) && dynamic_cast<CMultiUndoerAction*>(mUndo.back()))
			{
				dynamic_cast<CMultiUndoerAction*>(mUndo.back())->Commit();
			}

			// Pull off the back of the undo stack
			LAction* action = mUndo.back();
			mUndo.pop_back();
			
			// Undo it
			action->Undo();
			
			// Add to back of redo stack
			mRedo.push_back(action);
		}
	}
}


// ---------------------------------------------------------------------------
//	¥ RedoAction												   [protected]
// ---------------------------------------------------------------------------
//	Redo the Action associated with this Undoer

void
CMultiUndoer::RedoAction()
{
	if (mRedo.size() != 0)
	{
		if (mRedo.back()->CanRedo())
		{
			// Pull off the back of the redo stack
			LAction* action = mRedo.back();
			mRedo.pop_back();
			
			// Undo it
			action->Redo();
			
			// Add to back of undo stack
			mUndo.push_back(action);
		}
	}
}


// ---------------------------------------------------------------------------
//	¥ FindUndoStatus											   [protected]
// ---------------------------------------------------------------------------
//	Enable/disable and set the text for the "undo" menu item

// CD: Modified to support separate Undo/Redo
void
CMultiUndoer::FindUndoStatus(
	SCommandStatus*		ioStatus,
	Boolean redo)
{
	*ioStatus->enabled = false;

	if (!redo)
	{
		if (mUndo.size() != 0)
		{
			LAction* action = mUndo.back();

			Str255	dummyString;

			if (action->CanUndo()) {	// Set "Undo" text
				*ioStatus->enabled = true;
				action->GetDescription(dummyString, ioStatus->name);
			}
		}
	}
	else 
	{
		if (mRedo.size() != 0)
		{
			LAction* action = mRedo.back();

			Str255	dummyString;

			if (action->CanRedo()) {	// Set "Redo" text
				*ioStatus->enabled = true;
				action->GetDescription(ioStatus->name, dummyString);
			}
		}
	}

	if (!(*ioStatus->enabled)) {			// Set text to "Can't Undo"
		::GetIndString(ioStatus->name, redo ? STRx_RedoEdit : STRx_UndoEdit, str_CantRedoUndo);
	}
}
