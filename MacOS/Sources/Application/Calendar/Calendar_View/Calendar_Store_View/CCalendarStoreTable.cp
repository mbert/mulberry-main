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

#include "CCalendarStoreTable.h"

#include "CAdminLock.h"
#include "CBetterScrollerX.h"
#include "CCalendarView.h"
#include "CCalendarStoreTitleTable.h"
#include "CCalendarStoreView.h"
#include "CCommands.h"
#include "CContextMenu.h"
#include "CDragIt.h"
#include "CDragTasks.h"
#include "CGUtils.h"
#include "CLocalCommon.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CResources.h"
#include "CTableMultiRowSelector.h"
#include "CTableRowGeometry.h"
#include "CXStringResources.h"

#include "cdfstream.h"

#include "CCalendarStoreNode.h"
#include "CCalendarStoreManager.h"

#include "CICalendar.h"
#include "CICalendarSync.h"

#include <LTableArrayStorage.h>
#include <UGAColorRamp.h>
#include <UStandardDialogs.h>

// ---------------------------------------------------------------------------
//	CCalendarStoreTable														  [public]
/**
	Default constructor */

CCalendarStoreTable::CCalendarStoreTable()
{
	InitCalendarStoreTable();
}

// Default constructor - just do parents' call
CCalendarStoreTable::CCalendarStoreTable(LStream *inStream)
	: CHierarchyTableDrag(inStream)
{
	InitCalendarStoreTable();
}


// ---------------------------------------------------------------------------
//	~CCalendarStoreTable														  [public]
/**
	Destructor */

CCalendarStoreTable::~CCalendarStoreTable()
{
}

#pragma mark -

void CCalendarStoreTable::InitCalendarStoreTable()
{
	mTableGeometry = new CTableRowGeometry(this, 32, 32);
	mTableSelector = new CTableMultiRowSelector(this);
	
	// Storage
	mTableStorage = new LTableArrayStorage(this, sizeof(calstore::CCalendarStoreNode*));
	
	mTableView = NULL;
	mListChanging = false;
	mHierarchyCol = 0;
}

// Get details of sub-panes
void CCalendarStoreTable::FinishCreateSelf(void)
{
	// Do inherited
	CHierarchyTableDrag::FinishCreateSelf();

	// Find table view in super view chain
	LView* super = GetSuperView();
	while(super && !dynamic_cast<CCalendarStoreView*>(super))
		super = super->GetSuperView();
	mTableView = dynamic_cast<CCalendarStoreView*>(super);

	// Set Drag & Drop pane to scroller
	mPane = GetSuperView();

	// Set Drag & Drop info
	SetTable(this, true);

	// Can drop servers, calendars & events
	AddDropFlavor(cDragCalServerType);
	AddDropFlavor(cDragCalendarType);
	AddDropFlavor(cDragCalendarItemType);

	// Set read only status of Drag and Drop
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

	// Turn on tooltips
	EnableTooltips();

	// Make it fit to the superview
	AdaptToNewSurroundings();

	SetTextTraits(CPreferences::sPrefs->mListTextTraits.GetValue().traits);
}

//	Respond to commands
Boolean CCalendarStoreTable::ObeyCommand(CommandT inCommand,void *ioParam)
{
	Boolean	cmdHandled = true;
	
	switch (inCommand)
	{
		case cmd_FileImport:
			OnImport();
			break;
			
		case cmd_FileExport:
			OnExport();
			break;
			
		case cmd_Properties:
		case cmd_ToolbarDetailsBtn:
			if (TestSelectionAnd((TestSelectionPP) &CCalendarStoreTable::TestSelectionServer))
				OnServerProperties();
			else if (TestSelectionAnd((TestSelectionPP) &CCalendarStoreTable::TestSelectionCalendarStoreNode))
				OnCalendarProperties();
			break;
			
		case cmd_ToolbarServerLoginBtn:
			OnLogin();
			break;
			
		case cmd_NewCalendar:
		case cmd_ToolbarNewCalendarBtn:
			OnNewCalendar();
			break;
			
		case cmd_RenameCalendar:
			OnRenameCalendar();
			break;
			
		case cmd_DeleteCalendar:
			OnDeleteCalendar();
			break;
			
		case cmd_CheckCalendar:
		case cmd_ToolbarCheckMailboxBtn:
			OnCheckCalendar();
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
			
		case cmd_RefreshCalendarList:
			OnRefreshList();
			break;
			
		case cmd_FreeBusyCalendar:
			OnFreeBusyCalendar();
			break;
			
		case cmd_SendCalendar:
			OnSendCalendar();
			break;
			
		case cmd_NewWebCalendar:
			OnNewWebCalendar();
			break;
			
		case cmd_RefreshWebCalendar:
			OnRefreshWebCalendar();
			break;
			
		case cmd_UploadWebCalendar:
			OnUploadWebCalendar();
			break;
			
		default:
			cmdHandled = CHierarchyTableDrag::ObeyCommand(inCommand, ioParam);
			break;
	}
	
	return cmdHandled;
}

