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

#include "CActionManager.h"
#include "CAddress.h"
#include "CAddressList.h"
#include "CCommands.h"
#include "CCopyToMenu.h"
#include "CEnvelope.h"
#include "CErrorHandler.h"
#include "CIconLoader.h"
#include "CLetterTextEditView.h"
#include "CMailControl.h"
#include "CMailboxInfoTable.h"
#include "CMailboxTitleTable.h"
#include "CMailboxToolbarPopup.h"
#include "CMailboxView.h"
#include "CMbox.h"
#include "CMboxRef.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CMulSelectionData.h"
#include "CPreferences.h"
#include "CServerTable.h"
#include "CStatusWindow.h"
#include "CUserAction.h"
#include "CTableRowGeometry.h"
#include "CTableRowSelector.h"
#include "CWindow.h"

#include <jASCIIConstants.h>

#include "StPenState.h"
#include "StValueChanger.h"

#include <JXDNDManager.h>
#include <JXImage.h>
#include <JXDisplay.h>
#include <JXColormap.h>
#include <JXWindowPainter.h>
#include <jXKeysym.h>

#include <JXTextMenu.h>

#include <algorithm>
#include <memory>

// Consts

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

// Static variables
bool CMailboxTable::sDropOnMailbox = false;
JColorList *CMailboxTable::sColorList = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxTable::CMailboxTable(JXScrollbarSet* scrollbarSet,
								 JXContainer* enclosure,
								 const HSizingOption hSizing,
								 const VSizingOption vSizing,
								 const JCoordinate x, const JCoordinate y,
								 const JCoordinate w, const JCoordinate h)
	: CTableDragAndDrop(scrollbarSet, enclosure, hSizing, vSizing, x,y,w,h)
{
	SetBorderWidth(0);

	InitMailboxTable();

	if (!sColorList) {
		sColorList = 
			new JColorList(CMulberryApp::sApp->GetCurrentDisplay()->GetColormap());
	}

	//make keypresses come to us
	WantInput(kTrue, kTrue, kTrue, kTrue);
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
	mSelectionPreserved = false;
	mDeferSelectionChanged = false;
	mResetTable = false;
	mPreviewUID = 0;
	mIsSelectionValid = false;
	mTestSelectionAndDeleted = false;
	mUpdating = false;

	mTableGeometry = new CTableRowGeometry(this, 72, 16);
	mTableSelector = new CTableRowSelector(this);

	SetRowSelect(true);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CMailboxTable::OnCreate(void)
{
	const JXContainer* parent = GetEnclosure();
	while(parent && !dynamic_cast<const CMailboxView*>(parent))
		parent = parent->GetEnclosure();
	mTableView = const_cast<CMailboxView*>(dynamic_cast<const CMailboxView*>(parent));

	// Do inherited
	CTableDragAndDrop::OnCreate();

	// Turn on tooltips
	EnableTooltips();
}
	

// Keep titles in sync
void CMailboxTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh)
{
	// Do scroll of main table first to avoid double-refresh
	CTableDragAndDrop::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);

	// Find titles in owner chain
	mTableView->GetBaseTitles()->ScrollImageBy(inLeftDelta, 0, inRefresh);
}

// Handle key presses
bool CMailboxTable::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	// Determine whether preview is triggered
	CKeyModifiers mods(modifiers);
	const CUserAction& preview = mTableView->GetPreviewAction();
	if ((preview.GetKey() == key) &&
		(preview.GetKeyModifiers() == mods))
	{
		PreviewMessage();
		return true;
	}

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if ((fullview.GetKey() == key) &&
		(fullview.GetKeyModifiers() == mods))
	{
		DoFileOpenMessage(mods.Get(CKeyModifiers::eAlt));
		return true;
	}

	switch (key)
	{
	case ' ':
		HandleSpacebar(mods.Get(CKeyModifiers::eShift));
		return true;

	// Toggle delete
	case kJDeleteKey:
	case kJForwardDeleteKey:
		if (mIsSelectionValid && GetMbox() && GetMbox()->HasAllowedFlag(NMessage::eDeleted))
		{
			DoFlagMailMessage(NMessage::eDeleted);
		}
		return true;

	case kJLeftArrow:
	case kJRightArrow:
		if (GetMbox() && (GetMbox()->GetSortBy() == cSortMessageThread))
		{
			TableIndexT row = GetFirstSelectedRow();
			if (row >= 1)
			{
				// Get message
				const CMessage* thisMsg = GetMbox()->GetMessage(row, true);

				if (thisMsg)
				{
					// Move up/down hierarchy to sibling
					const CMessage* theMsg = (key == kJLeftArrow) ?
								thisMsg->GetThreadPrevious() : thisMsg->GetThreadNext();

					// Go up to parent if no previous
					if ((key == kJLeftArrow) && !theMsg)
						theMsg = thisMsg->GetThreadParent();

					// Only if parent exists
					if (theMsg)
					{
						row = GetMbox()->GetMessageIndex(const_cast<CMessage*>(theMsg), true);

						// Select and bring into middle of view if outside
						ScrollToRow(row, true, true, eScroll_Center);
					}
				}
			}
		}
		return true;

	default:
		return CTableDragAndDrop::HandleChar(key, modifiers);
	}
}

