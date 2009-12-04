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


// Source for CMailboxTable class

#include "CMailboxTable.h"

#include "CAddress.h"
#include "CAddressList.h"
#include "CCommands.h"
#include "CContextMenu.h"
#include "CCopyToMenu.h"
#include "CDragIt.h"
#include "CDragTasks.h"
#include "CEnvelope.h"
#include "CErrorHandler.h"
#include "CMailControl.h"
#include "CMailboxToolbarPopup.h"
#include "CMailboxView.h"
#include "CMbox.h"
#include "CMboxRef.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "COptionClick.h"
#include "CPreferences.h"
#include "CResources.h"
#include "CStatusWindow.h"
#include "CStringResources.h"
#include "CTableMultiRowSelector.h"
#include "CTableRowGeometry.h"
#include "CUserAction.h"

#include <UGAColorRamp.h>

// Consts

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

// Static variables
bool CMailboxTable::sDropOnMailbox = false;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxTable::CMailboxTable()
{
	InitMailboxTable();
}

// Default constructor - just do parents' call
CMailboxTable::CMailboxTable(LStream *inStream)
		: CTableDrag(inStream)
{
	InitMailboxTable();
}

// Default destructor
CMailboxTable::~CMailboxTable()
{
}

// Do common init
void CMailboxTable::InitMailboxTable(void)
{
	// Zero out
	mMbox = NULL;
	mMboxError = false;
	mListChanging = false;
	mUpdateRequired = false;
	mSelectionPreserved = false;
	mDeferSelectionChanged = false;
	mResetTable = false;
	mIsSelectionValid = false;
	mTestSelectionAndDeleted = false;

	mTableGeometry = new CTableRowGeometry(this, 72, 16);
	mTableSelector = new CTableMultiRowSelector(this);

	mUpdating = false;
	mPreviewUID = 0;

	SetRowSelect(true);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CMailboxTable::FinishCreateSelf(void)
{
	// Do inherited
	CTableDrag::FinishCreateSelf();

	// Find table view in super view chain
	LView* super = GetSuperView();
	while(super && !dynamic_cast<CMailboxView*>(super))
		super = super->GetSuperView();
	mTableView = dynamic_cast<CMailboxView*>(super);

	// Turn on tooltips
	EnableTooltips();

	// Make it fit to the superview
	AdaptToNewSurroundings();
}

// Handle key presses
Boolean CMailboxTable::HandleKeyPress(
	const EventRecord	&inKeyEvent)
{
	// Look for preview/full view based on key stroke
	char key_press = (inKeyEvent.message & charCodeMask);
	if (key_press == char_Enter)
		key_press = char_Return;

	// Determine whether preview is triggered
	CKeyModifiers mods(inKeyEvent.modifiers);
	const CUserAction& preview = mTableView->GetPreviewAction();
	if ((preview.GetKey() == key_press) &&
		(preview.GetKeyModifiers() == mods))
	{
		PreviewMessage();
		return true;
	}

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if ((fullview.GetKey() == key_press) &&
		(fullview.GetKeyModifiers() == mods))
	{
		DoFileOpenMessage(mods.Get(CKeyModifiers::eAlt));
		return true;
	}

	switch (inKeyEvent.message & charCodeMask)
	{
		// Spacebar with option key gives non-breaking space
		case char_Space:
		case kNonBreakingSpaceCharCode:
			HandleSpacebar(mods.Get(CKeyModifiers::eShift));
			break;

		// Toggle delete
		case char_Backspace:
		case char_Clear:
			// Special case escape key
			if ((inKeyEvent.message & keyCodeMask) == vkey_Escape)
				return CTableDrag::HandleKeyPress(inKeyEvent);
			else if (mIsSelectionValid && GetMbox() && GetMbox()->HasAllowedFlag(NMessage::eDeleted))
			{
				DoFlagMailMessage(NMessage::eDeleted);
				LCommander::SetUpdateCommandStatus(true);
			}
			break;

		// Hierarchy arrows left/right
		case char_LeftArrow:
		case char_RightArrow:
			if (GetMbox() && (GetMbox()->GetSortBy() == cSortMessageThread))
			{
				STableCell aCell(0, 0);
				GetFirstSelection(aCell);
				if (aCell.row)
				{
					// Get message
					const CMessage* thisMsg = GetMbox()->GetMessage(aCell.row, true);
					if (thisMsg)
					{
						// Move up/down hierarchy to sibling
						const CMessage* theMsg = ((inKeyEvent.message & charCodeMask) == char_LeftArrow) ?
									thisMsg->GetThreadPrevious() : thisMsg->GetThreadNext();
						
						// Go up to parent if no previous
						if (((inKeyEvent.message & charCodeMask) == char_LeftArrow) && !theMsg)
							theMsg = thisMsg->GetThreadParent();

						// Only if parent exists
						if (theMsg)
						{
							aCell.row = GetMbox()->GetMessageIndex(const_cast<CMessage*>(theMsg), true);

							// Select and bring into middle of view if outside
							ScrollToRow(aCell.row, true, true, eScroll_Center);
						}
					}
				}
			}
			break;
		
		default:
			return CTableDrag::HandleKeyPress(inKeyEvent);
	}

	return true;
}

// Respond to commands
Boolean CMailboxTable::ObeyCommand(CommandT inCommand,void *ioParam)
{
	ResIDT	menuID;
	SInt16	menuItem;

	bool	cmdHandled = true;

	if (IsSyntheticCommand(inCommand, menuID, menuItem))
	{

		// Show then select the requested window
		if (CCopyToMenu::IsCopyToMenu(menuID))
		{
			CMbox* mbox = NULL;
			if (CCopyToMenu::GetMbox(menuID, menuItem, mbox))
				DoMessageCopy(mbox, COptionMenu::sOptionKey);
			cmdHandled = true;
		}
		else
			cmdHandled = CTableDrag::ObeyCommand(inCommand, ioParam);
	}
	else
	{
		switch (inCommand)
		{
		case cmd_Save:
		case cmd_ToolbarFileSaveBtn:
			OnFileSave();
			break;

		case cmd_Print:
		case cmd_ToolbarMessagePrintBtn:
			DoPrintMailMessage();
			break;

		case cmd_OpenMessage:
			DoFileOpenMessage(COptionMenu::sOptionKey);
			break;

		case cmd_Cut:
		case cmd_Copy:
		case cmd_Paste:
		case cmd_Clear:
			break;

		case cmd_ExpungeMailbox:
		case cmd_ToolbarMailboxExpungeBtn:
			DoMailboxExpunge();
			break;

		case cmd_SortMailboxTo:
			// Change sort by
			mTableView->SetSortBy(cSortMessageTo);
			break;

		case cmd_SortMailboxFrom:
			// Change sort by
			mTableView->SetSortBy(cSortMessageFrom);
			break;

		case cmd_SortMailboxReplyTo:
			// Change sort by
			mTableView->SetSortBy(cSortMessageReplyTo);
			break;

		case cmd_SortMailboxSender:
			// Change sort by
			mTableView->SetSortBy(cSortMessageSender);
			break;

		case cmd_SortMailboxCC:
			// Change sort by
			mTableView->SetSortBy(cSortMessageCc);
			break;

		case cmd_SortMailboxSubject:
			// Change sort by
			mTableView->SetSortBy(cSortMessageSubject);
			break;

		case cmd_SortMailboxThread:
			// Change sort by
			mTableView->SetSortBy(cSortMessageThread);
			break;

		case cmd_SortMailboxDateSent:
			// Change sort by
			mTableView->SetSortBy(cSortMessageDateSent);
			break;

		case cmd_SortMailboxDateReceived:
			// Change sort by
			mTableView->SetSortBy(cSortMessageDateReceived);
			break;

		case cmd_SortMailboxSize:
			// Change sort by
			mTableView->SetSortBy(cSortMessageSize);
			break;

		case cmd_SortMailboxFlags:
			// Change sort by
			mTableView->SetSortBy(cSortMessageFlags);
			break;

		case cmd_SortMailboxNumber:
			// Change sort by
			mTableView->SetSortBy(cSortMessageNumber);
			break;

		case cmd_SortMailboxSmart:
			// Change sort by
			mTableView->SetSortBy(cSortMessageSmart);
			break;

		case cmd_SortMailboxAttachment:
			// Change sort by
			mTableView->SetSortBy(cSortMessageAttachment);
			break;

		case cmd_SortMailboxParts:
			// Change sort by
			mTableView->SetSortBy(cSortMessageParts);
			break;

		case cmd_SortMailboxMatch:
			// Change sort by
			mTableView->SetSortBy(cSortMessageMatching);
			break;

		case cmd_SortMailboxDisconnected:
			// Change sort by
			mTableView->SetSortBy(cSortMessageDisconnected);
			break;

		case cmd_ToolbarMessageCopyPopup:
		case cmd_ToolbarMessageCopyOptionPopup:
		{
			CMbox* mbox = NULL;
			if (mTableView->GetCopyBtn() && mTableView->GetCopyBtn()->GetSelectedMbox(mbox, true))
				DoMessageCopy(mbox, inCommand == cmd_ToolbarMessageCopyOptionPopup);
			break;
		}

		case cmd_CopyNowMessage:
		case cmd_ToolbarMessageCopyBtn:
		case cmd_ToolbarMessageCopyOptionBtn:
			{
				bool option_key = COptionMenu::sOptionKey;
				if (inCommand == cmd_ToolbarMessageCopyBtn)
					option_key = false;
				else if (inCommand == cmd_ToolbarMessageCopyOptionBtn)
					option_key = true;
		
				CMbox* mbox = NULL;
				if (mTableView->GetCopyBtn() && mTableView->GetCopyBtn()->GetSelectedMbox(mbox))
				{
					if (mbox && (mbox != (CMbox*) -1L))
						DoMessageCopy(mbox, option_key);
					else if (!mbox)
						CErrorHandler::PutStopAlertRsrc("Alerts::Message::MissingCopyTo");
				}
			}
			break;

		case cmd_MessageCopyExplicit:
			return ObeyCommand(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? cmd_ToolbarMessageCopyOptionBtn : cmd_ToolbarMessageCopyBtn, NULL);
		
		case cmd_MessageMoveExplicit:
			return ObeyCommand(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? cmd_ToolbarMessageCopyBtn : cmd_ToolbarMessageCopyOptionBtn, NULL);

		case cmd_DeleteMessage:
		case cmd_ToolbarMessageDeleteBtn:
			DoFlagMailMessage(NMessage::eDeleted);
			break;

		case cmd_SpeakSelection:
			// Only do something if speech class does not handle it
			if (!CSpeechSynthesis::ObeyCommand(inCommand, ioParam))
				DoSpeakMessage();
			break;

		case cmd_CaptureAddress:
			DoCaptureAddress();
			break;

		default:
			cmdHandled = CTableDrag::ObeyCommand(inCommand, ioParam);
				break;
		}
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CMailboxTable::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	ResIDT	menuID;
	SInt16	menuItem;

	outUsesMark = false;

	if (IsSyntheticCommand(inCommand, menuID, menuItem))
	{
		// Check for copy to
		if (CCopyToMenu::IsCopyToMenu(menuID))
			outEnabled = true;
		else
			CTableDrag::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
	}
	else
	{
		switch (inCommand)
		{
		// These ones are never on
		case cmd_Cut:
		case cmd_Copy:
		case cmd_Paste:
		case cmd_Clear:
			outEnabled = false;
			break;

		// Must have some undeleted
		case cmd_Save:
		case cmd_ToolbarFileSaveBtn:
		case cmd_Print:
		case cmd_ToolbarMessagePrintBtn:
		case cmd_OpenMessage:
		case cmd_CopyToMessage:
		case cmd_CopyNowMessage:
		case cmd_ToolbarMessageCopyBtn:
		case cmd_ToolbarMessageCopyOptionBtn:
		case cmd_MessageCopyExplicit:
		case cmd_MessageMoveExplicit:

			outEnabled = false;

			// Enable menu only if there's a selection
			if (mIsSelectionValid)
			{

				// Also only if at least one is not deleted
				outEnabled = !mTestSelectionAndDeleted ||
								CPreferences::sPrefs->mOpenDeleted.GetValue();
			}
			
			switch(inCommand)
			{
			case cmd_CopyToMessage:
			{
				LStr255 txt1(STRx_Standards, CPreferences::sPrefs->deleteAfterCopy.GetValue() ? str_MoveToMailbox : str_CopyToMailbox);
				::PLstrcpy(outName, txt1);
				break;
			}
			case cmd_CopyNowMessage:
			{
				LStr255 txt1(STRx_Standards, CPreferences::sPrefs->deleteAfterCopy.GetValue() ? str_MoveNow : str_CopyNow);
				::PLstrcpy(outName, txt1);
				break;
			}
			case cmd_ToolbarMessageCopyBtn:
			case cmd_ToolbarMessageCopyOptionBtn:
			{
				LStr255 txt1(STRx_Standards, CPreferences::sPrefs->deleteAfterCopy.GetValue() ? str_MoveTo : str_CopyTo);
				::PLstrcpy(outName, txt1);
				break;
			}
			default:;
			}
			break;

		// These must have some deleted
		case cmd_ExpungeMailbox:
		case cmd_ToolbarMailboxExpungeBtn:
			outEnabled = GetMbox() && GetMbox()->AnyDeleted() && !GetMbox()->IsReadOnly();
			break;

		// These ones require a selection
		case cmd_DeleteMessage:
		case cmd_ToolbarMessageDeleteBtn:
			if (GetMbox() && !GetMbox()->HasAllowedFlag(NMessage::eDeleted))
			{
				outEnabled = false;
				LStr255 txt(STRx_Standards, str_Delete);
				::PLstrcpy(outName, txt);
			}
			else
			{
				outEnabled = mIsSelectionValid;
				LStr255 txt(STRx_Standards, (mTestSelectionAndDeleted ? str_Undelete : str_Delete));
				::PLstrcpy(outName, txt);
				if (mIsSelectionValid && (inCommand == cmd_ToolbarMessageDeleteBtn))
				{
					outUsesMark = true;
					outMark = mTestSelectionAndDeleted ? (UInt16)checkMark : (UInt16)noMark;
				}
			}
			break;

		case cmd_SortMailbox:
			outEnabled = true;
			break;

		case cmd_SortMailboxTo:
		case cmd_SortMailboxFrom:
		case cmd_SortMailboxReplyTo:
		case cmd_SortMailboxSender:
		case cmd_SortMailboxCC:
		case cmd_SortMailboxSubject:
		case cmd_SortMailboxThread:
		case cmd_SortMailboxDateSent:
		case cmd_SortMailboxDateReceived:
		case cmd_SortMailboxSize:
		case cmd_SortMailboxFlags:
		case cmd_SortMailboxNumber:
		case cmd_SortMailboxSmart:
		case cmd_SortMailboxAttachment:
		case cmd_SortMailboxParts:
		case cmd_SortMailboxMatch:
		case cmd_SortMailboxDisconnected:
			outEnabled = true;
			outUsesMark = true;
			if (GetMbox() && (GetMbox()->GetSortBy() == cSortMessageTo + inCommand - cmd_SortMailboxTo))
				outMark = checkMark;
			else
				outMark = noMark;
			break;

		case cmd_SpeakSelection:
			if (!CSpeechSynthesis::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName))
			{
				// Enable menu only if there's a selection
				if (mIsSelectionValid)
				{

					// Also only if at least one is not deleted
					outEnabled = !mTestSelectionAndDeleted ||
									CPreferences::sPrefs->mOpenDeleted.GetValue();
				}
				::PLstrcpy(outName, LStr255(STRx_Speech, str_SpeakSelection));
			}
			break;

		case cmd_CaptureAddress:
			outEnabled = mIsSelectionValid;
			break;

		// Pass up
		default:
			CTableDrag::FindCommandStatus(inCommand, outEnabled, outUsesMark,
								outMark, outName);
			break;
		}
	}
}

// Respond to clicks in the icon buttons
void CMailboxTable::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
	case msg_MailboxSortDirection:
		mTableView->SetShowBy((*(long*) ioParam == 0) ? cShowMessageAscending : cShowMessageDescending);
		break;
	}
}