//	Pass back status of a (menu) command
void CCalendarStoreTable::FindCommandStatus(
											CommandT	inCommand,
											Boolean		&outEnabled,
											Boolean		&outUsesMark,
											UInt16		&outMark,
											Str255		outName)
{
	outUsesMark = false;
	
	switch (inCommand)
	{
		case cmd_FileImport:
		{
			outEnabled = true;
			LStr255 txt(STRx_Standards, str_ImportCalendar);
			::PLstrcpy(outName, txt);
			break;
		}
		case cmd_FileExport:
		{
			outEnabled = TestSelectionAnd((TestSelectionPP) &CCalendarStoreTable::TestSelectionCanChangeCalendar);
			LStr255 txt(STRx_Standards, str_ExportCalendar);
			::PLstrcpy(outName, txt);
			break;
		}
			
			// These ones must have a selection
		case cmd_Properties:
		case cmd_ToolbarDetailsBtn:
			outEnabled = IsSelectionValid();;
			break;
			
		case cmd_ToolbarServerLoginBtn:
			// Logon button must have single server selected
			if (IsSingleSelection() && TestSelectionAnd((TestSelectionPP) &CCalendarStoreTable::TestSelectionServer))
			{
				TableIndexT row = GetFirstSelectedRow();
				calstore::CCalendarStoreNode* node = GetCellNode(row);
				
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
				if (node->GetProtocol()->IsOffline() && !node->GetProtocol()->IsDisconnected())
				{
					// Local items are always logged in (connected) so disable the button
					outEnabled = false;
					outUsesMark = true;
					::GetIndString(outName, STRx_Standards, str_Logoff);
				}
				
				// 2. (as above)
				else if (!node->GetProtocol()->CanDisconnect())
				{
					// 2.1 (as above)
					if (CConnectionManager::sConnectionManager.IsConnected())
					{
						outEnabled = true;
						outUsesMark = true;
						outMark = node->GetProtocol()->IsLoggedOn() ? (UInt16)checkMark : (UInt16)noMark;
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
						outMark = !node->GetProtocol()->IsDisconnected() ? (UInt16)checkMark : (UInt16)noMark;
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
			
		case cmd_NewCalendar:
		case cmd_ToolbarNewCalendarBtn:
			// Always enabled
			outEnabled = true;
			break;
			
		case cmd_RenameCalendar:
		case cmd_DeleteCalendar:
		case cmd_FreeBusyCalendar:
		case cmd_SendCalendar:
			// Only if calendar selection
			outEnabled = TestSelectionAnd((TestSelectionPP) &CCalendarStoreTable::TestSelectionCanChangeCalendar);
			break;
			
		case cmd_CheckCalendar:
		case cmd_ToolbarCheckMailboxBtn:
			// Always enabled
			outEnabled = true;
			break;
			
		case cmd_Hierarchy:
		case cmd_NewHierarchy:
			// Always able to do this, even if logged off or no selection
			outEnabled = true;
			break;
			
		case cmd_EditHierarchy:
		case cmd_DeleteHierarchy:
			outEnabled = TestSelectionAnd((TestSelectionPP) &CCalendarStoreTable::TestSelectionHierarchy);
			break;
			
		case cmd_RefreshCalendarList:
			// Only if single selection
			outEnabled = IsSingleSelection();
			break;
			
		case cmd_NewWebCalendar:
			// Always enabled if admin allows it
			outEnabled = !CAdminLock::sAdminLock.mNoLocalCalendars && 
			((calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol() == NULL) ||
			 !calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol()->IsDisconnected());
			break;
			
		case cmd_RefreshWebCalendar:
			// Always enabled if admin allows it
			outEnabled = !CAdminLock::sAdminLock.mNoLocalCalendars &&
			(calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol() != NULL) &&
			!calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol()->IsDisconnected() &&
			TestSelectionAnd((TestSelectionPP) &CCalendarStoreTable::TestSelectionWebCalendar);
			break;
			
		case cmd_UploadWebCalendar:
			// Always enabled if admin allows it
			outEnabled = !CAdminLock::sAdminLock.mNoLocalCalendars &&
			(calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol() != NULL) &&
			!calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol()->IsDisconnected() &&
			TestSelectionAnd((TestSelectionPP) &CCalendarStoreTable::TestSelectionUploadWebCalendar);
			break;
			
		default:
			CHierarchyTableDrag::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
			break;
	}
}

// Handle key presses
Boolean CCalendarStoreTable::HandleKeyPress(const EventRecord &inKeyEvent)
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

	switch (inKeyEvent.message & charCodeMask)
	{
	default:
		return CHierarchyTableDrag::HandleKeyPress(inKeyEvent);
	}

	return true;
}

void CCalendarStoreTable::ClickCell(const STableCell& inCell, const SMouseDownEvent& inMouseDown)
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
	if (event_match && (inCell.col == mHierarchyCol) &&
		DragAndDropIsPresent() &&
		(CellIsSelected(inCell) ||
			(!(inMouseDown.macEvent.modifiers & shiftKey) &&
			 !(inMouseDown.macEvent.modifiers & cmdKey))) &&
			 ValidDragSelection())
	{
		// Track item long enough to distinguish between a click to
		// select, and the beginning of a drag
		bool isDrag = !CContextMenuProcessAttachment::ProcessingContextMenu() &&
						::WaitMouseMoved(inMouseDown.macEvent.where);

		// Now do drag
		if (isDrag)
		{

			// If we leave the window, the drag manager will be changing thePort,
			// so we'll make sure thePort remains properly set.
			OutOfFocus(NULL);
			FocusDraw();
			OSErr err = CreateDragEvent(inMouseDown);
			OutOfFocus(NULL);

			// Now execute the drag task
			CDragTask::Execute();

			// Force refresh of selection
			RefreshSelection();

			return;
		}
	}

	// Determine which heading it is
	SColumnInfo col_info = mTableView->GetColumnInfo()[inCell.col - 1];

	switch(col_info.column_type)
	{
	case eCalendarStoreColumnSubscribe:
	case eCalendarStoreColumnColour:
		calstore::CCalendarStoreNode* node = GetCellNode(inCell.row);

		// Check for actual calendars
		if (!inMouseDown.delaySelect && node->IsViewableCalendar())
		{
			switch(col_info.column_type)
			{
			case eCalendarStoreColumnSubscribe:
				if (node->IsCached())
				{
					calstore::CCalendarStoreManager::sCalendarStoreManager->SubscribeNode(node, !node->IsSubscribed());
				}
				break;

			case eCalendarStoreColumnColour:
				DoChangeColour(inCell.row);
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

void CCalendarStoreTable::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	SwitchTarget(this);

	// Must set background color to grey if tracking drop flag
	StColorState	saveColors;

	STableCell	hitCell;
	SPoint32	imagePt;

	LocalToImagePoint(inMouseDown.whereLocal, imagePt);

	if (GetCellHitBy(imagePt, hitCell))
	{
		calstore::CCalendarStoreNode* node = GetCellNode(hitCell.row);
		if ((hitCell.col == mHierarchyCol) && UsesBackgroundColor(node))
		{
			FocusDraw();
			ApplyForeAndBackColors();
			::RGBBackColor(&GetBackgroundColor(node));
		}
	}

	CHierarchyTableDrag::ClickSelf(inMouseDown);
}

void CCalendarStoreTable::DrawCell(const STableCell &inCell, const Rect &inLocalRect)
{
	// Not if changing
	if (mListChanging)
		return;

	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row);

	calstore::CCalendarStoreNode* node = GetCellNode(inCell.row);

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
	if (UsesBackgroundColor(node))
	{
		Rect greyer = inLocalRect;
		greyer.bottom = greyer.top + 1;
		::EraseRect(&greyer);
		::RGBBackColor(&GetBackgroundColor(node));
		greyer = inLocalRect;
		greyer.top++;
		::EraseRect(&greyer);
	}
	else
		::EraseRect(&inLocalRect);

	SColumnInfo col_info = mTableView->GetColumnInfo()[inCell.col - 1];

	switch(col_info.column_type)
	{

	case eCalendarStoreColumnName:
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
		::Ploticns(&iconRect, atNone, selected_state ? ttSelected : ttNone, GetPlotIcon(node, GetCellCalendarProtocol(inCell.row)));

		// Get name of item
		theTxt = node->GetDisplayShortName();

		// Add protocol state descriptor
		if (node->IsProtocol())
		{
			if (node->GetProtocol()->IsDisconnected() && !CConnectionManager::sConnectionManager.IsConnected())
			{
				theTxt.AppendResource("UI::Server::TitleDisconnected");
			}
			else if (node->GetProtocol()->IsDisconnected() || !node->GetProtocol()->IsOffline() && !node->GetProtocol()->IsLoggedOn())
			{
				theTxt.AppendResource("UI::Server::TitleOffline");
			}
		}

		// Draw the string
		bool strike = false;
		SetTextStyle(node, GetCellCalendarProtocol(inCell.row), strike);
		::MoveTo(iconRect.right - 2, inLocalRect.bottom - mTextDescent);
		::DrawClippedStringUTF8(theTxt, inLocalRect.right - iconRect.right - 2, eDrawString_Left);
		if (strike)
		{
			::MoveTo(iconRect.right + 2, (inLocalRect.top + inLocalRect.bottom)/2);
			::LineTo(inLocalRect.right, (inLocalRect.top + inLocalRect.bottom)/2);
		}
		break;
	}

	case eCalendarStoreColumnSubscribe:
	{
		Rect	iconRect;
		iconRect.left = inLocalRect.left;
		iconRect.right = iconRect.left + 16;
		iconRect.bottom = inLocalRect.bottom - mIconDescent;
		iconRect.top = iconRect.bottom - 16;

		// Do status flag
		if (node->IsViewableCalendar())
		{
			if (node->IsCached())
			{
				if (node->IsSubscribed())
					::Ploticns(&iconRect, atNone, ttNone, ICNx_DiamondTicked);
				else
					::Ploticns(&iconRect, atNone, ttNone, ICNx_Diamond);
			}
			else
				::Ploticns(&iconRect, atNone, ttNone, ICNx_DisabledDiamond);
		}
		break;
	}

	case eCalendarStoreColumnColour:
		if (node->IsSubscribed())
		{
			uint32_t colour = calstore::CCalendarStoreManager::sCalendarStoreManager->GetCalendarColour(node);
			if (colour != 0)
			{
				Rect frame = inLocalRect;
				::InsetRect(&frame, 2, 2);
				RGBColor temp = UGAColorRamp::GetBlackColor();
				::RGBForeColor(&temp);
				::MacFrameRect(&frame);

				::InsetRect(&frame, 1, 1);
				temp = CGUtils::GetQDColor(colour);
				::RGBForeColor(&temp);
				::PaintRect(&frame);
			}
		}
		break;

	default:
		break;
	}
}

void CCalendarStoreTable::CalcCellFlagRect(const STableCell &inCell, Rect &outRect)
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
ResIDT CCalendarStoreTable::GetPlotIcon(const calstore::CCalendarStoreNode* node, calstore::CCalendarProtocol* proto)
{
	if (node->IsProtocol())
	{
		if (!proto)
			return ICNx_BrowseLocalHierarchy;
		else if (proto->GetAccountType() == CCalendarAccount::eHTTPCalendar)
			return 1818;
		else if (proto->CanDisconnect())
			return proto->IsDisconnected() ? ICNx_BrowseDisconnectedHierarchy : ICNx_BrowseRemoteHierarchy;
		else if (proto->GetAccountType() == CINETAccount::eLocalCalendar)
			return ICNx_BrowseLocalHierarchy;
		else
			return ICNx_BrowseRemoteHierarchy;
	}
	else if (node->IsDisplayHierarchy())
	{
		return ICNx_BrowseSearchHierarchy;
	}
	else if (node->IsDirectory())
	{
		return ICNx_BrowseDirectory;
	}
	else
	{
		if (node->IsCached())
			if (node->IsInbox())
				return 1823;
			else if (node->IsOutbox())
				return 1824;
			else
				return 1807;
		else
			return 1817;
	}
}

// Get appropriate text style
void CCalendarStoreTable::SetTextStyle(const calstore::CCalendarStoreNode* node, calstore::CCalendarProtocol* proto, bool& strike)
{
	strike = false;

	// Select appropriate color and style of text
	if (UEnvironment::HasFeature(env_SupportsColor))
	{
		if (node->IsProtocol())
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

			if (node->IsSubscribed())
			{
				iCal::CICalendar* cal = node->GetCalendar();
				if (node->IsInbox() && (cal != NULL) && cal->HasData())
				{
					text_color = CPreferences::sPrefs->mMboxUnseenStyle.GetValue().color;
					color_set = true;
					text_style = text_style | CPreferences::sPrefs->mMboxUnseenStyle.GetValue().style & 0x007F;
					strike =  strike || ((CPreferences::sPrefs->mMboxUnseenStyle.GetValue().style & 0x0080) != 0);
				}
				else
				{				
					text_color = CPreferences::sPrefs->mMboxFavouriteStyle.GetValue().color;
					color_set = true;
					text_style = text_style | CPreferences::sPrefs->mMboxFavouriteStyle.GetValue().style & 0x007F;
					strike =  strike || ((CPreferences::sPrefs->mMboxFavouriteStyle.GetValue().style & 0x0080) != 0);
				}
			}
			if (!color_set)
			{
				text_color = CPreferences::sPrefs->mMboxClosedStyle.GetValue().color;
				color_set = true;
				text_style = text_style | CPreferences::sPrefs->mMboxClosedStyle.GetValue().style & 0x007F;
				strike =  strike || ((CPreferences::sPrefs->mMboxClosedStyle.GetValue().style & 0x0080) != 0);
			}

			::RGBForeColor(&text_color);
			::TextFace(text_style);
		}
	}
}

bool CCalendarStoreTable::UsesBackgroundColor(const calstore::CCalendarStoreNode* node) const
{
	return node->IsProtocol() || node->IsDisplayHierarchy();
}

const RGBColor& CCalendarStoreTable::GetBackgroundColor(const calstore::CCalendarStoreNode* node) const
{
	if (node->IsDisplayHierarchy())
	{
		return CPreferences::sPrefs->mHierarchyBkgndStyle.GetValue().color;
	}
	else
	{
		return CPreferences::sPrefs->mServerBkgndStyle.GetValue().color;
	}
}

void CCalendarStoreTable::DoChangeColour(TableIndexT row)
{
	// Must be a valid active calendar
	calstore::CCalendarStoreNode* node = GetCellNode(row);

	if (node->IsSubscribed())
	{
		uint32_t colour = calstore::CCalendarStoreManager::sCalendarStoreManager->GetCalendarColour(node);

		Point		where = { 0, 0 };
		RGBColor	inColor = CGUtils::GetQDColor(colour);
		RGBColor	outColor;
		LStr255		prompt(rsrc::GetString("CCalendarStoreTable::PickCalendarColour"));

		UDesktop::Deactivate ();

		if (::GetColor(where, prompt, &inColor, &outColor))
		{
			calstore::CCalendarStoreManager::sCalendarStoreManager->ChangeNodeColour(node, CGUtils::GetColor(outColor));
			RefreshRow(row);
		}

		UDesktop::Activate ();
	}
}

// Keep titles in sync
void CCalendarStoreTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, Boolean inRefresh)
{
	// Find titles in owner chain
	mTableView->GetBaseTitles()->ScrollImageBy(inLeftDelta, 0, inRefresh);

	CHierarchyTableDrag::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
}

