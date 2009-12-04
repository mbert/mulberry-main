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
#include "CDrawUtils.h"
#include "CDragTasks.h"
#include "CErrorHandler.h"
#include "CFontCache.h"
#include "CIconLoader.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CTableRowGeometry.h"
#include "CTableRowSelector.h"
#include "CXStringResources.h"

#include "cdfstream.h"

#include "CCalendarStoreNode.h"
#include "CCalendarStoreManager.h"

#include "CICalendar.h"
#include "CICalendarSync.h"

#include <WIN_LTableArrayStorage.h>

IMPLEMENT_DYNCREATE(CCalendarStoreTable, CHierarchyTableDrag)

BEGIN_MESSAGE_MAP(CCalendarStoreTable, CHierarchyTableDrag)

	ON_UPDATE_COMMAND_UI(IDM_FILE_IMPORT, OnUpdateAlways)
	ON_COMMAND(IDM_FILE_IMPORT, OnImport)

	ON_UPDATE_COMMAND_UI(IDM_FILE_EXPORT, OnUpdateSelectionCalendar)
	ON_COMMAND(IDM_FILE_EXPORT, OnExport)

	ON_UPDATE_COMMAND_UI(IDM_EDIT_PROPERTIES, OnUpdateSelection)
	ON_COMMAND(IDM_EDIT_PROPERTIES, OnEditProperties)

	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_NEW, OnUpdateAlways)
	ON_COMMAND(IDM_CALENDAR_NEW, OnNewCalendar)

	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_RENAME, OnUpdateSelectionCalendar)
	ON_COMMAND(IDM_CALENDAR_RENAME, OnRenameCalendar)

	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_DELETE, OnUpdateSelectionCalendar)
	ON_COMMAND(IDM_CALENDAR_DELETE, OnDeleteCalendar)

	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_REFRESH, OnUpdateRefreshList)
	ON_COMMAND(IDM_CALENDAR_REFRESH, OnRefreshList)

	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_FREEBUSY, OnUpdateSelectionCalendar)
	ON_COMMAND(IDM_CALENDAR_FREEBUSY, OnFreeBusyCalendar)

	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_SENDEMAIL, OnUpdateSelectionCalendar)
	ON_COMMAND(IDM_CALENDAR_SENDEMAIL, OnSendCalendar)

	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_NEWWEBCAL, OnUpdateNewWebCalendar)
	ON_COMMAND(IDM_CALENDAR_NEWWEBCAL, OnNewWebCalendar)

	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_REFRESHWEBCAL, OnUpdateRefreshWebCalendar)
	ON_COMMAND(IDM_CALENDAR_REFRESHWEBCAL, OnRefreshWebCalendar)

	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_UPLOADWEBCAL, OnUpdateUploadWebCalendar)
	ON_COMMAND(IDM_CALENDAR_UPLOADWEBCAL, OnUploadWebCalendar)

	ON_WM_CREATE()

	// Toolbar
	ON_UPDATE_COMMAND_UI(IDC_TOOLBARSERVERLOGINBTN, OnUpdateLogin)
	ON_COMMAND(IDC_TOOLBARSERVERLOGINBTN, OnLogin)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARDETAILSBTN, OnUpdateSelection)
	ON_COMMAND(IDC_TOOLBARDETAILSBTN, OnEditProperties)

	ON_COMMAND(IDC_TOOLBAR_CALENDAR_NEWCALENDAR, OnNewCalendar)

END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
//	CCalendarStoreTable														  [public]
/**
	Default constructor */

CCalendarStoreTable::CCalendarStoreTable()
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
	mTableSelector = new CTableRowSelector(this);

	SetRowSelect(true);
	
	// Storage
	mTableStorage = new LTableArrayStorage(this, sizeof(calstore::CCalendarStoreNode*));
	
	mTableView = NULL;
	mListChanging = false;
}

int CCalendarStoreTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CHierarchyTableDrag::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Set Drag & Drop info

	// Can drag servers, calendars
	AddDragFlavor(CMulberryApp::sFlavorCalServer);
	AddDragFlavor(CMulberryApp::sFlavorCalendar);

	// Can drop servers, calendars & calendar items
	AddDropFlavor(CMulberryApp::sFlavorCalServer);
	AddDropFlavor(CMulberryApp::sFlavorCalendar);
	AddDropFlavor(CMulberryApp::sFlavorCalendarItem);

	SetReadOnly(false);
	SetDropCell(true);
	SetDropCursor(true);
	SetAllowDrag(true);
	SetAllowMove(true);
	SetHandleMove(false);		// The calendar move/copy operations handles deleting original item
	SetSelfDrag(true);

	CWnd* parent = GetParent();
	while(parent && !dynamic_cast<CCalendarStoreView*>(parent))
		parent = parent->GetParent();
	mTableView = dynamic_cast<CCalendarStoreView*>(parent);
	SetSuperCommander(mTableView);

	return 0;
}

