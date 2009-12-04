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
#include "CCalendarView.h"
#include "CCalendarStoreTitleTable.h"
#include "CCalendarStoreView.h"
#include "CCalendarUtils.h"
#include "CCommands.h"
#include "CErrorHandler.h"
#include "CIconLoader.h"
#include "CLocalCommon.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CMulSelectionData.h"
#include "CPreferences.h"
#include "CTableRowSelector.h"
#include "CTableRowGeometry.h"
#include "CXStringResources.h"

#include "CCalendarStoreNode.h"
#include "CCalendarStoreManager.h"

#include "CICalendar.h"
#include "CICalendarSync.h"

#include "cdfstream.h"

#include "StPenState.h"

#include <UNX_LTableArrayStorage.h>

#include <JPainter.h>
#include <JXChooseColorDialog.h>
#include <JXChooseSaveFile.h>
#include <JXColormap.h>
#include <JXDialogDirector.h>
#include <JXDNDManager.h>
#include <JXImage.h>
#include <JXWindow.h>
#include <jFileUtil.h>
#include <jXGlobals.h>

const short cTextIndent = 26;

// ---------------------------------------------------------------------------
//	CCalendarStoreTable														  [public]
/**
	Default constructor */

CCalendarStoreTable::CCalendarStoreTable(JXScrollbarSet* scrollbarSet, 
										JXContainer* enclosure,
										const HSizingOption hSizing, 
										const VSizingOption vSizing,
										const JCoordinate x, const JCoordinate y,
										const JCoordinate w, const JCoordinate h)
	: CHierarchyTableDrag(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
	SetBorderWidth(0);
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
	mTableSelector = new CTableRowSelector(this);
	
	// Storage
	mTableStorage = new LTableArrayStorage(this, sizeof(calstore::CCalendarStoreNode*));
	
	mTableView = NULL;
	mListChanging = false;
	mHierarchyCol = 1;

	SetRowSelect(true);
	
	mColourChooser = NULL;
	mColourChosen = false;
}

// Get details of sub-panes
void CCalendarStoreTable::OnCreate()
{
	// Find table view in super view chain
	const JXContainer* parent = GetEnclosure();
	while(parent && !dynamic_cast<const CCalendarStoreView*>(parent))
		parent = parent->GetEnclosure();
	mTableView = const_cast<CCalendarStoreView*>(dynamic_cast<const CCalendarStoreView*>(parent));

	// Do inherited
	CHierarchyTableDrag::OnCreate();

	// Can drop servers, calendars & events
	AddDropFlavor(CMulberryApp::sFlavorCalServer);
	AddDropFlavor(CMulberryApp::sFlavorCalendar);
	AddDropFlavor(CMulberryApp::sFlavorCalendarItem);

	// Set read only status of Drag and Drop
	SetReadOnly(false);
	SetDropCell(true);
	SetDropCursor(true);
	SetAllowDrag(true);
	SetAllowMove(true);
	SetSelfDrag(true);

	CreateContextMenu(CMainMenu::eContextCalendarStore);
}

// Keep titles in sync
void CCalendarStoreTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh)
{
	// Find titles in owner chain
	mTableView->GetBaseTitles()->ScrollImageBy(inLeftDelta, 0, inRefresh);

	CHierarchyTableDrag::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
}

void CCalendarStoreTable::Receive(JBroadcaster* sender, const Message& message)
{
	if (sender == mColourChooser && message.Is(JXDialogDirector::kDeactivated))
	{
		const JXDialogDirector::Deactivated* info = dynamic_cast<const JXDialogDirector::Deactivated*>(&message);
		assert( info != NULL );
		if (info->Successful())
		{
			mColourChoose = GetColormap()->JColormap::GetRGB(mColourChooser->GetColor());
			mColourChosen = true;
		}
		else
			mColourChosen = false;
		mColourChooser = NULL;
		return;
	}

	CHierarchyTableDrag::Receive(sender, message);
}

