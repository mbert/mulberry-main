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


//	CFileTable.cp

#include "CFileTable.h"

#include "CAdminLock.h"
#include "CAliasAttachment.h"
#include "CAttachmentList.h"
#include "CBalloonDialog.h"
#include "CBetterScrollerX.h"
#include "CCommands.h"
#include "CContextMenu.h"
#include "CDataAttachment.h"
#include "CDragIt.h"
#include "CErrorHandler.h"
#include "CFileAttachment.h"
#include "CLetterPartProp.h"
#include "CLetterWindow.h"
#include "CMailControl.h"
#include "CMessage.h"
#include "CMessageAttachment.h"
#include "CMessageList.h"
#include "CMessagePartProp.h"
#include "CMessageWindow.h"
#include "CMulberryCommon.h"
#include "CNodeVectorTree.h"
#include "CPreferences.h"
#include "CResources.h"
#include "CSimpleTitleTable.h"
#include "CStringResources.h"
#include "CTableMultiRowSelector.h"
#include "CTableRowGeometry.h"
#include "CXStringResources.h"

#include <LDisclosureTriangle.h>
#include <LDropFlag.h>
#include <LTableArrayStorage.h>

#include <UStandardDialogs.h>

#include <stdio.h>

CFileTable::CFileTable(LStream	*inStream)
	: CHierarchyTableDrag(inStream)

{
	mTableGeometry = new CTableRowGeometry(this, mFrameSize.width, 18);
	mTableSelector = new CTableMultiRowSelector(this);
	mTableStorage = new LTableArrayStorage(this, sizeof(CAttachment*));

	mBody = NULL;
	mAddAction = NULL;
	mRowShow = 0;
	mWindow = NULL;
	mTitles = NULL;
	mDirty = false;
	mAttachmentsOnly = false;
	mLocked = false;
}


CFileTable::~CFileTable()
{
}

// Get details of sub-panes
void CFileTable::FinishCreateSelf(void)
{
	// Do inherited
	LTableView::FinishCreateSelf();

	// Find window in super view chain
	mWindow = (CLetterWindow*) mSuperView;
	while(mWindow->GetPaneID() != paneid_LetterWindow)
		mWindow = (CLetterWindow*) mWindow->GetSuperView();

	// Create columns and adjust flag rect
	InsertCols(6, 1, NULL, 0, false);
	SetOneColumnSelect(3);
	AdaptToNewSurroundings();
	SetRect(&mFlagRect, 2, 5, 18, 17);

	// Set Drag & Drop pane to scroller
	mPane = GetSuperView();

	// Set Drag & Drop info
	SetTable(this, false);
	AddDropFlavor(cDragMsgType);
	AddDropFlavor(cDragAtchType);
	AddDropFlavor(cDragMsgAtchType);
	
	// Only if not locked out
	if (!CAdminLock::sAdminLock.mNoAttachments)
		AddDropFlavor(flavorTypeHFS);

	AddDragFlavor(cDragAtchType);

	SetDDReadOnly(false);
	SetDropCell(true);
	SetDropCursor(true);
	SetAllowDrag(true);
	SetAllowMove(true);
	SetSelfDrag(true);

	// Set appropriate Drag & Drop inset
	Rect ddInset = {1, 1, 1, 1};
	if (((CBetterScrollerX*) mPane)->HasVerticalScrollBar())
		ddInset.right += 15;
	if (((CBetterScrollerX*) mPane)->HasHorizontalScrollBar())
		ddInset.bottom += 15;
	SetHiliteInset(ddInset);

}

void CFileTable::SetBody(CAttachment* aBody)
{
	mBody = aBody;

	// Reset table based on new body
	ResetTable();
}

void CFileTable::SetRowShow(CAttachment* attach)
{
	TableIndexT rows;
	TableIndexT cols;
	GetWideOpenTableSize(rows, cols);

	// Look at each cell
	for(int woRow = 1; woRow <= rows; woRow++)
	{
		STableCell	woCell(woRow, 1);
		CAttachment* row_attach = NULL;
		UInt32 dataSize = sizeof(CAttachment*);
		GetCellData(woCell, &row_attach, dataSize);
		
		if (attach == row_attach)
		{
			mRowShow = woRow;
			Refresh();
			return;
		}
	}	
}

CAttachment* CFileTable::GetPartShow(void)
{
	STableCell	woCell(mRowShow, 1);
	CAttachment* attach = NULL;
	UInt32 dataSize = sizeof(CAttachment*);
	GetCellData(woCell, &attach, dataSize);
	
	return attach;
}

unsigned long CFileTable::CountParts() const
{
	if (GetAttachmentsOnly())
		return mRows;
	else if (mBody != NULL)
		return mBody->CountParts();
	else
		return 0;
}

bool CFileTable::HasAttachments() const
{
	if (GetAttachmentsOnly())
		return mRows > 0;
	else if (mBody != NULL)
		return mBody->CountParts() > 1;
	else
		return false;
}

// Handle key presses
Boolean CFileTable::HandleKeyPress(const EventRecord &inKeyEvent)
{
	switch (inKeyEvent.message & charCodeMask)
	{
	// Open the message
	case char_Return:
	case char_Enter:
	{
		// Try to show it, or do view
		STableCell aCell;
		GetFirstSelection(aCell);
		if (IsSelectionValid() && (!IsSingleSelection() || mAttachmentsOnly || !ShowPart(aCell.row)))
			DoViewParts();
		break;
	}

	// Toggle delete
	case char_Backspace:
	case char_Clear:
		// Special case escape key
		if ((inKeyEvent.message & keyCodeMask) == vkey_Escape)
			return CHierarchyTableDrag::HandleKeyPress(inKeyEvent);
		else if (!mLocked)
		{
			DeleteSelection();
			LCommander::SetUpdateCommandStatus(true);
		}
		break;

	default:
		return CHierarchyTableDrag::HandleKeyPress(inKeyEvent);
	}

	return true;
}

//	Pass back status of a (menu) command
void CFileTable::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_Cut:				// Cut, Copy, and Clear enabled
	case cmd_Copy:				//   if something is selected
	case cmd_Paste:
		//outEnabled = IsSelectionValid();
		outEnabled = false;
		break;

	case cmd_Clear:
		outEnabled = !mLocked && IsSelectionValid();
		break;

	// Item selected?
	case cmd_Properties:
		outEnabled = IsSelectionValid();
		break;

	// Available when not locked
	case cmd_NewTextPart:
	case cmd_NewPlainTextPart:
	case cmd_MultipartChoice:
	case cmd_MultipartMixed:
	case cmd_MultipartParallel:
	case cmd_MultipartDigest:
	case cmd_MultipartAlternative:
		outEnabled = !mLocked;
		break;

	case cmd_NewEnrichedTextPart:
	case cmd_NewHTMLTextPart:
		// Only if allowed by admin and not locked
		outEnabled = CAdminLock::sAdminLock.mAllowStyledComposition && !mLocked;
		break;

	default:
		CHierarchyTableDrag::FindCommandStatus(inCommand, outEnabled, outUsesMark,
							outMark, outName);
		break;
	}
}

