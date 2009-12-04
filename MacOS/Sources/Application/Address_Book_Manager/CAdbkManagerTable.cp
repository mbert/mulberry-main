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


// Source for CAdbkManagerTable class

#include "CAdbkManagerTable.h"

#include "CAdbkManagerTitleTable.h"
#include "CAdbkManagerView.h"
#include "CAdbkManagerWindow.h"
#include "CAdbkProtocol.h"
#include "CAddressBook.h"
#include "CAddressBookManager.h"
#include "CAddressBookWindow.h"
#include "CAdminLock.h"
#include "CBetterScrollerX.h"
#include "CCommands.h"
#include "CMessage.h"
#include "CResources.h"
#include "CPreferences.h"
#include "CTableMultiRowSelector.h"
#include "CTableRowGeometry.h"
#include "CTableView.h"
#include "CWaitCursor.h"

#include <LDropFlag.h>
#include <LNodeArrayTree.h>
#include <LTableArrayStorage.h>

#include <stdio.h>
#include <string.h>

const short cTextIndent = 22;

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAdbkManagerTable::CAdbkManagerTable()
{
	InitAdbkManagerTable();
}

// Default constructor - just do parents' call
CAdbkManagerTable::CAdbkManagerTable(LStream *inStream)
	: CHierarchyTableDrag(inStream)
{
	InitAdbkManagerTable();
}

// Default destructor
CAdbkManagerTable::~CAdbkManagerTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CAdbkManagerTable::InitAdbkManagerTable()
{
	mTableGeometry = new CTableRowGeometry(this, mFrameSize.width, 18);
	mTableSelector = new CTableMultiRowSelector(this);
	//mTableStorage = new LTableArrayStorage(this, sizeof(CAdbkList::node_type*));
	mManager = NULL;
	mListChanging = false;
	mHierarchyCol = 0;
}

// Get details of sub-panes
void CAdbkManagerTable::FinishCreateSelf()
{
	// Do inherited
	CHierarchyTableDrag::FinishCreateSelf();

	// Find table view in super view chain
	LView* super = GetSuperView();
	while(super && !dynamic_cast<CAdbkManagerView*>(super))
		super = super->GetSuperView();
	mTableView = dynamic_cast<CAdbkManagerView*>(super);

	// Set Drag & Drop pane to scroller
	mPane = GetSuperView();

	// Set Drag & Drop info
	SetTable(this, true);

	AddDropFlavor(cDragAddrType);
	AddDropFlavor(cDragAddrGrpType);
	AddDropFlavor(cDragMsgType);
	AddDropFlavor(kScrapFlavorTypeText);

	// Set appropriate Drag & Drop inset
	Rect ddInset = {1, 1, 1, 1};
	if (((CBetterScrollerX*) mPane)->HasVerticalScrollBar())
		ddInset.right += 15;
	if (((CBetterScrollerX*) mPane)->HasHorizontalScrollBar())
		ddInset.bottom += 15;
	SetHiliteInset(ddInset);

	// Set read only status of Drag and Drop
	SetDDReadOnly(false);

	// Make it fit to the superview
	AdaptToNewSurroundings();
}

// Handle key presses
Boolean CAdbkManagerTable::HandleKeyPress(const EventRecord &inKeyEvent)
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
		DoPreview();
		return true;
	}

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if ((fullview.GetKey() == key_press) &&
		(fullview.GetKeyModifiers() == mods))
	{
		DoFullView();
		return true;
	}

	return CHierarchyTableDrag::HandleKeyPress(inKeyEvent);
}