//	Respond to commands
bool CCalendarStoreTable::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eFileImport:
		OnImport();
		return true;

	case CCommand::eFileExport:
		OnExport();
		return true;

	case CCommand::eEditProperties:
	case CCommand::eToolbarDetailsBtn:
		if (TestSelectionAnd((TestSelectionPP) &CCalendarStoreTable::TestSelectionServer))
			OnServerProperties();
		else if (TestSelectionAnd((TestSelectionPP) &CCalendarStoreTable::TestSelectionCalendarStoreNode))
			OnCalendarProperties();
		return true;

	case CCommand::eToolbarServerLoginBtn:
		OnLogin();
		return true;

	case CCommand::eCalendarCreate:
	case CCommand::eToolbarCalendarCreateBtn:
		OnNewCalendar();
		return true;

	case CCommand::eCalendarRename:
		OnRenameCalendar();
		return true;

	case CCommand::eCalendarDelete:
		OnDeleteCalendar();
		return true;

	case CCommand::eCalendarCheck:
	case CCommand::eToolbarCalendarCheckBtn:
		OnNewCalendar();
		return true;

	case CCommand::eDispHNew:
		OnNewHierarchy();
		return true;

	case CCommand::eDispHEdit:
		OnRenameHierarchy();
		return true;

	case CCommand::eDispHRemove:
		OnDeleteHierarchy();
		return true;

	case CCommand::eCalendarRefresh:
		OnRefreshList();
		return true;

	case CCommand::eCalendarFreeBusy:
		OnFreeBusyCalendar();
		return true;

	case CCommand::eCalendarNewMessage:
		OnSendCalendar();
		return true;

	case CCommand::eCalendarAddWeb:
		OnNewWebCalendar();
		return true;

	case CCommand::eCalendarRefreshWeb:
		OnRefreshWebCalendar();
		return true;

	case CCommand::eCalendarUploadWeb:
		OnUploadWebCalendar();
		return true;

	default:;
	}

	return CHierarchyTableDrag::ObeyCommand(cmd, menu);
}

//	Pass back status of a (menu) command
void CCalendarStoreTable::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eFileImport:
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eFileExport:
		OnUpdateSelectionCalendarStoreNode(cmdui);
		return;

	// These ones must have a selection
	case CCommand::eEditProperties:
	case CCommand::eToolbarDetailsBtn:
		OnUpdateSelection(cmdui);
		return;

	case CCommand::eToolbarServerLoginBtn:
		OnUpdateLogin(cmdui);
		return;

	case CCommand::eCalendarCreate:
	case CCommand::eToolbarCalendarCreateBtn:
		// Always enabled
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eCalendarRename:
	case CCommand::eCalendarDelete:
	case CCommand::eCalendarFreeBusy:
	case CCommand::eCalendarNewMessage:
		// Only if calendar selection;
		OnUpdateSelectionCanChangeCalendar(cmdui);
		return;

	case CCommand::eCalendarRefresh:
		// Only if single selection;
		OnUpdateRefreshList(cmdui);
		return;

	case CCommand::eCalendarAddWeb:
		// Always enabled if admin allows it
		OnUpdateNewWebCalendar(cmdui);
		return;

	case CCommand::eCalendarRefreshWeb:
		// Always enabled if admin allows it
		OnUpdateRefreshWebCalendar(cmdui);
		return;

	case CCommand::eCalendarUploadWeb:
		// Always enabled if admin allows it
		OnUpdateUploadWebCalendar(cmdui);
		return;

	default:;
	}

	CHierarchyTableDrag::UpdateCommand(cmd, cmdui);
}

// Handle key presses
bool CCalendarStoreTable::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	// Look for preview/full view based on key stroke

	// Determine whether preview is triggered
	CKeyModifiers mods(modifiers);
	const CUserAction& preview = mTableView->GetPreviewAction();
	if ((preview.GetKey() == key) &&
		(preview.GetKeyModifiers() == mods))
	{
		DoPreview();
		return true;
	}

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if ((fullview.GetKey() == key) &&
		(fullview.GetKeyModifiers() == mods))
	{
		DoFullView();
		return true;
	}

	return CHierarchyTableDrag::HandleChar(key, modifiers);
}

void CCalendarStoreTable::LClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	// Determine which heading it is
	SColumnInfo col_info = mTableView->GetColumnInfo()[inCell.col - 1];

	switch(col_info.column_type)
	{
	case eCalendarStoreColumnSubscribe:
	case eCalendarStoreColumnColour:
	{
		calstore::CCalendarStoreNode* node = GetCellNode(inCell.row);

		// Check for actual calendars
		if (node->IsViewableCalendar())
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

			RefreshRow(inCell.row);
		}
		break;
	}

	default:
		CHierarchyTableDrag::LClickCell(inCell, modifiers);
		DoSingleClick(inCell.row, CKeyModifiers(modifiers));
		break;
	}
}

