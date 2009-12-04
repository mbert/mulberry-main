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

//	Undoable Actions associated with a Toolbox TextEdit record
//
//	Reworked for the undo and autoscrolling mechanisms of LTextEditView

#include "CTextWidgetAction.h"

#include "CTextWidget.h"

#include <LCommander.h>
#include <LPane.h>
#include <PP_KeyCodes.h>
#include <PP_Messages.h>
#include <PP_Resources.h>
#include <UMemoryMgr.h>
#include <UScrap.h>

#include <Fonts.h>

// ---------------------------------------------------------------------------
//	¥ CTWStyleTextAction					Constructor				  [public]
// ---------------------------------------------------------------------------

CTWStyleTextAction::CTWStyleTextAction(
	SInt16			inDescriptionIndex,
	CommandT		inActionCommand,
	LCommander*		inTextCommander,
	LPane*			inTextPane,
	bool			inAlreadyDone,
	bool			inWantStyles)

	: CMultiUndoerAction(STRx_RedoEdit, inDescriptionIndex, inAlreadyDone)
{
	mActionCommand	= inActionCommand;
	mTextCommander	= inTextCommander;
	mTextWidget		= dynamic_cast<CTextWidget*>(inTextPane);
	ThrowIfNil_(mTextWidget);

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

	mDeletedStyleH = (inWantStyles && mTextWidget->IsStyled()) ? mTextWidget->GetStyleRangeAs_ustl(mSelStart, mDeletedTextLen) : NULL;
}


// ---------------------------------------------------------------------------
//	¥ ~CTWStyleTextAction					Destructor				  [public]
// ---------------------------------------------------------------------------

CTWStyleTextAction::~CTWStyleTextAction()
{
	if (mDeletedTextH != NULL) {
		::DisposeHandle(mDeletedTextH);
	}

	if (mDeletedStyleH != NULL) {
		::DisposeHandle((Handle)mDeletedStyleH);
	}
}


// ---------------------------------------------------------------------------
//	¥ Redo
// ---------------------------------------------------------------------------

void
CTWStyleTextAction::Redo()
{
	if (CanRedo()) {
		RedoSelf();
		mTextCommander->ProcessCommand(mActionCommand, this);
	}

	mIsDone = true;
}


// ---------------------------------------------------------------------------
//	¥ Undo
// ---------------------------------------------------------------------------

void
CTWStyleTextAction::Undo()
{
	if (CanUndo()) {
		UndoSelf();
		mTextCommander->ProcessCommand(mActionCommand, this);
	}

	mIsDone = false;
}


// ---------------------------------------------------------------------------
//	¥ CanRedo
// ---------------------------------------------------------------------------

Boolean
CTWStyleTextAction::CanRedo() const
{
	return (!IsDone() && mTextCommander->IsOnDuty());
}


// ---------------------------------------------------------------------------
//	¥ CanUndo
// ---------------------------------------------------------------------------

Boolean
CTWStyleTextAction::CanUndo() const
{
	return (IsDone() && mTextCommander->IsOnDuty());
}


// ---------------------------------------------------------------------------
//	¥ IsPostable
// ---------------------------------------------------------------------------

Boolean
CTWStyleTextAction::IsPostable() const
{
		// Action is not postable (undoable) if we could not
		// allocate space to store the text that was deleted

	return (mDeletedTextH != NULL);
}


// ---------------------------------------------------------------------------
//	¥ UndoSelf
// ---------------------------------------------------------------------------

void
CTWStyleTextAction::UndoSelf()
{
	StFocusAndClipIfHidden	focus(mTextWidget);
										// Restore deleted text

	StHandleLocker	lock(mDeletedTextH);
	mTextWidget->InsertTextAndStyles((UniChar*)*mDeletedTextH, mDeletedTextLen, mDeletedStyleH, mSelStart);
	mTextWidget->ModifySelection(mSelStart, mSelEnd);
	mTextWidget->Refresh();
	mTextWidget->UserChangedText();
}


#pragma mark -

// ---------------------------------------------------------------------------
//	¥ CTWCutAction						Constructor				  [public]
// ---------------------------------------------------------------------------