// Clicked item
void CMailboxTable::LClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	CTableDragAndDrop::LClickCell(inCell, modifiers);
	DoSingleClick(inCell.row, inCell.col, CKeyModifiers(modifiers));
}

void CMailboxTable::LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	DoDoubleClick(inCell.row, inCell.col, CKeyModifiers(modifiers));
}

void CMailboxTable::Draw(JXWindowPainter& pDC, const JRect& rect)
{
	// Set text font before drawing any cells
	JFontStyle style;
	pDC.SetFont(mFontInfo.fontname, mFontInfo.size, style);
	
	DrawBackground(pDC, rect);

	// Undo the origin offset as we want to draw in local co-ords,
	// not image co-ords

	const JRect boundsG = GetBoundsGlobal();
	JRect localboundsG;
	LocalToImageRect(boundsG, localboundsG);
	pDC.SetOrigin(localboundsG.left, localboundsG.top);

		// Determine cells that need updating. Rather than checking
		// on a cell by cell basis, we just see which cells intersect
		// the bounding box of the update region. This is relatively
		// fast, but may result in unnecessary cell updates for
		// non-rectangular update regions.
	
	// Input rect is in image (bounds) coordinates
	JRect localRect;
	ImageToLocalRect(rect, localRect);

	STableCell	topLeftCell, botRightCell;
	FetchIntersectingCells(localRect, topLeftCell, botRightCell);
	
	// Only if open and not changing
	if (mTableView->IsOpen() && !mListChanging)
	{
		// Draw each cell within the update rect
		for (TableIndexT row = topLeftCell.row; row <= botRightCell.row; row++)
			DrawRow(&pDC, row, topLeftCell.col, botRightCell.col);
	}
	
	// Trigger update aftr current op ends
	else if (mTableView->IsOpen() && mListChanging)
		mUpdateRequired = true;
}

// Draw the titles
void CMailboxTable::DrawRow(JPainter* pDC, TableIndexT row, TableIndexT start_col, TableIndexT stop_col)
{
	if (!GetMbox())
		return;

	// Allow get message to fail during caching process
	CMessage* theMsg = NULL;
	try
	{
		theMsg = GetMbox()->GetMessage(row, true);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}
	if (!theMsg) return;

	// Check whether already cached
	if (!theMsg->IsFullyCached())
	{
		// do not allow caching while processing another network command
		if (GetMbox()->GetMsgProtocol() && !GetMbox()->GetMsgProtocol()->_get_mutex().is_locked())
		{
			StValueChanger<bool> _change(mListChanging, true);

			{
				// Preserve selection without scroll
				StMailboxTableSelection preserve_selection(this, false);
				
				try
				{
					theMsg = GetMbox()->GetCacheMessage(row, true);
				}
				catch (...)
				{
					CLOG_LOGCATCH(...);

					theMsg = NULL;
				}
				if (!theMsg) return;
			}

			UpdateItems();

			// If sorting by anything other than by message number, must do entire refresh
			//if (GetMbox()->GetSortBy() != cSortMessageNumber)
				Refresh();
		}
		else
		{
			// Force refresh of cell for when protocol is unblocked
			RefreshRow(row);

			return;
		}
	}

	// Check highlight state
	StPenState save(pDC);

	JRGB text_color;
	short text_style = normal;

	GetDrawStyle(theMsg, text_color, text_style);

	for(TableIndexT col = start_col; col <= stop_col; col++)
	{
		STableCell inCell(row, col);
		JRect	inLocalRect;
		GetLocalCellRect(inCell, inLocalRect);
		
		bool selected = CellIsSelected(inCell);

		// Check for coloured background - only do when there is no selection, or
		// the selection is an outline
		unsigned long bkgnd = 0x00FFFFFF;
		if (UsesBackground(theMsg))
		{
			StPenState save(pDC);
			JRect rect(inLocalRect);
			rect.bottom--;
			JColorIndex bkgnd_index = GetBackground(theMsg);
			pDC->SetPenColor(bkgnd_index);
			pDC->SetFilling(kTrue);
			pDC->RectInside(rect);
			
			JSize r, g, b;
			GetColormap()->GetRGB(bkgnd_index, &r, &g, &b);
			bkgnd = ((r & 0xFF00) << 8) | (g & 0xFF00) | ((b & 0xFF00) >> 8);
		}
		else if (selected && IsActive() && HasFocus())
		{
			JColorIndex bkgnd_index = GetColormap()->GetDefaultSelectionColor();
			
			JSize r, g, b;
			GetColormap()->GetRGB(bkgnd_index, &r, &g, &b);
			bkgnd = ((r & 0xFF00) << 8) | (g & 0xFF00) | ((b & 0xFF00) >> 8);
		}

		// Draw selection
		DrawCellSelection(pDC, inCell);

		DrawMessage(pDC, theMsg, inCell, inLocalRect, text_color, text_style, bkgnd);
	}
}

