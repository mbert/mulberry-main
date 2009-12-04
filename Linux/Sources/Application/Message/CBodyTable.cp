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

#include <jASCIIConstants.h>
#include <jXKeysym.h>

#include "CAdminLock.h"
#include "CAttachment.h"
#include "CAttachmentList.h"
#include "CCommands.h"
#include "CErrorHandler.h"
#include "CIconLoader.h"
#include "CMessage.h"
#include "CMessagePartProp.h"
#include "CMessageView.h"
#include "CMessageWindow.h"
#include "CMIMESupport.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CMulSelectionData.h"
#include "CPreferences.h"
#include "CSimpleTitleTable.h"
#include "CTableRowSelector.h"
#include "CTableRowGeometry.h"

#include <UNX_LTableArrayStorage.h>

#include <JTableSelection.h>
#include <JXImage.h>
#include <JPainter.h>
#include <JXWindow.h>

#include <stdio.h>

CBodyTable::CBodyTable(JXScrollbarSet* scrollbarSet, 
						JXContainer* enclosure,
						const HSizingOption hSizing, 
						const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h)
	:CHierarchyTableDrag(scrollbarSet, enclosure, hSizing, vSizing, x,y,w,h)
{
	SetBorderWidth(0);

	mTableGeometry = new CTableRowGeometry(this, 128, 18);
	mTableSelector = new CTableRowSelector(this);
	mTableStorage = new LTableArrayStorage(this, sizeof(CAttachment*));

	SetRowSelect(true);

	mBody = NULL;
	mWindow = NULL;
	mView = NULL;
	mTitles = NULL;
	mRowShow = 0;
	mFlat = true;
	mHierarchyCol = 3;
}


CBodyTable::~CBodyTable()
{
}

void CBodyTable::OnCreate()//LPCREATESTRUCT lpCreateStruct)
{
	mWindow = dynamic_cast<CMessageWindow*>(GetWindow()->GetDirector());

	const JXContainer* parent = GetEnclosure();
	while(parent && !dynamic_cast<const CMessageView*>(parent))
		parent = parent->GetEnclosure();
	mView = const_cast<CMessageView*>(dynamic_cast<const CMessageView*>(parent));

	LHierarchyTable::OnCreate();

	// Get keys but not Tab which is used to shift focus
	WantInput(kTrue);

	// Create columns and adjust flag rect
	InsertCols(5, 1);

	//We want the actual width
	JCoordinate cx = GetApertureWidth();
	// Name column has variable width
	SetColWidth(cx > 318 ? cx - 318 : 32, 5, 5);
	// Remaining columns have fixed width
	SetColWidth(16, 1, 1);
	SetColWidth(16, 2, 2);
	SetColWidth(238, 3, 3);
	SetColWidth(48, 4, 4);

	// Context menu
	CreateContextMenu(CMainMenu::eContextMessageParts);

	// Set Drag & Drop info
#ifndef LINUX_NOTYET
	AddDragFlavor(flavorTypeHFS);
#endif
	AddDragFlavor(CMulberryApp::sFlavorMsgAtchList);

	// Set read only status of Drag and Drop
	SetReadOnly(true);
	SetAllowDrag(true);
}

// Resize columns
void CBodyTable::ApertureResized(const JCoordinate dw,
				 const JCoordinate dh)
{
	CHierarchyTableDrag::ApertureResized(dw, dh);

	//Since only the name column has variable width, we just
	//adjust it.
	// NB Can be called before columns exist
	if (mCols)
	{
		JCoordinate cw = GetApertureWidth() - 318;
		if (cw < 32)
			cw = 32;
		SetColWidth(cw, 5, 5);
		
		if (mTitles != NULL)
			mTitles->SyncTable(this, false);
	}
}

//#pragma mark ____________________________________Keyboard/Mouse