CTWCutAction::CTWCutAction(
	LCommander*		inTextCommander,
	LPane*			inTextPane)

	: CTWStyleTextAction(str_Cut, cmd_ActionCut, inTextCommander, inTextPane)
{
}


// ---------------------------------------------------------------------------
//	¥ RedoSelf
// ---------------------------------------------------------------------------

void
CTWCutAction::RedoSelf()
{
	StFocusAndClipIfHidden	focus(mTextWidget);

	mTextWidget->ModifySelection(mSelStart, mSelEnd);
	mTextWidget->DoCutAction();
}

#pragma mark -

// ---------------------------------------------------------------------------
//	¥ CTWPasteAction					Constructor				  [public]
// ---------------------------------------------------------------------------

CTWPasteAction::CTWPasteAction(
	LCommander*		inTextCommander,
	LPane*			inTextPane)

	: CTWStyleTextAction(str_Paste, cmd_ActionPaste, inTextCommander, inTextPane)
{
										// Get text from clipbboard
	StHandleBlock	textH(Size_Zero);
	UScrap::GetData(kScrapFlavorTypeUnicode, textH);

										// Get style from clipboard
	StHandleBlock	styleH(Size_Zero);
	bool got_style = mTextWidget->IsStyled() && (UScrap::GetData(kScrapFlavorTypeUnicodeStyle, styleH) != 0);

										// Getting info from clipboard
										//   succeeded. Store Handles.
	mPastedTextH  = textH.Release();
	mPastedTextLen = ::GetHandleSize(mPastedTextH) / sizeof(UniChar);

	mPastedStyleH = got_style ? styleH.Release() : NULL;	// If there is no style data, we
															//   must use a NULL Handle rather
															//   than a zero-sized Handle
}


// ---------------------------------------------------------------------------
//	¥ ~CTWPasteAction					Destructor				  [public]
// ---------------------------------------------------------------------------

CTWPasteAction::~CTWPasteAction()
{
	if (mPastedTextH != NULL) {
		::DisposeHandle(mPastedTextH);
	}

	if (mPastedStyleH != NULL) {
		::DisposeHandle(mPastedStyleH);
	}
}


// ---------------------------------------------------------------------------
//	¥ RedoSelf
// ---------------------------------------------------------------------------

void
CTWPasteAction::RedoSelf()
{
	StFocusAndClipIfHidden	focus(mTextWidget);

	// Delete original selection
	if (mSelStart != mSelEnd)
		mTextWidget->DeleteText(mSelStart, mSelEnd - mSelStart);

	// Insert pasted text and select
	StHandleLocker	lock(mPastedTextH);
	mTextWidget->InsertTextAndStyles((UniChar*)*mPastedTextH, mPastedTextLen, mPastedStyleH, mSelStart);
	mTextWidget->ModifySelection(mSelStart, mSelStart + mPastedTextLen);
	mTextWidget->Refresh();
	mTextWidget->UserChangedText();
}


// ---------------------------------------------------------------------------
//	¥ UndoSelf
// ---------------------------------------------------------------------------

void
CTWPasteAction::UndoSelf()
{
	StFocusAndClipIfHidden	focus(mTextWidget);

	// Delete text that was pasted
	if (mPastedTextLen != 0)
		mTextWidget->DeleteText(mSelStart, mPastedTextLen);

	// Restore text deleted by the paste
	StHandleLocker	lock(mDeletedTextH);
	mTextWidget->InsertTextAndStyles((UniChar*)*mDeletedTextH, mDeletedTextLen, mDeletedStyleH, mSelStart);
	mTextWidget->ModifySelection(mSelStart, mSelEnd);
	mTextWidget->Refresh();
	mTextWidget->UserChangedText();
}


#pragma mark -

// ---------------------------------------------------------------------------
//	¥ CTWInsertAction					Constructor				  [public]
// ---------------------------------------------------------------------------

CTWInsertAction::CTWInsertAction(
	LCommander*		inTextCommander,
	LPane*			inTextPane,
	const UniChar*	inInsert,
	UniCharCount	inInsertLen)

	: CTWStyleTextAction(str_Typing, cmd_ActionTyping, inTextCommander, inTextPane, true)
{
										// Get text from clipbboard
	StHandleBlock	textH(inInsertLen * sizeof(UniChar));
	::BlockMove(inInsert, *textH, inInsertLen * sizeof(UniChar));

	mInsertedTextH  = textH.Release();
	mInsertedTextLen = inInsertLen;
}