// Draw the titles
void CMailboxTable::DrawMessage(JPainter* pDC,
								const CMessage* aMsg,
								const STableCell& cell, 
								const JRect& inLocalRect,
								const JRGB& color,
								short text_style,
							 	unsigned long bkgnd)
{
	cdstring		theTxt;
	ResIDT			smart_flag = 0;
	bool			multi = false;
	unsigned long	depth = 0;

	const CEnvelope* theEnv = aMsg->GetEnvelope();
	if (!theEnv)
		return;

	int x = inLocalRect.left + 4;
	int y = inLocalRect.top + mTextOrigin;

	ResIDT plot_flag = 0;
	bool do_plot = false;
	bool selected = CellIsSelected(cell);
	
	// Determine which heading it is
	SColumnInfo col_info = mTableView->GetColumnInfo()[cell.col - 1];

	// Determine which heading it is and draw it
	bool right_just = false;
	switch (col_info.column_type)
	{

		case eMboxColumnSmart:
			// Set up the icons
			if (aMsg->IsSmartFrom())
				smart_flag = IDI_FLAG_SMART_FROM;
			else if (aMsg->IsSmartTo())
				smart_flag = IDI_FLAG_SMART_TO;
			else if (aMsg->IsSmartCC())
				smart_flag = IDI_FLAG_SMART_CC;
			else
				smart_flag = IDI_FLAG_SMART_BCC;

			// Check for message From current user
			if (aMsg->IsSmartFrom())
			{
				// Display first to address
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
					smart_flag = IDI_FLAG_SMART_FROM;
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
			theTxt = theEnv->GetTextDate(true);
			break;

		case eMboxColumnDateReceived:
			theTxt = aMsg->GetTextInternalDate(true);
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
			plot_flag = IDI_FLAG_DELETED + GetPlotFlag(aMsg);
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
					plot_flag = IDI_FLAG_ENCRYPTED;

				// Check for signed data of various types
				else if (is_signed)
				{
					if (is_styled && is_attach)
						plot_flag = IDI_FLAG_SIGNED_ALTERNATIVE_ATTACHMENT;
					else if (is_styled && is_calendar)
						plot_flag = IDI_FLAG_SIGNED_ALTERNATIVE_CALENDAR;
					else if (is_styled)
						plot_flag = IDI_FLAG_SIGNED_ALTERNATIVE;
					else if (is_calendar)
						plot_flag = IDI_FLAG_SIGNED_CALENDAR;
					else if (is_attach)
						plot_flag = IDI_FLAG_SIGNED_ATTACHMENT;
					else
						plot_flag = IDI_FLAG_SIGNED;
				}

				// Check for unsigned data of various types
				else
				{
					if (is_styled && is_attach)
						plot_flag = IDI_FLAG_ALTERNATIVE_ATTACHMENT;
					else if (is_styled && is_calendar)
						plot_flag = IDI_FLAG_ALTERNATIVE_CALENDAR;
					else if (is_styled)
						plot_flag = IDI_FLAG_ALTERNATIVE;
					else if (is_calendar)
						plot_flag = IDI_FLAG_CALENDAR;
					else if (is_attach)
						plot_flag = IDI_FLAG_ATTACHMENT;
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
				plot_flag = IDI_FLAG_MATCH;
			break;

		case eMboxColumnDisconnected:
			do_plot = true;
			if (aMsg->IsFullLocal())
				plot_flag = IDI_FLAG_CACHED;
			else if (aMsg->IsPartialLocal())
				plot_flag = IDI_FLAG_PARTIAL;
			break;
	}

	// Or in style for multiple addresses
	if (multi)
	{
		text_style |= (CPreferences::sPrefs->mMultiAddress.GetValue());
	}

	// If message is fake and text is empty, use questiuon marks
	if (aMsg->IsFake() && (col_info.column_type != eMboxColumnSubject) && (col_info.column_type != eMboxColumnThread))
		theTxt = "???";

	// Draw the text
	if (!do_plot)
	{
		// Check for smart address
		switch(col_info.column_type)
		{
		case eMboxColumnSmart:
			// Only draw icon if not fake
			if (!aMsg->IsFake())
			{
				// Check for smart address
				JXImage* icon = CIconLoader::GetIcon(smart_flag, this, 16, bkgnd);
				pDC->Image(*icon, icon->GetBounds(), inLocalRect.left + 2, inLocalRect.top + mIconOrigin);
			}

			// Advance over icon (even if not present)
			x += 12;
			break;
		case eMboxColumnSubject:
		case eMboxColumnThread:
		{
			// Check for thread
			if (depth)
			{
				{
					// Set up dotted pen - use text color if selected to get proper inversion
					StPenState save(pDC);
					pDC->SetPenColor(pDC->GetColormap()->GetBlackColor());
					JArray<JSize> dashList;
					dashList.AppendElement(1);
					dashList.AppendElement(1);
					pDC->DrawDashedLines(kTrue);
					pDC->SetDashList(dashList, 0);

					// Prevent drawing outside of cell area
					unsigned long max_depth_offset = inLocalRect.width() - 48;

					// Draw right-angle line
					unsigned long depth_offset = (depth - 1) * 16;
					
					// Clip to cell width
					if (depth_offset < max_depth_offset)
					{
						pDC->Line(inLocalRect.left + depth_offset + 8, inLocalRect.top,
									inLocalRect.left + depth_offset + 8, (inLocalRect.top + inLocalRect.bottom)/2);
						pDC->LineTo(inLocalRect.left + depth_offset + 16, (inLocalRect.top + inLocalRect.bottom)/2);
					}

					// Check for parent/sibling lines
					const CMessage* parent = aMsg;
					while(parent)
					{
						// Draw vert line if the parent has a sibling after it (clip to cell width)
						if (parent->GetThreadNext() && (depth_offset < max_depth_offset))
						{
							pDC->Line(inLocalRect.left + depth_offset + 8, inLocalRect.top,
										inLocalRect.left + depth_offset + 8, inLocalRect.bottom);
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
				unsigned long text_offset = std::min(inLocalRect.width() - 32UL, 16 * depth);
				x += text_offset;
			}
			break;
		}
		default:;
		}

		// Set appropriate font & color
		JFontStyle style;
		style.color = sColorList->Add(color);
		style.bold = JBoolean(text_style & bold);
		style.italic = JBoolean(text_style & italic);
		style.underlineCount = JBoolean((text_style & underline) ? 1 : 0);
		pDC->SetPenColor(style.color);
		pDC->SetFontStyle(style);

		// Draw text
		::DrawClippedStringUTF8(pDC, theTxt.c_str(), JPoint(x, y), inLocalRect, right_just ? eDrawString_Right : eDrawString_Left);

		// Don't strike out fakes
		if ((text_style & strike) && !aMsg->IsFake())
		{
			 pDC->SetPenLocation(inLocalRect.left, (inLocalRect.top + inLocalRect.bottom)/2);
			 pDC->LineTo(inLocalRect.right, (inLocalRect.top + inLocalRect.bottom)/2);
		}

	}

	// Draw icon
	else
	{
		// Don't draw if its a fake
		if ((plot_flag != 0) && !aMsg->IsFake())
		{
			JXImage* icon = CIconLoader::GetIcon(plot_flag, this, 16, bkgnd);
			pDC->Image(*icon, icon->GetBounds(), inLocalRect.left, inLocalRect.top + mIconOrigin);
		}

		// Don't strike out fakes or flags
		if ((text_style & strike) && !aMsg->IsFake() && (col_info.column_type != eMboxColumnFlags))
		{
			// Set appropriate font & color
			JFontStyle style;
			style.color = sColorList->Add(color);
			pDC->SetPenColor(style.color);
			pDC->SetPenLocation(inLocalRect.left, (inLocalRect.top + inLocalRect.bottom)/2);
			pDC->LineTo(inLocalRect.right, (inLocalRect.top + inLocalRect.bottom)/2);
		}
	}
}

void CMailboxTable::GetDrawStyle(const CMessage* theMsg, JRGB& text_color, short& text_style) const
{
	bool deleted = false;
	if (theMsg->IsDeleted())
	{
		text_color = CPreferences::sPrefs->deleted.GetValue().color;
		text_style = CPreferences::sPrefs->deleted.GetValue().style;
		deleted = true;
	}
	else if (theMsg->IsFlagged())
	{
		text_color = CPreferences::sPrefs->important.GetValue().color;
		text_style = CPreferences::sPrefs->important.GetValue().style;
	}
	else if (theMsg->IsAnswered())
	{
		text_color = CPreferences::sPrefs->answered.GetValue().color;
		text_style = CPreferences::sPrefs->answered.GetValue().style;
	}
	else if (theMsg->IsUnseen())
	{
		text_color = CPreferences::sPrefs->unseen.GetValue().color;
		text_style = CPreferences::sPrefs->unseen.GetValue().style;
	}
	else
	{
		// Set default seen style
		text_color = CPreferences::sPrefs->seen.GetValue().color;
		text_style = CPreferences::sPrefs->seen.GetValue().style;
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
				text_style |= CPreferences::sPrefs->mLabels.GetValue()[i]->style;
				
				// The first label set always wins for colors
				got_label = true;
			}
		}
	}
}

long CMailboxTable::GetPlotFlag(const CMessage* aMsg) const
{
	if (aMsg->IsDeleted())
		return eDeleted_Flag;
	else if (aMsg->IsDraft())
		return eDraft_Flag;
	else if (aMsg->IsAnswered())
		return eAnswered_Flag;
	else if (aMsg->IsUnseen() && aMsg->IsRecent())
		return eRecent_Flag;
	else if (aMsg->IsUnseen())
		return eUnseen_Flag;
	else
		return eRead_Flag;
}

bool CMailboxTable::UsesBackground(const CMessage* aMsg) const
{
	// Must have mailbox
	if (!GetMbox())
		return false;

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

JColorIndex CMailboxTable::GetBackground(const CMessage* aMsg) const
{
	// Use background for matched/unmatched
	if (aMsg->IsSearch() && (GetMbox()->GetViewMode() == NMbox::eViewMode_AllMatched))
		// Only use match color when not in match mode
		return sColorList->Add(CPreferences::sPrefs->mMatch.GetValue().color);
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
						return sColorList->Add(CPreferences::sPrefs->mLabels.GetValue()[i]->bkgcolor);
				}
			}
		}
		
		// Use default non matched colour	
		return sColorList->Add(CPreferences::sPrefs->mNonMatch.GetValue().color);
	}
}

