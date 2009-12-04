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

#include "CINETCommon.h"
#include "CMailAccountManager.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMboxRef.h"
#include "CMulberryCommon.h"
#include "CNamespaceDialog.h"
#include "CNodeVectorTree.h"
#include "CPreferences.h"
#include "CResources.h"
#include "CTableMultiRowSelector.h"
#include "CTableRowGeometry.h"

#include <LDropFlag.h>
#include <LTableArrayStorage.h>

#include <UGAColorRamp.h>

#include <stdio.h>
#include <string.h>

const short cTextIndent = 20;

cdstring CServerBrowse::sSubscribedName;			// Text for subscribed row

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CServerBrowse::CServerBrowse()
{
	InitServerBrowse();
}

// Default constructor - just do parents' call
CServerBrowse::CServerBrowse(LStream *inStream)
	: CHierarchyTableDrag(inStream)
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
	mTableGeometry = new CTableRowGeometry(this, mFrameSize.width, 18);
	mTableSelector = new CTableMultiRowSelector(this);
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

// Get details of sub-panes
void CServerBrowse::FinishCreateSelf(void)
{
	// Do inherited
	CHierarchyTableDrag::FinishCreateSelf();

	// Set Drag & Drop pane to scroller
	mPane = GetSuperView();

	// Set Drag & Drop info
	SetTable(this, true);

	// Set read only status of Drag and Drop
	SetDDReadOnly(false);

	// Turn on tooltips
	EnableTooltips();

	// Make it fit to the superview
	AdaptToNewSurroundings();

	SetTextTraits(CPreferences::sPrefs->mListTextTraits.GetValue().traits);
}

// Update status items like toolbar
void CServerBrowse::UpdateState()
{
	// Nothing to do in this base class
}

#pragma mark ____________________________Draw & Click

// Click in the cell
void CServerBrowse::ClickCell(const STableCell& inCell,
								const SMouseDownEvent& inMouseDown)
{
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(inCell.row + TABLE_ROW_ADJUST);

	// Determine which heading it is
	SColumnInfo col_info = GetColumnInfo(inCell.col);

	// Not when click-through
	if (!inMouseDown.delaySelect)
	{
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

		bool do_refresh = false;
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
				do_refresh = true;
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
					do_refresh = true;
				}
			}
			break;
		default:;
		}
		if (do_refresh)
			RefreshRow(inCell.row);
	}
}

// Click
void CServerBrowse::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	SwitchTarget(this);

	// Must set background color to grey if tracking drop flag
	StColorState	saveColors;

	STableCell	hitCell;
	SPoint32	imagePt;

	LocalToImagePoint(inMouseDown.whereLocal, imagePt);

	if (GetCellHitBy(imagePt, hitCell))
	{
		TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(hitCell.row + TABLE_ROW_ADJUST);
		EServerBrowseDataType type = GetCellDataType(woRow);
		if ((hitCell.col == mHierarchyCol) && UsesBackgroundColor(type))
		{
			FocusDraw();
			ApplyForeAndBackColors();
			::RGBBackColor(&GetBackgroundColor(type));
		}
	}

	CHierarchyTableDrag::ClickSelf(inMouseDown);
}