// ---------------------------------------------------------------------------
//	¥ ~CTWPasteAction					Destructor				  [public]
// ---------------------------------------------------------------------------

CTWInsertAction::~CTWInsertAction()
{
	if (mInsertedTextH != NULL) {
		::DisposeHandle(mInsertedTextH);
	}
}


// ---------------------------------------------------------------------------
//	¥ RedoSelf
// ---------------------------------------------------------------------------

void
CTWInsertAction::RedoSelf()
{
	StFocusAndClipIfHidden	focus(mTextWidget);

	// Delete original selection
	if (mSelStart != mSelEnd)
		mTextWidget->DeleteText(mSelStart, mSelEnd - mSelStart);

	// Insert pasted text and select
	StHandleLocker	lock(mInsertedTextH);
	mTextWidget->InsertText((UniChar*)*mInsertedTextH, mInsertedTextLen, mSelStart);
	mTextWidget->ModifySelection(mSelStart, mSelStart + mInsertedTextLen);
	mTextWidget->Refresh();
	mTextWidget->UserChangedText();
}


// ---------------------------------------------------------------------------
//	¥ UndoSelf
// ---------------------------------------------------------------------------

void
CTWInsertAction::UndoSelf()
{
	StFocusAndClipIfHidden	focus(mTextWidget);

	// Delete text that was pasted
	if (mInsertedTextLen != 0)
		mTextWidget->DeleteText(mSelStart, mInsertedTextLen);

	// Restore text deleted by the paste
	StHandleLocker	lock(mDeletedTextH);
	mTextWidget->InsertTextAndStyles((UniChar*)*mDeletedTextH, mDeletedTextLen, mDeletedStyleH, mSelStart);
	mTextWidget->ModifySelection(mSelStart, mSelEnd);
	mTextWidget->Refresh();
	mTextWidget->UserChangedText();
}


#pragma mark -

// ---------------------------------------------------------------------------
//	¥ CTWClearAction					Constructor				  [public]
// ---------------------------------------------------------------------------

CTWClearAction::CTWClearAction(
	LCommander*		inTextCommander,
	LPane*			inTextPane)

	: CTWStyleTextAction(str_Clear, cmd_ActionClear, inTextCommander, inTextPane)
{
}


// ---------------------------------------------------------------------------
//	¥ RedoSelf
// ---------------------------------------------------------------------------

void
CTWClearAction::RedoSelf()
{
	StFocusAndClipIfHidden	focus(mTextWidget);

	mTextWidget->ModifySelection(mSelStart, mSelEnd);
	mTextWidget->DoClearAction();
}


#pragma mark -

// ---------------------------------------------------------------------------
//	¥ CTWTypingAction					Constructor				  [public]
// ---------------------------------------------------------------------------

CTWTypingAction::CTWTypingAction(
	LCommander*		inTextCommander,
	LPane*			inTextPane)

	: CTWStyleTextAction(str_Typing, cmd_ActionTyping, inTextCommander, inTextPane, true, false)
{
	mTypedTextH  = NULL;
	mTypedStyleH = NULL;
	mTypingStart = mTypingEnd = mSelStart;
}


// ---------------------------------------------------------------------------
//	¥ ~CTWTypingAction					Destructor				  [public]
// ---------------------------------------------------------------------------

CTWTypingAction::~CTWTypingAction()
{
		// Notify TextCommander that Action is being deleted.
		// The TextCommander usually stores a reference to a
		// TypingAction in order to add/remove characters
		// as the user performs a typing sequence.

	mTextCommander->ProcessCommand(cmd_ActionDeleted, this);

	if (mTypedTextH != NULL) {
		::DisposeHandle(mTypedTextH);
	}

	if (mTypedStyleH != NULL) {
		::DisposeHandle(mTypedStyleH);
	}
}

bool CTWTypingAction::NeedsNewAction(UniCharArrayOffset offset) const
{
	return offset != mTypingEnd;
}

// ---------------------------------------------------------------------------
//	¥ Commit
// ---------------------------------------------------------------------------
//	