// Get horiz scrollbar pos
void CMailboxTable::GetScrollPos(long& h, long& v) const
{
	const JRect ap = GetAperture();

	// Get view position of top item
	h = ap.left;
	v = ap.top;
}

// Get horiz scrollbar pos
void CMailboxTable::SetScrollPos(long h, long v)
{
	// Scroll to move top item to new location
	ScrollTo(h, v);
}

#pragma mark ____________________________Commands

// Print a specified mail message
bool CMailboxTable::PrintMailMessage(TableIndexT row)
{
	if (!GetMbox())
		return false;

#if NOTYET
	// Get the relevant message and envelope
	CMessage* theMsg = GetMbox()->GetMessage(row, true);
	if (!theMsg || !theMsg->IsFullyCached() || theMsg->IsFake()) return false;
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

		THPrint copy_print = *printRecordH;
		ThrowIfOSErr_(::HandToHand(&((Handle) copy_print)));

		// Found existing window so print
		theWindow->SetPrintH(copy_print);
		theWindow->DoPrint();
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
		THPrint copy_print = *printRecordH;
		ThrowIfOSErr_(::HandToHand(&((Handle) copy_print)));
		newWindow->SetPrintH(copy_print);
		newWindow->DoPrint();
		FRAMEWORK_DELETE_WINDOW(newWindow)
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Only delete if it still exists
		if (CMessageWindow::FindWindow(theMsg))
			FRAMEWORK_DELETE_WINDOW(newWindow)

		// Should throw out of here in case abort and mbox now destroyed
		CLOG_LOGRETHROW;
		throw;
	}
