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


// Source for CTextWidgetDragAction class

#include "CTextWidgetDragAction.h"

#include "CTextWidget.h"

//#include "CStringUtils.h"

// __________________________________________________________________________________________________
// C L A S S __ C T E D R A G A C T I O N
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTextWidgetDragAction::CTextWidgetDragAction(
	CTextWidget*	inTextWidget,
	LCommander*		inTextCommander,
	Handle			inDroppedTextH,
	UniCharCount	inDroppedTextLen,
	Handle			inDroppedStyleH,
	UniCharArrayOffset	inInsertPos,
	bool			delete_original,
	bool			list_item)
		: LAction(STRx_RedoDragTextActions, str_DragInsert)
{
	mTextWidget = inTextWidget;
	mTextCommander = inTextCommander;
	mActionCommand = cmd_ActionPaste;

		// Save current selection range and the selected text,
		// which will be deleted when doing the Action.

	mSelStart		= mTextWidget->mSelection.caret;
	mSelEnd			= mTextWidget->mSelection.caret + mTextWidget->mSelection.length;
	mDeletedTextLen	= mSelEnd - mSelStart;
	mDeletedTextH	= ::NewHandle(mDeletedTextLen * sizeof(UniChar));

		// If we can't allocate the memory to save the selected
		// text, we don't throw. We still want to "do" the Action,
		// but it won't be undoable. mDeletedTextH being NULL will
		// mark this Action as not postable (undoable). When
		// PostAction() is called, this Action will be "done" and
		// then deleted.

	if (mDeletedTextH != NULL) {
		::BlockMoveData(mTextWidget->mTextPtr + mSelStart, *mDeletedTextH, mDeletedTextLen * sizeof(UniChar));
	}

	mDeletedStyleH = mTextWidget->IsStyled() ? mTextWidget->GetStyleRangeAs_ustl(mSelStart, mDeletedTextLen) : NULL;

	mDroppedTextH = inDroppedTextH;
	mDroppedTextLen = inDroppedTextLen;
	mDroppedStyleH = inDroppedStyleH;
	mInsertPos = inInsertPos;
	mDeleteOriginal = delete_original;
	mListItem = list_item;

	// Adjust insert pos if list item
	if (mListItem && mInsertPos)
	{
		// Check for CR just before insert pos
		if (mTextWidget->mTextPtr[mInsertPos - 1] == '\r')
			// Step back
			mInsertPos--;
		else
		{
			// Step over text until char is CR is just after insert or end reached
			while(mInsertPos < mTextWidget->mTextLength)
			{
				if (mTextWidget->mTextPtr[mInsertPos] == '\r')
					break;
				else
					mInsertPos++;
			}
		}

		// Check for CR at end of insert text and remove
		if (((UniChar*)*mDroppedTextH)[mDroppedTextLen - 1] == '\r')
			mDroppedTextLen--;
	}
	
	// Adjust insert pos for delete of original
	if (delete_original && (mInsertPos > mSelStart))
		mInsertPos -= mDeletedTextLen;
}

// Default destructor
CTextWidgetDragAction::~CTextWidgetDragAction()
{
	if (mDeletedTextH != NULL) {
		::DisposeHandle(mDeletedTextH);
	}

	if (mDeletedStyleH != NULL) {
		::DisposeHandle((Handle)mDeletedStyleH);
	}

	if (mDroppedTextH != NULL) {
		::DisposeHandle(mDroppedTextH);
	}

	if (mDroppedStyleH != NULL) {
		::DisposeHandle((Handle)mDroppedStyleH);
	}
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Redo with command
void CTextWidgetDragAction::Redo()
{
	if (CanRedo()) {
		RedoSelf();
		mTextCommander->ProcessCommand(mActionCommand, this);
	}

	mIsDone = true;
}

// Undo with command
void CTextWidgetDragAction::Undo()
{
	if (CanUndo()) {
		UndoSelf();
		mTextCommander->ProcessCommand(mActionCommand, this);
	}

	mIsDone = false;
}

// RedoSelf
void CTextWidgetDragAction::RedoSelf()
{
	StFocusAndClipIfHidden	focus(mTextWidget);

	// Delete original selection if required
	if (mDeleteOriginal)
		mTextWidget->DeleteText(mSelStart, mSelEnd - mSelStart);

	// Check for list based insert
	if (mListItem)
	{
		UniChar uc = '\r';

		// Check for insert at start of text => CR after
		if ((mInsertPos == 0) || (mTextWidget->mTextPtr[mInsertPos - 1] == '\r'))
		{
			// Insert pasted text with trailing \r and select
			StHandleLocker	lock(mDroppedTextH);
			mTextWidget->InsertTextAndStyles((UniChar*)*mDroppedTextH, mDroppedTextLen, mDroppedStyleH, mInsertPos);
			mTextWidget->InsertText(&uc, 1, mInsertPos + mDroppedTextLen);
			mTextWidget->ModifySelection(mInsertPos, mInsertPos + mDroppedTextLen + 1);
		}
		else
		{
			// Insert pasted text with trailing \r and select
			StHandleLocker	lock(mDroppedTextH);
			mTextWidget->InsertText(&uc, 1, mInsertPos);
			mTextWidget->InsertTextAndStyles((UniChar*)*mDroppedTextH, mDroppedTextLen, mDroppedStyleH, mInsertPos + 1);
			mTextWidget->ModifySelection(mInsertPos, mInsertPos + mDroppedTextLen + 1);
		}
		mActualTextLen = mDroppedTextLen + 1;
	}

	else
	{
		// Insert pasted text and select
		StHandleLocker	lock(mDroppedTextH);
		mTextWidget->InsertTextAndStyles((UniChar*)*mDroppedTextH, mDroppedTextLen, mDroppedStyleH, mInsertPos);
		mTextWidget->ModifySelection(mInsertPos, mInsertPos + mDroppedTextLen);
		mActualTextLen = mDroppedTextLen;
	}
}

// UndoSelf
void CTextWidgetDragAction::UndoSelf()
{
	StFocusAndClipIfHidden	focus(mTextWidget);

	// Delete text that was pasted
	mTextWidget->DeleteText(mInsertPos, mActualTextLen);

	// Restore text deleted by the paste
	if (mDeleteOriginal)
	{
		StHandleLocker	lock(mDeletedTextH);
		mTextWidget->InsertTextAndStyles((UniChar*)*mDeletedTextH, mDeletedTextLen, mDeletedStyleH, mSelStart);
		mTextWidget->ModifySelection(mSelStart, mSelEnd);
	}
}
