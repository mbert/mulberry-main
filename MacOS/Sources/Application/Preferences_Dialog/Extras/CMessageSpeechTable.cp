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


//	CMessageSpeechTable.cp

#include "CMessageSpeechTable.h"

#include "CBalloonDialog.h"
#include "CBetterScrollerX.h"
#include "CMessageSpeechEditDialog.h"
#include "CMessageSpeechTableAction.h"
#include "CMulberryCommon.h"
#include "CTableMultiRowSelector.h"

#include <LTableMultiGeometry.h>

#include <stdio.h>

CMessageSpeechTable::CMessageSpeechTable(LStream *inStream)
	: CTableDrag(inStream)

{
	mTextTraits = nil;

	mTableGeometry = new LTableMultiGeometry(this, mFrameSize.width, 20);
	mTableSelector = new CTableMultiRowSelector(this);

	SetRowSelect(true);
}


CMessageSpeechTable::~CMessageSpeechTable()
{
	// Forget traits
	DISPOSE_HANDLE(mTextTraits);
}

// Get details of sub-panes
void CMessageSpeechTable::FinishCreateSelf(void)
{
	// Do inherited
	LTableView::FinishCreateSelf();
	AddAttachment(new LUndoer);

	// Create columns
	InsertCols(2, 1, nil, 0, false);
	SetColWidth(141, 1, 1);
	SetColWidth(286, 2, 2);

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

// Handle key presses
Boolean CMessageSpeechTable::HandleKeyPress(const EventRecord &inKeyEvent)
{
	switch (inKeyEvent.message & charCodeMask) {

		// Delete
		case char_Backspace:
		case char_Clear:
			// Special case escape key
			if ((inKeyEvent.message & keyCodeMask) == vkey_Escape)
				return CTableDrag::HandleKeyPress(inKeyEvent);
			else
				DeleteSelection();
			break;

		default:
			return CTableDrag::HandleKeyPress(inKeyEvent);
			break;
	}

	return true;
}

//	Pass back status of a (menu) command
void CMessageSpeechTable::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	// Not enabled since no clipboard support
	case cmd_Cut:
	case cmd_Copy:
	case cmd_Paste:
		outEnabled = false;
		break;

	// Only if a valid selection existss
	case cmd_Clear:
		outEnabled = IsSelectionValid();
		break;

	default:
		CTableDrag::FindCommandStatus(inCommand, outEnabled, outUsesMark,
							outMark, outName);
			break;
	}
}

//	Respond to commands
Boolean CMessageSpeechTable::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool	cmdHandled = true;

	switch (inCommand)
	{
	case cmd_Clear:
		DeleteSelection();
		break;

	default:
		cmdHandled = CTableDrag::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

// Do edit
void CMessageSpeechTable::ClickCell(const STableCell &inCell, const SMouseDownEvent &inMouseDown)
{
	// If multiclick then edit entry
	if ((GetClickCount() > 1) && !inMouseDown.delaySelect)
		EditEntry(inCell);

	// Carry on to do default action
	CTableDrag::ClickCell(inCell, inMouseDown);
}

void CMessageSpeechTable::DrawCell(const STableCell &inCell, const Rect &inLocalRect)
{
	CMessageSpeak& item = mList->at(inCell.row - 1);

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
			// Draw message item text
			::MoveTo(inLocalRect.left, inLocalRect.bottom - 6);
			cdstring item_desc(CSpeechSynthesis::sItemList.at(item.mItem - 1));
			::DrawClippedStringUTF8(item_desc, inLocalRect.right - inLocalRect.left, eDrawString_Left);
		}
		break;

	case 2:
		{
			// Draw spoken item
			::MoveTo(inLocalRect.left, inLocalRect.bottom - 6);
			cdstring item_text(item.mItemText);
			::DrawClippedStringUTF8(item_text, inLocalRect.right - inLocalRect.left, eDrawString_Left);
		}
		break;

	default:
		break;
	}
}

// Delete selected cells
void CMessageSpeechTable::DeleteSelection(void)
{
	// Copy existing list
	CMessageSpeakVector original = *mList;

	// Copy existing and remove selected
	CMessageSpeakVector changed = *mList;

	// Remove selected items in reverse
	DoToSelection1((DoToSelection1PP) &CMessageSpeechTable::RemoveFromList, &changed, false);

	// Initiate delete action
	PostAction(new CMessageSpeechTableAction(this, original, changed));
}

// Remove items from list
bool CMessageSpeechTable::RemoveFromList(TableIndexT row, CMessageSpeakVector* list)
{
	// Remove item
	list->erase(list->begin() + row - 1);

	return true;
}

// Edit specified item
void CMessageSpeechTable::EditEntry(const STableCell& aCell)
{
	{
		// Create the dialog
		CBalloonDialog	theHandler(paneid_MessageSpeechEdit, this);

		((CMessageSpeechEditDialog*) theHandler.GetDialog())->SetItem(&mList->at(aCell.row - 1));
		theHandler.StartDialog();

		// Let DialogHandler process events
		while (true)
		{
			MessageT hitMessage = theHandler.DoDialog();

			if (hitMessage == msg_OK)
			{
				CMessageSpeakVector original = *mList;
				CMessageSpeakVector changed = *mList;

				// Get changed data
				((CMessageSpeechEditDialog*) theHandler.GetDialog())->GetItem(&changed.at(aCell.row - 1));

				// Add info to action
				PostAction(new CMessageSpeechTableAction(this, original, changed));
				break;
			}
			else if (hitMessage == msg_Cancel)
			{
				break;
			}
		}
	}

	SwitchTarget(this);
}

// Reset the table from the mbox
void CMessageSpeechTable::ResetTable(void)
{
	short num_items = mList->size();

	if (mRows > num_items)
		RemoveRows(mRows - num_items, 1, false);
	else if (mRows < num_items)
		InsertRows(num_items - mRows, LArray::index_Last, nil, 0, false);

	Refresh();
}

void CMessageSpeechTable::SetList(CMessageSpeakVector* list)
{
	mList = list;
	ResetTable();
}