//	Respond to commands
Boolean CAdbkManagerTable::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool cmdHandled = true;
	
	switch (inCommand)
	{
		case cmd_Properties:
		case cmd_ToolbarDetailsBtn:
			OnAddressBookProperties();
			break;
			
		case cmd_NewAddressBook:
		case cmd_ToolbarAdbkMgrNewBtn:
			OnNewAddressBook();
			break;
			
		case cmd_OpenAddressBook:
		case cmd_ToolbarAdbkMgrOpenBtn:
			OnOpenAddressBook();
			break;
			
		case cmd_RenameAddressBook:
			OnRenameAddressBook();
			break;
			
		case cmd_DeleteAddressBook:
		case cmd_ToolbarAdbkMgrDeleteBtn:
			OnDeleteAddressBook();
			break;
			
		case cmd_ToolbarServerLoginBtn:
			OnLogin();
			break;
			
		case cmd_NewHierarchy:
			OnNewHierarchy();
			break;
			
		case cmd_EditHierarchy:
			OnRenameHierarchy();
			break;
			
		case cmd_DeleteHierarchy:
			OnDeleteHierarchy();
			break;
		case cmd_RefreshAddressBook:
			OnRefreshAddressBook();
			break;
			
		case cmd_SynchroniseAddressBook:
			OnSynchroniseAddressBook();
			break;
			
		case cmd_ClearDisconnectAddressBook:
			OnClearDisconnectAddressBook();
			break;
			
		case cmd_ToolbarAdbkMgrSearchBtn:
			OnSearchAddressBook();
			break;
			
		default:
			cmdHandled = CHierarchyTableDrag::ObeyCommand(inCommand, ioParam);
			break;
	}
	
	return cmdHandled;
}

