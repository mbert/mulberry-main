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

#include "CTextWidget.h"

#include "CATSUIStyle.h"
#include "CBetterScrollerX.h"
#include "CLog.h"
#include "CMultiUndoer.h"
#include "CTextWidgetAction.h"

#include "CGUtils.h"

#include "MoreATSUnicode.h"

#include <LCFString.h>
#include <LStream.h>
#include <UTextTraits.h>

#include <algorithm>
#include <memory>

EventLoopTimerUPP _IdleTimerUPP = NULL;

// ---------------------------------------------------------------------------
//	$$$ Testing
//	Option for whether or not to cache the line breaks and heights
//
//		Depending on the text (number of chars, fonts used, styles, etc.),
//		calculating line breaks and heights might be slow

// ---------------------------------------------------------------------------
//	¥ CTextWidget							Default Constructor		  [public]
// ---------------------------------------------------------------------------

CTextWidget::CTextWidget() :
	CTextWidgetDragAndDrop(UQDGlobals::GetCurrentWindowPort(), this)
{
	InitTextWidget();
}


// ---------------------------------------------------------------------------
//	¥ CTextWidget							Parameterized Constructor [public]
// ---------------------------------------------------------------------------

CTextWidget::CTextWidget(
	const SPaneInfo&	inPaneInfo,
	const SViewInfo&	inViewInfo)
	
	: LView(inPaneInfo, inViewInfo),
		CTextWidgetDragAndDrop(UQDGlobals::GetCurrentWindowPort(), this)
{
	InitTextWidget();
}


// ---------------------------------------------------------------------------
//	¥ CTextWidget							Stream Constructor		  [public]
// ---------------------------------------------------------------------------
//	$$$ For now, we use the same PPob data as LCaption. So you can create
//	a CTextWidget in Constructor by maing a LCaption and changing the
//	the Class ID to 'utbx'.

CTextWidget::CTextWidget(
	LStream*	inStream)
	
	: LView(inStream),
		CTextWidgetDragAndDrop(UQDGlobals::GetCurrentWindowPort(), this)
{
	InitTextWidget();

	Boolean readonly;
	Boolean noselection;
	Boolean wrap;
	Boolean multistyle;
	*inStream >> readonly;
	*inStream >> noselection;
	*inStream >> wrap;
	*inStream >> multistyle;

	mReadOnly = readonly;
	mSelectable = !noselection;
	mWordWrap = wrap;
	mAllowStyles = multistyle;
	
	if (!mWordWrap)
	{
		SDimension32 imageSize;
		GetImageSize(imageSize);
		ResizeImageTo(1000, imageSize.height, false);
	}
	else
	{
		SDimension16 frameSize;
		GetFrameSize(frameSize);
		ResizeImageTo(frameSize.width, frameSize.height, false);
	}
}

void CTextWidget::InitTextWidget()
{
	Deactivate();

	mPortOrigin = Point_00;
	mCurrentContext = NULL;

	mTextPtr	= NULL;
	mHiddenPtr  = NULL;
	mTextLength	= 0;
	mTextCapacity = 0;

	// Need some dummy values
	mLineCount = 0;

	mMargins = Rect_0000;

	mPasswordMode = false;
	mReadOnly = true;
	mSelectable = true;
	mWordWrap = true;
	mAllowStyles = false;
	mRedrawOn = true;
	mAdjustPending = false;
	
	mPendingDrawCaret = false;

	mPendingStyle = NULL;

	mUndoer = NULL;
	mTypingAction = NULL;
	
	mTSMDocument = 0;
	
	// Create TSM document
	OSType	typeList[1];
	typeList[0] = kUnicodeDocumentInterfaceType;
	OSErr err = ::NewTSMDocument(1, typeList, &mTSMDocument, (long) this);
	if (err != noErr)
		mTSMDocument = 0;
	else
	{
		::UseInputWindow(mTSMDocument, true);
	}

	SetDefaultBackground(Color_White);

	// Margins for better display inside of CBetterScrollerX
	if (dynamic_cast<CBetterScrollerX*>(GetSuperView()))
	{
		const Rect cDefaultMargins = { 2, 3, 2, 2 };  /* t,l,b,r */
	    SetMargins(cDefaultMargins);
	}
    
	// Set smart anti-aliased highlighting
	ATSURGBAlphaColor white = { 1.0F, 1.0F, 1.0F, 1.0F };
	ATSUUnhighlightData uhd;
	uhd.dataType = kATSUBackgroundColor;
	uhd.unhighlightData.backgroundColor = white;
	::ATSUSetHighlightingMethod(mTextLayout, kRedrawHighlighting, &uhd);

	// Make sure line end selection extends to full width
	ATSLineLayoutOptions options = kATSLineFillOutToWidth;
	mTextLayout.SetOneLayoutControl(kATSULineLayoutOptionsTag, sizeof(ATSLineLayoutOptions), &options);

	// Set System Font
	SetFontNum(::GetSysFont());
	SetFontSize(IntToFixed(::GetDefFontSize()));

	//	create timer UPP first time through
	if (_IdleTimerUPP == NULL)
	{
		_IdleTimerUPP = NewEventLoopTimerUPP(IdleTimer);
	}

	//	install an event loop timer for blinking the caret
	InstallEventLoopTimer(GetMainEventLoop(), 0.0, TicksToEventTime(GetCaretTime()), _IdleTimerUPP, this, &mSelection.timer);
}


// ---------------------------------------------------------------------------
//	¥ ~CTextWidget						Destructor				  [public]
// ---------------------------------------------------------------------------

CTextWidget::~CTextWidget()
{
	// remove our timer, if any
	if (mSelection.timer != NULL)
	{
		RemoveEventLoopTimer(mSelection.timer);
		mSelection.timer = NULL;
	}

	delete[] mTextPtr;
	delete[] mHiddenPtr;
	
	delete mPendingStyle;
	
	// Remove TSM document
	if (mTSMDocument != 0)
		::DeleteTSMDocument(mTSMDocument);
}


// ---------------------------------------------------------------------------
//	¥ FinishCreateSelf											   [protected]
// ---------------------------------------------------------------------------

void
CTextWidget::FinishCreateSelf()
{
									// Use CoreGraphics if present
	CGContextRef	context = GetCGContext();

	if (context != NULL) {
		mTextLayout.SetCGContext(context);
	}
	
	// Always add multi-undoer attachment
	AddAttachment(mUndoer = new CMultiUndoer);

	// Set Drag & Drop TE to info
	SetTextEdit(this);

	// Set appropriate Drag & Drop inset
	Rect ddInset = {0, 0, 0, 0};
	SetHiliteInset(ddInset);

	// Find parent window in chain
	LView* parent = GetSuperView();
	while((parent != NULL) && (dynamic_cast<LWindow*>(parent) == NULL))
		parent = parent->GetSuperView();
	LWindow* wnd = dynamic_cast<LWindow*>(parent);
	if (wnd)
		InstallEventHandler(GetWindowEventTarget(wnd->GetMacWindow()));
}

#pragma mark -

// ---------------------------------------------------------------------------
//	¥ SetTextPtr
// ---------------------------------------------------------------------------

void
CTextWidget::SetTextPtr(
	const void*		inTextP,
	ByteCount		inTextLength)
{
	LCFString	str;
	str.Append((UniChar*)inTextP, inTextLength / sizeof(UniChar));
	
	SetCFDescriptor(str);
}


#if PP_Uses_CFDescriptor

// ---------------------------------------------------------------------------
//	¥ CopyCFDescriptor												  [public]
// ---------------------------------------------------------------------------
//	Return contents as a CFString
//
//	CFStringRef will be NULL if we fail. This will happen if the MLTE object
//	contains data other than text (pictures, movies, etc.) or no text at all.

CFStringRef
CTextWidget::CopyCFDescriptor() const
{
	CFStringRef		stringRef = NULL;
	
	if (mTextLength != 0) {
		stringRef = ::CFStringCreateWithCharacters( NULL,
													mTextPtr,
													mTextLength );
	}

	return stringRef;
}


// ---------------------------------------------------------------------------
//	¥ SetCFDescriptor												  [public]
// ---------------------------------------------------------------------------

void
CTextWidget::SetCFDescriptor(
	CFStringRef	inString)
{
	UniCharCount	textLength	= (UniCharCount) ::CFStringGetLength(inString);
	UniChar*		textPtr		= new UniChar[textLength];
	::CFStringGetCharacters(inString, CFRangeMake(0, (CFIndex) textLength), textPtr);
	
	delete[] mTextPtr;
	delete[] mHiddenPtr;
	
	// Only do change notification if an actual change occurs
	bool changed = (textLength != mTextLength) || (textLength != 0);

	mTextCapacity = mTextLength = textLength;
	mTextPtr	= textPtr;
	if (mPasswordMode)
	{
		mHiddenPtr = new UniChar[mTextLength];
		for(UniCharArrayOffset i = 0; i < mTextLength; i++)
			mHiddenPtr[i] = 0x2022;	// Bullet character
	}
	else
		mHiddenPtr  = NULL;
	
	mLineCount = 0;
	mLineBreaks.clear();
	mLineHeights.clear();
	mLineDescents.clear();
	mLineOffsets.clear();
	
	// Wipe old selection
	mSelection.withHilight = 0;
	mSelection.anchor = 0;
	mSelection.caret = 0;
	mSelection.length = 0;
	mSelection.leading = true;
	mSelection.numberOfSelectedLines = true;
	delete[] mSelection.selBlocks;
	mSelection.selBlocks = NULL;

	PrepareTextLayout();
	if (changed)
		UserChangedText();
	Refresh();
}

#endif

#pragma mark -

// ---------------------------------------------------------------------------
//	¥ ObeyCommand
// ---------------------------------------------------------------------------
//	Handle standard editing commands

Boolean
CTextWidget::ObeyCommand(
	CommandT	inCommand,
	void*		ioParam)
{
	Boolean		cmdHandled = true;

	switch (inCommand)
	{
	case cmd_Cut:
		DoCut();
		break;
		
	case cmd_Copy:
		DoCopy();
		break;
		
	case cmd_Paste:
		DoPaste();
		break;
		
	case cmd_Clear:
		DoClear();
		break;
		
	case cmd_SelectAll:
		if (IsSelectable())
			SelectAll();
		break;

	case cmd_ActionCut:
	case cmd_ActionPaste:
	case cmd_ActionClear:
	case cmd_ActionTyping:
	{
		AdjustTextLayout();
		Refresh();
		UserChangedText();
		break;
	}

	case cmd_ActionDone:
	case cmd_ActionDeleted:
	{
		if (mTypingAction == static_cast<CTWTypingAction*>(ioParam)) {
			mTypingAction = NULL;
		}
		break;
	}

	default:
		cmdHandled = LCommander::ObeyCommand(inCommand, ioParam);
		break;
	}
	
	// Nearly all commands require an update
	SetUpdateCommandStatus(true);

	return cmdHandled;
}


// ---------------------------------------------------------------------------
//	¥ FindCommandStatus
// ---------------------------------------------------------------------------
//	Pass back the status of a Command

void
CTextWidget::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	switch (inCommand)
	{
	case cmd_Cut:
	case cmd_Clear:
		outEnabled = !IsReadOnly() && (mSelection.length != 0);
		break;
	
	case cmd_Copy:
		outEnabled = IsSelectable() && (mSelection.length != 0);
		break;

	case cmd_Paste:
		outEnabled = !IsReadOnly();
		break;
		
	case cmd_SelectAll:
		outEnabled = IsSelectable();
		break;

	default:
		LCommander::FindCommandStatus(inCommand, outEnabled,
								outUsesMark, outMark, outName);
		break;
	}
}


// ---------------------------------------------------------------------------
//	¥ HandleKeyPress
// ---------------------------------------------------------------------------
//	Handle key stroke directed at an EditField
//
//	Return true if the EditField handles the keystroke

Boolean
CTextWidget::HandleKeyPress(
	const EventRecord&	inKeyEvent)
{
	Boolean		keyHandled = true;
	SInt16		theKey = inKeyEvent.message & charCodeMask;
	
	// Arrow and navigation keys can respond to command key
	switch(theKey)
	{
	case char_LeftArrow:
	case char_RightArrow:
	case char_UpArrow:
	case char_DownArrow:
	{
		//	arrow keys
		DoArrowKey(inKeyEvent.message & charCodeMask, inKeyEvent.modifiers);
		::ObscureCursor();
		ShowSelection(true);
		return keyHandled;
	}

	case char_Home:
	case char_End:
	case char_PageUp:
	case char_PageDown:
		DoScrollKey(inKeyEvent.message & charCodeMask, inKeyEvent.modifiers);
		::ObscureCursor();
		return keyHandled;
	}

	// Always pass up when the command key is used with anything else
	if (inKeyEvent.modifiers & cmdKey)
		return LCommander::HandleKeyPress(inKeyEvent);

	switch(theKey)
	{
	case kBackspaceCharCode:
		if (!IsReadOnly())
		{
			if ((mSelection.caret != 0) || (mSelection.length != 0))
			{
				if ((mTypingAction == NULL) || mTypingAction->NeedsNewAction(mSelection.caret)) {
					mTypingAction = new CTWTypingAction(this, this);
					PostAction(mTypingAction);
				}

				// Look for option key and bump selection to word
				if ((inKeyEvent.modifiers & optionKey) && (mSelection.length == 0))
				{
					UniCharArrayOffset result;
					::ATSULeftwardCursorPosition(mTextLayout, mSelection.caret, kATSUByWord, &result);
					if (result < mSelection.caret)
					{
						mSelection.length = mSelection.caret - result;
						mSelection.caret = result;
					}
				}

				if (mTypingAction != NULL) {
					try {
						mTypingAction->BackwardErase();
					} catch (...) {
						CLOG_LOGCATCH(...);
						PostAction(NULL);
					}
				}

				if (mSelection.length != 0)
				{
					DoClearAction();
				}
				else
				{
					FocusDraw();
					DeleteText(mSelection.caret - 1, 1);
					Refresh();
					UserChangedText();
				}
				::ObscureCursor();
				mPendingDrawCaret = true;
			}
		}
		else
			keyHandled = false;
		break;

	case kDeleteCharCode:
		if (!IsReadOnly())
		{
			if ((mSelection.caret != mTextLength) || (mSelection.length != 0))
			{
				if ((mTypingAction == NULL) || mTypingAction->NeedsNewAction(mSelection.caret)) {
					mTypingAction = new CTWTypingAction(this, this);
					PostAction(mTypingAction);
				}

				// Look for option key and bump selection to word
				if ((inKeyEvent.modifiers & optionKey) && (mSelection.length == 0))
				{
					UniCharArrayOffset result;
					::ATSURightwardCursorPosition(mTextLayout, mSelection.caret, kATSUByWord, &result);
					if (result > mSelection.caret)
					{
						mSelection.length = result - mSelection.caret;
					}
				}

				if (mTypingAction != NULL) {
					try {
						mTypingAction->ForwardErase();
					} catch (...) {
						CLOG_LOGCATCH(...);
						PostAction(NULL);
					}
				}

				if (mSelection.length != 0)
				{
					DoClearAction();
				}
				else
				{
					FocusDraw();
					DeleteText(mSelection.caret, 1);
					Refresh();
					UserChangedText();
				}
				::ObscureCursor();
				ShowSelection(true);
				mPendingDrawCaret = true;
			}
		}
		else
			keyHandled = false;
		break;

	default:
		if (!IsReadOnly())
		{
			FocusDraw();

			if ((mTypingAction == NULL) || mTypingAction->NeedsNewAction(mSelection.caret)) {
				mTypingAction = new CTWTypingAction(this, this);
				PostAction(mTypingAction);
			}

			if (mTypingAction != NULL) {
				try {
					mTypingAction->InputCharacter();
				} catch(...) {
					CLOG_LOGCATCH(...);
					PostAction(NULL);
				}
			}

			// Clear any selection
			if (mSelection.length != 0)
			{
				DoClearAction();
			}
			
			// Now insert text
			UniChar uc = theKey;
			InsertText(&uc, 1, false);
			::ObscureCursor();
			ShowSelection(true);
			mPendingDrawCaret = true;
		}
		else
			keyHandled = false;
		break;
	}

	return !keyHandled ? LCommander::HandleKeyPress(inKeyEvent) : true;
}