#endif
	return false;
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

#pragma mark ____________________________Speech

#pragma mark ____________________________Display updating

void CMailboxTable::DoSelectionChanged(void)
{
	mIsSelectionValid = IsSelectionValid() && !TestSelectionAnd((TestSelectionPP) &CMailboxTable::TestSelectionFake);
	mTestSelectionAndDeleted = TestSelectionIgnore1And(&CMailboxTable::TestSelectionFlag, NMessage::eDeleted);
	
	// Do inherited to ensure broadcast
	CTableDragAndDrop::DoSelectionChanged();
}

// Reset the table from the mbox
void CMailboxTable::ResetTable(bool scroll_new)
{
	// Preserve selection
	StMailboxTableSelection preserve_selection(this, false);

	{
		// Prevent screen updates because of multi-thread access
		StValueChanger<bool> _change(mListChanging, true);

		long num_msgs = (GetMbox() ? GetMbox()->GetNumberMessages() : 0);

		TableIndexT	old_rows;
		TableIndexT	old_cols;
		GetTableSize(old_rows, old_cols);

		if (old_rows > num_msgs)
			RemoveRows(old_rows - num_msgs, 1, false);
		else if (old_rows < num_msgs)
			InsertRows(num_msgs - old_rows, 1, NULL, 0, false);

		// Previous selection no longer valid
		UnselectAllCells();
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
	// Prevent drawing to stop row delete causing immediate redraw and crash when scroll bar dissappears
	StDeferTableAdjustment changing(this);
	StDeferSelectionChanged noupdates(this);

	// Remove all rows then add all new ones
	RemoveAllRows(true);
}

void CMailboxTable::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch(cmd)
	{
	case CCommand::eFileOpenMessage:
	case CCommand::eFileSave:
	case CCommand::eToolbarFileSaveBtn:
	case CCommand::eFilePrint:
	case CCommand::eToolbarMessagePrintBtn:
		OnUpdateSelectionUndeleted(cmdui);
		return;

	case CCommand::eFileSaveAs:
		// Never
		OnUpdateNever(cmdui);
		return;

	case CCommand::eMessagesCopyTo:
	case CCommand::eMessagesCopyNow:
	case CCommand::eToolbarMessageCopyBtn:
	case CCommand::eCommandMessageCopy:
	case CCommand::eCommandMessageMove:
		OnUpdateMessageCopy(cmdui);
		return;

	case CCommand::eMailboxSortBy:
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eSortTo:
	case CCommand::eSortFrom:
	case CCommand::eSortReply:
	case CCommand::eSortSender:
	case CCommand::eSortCC:
	case CCommand::eSortSubject:
	case CCommand::eSortThread:
	case CCommand::eSortDateSent:
	case CCommand::eSortDateReceived:
	case CCommand::eSortSize:
	case CCommand::eSortFlags:
	case CCommand::eSortNumber:
	case CCommand::eSortSmartAddress:
	case CCommand::eSortAttachments:
	case CCommand::eSortParts:
	case CCommand::eSortMatching:
	case CCommand::eSortDisconnected:
		OnUpdateSortMenu(cmdui);
		return;

	case CCommand::eMailboxExpunge:
	case CCommand::eToolbarMailboxExpungeBtn:
		OnUpdateMailboxExpunge(cmdui);
		return;

	case CCommand::eMessagesDelete:
	case CCommand::eToolbarMessageDeleteBtn:
		OnUpdateMessageDelete(cmdui);
		return;

	case CCommand::eAddressesCaptureAddress:
		OnUpdateSelection(cmdui);
		return;

	default:;
	}

	if (cmdui->mMenu)
	{
		// Process copy to main and sub menus
		if ((dynamic_cast<CCopyToMenu::CCopyToMain*>(cmdui->mMenu) != NULL) ||
			(dynamic_cast<CCopyToMenu::CCopyToSub*>(cmdui->mMenu) != NULL))
		{
			OnUpdateMessageCopy(cmdui);
			return;
		}
	}

	CTableDragAndDrop::UpdateCommand(cmd, cmdui);
}

