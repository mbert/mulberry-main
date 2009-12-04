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

#include "CAdminLock.h"
#include "CAttachment.h"
#include "CAttachmentList.h"
#include "CDesktopIcons.h"
#include "CDrawUtils.h"
#include "CErrorHandler.h"
#include "CIconLoader.h"
#include "CMailControl.h"
#include "CMessage.h"
#include "CMessagePartProp.h"
#include "CMessageView.h"
#include "CMessageWindow.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CTableRowSelector.h"
#include "CTableRowGeometry.h"

#include <WIN_LTableArrayStorage.h>

IMPLEMENT_DYNCREATE(CBodyTable, CHierarchyTableDrag)

BEGIN_MESSAGE_MAP(CBodyTable, CHierarchyTableDrag)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateNever)				// Do not allow delete: this is read-only

	ON_UPDATE_COMMAND_UI(IDM_EDIT_PROPERTIES, OnUpdateSelection)
	ON_COMMAND(IDM_EDIT_PROPERTIES, OnEditProperties)

	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_VIEW_PARTS, OnUpdateExtractParts)
	ON_UPDATE_COMMAND_UI(IDM_MESSAGES_EXTRACT_PARTS, OnUpdateExtractParts)
	ON_COMMAND(IDM_MESSAGES_VIEW_PARTS, OnMessageViewParts)
	ON_COMMAND(IDM_MESSAGES_EXTRACT_PARTS, OnMessageExtractParts)

	ON_WM_CREATE()
	ON_WM_SIZE()

	// Toolbar
	ON_UPDATE_COMMAND_UI(IDC_TOOLBARDETAILSBTN, OnUpdateSelection)
	ON_COMMAND(IDC_TOOLBARDETAILSBTN, OnEditProperties)

END_MESSAGE_MAP()

CBodyTable::CBodyTable()
{
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

int CBodyTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CHierarchyTableDrag::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Drag and drop
	AddDragFlavor(CMulberryApp::sFlavorMsgAtchList);

	SetReadOnly(true);
	SetDropCell(false);
	SetAllowDrag(true);
	SetSelfDrag(false);
	SetAllowMove(false);

	// Create columns and adjust flag rect
	InsertCols(5, 1);

	// Find window in super view chain
	CWnd* parent = GetParent();
	while(parent && !dynamic_cast<CMessageWindow*>(parent))
		parent = parent->GetParent();
	mWindow = dynamic_cast<CMessageWindow*>(parent);

	// Find view in super view chain
	parent = GetParent();
	while(parent && !dynamic_cast<CMessageView*>(parent))
		parent = parent->GetParent();
	mView = dynamic_cast<CMessageView*>(parent);

	
	return 0;
}