// Import selected calendars
void CCalendarStoreTable::DoImportCalendar(calstore::CCalendarStoreNode* node, bool merge)
{
	// Must have valid node
	if (node == NULL)
		return;
	
	// Pick file to import from
	PPx::FSObject fspec;
	if (PP_StandardDialogs::AskOpenOneFile(0, fspec, kNavDefaultNavDlogOptions | kNavAllowPreviews | kNavAllFilesInPopup))
	{
		// Get full path and create stream
		cdstring fpath(fspec.GetPath());
		cdifstream fin(fpath);
		if (fin.fail())
			return;
		
		// May need to activate if not already
		calstore::CCalendarStoreManager::StNodeActivate _activate(node);
		
		iCal::CICalendar* cal_server = node->GetCalendar();
		if (cal_server != NULL)
		{
			if (merge)
			{
				// Read in new calendar
				iCal::CICalendar cal_local;
				cal_local.Parse(fin);
				
				// Do sync with node
				iCal::CICalendarSync sync(*cal_server, cal_local);
				sync.Sync();
				cal_server->SetDirty(true);
				cal_server->SetTotalReplace(true);
			}
			else
			{
				// Read calendar from file replacing existing data
				iCal::CICalendar* cal_server = node->GetCalendar();
				if (cal_server != NULL)
				{
					cal_server->Clear();
					cal_server->Parse(fin);
					cal_server->SetDirty(true);
					cal_server->SetTotalReplace(true);
				}
			}
		}
	}
}