//	Pass back status of a (menu) command
void CAdbkManagerTable::FindCommandStatus(
										  CommandT	inCommand,
										  Boolean		&outEnabled,
										  Boolean		&outUsesMark,
										  UInt16		&outMark,
										  Str255		outName)
{
	outUsesMark = false;
	
	switch (inCommand)
	{
			// These ones must have a selection
		case cmd_Properties:
		case cmd_ToolbarDetailsBtn:
		case cmd_RenameAddressBook:
		case cmd_DeleteAddressBook:
		case cmd_ToolbarAdbkMgrDeleteBtn:
			outEnabled = TestSelectionAnd((TestSelectionPP) &CAdbkManagerTable::TestSelectionAdbk);
			break;
			
		case cmd_SynchroniseAddressBook:
			outEnabled = TestSelectionAnd((TestSelectionPP) &CAdbkManagerTable::TestSelectionAdbkDisconnected);
			break;
			
		case cmd_ClearDisconnectAddressBook:
			outEnabled = TestSelectionAnd((TestSelectionPP) &CAdbkManagerTable::TestSelectionAdbkClearDisconnected);
			break;
			
			// These can only have logged on protocol selection
		case cmd_NewAddressBook:
		case cmd_ToolbarAdbkMgrNewBtn:
			outEnabled = true;
			break;
			
		case cmd_OpenAddressBook:
		case cmd_ToolbarAdbkMgrOpenBtn:
			// Remap command id for tests
			if (inCommand == cmd_ToolbarAdbkMgrNewBtn)
				inCommand = cmd_NewAddressBook;
			
			if (IsSingleSelection())
			{
				STableCell aCell(0,0);
				GetNextSelectedCell(aCell);
				CAdbkProtocol* proto = GetCellAdbkProtocol(aCell.row);
				outEnabled = ((proto == NULL) || ((inCommand == cmd_NewAddressBook) ? proto->IsLoggedOn() : false));
			}
			else if (!IsSelectionValid())
			{
				outEnabled = (inCommand == cmd_NewAddressBook) ? false :
				!CAdminLock::sAdminLock.mNoLocalAdbks;
			}
			else
				outEnabled = TestSelectionAnd((TestSelectionPP) &CAdbkManagerTable::TestSelectionAdbk);
			break;
			
			// These can only have logged on protocol selection
		case cmd_ToolbarServerLoginBtn:
			// Logon button must have single server selected
			if (IsSingleSelection() && TestSelectionAnd((TestSelectionPP) &CAdbkManagerTable::TestSelectionServer))
			{
				TableIndexT row = GetFirstSelectedRow();
				CAddressBook* adbk = GetCellNode(row);
				
				// Policy:
				//
				// 1. 	Local protocols are always logged in - login button is disabled
				// 2.	Protocols that cannot disconnect
				// 2.1	maintain their own logged in state when global connect state is on,
				// 2.2	else they are always logged out when global state is disconnected and the login button is disabled
				// 3.	Protocols that can disconnect
				// 3.1	when global connect state is on, they maintain their own logged in state based on disconnected state
				// 3.3	else they are always logged in and the login button is disabled
				
				// 1. (as above)
				if (adbk->GetProtocol()->IsOffline() && !adbk->GetProtocol()->IsDisconnected())
				{
					// Local items are always logged in (connected) so disable the button
					outEnabled = false;
					outUsesMark = true;
					::GetIndString(outName, STRx_Standards, str_Logoff);
				}
				
				// 2. (as above)
				else if (!adbk->GetProtocol()->CanDisconnect())
				{
					// 2.1 (as above)
					if (CConnectionManager::sConnectionManager.IsConnected())
					{
						outEnabled = true;
						outUsesMark = true;
						outMark = adbk->GetProtocol()->IsLoggedOn() ? (UInt16)checkMark : (UInt16)noMark;
						::GetIndString(outName, STRx_Standards, !outMark ? str_Logon : str_Logoff);
					}
					// 2.2 (as above)
					else
					{
						outEnabled = false;
						outUsesMark = true;
						::GetIndString(outName, STRx_Standards, str_Logoff);
					}
				}
				
				// 3. (as above)
				else
				{
					// 3.1 (as above)
					if (CConnectionManager::sConnectionManager.IsConnected())
					{
						outEnabled = true;
						outUsesMark = true;
						outMark = !adbk->GetProtocol()->IsDisconnected() ? (UInt16)checkMark : (UInt16)noMark;
						::GetIndString(outName, STRx_Standards, !outMark ? str_Logon : str_Logoff);
					}
					// 3.2 (as above)
					else
					{
						outEnabled = false;
						outUsesMark = true;
						::GetIndString(outName, STRx_Standards, str_Logoff);
					}
				}
			}
			else
			{
				outEnabled = false;
				outUsesMark = false;
				::GetIndString(outName, STRx_Standards, str_Logon);
			}
			break;
			
		case cmd_Hierarchy:
		case cmd_NewHierarchy:
			// Always able to do this, even if logged off or no selection
			outEnabled = true;
			break;
			
		case cmd_EditHierarchy:
		case cmd_DeleteHierarchy:
			outEnabled = TestSelectionAnd((TestSelectionPP) &CAdbkManagerTable::TestSelectionHierarchy);
			break;
			// These can only have logged off protocol selection
		case cmd_RefreshAddressBook:
			// Only if single selection;
			outEnabled = IsSingleSelection();
			break;
			
			// Always available
		case cmd_ToolbarAdbkMgrSearchBtn:
			outEnabled = true;
			break;
			
		default:
			CHierarchyTableDrag::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
			break;
	}
}