Boolean CTextWidget::HandleKeyPress(const EventRecord& inKeyEvent, const UniChar* uc, UniCharCount ucount)
{
	Boolean keyHandled = true;

	// Do single key input
	if (!IsReadOnly())
	{
		FocusDraw();

		if ((mTypingAction == NULL) || mTypingAction->NeedsNewAction(mSelection.caret)) {
			mTypingAction = new CTWTypingAction(this, this);
			PostAction(mTypingAction);
		}

		if (mTypingAction != NULL) {
			try {
				// Add each unicode character
				mTypingAction->InputCharacter(ucount);
			} catch(...) {
				CLOG_LOGCATCH(...);
				PostAction(NULL);
			}
		}

		// Clear any selection
		if (mSelection.length != 0)
		{
			DoClearAction();
		}
		
		// Now insert text
		InsertText(uc, ucount, false);
		::ObscureCursor();
		ShowSelection(true);
		mPendingDrawCaret = true;
	}
	else
		keyHandled = false;
	
	return keyHandled;
}

OSStatus CTextWidget::ProcessKeyEvent(EventHandlerCallRef inCallRef, EventRef inEventRef)
{
	OSStatus result = eventNotHandledErr;

	// Determine type of event
	/*UInt32 eclass = */ GetEventClass(inEventRef);
	UInt32 ekind = GetEventKind(inEventRef);

	switch(ekind)
	{
	case kEventTextInputUnicodeForKeyEvent:
		result = HandleKeyEvent(inCallRef, inEventRef);
		break;
	case kEventTextInputOffsetToPos:
		break;
	case kEventTextInputPosToOffset:
		break;
	case kEventTextInputGetSelectedText:
		break;
	default:;
	}
	
	return result;
}

OSStatus CTextWidget::HandleKeyEvent(EventHandlerCallRef inCallRef, EventRef inEventRef)
{
	OSStatus result = eventNotHandledErr;

	// Get unicode text size
	UInt32 dataSize;
	GetEventParameter(inEventRef, kEventParamTextInputSendText, typeUnicodeText, NULL, 0, &dataSize, NULL);
	UniCharCount ucount = dataSize / sizeof(UniChar);
	
	// Create space for chars
	std::auto_ptr<UniChar> uc(new UniChar[ucount]);
	
	// Get the unicode text
	GetEventParameter(inEventRef, kEventParamTextInputSendText, typeUnicodeText, NULL, dataSize, NULL, uc.get());
	
	// Get original raw event
	EventRef rawEvent;
	GetEventParameter(inEventRef, kEventParamTextInputSendKeyboardEvent, typeEventRef, NULL, sizeof(rawEvent), NULL, &rawEvent);
	
	// Get modifiers
	UInt32 modifiers;
	GetEventParameter(rawEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(modifiers), NULL, &modifiers);
	
	// Get char code
	char char_code;
	GetEventParameter(rawEvent, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(char_code), NULL, &char_code);
	
	// Get key code
	UInt32 key_code;
	GetEventParameter(rawEvent, kEventParamKeyCode, typeChar, NULL, sizeof(key_code), NULL, &key_code);
	
	// Special hack for delete key <-> escape key problem
	if (char_code == char_Backspace)
		key_code = 0x33000000;

	// Create fake event record
	EventRecord evt;
	evt.what = keyDown;
	evt.message = 0;
	evt.message |= char_code;
	evt.message |= ((key_code & 0xFF000000) >> 16);
	evt.when = 0;
	evt.where = Point_00;
	evt.modifiers = modifiers;
	
	if ((modifiers & cmdKey) == 0)
	{
		if ((ucount == 1) && (uc.get()[0] < 0x0080))
		{
			// Do regular key input
			if (HandleKeyPress(evt))
				result = noErr;
		}
		else
		{
			// Do unicode key input
			if (HandleKeyPress(evt, uc.get(), ucount))
				result = noErr;
		}
	}
	
	return result;
}

void CTextWidget::DoArrowKey(UniChar key, EventModifiers mods)
{
	UniCharArrayOffset selStart = mSelection.caret;
	UniCharArrayOffset selEnd = selStart + mSelection.length;
	UniCharArrayOffset oNewStartOffset = selStart;
	UniCharArrayOffset oNewEndOffset = selEnd;

	if ((mods & shiftKey) == 0)
	{
		if (selStart < selEnd)
		{
			if ((key == char_LeftArrow) || (key == char_UpArrow))
			{
				oNewEndOffset = oNewStartOffset = selStart;
			}
			else
			{
				oNewEndOffset = oNewStartOffset = selEnd;
			}
		}
		else
		{
			oNewEndOffset = oNewStartOffset = OffsetArrowKey(selStart, key, mods);
		}
	}
	else
	{
		if (selStart < selEnd)
		{
			UniCharArrayOffset boat = (mSelection.anchor == selStart) ? selEnd : selStart;
			UniCharArrayOffset new_boat = OffsetArrowKey(boat, key, mods);
			
			if (new_boat < mSelection.anchor)
			{
				oNewStartOffset = new_boat;
				oNewEndOffset = mSelection.anchor;
			}
			else
			{
				oNewStartOffset = mSelection.anchor;
				oNewEndOffset = new_boat;
			}
		}
		else
		{
			mSelection.anchor = selStart;
			UniCharArrayOffset boat = selStart;
			UniCharArrayOffset new_boat = OffsetArrowKey(boat, key, mods);
			
			if (new_boat < mSelection.anchor)
			{
				oNewStartOffset = new_boat;
				oNewEndOffset = mSelection.anchor;
			}
			else
			{
				oNewStartOffset = mSelection.anchor;
				oNewEndOffset = new_boat;
			}
		}
	}
	
	// Now do new selection
	if ((selStart != oNewStartOffset) || (selEnd != oNewEndOffset))
		ModifySelection(oNewStartOffset, oNewEndOffset);

	// Always make the caret visible immediately after a move
	if (mSelection.length == 0)
		DrawCaret(true);
}

UniCharArrayOffset CTextWidget::OffsetArrowKey(UniCharArrayOffset start, UniChar key, EventModifiers mods)
{
	UniCharArrayOffset result = 0;

	switch(key)
	{
	case char_LeftArrow:
		if ((mods & cmdKey) == 0)
		{
			if ((mods & optionKey) == 0)
				::ATSULeftwardCursorPosition(mTextLayout, start, kATSUByCharacter, &result);
			else
			{
				result = start;
				while(true)
				{
					UniCharArrayOffset new_result = result;
					::ATSULeftwardCursorPosition(mTextLayout, result, kATSUByWord, &new_result);
					if (new_result == result)
						break;
					result = new_result;
					UniChar uc = mTextPtr[result];
					if ((uc < 0x100) && !ispunct(uc & 0xFF) && !isspace(uc & 0xFF))
						break;
				}
			}
		}
		else
		{
			ItemCount line = OffsetToLine(start);
			if ((mLineCount != 0) && (line != 0))
				result = mLineBreaks[line - 1];
			else
				result = 0;
		}
		break;
	case char_RightArrow:
		if ((mods & cmdKey) == 0)
		{
			if ((mods & optionKey) == 0)
				::ATSURightwardCursorPosition(mTextLayout, start, kATSUByCharacter, &result);
			else
			{
				result = start;
				while(true)
				{
					UniCharArrayOffset new_result = result;
					::ATSURightwardCursorPosition(mTextLayout, result, kATSUByWord, &new_result);
					if ((new_result == result) || (new_result == 0))
						break;
					result = new_result;
					UniChar uc = mTextPtr[result - 1];
					if ((uc < 0x100) && !ispunct(uc & 0xFF) && !isspace(uc & 0xFF))
						break;
				}
			}
		}
		else
		{
			ItemCount line = OffsetToLine(start);
			if (mLineCount != 0)
			{
				result = mLineBreaks[line];
				if ((result > 0) && (result != mTextLength))
					result--;
			}
			else
				result = 0;
		}
		break;
	case char_UpArrow:
		if ((mods & cmdKey) == 0)
		{
			SPoint32 imagePt = OffsetToImagePoint(start);
			ItemCount line = OffsetToLine(start);
			if (line != 0)
			{
				bool leading;
				imagePt.v -= mLineHeights[line];
				result = PointToOffset(imagePt, leading);
				if (!leading)
					result--;
			}
			else
				result = 0;
		}
		else
		{
			result = 0;
		}
		break;
	case char_DownArrow:
		if ((mods & cmdKey) == 0)
		{
			SPoint32 imagePt = OffsetToImagePoint(start);
			ItemCount line = OffsetToLine(start);
			if ((mLineCount != 0) && (line < mLineCount - 1))
			{
				bool leading;
				imagePt.v += mLineHeights[line + 1];
				result = PointToOffset(imagePt, leading);
				if (!leading && (result != mTextLength))
					result--;
			}
			else
				result = mTextLength;
		}
		else
		{
			result = mTextLength;
		}
		break;
	}
	
	return result;
}

void CTextWidget::DoScrollKey(UniChar key, EventModifiers mods)
{
	Rect	marginFrame;
	CalcMarginFrameRect(marginFrame);

	UniCharArrayOffset selStart = mSelection.caret;
	UniCharArrayOffset selEnd = selStart + mSelection.length;
	UniCharArrayOffset oNewStartOffset = selStart;
	UniCharArrayOffset oNewEndOffset = selEnd;

	switch(key)
	{
	case char_Home:
		ScrollPinnedImageTo(0, 0, true);
		oNewStartOffset = 0;
		break;
	case char_End:
		ScrollPinnedImageTo(0, (mLineCount != 0) ? mLineOffsets[mLineCount - 1] : 0, true);
		oNewEndOffset = mTextLength;
		break;
	case char_PageUp:
		ScrollPinnedImageBy(0, marginFrame.top - marginFrame.bottom, true);
		{
			ItemCount line = PointToLine(topLeft(marginFrame));
			if ((mLineCount != 0) && (line != 0))
				oNewStartOffset = mLineBreaks[line - 1];
			else
				oNewStartOffset = 0;
		}
		break;
	case char_PageDown:
		ScrollPinnedImageBy(0, marginFrame.bottom - marginFrame.top, true);
		{
			ItemCount line = PointToLine(botRight(marginFrame));
			if (mLineCount != 0)
				oNewEndOffset = mLineBreaks[line];
			else
				oNewEndOffset = 0;
		}
		break;
	}

	// Now do new selection if shift key was down
	if (((mods & shiftKey) == shiftKey) && ((selStart != oNewStartOffset) || (selEnd != oNewEndOffset)))
		ModifySelection(oNewStartOffset, oNewEndOffset);
}

// ---------------------------------------------------------------------------
//	¥ ResizeFrameBy
// ---------------------------------------------------------------------------
//	Change the Frame size by the specified amounts
//
//		inWidthDelta and inHeightDelta specify, in pixels, how much larger
//		to make the Frame. Positive deltas increase the size, negative deltas
//		reduce the size.

void
CTextWidget::ResizeFrameBy(
	SInt16		inWidthDelta,
	SInt16		inHeightDelta,
	Boolean		inRefresh)
{
									// Resize Pane
	LView::ResizeFrameBy(inWidthDelta, inHeightDelta, inRefresh);

	// Reset line breaks in text if wrapping
	if (mWordWrap)
	{
		Rect	marginFrame;
		CalcMarginFrameRect(marginFrame);

		SDimension32 imageSize;
		GetImageSize(imageSize);
		ResizeImageTo(marginFrame.right - marginFrame.left, imageSize.height, true);

		AdjustTextLayout();
	}
}


// ---------------------------------------------------------------------------
//	¥ MoveBy
// ---------------------------------------------------------------------------
//	Move the location of the Frame by the specified amounts
//
//		inHorizDelta and inVertDelta specify, in pixels, how far to move the
//		Frame (within its surrounding Image). Positive horiz deltas move to
//		the right, negative to the left. Positive vert deltas move down,
//		negative up.

void
CTextWidget::MoveBy(
	SInt32		inHorizDelta,
	SInt32		inVertDelta,
	Boolean		inRefresh)
{
	LView::MoveBy(inHorizDelta, inVertDelta, inRefresh);
}


// Scroll to specific point
void
CTextWidget::ScrollImageBy(
	SInt32		inLeftDelta,		// Pixels to scroll horizontally
	SInt32		inTopDelta,			// Pixels to scroll vertically
	Boolean		inRefresh)
{
	LView::ScrollImageBy(inLeftDelta, inTopDelta, false);
	Draw(NULL);
}

void
CTextWidget::AdjustImageToText()
{
	// Determine total height
	SInt32 total_height = (mLineCount != 0) ? mLineOffsets[mLineCount - 1] + mLineHeights[mLineCount - 1] : 0;

	// Redo scroll units (need this before doing scroll bar update)
	mScrollUnit.h = 16;
	mScrollUnit.v = GetLineHeight(1);
	if (mScrollUnit.v == 0)
		mScrollUnit.v = 1;

	// Redo frame/image sizes to force scroll bar up date
	Rect	marginFrame;
	CalcMarginFrameRect(marginFrame);
	
	SDimension32 imageSize;
	GetImageSize(imageSize);
	ResizeImageTo(mWordWrap ? marginFrame.right - marginFrame.left : imageSize.width, total_height + mMargins.top + mMargins.bottom, true);
}

#pragma mark -

pascal void CTextWidget::IdleTimer(EventLoopTimerRef inTimer, void * inUserData)
{
	UInt32			maxSleep;

	static_cast<CTextWidget*>(inUserData)->Idle(&maxSleep);

	SetEventLoopTimerNextFireTime(inTimer, TicksToEventTime(maxSleep));
}

void CTextWidget::Idle(UInt32* maxSleep)
{
	UInt32		currentTime = TickCount();
	UInt32		blinkTime ;
	UInt32		caretInterval ;
	UInt32		sleep ;
	//static bool btn_down = false;

	if (IsActive())
	{
		// get caret interval
		caretInterval = GetCaretTime();

		// calculate when the caret should be blinked again
		blinkTime = mSelection.caretTime + caretInterval;

		if (currentTime < blinkTime)
		{
			sleep = blinkTime - currentTime;
		}
		else
		{
			// Must preserve visible and clip regions as this call can
			// occur asynchronously and it must not upset any other
			// drawing in progress
			StGrafPortSaver		_save;
			StClipOriginState	_state;
			StVisibleRgn		_vis(UQDGlobals::GetCurrentPort());
			
			BlinkCaret();
			sleep = caretInterval;
		}
	}
	else
	{
		// if we don't need to blink the caret, we can sleep forever
		sleep = GetCaretTime();
	}

	// return sleepTime to the caller if maxSleep isn't NULL
	if (maxSleep)
	{
		*maxSleep = sleep;
	}
}

