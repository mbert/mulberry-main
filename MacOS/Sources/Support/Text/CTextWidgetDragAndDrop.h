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


// Header for CTextWidgetDragAndDrop class

#ifndef __CTextWidgetDragAndDrop__MULBERRY__
#define __CTextWidgetDragAndDrop__MULBERRY__


// Constants

const	SInt16		cTEDragAutoScroll = 4;

// Classes

class LDragTask;
class CTextWidget;
class LCommander;

class CTextWidgetDragAndDrop : public LDragAndDrop
{
protected:
	CTextWidget*			mTextWidget;				// Pane containing textedit
	bool					mCopyCursor;				// Is copy cursor showing
	UniCharArrayOffset		mLastDragOffset;			// Last offset in text
	bool					mLastDragLeading;			// Insert cursor leading
	Rect					mHiliteInset;				// Hilite inset
	RGBColor				mHiliteBackground;			// Color of background for hilite
	Boolean					mCanScroll;					// Text can be scrolled
	static CTextWidget*		sDragFrom;					// Handle to TE to drag from
	static CTextWidget*		sDragTo;					// Handle to TE to drag to
	LArray					mDropFlavors;				// List of acceptable flavors

public:

					CTextWidgetDragAndDrop(WindowPtr inMacWindow, LPane* inPane);
	virtual 		~CTextWidgetDragAndDrop();
	
	virtual	void	SetTextEdit(CTextWidget* inWidget,
								Boolean canScroll = 0)
						{ mTextWidget = inWidget;
						  mCanScroll = canScroll; }							// Set TE details
	virtual void	SetHiliteInset(Rect &hiliteInset)
						{ mHiliteInset = hiliteInset; }						// Set hilite inset
	virtual void	SetHiliteBackground(const RGBColor &hiliteBackground)
						{ mHiliteBackground = hiliteBackground; }			// Set hilite background color

// Drag methods
protected:
	virtual OSErr	CreateDragEvent(const SMouseDownEvent& inMouseDown);	// Create the drag event

private:
	virtual void	AddDragText(LDragTask* theDragTask);				// Add mail message to drag

protected:
	virtual Boolean	ItemIsAcceptable(DragReference inDragRef,
								ItemReference inItemRef);				// Check its a message

	virtual Boolean	CheckIfViewIsAlsoSender(DragReference inDragRef);	// Check to see whether drop from another window

	virtual void	AddDropFlavor(FlavorType theFlavor,
									SInt32 pos = LArray::index_Last)	// Set its drop flavor
						{ mDropFlavors.InsertItemsAt(1, pos, &theFlavor); }

	virtual void	DoDragReceive(DragReference	inDragRef);				// Get multiple text items
	virtual void	CreateReceiveAction(LCommander* cmdr,
										Handle theFlavorData,
										Size theDataSize,
										Handle theStyles,
										Boolean delete_orig);			// Post required action

	virtual void	EnterDropArea(DragReference inDragRef,
									Boolean inDragHasLeftSender);		// Reset drop cursor
	virtual void	HiliteDropArea(DragReference inDragRef);			// Hilite region border
	virtual void	UnhiliteDropArea(DragReference inDragRef);			// Unhilite region border
	virtual void	LeaveDropArea(DragReference inDragRef);				// Do correct unhiliting of drop region
	virtual void	InsideDropArea(DragReference inDragRef);			// Handle drag into text
	virtual void	AdjustCursorDropArea(DragReference inDragRef);	// Adjust cursor over drop area
	virtual bool	IsCopyCursor(DragReference inDragRef);			// Use copy cursor?

private:

	virtual void	DrawDragInsert(UniCharArrayOffset theOffset,		// Draw drag insert cursor
									bool leading);

protected:
	virtual void	DoDragSendData(FlavorType inFlavor,
									ItemReference inItemRef,
									DragReference inDragRef);			// Other flavor requested by receiver

};

#endif
