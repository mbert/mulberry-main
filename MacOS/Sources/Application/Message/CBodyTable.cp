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


//	CBodyTable.cp

#include "CBodyTable.h"

#include "CAdminLock.h"
#include "CAttachment.h"
#include "CAttachmentList.h"
#include "CBalloonDialog.h"
#include "CBetterScrollerX.h"
#include "CCommands.h"
#include "CContextMenu.h"
#include "CDesktopIcons.h"
#include "CDragIt.h"
#include "CErrorDialog.h"
#include "CErrorHandler.h"
#include "CMailControl.h"
#include "CMessage.h"
#include "CMessagePartProp.h"
#include "CMessageView.h"
#include "CMessageWindow.h"
#include "CMulberryCommon.h"
#include "CNetworkException.h"
#include "CPreferences.h"
#include "CResources.h"
#include "CStaticText.h"
#include "CStringResources.h"
#include "CTableMultiRowSelector.h"
#include "CTableRowGeometry.h"

#include <LDropFlag.h>
#include <LNodeArrayTree.h>
#include <LTableArrayStorage.h>

#include <stdio.h>

CBodyTable::CBodyTable(LStream	*inStream)
	: CHierarchyTableDrag(inStream)

{
	mTableGeometry = new CTableRowGeometry(this, mFrameSize.width, 18);
	mTableSelector = new CTableMultiRowSelector(this);
	mTableStorage = new LTableArrayStorage(this, sizeof(CAttachment*));
	mBody = NULL;
	mWindow = NULL;
	mView = NULL;
	mRowShow = 0;
	mFlat = true;
	mDragInProgress = false;
}


CBodyTable::~CBodyTable()
{
}

// Get details of sub-panes
void CBodyTable::FinishCreateSelf()
{
	// Do inherited
	CHierarchyTableDrag::FinishCreateSelf();

	// Find window in super view chain
	LView* super = GetSuperView();
	while(super && !dynamic_cast<CMessageWindow*>(super))
		super = super->GetSuperView();
	mWindow = dynamic_cast<CMessageWindow*>(super);

	// Find view in super view chain
	super = GetSuperView();
	while(super && !dynamic_cast<CMessageView*>(super))
		super = super->GetSuperView();
	mView = dynamic_cast<CMessageView*>(super);

	super = (mWindow ? (LView*) mWindow : (LView*) mView);

	mContentTitle = (CStaticText*) super->FindPaneByID(paneid_MessageAttachContentTitle);
	cdstring temp;
	temp.FromResource("UI::Message::PartsContentFlat");
	mContentTitle->SetText(temp);

	// Create columns and adjust flag rect
	InsertCols(5, 1, NULL, 0, false);
	SetOneColumnSelect(3);
	AdaptToNewSurroundings();
	SetRect(&mFlagRect, 2, 5, 18, 17);

	// Get text traits resource
	SetTextTraits(CPreferences::sPrefs->mListTextTraits.GetValue().traits);

	// Set Drag & Drop pane to scroller
	mPane = GetSuperView();

	// Set Drag & Drop info
	SetTable(this, false);
	AddDragFlavor(flavorTypeHFS);
	AddDragFlavor(cDragMsgAtchType);

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
Boolean CBodyTable::HandleKeyPress(const EventRecord &inKeyEvent)
{
	switch (inKeyEvent.message & charCodeMask) {

		// Open the message
		case char_Return:
		case char_Enter:
		{
			// Try to show it
			STableCell aCell;
			GetFirstSelection(aCell);
			if (IsSelectionValid() &&
				(!IsSingleSelection() || !ShowPart(aCell.row)) &&
				TestSelectionOr((TestSelectionPP) &CBodyTable::TestSelectionCached))
				// Just extract it
				DoExtractParts(CPreferences::sPrefs->mViewDoubleClick.GetValue());
			break;
		}

		default:
			return CHierarchyTableDrag::HandleKeyPress(inKeyEvent);
			break;
	}

	return true;
}

//	Pass back status of a (menu) command
void CBodyTable::FindCommandStatus(
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
	case cmd_Clear:
	case cmd_Paste:
		//outEnabled = IsSelectionValid();
		outEnabled = false;
		break;

	// Item selected?
	case cmd_Properties:
		outEnabled = IsSelectionValid();
		break;

	case cmd_ViewPartsMessage:
	case cmd_ExtractPartsMessage:
		outEnabled = TestSelectionOr((TestSelectionPP) &CBodyTable::TestSelectionCached);
		break;

	default:
		CHierarchyTableDrag::FindCommandStatus(inCommand, outEnabled, outUsesMark,
							outMark, outName);
		break;
	}
}

