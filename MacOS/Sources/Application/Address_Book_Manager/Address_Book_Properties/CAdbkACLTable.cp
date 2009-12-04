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


//	CACLTable.cp

#include "CACLTable.h"

#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CPropMailboxACL.h"
#include "CTableRowGeometry.h"
#include "CTableRowSelector.h"

#include <LString.h>

#include <UDrawingState.h>
#include <UDrawingUtils.h>

#include <PP_KeyCodes.h>
#include <PP_Messages.h>

CACLTable* CACLTable::CreateFromStream(LStream *inStream)
{
	return (new CACLTable(inStream));
}


CACLTable::CACLTable(LStream *inStream)
	: CTableDrag(inStream)

{	
	mTableGeometry = new CTableRowGeometry(this, mFrameSize.width, 18);
	mTableSelector = new CTableRowSelector(this);
	mACLs = nil;
}


CACLTable::~CACLTable()
{
}

// Get details of sub-panes
void CACLTable::FinishCreateSelf(void)
{
	// Do inherited
	CTableDrag::FinishCreateSelf();

	// Create columns
	InsertCols(10, 1, nil, 0, false);
	AdaptToNewSurroundings();
	
	// Get text traits resource
	SetTextTraits(CPreferences::sPrefs->mListTextTraits);

	// Set read only status of Drag and Drop
	SetAllowDrag(false);
}

// Handle key presses
Boolean CACLTable::HandleKeyPress(const EventRecord &inKeyEvent)
{
	switch (inKeyEvent.message & charCodeMask)
	{
		// Nudge selection up
		case char_UpArrow:
			NudgeSelection(-1);
			break;
		
		// Nudge selection down	
		case char_DownArrow:
			NudgeSelection(1);
			break;
			
		default:
			return LCommander::HandleKeyPress(inKeyEvent);
	}
			
	return true;
}

//	Respond to commands
Boolean CACLTable::ObeyCommand(CommandT inCommand,void *ioParam)
{
	Boolean	cmdHandled = true;
	
	switch (inCommand) {

		case cmd_SelectAll:
			SelectAllCells();
			break;

		default:
			cmdHandled = LCommander::ObeyCommand(inCommand, ioParam);
			break;
	}
	
	return cmdHandled;
}

//	Pass back status of a (menu) command
void CACLTable::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	Char16		&outMark,
	Str255		outName)
{
	outUsesMark = false;
	
	switch (inCommand) {
	
		case cmd_SelectAll:			// Check if any addresses are present
			outEnabled = mRows > 0;
			break;

		default:
			LCommander::FindCommandStatus(inCommand, outEnabled, outUsesMark,
								outMark, outName);
			break;
	}
}

void CACLTable::DrawCell(const STableCell &inCell, const Rect &inLocalRect)
{
	// Get item
	CMboxACL& rights = mACLs->at(inCell.row - 1);

	// Save text state in stack object
	StTextState		textState;
	StColorState	saveColors;
	StColorPenState::Normalize();
	LStr255			theTxt;
	ResIDT			icon;

	// Set to required text
	UTextTraits::SetPortTextTraits(&mTextTraits);

	// Clip to cell frame & table frame
	Rect	clipper = mRevealedRect;
	PortToLocalPoint(topLeft(clipper));
	PortToLocalPoint(botRight(clipper));
	::SectRect(&clipper, &inLocalRect, &clipper);
	StClipRgnState	clip(clipper);

	switch(inCell.col)
	{
	
	case 1:
		// Get name of item
		theTxt = (const char*) rights.GetUID();
		::MoveTo(inLocalRect.left + 4, inLocalRect.bottom - mTextDescent);
		short width = inLocalRect.right - inLocalRect.left - 4;
		::DrawClippedString(theTxt, width);
		break;
	
	case 2:
		// Determine icon
		icon = rights.HasRight(CMboxACL::eMboxACL_Lookup) ? ICNx_ACLSet : ICNx_ACLUnset;
		break;

	case 3:
		// Determine icon
		icon = rights.HasRight(CMboxACL::eMboxACL_Read) ? ICNx_ACLSet : ICNx_ACLUnset;
		break;

	case 4:
		// Determine icon
		icon = rights.HasRight(CMboxACL::eMboxACL_Seen) ? ICNx_ACLSet : ICNx_ACLUnset;
		break;

	case 5:
		// Determine icon
		icon = rights.HasRight(CMboxACL::eMboxACL_Write) ? ICNx_ACLSet : ICNx_ACLUnset;
		break;

	case 6:
		// Determine icon
		icon = rights.HasRight(CMboxACL::eMboxACL_Insert) ? ICNx_ACLSet : ICNx_ACLUnset;
		break;

	case 7:
		// Determine icon
		icon = rights.HasRight(CMboxACL::eMboxACL_Post) ? ICNx_ACLSet : ICNx_ACLUnset;
		break;

	case 8:
		// Determine icon
		icon = rights.HasRight(CMboxACL::eMboxACL_Create) ? ICNx_ACLSet : ICNx_ACLUnset;
		break;

	case 9:
		// Determine icon
		icon = rights.HasRight(CMboxACL::eMboxACL_Delete) ? ICNx_ACLSet : ICNx_ACLUnset;
		break;

	case 10:
		// Determine icon
		icon = rights.HasRight(CMboxACL::eMboxACL_Admin) ? ICNx_ACLSet : ICNx_ACLUnset;
		break;

	default:
		break;
	}
	
	if (inCell.col > 1)
	{
		Rect	iconRect;
		iconRect.left = inLocalRect.left;
		iconRect.right = iconRect.left + 16;
		iconRect.bottom = inLocalRect.bottom - mIconDescent;
		iconRect.top = iconRect.bottom - 16;

		::PlotIconID(&iconRect, atNone, IsActive() ? ttNone : ttDisabled, icon);
	}
}

