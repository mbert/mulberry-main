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


// Source for CTextWidgetDragAndDrop class

#include "CTextWidgetDragAndDrop.h"

#include "CTextWidget.h"
#include "CTextWidgetDragAction.h"

#include <LCFString.h>
#include <LDragTask.h>
#include <UGAColorRamp.h>

// __________________________________________________________________________________________________
// C L A S S __ C T E X T D R A G A N D D R O P
// __________________________________________________________________________________________________

CTextWidget* CTextWidgetDragAndDrop::sDragFrom = NULL;
CTextWidget* CTextWidgetDragAndDrop::sDragTo = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTextWidgetDragAndDrop::CTextWidgetDragAndDrop(
	WindowPtr		inMacWindow,
	LPane*			inPane)
	: LDragAndDrop(inMacWindow, inPane)
{
	mTextWidget = NULL;
	::SetRect(&mHiliteInset, 1, 1, 1, 1);
	mHiliteBackground = UGAColorRamp::GetWhiteColor();
	mLastDragOffset = 0xFFFFFFFF;
	mLastDragLeading = false;
	mCanScroll = false;
	
	AddDropFlavor(kScrapFlavorTypeUnicode);
	AddDropFlavor(kScrapFlavorTypeUnicodeStyle);
	AddDropFlavor(kScrapFlavorTypeText);
}

// Default destructor
CTextWidgetDragAndDrop::~CTextWidgetDragAndDrop()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Create the drag
OSErr CTextWidgetDragAndDrop::CreateDragEvent(const SMouseDownEvent& inMouseDown)
{
	// Begin the drag task
	LDragTask theDragTask(inMouseDown.macEvent);

	// Add send data proc
	DragSendDataUPP dragSendDataPP = NewDragSendDataUPP(LDragAndDrop::HandleDragSendData);
	SetDragSendProc(theDragTask.GetDragReference(), dragSendDataPP, (LDropArea*) this);

	// Add selected text to drag
	AddDragText(&theDragTask);

	// Create drag region
	RgnHandle tempRgn = ::NewRgn();
	Point globalPoint = {0, 0};
	LocalToGlobal(&globalPoint);

	mTextWidget->GetHiliteRgn(theDragTask.GetDragRegion());

	::CopyRgn(theDragTask.GetDragRegion(), tempRgn);
	::InsetRgn(tempRgn, 1, 1);
	::DiffRgn(theDragTask.GetDragRegion(), tempRgn, theDragTask.GetDragRegion());
	::OffsetRgn(theDragTask.GetDragRegion(), globalPoint.h, globalPoint.v);
    ::DisposeRgn(tempRgn);

	// Zero drag cursor
	mLastDragOffset = 0xFFFFFFFF;
	mLastDragLeading = false;

	// Do the drag
	StValueChanger<CTextWidget*> _change1(sDragFrom, mTextWidget);
	sDragTo = NULL;
	OSErr err = theDragTask.DoDrag();

	// Dispose of send data
	DisposeDragSendDataUPP(dragSendDataPP);

	// Look for delete of original if dragged outside of this view
	if ((err == noErr) && (sDragFrom != sDragTo))
	{
		short mouseUpModifiers;
		::GetDragModifiers(theDragTask.GetDragReference(), NULL, NULL, &mouseUpModifiers);
		if ((mouseUpModifiers & optionKey) && (!mTextWidget->IsReadOnly()))
		{
			mTextWidget->DoClear();
		}
	}

	return err;
}

// Add text to drag
void CTextWidgetDragAndDrop::AddDragText(LDragTask* theDragTask)
{
	// Dummy drag rect
	Rect 		dragRect = {0, 0, 0, 0};

	// Promise text to anyone who wants it
	::AddDragItemFlavor(theDragTask->GetDragReference(), (ItemReference) mTextWidget, kScrapFlavorTypeUnicode, 0L, 0L, 0);
	if (mTextWidget->IsStyled())
		::AddDragItemFlavor(theDragTask->GetDragReference(), (ItemReference) mTextWidget, kScrapFlavorTypeUnicodeStyle, 0L, 0L, 0);
	::AddDragItemFlavor(theDragTask->GetDragReference(), (ItemReference) mTextWidget, kScrapFlavorTypeText, 0L, 0L, 0);
}