//	Respond to commands
Boolean CFileTable::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool cmdHandled = true;

	switch (inCommand)
	{
	case cmd_Cut:
	case cmd_Copy:
	case cmd_Paste:
		break;

	case cmd_Clear:
		if (!mLocked)
			DeleteSelection();
		break;

	case cmd_Properties:
		DoEditProperties();
		break;

	case cmd_AttachFile:
		// Only if not locked out
		if (!CAdminLock::sAdminLock.mNoAttachments)
			DoAttachFile();
		break;

	//Justin

	case cmd_NewPlainTextPart:
		DoNewTextPart(eContentSubPlain);
		break;

	case cmd_NewEnrichedTextPart:
		DoNewTextPart(eContentSubEnriched);
		break;

	case cmd_NewHTMLTextPart:
		DoNewTextPart(eContentSubHTML);
		break;

	case cmd_MultipartMixed:
		DoMultipartMixed();
		break;

	case cmd_MultipartParallel:
		DoMultipartParallel();
		break;

	case cmd_MultipartDigest:
		DoMultipartDigest();
		break;

	case cmd_MultipartAlternative:
		DoMultipartAlternative();
		break;

	case msg_TabSelect:
		if (!IsEnabled() || !IsVisible()) {
			cmdHandled = false;
		}
		break;

	default:
		cmdHandled = CHierarchyTableDrag::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

void CFileTable::DrawCell(const STableCell &inCell, const Rect &inLocalRect)
{
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row);

	CAttachment* attach = GetAttachment(inCell.row);

	// Save text state in stack object
	StTextState		textState;
	StColorState	saveColors;
	StColorPenState::Normalize();
	cdstring		theTxt;

	// Set to required text
	UTextTraits::SetPortTextTraits(&mTextTraits);

	// Erase to ensure drag hightlight is overwritten
	::EraseRect(&inLocalRect);

	// Clip to cell frame & table frame
	Rect	clipper = mRevealedRect;
	PortToLocalPoint(topLeft(clipper));
	PortToLocalPoint(botRight(clipper));
	::SectRect(&clipper, &inLocalRect, &clipper);
	StClipRgnState	clip(clipper);

	unsigned long col_type = 0;
	
	switch(inCell.col)
	{
	case 1:
		col_type = mAttachmentsOnly ? eColType_NameIcon : eColType_Diamond;
		break;
	case 2:
		col_type = mAttachmentsOnly ? eColType_Size : eColType_RW;
		break;
	case 3:
		col_type = mAttachmentsOnly ? eColType_MIME : eColType_MIMEIcon;
		break;
	case 4:
		col_type = eColType_Name;
		break;
	case 5:
		col_type = eColType_Size;
		break;
	case 6:
		col_type = eColType_Encoding;
		break;
	}

	switch(col_type)
	{

	case eColType_Diamond:
		Rect	iconRect;
		iconRect.left = inLocalRect.left;
		iconRect.right = iconRect.left + 16;
		iconRect.bottom = inLocalRect.bottom - mIconDescent;
		iconRect.top = iconRect.bottom - 16;
		// Erase if multipart
		if (attach->IsMultipart())
			::EraseRect(&iconRect);

		// Check for diamond type
		else if (woRow == mRowShow)
			::Ploticns(&iconRect, kAlignNone, kTransformNone, ICNx_DiamondTicked);
		else if (attach->CanEdit())
			::Ploticns(&iconRect, kAlignNone, kTransformNone, attach->IsSeen() ? ICNx_DiamondSeen : ICNx_Diamond);
		else
			::Ploticns(&iconRect, kAlignNone, kTransformNone, ICNx_DisabledDiamond);
		break;

	case eColType_RW:
		iconRect.left = inLocalRect.left;
		iconRect.right = iconRect.left + 16;
		iconRect.bottom = inLocalRect.bottom - mIconDescent;
		iconRect.top = iconRect.bottom - 16;
		::Ploticns(&iconRect, kAlignNone, kTransformNone, attach->CanChange() && !mLocked ? ICNx_ReadWrite : ICNx_ReadOnly);
		break;

	case eColType_MIME:
		{
			::MoveTo(inLocalRect.left, inLocalRect.bottom - mTextDescent);
			cdstring content(CMIMESupport::GenerateContentHeader(attach, false, lendl, false));
			::DrawClippedStringUTF8(content, inLocalRect.right - inLocalRect.left, eDrawString_Left);
		}
		break;

	case eColType_MIMEIcon:
		{
			DrawDropFlag(inCell, woRow);

			// Draw selection
			bool selected_state = DrawCellSelection(inCell);

			UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

			// Draw icon followed by encoding
			iconRect.left = inLocalRect.left + mFirstIndent + nestingLevel * mLevelIndent;
			iconRect.right = iconRect.left + 16;
			iconRect.bottom = inLocalRect.bottom - mIconDescent;
			iconRect.top = iconRect.bottom - 16;
			PlotAttachIcon(attach, iconRect, selected_state);

			// Get encoding mode
			cdstring content(CMIMESupport::GenerateContentHeader(attach, false, lendl, false));
			::MoveTo(iconRect.right - 2, inLocalRect.bottom - mTextDescent);
			short width = inLocalRect.right - iconRect.right - 2;
			::DrawClippedStringUTF8(content, width, eDrawString_Left);
		}
		break;

	case eColType_Name:
		if (!attach->IsMultipart())
		{
			::MoveTo(inLocalRect.left, inLocalRect.bottom - mTextDescent);
			cdstring at_name = attach->GetMappedName(true, false);
			if (!at_name.empty())
			{
				::DrawClippedStringUTF8(at_name, inLocalRect.right - inLocalRect.left, eDrawString_Left);
			}
		}
		break;

	case eColType_NameIcon:
		{
			DrawDropFlag(inCell, woRow);

			// Draw selection
			bool selected_state = DrawCellSelection(inCell);

			UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

			// Draw icon followed by encoding
			iconRect.left = inLocalRect.left + mFirstIndent + nestingLevel * mLevelIndent;
			iconRect.right = iconRect.left + 16;
			iconRect.bottom = inLocalRect.bottom - mIconDescent;
			iconRect.top = iconRect.bottom - 16;
			PlotAttachIcon(attach, iconRect, selected_state);

			// Get encoding mode
			::MoveTo(iconRect.right - 2, inLocalRect.bottom - mTextDescent);
			short width = inLocalRect.right - iconRect.right - 2;
			cdstring at_name = attach->GetMappedName(true, false);
			if (!at_name.empty())
			{
				::DrawClippedStringUTF8(at_name, width, eDrawString_Left);
			}
		}
		break;

	case eColType_Size:
		// Draw size
		if (!attach->IsMultipart())
		{
			long size = attach->GetSize();
			theTxt = ::GetNumericFormat(size);
			::MoveTo(inLocalRect.left, inLocalRect.bottom - mTextDescent);
			::DrawClippedStringUTF8(theTxt, inLocalRect.right - inLocalRect.left, eDrawString_Right);
		}
		break;

	case eColType_Encoding:
		if (!attach->IsMultipart() && !attach->IsMessage())
		{
			// Get encoding mode
			theTxt = rsrc::GetIndexedString("UI::Letter::EncodeMode", attach->GetTransferMode());
			::MoveTo(inLocalRect.left, inLocalRect.bottom - mTextDescent);
			::DrawClippedStringUTF8(theTxt, inLocalRect.right - inLocalRect.left, eDrawString_Left);
		}
		break;

	default:
		break;
	}
}

