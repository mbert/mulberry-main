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


// Source for CTextTable class

#include "CTextTable.h"

#include "CBetterScrollerX.h"
#include "CDragIt.h"
#include "CMulberryCommon.h"
#include "CTableMultiRowSelector.h"
#include "CTableRowGeometry.h"

#include <LTableArrayStorage.h>
#include <LTableSingleRowSelector.h>

// __________________________________________________________________________________________________
// C L A S S __ C T E X T T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTextTable::CTextTable()
{
	InitTextTable();
}

// Default constructor - just do parents' call
CTextTable::CTextTable(LStream *inStream)
		: CTableDrag(inStream)

{
	InitTextTable();

	*inStream >> mDoubleClickMsg;
	*inStream >> mSelectionMsg;
	ResIDT txtrID;
	*inStream >> txtrID;
	SetTextTraits(txtrID);
}

// Default destructor
CTextTable::~CTextTable()
{
}

// Common init
void CTextTable::InitTextTable(void)
{
	mTableGeometry = new CTableRowGeometry(this, 16, 16);
	mTableSelector = new CTableMultiRowSelector(this);
	mTableStorage = new LTableArrayStorage(this, (UInt32) 0);

	SetRowSelect(true);

	mDoubleClickMsg = msg_Nothing;
	mSelectionMsg = msg_Nothing;
	mDeleteMsg = msg_Nothing;
	mStringColumn = 1;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Broadcast message when selected cells change

void CTextTable::SelectionChanged()
{
	if (mSelectionMsg != msg_Nothing)
		BroadcastMessage(mSelectionMsg, (void*) this);

	// Do inherited
	CTableDrag::SelectionChanged();
}

// Allow only single selection
void CTextTable::SetSingleSelection(void)
{
	delete mTableSelector;
	mTableSelector = new LTableSingleRowSelector(this);
}

// Allow no selection
void CTextTable::SetNoSelection(void)
{
	delete mTableSelector;
	mTableSelector = NULL;
}

void CTextTable::AddItem(const cdstring& item)
{
	InsertRows(1, mRows, NULL, 0, false);

	STableCell aCell(mRows, 1);
	SetCellData(aCell, item.c_str(), item.length() + 1);

	RefreshCell(aCell);
}

void CTextTable::SetContents(const cdstrvect& items)
{
	RemoveAllRows(false);
	InsertRows(items.size(), 0, NULL, 0, false);

	STableCell aCell(0, 1);
	TableIndexT row = 1;
	for(cdstrvect::const_iterator iter = items.begin(); iter != items.end(); iter++, row++)
	{
		aCell.row = row;
		SetCellData(aCell, (*iter).c_str(), (*iter).length() + 1);
	}

	Refresh();
}

void CTextTable::GetSelection(ulvector& selection) const
{
	STableCell aCell(0, 0);

	while(GetNextSelectedCell(aCell))
	{
		if (aCell.col == mStringColumn)
			selection.push_back(aCell.row - 1);
	}
}

void CTextTable::GetSelection(cdstrvect& selection) const
{
	STableCell aCell(0, 0);

	while(GetNextSelectedCell(aCell))
	{
		if (aCell.col == mStringColumn)
		{
			cdstring temp;
			GetCellString(aCell, temp);

			selection.push_back(temp);
		}
	}
}

// Get details of sub-panes
void CTextTable::FinishCreateSelf(void)
{
	// Do inherited
	CTableDrag::FinishCreateSelf();

	// Make it fit to the superview
	InsertCols(1, 1, NULL, 0, false);
	mStringColumn = 1;

	// Get super frame's width - scroll bar
	SDimension16 super_frame;
	mSuperView->GetFrameSize(super_frame);
	super_frame.width -= 16;

	// Set image to super frame size
	ResizeImageTo(super_frame.width, mImageSize.height, true);

	// Set column width
	SetColWidth(super_frame.width, 1, 1);

	// No drag and drop unless exlicitly specified
	SetAllowDrag(false);

	// Hack to set frame style of scroller
	if (dynamic_cast<CBetterScrollerX*>(GetSuperView()) && !IsEnabled())
		static_cast<CBetterScrollerX*>(GetSuperView())->SetReadOnly(true);
}

void CTextTable::EnableDragAndDrop()
{
	// Set Drag & Drop pane to scroller
	mPane = GetSuperView();

	SetTable(this, false);
	AddDropFlavor(kScrapFlavorTypeText);
	AddDragFlavor(kScrapFlavorTypeText);

	SetDDReadOnly(false);
	SetDropCell(false);
	SetDropCursor(true);
	SetAllowDrag(true);
	SetSelfDrag(true);
	SetExternalDrag(false);

	// Set appropriate Drag & Drop inset
	CBetterScrollerX* scroller = dynamic_cast<CBetterScrollerX*>(mPane);
	if (scroller)
	{
		Rect ddInset = {1, 1, 1, 1};
		if (scroller->HasVerticalScrollBar())
			ddInset.right += 15;
		if (scroller->HasHorizontalScrollBar())
			ddInset.bottom += 15;
		SetHiliteInset(ddInset);
	}
}


// Handle key presses
Boolean CTextTable::HandleKeyPress(const EventRecord &inKeyEvent)
{
	switch (inKeyEvent.message & charCodeMask)
	{
		// Delete item
		case char_Backspace:
			if (mDeleteMsg != msg_Nothing)
				BroadcastMessage(mDeleteMsg, (void*) this);
			break;

		default:
			return CTableDrag::HandleKeyPress(inKeyEvent);
	}

	return true;
}

//	Respond to commands
Boolean CTextTable::ObeyCommand(CommandT inCommand,void *ioParam)
{
	Boolean	cmdHandled = true;

	switch (inCommand)
	{
	case cmd_Clear:
		if (mDeleteMsg != msg_Nothing)
			BroadcastMessage(mDeleteMsg, (void*) this);
		break;

	default:
		cmdHandled = CTableDrag::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CTextTable::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_Clear:
		outEnabled = IsSelectionValid() && (mDeleteMsg != msg_Nothing);
		break;

	default:
		CTableDrag::FindCommandStatus(inCommand, outEnabled, outUsesMark,
							outMark, outName);
		break;
	}
}

// Make it target first
void CTextTable::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	SwitchTarget(this);
	CTableDrag::ClickSelf(inMouseDown);
}