// Keep titles in sync
void CCalendarStoreTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh)
{
	// Find titles in owner chain
	mTableView->GetBaseTitles()->ScrollImageBy(inLeftDelta, 0, inRefresh);

	CHierarchyTableDrag::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
}

// Handle key down
bool CCalendarStoreTable::HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Look for preview/full view based on key stroke

	// Determine whether preview is triggered
	CKeyModifiers mods(0);
	const CUserAction& preview = mTableView->GetPreviewAction();
	if ((preview.GetKey() == nChar) &&
		(preview.GetKeyModifiers() == mods))
	{
		DoPreview();
		return true;
	}

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if ((fullview.GetKey() == nChar) &&
		(fullview.GetKeyModifiers() == mods))
	{
		DoFullView();
		return true;
	}

	return CHierarchyTableDrag::HandleKeyDown(nChar, nRepCnt, nFlags);
}

void CCalendarStoreTable::LClickCell(const STableCell& inCell, UINT nFlags)
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
		if (!node->IsProtocol() && !node->IsDirectory())
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
		CHierarchyTableDrag::LClickCell(inCell, nFlags);
		DoSingleClick(inCell.row, CKeyModifiers(0));
		break;
	}
}

// Double-clicked item
void CCalendarStoreTable::LDblClickCell(const STableCell& inCell, UINT nFlags)
{
	DoDoubleClick(inCell.row, CKeyModifiers(0));
}

void CCalendarStoreTable::DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect)
{
	// Not if changing
	if (mListChanging)
		return;

	StDCState		save(pDC);

	TableIndexT	woRow = GetWideOpenIndex(inCell.row + TABLE_ROW_ADJUST);

	calstore::CCalendarStoreNode* node = GetCellNode(inCell.row);

	// Erase to ensure drag hightlight is overwritten
	if (UsesBackgroundColor(inCell.row))
	{
		pDC->SetBkColor(GetBackgroundColor(inCell.row));

	   	// Erase the entire area. Using ExtTextOut is a neat alternative to FillRect and quicker, too!
	   	CRect cellRect = inLocalRect;
	   	cellRect.bottom--;
		pDC->ExtTextOut(cellRect.left, cellRect.top, ETO_OPAQUE, cellRect, _T(""), 0, nil);
	}

	SColumnInfo col_info = mTableView->GetColumnInfo()[inCell.col - 1];

	switch(col_info.column_type)
	{

	case eCalendarStoreColumnName:
	{
		// Always cache column number
		mHierarchyCol = inCell.col;

		// Get suitable icon
		UINT iconID = GetPlotIcon(node, GetCellCalendarProtocol(inCell.row));

		// Get name of item
		cdstring theTxt = node->GetShortName();

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
		SetTextStyle(pDC, node, GetCellCalendarProtocol(inCell.row), strike);
		int text_start = DrawHierarchyRow(pDC, inCell.row, inLocalRect, theTxt, iconID);
		if (strike)
		{
			CPen temp(PS_SOLID, 1, pDC->GetTextColor());
			CPen* old_pen = (CPen*) pDC->SelectObject(&temp);
			pDC->MoveTo(text_start, (inLocalRect.top + inLocalRect.bottom)/2);
			pDC->LineTo(inLocalRect.right, (inLocalRect.top + inLocalRect.bottom)/2);
			pDC->SelectObject(old_pen);
		}
		break;
	}

	case eCalendarStoreColumnSubscribe:
		// Do status flag
		if (!node->IsProtocol() && !node->IsDirectory())
		{
			if (node->IsCached())
			{
				if (node->IsSubscribed())
					CIconLoader::DrawIcon(pDC, inLocalRect.left, inLocalRect.top + mIconOrigin, IDI_DIAMONDTICKED, 16);
				else
					CIconLoader::DrawIcon(pDC, inLocalRect.left, inLocalRect.top + mIconOrigin, IDI_DIAMOND, 16);
			}
			else
				CIconLoader::DrawIcon(pDC, inLocalRect.left, inLocalRect.top + mIconOrigin, IDI_DIAMONDDISABLE, 16);
		}
		break;

	case eCalendarStoreColumnColour:
		if (node->IsSubscribed())
		{
			uint32_t colour = calstore::CCalendarStoreManager::sCalendarStoreManager->GetCalendarColour(node);
			if (colour != 0)
			{
				// Black outline
				CRect rect = inLocalRect;
				rect.DeflateRect(2, 2);
				pDC->FrameRect(&rect, &CDrawUtils::sBlackBrush);

				rect.DeflateRect(1, 1);
				CBrush fill;
				fill.CreateSolidBrush(CCalendarUtils::GetWinColor(colour));
				pDC->FillRect(&rect, &fill);
			}
		}
		break;

	default:
		break;
	}
}