// Click in the cell
void CAdbkManagerTable::ClickCell(const STableCell& inCell,
								const SMouseDownEvent& inMouseDown)
{
	// Determine which heading it is
	SColumnInfo col_info = mTableView->GetColumnInfo()[inCell.col - 1];

	switch(col_info.column_type)
	{
	case eAdbkColumnOpen:
	case eAdbkColumnResolve:
	case eAdbkColumnSearch:
		TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row + TABLE_ROW_ADJUST);

		CAddressBook* adbk = GetCellNode(inCell.row);

		// Do status flag
		if (!inMouseDown.delaySelect && adbk->IsAdbk())
		{
			bool set;
			switch(col_info.column_type)
			{
			case eAdbkColumnOpen:
				set = !adbk->IsOpenOnStart();
				adbk->SetFlags(CAddressBook::eOpenOnStart, set);
				CAddressBookManager::sAddressBookManager->SyncAddressBook(adbk, set);

				// Change prefs list
				CPreferences::sPrefs->ChangeAddressBookOpenOnStart(adbk, set);
				break;

			case eAdbkColumnResolve:
				set = !adbk->IsLookup();
				adbk->SetFlags(CAddressBook::eLookup, set);
				CAddressBookManager::sAddressBookManager->SyncAddressBook(adbk, set);

				// Change prefs list
				CPreferences::sPrefs->ChangeAddressBookLookup(adbk, set);
				break;

			case eAdbkColumnSearch:
				set = !adbk->IsSearch();
				adbk->SetFlags(CAddressBook::eSearch, set);
				CAddressBookManager::sAddressBookManager->SyncAddressBook(adbk, set);

				// Change prefs list
				CPreferences::sPrefs->ChangeAddressBookSearch(adbk, set);
				break;
			}

			RefreshCell(inCell);
		}
		break;
	default:
		// Look at user action and determine what to do
		if ((GetClickCount() > 1) && !inMouseDown.delaySelect)
			DoDoubleClick(inCell.row, CKeyModifiers(inMouseDown.macEvent.modifiers));
		else
			DoSingleClick(inCell.row, CKeyModifiers(inMouseDown.macEvent.modifiers));
		break;
	}
}

// Click
void CAdbkManagerTable::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	SwitchTarget(this);

	// Must set background color to grey if tracking drop flag
	StColorState	saveColors;

	STableCell	hitCell;
	SPoint32	imagePt;

	LocalToImagePoint(inMouseDown.whereLocal, imagePt);

	if (GetCellHitBy(imagePt, hitCell))
	{
		CAddressBook* adbk = GetCellNode(hitCell.row);
		if ((hitCell.col == mHierarchyCol) && UsesBackgroundColor(hitCell))
		{
			FocusDraw();
			ApplyForeAndBackColors();
			::RGBBackColor(&GetBackgroundColor(hitCell));
		}
	}

	CHierarchyTableDrag::ClickSelf(inMouseDown);
}