void
CTWTypingAction::Commit()
{
	mTextCommander->ProcessCommand(cmd_ActionDone, this);
}

// ---------------------------------------------------------------------------
//	¥ Reset
// ---------------------------------------------------------------------------
//	Re-initialize state of TypingAction

void
CTWTypingAction::Reset()
{
	mIsDone = true;

	if (mDeletedTextH != NULL) {			// Clear store of deleted text
		::DisposeHandle(mDeletedTextH);
	}
	if (mDeletedStyleH != NULL) {
		::DisposeHandle((Handle)mDeletedStyleH);
	}

	if (mTypedTextH != NULL) {			// New typing sequence is starting
		::DisposeHandle(mTypedTextH);	//   so delete saved text
		mTypedTextH = NULL;
	}
	if (mTypedStyleH != NULL) {
		::DisposeHandle(mTypedStyleH);
		mTypedStyleH = NULL;
	}

	mSelStart = mTextWidget->mSelection.caret;	// Save current selection state
	mSelEnd = mTextWidget->mSelection.caret + mTextWidget->mSelection.length;
	mDeletedTextLen = mSelEnd - mSelStart;

	mTypingStart = mTypingEnd = mSelStart;

	LCommander::SetUpdateCommandStatus(true);

										// Save currently selected text,
										//   which will be deleted when
										//   the next character is typed
	mDeletedTextH = ::NewHandle(mDeletedTextLen * sizeof(UniChar));
	ThrowIfMemFail_(mDeletedTextH);

	::BlockMoveData(mTextWidget->mTextPtr + mSelStart, *mDeletedTextH, mDeletedTextLen * sizeof(UniChar));

	mDeletedStyleH = NULL;
}


// ---------------------------------------------------------------------------
//	¥ InputCharacter
// ---------------------------------------------------------------------------
//	Handle an input character typing action
//
//	This function does not actually put any character into the text. It
//	just saves state for undoing the typing. You should call TEKey()
//	afterwards.

void
CTWTypingAction::InputCharacter(UniCharCount count)
{
	if ( (mTypingEnd != mTextWidget->mSelection.caret) ||
		 (mTypingEnd != mTextWidget->mSelection.caret + mTextWidget->mSelection.length) ||
		 (mTypingEnd == mTypingStart) ||
		 !IsDone() ) {
									// Selection has changed. Start a
		Reset();					//   fresh typing sequence
	}

	mTypingEnd += count;
}


// ---------------------------------------------------------------------------
//	¥ BackwardErase
// ---------------------------------------------------------------------------
//	Handle Backward Delete typing action
//
//	Backward delete erases the current selection if one or more characters
//	is selected. If the selection is a single insertion point, then
//	backward delete erases the one character before the insertion point.
//
//	This function does not actually erase any text. It just saves state
//	for undoing the action. You should call TEKey() with a backspace
//	character afterwards.

void
CTWTypingAction::BackwardErase()
{
	if ( (mTypingEnd != mTextWidget->mSelection.caret) ||
		 (mTypingEnd != mTextWidget->mSelection.caret + mTextWidget->mSelection.length) ) {
									// Selection has changed. Start a
		Reset();					//   fresh typing sequence

		if (mDeletedTextLen == 0) {
								// No selected text, save the one character
								//   that will be deleted
			::SetHandleSize(mDeletedTextH, sizeof(UniChar));
			mDeletedTextLen = 1;
			mTypingStart -= 1;
			*(UniChar*)*mDeletedTextH = *(mTextWidget->mTextPtr + mTypingStart);

		} else {				// Selection being deleted. Increment end
			mTypingEnd += 1;	//   to counteract decrement done on the
								//   last line of this function.
		}

	} else if (mTypingStart >= mTextWidget->mSelection.caret) {
									// Deleting before beginning of typing
		::SetHandleSize(mDeletedTextH, (mDeletedTextLen + 1) * sizeof(UniChar));
		ThrowIfMemError_();

		::BlockMoveData(*mDeletedTextH, ((UniChar*)*mDeletedTextH) + 1, mDeletedTextLen * sizeof(UniChar));
		mDeletedTextLen += 1;

		mTypingStart = mTextWidget->mSelection.caret - 1;
		*(UniChar*)*mDeletedTextH = *(mTextWidget->mTextPtr + mTypingStart);
	}

	mTypingEnd -= 1;
}