// Test each cell in the selection using logical AND
bool CMailboxTable::TestSelectionIgnore(TestSelectionIgnorePP proc, bool and_it)
{
	// Get first selected row (must be non-zero for a valid selection)
	TableIndexT firstRow = GetFirstSelectedRow();
	if (firstRow == 0)
		return false;

	// Get last selected row
	TableIndexT lastRow = GetLastSelectedRow();

	bool test = and_it ? true : false;

	// Iterate from first to last finding all in the range that are selected
	STableCell aCell(firstRow, 1);
	for(; aCell.row <= lastRow; aCell.row++)
	{
		if (CellIsSelected(aCell))
		{
			std::pair<bool, bool> result = (this->*proc)(aCell.row);
			if (result.second)
				test = (and_it ? (result.first && test) : (result.first || test));
		}
	}

	return test;
}

// Test each cell in the selection using logical AND
bool CMailboxTable::TestSelectionIgnore1(TestSelectionIgnore1PP proc, NMessage::EFlags flag, bool and_it)
{
	// Get first selected row (must be non-zero for a valid selection)
	TableIndexT firstRow = GetFirstSelectedRow();
	if (firstRow == 0)
		return false;

	// Get last selected row
	TableIndexT lastRow = GetLastSelectedRow();

	bool test = and_it ? true : false;

	// Iterate from first to last finding all in the range that are selected
	STableCell aCell(firstRow, 1);
	for(; aCell.row <= lastRow; aCell.row++)
	{
		if (CellIsSelected(aCell))
		{
			std::pair<bool, bool> result = (this->*proc)(aCell.row, flag);
			if (result.second)
				test = (and_it ? (result.first && test) : (result.first || test));
		}
	}

	return test;
}