// Export selected calendars
bool CCalendarStoreTable::ExportCalendar(TableIndexT row)
{
	// Get calendar for hit cell
	bool failagain = false;
	calstore::CCalendarStoreNode* node = GetCellNode(row);

	if (node == NULL)
		return false;

	// Pick file to export to
	cdstring name = node->GetDisplayShortName();
	name += ".ics";
	PPx::CFString cfstr(name.c_str(), kCFStringEncodingUTF8);
	PPx::FSObject fsspec;
	bool replacing;
	if (PP_StandardDialogs::AskSaveFile(cfstr, 'TEXT', fsspec, replacing, kNavDefaultNavDlogOptions | kNavNoTypePopup))
	{
		// Fix for window manager bug after a replace operation
		Activate();

		// Must create file in order for full path to work
		if (!replacing)
		{
			LFile file(fsspec);
			file.CreateNewFile('****', 'TEXT', smCurrentScript);
			fsspec.Update();
		}

		// Get full path and create stream
		cdstring fpath(fsspec.GetPath());
		cdofstream fout(fpath, std::ios_base::in | std::ios_base::binary | std::ios_base::trunc);
		if (fout.fail())
			return false;
		
		// May need to activate if not already
		calstore::CCalendarStoreManager::StNodeActivate _activate(node);
		
		// Write calendar to file
		iCal::CICalendar* cal = node->GetCalendar();
		if (cal != NULL)
			cal->Generate(fout);
		
		return true;
	}
	
	return false;
}