// Get appropriate icon id
int CCalendarStoreTable::GetPlotIcon(const calstore::CCalendarStoreNode* node, calstore::CCalendarProtocol* proto)
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
	else if (node->IsDirectory())
	{
		return IDI_SERVERDIR;
	}
	else
	{
		return node->IsCached() ? IDI_CALENDAR : IDI_CALENDARUNCACHED;
	}
}

// Get appropriate text style
void CCalendarStoreTable::SetTextStyle(CDC* pDC, const calstore::CCalendarStoreNode* node, calstore::CCalendarProtocol* proto, bool& strike)
{
	strike = false;

	// Select appropriate color and style of text
	//if (UEnvironment::HasFeature(env_SupportsColor))
	{
		bool color_set = false;
		RGBColor text_color;
		bool style_set = false;
		short text_style = normal;

		if (node->IsProtocol())
		{
			if (!proto || proto->IsLoggedOn())
			{
				text_color = CPreferences::sPrefs->mServerOpenStyle.GetValue().color;
				text_style = text_style | CPreferences::sPrefs->mServerOpenStyle.GetValue().style & 0x007F;
				strike = strike || ((CPreferences::sPrefs->mServerOpenStyle.GetValue().style & 0x0080) != 0);
			}
			else
			{
				text_color = CPreferences::sPrefs->mServerClosedStyle.GetValue().color;
				text_style = text_style | CPreferences::sPrefs->mServerClosedStyle.GetValue().style & 0x007F;
				strike =  strike || ((CPreferences::sPrefs->mServerClosedStyle.GetValue().style & 0x0080) != 0);
			}
			
			color_set = true;
			style_set = true;
		}
		else
		{
			text_color = CPreferences::sPrefs->mMboxClosedStyle.GetValue().color;
			text_style = text_style | CPreferences::sPrefs->mMboxClosedStyle.GetValue().style & 0x007F;
			strike =  strike || ((CPreferences::sPrefs->mMboxClosedStyle.GetValue().style & 0x0080) != 0);
			
			color_set = true;
			style_set = true;
		}

		if (color_set)
			pDC->SetTextColor(text_color);

		// Set appropriate font
		if (style_set)
		{
			switch(text_style)
			{
			case normal:
			default:
				//pDC->SelectObject(CMulberryApp::sAppListFont);
				break;
			case bold:
				pDC->SelectObject(CFontCache::GetListFontBold());
				break;
			case italic:
				pDC->SelectObject(CFontCache::GetListFontItalic());
				break;
			case bold + italic:
				pDC->SelectObject(CFontCache::GetListFontBoldItalic());
				break;
			case underline:
				pDC->SelectObject(CFontCache::GetListFontUnderline());
				break;
			case bold + underline:
				pDC->SelectObject(CFontCache::GetListFontBoldUnderline());
				break;
			case italic + underline:
				pDC->SelectObject(CFontCache::GetListFontItalicUnderline());
				break;
			case bold + italic + underline:
				pDC->SelectObject(CFontCache::GetListFontBoldItalicUnderline());
				break;
			}
		}
	}
}

bool CCalendarStoreTable::UsesBackgroundColor(TableIndexT row) const
{
	TableIndexT	woRow = GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	return mCollapsableTree->GetNestingLevel(woRow) == 0;
}

COLORREF CCalendarStoreTable::GetBackgroundColor(TableIndexT row) const
{
	return CPreferences::sPrefs->mServerBkgndStyle.GetValue().color;
}

