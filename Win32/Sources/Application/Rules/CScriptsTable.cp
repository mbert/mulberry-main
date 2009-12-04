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


// Source for CScriptsTable class


#include "CScriptsTable.h"

#include "CDrawUtils.h"
#include "CFilterManager.h"
#include "CFontCache.h"
#include "CMulberryCommon.h"
#include "CNodeVectorTree.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CRulesWindow.h"
#include "CTableRowGeometry.h"
#include "CTableRowSelector.h"

#include <WIN_LTableArrayStorage.h>

/////////////////////////////////////////////////////////////////////////////
// CScriptsTable

IMPLEMENT_DYNCREATE(CScriptsTable, CHierarchyTableDrag)

BEGIN_MESSAGE_MAP(CScriptsTable, CHierarchyTableDrag)
	ON_COMMAND(IDC_RULESNEWTARGETBTN, OnNewScript)
	ON_COMMAND(IDC_RULESNEWSCRIPTSBTN, OnNewScript)
	ON_COMMAND(IDC_RULESEDITBTN, OnEditScript)
	ON_COMMAND(IDC_RULESDELETEBTN, OnDeleteScript)
	ON_COMMAND(IDC_RULESGENERATEBTN, OnGenerateScript)

	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CScriptsTable::CScriptsTable()
{
	mTableGeometry = new CTableRowGeometry(this, 128, 18);
	mTableSelector = new CTableRowSelector(this);
	mTableStorage = new LTableArrayStorage(this, sizeof(void*));

	SetRowSelect(true);

	mType = CFilterItem::eLocal;
}

// Default destructor
CScriptsTable::~CScriptsTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int CScriptsTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CHierarchyTableDrag::OnCreate(lpCreateStruct) == -1)
		return -1;

	CWnd* parent = GetParent();
	while(parent && !dynamic_cast<CRulesWindow*>(parent))
		parent = parent->GetParent();
	mWindow = dynamic_cast<CRulesWindow*>(parent);

	// Set Drag & Drop info
	AddDragFlavor(CMulberryApp::sFlavorRuleType);
	AddDropFlavor(CMulberryApp::sFlavorRuleType);

	SetReadOnly(false);
	SetDropCell(true);
	SetAllowDrag(true);
	SetSelfDrag(true);
	SetAllowMove(false);

	// Create columns
	InsertCols(1, 1);

	return 0;
}

// Resize columns
void CScriptsTable::OnSize(UINT nType, int cx, int cy)
{
	CHierarchyTableDrag::OnSize(nType, cx, cy);

	// Adjust for vert scroll bar
	if (!(GetStyle() & WS_VSCROLL))
		cx -= 16;

	// Single column = whole width
	if (cy)
		SetColWidth(cx, 1, 1);
}

// Tell window to focus on this one
void CScriptsTable::OnSetFocus(CWnd* pOldWnd)
{
	CHierarchyTableDrag::OnSetFocus(pOldWnd);
	mWindow->FocusScripts();
	UpdateButtons();
}

void CScriptsTable::SetFilterType(CFilterItem::EType type)
{
	mType = type;

	if (mType == CFilterItem::eLocal)
		mWindow->mScriptsTitles.SetTitleInfo(false, false, "UI::Titles::Targets", 1, 0);
	else
		mWindow->mScriptsTitles.SetTitleInfo(false, false, "UI::Titles::Scripts", 1, 0);

	ResetTable();
}

// Update delete buttons
void CScriptsTable::UpdateButtons(void)
{
	mWindow->mEditBtn.EnableWindow(IsSelectionValid());
	mWindow->mDeleteBtn.EnableWindow(IsSelectionValid());

	if ((mType == CFilterItem::eSIEVE) &&
		IsSingleSelection() &&
		TestSelectionOr((TestSelectionPP) &CScriptsTable::TestSelectionScript))
		mWindow->mGenerateBtn.EnableWindow(true);
	else
		mWindow->mGenerateBtn.EnableWindow(false);
}

// Keep cached list in sync
void CScriptsTable::ResetTable(void)
{
	// Prevent window update during changes
	StDeferTableAdjustment changing(this);

	// Delete all existing rows
	RemoveAllRows(false);

	// Add all scripts
	short last_grp_row = 0;
	if (mType == CFilterItem::eLocal)
	{
		for(CTargetItemList::iterator iter1 = CPreferences::sPrefs->GetFilterManager()->GetTargets(mType).begin();
				iter1 != CPreferences::sPrefs->GetFilterManager()->GetTargets(mType).end(); iter1++)
		{
			// Insert script item
			last_grp_row = InsertSiblingRows(1, last_grp_row, &(*iter1), sizeof(CFilterScript*), true, false);

			// Add all rules in script
			for(CFilterItemList::iterator iter2 = (*iter1)->GetFilters().begin(); iter2 != (*iter1)->GetFilters().end(); iter2++)
				AddLastChildRow(last_grp_row, &(*iter2), sizeof(CFilterItem*), false, false);
		}
	}
	else
	{
		for(CFilterScriptList::iterator iter1 = CPreferences::sPrefs->GetFilterManager()->GetScripts(mType).begin();
				iter1 != CPreferences::sPrefs->GetFilterManager()->GetScripts(mType).end(); iter1++)
		{
			// Insert script item
			last_grp_row = InsertSiblingRows(1, last_grp_row, &(*iter1), sizeof(CFilterScript*), true, false);

			// Add all rules in script
			for(CFilterItemList::iterator iter2 = (*iter1)->GetFilters().begin(); iter2 != (*iter1)->GetFilters().end(); iter2++)
				AddLastChildRow(last_grp_row, &(*iter2), sizeof(CFilterItem*), false, false);
		}
	}

	// Collapse all rows
	TableIndexT collapse_row = 0;
	while(collapse_row < mRows)
	{
		TableIndexT	woRow;
		woRow = mCollapsableTree->GetWideOpenIndex(collapse_row);
		if (mCollapsableTree->IsCollapsable(woRow))
			DeepCollapseRow(woRow);
		collapse_row++;
	}

	UpdateButtons();
	RedrawWindow();
}