#pragma mark ____________________________Click & Draw

// Click in the cell
void CMailboxTable::ClickCell(
	const STableCell&		inCell,
	const SMouseDownEvent	&inMouseDown)
{
	if (!GetMbox())
		return;

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
	if (event_match && DragAndDropIsPresent() && CellIsSelected(inCell))
	{

		// Track item long enough to distinguish between a click to
		// select, and the beginning of a drag
		bool isDrag = !CContextMenuProcessAttachment::ProcessingContextMenu() &&
						::WaitMouseMoved(inMouseDown.macEvent.where);

		// Now do drag
		if (isDrag)
		{
			// Check delete state here
			bool delete_test = !mTestSelectionAndDeleted;

			// Get list of selected rows before copy which may change them
			ulvector nums;
			GetSelectedRows(nums);

			// Must use unsorted numbers between to IMAP commands
			ulvector actual_nums;
			GetMbox()->MapSorted(actual_nums, nums, true);

			// Need to preserve message selection as Copy operation may result in table reset
			ulvector preserve;
			for(ulvector::const_iterator iter = actual_nums.begin(); iter != actual_nums.end(); iter++)
				preserve.push_back(reinterpret_cast<unsigned long>(GetMbox()->GetMessage(*iter)));

			// Reset table reset flag
			mResetTable = false;

			// Set flag so we know if messages were dropped on another mailbox
			sDropOnMailbox = false;

			// If we leave the window, the drag manager will be changing thePort,
			// so we'll make sure thePort remains properly set.
			OutOfFocus(NULL);
			FocusDraw();
			short mouseUpModifiers;
			OSErr err = CreateDragEvent(inMouseDown, &mouseUpModifiers);
			OutOfFocus(NULL);

			// Now execute the drag task
			CDragTask::Execute();

			// Now see if delete after copy required - only if not DD read-only
			if (!err && !mReadOnly)
			{
				bool option_key = mouseUpModifiers & optionKey;

				// If drag OK and drop on another mailbox then delete selection if required and if not all already deleted
				if (sDropOnMailbox && (CPreferences::sPrefs->deleteAfterCopy.GetValue() ^ option_key) && delete_test &&
					GetMbox()->HasAllowedFlag(NMessage::eDeleted))
				{
					// Look for possible table reset and redo message number array
					if (mResetTable)
					{
						// Redo message numbers for delete operation
						actual_nums.clear();
						for(ulvector::const_iterator iter = preserve.begin(); iter != preserve.end(); iter++)
						{
							// See if message still exists and if so where
							unsigned long index = GetMbox()->GetMessageIndex(reinterpret_cast<CMessage*>(*iter));
							if (index)
								actual_nums.push_back(index);	
						}
					}
					
					// Set deleted flag on chosen messages
					GetMbox()->SetFlagMessage(actual_nums, false, NMessage::eDeleted, true, false);
				}
			}

			return;
		}
	}

	// If multiclick and not delayed then display message
	if ((GetClickCount() > 1) && !inMouseDown.delaySelect)
		DoDoubleClick(inCell.row, inCell.col, CKeyModifiers(inMouseDown.macEvent.modifiers));
	else
		DoSingleClick(inCell.row, inCell.col, CKeyModifiers(inMouseDown.macEvent.modifiers));
}