// Plot appropriate icon for attachment
void CFileTable::PlotAttachIcon(CAttachment* attach, Rect& iconRect, bool inHilite)
{
	if (attach->IsApplefile())
		::Ploticns(&iconRect, kAlignNone, inHilite? kTransformSelected : kTransformNone, ICNx_Applefile);
	else if (attach->IsMultipart())
		::Ploticns(&iconRect, kAlignNone, inHilite? kTransformSelected : kTransformNone, ICNx_Multipart);
	else if (attach->IsMessage())
		::Ploticns(&iconRect, kAlignNone, inHilite? kTransformSelected : kTransformNone, ICNx_MessagePart);
	else if (attach->IsCalendar())
		::Ploticns(&iconRect, kAlignNone, inHilite? kTransformSelected : kTransformNone, ICNx_Calendar_Flag);
	else if (attach->GetIconRef()->GetIconRef())
		::PlotIconRef(&iconRect, kAlignNone, inHilite? kTransformSelected : kTransformNone, kIconServicesNormalUsageFlag, attach->GetIconRef()->GetIconRef());
	else
		::Ploticns(&iconRect, kAlignNone, inHilite? kTransformSelected : kTransformNone, ICNx_Unknownfile);
}

// Draw or undraw active hiliting for a Cell
void CFileTable::HiliteCellActively(const STableCell &inCell, Boolean inHilite)
{
	if (!GetBody())
		return;

	CHierarchyTableDrag::HiliteCellActively(inCell, inHilite);
}

// Draw or undraw inactive hiliting for a Cell
void CFileTable::HiliteCellInactively(const STableCell &inCell, Boolean inHilite)
{
	if (!GetBody())
		return;

	CHierarchyTableDrag::HiliteCellInactively(inCell, inHilite);
}

void CFileTable::CalcCellFlagRect(const STableCell &inCell, Rect &outRect)
{
	if (inCell.col == (mAttachmentsOnly ? 1 : 3))
	{
		LHierarchyTable::CalcCellFlagRect(inCell, outRect);
		outRect.right = outRect.left + 16;
		outRect.bottom = outRect.top + 12;
		::OffsetRect(&outRect, 0, -2);
	}
	else
		::SetRect(&outRect, 0, 0, 0, 0);
}

// Show focus box
void CFileTable::BeTarget(void)
{
	if (mBody)
		Activate();
}

// Hide focus box
void CFileTable::DontBeTarget(void)
{
	if (mBody)
		Deactivate();
}

// Make it target first
void CFileTable::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	SwitchTarget(this);
	CHierarchyTableDrag::ClickSelf(inMouseDown);
}

// Click in the cell
void CFileTable::ClickCell(
	const STableCell		&inCell,
	const SMouseDownEvent	&inMouseDown)
{
	// Must check whether the current event is still the one we expect to handle.
	// Its possible that a dialog appeared as a result of the initial click, and now
	// the mouse is no longer down. However WaitMouseMoved will always wait for a mouse up
	// before carrying on.
	EventRecord currEvent;
	LEventDispatcher::GetCurrentEvent(currEvent);
	bool event_match = (inMouseDown.macEvent.what == currEvent.what) &&
						(inMouseDown.macEvent.message == currEvent.message) &&
						(inMouseDown.macEvent.when == currEvent.when) &&
						(inMouseDown.macEvent.where.h == currEvent.where.h) &&
						(inMouseDown.macEvent.where.v == currEvent.where.v) &&
						(inMouseDown.macEvent.modifiers == currEvent.modifiers);

	// Check whether D&D available and over a selected cell or not shift or cmd keys
	if (event_match && (mAttachmentsOnly || (inCell.col != 1)) &&
		DragAndDropIsPresent() &&
		(CellIsSelected(inCell) ||
			(!(inMouseDown.macEvent.modifiers & shiftKey) &&
			 !(inMouseDown.macEvent.modifiers & cmdKey))))
	{

		// Track item long enough to distinguish between a click to
		// select, and the beginning of a drag
		bool isDrag = !CContextMenuProcessAttachment::ProcessingContextMenu() &&
						::WaitMouseMoved(inMouseDown.macEvent.where);

		// Now do drag
		if (isDrag)
		{

			// Force update of current part
			mWindow->SyncPart();

			// If we leave the window, the drag manager will be changing thePort,
			// so we'll make sure thePort remains properly set.
			OutOfFocus(NULL);
			FocusDraw();
			OSErr err = CreateDragEvent(inMouseDown);
			OutOfFocus(NULL);

			return;
		}
	}

	// Do click in different text show
	if (!inMouseDown.delaySelect && !mAttachmentsOnly && (inCell.col == 1))
		ShowPart(inCell.row);

	// If multiclick then view
	if ((GetClickCount() > 1) && !inMouseDown.delaySelect)
	{
		// Try to show otherwise do view
		if (mAttachmentsOnly || !ShowPart(inCell.row))
			DoViewParts();
	}
}

void CFileTable::SetAttachmentsOnly(bool attachments)
{
	if (attachments != mAttachmentsOnly)
	{
		mAttachmentsOnly = attachments;
		
		// Change columns before resetting to ensure
		// data stored in cells is consistent
		TableIndexT new_cols = mAttachmentsOnly ? 3 : 6;
		TableIndexT old_cols = mCols;

		if (old_cols > new_cols)
			RemoveCols(old_cols - new_cols, 1, false);
		else if (old_cols < new_cols)
			InsertCols(new_cols - old_cols, 1, NULL, 0, false);
		
		AdaptToNewSurroundings();
		SetOneColumnSelect(mAttachmentsOnly ? 1 : 3);
		
		// Change titles
		if (mTitles)
		{
			mTitles->SyncTable(this, true);
			mTitles->LoadTitles(mAttachmentsOnly ? "UI::Titles::LetterAttachments" : "UI::Titles::LetterParts", mAttachmentsOnly ? 3 : 6, true);
		}
		if (GetBody())
		{
			CAttachment* attach = GetPartShow();
			ResetTable();
			SetRowShow(attach);
		}
	}
}

// Reset the table from the mbox
void CFileTable::ResetTable(void)
{
	TableIndexT old_show = mRowShow;

	// Delete all existing rows (use inherited function to prevent attachment delete)
	while (mRows)
		CHierarchyTableDrag::RemoveRows(1, mRows, false);

	if (mBody)
	{
		TableIndexT next_row = 1;

		// Add main part
		InsertPart(next_row, mBody, false);

		// Expand top rows only (when not in attachments mode)
		if (IsCollapsable(1) && !mAttachmentsOnly)
			ExpandRow(1);
	}

	// Restore previous row show
	mRowShow = old_show;

	Refresh();
}