// Draw or undraw active hiliting for a Cell
void CACLTable::HiliteCellActively(const STableCell &inCell, Boolean inHilite)
{
	Rect	cellFrame;

	// Must have list
	if (GetLocalCellRect(inCell, cellFrame) && FocusExposed() && mACLs)
	{
		StColorState saveColors;
		StColorState::Normalize();
	
		UDrawingUtils::SetHiliteModeOn();
		::InvertRect(&cellFrame);
	}
}

// Draw or undraw inactive hiliting for a Cell
void CACLTable::HiliteCellInactively(const STableCell &inCell, Boolean inHilite)
{
	Rect	cellFrame;

	// Must have list
	if (GetLocalCellRect(inCell, cellFrame) && FocusExposed() && mACLs)
	{
		// Adjust frame ends
		if (inCell.col == 1)
			cellFrame.left -= 1;

		if (inCell.col == mCols)
			cellFrame.right -= 1;

		cellFrame.bottom -= 2;

		StColorState saveColors;
		StColorState::Normalize();
	
		::PenNormal();
		::PenMode(srcXor);
		
		// Draw horiz frame lines
		UDrawingUtils::SetHiliteModeOn();
		::MoveTo(cellFrame.left, cellFrame.top);
		::LineTo(cellFrame.right-1, cellFrame.top);
		UDrawingUtils::SetHiliteModeOn();
		::MoveTo(cellFrame.left, cellFrame.bottom);
		::LineTo(cellFrame.right-1, cellFrame.bottom);
		
		if (inCell.col == 1)
		{
			UDrawingUtils::SetHiliteModeOn();
			::MoveTo(cellFrame.left, cellFrame.top+1);
			::LineTo(cellFrame.left, cellFrame.bottom-1);
		}
		if (inCell.col == mCols)
		{
			UDrawingUtils::SetHiliteModeOn();
			::MoveTo(cellFrame.right, cellFrame.top);
			::LineTo(cellFrame.right, cellFrame.bottom);
		}
	}
}

// Make it target first
void CACLTable::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	CTableDrag::ClickSelf(inMouseDown);
	
	// Find panel and force button update
	((CPropMailboxACL*) GetSuperView()->GetSuperView()->GetSuperView())->UpdateButtons();
}

// Click in the cell
void CACLTable::ClickCell(
	const STableCell		&inCell,
	const SMouseDownEvent	&inMouseDown)
{
	CTableDrag::ClickCell(inCell, inMouseDown);
}

void CACLTable::SetList(CMboxACLList* aList)
{
	mACLs = aList;

	// Create entries
	if (mACLs)
	{
		RemoveRows(mRows, 1, false);
		InsertRows(mACLs->size(), 0, nil, 0, true);
	}
}

// Adjust column widths
void CACLTable::AdaptToNewSurroundings(void)
{
	// Do inherited call
	CTableDrag::AdaptToNewSurroundings();
	
	// Set image to frame size
	ResizeImageTo(mFrameSize.width, mImageSize.height, true);

	// Name column has variable width
	SetColWidth(mFrameSize.width - 171, 1, 1);
	
	// Remaining columns have fixed width
	SetColWidth(19, 2, 10);

} // CACLTable::AdaptToNewSurroundings