void CTextWidget::BlinkCaret()
{
	//SInt16		direction = kHilite;
	//Boolean		useDualCaret = false;

	// do nothing if we're not active
	if (!IsActive() && !IsSelectable())
	{
		return;
	}

	// Invert caret
	DrawCaret(!mSelection.caretVisible);
}

void CTextWidget::DrawCaret(bool on)
{
	// Handle no redraw case
	if (!mRedrawOn)
	{
		mSelection.caretVisible = false;
		return;
	}

	// Handle nothing being shown case
	if (!mSelection.caretVisible && !on)
		return;

	FocusDraw();

	// Get caret location
	ATSUCaret oMainCaret;
	ATSUCaret oSecondCaret;
	Boolean oCaretIsSplit;
	if (::ATSUOffsetToPosition(mTextLayout, mSelection.caret, mSelection.leading, &oMainCaret, &oSecondCaret, &oCaretIsSplit) != noErr)
	{
		oCaretIsSplit = false;
		oMainCaret.fX = 0;
		oMainCaret.fY = IntToFixed(-10);
		oMainCaret.fDeltaX = 0;
		oMainCaret.fDeltaY = IntToFixed(3);
	}

	// Get CG image co-ords of caret adjusting for line offset
	ItemCount line = OffsetToLine(mSelection.caret);
	//if (!mSelection.leading && (line != 0) && (mSelection.caret != mTextLength))
	//	line--;
	float start_x = FixedToFloat(oMainCaret.fX);
	float start_y = FixedToFloat(oMainCaret.fY) + (float)((mLineCount != 0) ? mLineOffsets[line] + mLineHeights[line] - mLineDescents[line] : 0);
	float end_x = FixedToFloat(oMainCaret.fDeltaX);
	float end_y = FixedToFloat(oMainCaret.fDeltaY) + (float)((mLineCount != 0) ? mLineOffsets[line] + mLineHeights[line] - mLineDescents[line] : 0);

	// Special check for last line
	if ((mLineCount == 0) || (mLineHeights[line] == 0))
	{
		float lineHeight = end_y - start_y;
		float lineDescent = end_y;
		start_y += lineHeight - lineDescent;
		end_y += lineHeight - lineDescent;
	}
	
	// Special check for last char
	if (mTextLength && (mSelection.caret == mTextLength) && (mTextPtr[mTextLength - 1] == '\r'))
	{
		start_x = 0.0F;
		end_x = 0.0F;
		start_y += mLineHeights[line];
		end_y += mLineHeights[line];
	}

	// Convert to local co-ords with margins adjust
	SPoint32 start = { start_x, start_y };
	SPoint32 end = { end_x, end_y - 1 };

	// Is the new cursor inside the frame
	bool inside_frame = ImageRectIntersectsFrame(start.h, start.v, end.h + 1, end.v);
	start = ImageToMarginPoint32(start);
	end = ImageToMarginPoint32(end);

	// Invert caret
	SRect32 newCaret = { start.h, start.v, end.h, end.v };
	
	// Clip to frame + margins
	Rect marginFrame;
	CalcMarginFrameRect(marginFrame);
	newCaret.top = std::max(newCaret.top, (long)marginFrame.top);
	newCaret.bottom = std::min(newCaret.bottom, (long)marginFrame.bottom);		

	// Look for change in state
	if (mSelection.caretVisible ^ on)
	{
		StColorPenState _save;
		::PenMode(srcXor);

		// Hide old one
		if (mSelection.caretVisible)
		{
			::MoveTo(mSelection.lastCaret.left, mSelection.lastCaret.top);
			::LineTo(mSelection.lastCaret.right, mSelection.lastCaret.bottom);
			mSelection.caretVisible = false;
		}
		
		// Show the new one if no selection
		else if ((mSelection.length == 0) && inside_frame)
		{
			::MoveTo(newCaret.left, newCaret.top);
			::LineTo(newCaret.right, newCaret.bottom);
			mSelection.caretVisible = true;
		}
	}
	
	// Move old caret if on and position changed
	else if (on && ((mSelection.lastCaret.left != newCaret.left) ||
					(mSelection.lastCaret.top != newCaret.top) ||
					(mSelection.lastCaret.right != newCaret.right) ||
					(mSelection.lastCaret.bottom != newCaret.bottom)))
	{
		StColorPenState _save;
		::PenMode(srcXor);

		// Remove old one
		::MoveTo(mSelection.lastCaret.left, mSelection.lastCaret.top);
		::LineTo(mSelection.lastCaret.right, mSelection.lastCaret.bottom);
		mSelection.caretVisible = false;

		// Draw new one if no selection
		if ((mSelection.length == 0) && inside_frame)
		{
			::MoveTo(newCaret.left, newCaret.top);
			::LineTo(newCaret.right, newCaret.bottom);
			mSelection.caretVisible = true;
		}
	}
	
	// Update state
	mSelection.lastCaret = newCaret;
	mSelection.caretTime = ::TickCount();
}

// ---------------------------------------------------------------------------
//		¥ UserChangedText
// ---------------------------------------------------------------------------
//	Text of TextEdit has changed as a result of user action
//
//	Override to validate field and/or dynamically update as the user
//	types. This function is not called by SetDescriptor, which is typically
//	used to programatically change the text.

void
CTextWidget::UserChangedText()
{
}


// ---------------------------------------------------------------------------
//	¥ AdjustMouseSelf
// ---------------------------------------------------------------------------
//	Set cursor and mouse region

void
CTextWidget::AdjustMouseSelf(
	Point				inPortPt,
	const EventRecord&	/* inMacEvent */,
	RgnHandle			ioMouseRgn)
{
	if (!mReadOnly)
	{
		// Check for drag & drop and non-zero selection
		if (DragAndDropIsPresent())
		{
			// Is cursor over selection
			Point localPt = inPortPt;
			PortToLocalPoint(localPt);
			if (PointOverSelection(localPt))
				// Use arrow over selection
				UCursor::SetArrow();
			else
				// Use default cursor
				UCursor::SetIBeam();
		}
		else
		UCursor::SetIBeam();
	}
	else
		UCursor::SetArrow();
}

void
CTextWidget::CalcPortOrigin()
{
	mPortOrigin = Point_00;
	//LView::CalcPortOrigin();
}

// ---------------------------------------------------------------------------
//	¥ FocusDraw														  [public]
// ---------------------------------------------------------------------------

Boolean
CTextWidget::FocusDraw(
	LPane*	/* inSubPane */)
{
	Boolean	revealed = LView::FocusDraw();
	
		// To keep the MLTE happy, we must set the
		// origin to (0, 0). This invalidates PP's focus cache
		// and shifts the clipping region.
		
										// Get port bounds and clipping
										//   region before changing origin
	GrafPtr		port = UQDGlobals::GetCurrentPort();
	Rect		portRect;
	::GetPortBounds(port, &portRect);
	
	StRegion	clipRgn;
	::GetClip(clipRgn);
	
	::SetOrigin(0, 0);
	
										// After origin change, PP focus
	LView::OutOfFocus(NULL);				//   is no longer valid

										// We want to clip to the same
										//   screen area as before, so we
										//   must offset the clip region
										//   to account for the origin change
	clipRgn.OffsetBy((SInt16) -portRect.left, (SInt16) -portRect.top);
	::SetClip(clipRgn);

	return revealed;
}

// ---------------------------------------------------------------------------
//	¥ PortToLocalPoint
// ---------------------------------------------------------------------------
//	Convert point from Port to Local coordinates

void
CTextWidget::PortToLocalPoint(
	Point&	ioPoint) const
{
	//LView::PortToLocalPoint(ioPoint);
}		// No conversion needed. Local coords are the same as Port coords.


// ---------------------------------------------------------------------------
//	¥ LocalToPortPoint
// ---------------------------------------------------------------------------
//	Convert point from Local to Port coordinates

void
CTextWidget::LocalToPortPoint(
	Point&	ioPoint) const
{
	//LView::LocalToPortPoint(ioPoint);
}		// No conversion needed. Local coords are the same as Port coords.

#pragma mark -

void
CTextWidget::ActivateSelf()
{
	// Activate TSM
	if (mTSMDocument != 0)
		::ActivateTSMDocument(mTSMDocument);
}

void
CTextWidget::DeactivateSelf()
{
	// Deactivate TSM
	if (mTSMDocument != 0)
		::DeactivateTSMDocument(mTSMDocument);
	
	// Always hide caret
	DrawCaret(false);
}

// ---------------------------------------------------------------------------
//	¥ BeTarget
// ---------------------------------------------------------------------------
//	EditField is becoming the Target

void
CTextWidget::BeTarget()
{
	Activate();
}


// ---------------------------------------------------------------------------
//	¥ DontBeTarget
// ---------------------------------------------------------------------------
//	EditField is no longer the Target

void
CTextWidget::DontBeTarget()
{
	Deactivate();
}

#pragma mark -

void CTextWidget::SetPasswordMode()
{
	mPasswordMode = true;
	if (mTextPtr)
	{
		mHiddenPtr = new UniChar[mTextLength];
		for(UniCharArrayOffset i = 0; i < mTextLength; i++)
			mHiddenPtr[i] = 0x2022; // Bullet
	}
}

void CTextWidget::SetBackground(const RGBColor& color)
{
	if (dynamic_cast<CBetterScrollerX*>(GetSuperView()))
	{
		dynamic_cast<CBetterScrollerX*>(GetSuperView())->SetBackgroundColor(color);
	}
	
	mCurrentBackground = color;

	// Set smart anti-aliased highlighting
	ATSURGBAlphaColor atsuicolor = { CGUtils::GetCGRed(CGUtils::GetColor(color)), CGUtils::GetCGGreen(CGUtils::GetColor(color)), CGUtils::GetCGBlue(CGUtils::GetColor(color)), 1.0F };
	ATSUUnhighlightData uhd;
	uhd.dataType = kATSUBackgroundColor;
	uhd.unhighlightData.backgroundColor = atsuicolor;
	::ATSUSetHighlightingMethod(mTextLayout, kRedrawHighlighting, &uhd);

}

void CTextWidget::SetRedraw(bool redraw)
{
	if (mRedrawOn ^ redraw)
	{
		mRedrawOn = redraw;
		
		// When turning on force a refresh and possibly and adjust
		if (mRedrawOn)
		{
			if (mAdjustPending)
			{
				AdjustTextLayout();
				mAdjustPending = false;
			}
			Refresh();
		}
	}
}

// ---------------------------------------------------------------------------
//	¥ DrawSelf													   [protected]
// ---------------------------------------------------------------------------

void
CTextWidget::DrawSelf()
{
	// Do not draw if redraw is off
	if (!mRedrawOn)
		return;
	
	DrawCaret(false);

	Rect	frame;
	CalcLocalFrameRect(frame);

	ApplyForeAndBackColors();
	
	// Set clip to margin area	
	Rect	marginFrame;
	CalcMarginFrameRect(marginFrame);
	StClipRgnState	clipRgn;
	clipRgn.ClipToIntersection(marginFrame);

	// Set CG drawing
	{
		CGUtils::CGContextFromQD _cg(UQDGlobals::GetCurrentPort(), false);
		StValueChanger<CGContextRef> _context(mCurrentContext, _cg);
		mTextLayout.SetCGContext(_cg);
		CGRect cgclipper = ::CGContextGetClipBoundingBox(_cg);
		{
			CGUtils::CGContextSaver _saver(_cg);
			::CGContextSetRGBFillColor(_cg, CGUtils::GetCGRed(CGUtils::GetColor(mCurrentBackground)), CGUtils::GetCGGreen(CGUtils::GetColor(mCurrentBackground)), CGUtils::GetCGBlue(CGUtils::GetColor(mCurrentBackground)), 1.0F);
			::CGContextFillRect(_cg, cgclipper);
		}

		// Do not draw anymore if no text
		if ((mTextPtr == NULL) || (mTextLength == 0) || (mLineCount == 0))
			return;

		// Determine range of lines in the current view
		SPoint32 tl = MarginToImagePoint(topLeft(marginFrame));
		SPoint32 br = MarginToImagePoint(botRight(marginFrame));

		UInt32 first_line = 0;
		UInt32 last_line = (mLineCount != 0) ? mLineCount - 1 : 0;
		for(ItemCount line = 0; line < mLineCount; line++)
		{
			if (tl.v >= mLineOffsets[line])
			{
				first_line = line;
			}
			if (br.v <= mLineOffsets[line] + mLineHeights[line])
			{
				last_line = line;
				break;
			}
		}
		
		LATSUCoordinates	coords(mTextLayout, UQDGlobals::GetCurrentPort());

		UniCharArrayOffset lineStart = (first_line > 0) ? mLineBreaks[first_line - 1] : 0;
		
		for(ItemCount line = first_line; line <= last_line; line++)
		{
			SPoint32 imagePt = { 0, mLineOffsets[line] + mLineHeights[line] - mLineDescents[line] };
			SPoint32 localPt = ImageToMarginPoint32(imagePt);
			ATSUTextMeasurement	horizCoord;
			ATSUTextMeasurement	vertCoord;
			coords.QDToATSU(IntToFixed(localPt.h), IntToFixed(localPt.v), horizCoord, vertCoord);

			::ATSUDrawText(mTextLayout, lineStart, mLineBreaks[line] - lineStart, horizCoord, vertCoord);
			
			lineStart = mLineBreaks[line];
		}

		HilightCurrentSelection(first_line, last_line);
		
		//::CGContextFlush(_cg);
	}

	// This has to be done outside of CG context as it uses QD to draw
	if (mPendingDrawCaret)
	{
		mPendingDrawCaret = false;
		DrawCaret(true);
	}
}

// ---------------------------------------------------------------------------
//	¥ CountPanels
// ---------------------------------------------------------------------------
//	Return the number of horizontal and vertical Panels. A Panel is a
//	"frameful" of a View's Image, rounded down to integral number of lines

void
CTextWidget::CountPanels(
	UInt32	&outHorizPanels,
	UInt32	&outVertPanels)
{
	// Must have updated layout
	NeedTextLayout();

	SDimension32	imageSize;
	GetImageSize(imageSize);

	SDimension16	frameSize;
	GetFrameSize(frameSize);

	// No rounding for horizontal scroll
	outHorizPanels = 1;
	if (frameSize.width > 0  &&  imageSize.width > 0) {
		outHorizPanels = (UInt32) ((imageSize.width - 1) / frameSize.width) + 1;
	}

	// Do line height rounding for vertical panels
	outVertPanels = 1;
	unsigned long accumulated_height = 0;
	for(ItemCount line = 0; line < mLineCount; line++)
	{
		unsigned long current_line = GetLineHeight(line);
		if (accumulated_height + current_line > frameSize.height)
		{
			// Have completed a panel
			outVertPanels++;
			accumulated_height = 0;
		}
		else
			accumulated_height += current_line;
	}
}

// ---------------------------------------------------------------------------
//	¥ ScrollToPanel
// ---------------------------------------------------------------------------
//	Scroll View Image to the specified Panel
//
//	Return whether the specified Panel exists. If it doesn't, View is
//	not scrolled.