// Click - deselect if not in cell
void CMailboxTable::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	// Allow make this the target when clicked
	SwitchTarget(this);

	// Find where the click occurred
	STableCell	hitCell;
	SPoint32	imagePt;

	LocalToImagePoint(inMouseDown.whereLocal, imagePt);

	if (GetCellHitBy(imagePt, hitCell))
	{
		if (ClickSelect(hitCell, inMouseDown))
			ClickCell(hitCell, inMouseDown);

	} else
	{							// Click is outside of any Cell
		// Remove selection and update buttons
		UnselectAllCells();
	}
}

void CMailboxTable::DrawSelf()
{
	DrawBackground();

		// Determine cells that need updating. Rather than checking
		// on a cell by cell basis, we just see which cells intersect
		// the bounding box of the update region. This is relatively
		// fast, but may result in unnecessary cell updates for
		// non-rectangular update regions.

	RgnHandle	localUpdateRgnH = GetLocalUpdateRgn();
	Rect updateRect;
	::GetRegionBounds(localUpdateRgnH, &updateRect);
	::DisposeRgn(localUpdateRgnH);

	STableCell	topLeftCell, botRightCell;
	FetchIntersectingCells(updateRect, topLeftCell, botRightCell);

	// Only if open and not changing
	if (mTableView->IsOpen() && !mListChanging)
	{
		// Save text state in stack object
		StTextState		textState;
		StColorState	saveColors;
		StColorPenState::Normalize();

		// Set to required text
		UTextTraits::SetPortTextTraits(&mTextTraits);

		// Draw each cell within the update rect
		for (TableIndexT row = topLeftCell.row; row <= botRightCell.row; row++)
			DrawRow(row, topLeftCell.col, botRightCell.col);
	}
	
	// Trigger update aftr current op ends
	else if (mTableView->IsOpen() && mListChanging)
		mUpdateRequired = true;

#if !PP_Target_Carbon
	HiliteSelection(IsActive(), true);
#endif
}

// Draw the titles
void CMailboxTable::DrawRow(TableIndexT row, TableIndexT start_col, TableIndexT stop_col)
{
	if (!GetMbox())
		return;

	// Allow get message/envelope to fail during caching process
	CMessage* theMsg = GetMbox()->GetMessage(row, true);
	if (!theMsg) return;

	// Check whether already cached
	if (!theMsg->IsFullyCached())
	{
		// do not allow caching while processing another network command
		if (GetMbox()->GetMsgProtocol() && !GetMbox()->GetMsgProtocol()->_get_mutex().is_locked())
		{
			// This will cause caching

			StValueChanger<bool> _change(mListChanging, true);

			{
				StValueChanger<ETriState> _change(mVisible, triState_Off);

				{
					// Preserve selection without scroll
					StMailboxTableSelection preserve_selection(this, false);

					theMsg = GetMbox()->GetCacheMessage(row, true);
				}

				UpdateItems();

				OutOfFocus(NULL);
				FocusDraw();
			}

			// If sorting by anything other than by message number, must do entire refresh
			//if (GetMbox()->GetSortBy() != cSortMessageNumber)
			{
				Refresh();
				FocusDraw();
			}

			// Reset to required text font (may have been changed during network background ops)
			UTextTraits::SetPortTextTraits(&mTextTraits);
		}
		else
		{
			// Force refresh of cell for when protocol is unblocked
			RefreshRow(row);

			// Need to reset focus as refresh may upset it
			OutOfFocus(NULL);
			FocusDraw();
			return;
		}
	}

	// Check highlight state
	StColorState	saveColors;		// Save previous state in case of change
	RGBColor text_color = {0, 0, 0};
	Style text_style = normal;
	bool strikeThrough = false;

	// Select appropriate color and style of text
	if (UEnvironment::HasFeature(env_SupportsColor))
		GetDrawStyle(theMsg, text_color, text_style, strikeThrough);

	for(TableIndexT col = start_col; col <= stop_col; col++)
	{
		STableCell inCell(row, 0);
		inCell.col = col;

		Rect	inLocalRect;
		GetLocalCellRect(inCell, inLocalRect);

		{
			{
				// Clip to cell frame & table frame
				Rect	clipper = mRevealedRect;
				PortToLocalPoint(topLeft(clipper));
				PortToLocalPoint(botRight(clipper));
				::SectRect(&clipper,&inLocalRect,&clipper);
				StClipRgnState	clip(clipper);
				StColorState	saveColors;		// Save previous state in case of change

				// Check for background
				if (UsesBackground(theMsg))
				{
					FocusDraw();
					Rect greyer = inLocalRect;
					greyer.bottom = greyer.top + 1;
					::EraseRect(&greyer);
					RGBColor temp = GetBackground(theMsg);
					::RGBBackColor(&temp);
					greyer = inLocalRect;
					greyer.top++;
					::EraseRect(&greyer);
				}

#if PP_Target_Carbon
				// Draw selection
				DrawCellSelection(inCell);
#endif

				// Move to origin for text
				::MoveTo(inLocalRect.left, inLocalRect.bottom - mTextDescent);

				// Draw the actual cell data
				DrawMessage(theMsg, inCell, inLocalRect, text_color, text_style, strikeThrough);
			}
		}
	}
}