// ---------------------------------------------------------------------------
//	¥ ForwardErase
// ---------------------------------------------------------------------------
//	Handle Forward Delete typing action
//
//	Forward delete erases the current selection if one or more characters
//	is selected. If the selection is a single insertion point, then
//	forward delete erases the one character after the insertion point.
//
//	This function does not actually erase any text. It just saves state
//	for undoing the action. Afterwards, you should select the next
//	character if the selection is empty and call TEDelete().

void
CTWTypingAction::ForwardErase()
{
	if ( (mTypingEnd != mTextWidget->mSelection.caret) ||
		 (mTypingEnd != mTextWidget->mSelection.caret + mTextWidget->mSelection.length) ) {
									// Selection has changed. Start a
		Reset();					//   fresh typing sequence

		if (mSelStart == mSelEnd) {
									// Selection is a single insertion point
									// Select next character
			::SetHandleSize(mDeletedTextH, sizeof(UniChar));
			ThrowIfMemError_();

			*(UniChar*)*mDeletedTextH = *(mTextWidget->mTextPtr + mSelStart);
			mDeletedTextLen = 1;
		}

	} else {						// Selection hasn't changed
									// Select next character
		::SetHandleSize(mDeletedTextH, (mDeletedTextLen + 1) * sizeof(UniChar));
		ThrowIfMemError_();

		*(((UniChar*)*mDeletedTextH) + mDeletedTextLen) = *(mTextWidget->mTextPtr + mTypingEnd);
		mDeletedTextLen += 1;
	}
}


// ---------------------------------------------------------------------------
//	¥ RedoSelf
// ---------------------------------------------------------------------------
//	Redo a TypingAction by restoring the last typing sequence

void
CTWTypingAction::RedoSelf()
{
	StFocusAndClipIfHidden	focus(mTextWidget);

	// Delete original selection
	if (mDeletedTextLen != 0)
		mTextWidget->DeleteText(mTypingStart, mDeletedTextLen);

	// Insert typing run
	StHandleLocker	lock(mTypedTextH);
	mTextWidget->InsertText((UniChar*)*mTypedTextH, mTypingEnd - mTypingStart, mTypingStart);
	mTextWidget->ModifySelection(mTypingStart, mTypingEnd);
	mTextWidget->Refresh();
	mTextWidget->UserChangedText();
}


// ---------------------------------------------------------------------------
//	¥ UndoSelf
// ---------------------------------------------------------------------------
//	Undo a TypingAction by restoring the text and selection that
//	existed before the current typing sequence started

void
CTWTypingAction::UndoSelf()
{
	StFocusAndClipIfHidden	focus(mTextWidget);

	// Save current typing run
	if (mTypedTextH == NULL) {
		mTypedTextH = ::NewHandle((mTypingEnd - mTypingStart) * sizeof(UniChar));
		ThrowIfMemFail_(mTypedTextH);
	} else {
		::SetHandleSize(mTypedTextH, (mTypingEnd - mTypingStart) * sizeof(UniChar));
		ThrowIfMemError_();
	}
	::BlockMoveData(mTextWidget->mTextPtr + mTypingStart, *mTypedTextH, (mTypingEnd - mTypingStart) * sizeof(UniChar));

	// Delete current typing run
	if (mTypedStyleH != NULL) {
		::DisposeHandle(mTypedStyleH);
	}

	mTypedStyleH = NULL;

	if (mTypingStart != mTypingEnd)
		mTextWidget->DeleteText(mTypingStart, mTypingEnd - mTypingStart);

	// Restore original text
	StHandleLocker	lock(mDeletedTextH);
	mTextWidget->InsertText((UniChar*)*mDeletedTextH, mDeletedTextLen, mTypingStart);
	mTextWidget->ModifySelection(mSelStart, mSelEnd);
	mTextWidget->Refresh();
	mTextWidget->UserChangedText();
}


#pragma mark -

// ---------------------------------------------------------------------------
//	¥ CTWStyleAction					Constructor				  [public]
// ---------------------------------------------------------------------------
// 	A "base" class for the style actions (font, size, style/face, align/just,
//	color) to encapsulate some common functionality.