bool CMailboxTable::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch(cmd)
	{
	case CCommand::eFileOpenMessage:
		OnFileOpenMessage();
		return true;
	case CCommand::eFileSave:
	case CCommand::eToolbarFileSaveBtn:
		OnFileSave();
		return true;
	case CCommand::eFilePrint:
	case CCommand::eToolbarMessagePrintBtn:
		OnFilePrint();
		return true;

	case CCommand::eMailboxExpunge:
	case CCommand::eToolbarMailboxExpungeBtn:
		OnMailboxExpunge();
		return true;

	case CCommand::eSortTo:
	case CCommand::eSortFrom:
	case CCommand::eSortReply:
	case CCommand::eSortSender:
	case CCommand::eSortCC:
	case CCommand::eSortSubject:
	case CCommand::eSortThread:
	case CCommand::eSortDateSent:
	case CCommand::eSortDateReceived:
	case CCommand::eSortSize:
	case CCommand::eSortFlags:
	case CCommand::eSortNumber:
	case CCommand::eSortSmartAddress:
	case CCommand::eSortAttachments:
	case CCommand::eSortParts:
	case CCommand::eSortMatching:
	case CCommand::eSortDisconnected:
		OnSortItem(cmd - CCommand::eSortTo + 1);
		return true;

	case CCommand::eToolbarMessageCopyBtn:
	case CCommand::eToolbarMessageCopyPopup:
		// If there is a menu set the value from that now
		if (menu)
			mTableView->GetCopyBtn()->SetValue(menu->mIndex);
		OnMessageCopyPopup();
		return true;

	case CCommand::eMessagesCopyNow:
		OnMessageCopyNow();
		return true;

	case CCommand::eCommandMessageCopy:
		OnMessageCopyCmd();
		return true;

	case CCommand::eCommandMessageMove:
		OnMessageMoveCmd();
		return true;

	case CCommand::eMessagesDelete:
	case CCommand::eToolbarMessageDeleteBtn:
		OnMessageDelete();
		return true;

	case CCommand::eAddressesCaptureAddress:
		OnCaptureAddress();
		return true;
	}

	if (menu)
	{
		// Process copy to main and sub menus
		if ((dynamic_cast<CCopyToMenu::CCopyToMain*>(menu->mMenu) != NULL) ||
			(dynamic_cast<CCopyToMenu::CCopyToSub*>(menu->mMenu) != NULL))
		{
			OnMessageCopy(menu->mMenu, menu->mIndex);
			return true;
		}
	}

	return CTableDragAndDrop::ObeyCommand(cmd , menu);
}

void CMailboxTable::OnUpdateSelectionUndeleted(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mIsSelectionValid && (!mTestSelectionAndDeleted || CPreferences::sPrefs->mOpenDeleted.GetValue()));
}

void CMailboxTable::OnUpdateMailboxExpunge(CCmdUI* pCmdUI)
{
	// Must be some deleted
	pCmdUI->Enable(GetMbox() && GetMbox()->AnyDeleted() && !GetMbox()->IsReadOnly());
}

void CMailboxTable::OnUpdateSortMenu(CCmdUI* pCmdUI)
{
	// Always enabled
	pCmdUI->Enable(true);

	// Check if current sort method
	pCmdUI->SetCheck(GetMbox() && (GetMbox()->GetSortBy() == (int) pCmdUI->mMenuIndex));
}