// Draw the titles
void CServerBrowse::DrawCell(const STableCell &inCell,
								const Rect &inLocalRect)
{
	// Do not allow interim changes to throw
	if (mListChanging)
	{
		// Make sure this gets refreshed again!
		RefreshCell(inCell);
		return;
	}

	// Do not draw if manager is missing
	if (!CMailAccountManager::sMailAccountManager)
		return;

	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(inCell.row + TABLE_ROW_ADJUST);

	EServerBrowseDataType type = GetCellDataType(woRow);

	CMbox* mbox = NULL;
	bool directory = false;
	CMboxProtocol* server = NULL;
	void* plot_data = NULL;
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
	default:;
	}

	// Save text state in stack object
	StTextState		textState;
	StColorState	saveColors;
	StColorPenState::Normalize();
	cdstring		theTxt;

	// Set to required text
	UTextTraits::SetPortTextTraits(&mTextTraits);

	// Clip to cell frame & table frame
	Rect clipper = mRevealedRect;
	PortToLocalPoint(topLeft(clipper));
	PortToLocalPoint(botRight(clipper));
	::SectRect(&clipper,&inLocalRect,&clipper);
	StClipRgnState	clip(clipper);

	// Erase to ensure drag hightlight is overwritten
	FocusDraw();
	if (UsesBackgroundColor(type))
	{
		Rect greyer = inLocalRect;
		greyer.bottom = greyer.top + 1;
		::EraseRect(&greyer);
		::RGBBackColor(&GetBackgroundColor(type));
		greyer = inLocalRect;
		greyer.top++;
		::EraseRect(&greyer);
	}
	else
		::EraseRect(&inLocalRect);

	// Determine which heading it is
	SColumnInfo col_info = GetColumnInfo(inCell.col);

	switch(col_info.column_type)
	{
	case eServerColumnFlags:
	{
		Rect	iconRect;
		iconRect.left = inLocalRect.left;
		iconRect.right = iconRect.left + 16;
		iconRect.bottom = inLocalRect.bottom - mIconDescent;
		iconRect.top = iconRect.bottom - 16;

		// Do status flag
		ResIDT iconID = 0;
		if (mbox && !directory)
		{
			if (mbox->IsOpenSomewhere())
				iconID = ICNx_BrowseMailboxOpen;
			else
			{
				if (mbox->IsAutoCheck())
					iconID = ICNx_BrowseMailboxCloseCheck;
				if (CPreferences::sPrefs->mUnseenNew.GetValue() && CPreferences::sPrefs->mRecentNew.GetValue())
				{
					if ((mbox->GetNumberViewRecent() > 0) && (mbox->GetNumberUnseen() > 0))
						iconID = ICNx_BrowseMailboxCloseNew;
				}
				else if (CPreferences::sPrefs->mUnseenNew.GetValue())
				{
					if (mbox->GetNumberUnseen() > 0)
						iconID = ICNx_BrowseMailboxCloseNew;
				}
				else if (CPreferences::sPrefs->mRecentNew.GetValue())
				{
					if (mbox->GetNumberViewRecent() > 0)
						iconID = ICNx_BrowseMailboxCloseNew;
				}
			}
		}

		// Now plot icon or erase if none
		if (iconID)
			::Ploticns(&iconRect, atNone, ttNone, iconID);
		else
			::EraseRect(&iconRect);
		break;
	}

	case eServerColumnName:
	{
		// Always cache column number
		mHierarchyCol = inCell.col;
		SetOneColumnSelect(mHierarchyCol);

		DrawDropFlag(inCell, woRow);

#if PP_Target_Carbon
		// Draw selection after drop flag to ensure proper background
		bool selected_state = DrawCellSelection(inCell);
#else
		bool selected_state = false;
#endif

		UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);
		Rect	iconRect;
		iconRect.left = inLocalRect.left + mFirstIndent + nestingLevel * mLevelIndent;
		iconRect.right = iconRect.left + 16;
		iconRect.bottom = inLocalRect.bottom - mIconDescent;
		iconRect.top = iconRect.bottom - 16;
		::Ploticns(&iconRect, atNone, selected_state ? ttSelected : ttNone, GetPlotIcon(type, plot_data));

		// Move to origin for text
		::MoveTo(iconRect.right - 2, inLocalRect.bottom - mTextDescent);

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
			{
				cdstring temp = GetCellWD(woRow);
				if ((temp == cWILDCARD_NODIR) || (temp == cWILDCARD))
					temp.FromResource("UI::Namespace::Entire");
				theTxt = temp;
			}
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
		SetTextStyle(server, directory, mbox, strike);
		::DrawClippedStringUTF8(theTxt, inLocalRect.right - iconRect.right - 2, eDrawString_Left, eClipString_Center);
		if (strike)
		{
			::MoveTo(inLocalRect.left + cTextIndent + mFirstIndent + nestingLevel * mLevelIndent, (inLocalRect.top + inLocalRect.bottom)/2);
			::LineTo(inLocalRect.right, (inLocalRect.top + inLocalRect.bottom)/2);
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
					if (size != -1L)
					{
						theTxt = ::GetNumericFormat(size);
					}
					else if (mbox->GetProtocol()->IsLoggedOn() && !mbox->IsLocalMbox() || mbox->IsCachedMbox())
					{
						theTxt = cdstring::null_str;
						Rect	iconRect;
						iconRect.left = (inLocalRect.left + inLocalRect.right)/2 - 8;
						if (iconRect.left < inLocalRect.left)
							iconRect.left = inLocalRect.left;
						iconRect.right = iconRect.left + 16;
						iconRect.bottom = inLocalRect.bottom - mIconDescent;
						iconRect.top = iconRect.bottom - 16;
						::Ploticns(&iconRect, atNone, ttNone, ICNx_BrowseMailboxCalculateSize);
					}
				}
				break;
			}
			if (!theTxt.empty())
			{
				::MoveTo(inLocalRect.left, inLocalRect.bottom - mTextDescent);
				::DrawClippedStringUTF8(theTxt, inLocalRect.right - inLocalRect.left, eDrawString_Right);
			}
		}
		break;
	}

	case eServerColumnAutoCheck:
	{
		Rect	iconRect;
		iconRect.left = inLocalRect.left;
		iconRect.right = iconRect.left + 16;
		iconRect.bottom = inLocalRect.bottom - mIconDescent;
		iconRect.top = iconRect.bottom - 16;

		// Do status flag
		if (mbox && !directory)
			::Ploticns(&iconRect, atNone, ttNone, mbox->IsAutoCheck() ? ICNx_DiamondTicked : ICNx_Diamond);
		else
			::EraseRect(&iconRect);
		break;
	}
	}
}

// Draw or undraw active hiliting for a Cell
void CServerBrowse::HiliteCellActively(const STableCell &inCell, Boolean inHilite)
{
	if (mListChanging)
		return;

	CHierarchyTableDrag::HiliteCellActively(inCell, inHilite);
}