// Check whether drag item is acceptable
Boolean CTextWidgetDragAndDrop::ItemIsAcceptable(
	DragReference	inDragRef,
	ItemReference	inItemRef)
{
	// Do no accept if read-only
	if (mTextWidget->IsReadOnly())
		return false;

	// Check for mDropFlavors
	FlavorFlags		theFlags;
	LArrayIterator	iterator(mDropFlavors, LArrayIterator::from_Start);
	FlavorType		theFlavor;

	while (iterator.Next(&theFlavor)) {
		if (::GetFlavorFlags(inDragRef, inItemRef, theFlavor, &theFlags) == noErr)
			return true;
	}
	
	return false;
}

// Check to see whether drop from another window
Boolean CTextWidgetDragAndDrop::CheckIfViewIsAlsoSender(DragReference inDragRef)
{
	// Get drag attributes
	DragAttributes theDragAttributes;
	::GetDragAttributes(inDragRef, &theDragAttributes);

	// Compare with relevant flag
	return (theDragAttributes & kDragInsideSenderWindow) && sDragFrom && (sDragFrom == mTextWidget);

}

// Handle multiple text items
void CTextWidgetDragAndDrop::DoDragReceive(DragReference inDragRef)
{
	DragAttributes	dragAttrs;
	::GetDragAttributes(inDragRef, &dragAttrs);

	// Do not allow drag back into own selection
	if (CheckIfViewIsAlsoSender(inDragRef))
	{
		if ((mLastDragOffset >= mTextWidget->mSelection.caret) &&
			(mLastDragOffset <= mTextWidget->mSelection.caret + mTextWidget->mSelection.length))
			Throw_(dragNotAcceptedErr);

	}

	// We need to know who gets the drop
	sDragTo = mTextWidget;

	UInt16	itemCount;				// Number of Items in Drag
	::CountDragItems(inDragRef, &itemCount);
	Size total_utxt = 0;
	Size total_ustl = 0;
	Size total_text = 0;

	// Calculate size of all text elements + 1 for each CR
	for (UInt16 item = 1; item <= itemCount; item++)
	{
		ItemReference	itemRef;
		FlavorFlags		theFlags;		// We actually only use the flags to see if a flavor exists
		::GetDragItemReferenceNumber(inDragRef, item, &itemRef);

		// Look for 'utxt' items
		if (::GetFlavorFlags(inDragRef, itemRef, kScrapFlavorTypeUnicode, &theFlags) == noErr)
		{
			Size theDataSize;

			// Calculate size
			ThrowIfOSErr_(::GetFlavorDataSize(inDragRef, itemRef, kScrapFlavorTypeUnicode, &theDataSize));

			// Increment size (remember CR if needed)
			total_utxt += theDataSize;
			if (item > 1)
				total_utxt += sizeof(UniChar);
			
			// Look for 'ustl' items
			if (mTextWidget->IsStyled())
			{
				if (::GetFlavorFlags(inDragRef, itemRef, kScrapFlavorTypeUnicodeStyle, &theFlags) == noErr)
				{
					Size theDataSize;

					// Calculate size
					ThrowIfOSErr_(::GetFlavorDataSize(inDragRef, itemRef, kScrapFlavorTypeUnicodeStyle, &theDataSize));

					// Increment size (remember CR if needed)
					total_ustl += theDataSize;
				}
			}
		}
		// Look for 'TEXT' items
		else if (::GetFlavorFlags(inDragRef, itemRef, kScrapFlavorTypeText, &theFlags) == noErr)
		{
			Size theDataSize;

			// Calculate size
			ThrowIfOSErr_(::GetFlavorDataSize(inDragRef, itemRef, kScrapFlavorTypeText, &theDataSize));

			// Increment size (remember CR if needed)
			total_text += theDataSize;
			if (item > 1)
				total_text += sizeof(char);
		}
	}

	// If something there then get it
	if (total_utxt || total_text)
	{
		// Setup actual totals for type of text
		Size total = (total_utxt != 0) ? total_utxt : total_text;

		StHandleBlock txt(total);
		StHandleBlock styles((total_utxt != 0) ? total_ustl : 0);
		{
			StHandleLocker _lock1(txt);
			StHandleLocker _lock2(styles);
			char* p = *txt;

			// Remove frame highlight
			if (mIsHilited)
			{
				UnhiliteDropArea(inDragRef);
				mIsHilited = false;
			}

			// Get all text elements + CR
			for (UInt16 item = 1; item <= itemCount; item++)
			{
				ItemReference	itemRef;
				FlavorFlags		theFlags;		// We actually only use the flags to see if a flavor exists
				::GetDragItemReferenceNumber(inDragRef, item, &itemRef);

				// Handle 'utxt' items
				if (total_utxt != 0)
				{
					if (::GetFlavorFlags(inDragRef, itemRef, kScrapFlavorTypeUnicode, &theFlags) == noErr)
					{
						Size theDataSize;

						// Calculate size
						ThrowIfOSErr_(::GetFlavorDataSize(inDragRef, itemRef, kScrapFlavorTypeUnicode, &theDataSize));

						if (theDataSize)
						{
							// Add CR if more than one
							if (item > 1)
							{
								*((UniChar*)p) = '\r';
								p += sizeof(UniChar);
							}

							// Get the text we are receiving.
							::GetFlavorData(inDragRef, itemRef, kScrapFlavorTypeUnicode, p, &theDataSize, 0L);

							p += theDataSize;
						}
					}
					if (total_ustl != 0)
					{
						if (::GetFlavorFlags(inDragRef, itemRef, kScrapFlavorTypeUnicodeStyle, &theFlags) == noErr)
						{
							Size theDataSize;

							// Calculate size
							ThrowIfOSErr_(::GetFlavorDataSize(inDragRef, itemRef, kScrapFlavorTypeUnicodeStyle, &theDataSize));

							if (theDataSize)
							{
								// Get the styles we are receiving.
								::GetFlavorData(inDragRef, itemRef, kScrapFlavorTypeUnicodeStyle, *styles, &theDataSize, 0L);
							}
						}
					}
				}
				else
				{
					if (::GetFlavorFlags(inDragRef, itemRef, kScrapFlavorTypeText, &theFlags) == noErr)
					{
						Size theDataSize;

						// Calculate size
						ThrowIfOSErr_(::GetFlavorDataSize(inDragRef, itemRef, kScrapFlavorTypeText, &theDataSize));

						if (theDataSize)
						{
							// Add CR if more than one
							if (item > 1)
								*p++ = '\r';

							// Get the text we are receiving.
							::GetFlavorData(inDragRef, itemRef, kScrapFlavorTypeText, p, &theDataSize, 0L);

							p += theDataSize;
						}
					}
				}
			}
		}

		// Remove cursor from last location before inserts
		if (mLastDragOffset != 0xFFFFFFFF)
			DrawDragInsert(mLastDragOffset, mLastDragLeading);

		// If not option key and same view then delete original
		short mods;
		::GetDragModifiers(inDragRef, NULL, NULL, &mods);

		if (total_utxt)
			CreateReceiveAction(mTextWidget, txt.Release(), total_utxt / sizeof(UniChar), (total_ustl != 0) ? styles.Release() : NULL, CheckIfViewIsAlsoSender(inDragRef) && !(mods & optionKey));
		else
		{
			// Convert to unicode text
			cdstring sysstr(*txt, ::GetHandleSize(txt));
			LCFString cfstr(sysstr.c_str());

			UniCharCount insert_length = ::CFStringGetLength(cfstr);
			StHandleBlock utxt(insert_length * sizeof(UniChar));
			{
				StHandleLocker lock(utxt);
				::CFStringGetCharacters(cfstr, CFRangeMake(0, (CFIndex) insert_length), (UniChar*)*utxt);
			}
			CreateReceiveAction(mTextWidget, utxt.Release(), insert_length, NULL, CheckIfViewIsAlsoSender(inDragRef) && !(mods & optionKey));			
		}
	}

	// Do not redraw cursor when leaving
	mLastDragOffset = 0xFFFFFFFF;
	mLastDragLeading = false;
}