//	Respond to commands
Boolean CBodyTable::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool	cmdHandled = true;

	switch (inCommand)
	{
	case cmd_Cut:
	case cmd_Copy:
	case cmd_Paste:
	case cmd_Clear:
		break;

	case cmd_Properties:
		DoProperties();
		break;

	case cmd_ViewPartsMessage:
		DoExtractParts(true);
		break;

	case cmd_ExtractPartsMessage:
		DoExtractParts(false);
		break;

	case msg_TabSelect:
		if (!IsEnabled() || !IsVisible())
			cmdHandled = false;
		break;

	default:
		cmdHandled = CHierarchyTableDrag::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

// Test for selected cached attachments
bool CBodyTable::TestSelectionCached(TableIndexT row)
{
	// Is it cached?
	CAttachment* attach = GetAttachment(row);
	if (!attach)
		return false;

	return !attach->IsNotCached();
}

void CBodyTable::DrawCell(const STableCell &inCell, const Rect &inLocalRect)
{
	if (!GetBody())
		return;

	CAttachment* attach = GetAttachment(inCell.row);
	if (!attach)
		return;

	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row);

	// Save text state in stack object
	StTextState		textState;
	StColorState	saveColors;
	StColorPenState::Normalize();
	cdstring		theTxt;

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
		else if (attach->CanDisplay())
			::Ploticns(&iconRect, kAlignNone, kTransformNone, attach->IsSeen() ? ICNx_DiamondSeen : ICNx_Diamond);
		else if (attach->IsMessage())
			::Ploticns(&iconRect, kAlignNone, kTransformNone, attach->IsSeen() ? ICNx_MessageSeen : ICNx_MessageNotSeen);
		else
			::Ploticns(&iconRect, kAlignNone, kTransformNone, ICNx_DisabledDiamond);
		break;

	case 2:
		iconRect.left = inLocalRect.left;
		iconRect.right = iconRect.left + 16;
		iconRect.bottom = inLocalRect.bottom - mIconDescent;
		iconRect.top = iconRect.bottom - 16;
		// Check for tick
		if (attach->IsExtracted() && (attach->IsApplefile() || (!attach->IsMultipart() && !attach->IsMessage())))
			::Ploticns(&iconRect, kAlignNone, kTransformNone, ICNx_Tickmark);
		else
			::EraseRect(&iconRect);
		break;

	case 3:
		{
			// Special if multipart/appledouble and flat
			CAttachment* do_attach = attach;
			if (mFlat && attach->IsApplefile() && attach->IsMultipart() && attach->GetParts())
			{
				// Use first part not equal to applefile
				for(CAttachmentList::iterator iter = attach->GetParts()->begin(); iter != attach->GetParts()->end(); iter++)
				{
					if (!(*iter)->IsApplefile())
					{
						do_attach = *iter;
						break;
					}
				}
			}

			DrawDropFlag(inCell, woRow);

#if PP_Target_Carbon
			// Draw selection
			bool selected_state = DrawCellSelection(inCell);
#else
			bool selected_state = false;
#endif

			UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

			// Draw icon followed by encoding
			iconRect.left = inLocalRect.left + mFirstIndent + nestingLevel * mLevelIndent;
			iconRect.right = iconRect.left + 16;
			iconRect.bottom = inLocalRect.bottom - mIconDescent;
			iconRect.top = iconRect.bottom - 16;
			PlotAttachIcon(do_attach, iconRect, selected_state);

			// Get encoding mode
			cdstring content(CMIMESupport::GenerateContentHeader(do_attach, false, lendl, false));
			::MoveTo(iconRect.right - 2, inLocalRect.bottom - mTextDescent);
			short width = inLocalRect.right - iconRect.right - 2;
			::DrawClippedStringUTF8(content, width, eDrawString_Left);
		}
		break;

	case 4:
		// Draw size
		if (!attach->IsMultipart() || mFlat)
		{
			long size;
			if (!attach->IsMessage())
			{
				if (mFlat && attach->IsMultipart())
					size = attach->GetTotalSize(true);
				else
					size = attach->GetSize();
			}
			else
				size = attach->GetMessage()->GetSize();
			theTxt = ::GetNumericFormat(size);
			::MoveTo(inLocalRect.left, inLocalRect.bottom - mTextDescent);
			::DrawClippedStringUTF8(theTxt, inLocalRect.right - inLocalRect.left, eDrawString_Right);
		}
		break;

	case 5:
		if (!attach->IsMultipart() && !attach->IsMessage())
		{
			::MoveTo(inLocalRect.left, inLocalRect.bottom - mTextDescent);
			cdstring at_name = attach->GetMappedName(true, true);
			if (!at_name.empty())
			{
				::DrawClippedStringUTF8(at_name, inLocalRect.right - inLocalRect.left, eDrawString_Left);
			}
		}
		else if (attach->IsMultipart() && attach->IsApplefile())
		{
			::MoveTo(inLocalRect.left, inLocalRect.bottom - mTextDescent);
			cdstring ad_name = attach->GetMappedName(true, true);
			if (!ad_name.empty())
			{
				::DrawClippedStringUTF8(ad_name, inLocalRect.right - inLocalRect.left, eDrawString_Left);
			}
		}
		else if (attach->IsMessage())
		{
			::MoveTo(inLocalRect.left, inLocalRect.bottom - mTextDescent);
			if (!attach->GetMessage()->GetEnvelope()->GetSubject().empty())
			{
				cdstring name(attach->GetMessage()->GetEnvelope()->GetSubject());
				::DrawClippedStringUTF8(name, inLocalRect.right - inLocalRect.left, eDrawString_Left);
			}
		}
		break;

	default:
		break;
	}
}