// Resize columns
void CBodyTable::OnSize(UINT nType, int cx, int cy)
{
	CHierarchyTableDrag::OnSize(nType, cx, cy);

	// Adjust for vert scroll bar
	int adjust_cx = 0;
	if (!(GetStyle() & WS_VSCROLL))
	{
		cx -= 16;
		adjust_cx = 16;
	}

	if (cy)
	{
		// Name column has variable width
		SetColWidth(cx - 318, 5, 5);
		
		// Remaining columns have fixed width
		SetColWidth(16, 1, 1);
		SetColWidth(16, 2, 2);
		SetColWidth(238, 3, 3);
		SetColWidth(48, 4, 4);
		
		if (mTitles != NULL)
		{
			// Name column has variable width
			mTitles->SetColWidth(cx - 318 + adjust_cx, 5, 5);
			
			// Remaining columns have fixed width
			mTitles->SetColWidth(16, 1, 1);
			mTitles->SetColWidth(16, 2, 2);
			mTitles->SetColWidth(238, 3, 3);
			mTitles->SetColWidth(48, 4, 4);
		}
	}
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
	for(int woRow = 1; woRow <= rows; woRow++)
	{
		STableCell	woCell(woRow, 1);
		CAttachment* row_attach = NULL;
		UInt32 dataSize = sizeof(CAttachment*);
		GetCellData(woCell, &row_attach, dataSize);
		
		if (attach == row_attach)
		{
			mRowShow = woRow;
			RedrawWindow();
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
	// Prevent window update during changes
	StDeferTableAdjustment changing(this);

	// Delete all existing rows
	Clear();

	// May not have a body
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

// Test for selected cached attachments
bool CBodyTable::TestSelectionCached(TableIndexT row)
{
	// Is it cached?
	CAttachment* attach = GetAttachment(row);
	if (!attach)
		return false;

	return !attach->IsNotCached();
}

#pragma mark ____________________________________Keyboard/Mouse

// Handle key down
bool CBodyTable::HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_RETURN:
		// Try to show it
		if (IsSelectionValid() &&
			(!IsSingleSelection() || !ShowPart(GetFirstSelectedRow())) &&
			TestSelectionOr((TestSelectionPP) &CBodyTable::TestSelectionCached))
			// Just extract it
			ExtractViewParts(CPreferences::sPrefs->mViewDoubleClick.GetValue());
		break;

	case VK_TAB:
		DoTab();
		break;

	default:
		// Did not handle key
		return CHierarchyTableDrag::HandleKeyDown(nChar, nRepCnt, nFlags);
	}

	// Handled key
	return true;
}

// Double-clicked item
void CBodyTable::LDblClickCell(const STableCell& inCell, UINT nFlags)
{
	ExtractViewParts(CPreferences::sPrefs->mViewDoubleClick.GetValue());
}

// Click in the cell
void CBodyTable::LClickCell(const STableCell& inCell, UINT nFlags)
{
	if (inCell.col == 1)
		ShowPart(inCell.row);
	else
		CHierarchyTableDrag::LClickCell(inCell, nFlags);
}

#pragma mark ____________________________________Command Handlers

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

			// Create dialog
			CMessagePartProp dlog(CSDIFrame::GetAppTopWindow());
		
			dlog.SetFields(*attach, multi);
		
			// Let DialogHandler process events
			more = (dlog.DoModal() == IDOK);
		}

	} while (more);
	
	SetFocus();
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
	// Special hack to cope with command routing problems
	if (IsTarget())
	{
		ExtractViewParts(view);
		return;
	}

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
	if (!CMailControl::CheckSizeWarning(attach))
		return false;

	// Find window in super view chain and extract part if cached
	if (!attach->IsNotCached())
	{
		if (mWindow)
			mWindow->ExtractPart(attach, *view);
		else
			mView->ExtractPart(attach, *view);
	}

	return true;
}