// Double-clicked item
void CCalendarStoreTable::LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	DoDoubleClick(inCell.row, CKeyModifiers(modifiers));
}

void CCalendarStoreTable::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
	// Not if changing
	if (mListChanging)
		return;

	TableIndexT	woRow = GetWideOpenIndex(inCell.row);

	calstore::CCalendarStoreNode* node = GetCellNode(inCell.row);

	// Erase to ensure drag hightlight is overwritten
	unsigned long bkgnd = 0x00FFFFFF;
	if (UsesBackgroundColor(node))
	{
		StPenState save(pDC);
		JRect bgrect(inLocalRect);
		//If we don't make the rectangle smaller, there will be no border between
		//color stripes.  If we just use the table row's border, then the tree
		//lines get broken up.
		bgrect.bottom -=1;
		
		JColorIndex bkgnd_index = GetBackgroundColor(node);
		pDC->SetPenColor(bkgnd_index);
		pDC->SetFilling(kTrue);
		pDC->Rect(bgrect);
		
		JSize r, g, b;
		GetColormap()->GetRGB(bkgnd_index, &r, &g, &b);
		bkgnd = ((r & 0xFF00) << 8) | (g & 0xFF00) | ((b & 0xFF00) >> 8);
	}

	// Determine which heading it is
	SColumnInfo col_info = mTableView->GetColumnInfo()[inCell.col - 1];
	switch(col_info.column_type)
	{

	case eCalendarStoreColumnName:
	{
		// Always cache column number
		mHierarchyCol = inCell.col;

		// Get suitable icon
		ResIDT iconID = GetPlotIcon(node, GetCellCalendarProtocol(inCell.row));
		JXImage* icon = CIconLoader::GetIcon(iconID, this, 16, bkgnd);

		// Get name of item
		cdstring theTxt = node->GetDisplayShortName();

		// Add protocol state descriptor
		if (node->IsProtocol())
		{
			if (node->GetProtocol()->IsDisconnected() && !CConnectionManager::sConnectionManager.IsConnected())
			{
				theTxt.AppendResource("UI::Server::TitleDisconnected");
			}
			else if (node->GetProtocol()->IsDisconnected() || (!node->GetProtocol()->IsOffline() && !node->GetProtocol()->IsLoggedOn()))
			{
				theTxt.AppendResource("UI::Server::TitleOffline");
			}
		}

		// Draw the string
		bool strike = false;
		SetTextStyle(pDC, node, GetCellCalendarProtocol(inCell.row), strike);
		int text_start = DrawHierarchyRow(pDC, inCell.row, inLocalRect, theTxt, icon);
		break;
	}

	case eCalendarStoreColumnSubscribe:
		// Do status flag
		if (node->IsViewableCalendar())
		{
			ResIDT iconID;
			if (node->IsCached())
			{
				if (node->IsSubscribed())
					iconID = IDI_DIAMONDTICKED;
				else
					iconID = IDI_DIAMOND;
			}
			else
				iconID = IDI_DIAMONDDISABLE;
			JXImage* icon = CIconLoader::GetIcon(iconID, this, 16, bkgnd);
			pDC->Image(*icon, icon->GetBounds(), inLocalRect.left, inLocalRect.top + mIconOrigin);
		}
		break;

	case eCalendarStoreColumnColour:
		if (node->IsSubscribed())
		{
			uint32_t colour = calstore::CCalendarStoreManager::sCalendarStoreManager->GetCalendarColour(node);
			if (colour != 0)
			{
				// Black outline
				JRect rect = inLocalRect;
				rect.Shrink(2, 2);
				pDC->SetPenColor(GetColormap()->GetBlackColor());
				pDC->RectInside(rect);

				// Colour interior
				rect.Shrink(1, 1);
				JColorIndex cindex;
				GetColormap()->JColormap::AllocateStaticColor(CCalendarUtils::GetRGBColor(colour), &cindex);
				pDC->SetPenColor(cindex);
				pDC->SetFilling(kTrue);
				pDC->RectInside(rect);
			}
		}
		break;

	default:
		break;
	}
}