#pragma mark ____________________________Drag & Drop

// Check for valid drag selection
bool CCalendarStoreTable::ValidDragSelection() const
{
	// Loop over all selected cells
	int got_server = 0;
	int got_calendar = 0;
	STableCell selCell(0, 0);
	while(GetNextSelectedCell(selCell))
	{
		calstore::CCalendarStoreNode* node = GetCellNode(selCell.row, false);
		if (node->IsProtocol())
		{
			// Cannot drag either the local or web calendars protocols
			if ((node->GetProtocol()->GetAccountType() == CINETAccount::eLocalCalendar) || (node->GetProtocol()->GetAccountType() == CCalendarAccount::eHTTPCalendar))
				return false;
			
			got_server = 1;
		}
		else if (node->IsInbox() || node->IsOutbox())
			// Cannot operate on Inbox/Outbox
			got_calendar = 2;
		else
			got_calendar = 1;

		// Can only have one type
		if (got_server + got_calendar > 1)
			return false;
	}

	return true;
}

// Add drag cells
void CCalendarStoreTable::AddCellToDrag(CDragIt* theDragTask, const STableCell& theCell, Rect& dragRect)
{
	const void* data;
	UInt32 dataSize = sizeof(void*);
	FlavorType flavor;
	FlavorFlags flags = 0;

	// Assume drag only starts when selection is all the same type
	// Thus type of this cell is type of all cells
	calstore::CCalendarStoreNode* node = GetCellNode(theCell.row);

	if (node->IsProtocol())
	{
		// Dragging server to another location
		data = node->GetProtocol();
		flavor = cDragCalServerType;
	}
	else if (!node->IsInbox() && !node->IsOutbox())
	{
		// Dragging mailbox
		data = node;
		flavor = cDragCalendarType;
	}

	// Add this to drag
	if (data != NULL)
		theDragTask->AddFlavorItem(dragRect, (ItemReference) data, flavor, &data, dataSize, flags, true);
}