CTWStyleAction::CTWStyleAction(
	SInt16			inDescriptionIndex,
	LCommander*		inCommander,
	LPane*			inPane,
	bool			inAlreadyDone)

	: CMultiUndoerAction(STRx_RedoStyle, inDescriptionIndex, inAlreadyDone)
{
	mCommander	= inCommander;
	mTextWidget		= dynamic_cast<CTextWidget*>(inPane);
	ThrowIfNil_(mTextWidget);

	mSelStart		= mTextWidget->mSelection.caret;
	mSelEnd			= mTextWidget->mSelection.caret + mTextWidget->mSelection.length;
}


// ---------------------------------------------------------------------------
//	¥ CTWStyleAction					Destructor				  [public]
// ---------------------------------------------------------------------------

CTWStyleAction::~CTWStyleAction()
{
}


// ---------------------------------------------------------------------------
//	¥ CanRedo
// ---------------------------------------------------------------------------

Boolean
CTWStyleAction::CanRedo() const
{
	// Call inherited and ensure the commander is on duty.
	return CMultiUndoerAction::CanRedo() && mCommander->IsOnDuty();
}


// ---------------------------------------------------------------------------
//	¥ CanUndo
// ---------------------------------------------------------------------------

Boolean
CTWStyleAction::CanUndo() const
{
	// Call inherited and ensure the commander is on duty.
	return CMultiUndoerAction::CanUndo() && mCommander->IsOnDuty();
}


#pragma mark -

// ---------------------------------------------------------------------------
//	¥ CTWFontAction						Constructor				  [public]
// ---------------------------------------------------------------------------

CTWFontAction::CTWFontAction(
	SInt16			inFont,
	LCommander*		inCommander,
	LPane*			inPane,
	bool			inAlreadyDone)

	: CTWStyleAction(str_Font, inCommander, inPane, inAlreadyDone)
{
	mFont = inFont;

	//mTextWidget->GetFont(mSavedFont);			// Save the current font
}


// ---------------------------------------------------------------------------
//	¥ CTWFontAction						Constructor				  [public]
// ---------------------------------------------------------------------------

CTWFontAction::CTWFontAction(
	Str255			inFontName,
	LCommander*		inCommander,
	LPane*			inPane,
	bool			inAlreadyDone )

	: CTWStyleAction(str_Font, inCommander, inPane, inAlreadyDone)
{
	//::GetFNum(inFontName, &mFont);			// Get font number from name

	//mTextWidget->GetFont(mSavedFont);			// Save the current font
}


// ---------------------------------------------------------------------------
//	¥ ~CTWFontAction					Destructor				  [public]
// ---------------------------------------------------------------------------

CTWFontAction::~CTWFontAction()
{
}


// ---------------------------------------------------------------------------
//	¥ UndoSelf
// ---------------------------------------------------------------------------

void
CTWFontAction::UndoSelf()
{
#if 0
	Rect oldRect = (*(mTextWidget->GetMacTEH()))->destRect;

	TextStyle theStyle;
	theStyle.tsFont = mSavedFont;

	::TESetSelect( mSelStart, mSelEnd, mTextWidget->GetMacTEH() );
	::TESetStyle( doFont, &theStyle, false, mTextWidget->GetMacTEH() );

	mTextWidget->AlignTextEditRects();
	mTextWidget->AdjustImageToText();
	mTextWidget->ForceAutoScroll(oldRect);
	mTextWidget->Refresh();
#endif
}

// ---------------------------------------------------------------------------
//	¥ RedoSelf
// ---------------------------------------------------------------------------

void
CTWFontAction::RedoSelf()
{
#if 0
	Rect oldRect = (*(mTextWidget->GetMacTEH()))->destRect;

	TextStyle theStyle;
	theStyle.tsFont = mFont;

	::TESetSelect(mSelStart, mSelEnd, mTextWidget->GetMacTEH());
	::TESetStyle( doFont, &theStyle, false, mTextWidget->GetMacTEH() );

	mTextWidget->AlignTextEditRects();
	mTextWidget->AdjustImageToText();
	mTextWidget->ForceAutoScroll(oldRect);
	mTextWidget->Refresh();
#endif
}

