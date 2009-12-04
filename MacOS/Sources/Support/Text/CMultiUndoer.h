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

#ifndef _H_CMultiUndoer
#define _H_CMultiUndoer
#pragma once

#include <LUndoer.h>

#include <vector>

typedef std::vector<LAction*> CActionStack;

// ---------------------------------------------------------------------------

class	CMultiUndoer : public LUndoer {
public:
	enum { class_ID = FOUR_CHAR_CODE('Mund') };

						CMultiUndoer();

						CMultiUndoer( LStream* inStream );

	virtual				~CMultiUndoer();

			void		ClearActions(bool redo_only = false);

protected:
	CActionStack		mUndo;
	CActionStack		mRedo;

	virtual void		ExecuteSelf(
								MessageT		inMessage,
								void* 			ioParam);

	virtual void		PostAction( LAction* inAction );

	virtual void		UndoAction();
	virtual void		RedoAction();

	virtual void		FindUndoStatus( SCommandStatus* ioStatus,
										Boolean redo );

private:					// Unimplemented copy and assignment
						CMultiUndoer( const CMultiUndoer& );
	CMultiUndoer&			operator = ( const CMultiUndoer& );
	
};

class	CMultiUndoerAction : public LAction {
public:
		CMultiUndoerAction(
				ResIDT		inStringResID = STRx_RedoEdit,
				SInt16		inStringIndex = str_RedoUndo,
				bool		inAlreadyDone = false) :
			LAction(inStringResID, inStringIndex, inAlreadyDone) {}

		CMultiUndoerAction( const LAction&	inOriginal ) :
			LAction(inOriginal) {}

	virtual void		Commit() {}

};

#endif