// Draw the titles
void CMailboxTable::DrawMessage(const CMessage* aMsg, const STableCell& inCell, const Rect &inLocalRect,
										RGBColor text_color, Style text_style, bool strike_through)
{
	cdstring		theTxt;
	ResIDT			smart_flag = 0;
	bool			multi = false;
	unsigned long	depth = 0;

	const CEnvelope* theEnv = aMsg->GetEnvelope();
	if (!theEnv)
		return;

	ResIDT plot_flag = 0;
	bool do_plot = false;

	// Determine which heading it is
	SColumnInfo col_info = mTableView->GetColumnInfo()[inCell.col - 1];

	// Determine which heading it is and draw it
	bool right_just = false;
	switch (col_info.column_type)
	{

		case eMboxColumnSmart:
			// Set up the icons
			if (aMsg->IsSmartFrom())
				smart_flag = ICNx_SmartFrom_Flag;
			else if (aMsg->IsSmartTo())
				smart_flag = ICNx_SmartTo_Flag;
			else if (aMsg->IsSmartCC())
				smart_flag = ICNx_SmartCC_Flag;
			else
				smart_flag = ICNx_SmartList_Flag;

			// Check for message From current user
			if (aMsg->IsSmartFrom())
			{
				// Display first To address
				if (theEnv->GetTo()->size())
					theTxt = theEnv->GetTo()->front()->GetNamedAddress();
				multi = (theEnv->GetTo()->size() > 1);
			}
			else
			{
				// Display first from address (or if empty first to address)
				if (theEnv->GetFrom()->size())
				{
					theTxt = theEnv->GetFrom()->front()->GetNamedAddress();
					multi = (theEnv->GetFrom()->size() > 1);
				}
				else
				{
					if (theEnv->GetTo()->size())
						theTxt = theEnv->GetTo()->front()->GetNamedAddress();
					multi = (theEnv->GetTo()->size() > 1);
					smart_flag = ICNx_SmartFrom_Flag;
				}
			}
			break;

		case eMboxColumnFrom:
			if (theEnv->GetFrom()->size())
				theTxt = theEnv->GetFrom()->front()->GetNamedAddress();
			multi = (theEnv->GetFrom()->size() > 1);
			break;

		case eMboxColumnTo:
			if (theEnv->GetTo()->size())
				theTxt = theEnv->GetTo()->front()->GetNamedAddress();
			multi = (theEnv->GetTo()->size() > 1);
			break;

		case eMboxColumnReplyTo:
			if (theEnv->GetReplyTo()->size())
				theTxt = theEnv->GetReplyTo()->front()->GetNamedAddress();
			multi = (theEnv->GetReplyTo()->size() > 1);
			break;

		case eMboxColumnSender:
			if (theEnv->GetSender()->size())
				theTxt = theEnv->GetSender()->front()->GetNamedAddress();
			multi = (theEnv->GetSender()->size() > 1);
			break;

		case eMboxColumnCc:
			if (theEnv->GetCC()->size())
				theTxt = theEnv->GetCC()->front()->GetNamedAddress();
			multi = (theEnv->GetCC()->size() > 1);
			break;

		case eMboxColumnSubject:
		case eMboxColumnThread:
			// Check that fake subject exists
			if (aMsg->IsFake() && !aMsg->GetEnvelope()->HasFakeSubject())
				const_cast<CMessage*>(aMsg)->MakeFakeSubject();

			theTxt = theEnv->GetSubject();
			
			// Change depth if doing thread sort
			if (GetMbox()->GetSortBy() == cSortMessageThread)
				depth = aMsg->GetThreadDepth();
			break;

		case eMboxColumnDateSent:
			theTxt = theEnv->GetTextDate(true).c_str();
			break;

		case eMboxColumnDateReceived:
			theTxt = aMsg->GetTextInternalDate(true).c_str();
			break;

		case eMboxColumnSize:
			{
				unsigned long msg_size = aMsg->GetSize();
				theTxt = ::GetNumericFormat(msg_size);
				right_just = true;
			}
			break;

		case eMboxColumnFlags:
			do_plot = true;
			plot_flag = GetPlotFlag(aMsg);
			break;

		case eMboxColumnNumber:
			theTxt = (long) aMsg->GetMessageNumber();
			right_just = true;
			break;

		case eMboxColumnAttachments:
			{
				do_plot = true;
				
				bool is_styled = aMsg->GetBody()->HasStyledText();
				bool is_attach = aMsg->GetBody()->HasNonText();
				bool is_calendar = aMsg->GetBody()->HasCalendar();
				bool is_signed = aMsg->GetBody()->IsVerifiable();
				bool is_encrypted = aMsg->GetBody()->IsDecryptable();

				// Encrypted data always shown with single flag
				if (is_encrypted)
					plot_flag = ICNx_Encrypted_Flag;

				// Check for signed data of various types
				else if (is_signed)
				{
					if (is_styled && is_attach)
						plot_flag = ICNx_Signed_Alternative_Attachment_Flag;
					else if (is_styled && is_calendar)
						plot_flag = ICNx_Signed_Alternative_Calendar_Flag;
					else if (is_styled)
						plot_flag = ICNx_Signed_Alternative_Flag;
					else if (is_calendar)
						plot_flag = ICNx_Signed_Calendar_Flag;
					else if (is_attach)
						plot_flag = ICNx_Signed_Attachment_Flag;
					else
						plot_flag = ICNx_Signed_Flag;
				}

				// Check for unsigned data of various types
				else
				{
					if (is_styled && is_attach)
						plot_flag = ICNx_Alternative_Attachment_Flag;
					else if (is_styled && is_calendar)
						plot_flag = ICNx_Alternative_Calendar_Flag;
					else if (is_styled)
						plot_flag = ICNx_Alternative_Flag;
					else if (is_calendar)
						plot_flag = ICNx_Calendar_Flag;
					else if (is_attach)
						plot_flag = ICNx_Attachment_Flag;
					else
						plot_flag = 0;
				}
			}
			break;

		case eMboxColumnParts:
			theTxt = (long) aMsg->GetBody()->CountParts();
			right_just = true;
			break;

		case eMboxColumnMatch:
			do_plot = true;
			if (aMsg->IsSearch())
				plot_flag = ICNx_Match_Flag;
			break;

		case eMboxColumnDisconnected:
			do_plot = true;
			if (aMsg->IsFullLocal())
				plot_flag = ICNx_Cached_Flag;
			else if (aMsg->IsPartialLocal())
				plot_flag = ICNx_Partial_Flag;
			break;
	}

	// Or in style for multiple addresses
	if (multi)
	{
		text_style |= (CPreferences::sPrefs->mMultiAddress.GetValue() & 0x007F);
		strike_through |= ((CPreferences::sPrefs->mMultiAddress.GetValue() & 0x0080) != 0);
	}

	// If message is fake and text is empty, use questiuon marks
	if (aMsg->IsFake() && (col_info.column_type != eMboxColumnSubject) && (col_info.column_type != eMboxColumnThread))
		theTxt = "???";

	::RGBForeColor(&text_color);
	::TextFace(text_style);

	// Draw the text
	if (!do_plot)
	{
		short width = inLocalRect.right - inLocalRect.left;

		switch(col_info.column_type)
		{
		case eMboxColumnSmart:
			// Only draw icon if not fake
			if (!aMsg->IsFake())
			{
				// Check for smart address
				Rect iconRect = inLocalRect;
				iconRect.left += 2;
				iconRect.right = iconRect.left + 16;
				iconRect.bottom = inLocalRect.bottom - mIconDescent;
				iconRect.top = iconRect.bottom - 16;
				::Ploticns(&iconRect, atNone, ttNone, smart_flag);
			}

			// Advance over icon (even if not present)
			// NB These icons have small width - so only need 10 extra pixels instead of 16
			::Move(10, 0);
			width -= 10;
			break;
		case eMboxColumnSubject:
		case eMboxColumnThread:
		{
			// Check for thread
			if (depth)
			{
				{
					StColorPenState temp_pen;
					temp_pen.SetGrayPattern();
					RGBColor rgb = UGAColorRamp::GetColor(colorRamp_Gray9);
					::RGBForeColor(&rgb);

					// Prevent drawing outside of cell area
					unsigned long max_depth_offset = width - 48;

					// Draw right-angle line
					unsigned long depth_offset = (depth - 1) * 16;
					
					// Clip to cell width
					if (depth_offset < max_depth_offset)
					{
						::MoveTo(inLocalRect.left + depth_offset + 8, inLocalRect.top);
						::LineTo(inLocalRect.left + depth_offset + 8, (inLocalRect.top + inLocalRect.bottom)/2);
						::LineTo(inLocalRect.left + depth_offset + 16, (inLocalRect.top + inLocalRect.bottom)/2);
					}

					// Check for parent/sibling lines
					const CMessage* parent = aMsg;
					while(parent)
					{
						// Draw vert line if the parent has a sibling after it (clip to cell width)
						if (parent->GetThreadNext() && (depth_offset < max_depth_offset))
						{
							::MoveTo(inLocalRect.left + depth_offset + 8, inLocalRect.top);
							::LineTo(inLocalRect.left + depth_offset + 8, inLocalRect.bottom);
						}
						
						// No more vert lines if depth offset at first 'column'
						if (!depth_offset)
							break;
							
						// Bump back to the next parent 'column'
						depth_offset -= 16;
						parent = parent->GetThreadParent();
					}						
				}

				// Indent
				unsigned long text_offset = std::min(width - 32UL, 16 * depth);
				::Move(text_offset, 0);
				width -= text_offset;
			}
			break;
		}
		default:;
		}

		// Draw text
		::DrawClippedStringUTF8(theTxt, width, right_just ? eDrawString_Right : eDrawString_Left);

		// Don't strike out fakes
		if (strike_through && !aMsg->IsFake())
		{
			::MoveTo(inLocalRect.left, (inLocalRect.top + inLocalRect.bottom)/2);
			::LineTo(inLocalRect.right, (inLocalRect.top + inLocalRect.bottom)/2);
		}
	}

	// Draw icon
	else
	{
		Rect plotFrame = inLocalRect;
		plotFrame.left += 1;
		plotFrame.right = plotFrame.left + 16;
		plotFrame.bottom = inLocalRect.bottom - mIconDescent;
		plotFrame.top = plotFrame.bottom - 16;

		// Don't draw if its a fake
		if (plot_flag && !aMsg->IsFake())
			::Ploticns(&plotFrame, atNone, ttNone, plot_flag);

		// Don't strike out fakes or flags
		if (strike_through && !aMsg->IsFake() && (col_info.column_type != eMboxColumnFlags))
		{
			::MoveTo(inLocalRect.left, (inLocalRect.top + inLocalRect.bottom)/2);
			::LineTo(inLocalRect.right, (inLocalRect.top + inLocalRect.bottom)/2);
		}
	}
}