#pragma mark-

// ---------------------------------------------------------------------------
//	¥ CTWSizeAction						Constructor				  [public]
// ---------------------------------------------------------------------------

CTWSizeAction::CTWSizeAction(
	SInt16			inSize,
	LCommander*		inCommander,
	LPane*			inPane,
	bool			inAlreadyDone)

	: CTWStyleAction(str_Size, inCommander, inPane, inAlreadyDone)
{
#if 0
	mSize = inSize;

	mTextWidget->GetSize( mSavedSize );
#endif
}


// ---------------------------------------------------------------------------
//	¥ ~CTWSizeAction					Destructor				  [public]
// ---------------------------------------------------------------------------

CTWSizeAction::~CTWSizeAction()
{
}


// ---------------------------------------------------------------------------
//	¥ UndoSelf
// ---------------------------------------------------------------------------

void
CTWSizeAction::UndoSelf()
{
#if 0
	Rect oldRect = (*(mTextWidget->GetMacTEH()))->destRect;

	TextStyle theStyle;
	theStyle.tsSize = mSavedSize;

	::TESetSelect(mSelStart, mSelEnd, mTextWidget->GetMacTEH());
	::TESetStyle( doSize, &theStyle, false, mTextWidget->GetMacTEH() );

	mTextWidget->AlignTextEditRects();
	mTextWidget->AdjustImageToText();
	mTextWidget->ForceAutoScroll(oldRect);
	mTextWidget->Refresh();
#endif
}


// ---------------------------------------------------------------------------
//	¥ RedoSelf
// ---------------------------------------------------------------------------

void
CTWSizeAction::RedoSelf()
{
#if 0
	Rect oldRect = (*(mTextWidget->GetMacTEH()))->destRect;

	TextStyle theStyle;
	theStyle.tsSize = mSize;

	::TESetSelect(mSelStart, mSelEnd, mTextWidget->GetMacTEH());
	::TESetStyle( doSize, &theStyle, false, mTextWidget->GetMacTEH() );

	mTextWidget->AlignTextEditRects();
	mTextWidget->AdjustImageToText();
	mTextWidget->ForceAutoScroll(oldRect);
	mTextWidget->Refresh();
#endif
}


#pragma mark-

// ---------------------------------------------------------------------------
//	¥ CTWFaceAction						Constructor				  [public]
// ---------------------------------------------------------------------------

CTWFaceAction::CTWFaceAction(
	Style			inFace,
	bool			inToggle,
	LCommander*		inCommander,
	LPane*			inPane,
	bool			inAlreadyDone)

	: CTWStyleAction(str_Face, inCommander, inPane, inAlreadyDone)
{
#if 0
	mFace	= inFace;
	mToggle = inToggle;

	mTextWidget->GetStyle(mSavedFace);
#endif
}


// ---------------------------------------------------------------------------
//	¥ ~CTWFaceAction					Destructor				  [public]
// ---------------------------------------------------------------------------

CTWFaceAction::~CTWFaceAction()
{
}


// ---------------------------------------------------------------------------
//	¥ UndoSelf
// ---------------------------------------------------------------------------

void
CTWFaceAction::UndoSelf()
{
#if 0
	Rect oldRect = (*(mTextWidget->GetMacTEH()))->destRect;

	SInt16 theMode = doFace;
	TextStyle theStyle;

	if (mToggle) {
		theStyle.tsFace = mFace;
		theMode			+= doToggle;
	} else {
		theStyle.tsFace = mSavedFace;
	}

	::TESetSelect(mSelStart, mSelEnd, mTextWidget->GetMacTEH());
	::TESetStyle(theMode, &theStyle, false, mTextWidget->GetMacTEH());

	mTextWidget->AlignTextEditRects();
	mTextWidget->AdjustImageToText();
	mTextWidget->ForceAutoScroll(oldRect);
	mTextWidget->Refresh();
#endif
}


// ---------------------------------------------------------------------------
//	¥ RedoSelf
// ---------------------------------------------------------------------------