// Plot appropriate icon for attachment
void CBodyTable::PlotAttachIcon(CAttachment* attach, Rect& iconRect, bool inHilite)
{
	if (attach->IsNotCached())
		::Ploticns(&iconRect, kAlignNone, inHilite? kTransformSelected : kTransformNone, ICNx_MissingPart);
	else if (attach->IsApplefile())
		::Ploticns(&iconRect, kAlignNone, inHilite? kTransformSelected : kTransformNone, ICNx_Applefile);
	else if (attach->IsMultipart())
		::Ploticns(&iconRect, kAlignNone, inHilite? kTransformSelected : kTransformNone, ICNx_Multipart);
	else if (attach->IsMessage())
		::Ploticns(&iconRect, kAlignNone, inHilite? kTransformSelected : kTransformNone, ICNx_MessagePart);
	else if (attach->IsCalendar())
		::Ploticns(&iconRect, kAlignNone, inHilite? kTransformSelected : kTransformNone, ICNx_Calendar_Flag);
	else if (attach->GetIconRef()->GetIconRef() != NULL)
		::PlotIconRef(&iconRect, kAlignNone, inHilite? kTransformSelected : kTransformNone, kIconServicesNormalUsageFlag, attach->GetIconRef()->GetIconRef());
	else
		::Ploticns(&iconRect, kAlignNone, inHilite? kTransformSelected : kTransformNone, ICNx_Unknownfile);
}

// Draw or undraw active hiliting for a Cell
void CBodyTable::HiliteCellActively(const STableCell &inCell, Boolean inHilite)
{
	if (!GetBody())
		return;

	CHierarchyTableDrag::HiliteCellActively(inCell, inHilite);
}

// Draw or undraw inactive hiliting for a Cell
void CBodyTable::HiliteCellInactively(const STableCell &inCell, Boolean inHilite)
{
	if (!GetBody())
		return;

	CHierarchyTableDrag::HiliteCellInactively(inCell, inHilite);
}

void CBodyTable::CalcCellFlagRect(const STableCell &inCell, Rect &outRect)
{
	if (inCell.col == 3)
	{
		LHierarchyTable::CalcCellFlagRect(inCell, outRect);
		outRect.right = outRect.left + 16;
		outRect.bottom = outRect.top + 12;
		::OffsetRect(&outRect, 0, -2);
	}
	else
		::SetRect(&outRect, 0, 0, 0, 0);
}

// Make it target first
void CBodyTable::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	SwitchTarget(this);
	CHierarchyTableDrag::ClickSelf(inMouseDown);
}