TableIndexT CFileTable::InsertPart(TableIndexT& parentRow, CAttachment* part, bool child, int pos)
{
	TableIndexT child_insert = parentRow;

	bool is_attachment = (dynamic_cast<CDataAttachment*>(part) == NULL) || !part->IsMultipart() && !part->CanEdit();

	// Insert this part
	if (!mAttachmentsOnly || is_attachment)
	{
		if (child)
		{
			// Determine position of part and how to insert
			if (pos == 0)
			{
				InsertChildRows(1, parentRow, &part, sizeof(CAttachment*), part->IsMultipart() || part->IsMessage(), false);
				child_insert = parentRow + 1;
			}
			else if (pos < 0)
			{
				AddLastChildRow(parentRow, &part, sizeof(CAttachment*), part->IsMultipart() || part->IsMessage(), false);
				child_insert = parentRow + mCollapsableTree->CountAllDescendents(parentRow);
			}
			else
			{
				TableIndexT woRow = ((CNodeVectorTree*) mCollapsableTree)->SiblingIndex(parentRow, pos);
				child_insert = InsertSiblingRows(1, woRow, &part, sizeof(CAttachment*), part->IsMultipart() || part->IsMessage(), false);
			}
		}
		else
			// Update to new parent
			child_insert = InsertSiblingRows(1, parentRow, &part, sizeof(CAttachment*), part->IsMultipart() || part->IsMessage(), false);
	}

	// Add children (if there)
	if ((part->IsMultipart() || part->IsMessage()) && part->GetParts())
	{
		// Add each sibling
		for(CAttachmentList::iterator iter = part->GetParts()->begin(); iter != part->GetParts()->end(); iter++)
			InsertPart(child_insert, *iter, mRows > 0);
	}

	// Adjust visible part position
	if (child_insert <= mRowShow)
		mRowShow += CountAllDescendents(child_insert) + 1;

	return child_insert;
}

// Handle update of visible part
void CFileTable::DeleteSelection(void)
{
	// Check for delete of visible row
	TableIndexT oldrow = mRowShow;

	// Force update of current part
	mWindow->SyncPart();

	// Do inherited action
	DoToSelection((DoToSelectionPP) &CFileTable::DeleteRow, false);

	// Force change of multipart structure
	if (mBody && ConvertMultipart(false))
		ResetTable();

	// Update row show
	UpdateRowShow();
}

// DeleteRow
bool CFileTable::DeleteRow(TableIndexT row)
{
	TableIndexT	woRow = GetWideOpenIndex(row);
	RemoveRows(1, woRow, true);
	return true;
}

// Remove rows and adjust parts
void CFileTable::RemoveRows(UInt32 inHowMany, TableIndexT inFromRow, Boolean inRefresh)
{
	// Always called with inHowMany == 1

	// Look for visible row in hierarchy to be deleted
	if ((mRowShow >= inFromRow) && (mRowShow <= inFromRow + mCollapsableTree->CountAllDescendents(inFromRow)))
	{
		mRowShow = 0;
		mWindow->mCurrentPart = NULL;
	}

	// Look for change in position of row show
	if (mRowShow > inFromRow + mCollapsableTree->CountAllDescendents(inFromRow))
		mRowShow -= 1 + mCollapsableTree->CountAllDescendents(inFromRow);

	// Get attachment for this row
	STableCell woCell(inFromRow, 1);
	CAttachment* attach;
	UInt32 dataSize = sizeof(CAttachment*);
	GetCellData(woCell, &attach, dataSize);

	// Remove it from its parent (will be deleted)
	if (attach->GetParent())
		attach->GetParent()->RemovePart(attach);
	else
	{
		// Must be root part so delete
		delete mBody;
		mBody = NULL;
		mWindow->SetBody(NULL);
	}

	// Do inherited action
	LHierarchyTable::RemoveRows(inHowMany, inFromRow, inRefresh);

	SetDirty(true);
}

TableIndexT CFileTable::AddPart(CAttachment* attach, CAttachment* parent, int parent_row, int pos, bool refresh)
{
	bool force_reset = false;
	TableIndexT new_woRow = 0;

	if (mBody)
	{
		// Check for existing multipart
		force_reset = ConvertMultipart(true);

		// If changed bump up positions
		if (force_reset)
		{
			// Reset to new parent
			parent = mBody;
			parent_row = 1;
			pos = -1;
			new_woRow = -1;
		}

		// Get default parent
		if (!parent || !parent->CanChange())
		{
			parent = mBody;
			parent_row = 1;
			pos = -1;

			// Check whether only one selected
			if (!mAttachmentsOnly && IsSingleSelection())
			{
				// Check for multipart
				STableCell aCell(0, 0);
				GetFirstSelection(aCell);
				UInt32 woRow = GetWideOpenIndex(aCell.row);

				CAttachment* attach = GetAttachment(aCell.row);

				// Set parent to single multipart selected item
				if (attach->IsMultipart())
				{
					parent = attach;
					parent_row = woRow;
				}
			}
		}

		// Add to parts
		parent->AddPart(attach, pos);
	}
	else
	{
		mBody = attach;
		mWindow->SetBody(mBody);
		parent_row = 0;
		pos = 0;
		new_woRow = 1;
		force_reset = true;
	}

	// Reset table if required
	if (force_reset)
	{
		ResetTable();
		
		// Must adjust new row to the last one that could possibly be
		TableIndexT dummy;
		GetWideOpenTableSize(new_woRow, dummy);
	}
	else
	{
		// Manually insert part as child or sibling (if flat attachment only list)
		TableIndexT temp = parent_row;
		if (mAttachmentsOnly)
		{
			TableIndexT sibling = ((CNodeVectorTree*) mCollapsableTree)->GetSiblingIndex(temp);
			while(sibling != 0)
			{
				temp = sibling;
				sibling = ((CNodeVectorTree*) mCollapsableTree)->GetSiblingIndex(temp);
			}
		}
		new_woRow = InsertPart(temp, attach, !mAttachmentsOnly, pos);
	}

	// Force redraw if required
	if (refresh)
		Refresh();

	SetDirty(true);

	return new_woRow;
}

// Convert between single/multipart
// bool add		: indicates whether a part is about to be added, or has been removed
// returns bool	: true if root part changed (i.e. reset of whole table required
bool CFileTable::ConvertMultipart(bool add)
{
	// Must have a valid body to do this
	if (!mBody)
		return false;

	// Check for single (or unchangeable) part when adding a new part
	if (add && (!mBody->IsMultipart() || !mBody->CanChange()))
	{
		// Create new multipart entity
		CDataAttachment* root = new CDataAttachment();
		root->GetContent().SetContent(eContentMultipart, eContentSubMixed);

		// Add original part to new root
		CAttachment* child = mBody;
		root->AddPart(child);

		// Make body start at new root
		mBody = root;

		// Force window to update
		mWindow->SetBody(mBody, true);

		// Bump up row show if not zero and reset window's current part back to original
		if (mRowShow)
		{
			mRowShow++;
			mWindow->mCurrentPart = child;
		}

		// Force table reset
		return true;
	}

	// Check for multipart with a single or no child part
	else if (!add && mBody->IsMultipart() && (!mBody->GetParts() || (mBody->GetParts()->size() < 2)))
	{
		// Get old root
		CAttachment* old_root = mBody;

		// Get new root from old
		CAttachment* new_root = mBody->GetPart(2);

		// Remove new from old without delete and delete old
		if (new_root)
			old_root->RemovePart(new_root, false);
		delete old_root;

		// Make body start at new root
		mBody = new_root;

		// Force window to update
		mWindow->SetBody(mBody, true);

		// Bump down row show if not zero (window's current part remians the same)
		if (mRowShow)
		{
			mRowShow--;
		}

		// Iterate again
		ConvertMultipart(false);

		// Force table reset
		return true;
	}

	return false;
}

// Find first editable part
void CFileTable::UpdateRowShow(void)
{
	// Reset row show with first viewable if changed
	if (!mAttachmentsOnly && !mRowShow)
	{
		for(int selrow = 1; selrow <= mCollapsableTree->CountNodes(); selrow++)
		{
			STableCell woCell(selrow, 1);
			CAttachment* attach;
			UInt32 dataSize = sizeof(CAttachment*);
			GetCellData(woCell, &attach, dataSize);
			if (attach->CanEdit())
			{
				mRowShow = selrow;
				break;
			}
		}

		// Force window text reset
		mWindow->SetCurrentPart(GetAttachment(mRowShow, true));

		// Refresh new one
		TableIndexT row = GetExposedIndex(mRowShow);
		if (row)
			RefreshRow(row);
	}

	mWindow->UpdatePartsCaption();
}