// Draw the titles
void CAdbkManagerTable::DrawCell(const STableCell &inCell,
								const Rect &inLocalRect)
{
	// Not if changing
	if (mListChanging)
		return;

	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row + TABLE_ROW_ADJUST);

	CAddressBook* adbk = GetCellNode(inCell.row);

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

	// Erase to ensure drag hightlight is overwritten
	// Erase to ensure drag hightlight is overwritten
	FocusDraw();
	if (UsesBackgroundColor(inCell))
	{
		Rect greyer = inLocalRect;
		greyer.bottom = greyer.top + 1;
		::EraseRect(&greyer);
		::RGBBackColor(&GetBackgroundColor(inCell));
		greyer = inLocalRect;
		greyer.top++;
		::EraseRect(&greyer);
	}
	else
		::EraseRect(&inLocalRect);

	// Determine which heading it is
	SColumnInfo col_info = mTableView->GetColumnInfo()[inCell.col - 1];

	switch(col_info.column_type)
	{

	case eAdbkColumnName:
	{
		// Always cache column number
		mHierarchyCol = inCell.col;
		SetOneColumnSelect(mHierarchyCol);

		DrawDropFlag(inCell, woRow);

		// Draw selection
		bool selected_state = DrawCellSelection(inCell);

		UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);
		Rect	iconRect;
		iconRect.left = inLocalRect.left + mFirstIndent + nestingLevel * mLevelIndent;
		iconRect.right = iconRect.left + 16;
		iconRect.bottom = inLocalRect.bottom - mIconDescent;
		iconRect.top = iconRect.bottom - 16;
		::Ploticns(&iconRect, atNone, selected_state ? ttSelected : ttNone, GetPlotIcon(adbk, GetCellAdbkProtocol(inCell.row)));

		// Get name of item
		theTxt = adbk->GetDisplayShortName();
		
		// Add protocol state descriptor
		if (adbk->IsProtocol())
		{
			if (adbk->GetProtocol()->IsDisconnected() && !CConnectionManager::sConnectionManager.IsConnected())
			{
				theTxt.AppendResource("UI::Server::TitleDisconnected");
			}
			else if (adbk->GetProtocol()->IsDisconnected() || !adbk->GetProtocol()->IsOffline() && !adbk->GetProtocol()->IsLoggedOn())
			{
				theTxt.AppendResource("UI::Server::TitleOffline");
			}
		}

		// Draw the string
		bool strike = false;
		SetTextStyle(adbk, GetCellAdbkProtocol(inCell.row), strike);
		::MoveTo(iconRect.right - 2, inLocalRect.bottom - mTextDescent);
		::DrawClippedStringUTF8(theTxt, inLocalRect.right - iconRect.right - 2, eDrawString_Left);
		if (strike)
		{
			::MoveTo(iconRect.right + 2, (inLocalRect.top + inLocalRect.bottom)/2);
			::LineTo(inLocalRect.right, (inLocalRect.top + inLocalRect.bottom)/2);
		}
		break;
	}

	case eAdbkColumnOpen:
	case eAdbkColumnResolve:
	case eAdbkColumnSearch:
	{
		Rect iconRect;
		iconRect.left = inLocalRect.left;
		iconRect.right = iconRect.left + 16;
		iconRect.bottom = inLocalRect.bottom - mIconDescent;
		iconRect.top = iconRect.bottom - 16;

		// Do status flag
		if (adbk->IsAdbk())
		{
			bool ticked = false;
			switch(col_info.column_type)
			{
			case eAdbkColumnOpen:
				ticked = adbk->IsOpenOnStart();
				break;
			case eAdbkColumnResolve:
				ticked = adbk->IsLookup();
				break;
			case eAdbkColumnSearch:
				ticked = adbk->IsSearch();
				break;
			}

			if (ticked)
				::Ploticns(&iconRect, atNone, ttNone, ICNx_DiamondTicked);
			else
				::Ploticns(&iconRect, atNone, ttNone, ICNx_Diamond);
		}
		else
			::EraseRect(&iconRect);
		break;
	}

	default:
		break;
	}
}

// Draw or undraw active hiliting for a Cell
void CAdbkManagerTable::HiliteCellActively(const STableCell &inCell, Boolean inHilite)
{
	// Not if changing
	if (mListChanging)
		return;

	CHierarchyTableDrag::HiliteCellActively(inCell, inHilite);
}

// Draw or undraw inactive hiliting for a Cell
void CAdbkManagerTable::HiliteCellInactively(const STableCell &inCell, Boolean inHilite)
{
	// Not if changing
	if (mListChanging)
		return;

	CHierarchyTableDrag::HiliteCellInactively(inCell, inHilite);
}

void CAdbkManagerTable::CalcCellFlagRect(const STableCell &inCell, Rect &outRect)
{
	if (inCell.col == mHierarchyCol)
	{
		LHierarchyTable::CalcCellFlagRect(inCell, outRect);
		outRect.right = outRect.left + 16;
		outRect.bottom = outRect.top + 12;
		::OffsetRect(&outRect, 0, 2);
	}
	else
		::SetRect(&outRect, 0, 0, 0, 0);
}

// Get appropriate icon id
ResIDT CAdbkManagerTable::GetPlotIcon(const CAddressBook* adbk, CAdbkProtocol* proto)
{
	if (adbk->IsProtocol())
	{
		if (!proto)
			return ICNx_BrowseLocalHierarchy;
		else if (proto->CanDisconnect())
			return proto->IsDisconnected() ? ICNx_BrowseDisconnectedHierarchy : ICNx_BrowseRemoteHierarchy;
		else if ((proto->GetAccountType() == CINETAccount::eLocalAdbk) ||
					(proto->GetAccountType() == CINETAccount::eOSAdbk))
			return ICNx_BrowseLocalHierarchy;
		else
			return ICNx_BrowseRemoteHierarchy;
	}
	else if (adbk->IsDirectory() && !adbk->IsAdbk())
	{
		return ICNx_BrowseDirectory;
	}
	else
	{
		return adbk->IsCached() ? ICNx_AddressBook : ICNx_AddressBookUncached;
	}
}