void
CTWFaceAction::RedoSelf()
{
#if 0
	// set to mFace, if toggle, toggle, else SetStyle

	Rect oldRect = (*(mTextWidget->GetMacTEH()))->destRect;

	TextStyle theStyle;
	theStyle.tsFace = mFace;

	SInt16 theMode = doFace;

	if (mToggle) {
		theMode += doToggle;
	}

	::TESetSelect(mSelStart, mSelEnd, mTextWidget->GetMacTEH());
	::TESetStyle( theMode, &theStyle, false, mTextWidget->GetMacTEH() );

	mTextWidget->AlignTextEditRects();
	mTextWidget->AdjustImageToText();
	mTextWidget->ForceAutoScroll(oldRect);
	mTextWidget->Refresh();
#endif
}

#pragma mark -

// ---------------------------------------------------------------------------
//	¥ CTWAlignAction					Constructor				  [public]
// ---------------------------------------------------------------------------

CTWAlignAction::CTWAlignAction(
	SInt16			inAlign,
	LCommander*		inCommander,
	LPane*			inPane,
	bool			inAlreadyDone)

	: CTWStyleAction(str_Alignment, inCommander, inPane, inAlreadyDone)
{
#if 0
	mAlign		= inAlign;
	mSavedAlign = mTextWidget->GetAlignment();
#endif
}


// ---------------------------------------------------------------------------
//	¥ ~CTWAlignAction					Destructor				  [public]
// ---------------------------------------------------------------------------

CTWAlignAction::~CTWAlignAction()
{
}


// ---------------------------------------------------------------------------
//	¥ UndoSelf
// ---------------------------------------------------------------------------

void
CTWAlignAction::UndoSelf()
{
#if 0
	Rect oldRect = (*(mTextWidget->GetMacTEH()))->destRect;

	::TESetAlignment( mSavedAlign, mTextWidget->GetMacTEH() );

	mTextWidget->AlignTextEditRects();
	mTextWidget->AdjustImageToText();
	mTextWidget->ForceAutoScroll(oldRect);
	mTextWidget->Refresh();
#endif
}

// ---------------------------------------------------------------------------
//	¥ RedoSelf
// ---------------------------------------------------------------------------

void
CTWAlignAction::RedoSelf()
{
#if 0
	Rect oldRect = (*(mTextWidget->GetMacTEH()))->destRect;

	::TESetAlignment( mAlign, mTextWidget->GetMacTEH() );

	mTextWidget->AlignTextEditRects();
	mTextWidget->AdjustImageToText();
	mTextWidget->ForceAutoScroll(oldRect);
	mTextWidget->Refresh();
#endif
}

#pragma mark -

// ---------------------------------------------------------------------------
//	¥ CTWColorAction					Constructor				  [public]
// ---------------------------------------------------------------------------

CTWColorAction::CTWColorAction(
	const RGBColor&		inColor,
	LCommander*			inCommander,
	LPane*				inPane,
	bool				inAlreadyDone)

	: CTWStyleAction(str_Color, inCommander, inPane, inAlreadyDone)
{
#if 0
	mColor = inColor;
	mTextWidget->GetColor(mSavedColor);
#endif
}


// ---------------------------------------------------------------------------
//	¥ ~CTWColorAction					Destructor				  [public]
// ---------------------------------------------------------------------------

CTWColorAction::~CTWColorAction()
{
}


// ---------------------------------------------------------------------------
//	¥ UndoSelf
// ---------------------------------------------------------------------------

void
CTWColorAction::UndoSelf()
{
#if 0
	TextStyle theStyle;
	theStyle.tsColor = mSavedColor;

	::TESetSelect(mSelStart, mSelEnd, mTextWidget->GetMacTEH());
	::TESetStyle(doColor, &theStyle, false, mTextWidget->GetMacTEH());

	mTextWidget->Refresh();
#endif
}

// ---------------------------------------------------------------------------
//	¥ RedoSelf
// ---------------------------------------------------------------------------

void
CTWColorAction::RedoSelf()
{
#if 0
	TextStyle theStyle;
	theStyle.tsColor = mColor;

	::TESetSelect(mSelStart, mSelEnd, mTextWidget->GetMacTEH());
	::TESetStyle(doColor, &theStyle, false, mTextWidget->GetMacTEH());

	mTextWidget->Refresh();
#endif
}
