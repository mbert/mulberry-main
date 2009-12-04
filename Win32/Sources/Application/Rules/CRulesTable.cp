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


// Source for CRulesTable class

#include "CRulesTable.h"

#include "CDrawUtils.h"
#include "CFilterManager.h"
#include "CFontCache.h"
#include "CIconLoader.h"
#include "CMailboxInfoTable.h"
#include "CMailboxWindow.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CRulesWindow.h"
#include "CSMTPWindow.h"
#include "CTableRowSelector.h"
#include "CTableRowGeometry.h"

const short cTextIndent = 18;

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

BEGIN_MESSAGE_MAP(CRulesTable, CTableDragAndDrop)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)

	ON_COMMAND(IDC_RULESNEWBTN, OnNewRules)
	ON_COMMAND(IDC_RULESEDITBTN, OnEditRules)
	ON_COMMAND(IDC_RULESDELETEBTN, OnDeleteRules)
	ON_COMMAND(IDC_RULESAPPLYBTN, OnApplyRules)

	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CRulesTable::CRulesTable()
{
	mTableGeometry = new CTableRowGeometry(this, 128, 18);
	mTableSelector = new CTableRowSelector(this);

	SetRowSelect(true);
	
	mType = CFilterItem::eLocal;
}

// Default destructor
CRulesTable::~CRulesTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int CRulesTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTableDragAndDrop::OnCreate(lpCreateStruct) == -1)
		return -1;

	CWnd* parent = GetParent();
	while(parent && !dynamic_cast<CRulesWindow*>(parent))
		parent = parent->GetParent();
	mWindow = dynamic_cast<CRulesWindow*>(parent);

	AddDropFlavor(CMulberryApp::sFlavorRuleType);
	AddDragFlavor(CMulberryApp::sFlavorRuleType);

	SetReadOnly(false);
	SetDropCell(false);
	SetDropCursor(true);
	SetAllowDrag(true);
	SetSelfDrag(true);
	SetExternalDrag(false);

	// Create columns
	InsertCols(1, 1);

	return 0;
}

// Resize columns
void CRulesTable::OnSize(UINT nType, int cx, int cy)
{
	CTableDragAndDrop::OnSize(nType, cx, cy);

	// Adjust for vert scroll bar
	if (!(GetStyle() & WS_VSCROLL))
		cx -= 16;

	// Icon column: fixed width. Name column: variable
	if (cy)
		SetColWidth(cx, 1, 1);
}

// Tell window to focus on this one
void CRulesTable::OnSetFocus(CWnd* pOldWnd)
{
	CTableDragAndDrop::OnSetFocus(pOldWnd);
	mWindow->FocusRules();
	UpdateButtons();
}

// Keep titles in sync
void CRulesTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh)
{
	// Find titles in owner chain
	mWindow->mRulesTitles.ScrollImageBy(inLeftDelta, 0, inRefresh);

	CTableDragAndDrop::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
}

void CRulesTable::SetFilterType(CFilterItem::EType type)
{
	mType = type;

	ResetTable();
}

// Handle key presses
bool CRulesTable::HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	// Edit the rule
	case VK_RETURN:
		OnEditRules();
		break;

	// Delete
	case VK_BACK:
	case VK_DELETE:
		OnDeleteRules();
		break;

	default:
		return CTableDragAndDrop::HandleKeyDown(nChar, nRepCnt, nFlags);
	}

	return true;
}

// Click in the cell
void CRulesTable::LClickCell(const STableCell& inCell, UINT nFlags)
{
	// Try drag and drop
	if (mAllowDrag)
	{
		if (DoDrag(inCell.row))
			return;

		// Active if mouse activate not done
		GetParentFrame()->ActivateFrame();
		SetFocus();
	}
}

// Double-clicked item
void CRulesTable::LDblClickCell(const STableCell& inCell, UINT nFlags)
{
	OnEditRules();
}