void CMailboxTable::OnUpdateMessageCopy(CCmdUI* pCmdUI)
{
	// Force reset of mailbox menus - only done if required
	if (pCmdUI->mMenu)
		CCopyToMenu::ResetMenuList(pCmdUI->mMenu, NULL);

	// Do update for non-deleted selection
	bool enable = mIsSelectionValid && (!mTestSelectionAndDeleted || CPreferences::sPrefs->mOpenDeleted.GetValue());
	pCmdUI->Enable(enable);

	// Make sure we are listening to submenus of main copy to
	const JXMenu* submenu = NULL;
	if (pCmdUI->mMenu && pCmdUI->mMenu->GetSubmenu(pCmdUI->mMenuIndex, &submenu) &&
		(dynamic_cast<const CCopyToMenu::CCopyToSub*>(submenu) != NULL))
		ListenTo(submenu);

	// Adjust menu title move/copy
	cdstring txt;
	switch(pCmdUI->mCmd)
	{
	case CCommand::eMessagesCopyNow:
		txt.FromResource(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? IDS_MOVENOW_CMD_TEXT : IDS_COPYNOW_CMD_TEXT);
		break;
	case CCommand::eToolbarMessageCopyBtn:
		txt.FromResource(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? IDS_MOVETO_CMD_TEXT : IDS_COPYTO_CMD_TEXT);
		break;
	case CCommand::eMessagesCopyTo:
		txt.FromResource(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? IDS_MOVETOMAILBOX_CMD_TEXT : IDS_COPYTOMAILBOX_CMD_TEXT);
		break;
	default:;
		break;
	}

	if (!txt.empty())
		pCmdUI->SetText(txt);
}

void CMailboxTable::OnUpdateMessageDelete(CCmdUI* pCmdUI)
{
	cdstring txt;
	if (GetMbox() && !GetMbox()->HasAllowedFlag(NMessage::eDeleted))
	{
		pCmdUI->Enable(false);
		txt.FromResource(IDS_DELETE_CMD_TEXT);
	}
	else
	{
		pCmdUI->Enable(mIsSelectionValid);
		txt.FromResource(mTestSelectionAndDeleted ? IDS_UNDELETE_CMD_TEXT : IDS_DELETE_CMD_TEXT);
		if (!pCmdUI->mMenu)
			pCmdUI->SetCheck(mTestSelectionAndDeleted);
	}
	pCmdUI->SetText(txt);
}

void CMailboxTable::OnFileOpenMessage()
{
	DoFileOpenMessage(GetDisplay()->GetLatestKeyModifiers().control());
}

void CMailboxTable::OnFilePrint() 
{
	DoToSelection((DoToSelectionPP) &CMailboxTable::PrintMailMessage);
}

void CMailboxTable::OnMailboxExpunge()
{
	BeginMboxUpdate();
	
	try
	{
		DoMailboxExpunge(false);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		EndMboxUpdate(false);
	}

	EndMboxUpdate();

	Focus();
}

void CMailboxTable::OnSortDirection()
{
	if (!GetMbox())
		return;

	EShowMessageBy show = GetMbox()->GetShowBy();
	show = (EShowMessageBy) (3 - show);
	//mTableView->GetSortBtn()->SetPushed(show == cShowMessageDescending);
	mTableView->SetShowBy(show);
}

void CMailboxTable::OnSortItem(JIndex choice)
{
	mTableView->SetSortBy(static_cast<ESortMessageBy>(cSortMessageTo + choice - 1));
}

void CMailboxTable::OnMessageCopy(JXTextMenu* menu, JIndex choice)
{
	bool option_key = GetDisplay()->GetLatestKeyModifiers().control();

	// Fake copy to Choose item
	CMbox* copy_mbox = NULL;
	if (CCopyToMenu::GetMbox(menu, true, choice, copy_mbox))
	{
		if (copy_mbox)
			DoMessageCopy(copy_mbox, option_key);
		else
			CErrorHandler::PutStopAlertRsrc("Alerts::Message::MissingCopyTo");
	}
}

void CMailboxTable::OnMessageCopyPopup()
{
	bool option_key = GetDisplay()->GetLatestKeyModifiers().control();

	// Copy the message (make sure this window is not deleted as we're using it for the next message)
	CMbox* mbox = NULL;
	if (mTableView->GetCopyBtn() && mTableView->GetCopyBtn()->GetSelectedMbox(mbox))
	{
		if (mbox && (mbox != (CMbox*) -1L))
			DoMessageCopy(mbox, option_key);
		else if (!mbox)
			CErrorHandler::PutStopAlertRsrc("Alerts::Message::MissingCopyTo");
	}
}

void CMailboxTable::OnMessageCopyNow()
{
	bool option_key = GetDisplay()->GetLatestKeyModifiers().control();
	OnMessageCopyIt(option_key);
}

// Copy the message
void CMailboxTable::OnMessageCopyCmd()
{
	// Make sure an explicit copy (no delete after copy) is always done
	OnMessageCopyIt(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? true : false);
}

// Copy the message
void CMailboxTable::OnMessageMoveCmd()
{
	// Make sure an explicit move (delete after copy) is always done
	OnMessageCopyIt(CPreferences::sPrefs->deleteAfterCopy.GetValue() ? false : true);
}

void CMailboxTable::OnMessageCopyIt(bool option_key)
{
	// Get mailbox from toolbar button
	CMbox* mbox = NULL;
	if (mTableView->GetCopyBtn() && mTableView->GetCopyBtn()->GetSelectedMbox(mbox))
	{
		if (mbox && (mbox != (CMbox*) -1L))
			DoMessageCopy(mbox, option_key);
		else if (!mbox)
			CErrorHandler::PutStopAlertRsrc("Alerts::Message::MissingCopyTo");
	}
}