bool CFileTable::ShowPart(TableIndexT row)
{
	// Do click in different text show
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row);

	CAttachment* attach = GetAttachment(row);

	if ((woRow != mRowShow) && attach->CanEdit())
	{
		// Refresh old one
		TableIndexT old_row = GetExposedIndex(mRowShow);

		// Change value before refresh to ensure no tick-mark
		mRowShow = woRow;
		RefreshRow(old_row);

		// Show chosen part
		mWindow->SetCurrentPart(GetAttachment(mRowShow, true));

		// Refresh new one
		RefreshRow(row);
		
		return true;
	}
	else
		return false;
}

// Test for selected message deleted
bool CFileTable::TestSelectionChangeable(TableIndexT row)
{
	// Is it changeable?
	CAttachment* attach = GetAttachment(row);
	return attach->CanChange();
}

// Test for selected message deleted
bool CFileTable::TestSelectionUnchangeable(TableIndexT row)
{
	// Is it unchangeable?
	CAttachment* attach = GetAttachment(row);
	return !attach->CanChange();
}

// Adjust column widths
void CFileTable::AdaptToNewSurroundings(void)
{
	// Do inherited call
	LHierarchyTable::AdaptToNewSurroundings();

	// Set image to frame size
	ResizeImageTo(mFrameSize.width, mImageSize.height, true);

	if (mAttachmentsOnly)
	{
		// Name column has variable width
		SetColWidth(mFrameSize.width - 260, 1, 1);

		// Remaining columns have fixed width
		SetColWidth(48, 2, 2);
		SetColWidth(212, 3, 3);
	}
	else
	{
		// Name column has variable width
		SetColWidth(mFrameSize.width - 370, 4, 4);

		// Remaining columns have fixed width
		SetColWidth(16, 1, 1);
		SetColWidth(16, 2, 2);
		SetColWidth(212, 3, 3);
		SetColWidth(48, 5, 5);
		SetColWidth(80, 6, 6);
	}

	if (mTitles)
		mTitles->SyncTable(this);
}

// Do properties dialog
void CFileTable::DoEditProperties(void)
{
	bool changed = false;

	// Can only do to selection
	if (!IsSelectionValid())
		return;

	// Test for selection all changeable or all not changeable
	bool all_change = TestSelectionAnd((TestSelectionPP) &CFileTable::TestSelectionChangeable);
	bool all_unchange = TestSelectionAnd((TestSelectionPP) &CFileTable::TestSelectionUnchangeable);

	// All changeable => allow edit
	if (all_change && !mLocked)
	{
		// Get content for first item
		STableCell selCell(0, 0);
		GetNextSelectedCell(selCell);

		UInt32 woRow = GetWideOpenIndex(selCell.row);

		CAttachment* attach = GetAttachment(selCell.row);

		// Make a copy of the content
		CMIMEContent content = attach->GetContent();

		// Modify content if others selected
		while(GetNextSelectedCell(selCell))
		{
			if (selCell.col == 1)
			{
				CAttachment* other_attach = GetAttachment(selCell.row);

				content.NullDiff(other_attach->GetContent());
			}
		}

		// Create the dialog
		CBalloonDialog	theHandler(paneid_LetterPart, this);

		((CLetterPartProp*) theHandler.GetDialog())->SetFields(content);
		theHandler.StartDialog();

		// Let DialogHandler process events
		while (true)
		{
			MessageT hitMessage = theHandler.DoDialog();

			if (hitMessage == msg_OK)
			{
				((CLetterPartProp*) theHandler.GetDialog())->GetFields(content);

				// Copy back into all selected items
				selCell = STableCell(0, 0);
				while(GetNextSelectedCell(selCell))
				{
					if (selCell.col == 1)
					{
						woRow = GetWideOpenIndex(selCell.row);
						CAttachment* attach = GetAttachment(selCell.row);
						if (attach->GetContent().GetContentSubtype() != content.GetContentSubtype())
						{
							changed = true;
						}
						if (((attach->GetContent().GetContentSubtype() == eContentSubEnriched) ||
							(attach->GetContent().GetContentSubtype() == eContentSubHTML)) &&
							(content.GetContentSubtype() == eContentSubPlain))
						{
							if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Letter::WarnLoseFormatting") == CErrorHandler::Cancel)
							{
								break;
							}
						}
						content.NullAdd(attach->GetContent());

						// Need to force attachment name to be re-mapped
						attach->SetName(cdstring::null_str);

						if (changed && (woRow == mRowShow) && !mAttachmentsOnly)
							mWindow->SetCurrentPart(GetAttachment(mRowShow, true));

					}
				}

				RefreshSelection();

				SetDirty(true);
				break;
			}
			else if (hitMessage == msg_Cancel)
				break;
		}
	}

	// All unchangeable => no edit
	else if (all_unchange || mLocked)
	{
		// Special if multiple
		bool multi = !IsSingleSelection();

		bool more = multi;
		STableCell selCell(0, 0);

		do
		{
			// Exit loop if no more
			if (!GetNextSelectedCell(selCell))
				break;

			if (selCell.col == 1)
			{
				CAttachment* attach = GetAttachment(selCell.row);

				// Create the dialog
				CBalloonDialog	theHandler(paneid_MessagePart, this);

				((CMessagePartProp*) theHandler.GetDialog())->SetFields(*attach, multi);
				theHandler.StartDialog();

				// Let DialogHandler process events
				while (true)
				{
					MessageT hitMessage = theHandler.DoDialog();

					if (hitMessage == msg_OK)
						break;
					else if (hitMessage == msg_Cancel)
					{
						more = false;
						break;
					}
				}
			}
		} while (more);
	}
	else
		// Warn user of mixed selection
		CErrorHandler::PutStopAlertRsrc("Alerts::Letter::MixedFileTableSelection");
}

// View selected parts
void CFileTable::DoViewParts()
{
	if (!TestSelectionAnd((TestSelectionPP) &CFileTable::CheckViewPart))
		CErrorHandler::PutStopAlertRsrc("Alerts::Message::CannotDownload");
	else
	{
		DoToSelection((DoToSelectionPP) &CFileTable::ViewPart);
		RefreshSelection();
	}
}

// Check for valid view of specified part
bool CFileTable::CheckViewPart(TableIndexT row)
{
	// Check against admin locks
	CAttachment* attach = GetAttachment(row);
	if (!attach)
		return false;

	// Look for local file attachment - we always allow the user to view these
	// irrespective of admin locks since they can view it via the file system anyway
	CFileAttachment* fattach = dynamic_cast<CFileAttachment*>(attach);

	return (fattach != NULL) || CAdminLock::sAdminLock.CanDownload(attach->GetContent());
}

// View the part
bool CFileTable::ViewPart(TableIndexT row)
{
	CAttachment* attach = GetAttachment(row);
	if (!attach)
		return false;
	CFileAttachment* fattach = dynamic_cast<CFileAttachment*>(attach);

	// Check part size first (only if its not on disk already)
	if ((fattach == NULL) && !CMailControl::CheckSizeWarning(attach))
		return false;

	// View chosen part if cached
	if (!attach->IsNotCached())
	{
		// Behaviour:
		//   file attachments are viewed directly off disk
		//   message attachments are viewed via their owning message
		if (fattach)
			fattach->ViewFile();
		else
		{
		}
	}
	return true;
}