// Test for selected cached attachments
bool CBodyTable::TestSelectionCached(TableIndexT row)
{
	// Is it cached?
	CAttachment* attach = GetAttachment(row);
	if (!attach)
		return false;

	return !attach->IsNotCached();
}

// Handle key down
bool CBodyTable::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	switch (key)
	{
	case kJReturnKey:
		// Try to show it
		if (IsSelectionValid() &&
			(!IsSingleSelection() || !ShowPart(GetFirstSelectedRow())) &&
			TestSelectionOr((TestSelectionPP) &CBodyTable::TestSelectionCached))
			// Just extract it
			ExtractViewParts(CPreferences::sPrefs->mViewDoubleClick.GetValue());
		return true;

	default:
		// Did not handle key
		return CHierarchyTableDrag::HandleChar(key, modifiers);
	}
}

void CBodyTable::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eEditProperties:
		OnUpdateSelection(cmdui);
		return;
	case CCommand::eMessagesViewParts:
	case CCommand::eMessagesExtractParts:
		OnUpdateExtractParts(cmdui);
		return;
	default:;
	}

	CHierarchyTableDrag::UpdateCommand(cmd, cmdui);
}

bool CBodyTable::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eEditProperties:
		OnEditProperties();
		return true;

	case CCommand::eMessagesViewParts:
		OnMessageViewParts();
		return true;

	case CCommand::eMessagesExtractParts:
		OnMessageExtractParts();
		return true;
	}

	return CHierarchyTableDrag::ObeyCommand(cmd, menu);
}

// Double-clicked item
void CBodyTable::LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	ExtractViewParts(CPreferences::sPrefs->mViewDoubleClick.GetValue());
}

// Click in the cell
void CBodyTable::LClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	if (inCell.col == 1)
		ShowPart(inCell.row);
	else
		CHierarchyTableDrag::LClickCell(inCell, modifiers);
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
		cdstring s;
		mTitles->ChangeTitle(3, s.FromResource(mFlat ? "UI::Message::PartsContentFlat" : "UI::Message::PartsContentHierarchic"));

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
	GetWideOpenTableSize(rows);

	// Look at each cell
	for(TableIndexT woRow = 1; woRow <= rows; woRow++)
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

// Reset the table from the mbox
void CBodyTable::ResetTable()
{
	// Delete all existing rows
	Clear();

	// Might not have a body
	if (!GetBody())
		return;

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

// Add address to list
bool CBodyTable::AddEntryToList(TableIndexT row, CAttachmentList* list)
{
	// Get selected address
	CAttachment* attach = GetAttachment(row);
	if (!attach)
		return false;

	if (list->DuplicateItem(attach))
		return false;
	else
	{
		list->push_back(attach);
		return true;
	}
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

//#pragma mark ____________________________________Command Handlers

bool CBodyTable::ShowPart(TableIndexT row)
{
	// Do click in different text show
	TableIndexT	woRow = GetWideOpenIndex(row);
	
	CAttachment* attach = GetAttachment(row);
	if (!attach)
		return false;

	if ((woRow != mRowShow) && attach->CanDisplay())
	{
		bool do_show = true;

		// Check message size first
		if (CPreferences::sPrefs->mDoSizeWarn.GetValue() && (CPreferences::sPrefs->warnMessageSize.GetValue() > 0) &&
			(attach->GetSize() > CPreferences::sPrefs->warnMessageSize.GetValue() * 1024UL))
		{
			bool dontshow = false;
			short answer = CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Message::WarnSize", 0, &dontshow);
			if (dontshow)
				CPreferences::sPrefs->mDoSizeWarn.SetValue(false);
			if (answer == CErrorHandler::Cancel)
				do_show = false;
		}

		if (do_show)
		{
			// Refresh old one to force row show off
			TableIndexT old_row = GetExposedIndex(mRowShow);
			mRowShow = woRow;
			RefreshRow(old_row);

			if (mWindow)
				mWindow->ShowPart(attach);
			else
				mView->ShowPart(attach);

			// Refresh new one
			RefreshRow(row);
		}
		return do_show;
	}
	
	// Check for sub-message display
	else if (attach->IsMessage())
	{
		if (mWindow)
			mWindow->ShowSubMessage(attach);
		else
			mView->ShowSubMessage(attach);

		// Refresh current one
		RefreshRow(row);

		return true;
	}
	
	return (woRow == mRowShow);
}

void CBodyTable::OnUpdateExtractParts(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TestSelectionOr((TestSelectionPP) &CBodyTable::TestSelectionCached));
}

void CBodyTable::OnEditProperties()
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

			more = CMessagePartProp::PoseDialog(*attach, multi);
		}

	} while (more);
}

