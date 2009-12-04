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


// Source for CServerBrowse class

#include "CServerBrowse.h"

#include "CCopyToMenu.h"
#include "CDrawUtils.h"
#include "CFontCache.h"
#include "CIconLoader.h"
#include "CINETCommon.h"
#include "CMailAccountManager.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMboxRef.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CNodeVectorTree.h"
#include "CPreferences.h"
#include "CTableRowSelector.h"
#include "CTableRowGeometry.h"

#include <stdio.h>
#include <string.h>

const short cTextIndent = 26;

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

IMPLEMENT_DYNCREATE(CServerBrowse, CHierarchyTableDrag)

BEGIN_MESSAGE_MAP(CServerBrowse, CHierarchyTableDrag)
	ON_WM_CREATE()
END_MESSAGE_MAP()

cdstring CServerBrowse::sSubscribedName;			// Text for subscribed row

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CServerBrowse::CServerBrowse()
{
	InitServerBrowse();
}

// Default destructor
CServerBrowse::~CServerBrowse()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CServerBrowse::InitServerBrowse(void)
{
	mTableGeometry = new CTableRowGeometry(this, 128, 18);
	mTableSelector = new CTableRowSelector(this);

	SetRowSelect(true);

	mServer = NULL;
	mManager = false;
	mSingle = true;
	mShowFavourites = true;
	mRecordExpansion = true;

	mListChanging = false;
	
	mHierarchyCol = 0;
	
	// Do not listen until reset completed
	Stop_Listening();
	
	// Allow selection via keyboard
	SetKeySelection(true);

	// Load static text
	if (sSubscribedName.empty())
		sSubscribedName.FromResource("UI::Server::Subscribed");
}

int CServerBrowse::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CHierarchyTableDrag::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Set read only status of Drag and Drop
	SetReadOnly(false);
	
	// Tooltips are active
	EnableToolTips();

	return 0;
}

// Update status items like toolbar
void CServerBrowse::UpdateState()
{
	// Nothing to do in this base class
}

#pragma mark ____________________________Draw & Click

// Click in the cell
void CServerBrowse::LClickCell(const STableCell& inCell, UINT nFlags)
{
	int woRow = mCollapsableTree->GetWideOpenIndex(inCell.row + TABLE_ROW_ADJUST);
	bool handled = false;

	// Determine which heading it is
	SColumnInfo col_info = GetColumnInfo(inCell.col);
	
	EServerBrowseDataType type = GetCellDataType(woRow);

	CMbox* mbox = NULL;
	bool ref = false;
	bool directory = false;
	switch(type)
	{
	case eServerBrowseMbox:
		mbox = GetCellMbox(woRow);
		directory = mbox->IsDirectory();
		break;
	case eServerBrowseMboxRef:
		ref = true;
		mbox = GetCellMboxRef(woRow)->ResolveMbox(true);
		directory = GetCellMboxRef(woRow)->IsDirectory();
		break;
	}

	switch(col_info.column_type)
	{
	case eServerColumnAutoCheck:
		// Only if real mailbox
		if (mbox && !directory)
		{
			bool set = !mbox->IsAutoCheck();
			if (set)
			{
				const CMailNotification& notify = CPreferences::sPrefs->mMailNotification.GetValue().front();
				unsigned long index = notify.GetFavouriteIndex(CPreferences::sPrefs);
				CMailAccountManager::sMailAccountManager->AddFavouriteItem(static_cast<CMailAccountManager::EFavourite>(index), mbox);
			}
			else
			{
				CMailAccountManager::EFavourite type;
				if (CMailAccountManager::sMailAccountManager->GetFavouriteAutoCheck(mbox, type))
					CMailAccountManager::sMailAccountManager->RemoveFavouriteItem(type, mbox);
			}
			handled = true;
		}
		break;
	case eServerColumnSize:
		if (mbox && !directory)
		{
			// Check if size calculation requested
			if ((mbox->GetSize() == ULONG_MAX) &&
				mbox->GetProtocol()->IsLoggedOn() &&
				(!mbox->IsLocalMbox() || mbox->IsCachedMbox()))
			{
				mbox->CheckSize();
				handled = true;
			}
		}
		break;
	default:;
	}
	
	if (handled)
		RefreshRow(inCell.row);
	else
		// Do inherited
		CHierarchyTableDrag::LClickCell(inCell, nFlags);
}