// Make sure list is visible in draft window
void CFileTable::ExposePartsList()
{
	// Check whether twisted or not
	if (!mWindow->IsPartsTwist())
		// twist without changing focus
		mWindow->DoPartsTwist(true, false);
}

// Do file attachment
void CFileTable::DoAttachFile(void)
{
	bool done = false;
	std::auto_ptr<CAttachmentList> attachList(new CAttachmentList);
	attachList->set_delete_data(false);

	// Determine if NavServices available
	{
		LFileTypeList	fileTypes((NavTypeList**) NULL);
		PP_StandardDialogs::LFileChooser	chooser;
		
		chooser.GetDialogOptions()->optionFlags = kNavDefaultNavDlogOptions; // Can select multiple files
		
		done = chooser.AskOpenFile(fileTypes);
		
		if (done)
		{
			for(int i = 1; i <= chooser.GetNumberOfFiles(); i++)
			{
				PPx::FSObject fspec;
				chooser.GetFileSpec(i, fspec);

				// Create new file attachment and add to list
				CFileAttachment* attached = new CFileAttachment(fspec);
				attachList->push_back(attached);
			}
		}
	}
	
	// Was it successful
	if (done)
	{
		bool added = false;

		// Want to select only the new ones
		UnselectAllCells();

		// Add parts to table
		TableIndexT first_added = 0;
		for(CAttachmentList::iterator iter = attachList->begin(); iter != attachList->end(); iter++)
		{
			// Must be unique to add
			if (!mBody || mBody->UniqueFile(*static_cast<CFileAttachment*>(*iter)->GetFSSpec()))
			{
				// Create file attachment and add
				TableIndexT woRow = AddPart(*iter, NULL, 0, 0, false);
				TableIndexT exp_row = GetExposedIndex(woRow);
				
				// Select each one added
				SelectRow(exp_row);
				
				// Cache the first one added for scroll into view
				if (first_added == 0)
					first_added = exp_row;

				added = true;
			}
			else
				delete *iter;
		}

		// Scroll so first one is in the view
		if (added)
			ScrollToRow(first_added, false, false, eScroll_Top);
		
		// Refresh table if change
		if (added)
			Refresh();

		// Clear all items from list as they are either in use or deleted
		attachList->clear();
		
		// Force parts visible
		if (added)
			ExposePartsList();
	}
}

// Do new text attachment
void CFileTable::DoNewTextPart(EContentSubType subType)
{
	// Must always be in full parts mode if we have more than one inline text part
	SetAttachmentsOnly(false);

	TableIndexT old_show = mRowShow;

	CAttachment* tattach = new CDataAttachment((char*) NULL);

	tattach->GetContent().SetContent(eContentText, subType);

	TableIndexT woRow = AddPart(tattach, NULL, 0, 0, true);

	// Refresh old one
	TableIndexT old_row = GetExposedIndex(old_show);
	RefreshRow(old_row);

	// Change value
	mRowShow = woRow;

	// Show new part
	mWindow->SetCurrentPart(GetAttachment(mRowShow, true));
	mWindow->UpdatePartsCaption();

	// Refresh new one
	TableIndexT row = GetExposedIndex(mRowShow);
	RefreshRow(row);
	
	// Select only the new one
	UnselectAllCells();
	SelectRow(row);
	ScrollToRow(row, false, false, eScroll_Top);
	
	// Force parts visible
	ExposePartsList();
}

// Do new multipart attachment
void CFileTable::DoMultipartMixed(void)
{
	DoMultipart(eContentSubMixed);
}

// Do new multipart attachment
void CFileTable::DoMultipartParallel(void)
{
	DoMultipart(eContentSubParallel);
}

// Do new multipart attachment
void CFileTable::DoMultipartDigest(void)
{
	DoMultipart(eContentSubDigest);
}

// Do new multipart attachment
void CFileTable::DoMultipartAlternative(void)
{
	DoMultipart(eContentSubAlternative);
}

// Do new multipart attachment
void CFileTable::DoMultipart(EContentSubType subType)
{
	// Must always be in full parts mode if we add multiparts
	SetAttachmentsOnly(false);

	CDataAttachment* mattach = new CDataAttachment;
	mattach->GetContent().SetContent(eContentMultipart, subType);
	TableIndexT woRow = AddPart(mattach, NULL, 0, 0, true);
	TableIndexT expRow = GetExposedIndex(woRow);
	mWindow->UpdatePartsCaption();
	
	// Select only the new one
	UnselectAllCells();
	SelectRow(expRow);
	ScrollToRow(expRow, false, false, eScroll_Top);
	
	// Force parts visible
	ExposePartsList();
}

// Add whole message as part
void CFileTable::ForwardMessages(CMessageList* msgs, EForwardOptions forward)
{
	for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
	{
		// Check that message has more than one alternative text part
		if (!(*iter)->HasUniqueTextPart() || (forward & eForwardAttachment))
		{
			// Check for forward using message/rfc822 part
			CAttachment* new_attach = NULL;
			if (forward & eForwardRFC822)
				new_attach = new CMessageAttachment(*iter, *iter);
			else
			{
				// Make aliases of original body
				if ((*iter)->GetBody()->IsMessage())
					new_attach = new CMessageAttachment(*iter, (*iter)->GetBody()->GetMessage());
				else
					new_attach = new CAliasAttachment(*iter, (*iter)->GetBody());
			}
			AddPart(new_attach, NULL, 0, 0, true);
		}
	}
	
	// Expose parts if items were added
	if (mRows > 1)
		ExposePartsList();
}

// Add whole message as total
void CFileTable::BounceMessages(CMessageList* msgs)
{
	// Dump existing
	delete mBody;
	SetBody(NULL);
	mWindow->SetBody(NULL);

	// Must always be in full parts mode when bouncing
	SetAttachmentsOnly(false);

	CAttachment* new_attach = NULL;

	for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
	{
		// Make aliases of original body
		if ((*iter)->GetBody()->IsMessage())
			new_attach = new CMessageAttachment(*iter, (*iter)->GetBody()->GetMessage());
		else
			new_attach = new CAliasAttachment(*iter, (*iter)->GetBody());

		AddPart(new_attach, NULL, 0, 0, true);
	}
	
	// Expose parts if items were added
	if (mRows > 1)
		ExposePartsList();
}

// Add whole message as total
void CFileTable::RejectMessages(CMessageList* msgs, bool return_msg)
{
	// Dump existing
	delete mBody;
	SetBody(NULL);
	mWindow->SetBody(NULL);

	// Must always be in full parts mode when rejecting
	SetAttachmentsOnly(false);

	// Can only do this to one message
	if (msgs->size() != 1)
		return;
	
	// Create the DSN message
	CAttachment* dsn = msgs->front()->CreateRejectDSNBody(return_msg);
	
	// Give this to the table
	if (dsn)
		AddPart(dsn, NULL, 0, 0, true);

	UpdateRowShow();
	
	// Lock it to prevent changes to reject
	mLocked = true;
	
	// Prevent drag and drop when locked
	SetDDReadOnly(true);
}