// Draw the titles
void CRulesTable::DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect)
{
	// Save text & color state in stack objects
	StDCState save(pDC);

	// Draw selection
	DrawCellSelection(pDC, inCell);

	// Get its rule
	CFilterItem* item = CPreferences::sPrefs->GetFilterManager()->GetFilters(mType).at(inCell.row - TABLE_START_INDEX);

	// Determine which heading it is
	SColumnInfo col_info = (*mColumnInfo)[inCell.col - 1];
	cdstring theTxt;

	CIconLoader::DrawIcon(pDC, inLocalRect.left, inLocalRect.top + mIconOrigin, IDI_RULESICON, 16);

	// Get name of item
	theTxt = item->GetName();

	// Use italic for unused rules
	if (!CPreferences::sPrefs->GetFilterManager()->RuleUsed(item))
		pDC->SelectObject(CFontCache::GetListFontItalic());

	// Draw the string
	int x = inLocalRect.left + cTextIndent;
	int y = inLocalRect.top + mTextOrigin;
	::DrawClippedStringUTF8(pDC, theTxt, CPoint(x, y), inLocalRect, eDrawString_Left);
}

// Update delete buttons
void CRulesTable::UpdateButtons(void)
{
	mWindow->mEditBtn.EnableWindow(IsSelectionValid());
	mWindow->mDeleteBtn.EnableWindow(IsSelectionValid());
}

// Reset the table from the mboxList
void CRulesTable::ResetTable(void)
{
	// Add cached mailboxes
	TableIndexT num_rules = CPreferences::sPrefs->GetFilterManager()->GetFilters(mType).size();
	TableIndexT old_rows = mRows;

	if (old_rows > num_rules)
		RemoveRows(old_rows - num_rules, 1, false);
	else if (old_rows < num_rules)
		InsertRows(num_rules - old_rows, 1, NULL, 0, false);

	// Refresh list
	RedrawWindow(nil, nil, RDW_INVALIDATE);
	UpdateButtons();

} // CRulesTable::ResetTable

#pragma mark ____________________________Drag & Drop

void CRulesTable::GetSelection(ulvector& selection) const
{
	STableCell aCell(0, 0);
	while(GetNextSelectedCell(aCell))
	{
		if (aCell.col == 1)
			selection.push_back(aCell.row - 1);
	}
}

// Add selected mboxes to list
bool CRulesTable::AddSelectionToDrag(TableIndexT row, CFilterItems* list)
{
	// Get its rule
	CFilterItem* item = CPreferences::sPrefs->GetFilterManager()->GetFilters(mType).at(row - TABLE_START_INDEX);
	list->push_back(item);
	return true;
}

// Send data to target
BOOL CRulesTable::OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal)
{
	BOOL rendered = false;

	// Make list of selected messages
	CFilterItems items;
	DoToSelection1((DoToSelection1PP) &CRulesTable::AddSelectionToDrag, &items);

	if  (lpFormatEtc->cfFormat == CMulberryApp::sFlavorRuleType)
	{
		// Allocate global memory for the text if not already
		if (!*phGlobal)
			*phGlobal = ::GlobalAlloc(GMEM_DDESHARE, items.size() * sizeof(CFilterItem*) + sizeof(int));
		if (*phGlobal)
		{
			// Copy to global after lock
			CFilterItem** pFilter = (CFilterItem**) ::GlobalLock(*phGlobal);
			*((int*) pFilter) = items.size();
			pFilter += sizeof(int);
			for(CFilterItems::iterator iter = items.begin(); iter != items.end(); iter++)
				*pFilter++ = *iter;
			::GlobalUnlock(*phGlobal);
			
			rendered = true;
		}
	}
	
	return rendered;
}

// Drop data at cell
bool CRulesTable::DropDataAtCell(unsigned int theFlavor, char* drag_data,
											unsigned long data_size, const STableCell& cell)
{
	// Clear drag accumulation objects
	mDragged.clear();
	mDropRow = 0;

	// Just add drag items to accumulator
	GetSelection(mDragged);
	mDropRow = cell.row - 1;

	// Move rules to drop location
	CPreferences::sPrefs->GetFilterManager()->MoveRules(mDragged, mDropRow, mType);

	// Refresh list
	UnselectAllCells();
	RedrawWindow(nil, nil, RDW_INVALIDATE);
	UpdateButtons();
	
	return true;
}