// Can cell expand for drop
bool CCalendarStoreTable::CanDropExpand(DragReference inDragRef, unsigned long woRow)
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
	case cDragCalServerType:
		return false;
	case cDragCalendarType:
	case cDragCalendarItemType:
		return CHierarchyTableDrag::CanDropExpand(inDragRef, woRow);
	default:
		// Anything else cannot
		return false;
	}
}

// Adjust cursor over drop area
bool CCalendarStoreTable::IsCopyCursor(DragReference inDragRef)
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
	case cDragCalendarItemType:
	{
		// Toggle based on modifiers
		short mouseModifiers;
		::GetDragModifiers(inDragRef, &mouseModifiers, NULL, NULL);
		bool option_key = mouseModifiers & optionKey;

		return option_key;
	}
	case cDragCalServerType:
		// Server always moved
		return false;
	case cDragCalendarType:
		// Calendars moved if drop at or no drop cell
		if (mLastDropCursor.row || !mLastDropCell.row)
			return false;

		// Must check cell dropped into to see if directory
		// Get drop cell type
		calstore::CCalendarStoreNode* node = GetCellNode(mLastDropCell.row);
		if (node->IsDirectory())
			return false;
		else
			return true;

	default:
		// Anything else is an error!
		return false;
	}
}

// Test drag insert cursor
bool CCalendarStoreTable::IsDropCell(DragReference inDragRef, STableCell theCell)
{
	if (IsValidCell(theCell))
	{
		// Get flavor for this item
		ItemReference theItemRef;
		FlavorType theFlavor;
		Size dataSize = 4;
		void* data = NULL;
		if (::GetDragItemReferenceNumber(inDragRef, 1, &theItemRef) != noErr)
			return false;
		if (::GetFlavorType(inDragRef, theItemRef, 1, &theFlavor) != noErr)
			return false;
		if (::GetFlavorData(inDragRef, theItemRef, theFlavor, &data, &dataSize, 0) != noErr)
			return false;

		// Get drop cell type
		calstore::CCalendarStoreNode* node = GetCellNode(theCell.row);

		switch(theFlavor)
		{
		case cDragCalendarItemType:
		{
			// Drop into valid calendars only
			return node->IsViewableCalendar();
		}
		case cDragCalServerType:
			// Servers always moved
			return false;
		case cDragCalendarType:
		{
			if (node->IsProtocol() || node->IsInbox() || node->IsOutbox())
				return false;
			else
				// Allow drop into any calendar (directory = move, calendar = copy)
				return true;
		}
		default:
			// Anything else is an error!
			return false;
		}
	}
	else
		return false;
}