// Click in the cell
void CTextTable::ClickCell(
	const STableCell&		inCell,
	const SMouseDownEvent&	inMouseDown)
{
	// Check whether D&D available and over a selected cell or not shift or cmd keys
	if (mAllowDrag &&
		DragAndDropIsPresent() &&
		(CellIsSelected(inCell) ||
			(!(inMouseDown.macEvent.modifiers & shiftKey) &&
			 !(inMouseDown.macEvent.modifiers & cmdKey))))
	{

		// Track item long enough to distinguish between a click to
		// select, and the beginning of a drag
		//bool isDrag = !CContextMenuProcessAttachment::ProcessingContextMenu() &&
		//				::WaitMouseMoved(inMouseDown.macEvent.where);
		bool isDrag = ::WaitMouseMoved(inMouseDown.macEvent.where);

		// Now do drag
		if (isDrag)
		{
			// If we leave the window, the drag manager will be changing thePort,
			// so we'll make sure thePort remains properly set.
			OutOfFocus(NULL);
			FocusDraw();
			OSErr err = CreateDragEvent(inMouseDown);
			OutOfFocus(NULL);

			return;
		}
	}

	if (GetClickCount() == 1)
		// Carry on to do default action
		CTableDrag::ClickCell(inCell, inMouseDown);

	else if (GetClickCount() == 2)
	{
		if (mDoubleClickMsg != msg_Nothing)
			BroadcastMessage(mDoubleClickMsg, (void*) this);
	}

}

// Draw the entries
void CTextTable::DrawCell(const STableCell& inCell,
								const Rect& inLocalRect)
{
	// Save text & color state in stack objects
	StTextState		textState;
	StColorState	saveColors;
	StColorPenState::Normalize();

	// Set to required text
	UTextTraits::SetPortTextTraits(&mTextTraits);

	// Make sure there's a white background for the selection InvertRect
	::EraseRect(&inLocalRect);

#if PP_Target_Carbon
	// Draw selection
	DrawCellSelection(inCell);
#endif

	// Clip to cell frame & table frame
	Rect	clipper = mRevealedRect;
	PortToLocalPoint(topLeft(clipper));
	PortToLocalPoint(botRight(clipper));
	::SectRect(&clipper,&inLocalRect,&clipper);
	StClipRgnState	clip(clipper);

	// Move to origin for text
	::MoveTo(inLocalRect.left, inLocalRect.bottom - mTextDescent);

	// Get data
	cdstring temp;
	GetCellString(inCell, temp);

	// Draw the string
	::DrawClippedStringUTF8(temp, inLocalRect.right - inLocalRect.left, eDrawString_Left);

}

void CTextTable::GetCellString(const STableCell& inCell, cdstring& str) const
{
	// Get data length
	UInt32	len = 0;
	GetCellData(inCell, NULL, len);
	
	// Allocate data size and load in data
	str.reserve(len);
	GetCellData(inCell, str.c_str_mod(), len);
}

#pragma mark ____________________________Drag & Drop

// Add mail message to drag
void CTextTable::AddCellToDrag(CDragIt* theDragTask,
										const STableCell& aCell,
										Rect& dragRect)
{
	// Get the relevant message and envelope
	unsigned long row = aCell.row - 1;

	// Add this message to drag
	theDragTask->AddFlavorItem(dragRect, (ItemReference) row, kScrapFlavorTypeText,
								&row, sizeof(unsigned long), flavorSenderOnly, true);
}

// Handle multiple items
void CTextTable::DoDragReceive(DragReference inDragRef)
{
	// Clear drag accumulation object
	mDragged.mTable = this;
	mDragged.mDragged.clear();
	mDragged.mDropped = 0;

	// Do default action
	CTableDrag::DoDragReceive(inDragRef);

	// Broadcast the entire drag
	Broadcast_Message(eBroadcast_Drag, &mDragged);
}

// Drop data at cell
void CTextTable::DropDataAtCell(FlavorType theFlavor,
										char* drag_data,
										Size data_size,
										const STableCell& beforeCell)
{
	// Just add drag items to accumulator
	mDragged.mDragged.push_back(*reinterpret_cast<unsigned long*>(drag_data));
	mDragged.mDropped = beforeCell.row - 1;
}