// Get appropriate icon id
ResIDT CCalendarStoreTable::GetPlotIcon(const calstore::CCalendarStoreNode* node, calstore::CCalendarProtocol* proto)
{
	if (node->IsProtocol())
	{
		if (!proto)
			return IDI_SERVERLOCAL;
		else if (proto->GetAccountType() == CCalendarAccount::eHTTPCalendar)
			return IDI_CALENDARWEBCAL;
		else if (proto->CanDisconnect())
			return proto->IsDisconnected() ? IDI_SERVERDISCONNECTED : IDI_SERVERREMOTE;
		else if (proto->GetAccountType() == CINETAccount::eLocalCalendar)
			return IDI_SERVERLOCAL;
		else
			return IDI_SERVERREMOTE;
	}
	else if (node->IsDisplayHierarchy())
	{
		return IDI_SERVERSEARCHHIER;
	}
	else if (node->IsDirectory())
	{
		return IDI_SERVERDIR;
	}
	else
	{
		if (node->IsCached())
		{
			if (node->IsInbox())
				return IDI_CALENDAR;
			else if (node->IsOutbox())
				return IDI_CALENDAR;
			else
				return IDI_CALENDAR;
		}
		else
		{
			IDI_CALENDARUNCACHED;
		}
	}
}

// Get appropriate text style
void CCalendarStoreTable::SetTextStyle(JPainter* pDC, const calstore::CCalendarStoreNode* node, calstore::CCalendarProtocol* proto, bool& strike)
{
	strike = false;

	// Select appropriate color and style of text
	bool color_set = false;
	bool style_set = false;
	JFontStyle text_style;
	short style = normal;

	if (node->IsProtocol())
	{
		if (!proto || proto->IsLoggedOn())
		{
			GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->mServerOpenStyle.GetValue().color, &text_style.color);
			style = CPreferences::sPrefs->mServerOpenStyle.GetValue().style;
		}
		else
		{
			GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->mServerClosedStyle.GetValue().color, &text_style.color);
			style = CPreferences::sPrefs->mServerClosedStyle.GetValue().style;
		}
		
		color_set = true;
		style_set = true;
	}
	else
	{
		if (node->IsSubscribed())
		{
			iCal::CICalendar* cal = node->GetCalendar();
			if (node->IsInbox() && (cal != NULL) && cal->HasData())
			{
				GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->mMboxUnseenStyle.GetValue().color, &text_style.color);
				color_set = true;
				style = style | CPreferences::sPrefs->mMboxUnseenStyle.GetValue().style;
				style_set = true;
			}
			else
			{
				GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->mMboxFavouriteStyle.GetValue().color, &text_style.color);
				color_set = true;
				style = style | CPreferences::sPrefs->mMboxFavouriteStyle.GetValue().style;
				style_set = true;
			}
		}
		if (!color_set)
		{
			GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->mMboxClosedStyle.GetValue().color, &text_style.color);
			color_set = true;
			style = style | CPreferences::sPrefs->mMboxClosedStyle.GetValue().style;
			style_set = true;
		}
	}

	text_style.bold = JBoolean(style & bold);
	text_style.italic = JBoolean(style & italic);
	text_style.underlineCount = JBoolean((style & underline) ? 1 : 0);
	text_style.strike = JBoolean(style & strike);

	if (color_set)
		pDC->SetPenColor(text_style.color);
	else
		text_style.color = pDC->GetPenColor();

	if (style_set)
		pDC->SetFontStyle(text_style);
}

bool CCalendarStoreTable::UsesBackgroundColor(const calstore::CCalendarStoreNode* node) const
{
	return node->IsProtocol() || node->IsDisplayHierarchy();
}

JColorIndex CCalendarStoreTable::GetBackgroundColor(const calstore::CCalendarStoreNode* node) const
{
	JColorIndex ret;
	if (node->IsDisplayHierarchy())
		GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->mHierarchyBkgndStyle.GetValue().color, &ret);
	else
		GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->mServerBkgndStyle.GetValue().color, &ret);
	return ret;
}

void CCalendarStoreTable::OnUpdateSelectionCalendarStoreNode(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TestSelectionAnd((TestSelectionPP) &CCalendarStoreTable::TestSelectionCalendarStoreNode));
}

void CCalendarStoreTable::OnUpdateSelectionCanChangeCalendar(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TestSelectionAnd((TestSelectionPP) &CCalendarStoreTable::TestSelectionCanChangeCalendar));
}