// Get text style
void CAdbkManagerTable::SetTextStyle(const CAddressBook* adbk, CAdbkProtocol* proto, bool& strike)
{
	strike = false;

	// Select appropriate color and style of text
	if (UEnvironment::HasFeature(env_SupportsColor))
	{
		if (adbk->IsProtocol())
		{
			bool color_set = false;
			RGBColor text_color;
			Style text_style = normal;

			if (!proto || proto->IsLoggedOn())
			{
				text_color = CPreferences::sPrefs->mServerOpenStyle.GetValue().color;
				color_set = true;
				text_style = text_style | CPreferences::sPrefs->mServerOpenStyle.GetValue().style & 0x007F;
				strike = strike || ((CPreferences::sPrefs->mServerOpenStyle.GetValue().style & 0x0080) != 0);
			}
			if (!color_set)
			{
				text_color = CPreferences::sPrefs->mServerClosedStyle.GetValue().color;
				color_set = true;
				text_style = text_style | CPreferences::sPrefs->mServerClosedStyle.GetValue().style & 0x007F;
				strike =  strike || ((CPreferences::sPrefs->mServerClosedStyle.GetValue().style & 0x0080) != 0);
			}

			::RGBForeColor(&text_color);
			::TextFace(text_style);
		}
		else
		{
			bool color_set = false;
			RGBColor text_color;
			Style text_style = normal;

			text_color = CPreferences::sPrefs->mMboxClosedStyle.GetValue().color;
			color_set = true;
			text_style = text_style | CPreferences::sPrefs->mMboxClosedStyle.GetValue().style & 0x007F;
			strike =  strike || ((CPreferences::sPrefs->mMboxClosedStyle.GetValue().style & 0x0080) != 0);

			::RGBForeColor(&text_color);
			::TextFace(text_style);
		}
	}
}

bool CAdbkManagerTable::UsesBackgroundColor(const STableCell &inCell) const
{
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row);
	return mCollapsableTree->GetNestingLevel(woRow) == 0;
}

const RGBColor& CAdbkManagerTable::GetBackgroundColor(const STableCell &inCell) const
{
	return CPreferences::sPrefs->mServerBkgndStyle.GetValue().color;
}

// Keep titles in sync
void CAdbkManagerTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, Boolean inRefresh)
{
	// Find titles in owner chain
	mTableView->GetBaseTitles()->ScrollImageBy(inLeftDelta, 0, inRefresh);

	CHierarchyTableDrag::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
}

// Adjust cursor over drop area
bool CAdbkManagerTable::IsCopyCursor(DragReference inDragRef)
{
	return true;
}

// Test drag insert cursor
bool CAdbkManagerTable::IsDropCell(DragReference inDragRef, STableCell theCell)
{
	CAddressBook* adbk = GetCellNode(theCell.row);
	return IsValidCell(theCell) && adbk->IsAdbk();
}

// Handle multiple messages
void CAdbkManagerTable::DoDragReceive(DragReference inDragRef)
{
	// Init drop params
	mDropFirst = true;
	mDropSort = false;
	mDropAdbk = NULL;
	mDropAdbkWnd = NULL;
	mAddressAdded = false;
	mGroupAdded = false;

	try
	{
		// Do default action
		CHierarchyTableDrag::DoDragReceive(inDragRef);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Do not allow failure
	}

	// Always force window update
	if (mDropAdbkWnd)
		mDropAdbkWnd->GetAddressBookView()->ResetTable();
	mDropAdbk = NULL;
}