void CCalendarStoreTable::OnUpdateSelectionCalendar(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TestSelectionAnd((TestSelectionPP) &CCalendarStoreTable::TestSelectionCalendar));
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
		pCmdUI->SetText(title.win_str());

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
			pCmdUI->SetText(title.win_str());
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
				pCmdUI->SetText(title.win_str());
			}
			// 2.2 (as above)
			else
			{
				pCmdUI->Enable(false);
				cdstring title;
				title.FromResource(IDS_LOGOUT);
				pCmdUI->SetText(title.win_str());
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
				pCmdUI->SetText(title.win_str());
			}
			// 3.2 (as above)
			else
			{
				pCmdUI->Enable(false);
				cdstring title;
				title.FromResource(IDS_LOGOUT);
				pCmdUI->SetText(title.win_str());
			}
		}
	}
	else
	{
		pCmdUI->Enable(false);
		pCmdUI->SetCheck(false);
		cdstring title;
		title.FromResource(IDS_LOGIN);
		pCmdUI->SetText(title.win_str());
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

		RGBColor	inColor = CCalendarUtils::GetWinColor(colour);

		CColorDialog dlg(0, 0, CSDIFrame::GetAppTopWindow());
		dlg.m_cc.rgbResult = inColor;
		dlg.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT;

		if (dlg.DoModal() == IDOK)
		{
			calstore::CCalendarStoreManager::sCalendarStoreManager->ChangeNodeColour(node, CCalendarUtils::GetCalColor(dlg.GetColor()));
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
	
	// Do standard open dialog
	// prompt the user
	CString filter = _T("Anyfile (*.*) | *.*||");
	CFileDialog dlgFile(true, NULL, NULL, OFN_FILEMUSTEXIST, filter, CSDIFrame::GetAppTopWindow());

	// Check for good reply
	if (dlgFile.DoModal() == IDOK)
	{
		// Get full path and create stream
		cdstring fpath(dlgFile.GetPathName());
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
	CString saveAsName = cdustring(node->GetShortName());
	CString filter = _T("Exported Calendar (*.ics)|*.ics||");
	CFileDialog dlgFile(false, _T(".ics"), saveAsName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, CSDIFrame::GetAppTopWindow());

	// Check for good reply
	if (dlgFile.DoModal() == IDOK)
	{
		// Get full path and create stream
		cdstring pname(dlgFile.GetPathName());
		cdofstream fout(pname, ios_base::in | ios_base::binary | ios_base::trunc);
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

void CCalendarStoreTable::OnEditProperties()
{
	if (TestSelectionAnd((TestSelectionPP) &CCalendarStoreTable::TestSelectionServer))
		OnServerProperties();
	else if (TestSelectionAnd((TestSelectionPP) &CCalendarStoreTable::TestSelectionCalendar))
		OnCalendarProperties();
}


#pragma mark ____________________________Drag & Drop

// Prevent drag if improper selection
BOOL CCalendarStoreTable::DoDrag(TableIndexT row)
{
	return ValidDragSelection() && CHierarchyTableDrag::DoDrag(row);
}

// Only add CCalendarStoreTable item flavors to drag
void CCalendarStoreTable::SetDragFlavors(TableIndexT row)
{
	// Get the relevant data
	calstore::CCalendarStoreNode* node = GetCellNode(row);

	if (node->IsProtocol())
		// Dragging server to another location
		mDrag.DelayRenderData(CMulberryApp::sFlavorCalServer);
	else
		// Dragging calendar
		mDrag.DelayRenderData(CMulberryApp::sFlavorCalendar);
}

BOOL CCalendarStoreTable::OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal)
{
	BOOL rendered = false;

	if  ((lpFormatEtc->cfFormat == CMulberryApp::sFlavorCalServer) ||
		 (lpFormatEtc->cfFormat == CMulberryApp::sFlavorCalendar))
	{
		ulvector rows;
		GetSelectedRows(rows);
		int count = rows.size();

		// Allocate global memory for the text if not already
		if (!*phGlobal)
			*phGlobal = ::GlobalAlloc(GMEM_DDESHARE, count * sizeof(void*) + sizeof(int));

		if (*phGlobal)
		{
			// Copy to global after lock
			void** pGrp = (void**) ::GlobalLock(*phGlobal);
			*((int*) pGrp) = count;
			pGrp += sizeof(int);
			for(ulvector::const_iterator iter = rows.begin(); iter != rows.end(); iter++)
			{
				calstore::CCalendarStoreNode* node = GetCellNode(*iter);
				if (node->IsProtocol())
					*pGrp++ = node->GetProtocol();
				else
					*pGrp++ = node;
			}
			::GlobalUnlock(*phGlobal);
			
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
		calstore::CCalendarStoreNode* node = GetCellNode(selCell.row);
		if (node->IsProtocol())
		{
			// Cannot drag either the local or web calendars protocols
			if ((node->GetProtocol()->GetAccountType() == CINETAccount::eLocalCalendar) || (node->GetProtocol()->GetAccountType() == CCalendarAccount::eHTTPCalendar))
				return false;
			
			got_server = 1;
		}
		else
			got_calendar = 1;

		// Can only have one type
		if (got_server + got_calendar > 1)
			return false;
	}

	return true;
}

// Test drag insert cursor
bool CCalendarStoreTable::IsDropCell(COleDataObject* pDataObject, const STableCell& theCell)
{
	if (IsValidCell(theCell))
	{
		// Get flavor for this item
		unsigned int theFlavor = GetBestFlavor(pDataObject);

		// Get drop cell type
		calstore::CCalendarStoreNode* node = GetCellNode(theCell.row);

		if (theFlavor == CMulberryApp::sFlavorCalendarItem)
		{
			// Drop into valid calendars only
			return !node->IsProtocol() && !node->IsDirectory();
		}
		else if (theFlavor == CMulberryApp::sFlavorCalServer)
			// Servers always moved
			return false;
		else if (theFlavor == CMulberryApp::sFlavorCalendar)
		{
			if (node->IsProtocol())
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
bool CCalendarStoreTable::IsDropAtCell(COleDataObject* pDataObject, STableCell& aCell)
{
	// Get flavor for this item
	unsigned int theFlavor = GetBestFlavor(pDataObject);

	// Adjust for end of table
	int adjust = 0;
	if (aCell.row > mRows)
		adjust = 1;

	if (IsValidCell(aCell) || adjust)
	{
		// Get drop cell type
		calstore::CCalendarStoreNode* node = GetCellNode(aCell.row - adjust);
		TableIndexT woRow = GetWideOpenIndex(aCell.row + TABLE_ROW_ADJUST);

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
bool CCalendarStoreTable::CanDropExpand(COleDataObject* pDataObject, TableIndexT woRow)
{
	// Get flavor for this item
	unsigned int theFlavor = GetBestFlavor(pDataObject);

	if ((theFlavor == CMulberryApp::sFlavorCalendar) ||
		(theFlavor == CMulberryApp::sFlavorCalendarItem))
		return CHierarchyTableDrag::CanDropExpand(pDataObject, woRow);
	else
		// Anything else cannot
		return false;
}

// Adjust cursor over drop area
DROPEFFECT CCalendarStoreTable::GetDropEffect(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	// Get flavor for this item
	unsigned int theFlavor = GetBestFlavor(pDataObject);

	// Toggle based on modifiers
	bool option_key = ((dwKeyState & MK_ALT) == MK_ALT);

	if (theFlavor == CMulberryApp::sFlavorCalendarItem)
	{
		return option_key ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
	}
	else if (theFlavor == CMulberryApp::sFlavorCalServer)
		// Server always moved
		return DROPEFFECT_MOVE;
	else if (theFlavor == CMulberryApp::sFlavorCalendar)
	{
		// Calendars moved if drop at or no drop cell
		if (mLastDropCursor.row || !mLastDropCell.row)
			return DROPEFFECT_MOVE;

		// Must check cell dropped into to see if directory
		// Get drop cell type
		calstore::CCalendarStoreNode* node = GetCellNode(mLastDropCell.row);
		if (node->IsDirectory())
			return DROPEFFECT_MOVE;
		else
			return DROPEFFECT_COPY;
	}
	else
		// Anything else is an error!
		return DROPEFFECT_MOVE;
}

// Drop data into cell
bool CCalendarStoreTable::DropData(unsigned int theFlavor, char* drag_data, unsigned long data_size)
{
	return false;
}

// Drop data into cell
bool CCalendarStoreTable::DropDataIntoCell(unsigned int theFlavor, char* drag_data,
											unsigned long data_size, const STableCell& theCell)
{
	if (theFlavor == CMulberryApp::sFlavorCalendarItem)
	{
		return false; // For now
	}
	else if (theFlavor == CMulberryApp::sFlavorCalendar)
	{
		// Get drop cell type
		calstore::CCalendarStoreNode* node = GetCellNode(theCell.row);

		// Ask user before doing task
		if (CErrorHandler::PutCautionAlertRsrcStr(true, node->IsDirectory() ? "CCalendarStoreTable::ReallyMoveCalendar" : "CCalendarStoreTable::ReallyCopyCalendar", node->GetName()) == CErrorHandler::Cancel)
			return false;

		int count = *((int*) drag_data);
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
bool CCalendarStoreTable::DropDataAtCell(unsigned int theFlavor, char* drag_data,
											unsigned long data_size, const STableCell& beforeCell)
{
	// Adjust for end of table
	int adjust = 0;
	if (beforeCell.row > mRows)
		adjust = 1;

	// Get drop cell type
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(beforeCell.row + TABLE_ROW_ADJUST - adjust);
	calstore::CCalendarStoreNode* node = GetCellNode(beforeCell.row - adjust);

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

		int count = *((int*) drag_data);
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

		int count = *((int*) drag_data);
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