void CCalendarStoreTable::OnUpdateLogin(CCmdUI* pCmdUI)
{
	// Logon button must have single server selected
	if (IsSingleSelection() && TestSelectionAnd((TestSelectionPP) &CCalendarStoreTable::TestSelectionServer))
	{
		TableIndexT row = GetFirstSelectedRow();
		calstore::CCalendarStoreNode* node = GetCellNode(row);

		pCmdUI->Enable(node->IsProtocol());
		pCmdUI->SetCheck(node->IsProtocol() && node->GetProtocol()->IsLoggedOn());
		cdstring title;
		title.FromResource(node->IsProtocol() && node->GetProtocol()->IsLoggedOn() ? IDS_LOGOUT : IDS_LOGIN);
		pCmdUI->SetText(title);

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
			pCmdUI->Enable(false);
			cdstring title;
			title.FromResource(IDS_LOGOUT);
			pCmdUI->SetText(title);
		}

		// 2. (as above)
		else if (!node->GetProtocol()->CanDisconnect())
		{
			// 2.1 (as above)
			if (CConnectionManager::sConnectionManager.IsConnected())
			{
				pCmdUI->Enable(true);
				pCmdUI->SetCheck(node->GetProtocol()->IsLoggedOn());
				cdstring title;
				title.FromResource(!node->GetProtocol()->IsLoggedOn() ? IDS_LOGIN : IDS_LOGOUT);
				pCmdUI->SetText(title);
			}
			// 2.2 (as above)
			else
			{
				pCmdUI->Enable(false);
				cdstring title;
				title.FromResource(IDS_LOGOUT);
				pCmdUI->SetText(title);
			}
		}
		
		// 3. (as above)
		else
		{
			// 3.1 (as above)
			if (CConnectionManager::sConnectionManager.IsConnected())
			{
				pCmdUI->Enable(true);
				pCmdUI->SetCheck(!node->GetProtocol()->IsDisconnected());
				cdstring title;
				title.FromResource(node->GetProtocol()->IsDisconnected() ? IDS_LOGIN : IDS_LOGOUT);
				pCmdUI->SetText(title);
			}
			// 3.2 (as above)
			else
			{
				pCmdUI->Enable(false);
				cdstring title;
				title.FromResource(IDS_LOGOUT);
				pCmdUI->SetText(title);
			}
		}
	}
	else
	{
		pCmdUI->Enable(false);
		pCmdUI->SetCheck(false);
		cdstring title;
		title.FromResource(IDS_LOGIN);
		pCmdUI->SetText(title);
	}
}

void CCalendarStoreTable::OnUpdateRefreshList(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsSingleSelection());
}

void CCalendarStoreTable::OnUpdateNewWebCalendar(CCmdUI* pCmdUI)
{
	// Always enabled if admin allows it
	pCmdUI->Enable(!CAdminLock::sAdminLock.mNoLocalCalendars &&
					((calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol() == NULL) ||
					!calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol()->IsDisconnected()));
}

void CCalendarStoreTable::OnUpdateRefreshWebCalendar(CCmdUI* pCmdUI)
{
	// Always enabled if admin allows it
	pCmdUI->Enable(!CAdminLock::sAdminLock.mNoLocalCalendars &&
					(calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol() != NULL) &&
					!calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol()->IsDisconnected() &&
					TestSelectionAnd((TestSelectionPP) &CCalendarStoreTable::TestSelectionWebCalendar));
}

void CCalendarStoreTable::OnUpdateUploadWebCalendar(CCmdUI* pCmdUI)
{
	// Always enabled if admin allows it
	pCmdUI->Enable(!CAdminLock::sAdminLock.mNoLocalCalendars &&
					(calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol() != NULL) &&
					!calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol()->IsDisconnected() &&
					TestSelectionAnd((TestSelectionPP) &CCalendarStoreTable::TestSelectionUploadWebCalendar));
}