void CMailboxTable::GetDrawStyle(const CMessage* theMsg, RGBColor& text_color, Style& text_style, bool& strikeThrough) const
{
	bool deleted = false;
	if (theMsg->IsDeleted())
	{
		text_color = CPreferences::sPrefs->deleted.GetValue().color;
		text_style = CPreferences::sPrefs->deleted.GetValue().style & 0x007F;
		strikeThrough = ((CPreferences::sPrefs->deleted.GetValue().style & 0x0080) != 0);
		deleted = true;
	}
	else if (theMsg->IsFlagged())
	{
		text_color = CPreferences::sPrefs->important.GetValue().color;
		text_style = CPreferences::sPrefs->important.GetValue().style & 0x007F;
		strikeThrough = ((CPreferences::sPrefs->important.GetValue().style & 0x0080) != 0);
	}
	else if (theMsg->IsAnswered())
	{
		text_color = CPreferences::sPrefs->answered.GetValue().color;
		text_style = CPreferences::sPrefs->answered.GetValue().style & 0x007F;
		strikeThrough = ((CPreferences::sPrefs->answered.GetValue().style & 0x0080) != 0);
	}
	else if (theMsg->IsUnseen())
	{
		text_color = CPreferences::sPrefs->unseen.GetValue().color;
		text_style = CPreferences::sPrefs->unseen.GetValue().style & 0x007F;
		strikeThrough = ((CPreferences::sPrefs->unseen.GetValue().style & 0x0080) != 0);
	}
	else
	{
		// Set default seen style
		text_color = CPreferences::sPrefs->seen.GetValue().color;
		text_style = CPreferences::sPrefs->seen.GetValue().style & 0x007F;
		strikeThrough = ((CPreferences::sPrefs->seen.GetValue().style & 0x0080) != 0);
	}
	
	// Look for labels only if not deleted
	if (!deleted)
	{
		bool got_label = false;
		for(unsigned long i = 0; i < NMessage::eMaxLabels; i++)
		{
			if (theMsg->HasLabel(i))
			{
				// Only do coloring once
				if (!got_label && CPreferences::sPrefs->mLabels.GetValue()[i]->usecolor)
					text_color = CPreferences::sPrefs->mLabels.GetValue()[i]->color;
				text_style |= CPreferences::sPrefs->mLabels.GetValue()[i]->style & 0x007F;
				strikeThrough |= ((CPreferences::sPrefs->mLabels.GetValue()[i]->style & 0x0080) != 0);
				
				// The first label set always wins for colors
				got_label = true;
			}
		}
	}
}

ResIDT CMailboxTable::GetPlotFlag(const CMessage* aMsg) const
{
	if (aMsg->IsDeleted())
		return ICNx_Deleted_Flag;
	else if (aMsg->IsDraft())
		return ICNx_Draft_Flag;
	else if (aMsg->IsAnswered())
		return ICNx_Answered_Flag;
	else if (aMsg->IsUnseen() && aMsg->IsRecent())
		return ICNx_Recent_Flag;
	else if (aMsg->IsUnseen())
		return ICNx_Unseen_Flag;
	else
		return ICNx_Read_Flag;
}