Boolean
CTextWidget::ScrollToPanel(
	const PanelSpec	&inPanel)
{
	// Must have updated layout
	NeedTextLayout();

	Boolean	panelInImage = false;

	SDimension32	imageSize;
	GetImageSize(imageSize);

	SDimension16	frameSize;
	GetFrameSize(frameSize);

	// No rounding for horizontal scroll
	UInt32 horizPanelCount = 1;
	if (frameSize.width > 0  &&  imageSize.width > 0) {
		horizPanelCount = (UInt32) ((imageSize.width - 1) / frameSize.width) + 1;
	}

	// Horizontal pos is simple scroll
	SInt32 horizPos = -1;
	if (inPanel.horizIndex <= horizPanelCount) {
		horizPos = (SInt32) (frameSize.width * (inPanel.horizIndex - 1));
	}

	// Vertical pos is line integral scroll
	SInt32 vertPos = -1;
	UInt32	vertPanelCount = 1;
	unsigned long accumulated_height = 0;
	unsigned long total_height = 0;
	for(ItemCount line = 0; line < mLineCount; line++)
	{
		// Check whether we are at the panel we want
		if (vertPanelCount == inPanel.vertIndex)
		{
			// We've got our position
			vertPos = total_height;
			break;
		}

		unsigned long current_line = GetLineHeight(line);
		if (accumulated_height + current_line > frameSize.height)
		{
			// Have completed a panel
			vertPanelCount++;
			accumulated_height = 0;
		}
		else
		{
			accumulated_height += current_line;
			total_height += current_line;
		}
	}

	if ((horizPos != -1) && (vertPos != -1))
	{
		ScrollImageTo(horizPos, vertPos, false);
		panelInImage = true;
	}

	return panelInImage;
}

// ---------------------------------------------------------------------------
//	¥ PrintPanelSelf								[protected, virtual]
// ---------------------------------------------------------------------------
//	Print a TextEditView

void
CTextWidget::PrintPanelSelf(
	const PanelSpec&	/* inPanel */)
{
	// Turn on redraw for the print operation
	StValueChanger<bool> _change(mRedrawOn, true);
	NeedTextLayout();
	DrawSelf();
}

// Allow background window D&D
void CTextWidget::Click(SMouseDownEvent &inMouseDown)
{
	if (inMouseDown.delaySelect && DragAndDropIsPresent())
	{
		// In order to support dragging from an inactive window,
		// we must explicitly test for delaySelect and the
		// presence of Drag and Drop.

		// Convert to a local point.
		PortToLocalPoint(inMouseDown.whereLocal);

		// Execute click attachments.
		if (ExecuteAttachments(msg_Click, &inMouseDown))
		{
			// Handle the actual click event.
			ClickSelf(inMouseDown);
		}

	}
	else
		// Call inherited for default behavior.
		LView::Click(inMouseDown);
}

// ---------------------------------------------------------------------------
//	¥ ClickSelf
// ---------------------------------------------------------------------------

void
CTextWidget::ClickSelf(
	const SMouseDownEvent	&inMouseDown)
{
	// Is click on selection with D&D and not delayed selection (also ignore if triple clicking)
	if (PointOverSelection(inMouseDown.whereLocal) && DragAndDropIsPresent() && (IsTarget() || inMouseDown.delaySelect) && !GotTripleClick(inMouseDown.macEvent))
	{
		// Track item long enough to distinguish between a click to
		// select, and the beginning of a drag
		bool isDrag = ::WaitMouseMoved(inMouseDown.macEvent.where);

		// Now do drag
		if (isDrag)
		{

			// If we leave the window, the drag manager will be changing thePort,
			// so we'll make sure thePort remains properly set.
			//OutOfFocus(NULL);
			FocusDraw();
			/* OSErr err = */ CreateDragEvent(inMouseDown);
			//OutOfFocus(NULL);
			
			return;
		}
	}

	OSStatus status = noErr;
	bool withShift = ((inMouseDown.macEvent.modifiers & shiftKey) != 0);

	if (not IsTarget()) {			// If not the Target, clicking in an
									//   EditField makes it the Target.
		SwitchTarget(this);
	}
	
	// Always remove current caret
	DrawCaret(false);

	// Adjust for click in margins
	Point localPt = inMouseDown.whereLocal;
	
	bool leading;
	UniCharArrayOffset newPos = PointToOffset(localPt, leading);
	if ((leading == false) && (mTextPtr[newPos] != '\r') && (newPos != mTextLength))
	{
		newPos--;
		leading = true;
	}
	UniCharArrayOffset startSel, endSel, anchor, anchor1;
	bool trackWord = false;
	bool trackLine = false;

	// Look for shift
	if (!withShift)
	{
		// test for triple-click
		if (GotTripleClick(inMouseDown.macEvent))
		{
			SelectLine(newPos, leading, &startSel, &endSel);
			anchor = startSel; anchor1 = endSel;	// we'll need both ends for tracking by word
			trackLine = true;
		}
		// test for double-click
		else if (GotDoubleClick(inMouseDown.macEvent))
		{
			SelectWord(newPos, leading, &startSel, &endSel);
			anchor = startSel; anchor1 = endSel;	// we'll need both ends for tracking by word
			trackWord = true;

			mSelection.lastDblClickTime = inMouseDown.macEvent.when;
			mSelection.lastDblClickPoint = inMouseDown.macEvent.where;
		}
		else
		{
			FocusDraw();

			// Set clip to margin area	
			Rect	marginFrame;
			CalcMarginFrameRect(marginFrame);
			StClipRgnState	clipRgn;
			clipRgn.ClipToIntersection(marginFrame);

			CGUtils::CGContextFromQD _cg(UQDGlobals::GetCurrentPort(), false);
			StValueChanger<CGContextRef> _context(mCurrentContext, _cg);
			mTextLayout.SetCGContext(_cg);

			// if simple click, then disable current selection if any first
			UnhilightCurrentSelection(0, mLineCount);
			mSelection.withHilight = false;
			mSelection.length = 0;
			mSelection.anchor = mSelection.caret = newPos;
			mSelection.leading = leading;
			anchor = startSel = endSel = mSelection.caret;
			
			mSelection.lastClickTime = inMouseDown.macEvent.when;
			mSelection.lastClickPoint = inMouseDown.macEvent.where;
			
			// Adjust for \r at line end
			if (!mSelection.leading && (mTextPtr[mSelection.caret - 1] == '\r'))
			{
				mSelection.anchor = --mSelection.caret;
				mSelection.leading = true;
			}
			
			// Update selection state
			SetSelectedBlocks();
		}
	}
	else
	{
		// if shift click, then extends the current selection if any
		startSel = mSelection.caret;
		endSel = startSel + mSelection.length;
		if (newPos < startSel)
		{
			startSel = newPos;
			anchor = endSel;
		}
		else
		{
			endSel = newPos;
			anchor = startSel;
		}
		mSelection.caret = startSel;
		mSelection.length = endSel - startSel;
	}

	if (mSelection.length > 0)
	{
		ModifySelection(startSel, endSel);
		mSelection.withHilight = true;
	}
	
	// as long as the mouse button is down, dragging...
	// !!! if we selected a word, we should extend by word, too.
	Point thePoint = inMouseDown.wherePort;
	Point lastPoint = thePoint;
	UniCharArrayOffset oldPos = newPos;
	while (::StillDown())
	{
		::GetMouse(&thePoint);

		if (/*(mLineCount > 1) && */ AutoScrollImage(thePoint)) {
			FocusDraw();
			Rect	frame;
			CalcLocalFrameRect(frame);
			SInt32 pt = ::PinRect(&frame, thePoint);
			thePoint = *(Point*)&pt;
		}

		if ((*((long *)&thePoint)) == (*((long *)&lastPoint))) continue;
		lastPoint = thePoint;
		newPos = PointToOffset(thePoint, leading);
		if (newPos == oldPos) continue;
		oldPos = newPos;

		if (!trackWord && !trackLine)
		{
			if (newPos < anchor)
			{
				startSel = newPos;
				endSel = anchor;
			}
			else
			{
				endSel = newPos;
				startSel = anchor;
			}
		}
		else if (trackWord)
		{
			if (newPos < anchor)
			{
				endSel = anchor1;
				status = ::ATSUPreviousCursorPosition(mTextLayout, leading ? newPos + 1 : newPos, kATSUByWord, &startSel);
				if (status != noErr) DebugStr("\p ATSUPreviousCursorPosition failed");
			}
			else
			{
				startSel = anchor;
				status = ::ATSUNextCursorPosition(mTextLayout, leading ? newPos : newPos - 1, kATSUByWord, &endSel);
				if (status != noErr) DebugStr("\p ATSUNextCursorPosition failed");
			}
		}
		else if (trackLine)
		{
			if (newPos < anchor)
			{
				endSel = anchor1;
				ItemCount lineNo = OffsetToLine(newPos);
				startSel = ((mLineCount != 0) && (lineNo > 0)) ? mLineBreaks[lineNo - 1] : 0;
			}
			else
			{
				startSel = anchor;
				ItemCount lineNo = OffsetToLine(newPos);
				endSel = (mLineCount != 0) ? mLineBreaks[lineNo] : mTextLength;
			}
		}

		if (startSel != mSelection.caret || endSel != mSelection.caret + mSelection.length)
			ModifySelection(startSel, endSel);
		mSelection.withHilight = true;
	}
}


// ---------------------------------------------------------------------------
//	¥ PrepareTextLayout											   [protected]
// ---------------------------------------------------------------------------

void
CTextWidget::PrepareTextLayout()
{
	try
	{
		mTextLayout.SetTextPointer(GetTextPtr(), 0, mTextLength, mTextLength);
		mTextLayout.SetRunStyle(mStyle, 0, mTextLength);
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}
	
    // Turn on font substitution
	::ATSUSetTransientFontMatching(mTextLayout, true);

	AdjustTextLayout();
}


// ---------------------------------------------------------------------------
//	¥ AdjustTextLayout											   [protected]
// ---------------------------------------------------------------------------

void
CTextWidget::AdjustTextLayout()
{
	if (mTextPtr == NULL)
		return;
	
	// If redraw is off, mark an adjust as pending
	if (!mRedrawOn)
	{
		mAdjustPending = true;
		return;
	}

	SDimension32 imageSize;
	GetImageSize(imageSize);
	
	mTextLayout.MeasureLinesMore(mTextLength, IntToFixed(imageSize.width), mLineCount, mLineBreaks, mLineHeights, mLineDescents, mLineOffsets);
							  
	// Readjust cached selection info
	SetSelectedBlocks();
	
	// Resync image to new text height
	AdjustImageToText();
}

// ---------------------------------------------------------------------------
//	¥ NeedTextLayout											   [protected]
// ---------------------------------------------------------------------------

void
CTextWidget::NeedTextLayout()
{
	// Only need to do this when some text and adjust is pending
	if ((mTextPtr == NULL) || !mAdjustPending)
		return;
	
	SDimension32 imageSize;
	GetImageSize(imageSize);
	
	mTextLayout.MeasureLinesMore(mTextLength, IntToFixed(imageSize.width), mLineCount, mLineBreaks, mLineHeights, mLineDescents, mLineOffsets);
							  
	// Readjust cached selection info
	SetSelectedBlocks();
	
	// Resync image to new text height
	AdjustImageToText();
	
	// Reset flag
	mAdjustPending = false;
}

#pragma mark -

ItemCount CTextWidget::OffsetToLine(UniCharArrayOffset offset) const
{
	// Better solution would be to do a binary search
	// Iterate over all lines
	for(ItemCount line = 0; line < mLineCount; line++)
	{
		if (offset < mLineBreaks[line])
			return line;
	}
	
	return (mLineCount != 0) ? mLineCount - 1 : 0;
}

ItemCount CTextWidget::PointToLine(Point inLocalPt) const
{
	// Convert to image pt
	SPoint32 imagePt = MarginToImagePoint(inLocalPt);

	return PointToLine(imagePt);
}

ItemCount CTextWidget::PointToLine(SPoint32 inImagePt) const
{
	if (inImagePt.v < 0)
		return 0;
	else
	{
		// Iterate over all lines
		for(ItemCount line = 0; line < mLineCount; line++)
		{
			if (inImagePt.v < mLineOffsets[line] + mLineHeights[line])
				return line;
		}
		
		return (mLineCount != 0) ? mLineCount - 1 : 0;
	}
}

UniCharArrayOffset CTextWidget::PointToOffset(Point inLocalPt, bool& leading) const
{
	// If point is in left or right margin, we must adjust it to be at the margin
	if ((mMargins.left != 0) || (mMargins.right != 0))
	{
		Rect marginFrame;
		CalcMarginFrameRect(marginFrame);
		if (inLocalPt.h < marginFrame.left)
			inLocalPt.h = marginFrame.left;
		if (inLocalPt.h > marginFrame.right)
			inLocalPt.h = marginFrame.right;
		
	}
	// Convert to image pt
	SPoint32 imagePt = MarginToImagePoint(inLocalPt);
	
	return PointToOffset(imagePt, leading);
}

UniCharArrayOffset CTextWidget::PointToOffset(SPoint32 inImagePt, bool& leading) const
{
	UniCharArrayOffset result;

	if ((mTextLength == 0) || (mLineCount == 0) || (inImagePt.h < 0) || (inImagePt.v < 0))
	{
		result = 0;
		leading = true;
	}
	else if ((inImagePt.v > 0) && (inImagePt.v > mLineOffsets[mLineCount - 1] + mLineHeights[mLineCount - 1]))
	{
		result = mTextLength;
		leading = false;
	}
	else
	{
		// Determine first which line
		ItemCount theLine = PointToLine(inImagePt);
		
		// and we call the right stuff
		UniCharArrayOffset there, notThere;

		// we seed the offset with the first Character Offset of that line
		if (theLine == 0)
			there = 0;
		else
			there = mLineBreaks[theLine - 1];

		ATSUTextMeasurement xl = IntToFixed(inImagePt.h);
		ATSUTextMeasurement yl = IntToFixed(inImagePt.v - mLineOffsets[theLine]);
		Boolean bleading;
		OSStatus status = ::ATSUPositionToOffset(mTextLayout, xl, yl, &there, &bleading, &notThere);
		leading = bleading;
		if (status != noErr)
			{
			DebugStr("\p ATSUPositionToOffset failed");
			result = 0;
			}
		else
			result = there;
		
	}

	// sanity check
	if (result > (mTextLength - 1))
		result = mTextLength;

	return result;
}

Point CTextWidget::OffsetToPoint(UniCharArrayOffset offset, bool leading) const
{
	Point result = ImageToMarginPoint(OffsetToImagePoint(offset, leading));
	return result;
}

SPoint32 CTextWidget::OffsetToImagePoint(UniCharArrayOffset offset, bool leading) const
{
	SPoint32 result;

	// Get line position
	ItemCount line = OffsetToLine(offset);
	
	// Middle of the line
	result.v = (mLineCount != 0) ? mLineOffsets[line] + mLineHeights[line] / 2 : 0;

	ATSUCaret oMainCaret;
	ATSUCaret oSecondCaret;
	Boolean oCaretIsSplit;
	::ATSUOffsetToPosition(mTextLayout, offset, leading, &oMainCaret, &oSecondCaret, &oCaretIsSplit);
	
	result.h = FixedToInt(oMainCaret.fX);
	
	return result;
}

SPoint32 CTextWidget::MarginToImagePoint(const Point& inLocalPt) const
{
	Point adjusted = inLocalPt;
	adjusted.h -= mMargins.left;
	adjusted.v -= mMargins.top;
	
	SPoint32 result;
	LocalToImagePoint(adjusted, result);
	
	return result;
}