// Test drop at cell
bool CCalendarStoreTable::IsDropAtCell(DragReference inDragRef, STableCell& aCell)
{
	// Adjust for end of table
	int adjust = 0;
	if (aCell.row > mRows)
		adjust = 1;

	if (IsValidCell(aCell) || adjust)
	{
		// Get flavor for this item
		ItemReference theItemRef;
		FlavorType theFlavor;
		Size dataSize = 4;
		void* data = NULL;
		if (::GetDragItemReferenceNumber(inDragRef, 1, &theItemRef) != noErr)
			return false;
		if (::GetFlavorType(inDragRef, theItemRef, 1, &theFlavor) != noErr)
			return false;
		if (::GetFlavorData(inDragRef, theItemRef, theFlavor, &data, &dataSize, 0) != noErr)
			return false;

		// Get drop cell type
		calstore::CCalendarStoreNode* node = GetCellNode(aCell.row - adjust);
		TableIndexT woRow = GetWideOpenIndex(aCell.row);

		switch(theFlavor)
		{
		case cDragCalendarItemType:
		{
			// Always drop into, never at
			return false;
		}
		case cDragCalServerType:
			// Must be same window
			if (sTableSource != this)
				return false;

			// Server can only be dropped before/after server in the same window
			if (node->IsProtocol())
				return true;

			// Beyond last row allowed
			if (adjust)
				return true;

			// Check for row immediately after server
			if (woRow > 1)
			{
				calstore::CCalendarStoreNode* prev_node = GetCellNode(woRow - 1 + adjust, true);
				return (prev_node->IsProtocol());
			}
			return false;
		case cDragCalendarType:
		{
			return true;
		}
		default:
			// Anything else is an error!
			return false;
		}
	}
	else
		return false;
}

