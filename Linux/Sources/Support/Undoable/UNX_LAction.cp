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


//	Abstract class for an undoable action.

#include "UNX_LAction.h"

#include "CXStringResources.h"

// ---------------------------------------------------------------------------
//		 LAction(ResIDT, Int16, bool)
// ---------------------------------------------------------------------------
//	Constructor
//
//	inStringResID is the ID of the STR# resource for the "Redo" description
//
//	inStringIndex is the index in the STR# of the "Redo" description for
//		this Action
//
//	inAlreadyDone specifies whether the Action has already been done
//		and does not need to be redone when posted
//
//	By convention, the "Undo" description is at the same index in the
//		STR# resource with ID = inStringResID + 1

LAction::LAction(
	int			inStringIndex,
	bool		inAlreadyDone)
{
	mStringIndex = inStringIndex;
	mIsDone = inAlreadyDone;
}

LAction::LAction(
	const char*	inStringR,
	const char*	inStringU,
	bool		inAlreadyDone)
{
	mStringIndex = 0;
	mStringR = inStringR;
	mStringU = inStringU;
	mIsDone = inAlreadyDone;
}


// ---------------------------------------------------------------------------
//		 ~LAction
// ---------------------------------------------------------------------------
//	Destructor

LAction::~LAction()
{
}


// ---------------------------------------------------------------------------
//		 Finalize
// ---------------------------------------------------------------------------
//	Perform clean up for the Action
//
//	This function gets called when the Action is no longer "current",
//	meaning that another Action has been posted to the Undoer owning
//	this Action. For single "undo" implementations, the Action will
//	be deleted afterwards. 

void
LAction::Finalize()
{
}


// ---------------------------------------------------------------------------
//		 Redo
// ---------------------------------------------------------------------------
//	Wrapper function which calls RedoSelf() if the Action can be redone
//
//	Note that the first time this function is called, "Redo" really
//	means "Do" the Action (unless isAlreadyDone is set to true when
//	creating the Action).

void
LAction::Redo()
{
	if (CanRedo()) {
		RedoSelf();
	}
	
	mIsDone = true;
}


// ---------------------------------------------------------------------------
//		 Undo
// ---------------------------------------------------------------------------
//	Wrapper function which calls UndoSelf() if the Action can be undone

void
LAction::Undo()
{
	if (CanUndo()) {
		UndoSelf();
	}
		
	mIsDone = false;
}


// ---------------------------------------------------------------------------
//		 IsPostable
// ---------------------------------------------------------------------------
//	Return whether an Action is postable, meaning that it affects the
//	"undo" state.
//
//	This implementation always returns true. Override to return false for
//	actions that are not undoable.

bool
LAction::IsPostable() const
{
	return true;
}


// ---------------------------------------------------------------------------
//		 CanRedo
// ---------------------------------------------------------------------------
//	Return whether the Action can be redone

bool
LAction::CanRedo() const
{
	return !IsDone();
}


// ---------------------------------------------------------------------------
//		 CanUndo
// ---------------------------------------------------------------------------
//	Return whether the Action can be undone

bool
LAction::CanUndo() const
{
	return IsDone();
}


// ---------------------------------------------------------------------------
//		 GetDescription
// ---------------------------------------------------------------------------
//	Pass back the Redo and Undo strings for the Action

void
LAction::GetDescription(
	cdstring&	outRedoString,
	cdstring&	outUndoString) const
{
	if (mStringIndex == 0)
	{
		if (!mStringR.empty())
			outRedoString = mStringR;
		else
			outRedoString = rsrc::GetString("UI::General::Redo");
		if (!mStringU.empty())
			outUndoString = mStringU;
		else
			outUndoString = rsrc::GetString("UI::General::Undo");
	}
	else
	{
		outRedoString.FromResource(mStringIndex);
		outUndoString.FromResource(mStringIndex + 1);
	}
}