bool CMailboxTable::UsesBackground(const CMessage* aMsg) const
{
	// Must have mailbox
	if (!GetMbox())
		return false;

	// If not matching and background flag for matched/unmatched is on
	if (GetMbox()->GetViewMode() == NMbox::eViewMode_AllMatched)
	{
		if (aMsg->IsSearch() && CPreferences::sPrefs->mMatch.GetValue().style)
			return true;
		else if (!aMsg->IsSearch() && CPreferences::sPrefs->mNonMatch.GetValue().style)
			return true;
		
		// Fall through to look for labels
	}
	
	// Look for labels if not deleted
	if (!aMsg->IsDeleted())
	{
		for(unsigned long i = 0; i < NMessage::eMaxLabels; i++)
		{
			if (aMsg->HasLabel(i))
			{
				// The first label set always wins for colors
				return CPreferences::sPrefs->mLabels.GetValue()[i]->usebkgcolor;
			}
		}
	}

	return false;
}

RGBColor CMailboxTable::GetBackground(const CMessage* aMsg) const
{
	// Use background for matched/unmatched in non-Match mode only - override label colour
	if (aMsg->IsSearch() && (GetMbox()->GetViewMode() == NMbox::eViewMode_AllMatched))
		return CPreferences::sPrefs->mMatch.GetValue().color;
	else
	{
		// Look for labels if not deleted
		if (!aMsg->IsDeleted())
		{
			for(unsigned long i = 0; i < NMessage::eMaxLabels; i++)
			{
				if (aMsg->HasLabel(i))
				{
					// The first label set always wins for colors
					if (CPreferences::sPrefs->mLabels.GetValue()[i]->usebkgcolor)
						return CPreferences::sPrefs->mLabels.GetValue()[i]->bkgcolor;
				}
			}
		}
		
		// Usse default n on matched colour	
		return CPreferences::sPrefs->mNonMatch.GetValue().color;
	}
}

// Draw or undraw active hiliting for a Cell
void CMailboxTable::HiliteCellActively(const STableCell &inCell, Boolean inHilite)
{
	// Only if open
	if (!mTableView->IsOpen())
		return;

	// Only if not changing
	if (mListChanging)
		return;

	if (!GetMbox())
		return;

	Rect cellFrame;
	if (GetLocalCellRect(inCell, cellFrame) && FocusExposed())
	{
		// Deal with background
		ApplyForeAndBackColors();
		::EraseRect(&cellFrame);

		// Save text state in stack object
		StTextState		textState;
		StColorState	saveColors;
		StColorPenState::Normalize();

		// Set to required text
		UTextTraits::SetPortTextTraits(&mTextTraits);

		StValueChanger<bool> _preserve(mDrawSelection, inHilite);
		DrawRow(inCell.row, inCell.col, inCell.col);
	}
}

// Draw or undraw inactive hiliting for a Cell
void CMailboxTable::HiliteCellInactively(const STableCell &inCell, Boolean inHilite)
{
	// Only if not changing
	if (mListChanging)
		return;

	Rect cellFrame;
	if (GetLocalCellRect(inCell, cellFrame) && FocusExposed())
	{
		// Deal with background
		ApplyForeAndBackColors();
		::EraseRect(&cellFrame);

		// Save text state in stack object
		StTextState		textState;
		StColorState	saveColors;
		StColorPenState::Normalize();

		// Set to required text
		UTextTraits::SetPortTextTraits(&mTextTraits);

		StValueChanger<bool> _preserve(mDrawSelection, inHilite);
		DrawRow(inCell.row, inCell.col, inCell.col);
	}
}

#pragma mark ____________________________Commands

// Print a mail message
void CMailboxTable::DoPrintMailMessage(void)
{
	LPrintSpec printSpec;

	try
	{
		//  Hide status window as Print Manager gets name of top window and does not know about floats
		{
			StPrintSession	session(printSpec);
			StStatusWindowHide hide;
			if (UPrinting::AskPrintJob(printSpec))
			{
				// Display each selected message
				DoToSelection1((DoToSelection1PP) &CMailboxTable::PrintMailMessage, &printSpec);
			}
		}

		// Print job status window might mess up window order
		UDesktop::NormalizeWindowOrder();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);


		// Print job status window might mess up window order
		// Always do this just in case...
		UDesktop::NormalizeWindowOrder();

		// throw up
		CLOG_LOGRETHROW;
		throw;
	}
}

// Print a specified mail message
bool CMailboxTable::PrintMailMessage(TableIndexT row, LPrintSpec* printSpec)
{
	if (!GetMbox())
		return false;

	// Get the relevant message and envelope
	CMessage* theMsg = GetMbox()->GetMessage(row, true);
	if (!theMsg || !theMsg->IsFullyCached() || theMsg->IsFake())
		return false;

	// Don't print if its deleted
	if (theMsg->IsDeleted() && !CPreferences::sPrefs->mOpenDeleted.GetValue())
	{
		::SysBeep(1);
		return false;
	}

	// Does window already exist?
	CMessageWindow*	theWindow = CMessageWindow::FindWindow(theMsg);
	if (theWindow)
	{
		// Found existing window so print
		theWindow->SetPrintSpec(printSpec);
		try
		{
			theWindow->DoPrint();
			theWindow->SetPrintSpec(NULL);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			theWindow->SetPrintSpec(NULL);
			CLOG_LOGRETHROW;
			throw;
		}
		return false;
	}

	// Check message size first
	if (!CMailControl::CheckSizeWarning(theMsg))
		return false;

	CMessageWindow* newWindow = NULL;
	try
	{
		// Create the message window
		newWindow = (CMessageWindow*) CMessageWindow::CreateWindow(paneid_MessageWindow, CMulberryApp::sApp);
		newWindow->SetMessage(theMsg);
		newWindow->SetPrintSpec(printSpec);
		newWindow->DoPrint();
		newWindow->SetPrintSpec(NULL);
		FRAMEWORK_DELETE_WINDOW(newWindow)
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Only delete if it still exists
		if (CMessageWindow::FindWindow(theMsg))
		{
			newWindow->SetPrintSpec(NULL);
			FRAMEWORK_DELETE_WINDOW(newWindow)
		}

		// Should throw out of here in case abort and mbox now destroyed
		CLOG_LOGRETHROW;
		throw;
	}

	return false;
}

#pragma mark ____________________________Display updating

void CMailboxTable::DoSelectionChanged(void)
{
	mIsSelectionValid = IsSelectionValid() && !TestSelectionAnd((TestSelectionPP) &CMailboxTable::TestSelectionFake);
	mTestSelectionAndDeleted = TestSelectionIgnore1And(&CMailboxTable::TestSelectionFlag, NMessage::eDeleted);
	
	// Do inherited to ensure broadcast
	CTableDrag::DoSelectionChanged();
}

