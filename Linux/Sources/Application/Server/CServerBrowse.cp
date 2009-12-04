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

#include <JXImage.h>
#include <JPainter.h>
#include <JXColormap.h>
#include <JColormap.h>

const short cTextIndent = 26;

#include "StPenState.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

	// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

cdstring CServerBrowse::sSubscribedName;

// Default constructor
CServerBrowse::CServerBrowse(JXScrollbarSet* scrollbarSet, 
								JXContainer* enclosure,
								const HSizingOption hSizing, 
								const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h)
	: CHierarchyTableDrag(scrollbarSet, enclosure, hSizing, vSizing, x,y,w,h)
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

void CServerBrowse::OnCreate()
{
	CHierarchyTableDrag::OnCreate();

	// Set read only status of Drag and Drop
	SetReadOnly(false);

	// Turn on tooltips
	EnableTooltips();
}

// Update status items like toolbar
void CServerBrowse::UpdateState()
{
	// Nothing to do in this base class
}

//#pragma mark ____________________________Draw & Click

// Click in the cell
void CServerBrowse::LClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
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
	default:;
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
		CHierarchyTableDrag::LClickCell(inCell, modifiers);
}


// Draw the titles
void CServerBrowse::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
	// Do not allow interim changes to throw
	if (mListChanging)
		return;

	// Do not draw if manager is missing
	if (!CMailAccountManager::sMailAccountManager)
		return;

	StPenState savestate(pDC);

	int woRow = mCollapsableTree->GetWideOpenIndex(inCell.row);

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
	default:
		;
	}

	// Erase to ensure drag hightlight is overwritten
	unsigned long bkgnd = 0x00FFFFFF;
	if (UsesBackgroundColor(type))
	{
		StPenState save(pDC);
		JRect bgrect(inLocalRect);
		//If we don't make the rectangle smaller, there will be no border between
		//color stripes.  If we just use the table row's border, then the tree
		//lines get broken up.
		bgrect.bottom -=1;
		
		JColorIndex bkgnd_index = GetBackgroundColor(type);
		pDC->SetPenColor(bkgnd_index);
		pDC->SetFilling(kTrue);
		pDC->Rect(bgrect);
		
		JSize r, g, b;
		GetColormap()->GetRGB(bkgnd_index, &r, &g, &b);
		bkgnd = ((r & 0xFF00) << 8) | (g & 0xFF00) | ((b & 0xFF00) >> 8);
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
			ResIDT iconID = 0;
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
			{
				JXImage* icon = CIconLoader::GetIcon(iconID, this, 16, bkgnd);
				pDC->Image(*icon, icon->GetBounds(), inLocalRect.left, inLocalRect.top + mIconOrigin);
			}
		}
		break;
  
	case eServerColumnName:
	{
		// Always cache column number
		mHierarchyCol = inCell.col;

		// Get suitable icon
		ResIDT iconID = IDI_SERVERDIR + GetPlotIcon(type, plot_data);
		JXImage* icon = CIconLoader::GetIcon(iconID, this, 16, bkgnd);

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
				if (GetCellMboxRefList(woRow)->IsHierarchic() && !GetCellMboxRef(woRow)->IsWildcard())
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
		int text_start = DrawHierarchyRow(pDC, inCell.row, inLocalRect, theTxt.c_str(), icon, true);
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
								JCoordinate left = (inLocalRect.left + inLocalRect.right)/2 - 8;
								if (left < inLocalRect.left)
									left = inLocalRect.left;
								JXImage* icon = CIconLoader::GetIcon(IDI_SERVER_CALCULATE_SIZE, this, 16, bkgnd);
								pDC->Image(*icon, icon->GetBounds(), left, inLocalRect.top + mIconOrigin);
							}
						}
						break;
					}
				if (!theTxt.empty())
					::DrawClippedStringUTF8(pDC, theTxt.c_str(), JPoint(inLocalRect.left, inLocalRect.top), inLocalRect, eDrawString_Right);
			}
		break;
	}

	case eServerColumnAutoCheck:
		// Do status flag
		if (mbox && !directory)
			{
				ResIDT iconID = 0;
				if (mbox->IsAutoCheck())
					iconID = IDI_DIAMONDTICKED;
				else
					iconID = IDI_DIAMOND;
				JXImage* icon = CIconLoader::GetIcon(IDI_SERVER_CALCULATE_SIZE, this, 16, bkgnd);
				pDC->Image(*icon, icon->GetBounds(), inLocalRect.left, inLocalRect.top + mIconOrigin);
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

//#pragma mark ____________________________Draw state

// Get appropriate icon id
CServerBrowse::IconResIDT CServerBrowse::GetPlotIcon(EServerBrowseDataType type, 
																								 void* data)
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
		else if (mboxref->IsWildcard())
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
void CServerBrowse::SetTextStyle(JPainter* pDC, CMboxProtocol* proto, bool directory, CMbox* mbox, bool& strike)
{
	strike = false;

	bool color_set = false;
	bool style_set = false;
	JFontStyle text_style;
	short style = normal;
	if (proto)
		{
			if (proto->IsLoggedOn())
				{
					GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->
																				 mServerOpenStyle.GetValue().color,
																				 &text_style.color);
					style = CPreferences::sPrefs->mServerOpenStyle.GetValue().style;
				}
			else
				{
					GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->
																				 mServerClosedStyle.GetValue().color,
																				 &text_style.color);
					style = CPreferences::sPrefs->mServerClosedStyle.GetValue().style;
				}
			
			color_set = true;
			style_set = true;
		}
	else if (mbox)
		{
			if (!directory && mbox->IsOpenSomewhere())
				{
					if (!color_set)
						GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->mMboxOpenStyle.GetValue().color, &text_style.color);
					color_set = true;
					style = style | CPreferences::sPrefs->mMboxOpenStyle.GetValue().style;
				}
			if (!directory && (mbox->GetNumberViewRecent() > 0) &&
				(!CPreferences::sPrefs->mIgnoreRecent.GetValue() || (mbox->GetNumberUnseen() > 0)))
				{
					if (!color_set)
						GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->mMboxRecentStyle.GetValue().color, &text_style.color);
					color_set = true;
					style = style | CPreferences::sPrefs->mMboxRecentStyle.GetValue().style;
				}
			if (!directory && (mbox->GetNumberUnseen() > 0))
				{
					if (!color_set)
						GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->mMboxUnseenStyle.GetValue().color, &text_style.color);
					color_set = true;
					style = style | CPreferences::sPrefs->mMboxUnseenStyle.GetValue().style;
				}
			if (!directory && mbox->IsAutoCheck())
				{
					if (!color_set)
						GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->mMboxFavouriteStyle.GetValue().color, &text_style.color);
					color_set = true;
					style = style | CPreferences::sPrefs->mMboxFavouriteStyle.GetValue().style;
				}
			if (!color_set)
				{
					GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->mMboxClosedStyle.GetValue().color, &text_style.color);
					color_set = true;
					style = style | CPreferences::sPrefs->mMboxClosedStyle.GetValue().style;
				}
			
			style_set = true;
		}
	text_style.bold = JBoolean(style & bold);
	text_style.italic = JBoolean(style & italic);
	text_style.underlineCount = JBoolean((style & underline) ? 1 : 0);
	text_style.strike = JBoolean(style & strike);

	if (color_set)
		pDC->SetPenColor(text_style.color);
	else {
		text_style.color = pDC->GetPenColor();
	}

	if (style_set) {
		pDC->SetFontStyle(text_style);
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


JColorIndex CServerBrowse::GetBackgroundColor(EServerBrowseDataType type) const
{
	JColorIndex ret;
	switch(type)
	{
	case eServerBrowseWD:
	case eServerBrowseSubs:
		GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->mHierarchyBkgndStyle.GetValue().color, &ret);
		break;
	case eServerBrowseMboxRefList:
		GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->mFavouriteBkgndStyle.GetValue().color, &ret);
		break;
	case eServerBrowseServer:
	default:
		GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->mServerBkgndStyle.GetValue().color, &ret);
		break;
	}
	return ret;
}

//#pragma mark ____________________________Row Management

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
	
	if (exposedRemoved > 0) 
	{
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
	
	if (exposedRemoved > 0)
	{
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