// Draw the titles
void CServerBrowse::DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect)
{
	// Do not allow interim changes to throw
	if (mListChanging)
		return;

	// Do not draw if manager is missing
	if (!CMailAccountManager::sMailAccountManager)
		return;

	StDCState save(pDC);

	int woRow = mCollapsableTree->GetWideOpenIndex(inCell.row + TABLE_ROW_ADJUST);

	EServerBrowseDataType type = GetCellDataType(woRow);

	CMbox* mbox = NULL;
	void* plot_data = NULL;
	bool directory = false;
	CMboxProtocol* server = NULL;
	switch(type)
	{
	case eServerBrowseMbox:
		mbox = GetCellMbox(woRow);
		if (mbox == NULL)
			return;
		plot_data = mbox;
		directory = mbox->IsDirectory();
		break;
	case eServerBrowseMboxRef:
		plot_data = GetCellMboxRef(woRow);
		if (plot_data == NULL)
			return;
		mbox = GetCellMboxRef(woRow)->ResolveMbox(true);
		directory = GetCellMboxRef(woRow)->IsDirectory();
		break;
	case eServerBrowseServer:
		server = GetCellServer(woRow);
		if (server == NULL)
			return;
		plot_data =	server;
		break;
	}

	// Erase to ensure drag hightlight is overwritten
	if (UsesBackgroundColor(type))
	{
		pDC->SetBkColor(GetBackgroundColor(type));

	   	// Erase the entire area. Using ExtTextOut is a neat alternative to FillRect and quicker, too!
	   	CRect cellRect = inLocalRect;
	   	cellRect.bottom--;
		pDC->ExtTextOut(cellRect.left, cellRect.top, ETO_OPAQUE, cellRect, _T(""), 0, NULL);
	}

	// Determine which heading it is
	SColumnInfo col_info = GetColumnInfo(inCell.col);
	cdstring theTxt;

	switch(col_info.column_type)
	{
	case eServerColumnFlags:
		// Do status flag
		if (mbox && !directory)
		{
			UINT iconID = 0;
			if (mbox->IsOpenSomewhere())
				iconID = IDI_SERVER_MAILBOX_OPEN;
			else
			{
				if (mbox->IsAutoCheck())
					iconID = IDI_SERVER_MAILBOX_CLOSECHECK;
				if (CPreferences::sPrefs->mUnseenNew.GetValue() && CPreferences::sPrefs->mRecentNew.GetValue())
				{
					if ((mbox->GetNumberViewRecent() > 0) && (mbox->GetNumberUnseen() > 0))
						iconID = IDI_SERVER_MAILBOX_CLOSENEW;
				}
				else if (CPreferences::sPrefs->mUnseenNew.GetValue())
				{
					if (mbox->GetNumberUnseen() > 0)
						iconID = IDI_SERVER_MAILBOX_CLOSENEW;
				}
				else if (CPreferences::sPrefs->mRecentNew.GetValue())
				{
					if (mbox->GetNumberViewRecent() > 0)
						iconID = IDI_SERVER_MAILBOX_CLOSENEW;
				}
			}

			if (iconID != 0)
				CIconLoader::DrawIcon(pDC, inLocalRect.left, inLocalRect.top + mIconOrigin, iconID, 16);
		}
		break;
	
	case eServerColumnName:
	{
		// Always cache column number
		mHierarchyCol = inCell.col;

		// Get suitable icon
		UINT iconID = IDI_SERVERDIR + GetPlotIcon(type, plot_data);

		// Determine name to use
		switch(type)
		{
		case eServerBrowseMbox:
			// Always do INBOX
			if (!IsCellINBOX(woRow) && GetCellMboxList(woRow)->IsHierarchic())
				theTxt = mbox->GetShortName();
			else
				theTxt = mbox->GetName();
			break;
		case eServerBrowseMboxRef:
			if (GetCellMboxRefList(woRow)->IsHierarchic() && !GetCellMboxRef(woRow)->IsWildcard() && GetCellMboxRef(woRow)->GetWDLevel())
				theTxt = GetCellMboxRef(woRow)->GetShortName();
			else
				theTxt = GetCellMboxRef(woRow)->GetAccountName(CMailAccountManager::sMailAccountManager->GetProtocolCount() > 1);
			break;
		case eServerBrowseWD:
			theTxt = GetCellWD(woRow);
			if ((theTxt == cWILDCARD_NODIR) || (theTxt == cWILDCARD))
				theTxt.FromResource("UI::Namespace::Entire");
			break;
		case eServerBrowseSubs:
			theTxt = sSubscribedName;
			break;
		case eServerBrowseMboxRefList:
			theTxt = GetCellMboxRefList(woRow)->GetName();
			break;
		case eServerBrowseServer:
			theTxt = server->GetAccountName();
			break;
		default:
			theTxt = cdstring::null_str;
		}

		// Draw the string
		bool strike = false;
		SetTextStyle(pDC, server, directory, mbox, strike);
		int text_start = DrawHierarchyRow(pDC, inCell.row, inLocalRect, theTxt, iconID, true);
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

	case eServerColumnTotal:
	case eServerColumnNew:
	case eServerColumnUnseen:
	case eServerColumnSize:
	{
		// Draw total number
		if (mbox && !directory)
		{
			theTxt = "-";
			switch(col_info.column_type)
			{
			case eServerColumnTotal:
				if (mbox->HasStatus())
					theTxt = (long) mbox->GetNumberFound();
				break;
			case eServerColumnNew:
				if (mbox->HasStatus())
					theTxt = (long) mbox->GetNumberViewRecent();
				break;
			case eServerColumnUnseen:
				if (mbox->HasStatus())
					theTxt = (long) mbox->GetNumberUnseen();
				break;
			case eServerColumnSize:
				{
					unsigned long size = mbox->GetSize();
					if ((size == ULONG_MAX) && mbox->IsLocalMbox() && mbox->IsCachedMbox())
					{
						mbox->CheckSize();
						size = mbox->GetSize();
					}
					
					// Is size available
					if (size != ULONG_MAX)
					{
						theTxt = ::GetNumericFormat(size);
					}
					else if (mbox->GetProtocol()->IsLoggedOn() && !mbox->IsLocalMbox() || mbox->IsCachedMbox())
					{
						theTxt = cdstring::null_str;
						int left = (inLocalRect.left + inLocalRect.right)/2 - 8;
						if (left < inLocalRect.left)
							left = inLocalRect.left;
						CIconLoader::DrawIcon(pDC, left, inLocalRect.top + mIconOrigin, IDI_SERVER_CALCULATE_SIZE, 16);
					}
				}
				break;
			}
			if (!theTxt.empty())
				::DrawClippedStringUTF8(pDC, theTxt, CPoint(inLocalRect.left, inLocalRect.top + mTextOrigin), inLocalRect, eDrawString_Right);
		}
		break;
	}

	case eServerColumnAutoCheck:
		// Do status flag
		if (mbox && !directory)
		{
			if (mbox->IsAutoCheck())
				CIconLoader::DrawIcon(pDC, inLocalRect.left, inLocalRect.top + mIconOrigin, IDI_DIAMONDTICKED, 16);
			else
				CIconLoader::DrawIcon(pDC, inLocalRect.left, inLocalRect.top + mIconOrigin, IDI_DIAMOND, 16);
		}
		break;
	
	}
}

SColumnInfo	CServerBrowse::GetColumnInfo(TableIndexT col)
{
	SColumnInfo col_info;
	col_info.column_type = eServerColumnName;
	col_info.column_width = 10;
	
	return col_info;
}

#pragma mark ____________________________Draw state

// Get appropriate icon id
ResIDT CServerBrowse::GetPlotIcon(EServerBrowseDataType type, void* data)
{
	switch(type)
	{
	case eServerBrowseMbox:
	{
		CMbox* mbox = static_cast<CMbox*>(data);
		if (mbox == NULL)
			return eServerIconMailbox;
		else if (mbox->IsDirectory())
			return eServerIconDirectory;
		else if (mbox->Error())
			return eServerIconMailboxNoSelect;
		else if (mbox->IsLocalMbox())
			return mbox->IsCachedMbox() ? eServerIconMailboxCached : eServerIconMailboxUncached;
		else if (mbox->NoSelect())
			return (mbox->NoInferiors() ? eServerIconMailboxNoSelect : eServerIconDirectory);
		else if (mbox->Marked())
			return eServerIconMailboxMarked;
		else if (mbox->Unmarked())
			return eServerIconMailboxUnmarked;
		else if (mbox->NoInferiors())
			return eServerIconMailbox;
		else
			return eServerIconMailbox;
	}
	case eServerBrowseMboxRef:
	{
		CMboxRef* mboxref = static_cast<CMboxRef*>(data);
		if (mboxref == NULL)
			return eServerIconDirectoryRef;
		if (mboxref->IsWildcard())
			return eServerIconSearchRefHierarchy;
		else if (mboxref->IsDirectory())
			return eServerIconDirectoryRef;
		else
		{
			CMbox* mbox = mboxref->ResolveMbox(true);
			if (!mbox)
				return (mboxref->HasFailed() ? eServerIconMailboxRefNoSelect : eServerIconMailboxRef);
			else if (mbox->IsDirectory())
				return eServerIconDirectoryRef;
			else if (mbox->Error())
				return eServerIconMailboxRefNoSelect;
			else if (mbox->NoSelect())
				return (mbox->NoInferiors() ? eServerIconMailboxRefNoSelect : eServerIconDirectoryRef);
			else if (mbox->Marked())
				return eServerIconMailboxRefMarked;
			else if (mbox->Unmarked())
				return eServerIconMailboxRefUnmarked;
			else if (mbox->NoInferiors())
				return eServerIconMailboxRef;
			else
				return eServerIconMailboxRef;
		}
	}
	case eServerBrowseWD:
		return eServerIconSearchHierarchy;
	case eServerBrowseSubs:
		return eServerIconSubscribed;
	case eServerBrowseMboxRefList:
		return eServerIconFavouriteHierarchy;
	case eServerBrowseServer:
		{
			CMboxProtocol* proto = static_cast<CMboxProtocol*>(data);
			if (proto == NULL)
				return eServerIconRemoteHierarchy;
			else if (proto->CanDisconnect())
				return proto->IsDisconnected() ? eServerIconDisconnectedHierarchy : eServerIconRemoteHierarchy;
			else if (proto->GetAccountType() == CINETAccount::eLocal)
				return eServerIconLocalHierarchy;
			else if (proto->GetAccountType() == CINETAccount::ePOP3)
				return eServerIconPOP3Hierarchy;
			else
				return eServerIconRemoteHierarchy;
		}
	default:
		return eServerIconMailboxNoSelect;
	}
}

// Get text style
void CServerBrowse::SetTextStyle(CDC* pDC, CMboxProtocol* proto, bool directory, CMbox* mbox, bool& strike)
{
	strike = false;

	// Select appropriate color and style of text
	//if (UEnvironment::HasFeature(env_SupportsColor))
	{
		bool color_set = false;
		RGBColor text_color;
		bool style_set = false;
		short text_style = normal;

		if (proto)
		{
			if (proto->IsLoggedOn())
			{
				text_color = CPreferences::sPrefs->mServerOpenStyle.GetValue().color;
				text_style = CPreferences::sPrefs->mServerOpenStyle.GetValue().style & 0x007F;
				strike = ((CPreferences::sPrefs->mServerOpenStyle.GetValue().style & 0x0080) != 0);
			}
			else
			{
				text_color = CPreferences::sPrefs->mServerClosedStyle.GetValue().color;
				text_style = CPreferences::sPrefs->mServerClosedStyle.GetValue().style & 0x007F;
				strike =  ((CPreferences::sPrefs->mServerClosedStyle.GetValue().style & 0x0080) != 0);
			}
			
			color_set = true;
			style_set = true;
		}
		else if (mbox)
		{
			if (!directory && mbox->IsOpenSomewhere())
			{
				if (!color_set)
					text_color = CPreferences::sPrefs->mMboxOpenStyle.GetValue().color;
				color_set = true;
				text_style = text_style | CPreferences::sPrefs->mMboxOpenStyle.GetValue().style & 0x007F;
				strike =  strike || ((CPreferences::sPrefs->mMboxOpenStyle.GetValue().style & 0x0080) != 0);
			}
			if (!directory && (mbox->GetNumberViewRecent() > 0) &&
				(!CPreferences::sPrefs->mIgnoreRecent.GetValue() || (mbox->GetNumberUnseen() > 0)))
			{
				if (!color_set)
					text_color = CPreferences::sPrefs->mMboxRecentStyle.GetValue().color;
				color_set = true;
				text_style = text_style | CPreferences::sPrefs->mMboxRecentStyle.GetValue().style & 0x007F;
				strike = strike || ((CPreferences::sPrefs->mMboxRecentStyle.GetValue().style & 0x0080) != 0);
			}
			if (!directory && (mbox->GetNumberUnseen() > 0))
			{
				if (!color_set)
					text_color = CPreferences::sPrefs->mMboxUnseenStyle.GetValue().color;
				color_set = true;
				text_style = text_style | CPreferences::sPrefs->mMboxUnseenStyle.GetValue().style & 0x007F;
				strike =  strike || ((CPreferences::sPrefs->mMboxUnseenStyle.GetValue().style & 0x0080) != 0);
			}
			if (!directory && mbox->IsAutoCheck())
			{
				if (!color_set)
					text_color = CPreferences::sPrefs->mMboxFavouriteStyle.GetValue().color;
				color_set = true;
				text_style = text_style | CPreferences::sPrefs->mMboxFavouriteStyle.GetValue().style & 0x007F;
				strike =  strike || ((CPreferences::sPrefs->mMboxFavouriteStyle.GetValue().style & 0x0080) != 0);
			}
			if (!color_set)
			{
				text_color = CPreferences::sPrefs->mMboxClosedStyle.GetValue().color;
				color_set = true;
				text_style = text_style | CPreferences::sPrefs->mMboxClosedStyle.GetValue().style & 0x007F;
				strike =  strike || ((CPreferences::sPrefs->mMboxClosedStyle.GetValue().style & 0x0080) != 0);
			}
			
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

bool CServerBrowse::UsesBackgroundColor(EServerBrowseDataType type) const
{
	switch(type)
	{
	case eServerBrowseWD:
	case eServerBrowseSubs:
		return CPreferences::sPrefs->mHierarchyBkgndStyle.GetValue().style;
	case eServerBrowseMboxRefList:
		return CPreferences::sPrefs->mFavouriteBkgndStyle.GetValue().style;
	case eServerBrowseServer:
		return CPreferences::sPrefs->mServerBkgndStyle.GetValue().style;
	default:
		return false;
	}
}

COLORREF CServerBrowse::GetBackgroundColor(EServerBrowseDataType type) const
{
	switch(type)
	{
	case eServerBrowseWD:
	case eServerBrowseSubs:
		return CPreferences::sPrefs->mHierarchyBkgndStyle.GetValue().color;
	case eServerBrowseMboxRefList:
		return CPreferences::sPrefs->mFavouriteBkgndStyle.GetValue().color;
	case eServerBrowseServer:
	default:
		return CPreferences::sPrefs->mServerBkgndStyle.GetValue().color;
	}
}

#pragma mark ____________________________Row Management

// Remove rows and adjust parts
void CServerBrowse::RemoveRows(UInt32 inHowMany, TableIndexT inFromRow, bool inRefresh)
{
	if (inHowMany != 1) {
		//SignalPStr_("\pCan only remove one row at a time from a HierarchyTable");
		return;
	}
	
	UInt32	exposedRow = mCollapsableTree->GetExposedIndex(inFromRow);
	
		// Remove Row from CollapsableTree and delete the data
		// for the removed rows
	
	UInt32	totalRemoved, exposedRemoved;
	mCollapsableTree->RemoveNode(inFromRow, totalRemoved, exposedRemoved);
		
	mData.erase(mData.begin() + (inFromRow - 1), mData.begin() + (inFromRow - 1 + totalRemoved));
	
		// Adjust the visual display of the Table if we removed
		// any exposed rows
	
	if (exposedRemoved > 0) {
		if (inRefresh) {				// Redraw cells below removed rows
			STableCell	topLeftCell(exposedRow, 1);
			STableCell	botRightCell(mRows, mCols);

			RefreshCellRange(topLeftCell, botRightCell);
		}

		mRows -= exposedRemoved;
		mTableGeometry->RemoveRows(exposedRemoved, exposedRow);

		if (mTableSelector != NULL) {
			mTableSelector->RemoveRows(exposedRemoved, exposedRow);
		}

		AdjustImageSize(false);
	}
}

// Remove child nodes from the list
void CServerBrowse::RemoveChildren(TableIndexT& parent_row, bool refresh)
{
	int	exposedRow = mCollapsableTree->GetExposedIndex(parent_row);

	UInt32	totalRemoved, exposedRemoved;
	((CNodeVectorTree*) mCollapsableTree)->RemoveChildren(parent_row, totalRemoved, exposedRemoved);
	
	if (totalRemoved)
		mData.erase(mData.begin() + parent_row, mData.begin() + (parent_row + totalRemoved));

		// Adjust the visual display of the Table if we removed
		// any exposed rows
	
	if (exposedRemoved > 0) {
		if (refresh) {				// Redraw cells below removed rows
			RefreshRowsBelow(parent_row);
		}

		mRows -= exposedRemoved;
		mTableGeometry->RemoveRows(exposedRemoved, exposedRow + 1);

		if (mTableSelector != NULL) {
			mTableSelector->RemoveRows(exposedRemoved, exposedRow + 1);
		}

		AdjustImageSize(false);
	}
}