void CCalendarStoreTable::DoChangeColour(TableIndexT row)
{
	// Must be a valid active calendar
	calstore::CCalendarStoreNode* node = GetCellNode(row);

	if (node->IsSubscribed())
	{
		uint32_t colour = calstore::CCalendarStoreManager::sCalendarStoreManager->GetCalendarColour(node);
		JColorIndex cindex;
		GetColormap()->JColormap::AllocateStaticColor(CCalendarUtils::GetRGBColor(colour), &cindex);

		mColourChooser = new JXChooseColorDialog(GetWindow()->GetDirector(), cindex);
		mColourChosen = false;

		ListenTo(mColourChooser);
		mColourChooser->BeginDialog();

		// display the inactive cursor in all the other windows

		JXApplication* app = JXGetApplication();
		app->DisplayInactiveCursor();

		// block with event loop running until we get a response

		JXWindow* window = mColourChooser->GetWindow();
		while (mColourChooser)
			app->HandleOneEventForWindow(window);
			
		if (mColourChosen)
		{
			calstore::CCalendarStoreManager::sCalendarStoreManager->ChangeNodeColour(node, CCalendarUtils::GetCalColor(mColourChoose));
			RefreshRow(row);
		}
	}
}

// Import selected calendars
void CCalendarStoreTable::DoImportCalendar(calstore::CCalendarStoreNode* node, bool merge)
{
	// Must have valid node
	if (node == NULL)
		return;
	
	// Pick file to import from
	JString fname;
	if (JXGetChooseSaveFile()->ChooseFile("Calendar to Import:", NULL, &fname))
	{
		// Get full path and create stream
		cdstring fpath(fname);
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
	cdstring name = node->GetDisplayName();
	name += ".ics";
	bool replacing;
	JString fname(name);
	if (JXGetChooseSaveFile()->SaveFile("Export Calendar to:", NULL, fname, &fname))
	{
		try
		{
			// Create file object
			{
				cdofstream touchit(fname);
			}

			// Check it exists
			if (!JFileWritable(fname))
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
				
			// Get full path and create stream
			cdstring fpath(fname);
			cdofstream fout(fpath, ios_base::in | ios_base::binary | ios_base::trunc);
			if (fout.fail())
				return false;
			
			// May need to activate if not already
			calstore::CCalendarStoreManager::StNodeActivate _activate(node);
			
			// Write calendar to file
			iCal::CICalendar* cal = node->GetCalendar();
			if (cal != NULL)
				cal->Generate(fout);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Inform user
			//CErrorHandler::PutStopAlertRsrc("Alerts::Adbk::ExportError");

			// Throw up
			CLOG_LOGRETHROW;
			throw;
		}
		
		return true;
	}
	
	return false;
}

#pragma mark ____________________________Drag & Drop

// Prevent drag if improper selection
bool CCalendarStoreTable::DoDrag(const JPoint& pt,
							const JXButtonStates& buttonStates,
							const JXKeyModifiers& modifiers)
{
	return ValidDragSelection() && CHierarchyTableDrag::DoDrag(pt, buttonStates, modifiers);
}

//return the atom for the given selection
bool CCalendarStoreTable::GetSelectionAtom(CFlavorsList& atoms)
{
	// Get the relevant data
	TableIndexT row = GetFirstSelectedRow();
	calstore::CCalendarStoreNode* node = GetCellNode(row);

	if (node->IsProtocol())
		// Dragging server to another location
		atoms.push_back(CMulberryApp::sFlavorCalServer);
	else
		// Dragging calendar
		atoms.push_back(CMulberryApp::sFlavorCalendar);

	//if we get out of the switch then we're ok
	mDragFlavor = atoms.front();
	return true;
}

bool CCalendarStoreTable::RenderSelectionData(CMulSelectionData* seldata, Atom type)
{
	bool rendered = false;

	// gcc is stupid and does not know which ItemIsAcceptable to use
	// here even though the argument types are clearly different
	if (CTableDragAndDrop::ItemIsAcceptable(type))
	{
		ulvector rows;
		GetSelectedRows(rows);
		int count = rows.size();

		// Allocate global memory for the text
		unsigned long dataLength = count * sizeof(void*) + sizeof(int);
		unsigned char* data = new unsigned char[dataLength];
		if (data)
		{
			int* i = reinterpret_cast<int*>(data);
			*i = count;
			i++;
			void** vdata = reinterpret_cast<void**>(i);
			for(ulvector::const_iterator iter = rows.begin(); iter != rows.end(); iter++)
			{
				calstore::CCalendarStoreNode* node = GetCellNode(*iter);
				if (node->IsProtocol())
				{
					*vdata = node->GetProtocol();
					vdata++;
				}
				else if (!node->IsInbox() && !node->IsOutbox())
				{
					*vdata = node;
					vdata++;
				}
			}
			seldata->SetData(type, data, dataLength);
			rendered = true;
		}
	}

	return rendered;
}

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
			got_calendar = 2;
		else
			got_calendar = 1;

		// Can only have one type
		if (got_server + got_calendar > 1)
			return false;
	}

	return true;
}