// Add whole message as total with editable bits
void CFileTable::SendAgainMessages(CMessageList* msgs)
{
	// Dump existing
	delete mBody;
	SetBody(NULL);
	mWindow->SetBody(NULL);

	for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
	{
		CAttachment* new_attach = NULL;

		// Clone entire body to editable parts
		// This will take care of signed/encrypted/alternative types
		new_attach = CAttachment::CloneAttachment(*iter, (*iter)->GetBody());

		// Add to list and update visible part
		AddPart(new_attach, NULL, 0, 0, true);
	}

	UpdateRowShow();
	
	// Expose parts if items were added
	if (mRows > 1)
		ExposePartsList();
	
	// We need to reset the display of the current part
	mWindow->SetCurrentPart(mWindow->GetCurrentPart());
}

// Add whole message as part
void CFileTable::DigestMessages(CMessageList* msgs)
{
	// Must always be in full parts mode when rejecting
	SetAttachmentsOnly(false);

	// Add multipart digest
	CDataAttachment* mattach = new CDataAttachment();
	mattach->GetContent().SetContent(eContentMultipart, eContentSubDigest);
	TableIndexT parent_row = AddPart(mattach, NULL, 0, 0, true);

	// Add all messages into digest
	for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
	{
		CMessageAttachment* new_attach = new CMessageAttachment(*iter, *iter);
		AddPart(new_attach, mattach, parent_row, -1, true);
	}
	
	// Expose parts if items were added
	if (mRows > 1)
		ExposePartsList();
}

// Add file from path
void CFileTable::AddFile(const cdstring& file)
{
	// Create new file attachment and add to list
	CFileAttachment* attached = new CFileAttachment(file);

	// Must be unique to add
	if (attached->ValidFile() &&
		(!mBody || mBody->UniqueFile(*attached->GetFSSpec())))
	{
		// Create file attachment and add
		AddPart(attached, NULL, 0, 0, true);

		// Force parts visible
		ExposePartsList();
	}
	else
		delete attached;
}

// Add attachment
void CFileTable::AddAttachment(CDataAttachment* attach)
{
	// Add attachment
	AddPart(attach, NULL, 0, 0, true);

	// Force parts visible
	ExposePartsList();
}

// Server reset
void CFileTable::ServerReset(const CMboxProtocol* proto)
{
	// Find any parts dependent on this message and remove them
	if (mBody && CAttachment::ServerReset(mBody, proto))
	{
		ResetTable();
	}
}

// Mailbox reset
void CFileTable::MailboxReset(const CMbox* mbox)
{
	// Find any parts dependent on this message and remove them
	if (mBody && CAttachment::MailboxReset(mBody, mbox))
	{
		ResetTable();
	}
}

// Message removed
void CFileTable::MessageRemoved(const CMessage* msg)
{
	// Find any parts dependent on this message and remove them
	if (mBody && CAttachment::MessageRemoved(mBody, msg))
	{
		ResetTable();
	}
}

// Get attachment from row
CAttachment* CFileTable::GetAttachment(TableIndexT row, bool is_worow)
{
	if (row == 0)
		return NULL;

	TableIndexT	woRow = is_worow ? row : GetWideOpenIndex(row);

	STableCell	woCell(woRow, 1);
	CAttachment* attach;
	UInt32 dataSize = sizeof(CAttachment*);
	GetCellData(woCell, &attach, dataSize);
	
	return attach;
}

// Add drag cells
void CFileTable::AddCellToDrag(CDragIt* theDragTask, const STableCell& theCell, Rect& dragRect)
{
	// Add this message to drag
	CAttachment* attach = GetAttachment(theCell.row);
	theDragTask->AddFlavorItem(dragRect, (ItemReference) attach, cDragAtchType,
								&attach, sizeof(CAttachment*), flavorSenderOnly, true);
}

// Current part changed
void CFileTable::ChangedCurrent(void)
{
	// Refresh new one

	if (!mAttachmentsOnly)
	{
		TableIndexT row = GetExposedIndex(mRowShow);
		RefreshRow(row);
		mWindow->SetCurrentPart(GetAttachment(mRowShow, true));
	}
	else
		mWindow->SetCurrentPart(mWindow->GetCurrentPart());
}

// Test drop into cell
bool CFileTable::IsDropCell(DragReference inDragRef, STableCell aCell)
{
	// Look for drop on self
	if (sTableSource == this)
	{
		// Make sure row is not selected

		if (CellIsSelected(aCell))
			return false;

		// Make sure parent row is not selected
		int woRow = GetWideOpenIndex(aCell.row);
		while((woRow = GetParentIndex(woRow)) > 0)
		{
			STableCell parent_cell(GetExposedIndex(woRow), 1);
			if (CellIsSelected(parent_cell))
				return false;
		}
	}

	// Only drop into changable multipart part
	int woRow = GetWideOpenIndex(aCell.row);
	CAttachment* attach = GetAttachment(aCell.row);
	return IsCollapsable(woRow) && attach->CanChange();
}

// Test drop at cell
bool CFileTable::IsDropAtCell(DragReference inDragRef, STableCell& aCell)
{
	// Do not drop above top row
	if (aCell.row == 1)
		return false;

	TableIndexT parent_row = 0;
	CAttachment* parent = NULL;
	TableIndexT pos = 0;
	GetDropAtParent(aCell.row, parent_row, parent, pos);
	if (!parent && (aCell.row <= mRows))
		return false;

	// Look for drop on self
	if ((sTableSource == this) && parent_row)
	{
		// Make sure parent row is not selected
		int woRow = parent_row;
		do
		{
			STableCell parent_cell(GetExposedIndex(woRow), 1);
			if (CellIsSelected(parent_cell))
				return false;
		} while((woRow = GetParentIndex(woRow)) > 0);
	}

	// Must be OK if we get here
	return true;
}

// Drop data into whole table
// Occurs when table is empty before drop
void CFileTable::DropData(FlavorType theFlavor,
										char* drag_data,
										Size data_size)
{
	// Do fake DropAtRow for top row
	STableCell aCell(1, 1);

	bool added = false;

	TableIndexT parent_row = 0;
	CAttachment* parent = NULL;
	TableIndexT pos = -1;

	if (theFlavor == cDragMsgType)
	{
		CMessage* theMsg = *((CMessage**) drag_data);
		CMessageAttachment* new_attach = new CMessageAttachment(theMsg, theMsg);
		AddPart(new_attach, parent, parent_row, pos, false);
		added = true;
	}
	else if (theFlavor == cDragAtchType)
	{
		CAttachment* theAtch = *((CAttachment**) drag_data);
		CAttachment* new_attach = CAttachment::CopyAttachment(*theAtch);
		AddPart(new_attach, parent, parent_row, pos, false);
		added = true;
	}
	else if (theFlavor == cDragMsgAtchType)
	{
		CMessage* owner = *((CMessage**) drag_data);
		drag_data += sizeof(CMessage*);
		CAttachment* theAtch = *((CAttachment**) drag_data);
		if (theAtch->IsMessage())
		{
			CMessageAttachment* new_attach = new CMessageAttachment(owner, theAtch->GetMessage());		// Make alias, not copy
			AddPart(new_attach, parent, parent_row, pos, false);
		}
		else
		{
			CAliasAttachment* new_attach = new CAliasAttachment(owner, theAtch);		// Make alias, not copy
			AddPart(new_attach, parent, parent_row, pos, false);
		}
		added = true;
	}
	
	// Only if not locked out
	else if ((theFlavor == flavorTypeHFS) && !CAdminLock::sAdminLock.mNoAttachments)
	{
		HFSFlavor hfs = *((HFSFlavor*) drag_data);
		CFileAttachment* theAtch = new CFileAttachment(hfs.fileSpec);
		AddPart(theAtch, parent, parent_row, pos, false);
		added = true;
	}

	// Refresh table if change
	if (added)
	{
		UpdateRowShow();
		Refresh();
	}
}