void CBodyTable::OnMessageViewParts()
{
	ExtractViewParts(true);
}

void CBodyTable::OnMessageExtractParts()
{
	ExtractViewParts(false);
}

void CBodyTable::ExtractViewParts(bool view)
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

void CBodyTable::DoExtractCurrentPart(bool view)
{
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
	CAttachment* attach = GetAttachment(row, *worow);
	if (!attach)
		return false;

	// Check against admin locks	
	return CAdminLock::sAdminLock.CanDownload(attach->GetContent());
	
}

// Extract the part
bool CBodyTable::ExtractPart(TableIndexT row, bool* view, bool* worow)
{
	CAttachment* attach = GetAttachment(row, *worow);
	if (!attach)
		return false;

	// Check part size first
	if (CPreferences::sPrefs->mDoSizeWarn.GetValue() && (CPreferences::sPrefs->warnMessageSize.GetValue() > 0) &&
		(attach->GetSize() > CPreferences::sPrefs->warnMessageSize.GetValue() * 1024UL))
	{
		bool dontshow = false;
		short answer = CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Message::WarnSize", 0, &dontshow);
		if (dontshow)
			CPreferences::sPrefs->mDoSizeWarn.SetValue(false);
		if (answer == CErrorHandler::Cancel)
			return false;
	}

	// Extract part if cached
	if (!attach->IsNotCached())
	{
		if (mWindow)
			mWindow->ExtractPart(attach, *view);
		else
			mView->ExtractPart(attach, *view);
	}		

	return true;
}