// Click in the cell
bool CBodyTable::ShowPart(TableIndexT row)
{
	// Do click in different text show
	UInt32	woRow = GetWideOpenIndex(row);
	
	CAttachment* attach = GetAttachment(row);
	if (!attach)
		return false;

	if ((woRow != mRowShow) && attach->CanDisplay())
	{
		BOOL do_show = true;

		// Check message size first
		if (!CMailControl::CheckSizeWarning(attach))
			do_show = false;

		if (do_show)
		{
			// Refresh old one to force row show off
			TableIndexT old_row = GetExposedIndex(mRowShow);
			mRowShow = woRow;
			RefreshRow(old_row);

			// Find window in super view chain and show new part
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
		// Find window in super view chain and show sub message
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

// Draw the items
void CBodyTable::DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect)
{
	if (!GetBody())
		return;

	CAttachment* attach = GetAttachment(inCell.row);
	if (!attach)
		return;

	StDCState save(pDC);

	UInt32	woRow = GetWideOpenIndex(inCell.row);

	switch(inCell.col)
	{
	
	case 1:
		// Button if not multipart
		if (!attach->IsMultipart())
		{
			UINT	plot_flag = 0;

			// Check for diamond type
			if (woRow == mRowShow)
				plot_flag = IDI_DIAMONDTICKED;
			else if (attach->CanDisplay())
				plot_flag = (attach->IsSeen() ? IDI_DIAMONDHIGHLIGHT : IDI_DIAMOND);
			else if (attach->IsMessage())
				plot_flag = (attach->IsSeen() ? IDI_SQUAREHIGHLIGHT : IDI_SQUARE);
			else
				plot_flag = IDI_DIAMONDDISABLE;
			CIconLoader::DrawIcon(pDC, inLocalRect.left, inLocalRect.top + mIconOrigin, plot_flag, 16);
		}
		break;

	case 2:	
		// Check for tick
		if (attach->IsExtracted() && (attach->IsApplefile() || (!attach->IsMultipart() && !attach->IsMessage())))
			CIconLoader::DrawIcon(pDC, inLocalRect.left, inLocalRect.top + mIconOrigin, IDI_TICKMARK, 16);
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
			HICON hIcon = GetAttachIcon(pDC, do_attach);
				
			// Get encoding mode
			{
				cdstring content = CMIMESupport::GenerateContentHeader(do_attach, false, lendl, false);
				DrawHierarchyRow(pDC, inCell.row, inLocalRect, content, hIcon);
			}
		}
		break;
	
	case 4:
		// Draw size
		if (!attach->IsMultipart() || mFlat)
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
			::DrawClippedStringUTF8(pDC, theTxt, CPoint(inLocalRect.left, inLocalRect.top + mTextOrigin), inLocalRect, eDrawString_Right);
		}
		break;
	
	case 5:
		if (!attach->IsMultipart() && !attach->IsMessage())
		{
			cdstring at_name = attach->GetMappedName(true, true);
			if (!at_name.empty())
			{
				cdstring name(at_name);
				::DrawClippedStringUTF8(pDC, name, CPoint(inLocalRect.left, inLocalRect.top + mTextOrigin), inLocalRect, eDrawString_Left);
			}
		}
		else if (attach->IsMultipart() && attach->IsApplefile())
		{
			cdstring name = attach->GetMappedName(true, true);
			::DrawClippedStringUTF8(pDC, name, CPoint(inLocalRect.left, inLocalRect.top + mTextOrigin), inLocalRect, eDrawString_Left);
		}
		else if (attach->IsMessage())
		{
			if (!attach->GetMessage()->GetEnvelope()->GetSubject().empty())
			{
				cdstring name(attach->GetMessage()->GetEnvelope()->GetSubject());
				::DrawClippedStringUTF8(pDC, name, CPoint(inLocalRect.left, inLocalRect.top + mTextOrigin), inLocalRect, eDrawString_Left);
			}
		}
		break;
	
	default:
		break;
	}
}

// Plot appropriate icon for attachment
HICON CBodyTable::GetAttachIcon(CDC* pDC, CAttachment* attach)
{
	UINT nID = 0;
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
	else if (attach->GetIconRef()->GetIconRef())
		return attach->GetIconRef()->GetIconRef();
	else
		nID = IDI_UNKNOWNFILE;
	
	return CIconLoader::GetIcon(pDC, nID, 16);
}

#pragma mark ____________________________________Drag&Drop

// Prevent drag if improper selection
BOOL CBodyTable::DoDrag(TableIndexT row)
{
	// Must have some cached attachments
	return TestSelectionOr((TestSelectionPP) &CBodyTable::TestSelectionCached) &&
				CHierarchyTableDrag::DoDrag(row);
}

// Send data to target
BOOL CBodyTable::OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal)
{
	BOOL rendered = false;

	if  (lpFormatEtc->cfFormat == CMulberryApp::sFlavorMsgAtchList)
	{
		// Create list to hold deleted items
		CAttachmentList attchs;

		// Add each selected address
		DoToSelection1((DoToSelection1PP) &CBodyTable::AddEntryToList, &attchs);
		
		// Allocate global memory for the text if not already
		if (!*phGlobal)
			*phGlobal = ::GlobalAlloc(GMEM_DDESHARE, attchs.size() * sizeof(CAttachment*) + sizeof(int) + sizeof(CMessage*));
		if (*phGlobal)
		{
			// Copy to global after lock
			CAttachment** pAddr = (CAttachment**) ::GlobalLock(*phGlobal);
			CWnd* msgWnd = GetParent()->GetParent()->GetParent();
			*((CMessage**) pAddr) = (mWindow ? mWindow->GetMessage() : mView->GetMessage());
			pAddr += sizeof(CMessage*);
			*((int*) pAddr) = attchs.size();
			pAddr += sizeof(int);
			for(CAttachmentList::iterator iter = attchs.begin(); iter != attchs.end(); iter++)
				*pAddr++ = *iter;
			//::memcpy(pAddr, attchs.begin(), attchs.size() * sizeof(CAttachment*));
			::GlobalUnlock(*phGlobal);
			
			rendered = true;
		}
		
		// Do not delete originals
		attchs.clear();
	}
	
	return rendered;
}