// Drop data into cell
void CCalendarStoreTable::DropData(FlavorType theFlavor, char* drag_data, Size data_sizel)
{
	switch(theFlavor)
	{
	case cDragCalServerType:
	{
		// Make this window show this server if not manager
		//new CServerDragOpenServerTask(this, *(CMboxProtocol**) drag_data);
		break;
	}
	default:;
	}

}

// Drop data into cell
void CCalendarStoreTable::DropDataIntoCell(FlavorType theFlavor, char* drag_data,
										Size data_size, const STableCell& theCell)
{
	switch(theFlavor)
	{
	case cDragCalendarItemType:
	{
		break;
	}
	case cDragCalendarType:
	{
		calstore::CCalendarStoreNode* drag = *(calstore::CCalendarStoreNode**) drag_data;

		// Get drop cell type
		calstore::CCalendarStoreNode* node = GetCellNode(theCell.row);
		if (!node->IsProtocol() && !node->IsInbox() && !node->IsOutbox())
		{
			CDragCalendarIntoCalendarTask* task = dynamic_cast<CDragCalendarIntoCalendarTask*>(CDragTask::GetCurrentDragTask());
			if (task == NULL)
			{
				task = new CDragCalendarIntoCalendarTask(node);
			}
			if (task != NULL)
				task->AddCalendar(drag);
		}
		break;
	}
	default:;
	}

}

// Drop data at cell
void CCalendarStoreTable::DropDataAtCell(FlavorType theFlavor,
										char* drag_data,
										Size data_size,
										const STableCell& beforeCell)
{
	// Adjust for end of table
	int adjust = 0;
	if (beforeCell.row > mRows)
		adjust = 1;

	// Get drop cell type
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(beforeCell.row + TABLE_ROW_ADJUST - adjust);
	calstore::CCalendarStoreNode* node = GetCellNode(beforeCell.row - adjust);

	switch(theFlavor)
	{
	case cDragCalServerType:
	{
		CDragCalendarServerTask* task = dynamic_cast<CDragCalendarServerTask*>(CDragTask::GetCurrentDragTask());
		if (!task)
		{
			long dropat = -1L;
			if (adjust)
				dropat = calstore::CCalendarStoreManager::sCalendarStoreManager->GetProtocolCount();
			else
			{
				calstore::CCalendarProtocol* above = node->GetProtocol();
				if (node->IsProtocol())
					dropat = calstore::CCalendarStoreManager::sCalendarStoreManager->GetProtocolIndex(above);
				else
					dropat = calstore::CCalendarStoreManager::sCalendarStoreManager->GetProtocolIndex(above) + 1;
			}
			task = new CDragCalendarServerTask(dropat);
		}

		calstore::CCalendarProtocol* moved = *(calstore::CCalendarProtocol**) drag_data;
		long old_index = calstore::CCalendarStoreManager::sCalendarStoreManager->GetProtocolIndex(moved);
		task->AddIndex(old_index);
		break;
	}

	case cDragCalendarType:
	{
		calstore::CCalendarStoreNode* drag = *(calstore::CCalendarStoreNode**) drag_data;
		if (!node->IsProtocol())
		{
			CDragCalendarAtCalendarTask* task = dynamic_cast<CDragCalendarAtCalendarTask*>(CDragTask::GetCurrentDragTask());
			if (task == NULL)
			{
				task = new CDragCalendarAtCalendarTask(node->GetParent(), node);
			}
			if (task != NULL)
				task->AddCalendar(drag);
		}
		break;
	}
	default:;
	}

}
