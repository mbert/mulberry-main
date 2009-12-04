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

#ifndef _H_CTextWidgetAction
#define _H_CTextWidgetAction
#pragma once

#include "CMultiUndoer.h"

class LCommander;
class LPane;
class CTextWidget;

// ---------------------------------------------------------------------------

class	CTWStyleTextAction : public CMultiUndoerAction {
public:
						CTWStyleTextAction(
								SInt16		inDescriptionIndex,
								MessageT	inActionCommand,
								LCommander*	inTextCommander,
								LPane*		inTextPane,
								bool		inAlreadyDone = false,
								bool		inWantStyles = true);

	virtual				~CTWStyleTextAction();

	virtual void		Redo();
	virtual void		Undo();

	virtual Boolean		CanRedo() const;
	virtual Boolean		CanUndo() const;

	virtual	Boolean		IsPostable() const;

protected:
	LCommander*			mTextCommander;
	CTextWidget*		mTextWidget;
	MessageT			mActionCommand;

	Handle				mDeletedTextH;
	UniCharCount		mDeletedTextLen;
	UniCharArrayOffset	mSelStart;
	UniCharArrayOffset	mSelEnd;
	Handle				mDeletedStyleH;

	virtual void		UndoSelf();
};

// ---------------------------------------------------------------------------

class	CTWCutAction : public CTWStyleTextAction {
public:
						CTWCutAction(
								LCommander*	inTextCommander,
								LPane*		inTextPane);

	virtual				~CTWCutAction() { }

protected:
	virtual void		RedoSelf();
};

// ---------------------------------------------------------------------------

class	CTWPasteAction : public CTWStyleTextAction {
public:
						CTWPasteAction(
								LCommander*		inTextCommander,
								LPane*			inTextPane);

	virtual				~CTWPasteAction();

protected:
	Handle				mPastedTextH;
	UniCharCount		mPastedTextLen;
	Handle				mPastedStyleH;

	virtual void		RedoSelf();
	virtual void		UndoSelf();
};


// ---------------------------------------------------------------------------

class	CTWInsertAction : public CTWStyleTextAction {
public:
						CTWInsertAction(
								LCommander*		inTextCommander,
								LPane*			inTextPane,
								const UniChar*	inInsert,
								UniCharCount	inInsertLen);

	virtual				~CTWInsertAction();

protected:
	Handle				mInsertedTextH;
	UniCharCount		mInsertedTextLen;

	virtual void		RedoSelf();
	virtual void		UndoSelf();
};


// ---------------------------------------------------------------------------

class	CTWClearAction : public CTWStyleTextAction {
public:
						CTWClearAction(
								LCommander*	inTextCommander,
								LPane*		inTextPane);

	virtual				~CTWClearAction() { }

protected:
	virtual void		RedoSelf();
};

// ---------------------------------------------------------------------------

class	CTWTypingAction : public CTWStyleTextAction {
public:
						CTWTypingAction(
								LCommander*	inTextCommander,
								LPane*		inTextPane);

	virtual				~CTWTypingAction();

	virtual void		Commit();
	virtual void		Reset();
	virtual void		InputCharacter(UniCharCount count = 1);
	virtual void		BackwardErase();
	virtual void		ForwardErase();
	
			bool		NeedsNewAction(UniCharArrayOffset offset) const;

protected:
	Handle				mTypedTextH;
	Handle				mTypedStyleH;
	UniCharArrayOffset	mTypingStart;
	UniCharArrayOffset	mTypingEnd;

	virtual void		RedoSelf();
	virtual void		UndoSelf();
};


// ---------------------------------------------------------------------------

class CTWStyleAction : public CMultiUndoerAction {
public:
						CTWStyleAction(
								SInt16		inDescriptionIndex,
								LCommander*	inCommander,
								LPane*		inPane,
								bool		inAlreadyDone = false);

	virtual				~CTWStyleAction();

	virtual Boolean		CanRedo() const;
	virtual Boolean		CanUndo() const;

protected:
	LCommander*			mCommander;
	CTextWidget*		mTextWidget;

	UniCharArrayOffset	mSelStart;
	UniCharArrayOffset	mSelEnd;
};

// ---------------------------------------------------------------------------

class CTWFontAction : public CTWStyleAction {
public:
						CTWFontAction(
								SInt16		inFont,
								LCommander*	inCommander,
								LPane*		inPane,
								bool		inAlreadyDone = false);

						CTWFontAction(
								Str255		inFontName,
								LCommander*	inCommander,
								LPane*		inPane,
								bool		inAlreadyDone = false);

	virtual				~CTWFontAction();

protected:
	SInt16				mFont;
	SInt16				mSavedFont;

	virtual	void		RedoSelf();
	virtual void		UndoSelf();
};

// ---------------------------------------------------------------------------

class CTWSizeAction : public CTWStyleAction {
public:
						CTWSizeAction(
								SInt16		inSize,
								LCommander*	inCommander,
								LPane*		inPane,
								bool		inAlreadyDone = false);

	virtual				~CTWSizeAction();

protected:
	SInt16				mSize;
	SInt16				mSavedSize;

	virtual	void		RedoSelf();
	virtual void		UndoSelf();
};

// ---------------------------------------------------------------------------

class CTWFaceAction : public CTWStyleAction {
public:
						CTWFaceAction(
								Style		inFace,
								bool		inToggle,
								LCommander*	inCommander,
								LPane*		inPane,
								bool		inAlreadyDone = false);

	virtual				~CTWFaceAction();

protected:
	Style				mFace;
	Style				mSavedFace;
	bool				mToggle;

	virtual	void		RedoSelf();
	virtual void		UndoSelf();
};

// ---------------------------------------------------------------------------

class CTWAlignAction : public CTWStyleAction {
public:
						CTWAlignAction(
								SInt16		inAlign,
								LCommander*	inCommander,
								LPane*		inPane,
								bool		inAlreadyDone = false);

	virtual				~CTWAlignAction();

protected:
	SInt16				mAlign;
	SInt16				mSavedAlign;

	virtual	void		RedoSelf();
	virtual	void		UndoSelf();
};

// ---------------------------------------------------------------------------

class CTWColorAction : public CTWStyleAction {
public:
						CTWColorAction(
								const RGBColor&	inColor,
								LCommander*		inCommander,
								LPane*			inPane,
								bool			inAlreadyDone = false);

	virtual				~CTWColorAction();

protected:
	RGBColor			mColor;
	RGBColor			mSavedColor;

	virtual	void		RedoSelf();
	virtual	void		UndoSelf();
};

#endif

