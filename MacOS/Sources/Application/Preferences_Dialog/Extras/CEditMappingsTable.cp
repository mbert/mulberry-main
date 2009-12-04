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


//	CEditMappingsTable.cp

#include "CEditMappingsTable.h"

#include "CBetterScrollerX.h"
#include "CDesktopIcons.h"
#include "CHelpAttach.h"
#include "CMIMEMap.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CPrefsEditMappings.h"

#include <LTableMultiGeometry.h>
#include <LTableSingleRowSelector.h>


#include <stdio.h>

CEditMappingsTable::CEditMappingsTable(LStream	*inStream)
	: CTableDrag(inStream)

{
	mTextTraits = nil;

	mTableGeometry = new LTableMultiGeometry(this, mFrameSize.width, 20);
	mTableSelector = new LTableSingleRowSelector(this);
	mMappings = nil;

	mRowShow = 0;

	SetRowSelect(true);
}


CEditMappingsTable::~CEditMappingsTable()
{
	// Forget traits
	DISPOSE_HANDLE(mTextTraits);
}

// Get details of sub-panes
void CEditMappingsTable::FinishCreateSelf(void)
{
	// Do inherited
	LTableView::FinishCreateSelf();

	// Find window in super view chain
	mWindow = (CPrefsEditMappings*) mSuperView;
	while(mWindow->GetPaneID() != paneid_PrefsEditMappings)
		mWindow = (CPrefsEditMappings*) mWindow->GetSuperView();

	// Create columns
	InsertCols(3, 1, nil, 0, false);
	SetColWidth(174, 1, 1);
	SetColWidth(184, 2, 2);
	SetColWidth(69, 3, 3);

	// Get text traits resource - detatch & lock
	mTextTraits = UTextTraits::LoadTextTraits(261);
	ThrowIfResFail_(mTextTraits);
	::DetachResource((Handle) mTextTraits);
	::MoveHHi((Handle) mTextTraits);
	::HLock((Handle) mTextTraits);

	// Set Drag & Drop pane to scroller
	mPane = GetSuperView();

	// Set Drag & Drop info
	SetTable(this, false);

	// Set appropriate Drag & Drop inset
	Rect ddInset = {1, 1, 1, 1};
	if (((CBetterScrollerX*) mPane)->HasVerticalScrollBar())
		ddInset.right += 15;
	if (((CBetterScrollerX*) mPane)->HasHorizontalScrollBar())
		ddInset.bottom += 15;
	SetHiliteInset(ddInset);

	// Set read only status of Drag and Drop
	SetDDReadOnly(true);

}

//	Pass back status of a (menu) command
void CEditMappingsTable::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand) {

		case cmd_SelectAll:			// Check if any items are present
			outEnabled = false;
			break;

		default:
			CTableDrag::FindCommandStatus(inCommand, outEnabled, outUsesMark,
								outMark, outName);
			break;
	}
}

//	Respond to commands
Boolean CEditMappingsTable::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool	cmdHandled = true;

	switch (inCommand) {

		case cmd_SelectAll:
		case msg_TabSelect:
			break;

		default:
			cmdHandled = CTableDrag::ObeyCommand(inCommand, ioParam);
			break;
	}

	return cmdHandled;
}

void CEditMappingsTable::DrawCell(const STableCell &inCell, const Rect &inLocalRect)
{
	CMIMEMap& mapping = (*mMappings)[inCell.row - 1];

	// Save text state in stack object
	StTextState		textState;
	StColorState	saveColors;
	StColorPenState::Normalize();

#if PP_Target_Carbon
	// Draw selection
	DrawCellSelection(inCell);
#endif

	// Set to required text
	UTextTraits::SetPortTextTraits(*mTextTraits);

	// Clip to cell frame & table frame
	Rect	clipper = mRevealedRect;
	PortToLocalPoint(topLeft(clipper));
	PortToLocalPoint(botRight(clipper));
	::SectRect(&clipper, &inLocalRect, &clipper);
	StClipRgnState	clip(clipper);

	switch(inCell.col)
	{

	case 1:
		{
			// Draw MIME type/subtype
			::MoveTo(inLocalRect.left, inLocalRect.bottom - 6);
			cdstring mime;
			if (mapping.GetMIMEType())
				mime = mapping.GetMIMEType();
			else
				mime = "????";
			mime += '/';
			if (mapping.GetMIMESubtype())
				mime += mapping.GetMIMESubtype();
			else
				mime += "????";
			::DrawClippedStringUTF8(mime, inLocalRect.right - inLocalRect.left, eDrawString_Left);
		}
		break;

	case 2:
		{
			// Draw icon followed by name
			Rect iconRect;
			iconRect.left = inLocalRect.left + 3;
			iconRect.right = iconRect.left + 16;
			iconRect.bottom = inLocalRect.bottom - 2;
			iconRect.top = iconRect.bottom - 16;
			IconRef icons = CDesktopIcons::GetDesktopIcons(mapping.GetFileCreator(), 'APPL')->GetIconRef();
			if (icons)
				::PlotIconRef(&iconRect, kAlignNone, kTransformNone, kIconServicesNormalUsageFlag, icons);

			::MoveTo(iconRect.right - 2, inLocalRect.bottom - 6);
			LStr255 name;
			if (mapping.GetFileCreator() != '????')
				CDesktopIcons::GetAPPLName(mapping.GetFileCreator(), name);
			else
				name = "????";
			cdstring temp(name);
			::DrawClippedStringUTF8(temp, inLocalRect.right - iconRect.right - 2, eDrawString_Left);
		}
		break;

	case 3:
		{
			// Get type
			::MoveTo(inLocalRect.left, inLocalRect.bottom - 6);
			OSType ftype = mapping.GetFileType();
			cdstring name_type((const char*)&ftype, 4);
			::DrawClippedStringUTF8(name_type, inLocalRect.right - inLocalRect.left, eDrawString_Left);
		}
		break;

	default:
		break;
	}
}

// Select the cell
void CEditMappingsTable::SelectCell(const STableCell &inCell)
{
	if (inCell.row != mRowShow)
	{
		// Change value
		mRowShow = inCell.row;

		// Tell window to set
		mWindow->ShowMapping(mRowShow);
	}

	CTableDrag::SelectCell(inCell);
}

// Select the cell
Boolean CEditMappingsTable::ClickSelect(const STableCell &inCell, const SMouseDownEvent &inMouseDown)
{
	if (inCell.row != mRowShow)
	{
		// Change value
		mRowShow = inCell.row;

		// Tell window to set
		mWindow->ShowMapping(mRowShow);
	}

	return CTableDrag::ClickSelect(inCell, inMouseDown);
}

// Click in the cell
void CEditMappingsTable::UnselectAllCells(void)
{
	if (mRowShow)
	{
		// Change value
		mRowShow = 0;

		// Tell window to set
		mWindow->ShowMapping(mRowShow);
	}
	CTableDrag::UnselectAllCells();
}

// Reset the table from the mbox
void CEditMappingsTable::ResetTable(void)
{
	short num_parts = mMappings->size();

	if (mRows > num_parts)
		RemoveRows(mRows - num_parts, 1, false);
	else if (mRows < num_parts)
		InsertRows(num_parts - mRows, LArray::index_Last, nil, 0, false);

	UnselectAllCells();
	Refresh();
}

void CEditMappingsTable::SetMappings(CMIMEMapVector* mapping)
{
	mMappings = mapping;

	// Reset table based on new body
	ResetTable();
}