// Drop data into cell
void CFileTable::DropDataIntoCell(FlavorType theFlavor,
										char* drag_data,
										Size data_size,
										const STableCell& theCell)
{
	bool added = false;

	// Only allow drag to group
	int	woRow = GetWideOpenIndex(theCell.row);
	if (!IsCollapsable(woRow))
		return;

	// Get attachment to drop into
	CAttachment* parent = GetAttachment(theCell.row);

	if (theFlavor == cDragMsgType)
	{
		CMessage* theMsg = *((CMessage**) drag_data);
		CMessageAttachment* new_attach = new CMessageAttachment(theMsg, theMsg);
		AddPart(new_attach, parent, woRow, -1, false);
		added = true;
	}
	else if (theFlavor == cDragAtchType)
	{
		CAttachment* theAtch = *((CAttachment**) drag_data);
		CAttachment* new_attach = CAttachment::CopyAttachment(*theAtch);
		AddPart(new_attach, parent, woRow, -1, false);
		added = true;
	}
	else if (theFlavor == cDragMsgAtchType)
	{
		CMessage* owner = *((CMessage**) drag_data);
		drag_data += sizeof(CMessage*);
		CAttachment* theAtch = *((CAttachment**) drag_data);
		if (theAtch->IsMessage())
		{
			CMessageAttachment* new_attach = new CMessageAttachment(owner, theAtch->GetMessage());		// Make alias, not copy
			AddPart(new_attach, parent, woRow, -1, false);
		}
		else
		{
			CAliasAttachment* new_attach = new CAliasAttachment(owner, theAtch);		// Make alias, not copy
			AddPart(new_attach, parent, woRow, -1, false);
		}
		added = true;
	}
	
	// Only if not locked out
	else if ((theFlavor == flavorTypeHFS) && !CAdminLock::sAdminLock.mNoAttachments)
	{
		HFSFlavor hfs = *((HFSFlavor*) drag_data);
		CFileAttachment* theAtch = new CFileAttachment(hfs.fileSpec);
		AddPart(theAtch, parent, woRow, -1, false);
		added = true;
	}

	// Refresh table if change
	if (added)
	{
		UpdateRowShow();
		Refresh();
	}
}

// Drop data at cell
void CFileTable::DropDataAtCell(FlavorType theFlavor,
										char* drag_data,
										Size data_size,
										const STableCell& beforeCell)
{
	bool added = false;

	// Get location of drop cell or first cell if beyond the start
	int	woRow = GetWideOpenIndex(beforeCell.row - 1);
	if (!woRow)
		woRow = GetWideOpenIndex(1);

	TableIndexT old_total = mRows;

	TableIndexT parent_row = 0;
	CAttachment* parent = NULL;
	TableIndexT pos = 0;
	TableIndexT row = beforeCell.row;
	GetDropAtParent(row, parent_row, parent, pos);

	if (theFlavor == cDragMsgType)
	{
		CMessage* theMsg = *((CMessage**) drag_data);
		CMessageAttachment* new_attach = new CMessageAttachment(theMsg, theMsg);
		AddPart(new_attach, parent, parent_row, pos, false);
		added = true;
	}
	else if (theFlavor == cDragAtchType)
	{
		CAttachment* theAtch = *((CAttachment**) drag_data);
		CAttachment* new_attach = CAttachment::CopyAttachment(*theAtch);
		AddPart(new_attach, parent, parent_row, pos, false);
		added = true;
	}
	else if (theFlavor == cDragMsgAtchType)
	{
		CMessage* owner = *((CMessage**) drag_data);
		drag_data += sizeof(CMessage*);
		CAttachment* theAtch = *((CAttachment**) drag_data);
		if (theAtch->IsMessage())
		{
			CMessageAttachment* new_attach = new CMessageAttachment(owner, theAtch->GetMessage());		// Make alias, not copy
			AddPart(new_attach, parent, parent_row, pos, false);
		}
		else
		{
			CAliasAttachment* new_attach = new CAliasAttachment(owner, theAtch);		// Make alias, not copy
			AddPart(new_attach, parent, parent_row, pos, false);
		}
		added = true;
	}
	
	// Only if not locked out
	else if ((theFlavor == flavorTypeHFS) && !CAdminLock::sAdminLock.mNoAttachments)
	{
		HFSFlavor hfs = *((HFSFlavor*) drag_data);
		CFileAttachment* theAtch = new CFileAttachment(hfs.fileSpec);
		AddPart(theAtch, parent, parent_row, pos, false);
		added = true;
	}

	// Refresh table if change
	if (added)
	{
		UpdateRowShow();
		Refresh();

		// Must update drop at position to take into account new item
		int add = (mRows - old_total > 1) ? 2 : 1;
		mLastDropCursor.row = std::min(mLastDropCursor.row + add, mRows);
		
	}
}

// Get parent of cell for drop at
// returns bool	: indicates whether original drop parent is used or not
void CFileTable::GetDropAtParent(TableIndexT& at_row, TableIndexT& parent_row, CAttachment*& parent, TableIndexT& pos)
{
	// Get location of drop cell or first cell if beyond the start
	int	woRow = GetWideOpenIndex(at_row - 1);	// This is row before cursor
	if (!woRow)
		woRow = GetWideOpenIndex(1);

	parent_row = 0;
	parent = NULL;
	pos = 0;
	if (IsCollapsable(woRow) && IsExpanded(woRow))
	{
		parent_row = woRow;
		STableCell woCell(woRow, 1);
		UInt32 dataSize = sizeof(CAttachment*);
		GetCellData(woCell, &parent, dataSize);
		pos = 0;
	}
	else
	{
		// Get attachment to drop into
		// Note if there is only a single part it will be magically converted into a multipart
		parent_row = GetParentIndex(woRow);
		if (parent_row)
		{
			STableCell woCell(parent_row, 1);
			UInt32 dataSize = sizeof(CAttachment*);
			GetCellData(woCell, &parent, dataSize);
			pos = ((CNodeVectorTree*) mCollapsableTree)->SiblingPosition(woRow);
		}
		else
		{
			parent_row = 0;
			parent = NULL;
			pos = -1;
		}
	}

	// Now check that parent is changeable
	if (parent && !parent->CanChange())
	{
		// Find parent's parent that is changeable
		CAttachment* next = parent->GetParent();
		int next_row = GetParentIndex(parent_row);
		while(next && !next->CanChange())
		{
			parent = next;
			parent_row = next_row;
			next = parent->GetParent();
			next_row = GetParentIndex(parent_row);
		}

		// Found a parent that will accept new attachment
		if (next)
		{
			// New insert position is at child parent
			at_row = parent_row;

			// Find position for insert
			pos = ((CNodeVectorTree*) mCollapsableTree)->SiblingPosition(parent_row);
			parent = next;
			parent_row = next_row;
		}
		else
		{
			parent_row = 0;
			parent = NULL;
			pos = -1;

			// Force to last row
			at_row = mRows + 1;
		}
	}
}