// Drop data into cell
void CAdbkManagerTable::DropDataIntoCell(FlavorType theFlavor, char* drag_data,
										Size data_size, const STableCell& theCell)
{
	if (mDropFirst)
	{
		// Determine adbk to use for copy
		mDropAdbk = GetCellNode(theCell.row);

		// Does window already exist?
		mDropAdbkWnd = CAddressBookWindow::FindWindow(mDropAdbk);
		mDropSort = (mDropAdbkWnd != NULL);

		mDropFirst = false;
	}

	switch (theFlavor)
	{

		case cDragMsgType:
			CMessage* theMsg = *((CMessage**) drag_data);
			CEnvelope* theEnv = theMsg->GetEnvelope();
			ThrowIfNil_(theEnv);

			// Add From
			if (theEnv->GetFrom()->size())
			{
				CAddress* theAddr = theEnv->GetFrom()->front();
				if (!CPreferences::sPrefs->TestSmartAddress(*theAddr) &&
					!mDropAdbk->GetAddressList()->IsDuplicate(theAddr))
				{
					// Add to list
					mDropAdbk->AddAddress(new CAdbkAddress(*theAddr), mDropSort);
					mAddressAdded = true;
				}
			}

			// Add Reply-To
			if (theEnv->GetReplyTo()->size())
			{
				CAddress* theAddr = theEnv->GetReplyTo()->front();
				if (!CPreferences::sPrefs->TestSmartAddress(*theAddr) &&
					!mDropAdbk->GetAddressList()->IsDuplicate(theAddr))
				{
					// Add to list
					mDropAdbk->AddAddress(new CAdbkAddress(*theAddr), mDropSort);
					mAddressAdded = true;
				}
			}

			// Add all To: except me
			for(CAddressList::iterator iter = theEnv->GetTo()->begin(); iter != theEnv->GetTo()->end(); iter++)
			{
				CAddress* theAddr = *iter;
				if (!CPreferences::sPrefs->TestSmartAddress(*theAddr) &&
					!mDropAdbk->GetAddressList()->IsDuplicate(theAddr))
				{
					// Add to list
					mDropAdbk->AddAddress(new CAdbkAddress(*theAddr), mDropSort);
					mAddressAdded = true;
				}
			}

			// Add all Ccs except me
			for(CAddressList::iterator iter = theEnv->GetCC()->begin(); iter != theEnv->GetCC()->end(); iter++)
			{
				CAddress* theAddr = *iter;
				if (!CPreferences::sPrefs->TestSmartAddress(*theAddr) &&
					!mDropAdbk->GetAddressList()->IsDuplicate(theAddr))
				{
					// Add to list
					mDropAdbk->AddAddress(new CAdbkAddress(**iter), mDropSort);
					mAddressAdded = true;
				}
			}

			break;

		case cDragAddrType:
			CAddress* theAddr = *((CAddress**) drag_data);
			if (!CPreferences::sPrefs->TestSmartAddress(*theAddr) &&
				!mDropAdbk->GetAddressList()->IsDuplicate(theAddr))
			{
				// Add to list
				mDropAdbk->AddAddress(new CAdbkAddress(*theAddr), mDropSort);
				mAddressAdded = true;
			}
			break;

		case cDragAddrGrpType:
			CGroup* theGrp = *((CGroup**) drag_data);

			// Add group to new group
			mDropAdbk->AddGroup(new CGroup(*theGrp), mDropSort);
			mGroupAdded = true;
			break;

		case kScrapFlavorTypeText:

			// Parse text into list
			CAddressList list(drag_data, data_size);

			// Try to add these to table
			for(CAddressList::iterator iter = list.begin(); iter != list.end(); iter++)
			{
				CAddress* theAddr = *iter;
				if (!CPreferences::sPrefs->TestSmartAddress(*theAddr) &&
					!mDropAdbk->GetAddressList()->IsDuplicate(theAddr))
				{
					// Now add to address book
					mDropAdbk->AddAddress(new CAdbkAddress(*theAddr), mDropSort);
					mAddressAdded = true;
				}
			}
			break;
	}
}