Point CTextWidget::ImageToMarginPoint(const SPoint32& pt) const
{
	Point result;
	result.h = pt.h + mPortOrigin.h + mMargins.left + mImageLocation.h;
	result.v = pt.v + mPortOrigin.v + mMargins.top + mImageLocation.v;
	
	return result;
}

SPoint32 CTextWidget::ImageToMarginPoint32(const SPoint32& pt) const
{
	SPoint32 result = pt;
	result.h += mPortOrigin.h + mMargins.left + mImageLocation.h;
	result.v += mPortOrigin.v + mMargins.top + mImageLocation.v;
	
	return result;
}

SRect32 CTextWidget::ImageToMarginRect32(const SRect32& rect) const
{
	SRect32 result = rect;
	result.left += mPortOrigin.h + mMargins.left + mImageLocation.h;
	result.top += mPortOrigin.v + mMargins.top + mImageLocation.v;
	result.right += mPortOrigin.h + mMargins.left + mImageLocation.h;
	result.bottom += mPortOrigin.v + mMargins.top + mImageLocation.v;
	
	return result;
}

void CTextWidget::CalcMarginFrameRect(Rect& frame) const
{
	CalcLocalFrameRect(frame);
	frame.left += mMargins.left;
	frame.top += mMargins.top;
	frame.right -= mMargins.right;
	frame.bottom -= mMargins.bottom;
}

#pragma mark -

bool CTextWidget::GotDoubleClick(const EventRecord& theEvent)
{
	long deltaH = theEvent.where.h - mSelection.lastClickPoint.h;
	long deltaV = theEvent.where.v - mSelection.lastClickPoint.v;
	if (deltaH < 0) deltaH = -deltaH;
	if (deltaV < 0) deltaV = -deltaV;
	if (	(deltaH < 5) &&
			(deltaV < 5) &&
			((theEvent.when - mSelection.lastClickTime) < GetDblTime())		)
		return true;
	return false;
}

bool CTextWidget::GotTripleClick(const EventRecord& theEvent)
{
	long deltaH = theEvent.where.h - mSelection.lastDblClickPoint.h;
	long deltaV = theEvent.where.v - mSelection.lastDblClickPoint.v;
	if (deltaH < 0) deltaH = -deltaH;
	if (deltaV < 0) deltaV = -deltaV;
	if (	(deltaH < 5) &&
			(deltaV < 5) &&
			((theEvent.when - mSelection.lastDblClickTime) < GetDblTime())		)
		return true;
	return false;
}

void CTextWidget::SelectLine(UniCharArrayOffset thePos, bool leading, UniCharArrayOffset* startSel, UniCharArrayOffset* endSel)
{
	ItemCount lineNo = OffsetToLine(thePos);
	*startSel = ((mLineCount != 0) && (lineNo > 0)) ? mLineBreaks[lineNo - 1] : 0;
	*endSel = (mLineCount != 0) ? mLineBreaks[lineNo] : mTextLength;

	// and we set all the proper values.
	mSelection.anchor = mSelection.caret = *startSel;
	mSelection.length = *endSel - *startSel;
}

void CTextWidget::SelectWord(UniCharArrayOffset thePos, bool leading, UniCharArrayOffset* startSel, UniCharArrayOffset* endSel)
{
	OSStatus status = noErr;

	// let's get the end and the start of the word
	// the input position needs to be adjusted so that we don't grab the next word by mistake
	// note that leading *must* be true if thePos is the beginning of the text and false if it's the end
	status = ::ATSUNextCursorPosition(mTextLayout, leading ? thePos : thePos - 1, kATSUByWord, endSel);
	if (status != noErr) DebugStr("\p ATSUNextCursorPosition failed");
	status = ::ATSUPreviousCursorPosition(mTextLayout, leading ? thePos + 1 : thePos, kATSUByWord, startSel);
	if (status != noErr) DebugStr("\p ATSUPreviousCursorPosition failed");

	// and we set all the proper values.
	mSelection.anchor = mSelection.caret = *startSel;
	mSelection.length = *endSel - *startSel;
}

// if we just unhighlight the old and then highlight the new, we will flash.
void CTextWidget::ModifySelection(UniCharArrayOffset newStart, UniCharArrayOffset newEnd)
{
	// Always remove current caret
	DrawCaret(false);

	FocusDraw();

	// Set clip to margin area	
	Rect	marginFrame;
	CalcMarginFrameRect(marginFrame);
	StClipRgnState	clipRgn;
	clipRgn.ClipToIntersection(marginFrame);

	CGUtils::CGContextFromQD _cg(UQDGlobals::GetCurrentPort(), false);
	StValueChanger<CGContextRef> _context(mCurrentContext, _cg);
	mTextLayout.SetCGContext(_cg);

	LATSUCoordinates	coords(mTextLayout, UQDGlobals::GetCurrentPort());

	OSStatus status;
	
	mSelection.caret = std::min(newStart, mTextLength);
	mSelection.length = std::min(newEnd - newStart, mTextLength - mSelection.caret);
	mSelection.leading = true;
	
	if (mSelection.withHilight)
	{
		if (mRedrawOn)
		{
			// exercise for the reader: combine the update of the selected blocks data structure with the highlight updates.
			ItemCount i;
			
			// update the old highlights as necessary. 
			for (i = 0; i < mSelection.numberOfSelectedLines; i++)
			{
				SSelectedBlock *sb = &(mSelection.selBlocks[i]);
				UniCharArrayOffset	selStart = sb->selStart;
				UniCharArrayOffset	selEnd = sb->selStart + sb->selLength;
				UniCharArrayOffset	thisLineStart = sb->lineNumber ? mLineBreaks[sb->lineNumber - 1] : 0;
				UniCharArrayOffset	thisLineEnd = mLineBreaks[sb->lineNumber];
				UniCharArrayOffset	tempStart = std::max(thisLineStart, std::min(newStart, thisLineEnd));
				UniCharArrayOffset	tempEnd = std::min(thisLineEnd, std::max(newEnd, thisLineStart));
				
				SPoint32 localPt = ImageToMarginPoint32(sb->imageLineSel);
				ATSUTextMeasurement	horizCoord;
				ATSUTextMeasurement	vertCoord;
				coords.QDToATSU(IntToFixed(localPt.h), IntToFixed(localPt.v), horizCoord, vertCoord);

				status = noErr;
				
				// unlighlight on left
				if (tempStart > sb->selStart)
				{
					status = ::ATSUUnhighlightText(mTextLayout, horizCoord, vertCoord, selStart, std::min(tempStart, selEnd) - selStart);
					selStart = std::min(tempStart, selEnd);
					if (selStart > selEnd)
						selEnd = selStart;
				}

				// unhighlight on right
				if (tempEnd < selEnd)
				{
					status = ::ATSUUnhighlightText(mTextLayout, horizCoord, vertCoord, std::max(tempEnd, selStart), selEnd - tempEnd);
					selEnd = std::max(tempEnd, selStart);
					if (selEnd < selStart)
						selEnd = selStart;
				}
				
				// highlight on left
				if (tempStart < selStart)
				{
					status = ::ATSUHighlightText(mTextLayout, horizCoord, vertCoord, tempStart, std::min(selStart, tempEnd) - tempStart);
				}
				
				// highlight on right
				if (tempEnd > selEnd)
				{
					status = ::ATSUHighlightText(mTextLayout, horizCoord, vertCoord, std::max(tempStart, selEnd), tempEnd - std::max(tempStart, selEnd));
				}
			}
			
			// take care of any new lines in the highlight
			ItemCount oldFirstLine = mSelection.selBlocks[0].lineNumber;
			ItemCount oldLastLine = mSelection.selBlocks[mSelection.numberOfSelectedLines - 1].lineNumber;
			int newFirstLine = -1, newLastLine = -1;
			
			// find the first line where the new selection starts
			for (i = 0; (i < mLineCount) && (newFirstLine == -1); i++)
				if (newStart < mLineBreaks[i]) newFirstLine = i;
			if (newFirstLine == -1) newFirstLine = mLineCount - 1;
			
			// find the last line where the new selection ends
			for (i = newFirstLine; (i < mLineCount) && (newLastLine == -1); i++)
				if (newEnd <= mLineBreaks[i]) newLastLine = i;
			if (newLastLine == -1) newLastLine = mLineCount - 1;
			
			for (i = newFirstLine; i <= newLastLine; i++)
			{
				if (i < oldFirstLine || i > oldLastLine)
				{
					SPoint32 imagePt = { 0, mLineOffsets[i] + mLineHeights[i] - mLineDescents[i] };
					SPoint32 localPt = ImageToMarginPoint32(imagePt);
					ATSUTextMeasurement	horizCoord;
					ATSUTextMeasurement	vertCoord;
					coords.QDToATSU(IntToFixed(localPt.h), IntToFixed(localPt.v), horizCoord, vertCoord);

					UniCharArrayOffset thisLineStart = std::max(i ? mLineBreaks[i - 1] : 0, newStart);
					UniCharArrayOffset thisLineEnd = std::min(mLineBreaks[i], newEnd);
					status = ::ATSUHighlightText(mTextLayout, horizCoord, vertCoord, thisLineStart, thisLineEnd - thisLineStart);
					if (status != noErr) DebugStr("\p ATSUHighlightText failed");
				}
			}
		}
			
		SetSelectedBlocks();
	}
	else
	{
		SetSelectedBlocks();
		HilightCurrentSelection(0, mLineCount);
	}
	
	// Selection change requires command update
	SetUpdateCommandStatus(true);
}

void CTextWidget::SetSelectedBlocks()
{
	// Mark selection state
	mSelection.withHilight = (mSelection.length != 0);

	// Special case empty text
	if ((mTextLength == 0) || (mLineCount == 0))
	{
		mSelection.numberOfSelectedLines = 0;
		if (mSelection.selBlocks != NULL) ::free(mSelection.selBlocks);
		mSelection.selBlocks = NULL;
		mSelection.length = 0;
		mSelection.withHilight = false;
		return;
	}

	int i, firstLine = -1, lastLine = -1;
	
	// find the first line where the selection starts
	for (i = 0; (i < mLineCount) && (firstLine == -1); i++)
		if (mSelection.caret < mLineBreaks[i]) firstLine = i;
	if (firstLine == -1) firstLine = mLineCount - 1;
		
	// find the last line where the selection ends
	for (i = firstLine; (i < mLineCount) && (lastLine == -1); i++)
		if ((mSelection.caret + mSelection.length) < mLineBreaks[i]) lastLine = i;
	if (lastLine == -1) lastLine = mLineCount - 1;
	
	// let's allocate (deallocating first if it's previously been allocated)...
	mSelection.numberOfSelectedLines = lastLine - firstLine + 1;
	if (mSelection.selBlocks != NULL) ::free(mSelection.selBlocks);
	mSelection.selBlocks = (SSelectedBlock *)malloc(mSelection.numberOfSelectedLines * sizeof(SSelectedBlock));
	
	// and fill the values
	UniCharArrayOffset pos = mSelection.caret;
	UniCharCount length, lengthToSelect = mSelection.length;
	for (i = 0; i < mSelection.numberOfSelectedLines; i++)
	{
		SSelectedBlock *sb = &(mSelection.selBlocks[i]);
		sb->selStart = pos;
		sb->lineNumber = firstLine + i;
		length = mLineBreaks[firstLine + i] - pos;
		if (length >= lengthToSelect) length = lengthToSelect;
		sb->selLength = length;
		lengthToSelect -= length;
		pos = mLineBreaks[sb->lineNumber];
		sb->imageLineSel.h = 0;
		sb->imageLineSel.v = mLineOffsets[sb->lineNumber] + mLineHeights[sb->lineNumber] - mLineDescents[sb->lineNumber];
	}
}

void CTextWidget::HilightCurrentSelection(UniCharArrayOffset startLine, UniCharArrayOffset endLine)
{
	if ((mSelection.length == 0) || !mRedrawOn) return;

	LATSUCoordinates	coords(mTextLayout, UQDGlobals::GetCurrentPort());

	for (ItemCount i = 0; i < mSelection.numberOfSelectedLines; i++)
	{
		SSelectedBlock *sb = &(mSelection.selBlocks[i]);

		// Only do if in line range
		if ((sb->lineNumber >= startLine) && (sb->lineNumber <= endLine))
		{
			SPoint32 localPt = ImageToMarginPoint32(sb->imageLineSel);
			ATSUTextMeasurement	horizCoord;
			ATSUTextMeasurement	vertCoord;
			coords.QDToATSU(IntToFixed(localPt.h), IntToFixed(localPt.v), horizCoord, vertCoord);

			OSStatus status = IsActive() ? ::ATSUHighlightText(mTextLayout, horizCoord, vertCoord, sb->selStart, sb->selLength) :
											::ATSUHighlightInactiveText(mTextLayout, horizCoord, vertCoord, sb->selStart, sb->selLength);
			if (status != noErr) DebugStr("\p ATSUHighlightText failed");
		}
	}
}

void CTextWidget::UnhilightCurrentSelection(UniCharArrayOffset startLine, UniCharArrayOffset endLine)
{
	if ((!mSelection.withHilight) || (mSelection.length == 0) || !mRedrawOn) return;

	LATSUCoordinates	coords(mTextLayout, UQDGlobals::GetCurrentPort());

	for (ItemCount i = 0; i < mSelection.numberOfSelectedLines; i++)
	{
		SSelectedBlock *sb = &(mSelection.selBlocks[i]);

		// Only do if in line range
		if ((sb->lineNumber >= startLine) && (sb->lineNumber <= endLine))
		{
			SPoint32 localPt = ImageToMarginPoint32(sb->imageLineSel);
			ATSUTextMeasurement	horizCoord;
			ATSUTextMeasurement	vertCoord;
			coords.QDToATSU(IntToFixed(localPt.h), IntToFixed(localPt.v), horizCoord, vertCoord);

			OSStatus status = ATSUUnhighlightText(mTextLayout, horizCoord, vertCoord, sb->selStart, sb->selLength);
			if (status != noErr) DebugStr("\p ATSUUnhighlightText failed");
		}
	}
}

void CTextWidget::ShowSelection(bool refresh)
{
	if (mTextLength == 0)
		return;

	bool need_scroll = false;

	Rect marginFrame;
	CalcMarginFrameRect(marginFrame);
	SPoint32 imagePtTop = MarginToImagePoint(topLeft(marginFrame));
	SPoint32 imagePtBottom = MarginToImagePoint(botRight(marginFrame));

	// Scroll point starts out as the initial scroll point and we adjust it from there as needed
	SPoint32 scrollPt = imagePtTop;

	UniCharArrayOffset offsetToMakeVisible = mSelection.caret;

	// If selection exists then use the anchor to determine which end is 'active'
	if ((mSelection.length != 0) && (mSelection.anchor == mSelection.caret))
	{
		// Make end of selection visible
		offsetToMakeVisible += mSelection.length;
	}

	// Now use actual caret-visible point to determine scrolling

	// Get image point of selection start
	ItemCount line = OffsetToLine(offsetToMakeVisible);

	// Offset point
	SPoint32 offsetPt = OffsetToImagePoint(offsetToMakeVisible);

	// Get image point of line top and bottom
	SPoint32 caretPtTop = { offsetPt.h, mLineOffsets[line] };
	SPoint32 caretPtBottom = { offsetPt.h, mLineOffsets[line] + mLineHeights[line] };

	// Caret above frame
	if (caretPtTop.v < imagePtTop.v)
	{
		// Top of caret is at top of frame
		scrollPt.v = caretPtTop.v;
		need_scroll = true;
	}
	
	// Caret below frame
	else if (caretPtBottom.v > imagePtBottom.v)
	{
		// Bottom of caret is at bottom of frame
		scrollPt.v = caretPtBottom.v - (marginFrame.bottom - marginFrame.top);
		need_scroll = true;
	}

	// Caret to left of frame
	if (caretPtTop.h < imagePtTop.h)
	{
		// Caret is at left of frame
		scrollPt.h = caretPtTop.h;
		need_scroll = true;
	}
	
	// Caret to right of frame
	else if (caretPtBottom.h > imagePtBottom.h)
	{
		// Caret is at right of frame
		scrollPt.h = caretPtBottom.h- (marginFrame.right - marginFrame.left);
		need_scroll = true;
	}

	// Now scroll to designated to-left point if needed
	if (need_scroll)
		ScrollPinnedImageTo(scrollPt.h, scrollPt.v, refresh);
}