void CMailboxTable::OnMessageDelete()
{
	DoFlagMailMessage(NMessage::eDeleted);

	// Set focus back to table after button push
	Focus();
}

void CMailboxTable::OnCaptureAddress()
{
	DoCaptureAddress();
}

#pragma mark ____________________________________Drag&Drop

// Add selected mboxes to list
bool CMailboxTable::AddSelectionToDrag(TableIndexT row, CMessageList* list)
{
	if (!GetMbox())
		return false;

	// Determine message
	CMessage* theMsg = GetMbox()->GetMessage(row, true);
	if (!theMsg || theMsg->IsFake())
		return false;

	list->push_back(theMsg);
	return true;
}

bool CMailboxTable::RenderSelectionData(CMulSelectionData* seldata, Atom type)
{
	bool rendered = false;

	// Make list of selected messages
	std::auto_ptr<CMessageList> msgs(new CMessageList);
	msgs->SetOwnership(false);
	DoToSelection1((DoToSelection1PP) &CMailboxTable::AddSelectionToDrag, msgs.get());

	if (type == CMulberryApp::sFlavorMsgList)
	{
		seldata->SetData(type, reinterpret_cast<unsigned char*>(msgs.release()), sizeof(CMessageList*));
		rendered = true;
	}
	else if ((type == GetDisplay()->GetSelectionManager()->GetMimePlainTextXAtom()) ||
			 (type == GetDisplay()->GetSelectionManager()->GetTextXAtom()))
	{
		cdstring txt;
		
		// Add text of each message
		for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
		{
			// Check message size first
			if (!CMailControl::CheckSizeWarning(*iter))
				return false;

			// Read in message first
			CMailControl::BlockBusy(true);
			const char* data = NULL;
			try
			{
				data = (*iter)->ReadPart();
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
				txt += (*iter)->GetHeader();

			// Add message text to stream (include terminator)
			if (data)
				txt += data;

		}

		// Allocate global memory for the text if not already
		unsigned long dataLength = txt.length() + 1;
		unsigned char* data = new unsigned char[dataLength];
		if (data)
		{
			::memcpy(data, txt.c_str(), dataLength);
			seldata->SetData(type, data, dataLength);
			rendered = true;
		}
	}

	return rendered;
}

// Determine effect
Atom CMailboxTable::GetDNDAction(const JXContainer* target,
									 const JXButtonStates& buttonStates,
									 const JXKeyModifiers& modifiers)
{
	bool option_key = modifiers.control();
	Atom copy = GetDNDManager()->GetDNDActionCopyXAtom();
	Atom move = GetDNDManager()->GetDNDActionMoveXAtom();

	// Can only move if target is another mailbox or server
	bool allow_move = false;
	if (dynamic_cast<const CMailboxTable*>(target) ||
		dynamic_cast<const CServerTable*>(target) ||
		dynamic_cast<const CLetterTextEditView*>(target))
		allow_move = true;

	if (allow_move && (CPreferences::sPrefs->deleteAfterCopy.GetValue() ^ option_key))
		return move;
	else
		return copy;
}

// Drop data into table
bool CMailboxTable::DropData(Atom theFlavor, unsigned char* drag_data, unsigned long data_size)
{
	if (!GetMbox())
		return false;

	// Set flag
	sDropOnMailbox = true;

	if (theFlavor == CMulberryApp::sFlavorMsgList)
	{
		// Get list of chosen message nums
		ulvector nums;
		CMessageList* msgs = reinterpret_cast<CMessageList*>(drag_data);
		CMbox* mbox_from = NULL;
		for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
		{
			nums.push_back((*iter)->GetMessageNumber());
			mbox_from = (*iter)->GetMbox();
		}

		// Do mail message copy from mbox in drag to this mbox
		if (mbox_from && (nums.size() > 0))
		{
			try
			{
				// Do copy action - delete if drop was a move
				CActionManager::CopyMessage(mbox_from, GetMbox(), &nums, mDropActionMove);
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Make sure not signalled as mailbox to prevent delete after copy
				sDropOnMailbox = false;
				
				CLOG_LOGRETHROW;
				throw;
			}
		}
	}
	else if (theFlavor == CMulberryApp::sFlavorMboxList)
	{
		// Only handle the first one in the list
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		if (count)
		{
			CMbox* mbox = ((CMbox**) drag_data)[0];
			if (mbox)
				mTableView->Recycle(mbox);
		}
	}
	else if (theFlavor == CMulberryApp::sFlavorMboxRefList)
	{
		// Only handle the first one in the list
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		if (count)
		{
			CMboxRef* mboxref = ((CMboxRef**) drag_data)[0];
			CMbox* mbox = mboxref->ResolveMbox();
			if (mbox)
				mTableView->Recycle(mbox);
		}
	}
	
	return true;
}