#pragma mark ____________________________________Keyboard/Mouse

// Click in the cell
void CScriptsTable::LClickCell(const STableCell& inCell, UINT nFlags)
{
	CHierarchyTableDrag::LClickCell(inCell, nFlags);
}

// Double-clicked item
void CScriptsTable::LDblClickCell(const STableCell& inCell, UINT nFlags)
{
	OnEditScript();
}

// Handle key down
bool CScriptsTable::HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_RETURN:
		OnEditScript();
		break;

	case VK_BACK:
	case VK_DELETE:
		OnDeleteScript();
		break;

	case VK_TAB:
		//mWindow->FocusRules();
		mWindow->SetFocus();
		break;

	default:
		// Did not handle key
		return CHierarchyTableDrag::HandleKeyDown(nChar, nRepCnt, nFlags);
	}

	// Handled key
	return true;
}

// Draw the titles
void CScriptsTable::DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect)
{
	StDCState save(pDC);

	TableIndexT woRow = GetWideOpenIndex(inCell.row);
	UInt32 nestingLevel = GetNestingLevel(woRow);
	bool is_script = nestingLevel == 0;
	CFilterScript* script = NULL;
	CFilterItem* filter = NULL;

	STableCell	woCell(woRow, inCell.col);
	UInt32 dataSize = sizeof(void*);
	GetCellData(woCell, is_script ? (void*) &script : (void*) &filter, dataSize);

	UINT iconID;
	if (mType == CFilterItem::eLocal)
		iconID = is_script ? IDI_TARGETSICON : IDI_RULESICON;
	else
		iconID = is_script ? IDI_SCRIPTSICON : IDI_RULESICON;
	cdstring content = is_script ? script->GetName() : filter->GetName();

	// Use italic for disabled scripts
	if (script && !script->IsEnabled())
		pDC->SelectObject(CFontCache::GetListFontItalic());

	DrawHierarchyRow(pDC, inCell.row, inLocalRect, content, iconID);
}

#pragma mark ____________________________________Drag&Drop

// Determine if drop into cell
bool CScriptsTable::IsDropCell(COleDataObject* pDataObject, const STableCell& cell)
{
	TableIndexT woRow = GetWideOpenIndex(cell.row);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);
	return IsValidCell(cell) && (nestingLevel == 0);
}

// Test drop at cell
bool CScriptsTable::IsDropAtCell(COleDataObject* pDataObject, STableCell& cell)
{
	TableIndexT woRow = GetWideOpenIndex(cell.row);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);
	return IsValidCell(cell) && (nestingLevel == 1);
}

// Determine effect
DROPEFFECT CScriptsTable::GetDropEffect(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	// Always copy
	return DROPEFFECT_COPY;
}

// Drop data into cell
bool CScriptsTable::DropDataIntoCell(unsigned int theFlavor, char* drag_data,
											unsigned long data_size, const STableCell& cell)
{
	bool added = false;

	// Get its info
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(cell.row);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);
	bool is_script = nestingLevel == 0;
	CFilterScript* script = NULL;

	// Only if script
	if (!is_script)
	{
		CLOG_LOGTHROW(CGeneralException, -1L);
		throw CGeneralException(-1L);
	}

	// Get data item
	STableCell	woCell(woRow, cell.col);
	UInt32 dataSize = sizeof(CFilterScript*);
	GetCellData(woCell, &script, dataSize);

	if (theFlavor == CMulberryApp::sFlavorRuleType)
	{
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		for(int i = 0; i < count; i++)
		{
			CFilterItem* filter = ((CFilterItem**) drag_data)[i];
			if (script->AddFilter(filter))
			{
				AddLastChildRow(woRow, &filter, sizeof(CFilterItem*), false, true);
				added = true;
			}
		}
	}
	
	RefreshRow(cell.row);

	return added;
}

bool CScriptsTable::DropDataAtCell(unsigned int theFlavor, char* drag_data, unsigned long data_size, const STableCell& cell)
{
	return false;
}