#pragma mark -

// ---------------------------------------------------------------------------
//	¥ SetFontNum
// ---------------------------------------------------------------------------

void
CTextWidget::SetFontNum(short inNum)
{
	Str255 fName;
	::GetFontName(inNum, fName);
	SetFontName(fName);
}

// ---------------------------------------------------------------------------
//	¥ SetFontName
// ---------------------------------------------------------------------------

void
CTextWidget::SetFontName(
	ConstStringPtr	inName)
{
	ATSUFontID fontID;
	if (::ATSUFindFontFromName((Ptr) (inName + 1), inName[0], kFontFullName, kFontMacintoshPlatform, kFontRomanScript, kFontNoLanguage, &fontID) != noErr)
		return;

	// Loop over style runs
	bool changed = false;
	UniCharArrayOffset start = mSelection.caret;
	UniCharCount length = mSelection.length;

	while(length > 0)
	{
		// Get style run for start char
		ATSUStyle style;
		UniCharArrayOffset runStart;
		UniCharCount runLength;
		if (::ATSUGetRunStyle(mTextLayout, start, &style, &runStart, &runLength) != noErr)
			return;
		
		// Get minimum length of change within this run
		UniCharCount applyLength = std::min(runLength - (start - runStart), length);
		
		// Now create new style
		CATSUIStyle newStyle;
		::ATSUCopyAttributes(style, newStyle.Get());
		
		// Only change if different
		if (newStyle.GetFontID() != fontID)
		{
			newStyle.SetATSUFont(fontID);
			
			// Apply style over run range
			try
			{
				mTextLayout.SetRunStyle(newStyle, start, applyLength);
			}
			catch(...)
			{
				CLOG_LOGCATCH(...);
				
				// Force end of loop
				applyLength = length;
			}
			
			changed = true;
		}
		
		// Adjust apply lengths
		start += applyLength;
		length -= applyLength;
	}

	// Apply to default font if nothing present
	if ((mSelection.length == 0) || (mTextPtr == NULL))
		mStyle.SetATSUFont(fontID);
	
	if (mSelection.caret == mTextLength)
	{
		CreatePendingStyle();
		mPendingStyle->SetATSUFont(fontID);
	}
	
	if (changed)
		AdjustTextLayout();
}


// ---------------------------------------------------------------------------
//	¥ SetFontSize
// ---------------------------------------------------------------------------

void
CTextWidget::SetFontSize(
	Fixed	inSize)
{
	// Loop over style runs
	bool changed = false;
	UniCharArrayOffset start = mSelection.caret;
	UniCharCount length = mSelection.length;

	while(length > 0)
	{
		// Get style run for start char
		ATSUStyle style;
		UniCharArrayOffset runStart;
		UniCharCount runLength;
		if (::ATSUGetRunStyle(mTextLayout, start, &style, &runStart, &runLength) != noErr)
			return;
		
		// Get minimum length of change within this run
		UniCharCount applyLength = std::min(runLength - (start - runStart), length);
		
		// Now create new style
		CATSUIStyle newStyle;
		::ATSUCopyAttributes(style, newStyle.Get());

		// Only change if different
		if (newStyle.GetFontSize() != inSize)
		{
			newStyle.SetFontSize(inSize);
			
			// Apply style over run range
			try
			{
				mTextLayout.SetRunStyle(newStyle, start, applyLength);
			}
			catch(...)
			{
				CLOG_LOGCATCH(...);
				
				// Foece end of loop
				applyLength = length;
			}
			
			changed = true;
		}

		// Adjust apply lengths
		start += applyLength;
		length -= applyLength;
	}
	
	// Apply to default font if nothing present
	if ((mSelection.length == 0) || (mTextPtr == NULL))
		mStyle.SetFontSize(inSize);
	
	if (mSelection.caret == mTextLength)
	{
		CreatePendingStyle();
		mPendingStyle->SetFontSize(inSize);
	}
	
	if (changed)
		AdjustTextLayout();
}


// ---------------------------------------------------------------------------
//	¥ SetFontStyle
// ---------------------------------------------------------------------------

void CTextWidget::SetFontStyle(Style inStyle, bool add)
{
	// Loop over style runs
	UniCharArrayOffset start = mSelection.caret;
	UniCharCount length = mSelection.length;

	while(length > 0)
	{
		// Get style run for start char
		ATSUStyle style;
		UniCharArrayOffset runStart;
		UniCharCount runLength;
		if (::ATSUGetRunStyle(mTextLayout, start, &style, &runStart, &runLength) != noErr)
			return;
		
		// Get minimum length of change within this run
		UniCharCount applyLength = std::min(runLength - (start - runStart), length);
		
		// Now create new style
		CATSUIStyle newStyle;
		::ATSUCopyAttributes(style, newStyle.Get());
		
		// Create merged style
		if (inStyle == normal)
		{
			newStyle.SetBoldface(false);
			newStyle.SetItalic(false);
			newStyle.SetUnderline(false);
		}
		else
		{
			if ((inStyle & bold) != 0)
				newStyle.SetBoldface(add);
			if ((inStyle & italic) != 0)
				newStyle.SetItalic(add);
			if ((inStyle & underline) != 0)
				newStyle.SetUnderline(add);
		}
		
		// Apply style over run range
		try
		{
			mTextLayout.SetRunStyle(newStyle, start, applyLength);
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
			
			// Force end of loop
			applyLength = length;
		}
		
		// Adjust apply lengths
		start += applyLength;
		length -= applyLength;
	}
	
	// Apply to default font if nothing present
	if ((mSelection.length == 0) || (mTextPtr == NULL))
	{
		// Create merged style
		if (inStyle == normal)
		{
			mStyle.SetBoldface(false);
			mStyle.SetItalic(false);
			mStyle.SetUnderline(false);
		}
		else
		{
			if ((inStyle & bold) != 0)
				mStyle.SetBoldface(add);
			if ((inStyle & italic) != 0)
				mStyle.SetItalic(add);
			if ((inStyle & underline) != 0)
				mStyle.SetUnderline(add);
		}
	}
	
	
	if (mSelection.caret == mTextLength)
	{
		CreatePendingStyle();
		
		// Create merged style
		if (inStyle == normal)
		{
			mPendingStyle->SetBoldface(false);
			mPendingStyle->SetItalic(false);
			mPendingStyle->SetUnderline(false);
		}
		else
		{
			if ((inStyle & bold) != 0)
				mPendingStyle->SetBoldface(add);
			if ((inStyle & italic) != 0)
				mPendingStyle->SetItalic(add);
			if ((inStyle & underline) != 0)
				mPendingStyle->SetUnderline(add);
		}
	}

	AdjustTextLayout();
}

void CTextWidget::ModifyFontInfo( SInt32 selStart, SInt32 selEnd, Style inStyle, bool add, const RGBColor&	inColor )
{
}

// ---------------------------------------------------------------------------
//	¥ SetFontColor
// ---------------------------------------------------------------------------

void
CTextWidget::SetFontColor(
	const RGBColor&	inColor)
{
	// Loop over style runs
	bool changed = false;
	UniCharArrayOffset start = mSelection.caret;
	UniCharCount length = mSelection.length;

	while(length > 0)
	{
		// Get style run for start char
		ATSUStyle style;
		UniCharArrayOffset runStart;
		UniCharCount runLength;
		if (::ATSUGetRunStyle(mTextLayout, start, &style, &runStart, &runLength) != noErr)
			return;
		
		// Get minimum length of change within this run
		UniCharCount applyLength = std::min(runLength - (start - runStart), length);
		
		// Now create new style
		CATSUIStyle newStyle;
		::ATSUCopyAttributes(style, newStyle.Get());

		// Only change if different
		if ((newStyle.GetColor().red != inColor.red) ||
			(newStyle.GetColor().green != inColor.green) ||
			(newStyle.GetColor().blue != inColor.blue))
		{
			newStyle.SetColor(inColor);
			
			// Apply style over run range
			try
			{
				mTextLayout.SetRunStyle(newStyle, start, applyLength);
			}
			catch(...)
			{
				CLOG_LOGCATCH(...);
				
				// Force end of loop
				applyLength = length;
			}
			
			changed = true;
		}
		
		// Adjust apply lengths
		start += applyLength;
		length -= applyLength;
	}
	
	// Apply to default font if nothing present
	if ((mSelection.length == 0) || (mTextPtr == NULL))
		mStyle.SetColor(inColor);
	
	
	if (mSelection.caret == mTextLength)
	{
		CreatePendingStyle();
		mPendingStyle->SetColor(inColor);
	}
	
	if (changed)
	{
		AdjustTextLayout();
		Refresh();
	}
}

void CTextWidget::SetSpellHighlight(UniCharArrayOffset sel_start, UniCharArrayOffset sel_end, bool isSpelling)
{
	// Loop over style runs
	UniCharArrayOffset start = sel_start;
	UniCharCount length = sel_end - sel_start;
	//bool once = true;

	while(length > 0)
	{
		// Get style run for start char
		ATSUStyle style;
		UniCharArrayOffset runStart;
		UniCharCount runLength;
		if (::ATSUGetRunStyle(mTextLayout, start, &style, &runStart, &runLength) != noErr)
			return;
		
		// Get minimum length of change within this run
		UniCharCount applyLength = std::min(runLength - (start - runStart), length);
		
		// Now create new style
		CATSUIStyle newStyle;
		::ATSUCopyAttributes(style, newStyle.Get());
		newStyle.SetSpelling(isSpelling);
		
		// Apply style over run range
		try
		{
			mTextLayout.SetRunStyle(newStyle, start, applyLength);
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
			
			// Force end of loop
			applyLength = length;
		}
		
		// Adjust apply lengths
		start += applyLength;
		length -= applyLength;
	}
	Refresh();
}

void CTextWidget::SetAlignment(int align)
{
	Fract value;
	switch(align)
	{
	case teJustLeft:
		value = kATSUStartAlignment;
		break;
	case teJustCenter:
		value = kATSUCenterAlignment;
		break;
	case teJustRight:
		value = kATSUEndAlignment;
		break;
	}
	try
	{
		mTextLayout.SetOneLayoutControl(kATSULineFlushFactorTag, sizeof(Fract), &value);
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}
}

void CTextWidget::ClearUndo()
{
	if (mUndoer != NULL)
	{
		mUndoer->ClearActions();
		SetUpdateCommandStatus(true);
	}
}

#pragma mark -

unsigned long CTextWidget::GetLineCount() const
{
	return mLineCount;
}

long CTextWidget::GetTextLength() const
{
	return mTextLength;
}

unsigned long CTextWidget::GetLineHeight(unsigned long line) const
{
	if (line < mLineCount)
		return mLineHeights[line];
	else
		return 0;
}

unsigned long CTextWidget::GetLineOffset(unsigned long line) const
{
	if (line < mLineCount)
		return mLineOffsets[line];
	else
		return 0;
}

unsigned long CTextWidget::GetFullWidth() const
{
	unsigned long result = 0;

	// Loop over each line getting its width
	for(ItemCount i = 0; i < mLineCount; i++)
	{
		UniCharArrayOffset lineStart = (i == 0) ? 0 : mLineBreaks[i - 1];
		UniCharCount lineLength = mLineBreaks[i] - lineStart;

		ATSUTextMeasurement oTextBefore;
		ATSUTextMeasurement oTextAfter;
		ATSUTextMeasurement oAscent;
		ATSUTextMeasurement oDescent;

		if (::ATSUGetUnjustifiedBounds(mTextLayout, lineStart, lineLength, &oTextBefore, &oTextAfter, &oAscent, &oDescent) == noErr)
		{
			result = std::max(result, (unsigned long)FixedToInt(oTextAfter - oTextBefore));
		}
	}
	
	return result;
}

unsigned long CTextWidget::GetFullHeight() const
{
	if (mLineCount != 0)
		return mLineOffsets[mLineCount - 1] + mLineHeights[mLineCount - 1];
	else
		return 0;
}

Point CTextWidget::GetPoint(SInt32 offset) const
{
	return OffsetToPoint((UniCharArrayOffset)offset);
}

SInt32 CTextWidget::GetOffset(Point localpt) const
{
	bool leading;
	return PointToOffset(localpt, leading);
}

void CTextWidget::GetSelection(SInt32* selStart, SInt32* selEnd) const
{
	if (selStart != NULL)
		*selStart = mSelection.caret;
	if (selEnd != NULL)
		*selEnd = mSelection.caret + mSelection.length;
}

void CTextWidget::SetSelection(SInt32 selStart, SInt32 selEnd)
{
	ModifySelection(selStart, selEnd);
}

void CTextWidget::MoveSelection(SInt32 selStart, SInt32 selEnd)
{
	ModifySelection(selStart, selEnd);
	ShowSelection(true);
}

// See if point over selection
Boolean CTextWidget::PointOverSelection(Point localPt)
{
	if (mSelection.length == 0)
		return false;

	bool leading;
	UniCharArrayOffset offset = PointToOffset(localPt, leading);
	return (offset >= mSelection.caret) && (offset <= mSelection.caret + mSelection.length);
}

void CTextWidget::GetHiliteRgn(RgnHandle rgn)
{
	Rect	marginFrame;
	CalcMarginFrameRect(marginFrame);

	// Get local co-ords of each selected line that is visible
	for (ItemCount i = 0; i < mSelection.numberOfSelectedLines; i++)
	{
		SSelectedBlock *sb = &(mSelection.selBlocks[i]);

		SRect32 imageRect;
		
		// Top/bottom determined by line offset and height
		imageRect.top = mLineOffsets[sb->lineNumber];
		imageRect.bottom = mLineOffsets[sb->lineNumber] + mLineHeights[sb->lineNumber];
		
		// Left/right determined by selection range
		SPoint32 left_char = OffsetToImagePoint(sb->selStart);
		SPoint32 right_char = OffsetToImagePoint(sb->selStart + sb->selLength);
		imageRect.left = left_char.h;
		imageRect.right = right_char.h;

		// Convert to local co-ords
		SRect32 localRect = ImageToMarginRect32(imageRect);

		// Clip to frame
		Rect visible;
		visible.top = std::max((SInt32)marginFrame.top, localRect.top);
		visible.left = std::max((SInt32)marginFrame.left, localRect.left);
		visible.bottom = std::min((SInt32)marginFrame.bottom, localRect.bottom);
		visible.right = std::min((SInt32)marginFrame.right, localRect.right);

		StRegion visibleRgn(visible);

		// Add to region
		::UnionRgn(rgn, visibleRgn, rgn);
	}
}