// Test drag insert cursor
bool CCalendarStoreTable::IsDropCell(JArray<Atom>& typeList, const STableCell& cell)
{
	if (IsValidCell(cell))
	{
		// Get flavor for this item
		Atom theFlavor;
		FlavorsMatch(typeList, mDropFlavors,&theFlavor);

		// Get drop cell type
		calstore::CCalendarStoreNode* node = GetCellNode(cell.row);

		if (theFlavor == CMulberryApp::sFlavorCalendarItem)
		{
			// Drop into valid calendars only
			return node->IsViewableCalendar();
		}
		else if (theFlavor == CMulberryApp::sFlavorCalServer)
			// Servers always moved
			return false;
		else if (theFlavor == CMulberryApp::sFlavorCalendar)
		{
			if (node->IsProtocol() || node->IsInbox() || node->IsOutbox())
				return false;
			else
				// Allow drop into any calendar (directory = move, mbox = copy)
				return true;
		}
		else
			// Anything else is an error!
			return false;
	}
	else
		return false;
}

// Test drop at cell
bool CCalendarStoreTable::IsDropAtCell(JArray<Atom>& typeList, STableCell& aCell)
{
	// Adjust for end of table
	int adjust = 0;
	if (aCell.row > mRows)
		adjust = 1;

	if (IsValidCell(aCell) || adjust)
	{
		// Get flavor for this item
		Atom theFlavor;
		FlavorsMatch(typeList, mDropFlavors,&theFlavor);

		// Get drop cell type
		calstore::CCalendarStoreNode* node = GetCellNode(aCell.row - adjust);
		TableIndexT woRow = GetWideOpenIndex(aCell.row);

		if (theFlavor == CMulberryApp::sFlavorCalendarItem)
		{
			// Always drop into, never at
			return false;
		}
		else if (theFlavor == CMulberryApp::sFlavorCalServer)
		{
			// Must be same window
			if (sTableDragSource != this)
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
		}
		else if (theFlavor == CMulberryApp::sFlavorCalendar)
		{
			return true;
		}
		else
			// Anything else is an error!
			return false;
	}
	else
		return false;
}

// Can cell expand for drop
bool CCalendarStoreTable::CanDropExpand(const JArray<Atom>& typeList, TableIndexT row)
{
	// Get flavor for this item
	Atom theFlavor;
	FlavorsMatch(typeList, mDropFlavors,&theFlavor);

	if ((theFlavor == CMulberryApp::sFlavorCalServer) ||
		(theFlavor == CMulberryApp::sFlavorCalendar) ||
		(theFlavor == CMulberryApp::sFlavorCalendarItem))
		return CHierarchyTableDrag::CanDropExpand(typeList, row);
	else
		// Anything else cannot
		return false;
}

// Determine effect
Atom CCalendarStoreTable::GetDNDAction(const JXContainer* target,
									 const JXButtonStates& buttonStates,
									 const JXKeyModifiers& modifiers)
{
	// Get flavor for this item
	unsigned int theFlavor = mDragFlavor;
	JXDNDManager* dndMgr = GetDNDManager();
	Atom copy = dndMgr->GetDNDActionCopyXAtom();
	Atom move = dndMgr->GetDNDActionMoveXAtom();

	if (theFlavor == CMulberryApp::sFlavorCalendarItem)
	{
		bool option_key = modifiers.control();

		return option_key ? copy : move;
	}
	else if (theFlavor == CMulberryApp::sFlavorCalServer)
		// Server always moved
		return move;

	else if (theFlavor == CMulberryApp::sFlavorCalendar)
	{
		// Mailboxes & references moved if drop at or no drop cell
		if (mLastDropCursor.row || !mLastDropCell.row)
			return move;
		
		// Must check cell dropped into to see if directory
		// Get drop cell type
		calstore::CCalendarStoreNode* node = GetCellNode(mLastDropCell.row);
		if (node->IsDirectory())
			return move;
		else
			return copy;
	}

	else
		// Anything else is an error!
		return move;
}

