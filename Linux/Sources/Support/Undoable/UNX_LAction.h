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
//
//	LAction is used with LUndoer and LCommander to implement undo.
//	You attach an LUndoer object to a Commander (typically an Application
//	or Document). Then create an object of a subclass of LAction whenever
//	the user does something that you want to be undoable, and call
//	PostAction for that Commander.
//
//	Actions are always performed (via Redo) when they're posted.
//	Thereafter, they can be undone (Undo) and redone (Redo) as appropriate.
//	
//	If an action has already been done at the time of posting, it is
//	the responsibility of the poster to appropriately set inAlreadyDone
//	to true when posting the Action.
//	
//	Finalize is called immediately before an Action is replaced
//	with a new undoable Action.

#ifndef __UNX_LACTION__MULBERRY__
#define __UNX_LACTION__MULBERRY__

#include "cdstring.h"

class	LAction {
public:
						LAction(
								int			inStringIndex = 0,
								bool		inAlreadyDone = false);
						LAction(
								const char*	inStringR,
								const char*	inStringU,
								bool		inAlreadyDone = false);
								
	virtual				~LAction();

	virtual void		Finalize();
	
	virtual void		Redo();			//	Also functions as "Do"
	virtual void		Undo();

	bool				IsDone() const			{ return mIsDone; }
	virtual bool		IsPostable() const;
	virtual bool		CanRedo() const;
	virtual bool		CanUndo() const;
	
	virtual void		GetDescription(
								cdstring&		outRedoString,
								cdstring&		outUndoString) const;

protected:
	int					mStringIndex;		//	Index in STR# for redo item
	cdstring			mStringR;
	cdstring			mStringU;
	bool				mIsDone;			//	Is Action done or redone?
	
						//	Pure Virtual functions. Subclasses must override!
	virtual void		RedoSelf() = 0;
	virtual void		UndoSelf() = 0;
};

#endif