#pragma mark -

void CTextWidget::DoCut()
{
	if (not IsReadOnly())
	{
		PostAction( new CTWCutAction(this, this));
		ShowSelection(true);
	}
}

void CTextWidget::DoCutAction()
{
	if (not IsReadOnly())
	{
		FocusDraw();

		// First copy selection to clipboard
		DoCopy();
		
		// Delete the selected text
		DeleteText(mSelection.caret, mSelection.length);
		Refresh();
		UserChangedText();
	}
}

void CTextWidget::DoCopy()
{
	DoCopyAction();
}

void CTextWidget::DoCopyAction()
{
	if (IsSelectable() && (mSelection.length != 0) && !mPasswordMode)
	{
		FocusDraw();
		
		// Plain unicode text
		StHandleBlock utxt(GetTextRangeAs_utxt(mSelection.caret, mSelection.length));
		UScrap::SetData(kScrapFlavorTypeUnicode, utxt);
		
		// Add unicode styles if needed
		if (IsStyled())
		{
			StHandleBlock ustl(GetStyleRangeAs_ustl(mSelection.caret, mSelection.length));
			if (ustl.Get() != NULL)
				UScrap::SetData(kScrapFlavorTypeUnicodeStyle, ustl, false);
		}

		// Plain text in System encoding
		StHandleBlock text(GetTextRangeAs_TEXT(utxt));
		UScrap::SetData(kScrapFlavorTypeText, text, false);
	}
}

// Text conversion
Handle CTextWidget::GetTextRangeAs_utxt(UniCharArrayOffset start, UniCharCount length) const
{
	StHandleBlock utxt(length * sizeof(UniChar));
	{
		StHandleLocker lock(utxt);
		::memcpy(*utxt, mTextPtr + start, length * sizeof(UniChar));
	}
	return utxt.Release();
}

// Text conversion
Handle CTextWidget::GetTextRangeAs_TEXT(UniCharArrayOffset start, UniCharCount length) const
{
	StHandleBlock utxt(GetTextRangeAs_utxt(start, length));
	return GetTextRangeAs_TEXT(utxt);
}

// Text conversion
Handle CTextWidget::GetTextRangeAs_TEXT(Handle utxt) const
{
	StHandleBlock ptxt(0L);
	
	SInt8 iTextState = ::HGetState( utxt );
	SInt8 oTextState = ::HGetState( ptxt );
	
	//UniCharCount ucCount = 0;
	TextEncoding ucs2Encoding = ::CreateTextEncoding( kTextEncodingUnicodeDefault, kTextEncodingDefaultVariant, kUnicode16BitFormat );
	TextEncoding systemEncoding = ::CreateTextEncoding( kTextEncodingMacRoman, kTextEncodingDefaultVariant, kTextEncodingDefaultFormat );
	TECObjectRef encodingConverter;
	OSStatus returnValue = ::TECCreateConverter( &encodingConverter, ucs2Encoding, systemEncoding );
	if ( returnValue == noErr)
	{
		TextPtr outputBuffer;
		ByteCount inputBufferLength = ::GetHandleSize( utxt );
		TextPtr inputBuffer = TextPtr( *utxt );
		
		ByteCount outputBufferLength = 4 * inputBufferLength;	//	Worst case scenario;
																//	Allows for TEC bidi handling and surrogates
		ByteCount actualInputLength;
		ByteCount actualOutputLength;
		
		::MoveHHi( utxt );
		::HLock( utxt );
		::HUnlock( ptxt );
		::SetHandleSize( ptxt, outputBufferLength );
		::MoveHHi( ptxt );
		::HLock( ptxt );
		outputBuffer = TextPtr( *ptxt );
		returnValue = ::TECConvertText(	encodingConverter,
										inputBuffer, inputBufferLength, &actualInputLength,
										outputBuffer, outputBufferLength, &actualOutputLength );
		::HUnlock( ptxt );
		::SetHandleSize( ptxt, actualOutputLength );
		
		::TECDisposeConverter( encodingConverter );
	}
	
	::HSetState( ptxt, oTextState );
	::HSetState( utxt, iTextState );
	
	return ptxt.Release();
}

// Get style range
Handle CTextWidget::GetStyleRangeAs_ustl(UniCharArrayOffset start, UniCharCount length) const
{
	if (length == 0)
	{
		return NULL;
	}

	// Count the number of style runs in the text range
	unsigned long styleCount = 1;
	UniCharArrayOffset startPos = start;
	while(true)
	{
		// Get next run details
		ATSUStyle style;
		UniCharArrayOffset runStart;
		UniCharCount runLength;
		::ATSUGetRunStyle(mTextLayout, startPos, &style, &runStart, &runLength);
		if (runStart + runLength >= start + length)
			break;

		startPos = runStart + runLength;
		styleCount++;
	}
	
	// Allocate space for styles and run info
	ATSUStyleRunInfo* atsui_runs = new ATSUStyleRunInfo[styleCount];
	CATSUIStyle* atsui_cstyles = new CATSUIStyle[styleCount];
	ATSUStyle* atsui_styles = new ATSUStyle[styleCount];
	
	// Now loop over runs and add to array
	startPos = start;
	unsigned long index = 0;
	while(true)
	{
		// Get next run details
		ATSUStyle style;
		UniCharArrayOffset runStart;
		UniCharCount runLength;
		::ATSUGetRunStyle(mTextLayout, startPos, &style, &runStart, &runLength);
		
		// Add to array
		atsui_runs[index].runLength = runLength - (startPos - runStart);
		atsui_runs[index].styleObjectIndex = index;
		::ATSUCopyAttributes(style, atsui_cstyles[index]);
		atsui_styles[index] = atsui_cstyles[index];

		if (runStart + runLength >= start + length)
		{
			// Adjust the final run to truncate to the actual length being copied
			atsui_runs[index].runLength = start + length - startPos;
			break;
		}

		startPos = runStart + runLength;
		index++;
	}

	// Now get size of flattened styles data
	ByteCount bufferSize = 0;
	::ATSUFlattenStyleRunsToStream(kATSUDataStreamUnicodeStyledText, kATSUFlattenOptionNoOptionsMask, styleCount, atsui_runs, styleCount, atsui_styles, 0, NULL, &bufferSize);

	// Create handle for data
	StHandleBlock styles(bufferSize);
	StHandleLocker lock(styles);

	// Now get flattened styles
	::ATSUFlattenStyleRunsToStream(kATSUDataStreamUnicodeStyledText, kATSUFlattenOptionNoOptionsMask, styleCount, atsui_runs, styleCount, atsui_styles, bufferSize, *styles, NULL);
	
	delete[] atsui_styles;
	delete[] atsui_cstyles;
	delete[] atsui_runs;
	
	return styles.Release();
}

void CTextWidget::DoPaste()
{
	if (not IsReadOnly())
	{
		PostAction( new CTWPasteAction(this, this) );
		ShowSelection(true);
	}
}

void CTextWidget::DoPasteAction()
{
	if (not IsReadOnly())
	{
		FocusDraw();

		StHandleBlock txt(0L);

		// Try to get Unicode text first
		if (UScrap::GetData(kScrapFlavorTypeUnicode, txt))
		{
			StHandleLocker lock1(txt);
			UniCharCount insert_length = ::GetHandleSize(txt) / sizeof(UniChar);
			
			// Check for styles
			StHandleBlock styles(0L);
			bool got_scrap = mAllowStyles && (UScrap::GetData(kScrapFlavorTypeUnicodeStyle, txt) != 0);
			if (got_scrap)
			{
				StHandleLocker lock2(styles);
				InsertTextAndStyles((UniChar*)*txt, insert_length, styles, false);				
			}
			else
				InsertText((UniChar*)*txt, insert_length, false);
		}
		
		// Try plain text next
		else if (UScrap::GetData(kScrapFlavorTypeText, txt))
		{
			StHandleLocker lock(txt);
			std::auto_ptr<char> temp(new char[::GetHandleSize(txt) + 1]);
			::memmove(temp.get(), *txt, ::GetHandleSize(txt));
			temp.get()[::GetHandleSize(txt)] = 0;
			LCFString cfstr(temp.get());
			temp.reset();

			UniCharCount insert_length = ::CFStringGetLength(cfstr);
			std::auto_ptr<UniChar> tempu(new UniChar[insert_length]);
			::CFStringGetCharacters(cfstr, CFRangeMake(0, (CFIndex) insert_length), tempu.get());
			InsertText(tempu.get(), insert_length, false);
		}
	}
}

void CTextWidget::DoClear()
{
	if (not IsReadOnly())
	{
		PostAction( new CTWClearAction(this, this ));
		ShowSelection(true);
	}
}

void CTextWidget::DoClearAction()
{
	if (not IsReadOnly())
	{
		FocusDraw();

		// Delete the selected text
		DeleteText(mSelection.caret, mSelection.length);
		Refresh();
		UserChangedText();
	}
}

void CTextWidget::SelectAll()
{
	ModifySelection(0, mTextLength);
}

void CTextWidget::InsertText(const UniChar* text, UniCharCount length, bool undoable)
{
	InsertTextAndStyles(text, length, NULL, undoable);
}

void CTextWidget::InsertTextAndStyles(const UniChar* text, UniCharCount length, Handle styles, bool undoable)
{
	// If undoable and not read-only, then use undo action
	if (undoable && !IsReadOnly())
	{
		// Post the action without actual doing it
		PostAction(new CTWInsertAction(this, this, text, length));
		
		// Fall through and do the actual action
	}

	// Do regular behaviour
	
	// Delete existing selection if any
	if (mSelection.length != 0)
		DeleteText(mSelection.caret, mSelection.length);
	
	// Insert new text
	//UniCharArrayOffset insertPos = mSelection.caret;
	//UniCharCount insertLength = length;
	InsertTextAndStyles(text, length, styles, mSelection.caret);

	// Always do refresh and user change action
	Refresh();
	UserChangedText();
}

const int cCapacityMargin = 1024;
const int cCapacityMarginFactor = 10;

void CTextWidget::InsertText(const UniChar* text, UniCharCount length, UniCharArrayOffset offset)
{
	InsertTextAndStyles(text, length, NULL, offset);
}

void CTextWidget::InsertTextAndStyles(const UniChar* text, UniCharCount length, Handle styles, UniCharArrayOffset offset)
{
	// Ignore if empty
	if (length == 0)
		return;

	// Find the line containing the insert location
	ItemCount start_line = OffsetToLine(offset);
	ItemCount end_line = start_line;
	ItemCount hard_break_adjust = 0;
	
	// Locate the next hard break
	for(ItemCount i = start_line; i < mLineCount; i++)
	{
		end_line = i;
		if ((mLineBreaks[i] > 0) && ((mTextPtr[mLineBreaks[i] - 1] == '\r') || (mTextPtr[mLineBreaks[i] - 1] == '\n')))
		{
			hard_break_adjust = (i < mLineCount - 1) ? 1 : 0;
			break;
		}
	}

	bool at_end = (offset == mTextLength);

	// Make sure that sufficient capacity for the insert exists
	if (mTextCapacity - mTextLength < length)
	{
		// New more capacity
		NewCapacity(mTextLength + length + cCapacityMargin);
	}
	
	// Now do the insert
	
	// First move existing data to the right if any to move
	if (mTextLength - offset > 0)
	{
		::memmove(mTextPtr + offset + length, mTextPtr + offset, (mTextLength - offset) * sizeof(UniChar));
		if (mPasswordMode)
			::memmove(mHiddenPtr + offset + length, mHiddenPtr + offset, (mTextLength - offset) * sizeof(UniChar));	
	}
	
	// Now copy in the new text
	::memmove(mTextPtr + offset, text, length * sizeof(UniChar));
	if (mPasswordMode)
	{
		for(UniCharArrayOffset i = offset; i < offset + length; i++)
			mHiddenPtr[i] = 0x2022;	// Bullet character
	}

	mTextLength += length;

	// Now tell layput about insert
	::ATSUTextInserted(mTextLayout, offset, length);

	// Adjust cursor
	if (mSelection.caret >= offset)
	{
		mSelection.caret += length;
		mSelection.anchor += length;
		
		// Force selection update
		SetSelectedBlocks();
	}

	// Do special style adjustment for end of text
	if ((mPendingStyle != NULL) && at_end)
	{
		try
		{
			mTextLayout.SetRunStyle(*mPendingStyle, offset, length);
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
		}
		delete mPendingStyle;
		mPendingStyle = NULL;
	}

	// If redraw is off, mark an adjust as pending
	if (!mRedrawOn)
	{
		mAdjustPending = true;
		return;
	}

	// Smart adjust line breaks

	// Bump up all the breaks from this line on
	for(ItemCount i = start_line; i < mLineCount; i++)
	{
		mLineBreaks[i] += length;
	}
	
	// Bump down start line as insertion of space may cause previous word to shift to previous line
	if (start_line > 0)
		start_line--;

	// Determine range of text to break
	UniCharArrayOffset start_char = (start_line == 0) ? 0 : mLineBreaks[start_line - 1];
	UniCharArrayOffset end_char = (mLineCount != 0) ? mLineBreaks[end_line] : mTextLength;

	// Get line length
	SDimension32 imageSize;
	GetImageSize(imageSize);
	try
	{
		mTextLayout.SetLineWidth(IntToFixed(imageSize.width));
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}

	// Now batch break the range of text from the insert line to the next hard break (determined prior to insert)
	ItemCount breakCount;
	/* OSStatus err = */ ::ATSUBatchBreakLines(mTextLayout, start_char, end_char - start_char, IntToFixed(imageSize.width), &breakCount);
	
	// Check to see if the number of lines has changed
	ItemCount lines_added = breakCount - (end_line - start_line + hard_break_adjust);
	ItemCount lines_added_adjust = lines_added;
	if (mLineCount == 0)
		lines_added_adjust++;

	// Create space for new line info
	if (lines_added_adjust > 0)
	{
		mLineBreaks.reserve(mLineCount + lines_added_adjust);
		mLineBreaks.insert(mLineBreaks.begin() + end_line, lines_added_adjust, 0);
		mLineHeights.reserve(mLineCount + lines_added_adjust);
		mLineHeights.insert(mLineHeights.begin() + end_line, lines_added_adjust, 0);
		mLineDescents.reserve(mLineCount + lines_added_adjust);
		mLineDescents.insert(mLineDescents.begin() + end_line, lines_added_adjust, 0);
		mLineOffsets.reserve(mLineCount + lines_added_adjust);
		mLineOffsets.insert(mLineOffsets.begin() + end_line, lines_added_adjust, 0);
	}
	
	// Now adjust actual line count
	mLineCount += lines_added_adjust;
	
	// Get new set of line breaks
	GetBreakInfo(start_char, end_char, start_line, end_line + lines_added, mLineBreaks);
	
	// Get new set of line details
	GetLineInfo(start_line, end_line + lines_added, mLineBreaks, mLineHeights, mLineDescents, mLineOffsets);

	// Redo line offsets of remaining lines
	for(ItemCount i = end_line + lines_added; i < mLineCount; i++)
	{
		mLineOffsets[i] = (i > 0) ? mLineOffsets[i - 1] + mLineHeights[i - 1] : 0;
	}
							  
	// Readjust cached selection info
	SetSelectedBlocks();
	
	// Resync image to new text height
	AdjustImageToText();
	
	// Now extract and apply atsui style info
	if (styles != NULL)
	{
		StHandleLocker	lock(styles);
		
		// First get size of arrays needed to hold info
		ByteCount styleSize = ::GetHandleSize(styles);
		ItemCount numberOfRuns = 0;
		ItemCount numberOfStyles = 0;
		::ATSUUnflattenStyleRunsFromStream(kATSUDataStreamUnicodeStyledText, kATSUUnFlattenOptionNoOptionsMask, styleSize, *styles, numberOfRuns, numberOfStyles, NULL, NULL, &numberOfRuns, &numberOfStyles);

		// Allocate arrays
		ATSUStyleRunInfo* atsui_runs = new ATSUStyleRunInfo[numberOfRuns];
		ATSUStyle* atsui_styles = new ATSUStyle[numberOfStyles];
		
		// Now get actual data
		::ATSUUnflattenStyleRunsFromStream(kATSUDataStreamUnicodeStyledText, kATSUUnFlattenOptionNoOptionsMask, styleSize, *styles, numberOfRuns, numberOfStyles, atsui_runs, atsui_styles, NULL, NULL);
		
		// Now apply each run
		for(int i = 0; i < numberOfRuns; i++)
		{
			// Apply indexed style to run length
			::ATSUSetRunStyle(mTextLayout, atsui_styles[atsui_runs[i].styleObjectIndex], offset, atsui_runs[i].runLength);
			offset += atsui_runs[i].runLength;
		}

		delete[] atsui_styles;
		delete[] atsui_runs;

		AdjustTextLayout();
	}
}