// Click in the cell
void CBodyTable::ClickCell(
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
	if (event_match && DragAndDropIsPresent() &&
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
			// Don't allow drag if MIME type blocked
			bool worow = false;
			if (!TestSelectionAnd1((TestSelection1PP) &CBodyTable::CheckExtractPart, &worow))
				CErrorHandler::PutStopAlertRsrc("Alerts::Message::CannotDownload");

			// Don't allow drag if all selected items are non cached
			else if (TestSelectionOr((TestSelectionPP) &CBodyTable::TestSelectionCached))
			{
				// If we leave the window, the drag manager will be changing thePort,
				// so we'll make sure thePort remains properly set.
				OutOfFocus(NULL);
				FocusDraw();
				OSErr err = CreateDragEvent(inMouseDown);
				OutOfFocus(NULL);
			}
			return;
		}
	}

	// Do click in different text show
	if (!inMouseDown.delaySelect && (inCell.col == 1))
		ShowPart(inCell.row);

	// If multiclick then view
	if ((GetClickCount() > 1) && !inMouseDown.delaySelect)
		DoExtractParts(CPreferences::sPrefs->mViewDoubleClick.GetValue());
}

// Reset the table from the mbox
void CBodyTable::ResetTable()
{
	// Delete all existing rows
	Clear();

	// May not have a body
	if (!GetBody())
	{
		Refresh();
		return;
	}

	TableIndexT next_row = 0;

	if (GetBody()->IsMultipart())
	{
		// Check for 'special' types of multiparts that must be treated as a whole
		if (GetBody()->IsApplefile())
			// Add this part
			InsertPart(next_row, GetBody(), false);
		else if (GetBody()->GetParts())
		{
			// Just add each sibling
			for(CAttachmentList::iterator iter = GetBody()->GetParts()->begin(); iter != GetBody()->GetParts()->end(); iter++)
				InsertPart(next_row, *iter, false);
		}
	}
	else if (GetBody()->IsMessage())
	{
		// Add this part
		InsertPart(next_row, GetBody(), false);
	}
	else
		InsertSiblingRows(1, 0, &mBody, sizeof(CAttachment*), false, false);

	Refresh();
}

void CBodyTable::InsertPart(TableIndexT& parentRow, CAttachment* part, bool child)
{
	TableIndexT thisRow;

	bool has_multiparts = part->IsMultipart() || part->IsMessage();
	bool is_aggregate = part->IsMultipart() && part->IsApplefile();

	// Insert this part
	if (!mFlat || !has_multiparts || is_aggregate)
	{
		if (child)
			thisRow = AddLastChildRow(parentRow, &part, sizeof(CAttachment*), !mFlat && has_multiparts, false);
		else
			parentRow = thisRow = InsertSiblingRows(1, parentRow, &part, sizeof(CAttachment*), !mFlat && has_multiparts, false);
	}

	// Add children
	if (part->IsMultipart() && (!mFlat || !is_aggregate))
	{
		// Add each sibling
		if (part->GetParts())
		{
			for(CAttachmentList::iterator iter = part->GetParts()->begin(); iter != part->GetParts()->end(); iter++)
				InsertPart(thisRow, *iter, !mFlat);
		}
		
		// Adjust position if flat
		if (mFlat)
			parentRow = thisRow;
	}

	// Add sub-messages parts
	else if (part->IsMessage())
	{
		// Get sub-message's main part
		CAttachment* submsg_body = part->GetMessage()->GetBody();
		InsertPart(thisRow, submsg_body, !mFlat);
		
		// Adjust position if flat
		if (mFlat)
			parentRow = thisRow;
	}
}

void CBodyTable::SetBody(CAttachment* aBody)
{
	mBody = aBody;

	// Reset table based on new body
	ResetTable();
}

void CBodyTable::ClearBody()
{
	// NB This may be called via a backgrun thread so it must not do UI
	mBody = NULL;
}

void CBodyTable::SetFlat(bool flat)
{
	if (flat != mFlat)
	{
		mFlat = flat;
		cdstring temp;
		temp.FromResource(flat ? "UI::Message::PartsContentFlat" : "UI::Message::PartsContentHierarchic");
		mContentTitle->SetText(temp);
		if (GetBody())
		{
			CAttachment* attach = GetPartShow();
			ResetTable();
			SetRowShow(attach);
		}
	}
}