// Post required action
void CTextWidgetDragAndDrop::CreateReceiveAction(LCommander* cmdr,
												Handle theFlavorData,
												Size theDataSize,
												Handle theStyles,
												Boolean delete_orig)
{
	cmdr->PostAction(new CTextWidgetDragAction(mTextWidget, cmdr, theFlavorData, theDataSize, theStyles, mLastDragOffset, delete_orig));
}

// Do correct unhiliting of drop region
void CTextWidgetDragAndDrop::EnterDropArea(
	DragReference	inDragRef,
	Boolean			inDragHasLeftSender)
{
	// Reset cursor
	mLastDragOffset = 0xFFFFFFFF;
	mLastDragLeading = false;

	// Do inherited
	LDragAndDrop::EnterDropArea(inDragRef, inDragHasLeftSender);

	// Set appropriate cursor
	mCopyCursor = false;
}

// Hilite/unhilite region two pixels wide round border
void CTextWidgetDragAndDrop::HiliteDropArea(
	DragReference	inDragRef)
{
	StColorState	saveColor;
	Rect	dropRect;

	// Get frame of scroller
	mPane->CalcLocalFrameRect(dropRect);

	// Inset as specified
	dropRect.left += mHiliteInset.left;
	dropRect.top += mHiliteInset.top;
	dropRect.right -= mHiliteInset.right;
	dropRect.bottom -= mHiliteInset.bottom;

	// Create drag region
	RgnHandle	dropRgn = ::NewRgn();
	::RectRgn(dropRgn, &dropRect);

	// Set correct background and do hilite
	::RGBBackColor(&mHiliteBackground);
	::ShowDragHilite(inDragRef, dropRgn, true);

	// Dispose drag region
	::DisposeRgn(dropRgn);
}