// Drop data into cell
bool CCalendarStoreTable::DropData(Atom theFlavor, unsigned char* drag_data, unsigned long data_size)
{
	if (theFlavor == CMulberryApp::sFlavorCalServer)
	{
		// Make this window show this server if not manager
		//new CServerDragOpenServerTask(this, *(CMboxProtocol**) drag_data);
		return true;
	}
	
	return false;
}

// Drop data into cell
bool CCalendarStoreTable::DropDataIntoCell(Atom theFlavor, unsigned char* drag_data, unsigned long data_size, const STableCell& cell)
{
	if (theFlavor == CMulberryApp::sFlavorCalendarItem)
	{
		return false; // For now
	}
	else if (theFlavor == CMulberryApp::sFlavorCalendar)
	{
		// Get drop cell type
		calstore::CCalendarStoreNode* node = GetCellNode(cell.row);
		if (node->IsProtocol() || node->IsInbox() || node->IsOutbox())
			return false;

		// Ask user before doing task
		if (CErrorHandler::PutCautionAlertRsrcStr(true, node->IsDirectory() ? "CCalendarStoreTable::ReallyMoveCalendar" : "CCalendarStoreTable::ReallyCopyCalendar", node->GetName()) == CErrorHandler::Cancel)
			return false;

		int count = *(int*) drag_data;
		drag_data += sizeof(int);
		for(int i = 0; i < count; i++)
		{
			calstore::CCalendarStoreNode* drag = ((calstore::CCalendarStoreNode**) drag_data)[i];
			if (drag->IsProtocol())
				continue;

			if (node->IsDirectory())
			{
				// Move mailbox
				drag->MoveCalendar(node, false);
			}
			else
			{
				// Do calendar -> calendar copy
				drag->CopyCalendar(node);
			}
		}
		return true;
	}
	
	return false;
}

// Drop data at cell
bool CCalendarStoreTable::DropDataAtCell(Atom theFlavor, unsigned char* drag_data, unsigned long data_size, const STableCell& before_cell)
{
	// Adjust for end of table
	int adjust = 0;
	if (before_cell.row > mRows)
		adjust = 1;

	// Get drop cell type
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(before_cell.row + TABLE_ROW_ADJUST - adjust);
	calstore::CCalendarStoreNode* node = GetCellNode(before_cell.row - adjust);

	if (theFlavor == CMulberryApp::sFlavorCalServer)
	{
		unsigned long new_index = 0;
		if (adjust)
			new_index = calstore::CCalendarStoreManager::sCalendarStoreManager->GetProtocolCount();
		else
		{
			calstore::CCalendarProtocol* above = node->GetProtocol();
			if (node->IsProtocol())
				new_index = calstore::CCalendarStoreManager::sCalendarStoreManager->GetProtocolIndex(above);
			else
				new_index = calstore::CCalendarStoreManager::sCalendarStoreManager->GetProtocolIndex(above) + 1;
		}

		int count = *(int*) drag_data;
		drag_data += sizeof(int);
		for(int i = 0; i < count; i++)
		{
			calstore::CCalendarProtocol* drag = ((calstore::CCalendarProtocol**) drag_data)[i];

			long old_index = calstore::CCalendarStoreManager::sCalendarStoreManager->GetProtocolIndex(drag);

			// Only if different
			if (old_index != new_index)
				calstore::CCalendarStoreManager::sCalendarStoreManager->MoveProtocol(old_index, new_index);

			// Must adjust if dragging above
			if (new_index < old_index)
				new_index++;
		}

		return true;
	}

	else if (theFlavor == CMulberryApp::sFlavorCalendar)
	{
		// Ask user before doing task
		if (CErrorHandler::PutCautionAlertRsrcStr(true, "CCalendarStoreTable::ReallyMoveCalendar", (node->GetParent() ? node->GetParent() : node)->GetName()) == CErrorHandler::Cancel)
			return false;

		int count = *(int*) drag_data;
		drag_data += sizeof(int);
		for(int i = 0; i < count; i++)
		{
			calstore::CCalendarStoreNode* drag = ((calstore::CCalendarStoreNode**) drag_data)[i];
			if (drag->IsProtocol())
				continue;
			
			// Move calendar into directory
			drag->MoveCalendar(node->GetParent() ? node->GetParent() : node, !node->GetParent());
		}

		return true;
	}

	return false;
}