// Draw or undraw inactive hiliting for a Cell
void CServerBrowse::HiliteCellInactively(const STableCell &inCell, Boolean inHilite)
{
	if (mListChanging)
		return;

	CHierarchyTableDrag::HiliteCellInactively(inCell, inHilite);
}

void CServerBrowse::CalcCellFlagRect(const STableCell &inCell, Rect &outRect)
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
		CMbox* mbox = static_cast<CMbox*>(data);
		if (mbox == NULL)
			return ICNx_BrowseMailbox;
		else if (mbox->IsDirectory())
			return ICNx_BrowseDirectory;
		else if (mbox->Error())
			return ICNx_BrowseMailboxNoSelect;
		else if (mbox->IsLocalMbox())
			return mbox->IsCachedMbox() ? ICNx_BrowseMailboxCached : ICNx_BrowseMailboxUncached;
		else if (mbox->NoSelect())
			return (mbox->NoInferiors() ? ICNx_BrowseMailboxNoSelect : ICNx_BrowseDirectory);
		else if (mbox->Marked())
			return ICNx_BrowseMailboxMarked;
		else if (mbox->Unmarked())
			return ICNx_BrowseMailboxUnmarked;
		else if (mbox->NoInferiors())
			return ICNx_BrowseMailbox;
		else
			return ICNx_BrowseMailbox;
	case eServerBrowseMboxRef:
		CMboxRef* mboxref = static_cast<CMboxRef*>(data);
		if (mboxref == NULL)
			return ICNx_BrowseDirectoryRef;
		else if (mboxref->IsWildcard())
			return ICNx_BrowseSearchRefHierarchy;
		else if (mboxref->IsDirectory())
			return ICNx_BrowseDirectoryRef;
		else
		{
			CMbox* mbox = mboxref->ResolveMbox(true);
			if (!mbox)
				return (mboxref->HasFailed() ? ICNx_BrowseMailboxRefNoSelect : ICNx_BrowseMailboxRef);
			else if (mbox->IsDirectory())
				return ICNx_BrowseDirectoryRef;
			else if (mbox->Error())
				return ICNx_BrowseMailboxRefNoSelect;
			else if (mbox->NoSelect())
				return (mbox->NoInferiors() ? ICNx_BrowseMailboxRefNoSelect : ICNx_BrowseDirectoryRef);
			else if (mbox->Marked())
				return ICNx_BrowseMailboxRefMarked;
			else if (mbox->Unmarked())
				return ICNx_BrowseMailboxRefUnmarked;
			else if (mbox->NoInferiors())
				return ICNx_BrowseMailboxRef;
			else
				return ICNx_BrowseMailboxRef;
		}
	case eServerBrowseWD:
		return ICNx_BrowseSearchHierarchy;
	case eServerBrowseSubs:
		return ICNx_BrowseSubscribed;
	case eServerBrowseMboxRefList:
		return ICNx_BrowseFavouriteHierarchy;
	case eServerBrowseServer:
		{
			CMboxProtocol* proto = static_cast<CMboxProtocol*>(data);
			if (proto == NULL)
				return ICNx_BrowseRemoteHierarchy;
			else if (proto->CanDisconnect())
				return proto->IsDisconnected() ? ICNx_BrowseDisconnectedHierarchy : ICNx_BrowseRemoteHierarchy;
			else if (proto->GetAccountType() == CINETAccount::eLocal)
				return ICNx_BrowseLocalHierarchy;
			else if (proto->GetAccountType() == CINETAccount::ePOP3)
				return ICNx_BrowsePOP3Hierarchy;
			else
				return ICNx_BrowseRemoteHierarchy;
		}
	default:
		return ICNx_BrowseMailboxNoSelect;
	}
}

// Get text style
void CServerBrowse::SetTextStyle(CMboxProtocol* proto, bool directory, CMbox* mbox, bool& strike)
{
	strike = false;

	// Select appropriate color and style of text
	if (UEnvironment::HasFeature(env_SupportsColor))
	{
		if (proto)
		{
			bool color_set = false;
			RGBColor text_color;
			Style text_style = normal;

			if (proto->IsLoggedOn())
			{
				if (!color_set)
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
		else if (mbox)
		{
			bool color_set = false;
			RGBColor text_color;
			Style text_style = normal;

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

			::RGBForeColor(&text_color);
			::TextFace(text_style);
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

const RGBColor& CServerBrowse::GetBackgroundColor(EServerBrowseDataType type) const
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
void CServerBrowse::RemoveRows(UInt32 inHowMany, TableIndexT inFromRow, Boolean inRefresh)
{
	if (inHowMany != 1) {
		SignalPStr_("\pCan only remove one row at a time from a HierarchyTable");
		return;
	}

	TableIndexT	exposedRow = mCollapsableTree->GetExposedIndex(inFromRow);

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
	TableIndexT	exposedRow = mCollapsableTree->GetExposedIndex(parent_row);

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