// Reset the table from the mbox
void CMailboxTable::ResetTable(bool scroll_new)
{
	{
		// Preserve selection
		StMailboxTableSelection preserve_selection(this, false);

		{
			// Prevent screen updates because of multi-thread access
			StValueChanger<bool> _change(mListChanging, true);

			TableIndexT	old_rows;
			TableIndexT	old_cols;
			long num_msgs = (GetMbox() ? GetMbox()->GetNumberMessages() : 0);

			GetTableSize(old_rows, old_cols);

			if (old_rows > num_msgs)
				RemoveRows(old_rows - num_msgs, 1, false);
			else if (old_rows < num_msgs)
				InsertRows(num_msgs - old_rows, 1, NULL, 0, false);

			// Previous selection no longer valid
			UnselectAllCells();
			LCommander::SetUpdateCommandStatus(true);
		}
	}
	
	// Scroll to display new messages
	if (scroll_new)
		ScrollForNewMessages();
	
	// Make sure first cell is at the top
	Refresh();
	
	// Set flag for reset
	mResetTable = true;
}

// Clear the table of all contents
void CMailboxTable::ClearTable(void)
{
	// Remove all rows
	RemoveRows(mRows, 1, true);
	
	// Update captions to empty
	Refresh();
}

// Keep titles in sync
void CMailboxTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, Boolean inRefresh)
{
	// Do scroll of main table first to avoid double-refresh
	CTableDrag::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);

	// Find titles in owner chain
	mTableView->GetBaseTitles()->ScrollImageBy(inLeftDelta, 0, inRefresh);
}

// Get horiz scrollbar pos
void CMailboxTable::GetScrollPos(long& h, long& v) const
{
	SPoint32 floc;
	GetFrameLocation(floc);

	SPoint32 iloc;
	GetImageLocation(iloc);

	h = floc.h - iloc.h;
	v = floc.v - iloc.v;
}

// Get horiz scrollbar pos
void CMailboxTable::SetScrollPos(long h, long v)
{
	ScrollPinnedImageTo(h, v, true);
}

#pragma mark ____________________________Drag & Drop

// Adjust cursor over drop area
bool CMailboxTable::IsCopyCursor(DragReference inDragRef)
{
	// Get flavor for this item
	ItemReference theItemRef;
	FlavorType theFlavor;
	if (::GetDragItemReferenceNumber(inDragRef, 1, &theItemRef) != noErr)
		return false;
	if (::GetFlavorType(inDragRef, theItemRef, 1, &theFlavor) != noErr)
		return false;

	switch(theFlavor)
	{
	case cDragMsgType:
	{
		// Toggle based on modifiers
		short mouseModifiers;
		::GetDragModifiers(inDragRef, &mouseModifiers, NULL, NULL);
		bool option_key = mouseModifiers & optionKey;

		return !(CPreferences::sPrefs->deleteAfterCopy.GetValue() ^ option_key);
	}
	case cDragMboxType:
	case cDragMboxRefType:
		// Mailbox drop is treated as a move
		return true;

	default:
		// Anything else is an error!
		return false;
	}
}

// Add mail message to drag
void CMailboxTable::AddCellToDrag(CDragIt* theDragTask,
										const STableCell& aCell,
										Rect& dragRect)
{
	if (!GetMbox())
		return;

	// Get the relevant message and envelope
	CMessage* theMsg = GetMbox()->GetMessage(aCell.row, true);
	if (!theMsg || theMsg->IsFake())
		return;

	// Add this message to drag
	theDragTask->AddFlavorItem(dragRect, (ItemReference) theMsg, cDragMsgType,
								&theMsg, sizeof(CMessage*), flavorSenderOnly, true);

	// Promise text to anyone who wants it
	theDragTask->AddFlavorItem(dragRect, (ItemReference) theMsg, kScrapFlavorTypeText, 0L, 0L, 0, false);
}

// Handle multiple messages
void CMailboxTable::DoDragReceive(DragReference inDragRef)
{
	if (!GetMbox())
		return;

	// Do default action
	CTableDrag::DoDragReceive(inDragRef);
}

// Drop data into table
void CMailboxTable::DropData(FlavorType theFlavor, char* drag_data, Size data_size)
{
	if (!GetMbox())
		return;

	switch(theFlavor)
	{
	case cDragMsgType:
	{
		CMessage* theMsg = *((CMessage**) drag_data);

		CDragMessagesTask* task = dynamic_cast<CDragMessagesTask*>(CDragTask::GetCurrentDragTask());
		if (!task)
		{
			CMbox* from = theMsg->GetMbox();

			task = new CDragMessagesTask(GetMbox(), from);
		}
		task->AddMessage(theMsg);
		break;
	}
	case cDragMboxType:
	{
		CMbox* mbox = *(CMbox**) drag_data;

		CDragMailboxToView* task = dynamic_cast<CDragMailboxToView*>(CDragTask::GetCurrentDragTask());
		if (!task)
			task = new CDragMailboxToView(mTableView, mbox);
		break;
	}
	case cDragMboxRefType:
	{
		CMboxRef* mboxref = *(CMboxRef**) drag_data;
		CMbox* mbox = mboxref->ResolveMbox();

		CDragMailboxToView* task = dynamic_cast<CDragMailboxToView*>(CDragTask::GetCurrentDragTask());
		if (!task)
			task = new CDragMailboxToView(mTableView, mbox);
		break;
	}
	default:;
	}
}

// Other flavor requested by receiver
void CMailboxTable::DoDragSendData(
	FlavorType		inFlavor,
	ItemReference	inItemRef,
	DragReference	inDragRef)
{
	if (inFlavor == kScrapFlavorTypeText)
	{
		LHandleStream h_stream;

		// Check message size first
#if 0	// Will crash if GA alert shown during D&D
		if (!CMailControl::CheckSizeWarning(((CMessage*) inItemRef)), true)
			return false;
#endif

		// Read in message first
		CMailControl::BlockBusy(true);
		const char* data = NULL;
		try
		{
			data = reinterpret_cast<CMessage*>(inItemRef)->ReadPart();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			CMailControl::BlockBusy(false);

			// Throw out of here as it is fatal
			CLOG_LOGRETHROW;
			throw;
		}
		CMailControl::BlockBusy(false);

		// Add header to stream if required
		if (CPreferences::sPrefs->saveMessageHeader.GetValue())
			h_stream.WriteBlock(((CMessage*) inItemRef)->GetHeader(),
								::strlen(((CMessage*) inItemRef)->GetHeader()));

		// Add message text to stream (include terminator)
		if (data)
			h_stream.WriteBlock(data, ::strlen(data) + 1);

		{
			// Get and lock handle
			Handle txt_hdl = h_stream.GetDataHandle();
			StHandleLocker lock(txt_hdl);

			// Give drag this data
			ThrowIfOSErr_(::SetDragItemFlavorData(inDragRef, inItemRef, kScrapFlavorTypeText,
											*txt_hdl, ::strlen(*txt_hdl), 0L));
		}
	}
	else
		Throw_(badDragFlavorErr);
}