// Hilite/unhilite region two pixels wide round border
void CTextWidgetDragAndDrop::UnhiliteDropArea(
	DragReference	inDragRef)
{
	StColorState	saveColor;

	FocusDropArea();

	// Set correct background and do hilite
	::RGBBackColor(&mHiliteBackground);
	::HideDragHilite(inDragRef);
}

// Do correct unhiliting of drop region
void CTextWidgetDragAndDrop::LeaveDropArea(
	DragReference	inDragRef)
{
	// Remove cursor and reset
	if (mLastDragOffset != 0xFFFFFFFF) {
		DrawDragInsert(mLastDragOffset, mLastDragLeading);
		mLastDragOffset = 0xFFFFFFFF;
		mLastDragLeading = false;
	}

	// Remove frame highlight
	if (mIsHilited) {
		UnhiliteDropArea(inDragRef);
		mIsHilited = false;
	}

	mCanAcceptCurrentDrag = false;
}

// Handle drag into text
void CTextWidgetDragAndDrop::InsideDropArea(DragReference inDragRef)
{
	// Get global mouse pos from Drag manager
	Point	mouseLoc;
	::GetDragMouse(inDragRef, NULL, &mouseLoc);

	// Convert to a local point
	mTextWidget->GlobalToPortPoint(mouseLoc);
	mTextWidget->PortToLocalPoint(mouseLoc);

	// Get height of current line
	bool dummy;
	UniCharArrayOffset theOffset = mTextWidget->PointToOffset(mouseLoc, dummy);

	// Check for change
	if (theOffset != mLastDragOffset)
	{
		// Remove cursor from last location
		if (mLastDragOffset != 0xFFFFFFFF)
			DrawDragInsert(mLastDragOffset, mLastDragLeading);

		if (!dummy && (theOffset > 0))
		{
			theOffset--;
			dummy = true;
		}

		// Draw new cursor
		DrawDragInsert(theOffset, dummy);
		mLastDragOffset = theOffset;
		mLastDragLeading = dummy;
	}

	if (mCanScroll)
	{
		Rect	frame;
		mTextWidget->CalcLocalFrameRect(frame);

		// Check for auto scroll
		SInt16	hScroll = 0;
		SInt16	vScroll = 0;

		if (mouseLoc.v < frame.top + cTEDragAutoScroll)
			vScroll = -17;
		else if (mouseLoc.v > frame.bottom - cTEDragAutoScroll)
			vScroll = 17;
		else if (mouseLoc.h < frame.left + cTEDragAutoScroll)
			hScroll = -16;
		else if (mouseLoc.h > frame.right - cTEDragAutoScroll)
			hScroll = 16;

		if (hScroll || vScroll) {
			DrawDragInsert(mLastDragOffset, mLastDragLeading);
			//((CTextDisplay*) mTextWidget)->ScrollPinnedImageBy(hScroll, vScroll, true);
			DrawDragInsert(mLastDragOffset, mLastDragLeading);
		}
	}

	// Set appropriate cursor
	AdjustCursorDropArea(inDragRef);

}

// Adjust cursor over drop area
void CTextWidgetDragAndDrop::AdjustCursorDropArea(DragReference inDragRef)
{
	// No special cursor processing
	if (IsCopyCursor(inDragRef) && !mCopyCursor)
	{
		UCursor::SetThemeCursor(kThemeCopyArrowCursor);
		mCopyCursor = true;
	}
	else if (!IsCopyCursor(inDragRef) && mCopyCursor)
	{
		UCursor::SetArrow();
		mCopyCursor = false;
	}
}