void CTextWidget::DeleteText(UniCharArrayOffset offset, UniCharCount length)
{
	// Ignore if empty
	if (length == 0)
		return;

	// Find the line containing the insert location
	ItemCount start_line = OffsetToLine(offset);
	ItemCount end_line = OffsetToLine(offset + length);
	ItemCount hard_break_adjust = 0;
	
	// Bump down start line if delete will collapse a line
	//if ((offset > 0) && (mTextPtr[offset - 1] == '\r') & (start_line > 0))
	if (start_line > 0)
		start_line--;

	// Locate the next hard break after the text being deleted
	for(ItemCount i = end_line; i < mLineCount; i++)
	{
		end_line = i;
		if ((mLineBreaks[i] > 0) && ((mTextPtr[mLineBreaks[i] - 1] == '\r') || (mTextPtr[mLineBreaks[i] - 1] == '\n')))
		{
			hard_break_adjust = (i < mLineCount - 1) ? 1 : 0;
			break;
		}
	}

	// Make sure length is within text array bounds
	UniCharCount rmv_length = std::min(mTextLength - offset, length);
	UniCharCount move_length = mTextLength - offset - rmv_length;
	
	// Now memmove the data
	::memmove(mTextPtr + offset, mTextPtr + offset + rmv_length, move_length * sizeof(UniChar));
	if (mPasswordMode)
		::memmove(mHiddenPtr + offset, mHiddenPtr + offset + rmv_length, move_length * sizeof(UniChar));
	mTextLength -= rmv_length;
	
	// Now tell layoput about deletion
	::ATSUTextDeleted(mTextLayout, offset, rmv_length);

	// Reduce capacity if over the margin by factor of 10
	if (mTextCapacity - mTextLength > cCapacityMargin * cCapacityMarginFactor)
		NewCapacity(mTextLength + cCapacityMargin);
	
	// Adjust selection
	if (mSelection.caret >= offset)
	{
		// Remove part of selection that overlaps the deleted portion
		if (mSelection.caret < offset + length)
		{
			if (mSelection.caret + mSelection.length > offset + length)
				mSelection.length -= offset + length - mSelection.caret;
			else
				mSelection.length = 0;
			mSelection.anchor = mSelection.caret = offset + length;
		}

		// Now subtract off deleted text
		mSelection.anchor = mSelection.caret -= length;
		
		// Force selection update
		SetSelectedBlocks();
	}

	// If redraw is off, mark an adjust as pending
	if (!mRedrawOn)
	{
		mAdjustPending = true;
		return;
	}

	// Smart adjust line breaks

	// Bump down all the breaks from this line on
	for(ItemCount i = start_line; i < mLineCount; i++)
	{
		mLineBreaks[i] -= length;
	}
	
	// Determine range of text to break
	UniCharArrayOffset start_char = (start_line == 0) ? 0 : mLineBreaks[start_line - 1];
	UniCharArrayOffset end_char = (mLineCount != 0) ? mLineBreaks[end_line] : mTextLength;

	// Get line length
	SDimension32 imageSize;
	GetImageSize(imageSize);
	try
	{
		mTextLayout.SetLineWidth(IntToFixed(imageSize.width));
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}

	// Now batch break the range of text from the insert line to the next hard break (determined prior to insert)
	ItemCount breakCount = 0;
	if (end_char > start_char)
		::ATSUBatchBreakLines(mTextLayout, start_char, end_char - start_char, IntToFixed(imageSize.width), &breakCount);
	
	// Check to see if the number of lines has changed
	ItemCount lines_removed = (end_line - start_line + hard_break_adjust) - breakCount;
	if (mTextLength == 0)
		lines_removed = mLineCount;

	// Create space for new line info
	if (lines_removed > 0)
	{
		mLineBreaks.erase(mLineBreaks.begin() + start_line, mLineBreaks.begin() + start_line + lines_removed);
		mLineHeights.erase(mLineHeights.begin() + start_line, mLineHeights.begin() + start_line + lines_removed);
		mLineDescents.erase(mLineDescents.begin() + start_line, mLineDescents.begin() + start_line + lines_removed);
		mLineOffsets.erase(mLineOffsets.begin() + start_line, mLineOffsets.begin() + start_line + lines_removed);
	}
	
	// Get new set of line breaks
	if ((start_line != end_line) || (lines_removed == 0))
		GetBreakInfo(start_char, end_char, start_line, end_line - lines_removed, mLineBreaks);
	
	// Get new set of line details
	if ((start_line != end_line) || (lines_removed == 0))
		GetLineInfo(start_line, end_line - lines_removed, mLineBreaks, mLineHeights, mLineDescents, mLineOffsets);
	
	mLineCount -= lines_removed;

	// Redo line offsets of remaining lines
	for(ItemCount i = end_line - lines_removed; i < mLineCount; i++)
	{
		mLineOffsets[i] = (i > 0) ? mLineOffsets[i - 1] + mLineHeights[i - 1] : 0;
	}
							  
	// Readjust cached selection info
	SetSelectedBlocks();
	
	// Resync image to new text height
	AdjustImageToText();
}

// Increase/decrease the capacity of the unichar buffer to the value provide
void CTextWidget::NewCapacity(UniCharCount capacity)
{
	if (mTextCapacity != capacity)
	{
		UniChar* new_text_ptr = new UniChar[capacity];
		UniChar* new_hidden_ptr = mPasswordMode ? new UniChar[capacity] : NULL;
		if (mTextPtr != NULL)
		{
			// Copy existing text in
			::memcpy(new_text_ptr, mTextPtr, mTextLength * sizeof(UniChar));
			if (mPasswordMode)
				::memcpy(new_hidden_ptr, mHiddenPtr, mTextLength * sizeof(UniChar));
			
			delete[] mTextPtr;
			mTextPtr = new_text_ptr;

			delete[] mHiddenPtr;
			mHiddenPtr = new_hidden_ptr;

			mTextCapacity = capacity;
			
			::ATSUTextMoved(mTextLayout, GetTextPtr());
		}
		else
		{
			mTextPtr = new_text_ptr;
			mHiddenPtr = new_hidden_ptr;
			mTextLength = 0;
			mTextCapacity = capacity;
			try
			{
				mTextLayout.SetTextPointer(GetTextPtr(), 0, mTextLength, mTextLength);
				mTextLayout.SetRunStyle(mStyle, 0, mTextLength);
			}
			catch(...)
			{
				CLOG_LOGCATCH(...);
			}
			
		    // Turn on font substitution
			::ATSUSetTransientFontMatching(mTextLayout, true);
		}
	}
}

void CTextWidget::CreatePendingStyle()
{
	if (mPendingStyle == NULL)
	{
		// Create new style
		mPendingStyle = new LATSUIStyle;
		
		// Set style to that of last char or default style
		if (mTextLength != 0)
		{
			// Get style run for last char
			ATSUStyle style;
			UniCharArrayOffset runStart;
			UniCharCount runLength;
			if (::ATSUGetRunStyle(mTextLayout, mTextLength - 1, &style, &runStart, &runLength) == noErr)
			{
				::ATSUCopyAttributes(style, mPendingStyle->Get());
			}
		}
		else
		{
			::ATSUCopyAttributes(mStyle, mPendingStyle->Get());
		}
	}
}

// Assume the array size is setup properly
void CTextWidget::GetBreakInfo(UniCharArrayOffset start, UniCharArrayOffset end, ItemCount start_line, ItemCount end_line, std::vector<UniCharArrayOffset>& outLineBreaks)
{
	ItemCount breakCount = 0;
	/* OSStatus status = */ ::ATSUGetSoftLineBreaks( mTextLayout, start, end - start,
												   end_line - start_line + 1,
												   &outLineBreaks[start_line],
												   &breakCount );
	
	if (end_line < outLineBreaks.size())
		outLineBreaks[end_line] = end;
	
}

void CTextWidget::GetLineInfo(ItemCount start_line, ItemCount end_line,
							const std::vector<UniCharArrayOffset>& lineBreaks,
							std::vector<unsigned long>&	outLineHeights,
							std::vector<unsigned long>&	outLineDescents,
							std::vector<unsigned long>&	outLineOffsets)
{
	// Special case when there is no text
	if (mTextLength != 0)
	{
										// Measure each line of text, storing
										//   the height in our array
		UniCharArrayOffset	lineStart  = (start_line == 0) ? 0 : lineBreaks[start_line - 1];

		for (ItemCount i = start_line; i <= end_line; i++)
		{
			// Get line ascent and descent values
			ATSUTextMeasurement ascent, descent;
			ByteCount dummy;
			::ATSUGetLineControl(mTextLayout, lineStart, kATSULineAscentTag, sizeof (ATSUTextMeasurement), &ascent, &dummy); 
			::ATSUGetLineControl(mTextLayout, lineStart, kATSULineDescentTag, sizeof (ATSUTextMeasurement), &descent, &dummy); 

			// Cache the values we got
			outLineHeights[i] = FixedToInt(ascent + descent);
			outLineDescents[i] = FixedToInt(descent);
			outLineOffsets[i] = (i > 0) ? outLineOffsets[i - 1] + outLineHeights[i - 1] : 0;
			
			// Bump to next line
			lineStart = lineBreaks[i];
		}
	}
	else
	{
		// Do it the old way, which seems to work
		ATSUTextMeasurement		before;
		ATSUTextMeasurement		after;
		ATSUTextMeasurement		ascent;
		ATSUTextMeasurement		descent;
		
		::ATSUMeasureText( mTextLayout, 0,
										0,
										&before,
										&after,
										&ascent,
										&descent );

		// Cache the values we got
		outLineHeights[0] = FixedToInt(ascent + descent);
		outLineDescents[0] = FixedToInt(descent);
		outLineOffsets[0] = 0;
	}
}

#pragma mark -

// ---------------------------------------------------------------------------
//	¥ DetermineLineBreaksFast										  [public]
// ---------------------------------------------------------------------------
//	Set soft line breaks for the text using the specified line length

ItemCount
CTextWidget::CATSUITextLayout::DetermineLineBreaksFast(
	ATSUTextMeasurement		inLineWidth,
	UniCharCount inTextLength)
{
	try
	{
		SetLineWidth(inLineWidth);
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}

	ItemCount breakCount;
	if ((inTextLength != 0) && (::ATSUBatchBreakLines(mTextLayout, kATSUFromTextBeginning, inTextLength, inLineWidth, &breakCount) == noErr))
		return breakCount;
	else
		return 0;
}


// ---------------------------------------------------------------------------
//	¥ MeasureLinesMore
// ---------------------------------------------------------------------------
//	Pass back the number of lines, line break offsets, and line heights for
//	the text in a layout.
//
//	Function allocates outLineBreaks and outLineHeights as arrays using
//	new[]. Caller is resposible for deleting these arrays (remember to
//	use delete[] instead of delete).
//
//	You may pass the line breaks and heights to the DrawLinesAt() function
//	to draw the text.

void
CTextWidget::CATSUITextLayout::MeasureLinesMore(
		UniCharCount			inTextLength,
		ATSUTextMeasurement		inLineWidth,
		ItemCount&				outLineCount,
		std::vector<UniCharArrayOffset>&		outLineBreaks,
		std::vector<unsigned long>&			outLineHeights,
		std::vector<unsigned long>&			outLineDescents,
		std::vector<unsigned long>&			outLineOffsets)
{
	// Clear out existing data
	outLineBreaks.clear();
	outLineHeights.clear();
	outLineDescents.clear();
	outLineOffsets.clear();

	// If no text leave it empty
	if (inTextLength == 0)
	{
		outLineCount = 0;
		return;
	}

									// Set soft line breaks
	ItemCount breakCount = DetermineLineBreaksFast(inLineWidth, inTextLength);
	outLineCount = breakCount + 1;	// Lines of text is one more than the
									//   number of line breaks
	
	OSStatus	status;
							
									// Allocate and fill array of soft line
									//   break offsets
	outLineBreaks.insert(outLineBreaks.begin(), outLineCount, 0);
	
	status = ::ATSUGetSoftLineBreaks( mTextLayout, kATSUFromTextBeginning,
												   kATSUToTextEnd,
												   breakCount,
												   &outLineBreaks[0],
												   &breakCount );
	outLineBreaks[breakCount] = inTextLength;
	
									
									// Allocate array to store line heights
	outLineHeights.insert(outLineHeights.begin(), outLineCount, 0);
	outLineDescents.insert(outLineDescents.begin(), outLineCount, 0);
	outLineOffsets.insert(outLineOffsets.begin(), outLineCount, 0);
	
									// Measure each line of text, storing
									//   the height in our array
	UniCharArrayOffset	lineStart  = 0;
	
	// Special case when there is no text
	ATSUTextMeasurement ascent, descent;
	for (ItemCount i = 0; i <= breakCount; i++) {
	
		// Get line ascent and descent values
		// Optimise for large line count (> 500) by only using the first one
		if ((i == 0) || (breakCount < 500))
		{
			ByteCount dummy;
			::ATSUGetLineControl(mTextLayout, lineStart, kATSULineAscentTag, sizeof (ATSUTextMeasurement), &ascent, &dummy); 
			::ATSUGetLineControl(mTextLayout, lineStart, kATSULineDescentTag, sizeof (ATSUTextMeasurement), &descent, &dummy); 

			// Cache the values we got
			outLineHeights[i] = FixedToInt(ascent + descent);
			outLineDescents[i] = FixedToInt(descent);
			outLineOffsets[i] = (i > 0) ? outLineOffsets[i - 1] + outLineHeights[i - 1] : 0;
		}
		else
		{
			outLineHeights[i] = outLineHeights[0];
			outLineDescents[i] = outLineDescents[0];
			outLineOffsets[i] = outLineOffsets[i - 1] + outLineHeights[i - 1];
		}
		
		// Bump to next line
		lineStart = outLineBreaks[i];
	}
}