void CBodyTable::SetRowShow(CAttachment* attach)
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
	
	mRowShow = 0;
}

CAttachment* CBodyTable::GetPartShow()
{
	STableCell	woCell(mRowShow, 1);
	CAttachment* attach = NULL;
	UInt32 dataSize = sizeof(CAttachment*);
	GetCellData(woCell, &attach, dataSize);
	
	return attach;
}

// Adjust column widths
void CBodyTable::AdaptToNewSurroundings()
{
	// Do inherited call
	LHierarchyTable::AdaptToNewSurroundings();

	// Set image to frame size
	ResizeImageTo(mFrameSize.width, mImageSize.height, true);

	// Name column has variable width
	SetColWidth(mFrameSize.width - 318, 5, 5);

	// Remaining columns have fixed width
	SetColWidth(16, 1, 1);
	SetColWidth(16, 2, 2);
	SetColWidth(238, 3, 3);
	SetColWidth(48, 4, 4);

} // CBodyTable::AdaptToNewSurroundings

// Do properties dialog
void CBodyTable::DoProperties()
{
	// Special if multiple
	bool multi = !IsSingleSelection();

	bool more = multi;
	STableCell	aCell(0, 0);

	do
	{
		// Exit loop if no more
		if (!GetNextSelectedCell(aCell))
			break;

		if (aCell.col == 1)
		{
			CAttachment* attach = GetAttachment(aCell.row);
			if (!attach)
				continue;

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

// Extract selected parts
void CBodyTable::DoExtractParts(bool view)
{
	bool worow = false;
	if (!TestSelectionAnd1((TestSelection1PP) &CBodyTable::CheckExtractPart, &worow))
		CErrorHandler::PutStopAlertRsrc("Alerts::Message::CannotDownload");
	else
	{
		DoToSelection2((DoToSelection2PP) &CBodyTable::ExtractPart, &view, &worow);
		RefreshSelection();
	}
}

// Extract current part
void CBodyTable::DoExtractCurrentPart(bool view)
{
	if (mRowShow == 0)
		return;

	bool worow = true;
	if (!CheckExtractPart(mRowShow, &worow))
		CErrorHandler::PutStopAlertRsrc("Alerts::Message::CannotDownload");
	else
	{
		ExtractPart(mRowShow, &view, &worow);
		TableIndexT exposed = GetExposedIndex(mRowShow);
		if (exposed)
			RefreshRow(exposed);
	}
}

// Check for valid extract of specified part
bool CBodyTable::CheckExtractPart(TableIndexT row, bool* worow)
{
	// Check against admin locks
	CAttachment* attach = GetAttachment(row, *worow);
	if (!attach)
		return false;

	return CAdminLock::sAdminLock.CanDownload(attach->GetContent());
}

// Extract the part
bool CBodyTable::ExtractPart(TableIndexT row, bool* view, bool* worow)
{
	CAttachment* attach = GetAttachment(row, *worow);
	if (!attach)
		return false;

	// Check part size first (only if no drag)
	if (!mDragInProgress && !CMailControl::CheckSizeWarning(attach))
		return false;

	// Extract chosen part if cached
	if (!attach->IsNotCached())
	{
		if (mWindow)
			mWindow->ExtractPart(attach, *view);
		else
			mView->ExtractPart(attach, *view);
	}
	return true;
}

bool CBodyTable::ShowPart(TableIndexT row)
{
	// Do click in different text show
	STableCell inCell(row, 1);
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row);

	CAttachment* attach = GetAttachment(row);
	if (!attach)
		return false;

	if ((woRow != mRowShow) && attach->CanDisplay())
	{
		bool do_show = true;

		// Check part size first
		if (!CMailControl::CheckSizeWarning(attach))
			do_show = false;

		if (do_show)
		{
			// Refresh old one
			TableIndexT old_row = GetExposedIndex(mRowShow);
			STableCell refresh(inCell);
			refresh.row = old_row;

			// Change value before refrech to turn off tick-mark
			mRowShow = woRow;
			RefreshCell(refresh);

			// Show chosen part
			if (mWindow)
				mWindow->ShowPart(attach);
			else
				mView->ShowPart(attach);

			// Refresh new one
			RefreshCell(inCell);
		}
		
		return do_show;
	}

	// Check for sub-message display
	else if (attach->IsMessage())
	{
		// Get rectangle for this point
		Rect row_rect;
		GetLocalRowRect(inCell.row, row_rect);
		LocalToPortPoint(topLeft(row_rect));
		LocalToPortPoint(botRight(row_rect));
		PortToGlobalPoint(topLeft(row_rect));
		PortToGlobalPoint(botRight(row_rect));

		if (mWindow)
			mWindow->ShowSubMessage(attach, row_rect);
		else
			mView->ShowSubMessage(attach, row_rect);

		// Refresh current one
		RefreshCell(inCell);
		
		return true;
	}
	
	return (woRow == mRowShow);
}

// Get attachment from row
CAttachment* CBodyTable::GetAttachment(TableIndexT row, bool worow)
{
	if (!GetBody())
		return NULL;

	TableIndexT	woRow = worow ? row : mCollapsableTree->GetWideOpenIndex(row);

	STableCell	woCell(woRow, 1);
	CAttachment* attach;
	UInt32 dataSize = sizeof(CAttachment*);
	GetCellData(woCell, &attach, dataSize);
	
	return attach;
}

// Add drag cells
void CBodyTable::AddCellToDrag(CDragIt* theDragTask, const STableCell& theCell, Rect& dragRect)
{
	PromiseHFSFlavor phfs;

	phfs.fileType = '****';
	phfs.fileCreator = '****';
	phfs.fdFlags = 0;
	phfs.promisedFlavor = cDragPromiseFSSpec;

	// Add this attachment to drag
	theDragTask->AddFlavorItem(dragRect, (ItemReference) theCell.row, flavorTypePromiseHFS,
								&phfs, sizeof(PromiseHFSFlavor), flavorNotSaved, true);
	theDragTask->AddFlavorItem(dragRect, (ItemReference) theCell.row, cDragPromiseFSSpec,
								0L, 0L, flavorNotSaved, false);
	theDragTask->AddFlavorItem(dragRect, (ItemReference) theCell.row, cDragMsgAtchType,
								0L, 0L, 0, false);
}

// Other flavor requested by receiver
void CBodyTable::DoDragSendData(
	FlavorType		inFlavor,
	ItemReference	inItemRef,
	DragReference	inDragRef)
{
	if (inFlavor == flavorTypePromiseHFS)
	{

		PromiseHFSFlavor phfs;

		phfs.fileType = '****';
		phfs.fileCreator = '****';
		phfs.fdFlags = 0;
		phfs.promisedFlavor = cDragPromiseFSSpec;

		// Give drag this data
		ThrowIfOSErr_(::SetDragItemFlavorData(inDragRef, inItemRef, flavorTypePromiseHFS,
										&phfs, sizeof(PromiseHFSFlavor), 0L));
	}
	else if (inFlavor == cDragPromiseFSSpec)
	{
		FSRef dropLoc;
		ThrowIfOSErr_(::GetDropDirectory(inDragRef, &dropLoc));

		try
		{
			CAttachment::sDropLocation = &dropLoc;
			// Extract attachment to disk at location
			StErrorCritical critical;
			//mDragInProgress = true;
			bool view = false;
			bool worow = false;
			ExtractPart((TableIndexT) inItemRef, &view, &worow);
			//mDragInProgress = false;
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Clean up and throw up
			CAttachment::sDropLocation = NULL;

			CLOG_LOGRETHROW;
			throw;
		}
		CAttachment::sDropLocation = NULL;

		// Give drag this data
		ThrowIfOSErr_(::SetDragItemFlavorData(inDragRef, inItemRef, cDragPromiseFSSpec,
										&dropLoc, sizeof(FSSpec), 0L));
	}
	else if (inFlavor == cDragMsgAtchType)
	{

		CAttachment* attach = GetAttachment((TableIndexT) inItemRef);
		CMessage* theMsg = (mWindow ? mWindow->GetMessage() : mView->GetMessage());

		// Give drag this data
		ThrowIfOSErr_(::SetDragItemFlavorData(inDragRef, inItemRef, cDragMsgAtchType,
												&theMsg, sizeof(CMessage*), 0L));
		ThrowIfOSErr_(::SetDragItemFlavorData(inDragRef, inItemRef, cDragMsgAtchType,
												&attach, sizeof(CAttachment*), 4L));
	}
	else
		Throw_(badDragFlavorErr);
}