// Adjust cursor over drop area
bool CTextWidgetDragAndDrop::IsCopyCursor(DragReference inDragRef)
{
	// Toggle based on modifiers
	short mouseModifiers;
	::GetDragModifiers(inDragRef, &mouseModifiers, NULL, NULL);
	bool option_key = mouseModifiers & optionKey;

	// Text is always copied when coming from outside
	return !(CheckIfViewIsAlsoSender(inDragRef) ^ option_key);
}

// Draw drag insert cursor
void CTextWidgetDragAndDrop::DrawDragInsert(UniCharArrayOffset theOffset, bool leading)
{
	StColorPenState	save_pen;				// Save pen state
	StColorPenState::Normalize();

	// Get height of current line
	ItemCount line = mTextWidget->OffsetToLine(theOffset);
	SInt32 theLineHeight = mTextWidget->GetLineHeight(line);

	// Get point for current char
	mTextWidget->FocusDraw();
	Point thePt = mTextWidget->OffsetToPoint(theOffset, leading);

	// Draw insert cursor with Xor
	::PenMode(patXor);
	::MoveTo(thePt.h - 1, thePt.v - theLineHeight / 2);
	::LineTo(thePt.h - 1, thePt.v + theLineHeight / 2);
}

// Other flavor requested by receiver
void CTextWidgetDragAndDrop::DoDragSendData(
	FlavorType		inFlavor,
	ItemReference	inItemRef,
	DragReference	inDragRef)
{
	if (inFlavor == kScrapFlavorTypeUnicode)
	{
		// Check for trailing space and extend selection
		if (mCanAcceptCurrentDrag &&
			(mTextWidget->mSelection.caret < mTextWidget->mTextLength) &&
			(mTextWidget->mTextPtr[mTextWidget->mSelection.caret + mTextWidget->mSelection.length] == ' '))
			mTextWidget->ModifySelection(mTextWidget->mSelection.caret, mTextWidget->mSelection.caret + mTextWidget->mSelection.length + 1);

		// Give drag this data
		StHandleBlock utxt(mTextWidget->GetTextRangeAs_utxt());
		StHandleLocker _lock(utxt);
		ThrowIfOSErr_(::SetDragItemFlavorData(inDragRef, inItemRef, kScrapFlavorTypeUnicode, *utxt, ::GetHandleSize(utxt), 0L));
	}
	else if (inFlavor == kScrapFlavorTypeUnicodeStyle)
	{
		// Check for trailing space and extend selection
		if (mCanAcceptCurrentDrag &&
			(mTextWidget->mSelection.caret < mTextWidget->mTextLength) &&
			(mTextWidget->mTextPtr[mTextWidget->mSelection.caret + mTextWidget->mSelection.length] == ' '))
			mTextWidget->ModifySelection(mTextWidget->mSelection.caret, mTextWidget->mSelection.caret + mTextWidget->mSelection.length + 1);

		// Give drag this data
		StHandleBlock ustl(mTextWidget->GetStyleRangeAs_ustl());
		StHandleLocker _lock(ustl);
		ThrowIfOSErr_(::SetDragItemFlavorData(inDragRef, inItemRef, kScrapFlavorTypeUnicodeStyle, *ustl, ::GetHandleSize(ustl), 0L));
	}
	else if (inFlavor == kScrapFlavorTypeText)
	{
		// Check for trailing space and extend selection
		if (mCanAcceptCurrentDrag &&
			(mTextWidget->mSelection.caret < mTextWidget->mTextLength) &&
			(mTextWidget->mTextPtr[mTextWidget->mSelection.caret + mTextWidget->mSelection.length] == ' '))
			mTextWidget->ModifySelection(mTextWidget->mSelection.caret, mTextWidget->mSelection.caret + mTextWidget->mSelection.length + 1);

		// Give drag this data
		StHandleBlock text(mTextWidget->GetTextRangeAs_TEXT());
		StHandleLocker _lock(text);
		ThrowIfOSErr_(::SetDragItemFlavorData(inDragRef, inItemRef, kScrapFlavorTypeText, *text, ::GetHandleSize(text), 0L));
	}
	else
		Throw_(badDragFlavorErr);
}