// Draw the items
void CBodyTable::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
	if (!GetBody())
		return;

	CAttachment* attach = GetAttachment(inCell.row);
	if (!attach)
		return;

	UInt32	woRow = GetWideOpenIndex(inCell.row);

	switch(inCell.col)
	{
	
	case 1:
		// Button if not multipart
		if (!attach->IsMultipart())
		{
			int	plot_flag = 0;

			// Check for diamond type
			if (woRow == mRowShow)
				plot_flag = IDI_DIAMONDTICKED;
			else if (attach->CanDisplay())
				plot_flag = (attach->IsSeen() ? IDI_DIAMONDHIGHLIGHT : IDI_DIAMOND);
			else if (attach->IsMessage())
				plot_flag = (attach->IsSeen() ? IDI_SQUAREHIGHLIGHT : IDI_SQUARE);
			else
				plot_flag = IDI_DIAMONDDISABLE;
				
			JXImage* icon = CIconLoader::GetIcon(plot_flag, this, 16, 0x00FFFFFF);
			pDC->Image(*icon, icon->GetBounds(), inLocalRect.left, inLocalRect.top + mIconOrigin);
		}
		break;

	case 2:	
		// Check for tick
		if (attach->IsExtracted() && (attach->IsApplefile() || (!attach->IsMultipart() && !attach->IsMessage())))
		{
			JXImage* icon = CIconLoader::GetIcon(IDI_TICKMARK, this, 16, 0x00FFFFFF);
			pDC->Image(*icon, icon->GetBounds(), inLocalRect.left, inLocalRect.top + mIconOrigin);
		}
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

			// Draw icon followed by encoding
			JXImage* icon = GetAttachIcon(do_attach);
				
			// Get encoding mode
			{
				cdstring content = CMIMESupport::GenerateContentHeader(do_attach, false, lendl, false);
				DrawHierarchyRow(pDC, inCell.row, inLocalRect, content, icon);
			}
		}
		break;

	case 4:
		// Draw size
		if (!attach->IsMultipart())
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
			cdstring theTxt = ::GetNumericFormat(size);
			::DrawClippedStringUTF8(pDC, theTxt.c_str(), JPoint(inLocalRect.left, inLocalRect.top), inLocalRect, eDrawString_Right);
		}
		break;
	
	case 5:
		if (!attach->IsMultipart() && !attach->IsMessage())
		{
			cdstring name = attach->GetMappedName(true, true);
			if (!name.empty())
				::DrawClippedStringUTF8(pDC, name.c_str(), JPoint(inLocalRect.left, inLocalRect.top), inLocalRect, eDrawString_Left);
		}
		else if (attach->IsMultipart() && attach->IsApplefile())
		{
			cdstring name = attach->GetMappedName(true, true);
			::DrawClippedStringUTF8(pDC, name.c_str(), JPoint(inLocalRect.left, inLocalRect.top), inLocalRect, eDrawString_Left);
		}
		else if (attach->IsMessage())
		{
			if (!attach->GetMessage()->GetEnvelope()->GetSubject().empty())
			{
				cdstring name(attach->GetMessage()->GetEnvelope()->GetSubject());
				::DrawClippedStringUTF8(pDC, name.c_str(), JPoint(inLocalRect.left, inLocalRect.top), inLocalRect, eDrawString_Left);
			}
		}
		break;
	
	default:
		break;
	}
}

JXImage* CBodyTable::GetAttachIcon(CAttachment* attach)
{
	unsigned int nID = 0;
	if (attach->IsNotCached())
		nID = IDI_MISSINGPART;
	if (attach->IsApplefile())
		nID = IDI_APPLEFILEPART;
	else if (attach->IsMultipart())
		nID = IDI_MULTIPART;
	else if (attach->IsMessage())
		nID = IDI_MESSAGEPART;
	else if (attach->IsCalendar())
		nID = IDI_CALENDARPART;
	//else if (attach->GetIconRef()->GetIconRef())
	//	return attach->GetIconRef()->GetIconRef();
	else
		nID = IDI_UNKNOWNFILE;
	
	return CIconLoader::GetIcon(nID, this, 16, 0x00FFFFFF);
}

//#pragma mark ____________________________________Drag&Drop

// Send data to target
bool CBodyTable::RenderSelectionData(CMulSelectionData* seldata, Atom type)
{
	bool rendered = false;

	if (type == CMulberryApp::sFlavorMsgAtchList)
	{
		// Create list to hold deleted items
		CAttachmentList attchs;

		// Add each selected address
		DoToSelection1((DoToSelection1PP) &CBodyTable::AddEntryToList, &attchs);
		
		// Allocate global memory for the text if not already
		unsigned long dataLength = attchs.size() * sizeof(CAttachment*) + sizeof(int) + sizeof(CMessage*);
		unsigned char* data = new unsigned char[dataLength];
		if (data)
		{
			// Copy to global after lock
			CAttachment** pAddr = reinterpret_cast<CAttachment**>(data);
			*((CMessage**) pAddr) = (mWindow ? mWindow->GetMessage() : mView->GetMessage());
			pAddr += sizeof(CMessage*);
			*((int*) pAddr) = attchs.size();
			pAddr += sizeof(int);
			for(CAttachmentList::iterator iter = attchs.begin(); iter != attchs.end(); iter++)
				*pAddr++ = *iter;
			
			seldata->SetData(type, data, dataLength);
			rendered = true;
		}
		
		// Do not delete originals
		attchs.clear();
	}
	
	return rendered;
}
