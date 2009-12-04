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

#include "CCommands.h"
#include "CFilterManager.h"
#include "CGeneralException.h"
#include "CIconLoader.h"
#include "CMulberryCommon.h"
#include "CNodeVectorTree.h"
#include "CPreferences.h"
#include "CRulesTable.h"
#include "CRulesWindow.h"
#include "CSimpleTitleTable.h"
#include "CStringUtils.h"
#include "CTitleTable.h"
#include "CTableRowGeometry.h"
#include "CTableRowSelector.h"

#include <UNX_LTableArrayStorage.h>

#include "HButtonText.h"

#include <JXColormap.h>
#include <JXDNDManager.h>
#include <JXImage.h>
#include "JXMultiImageButton.h"
#include <JPainter.h>
#include <JTableSelection.h>
#include <jASCIIConstants.h>
#include <jXKeysym.h>

// __________________________________________________________________________________________________
// C L A S S __ C G R O U P T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CScriptsTable::CScriptsTable(JXScrollbarSet* scrollbarSet, 
								JXContainer* enclosure,
								const HSizingOption hSizing, 
								const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h)
	: CHierarchyTableDrag(scrollbarSet, enclosure, hSizing, vSizing, x,y,w,h)
{
	SetBorderWidth(0);

	InitScriptsTable();
}

// Default destructor
CScriptsTable::~CScriptsTable()
{
}

// Common init
void CScriptsTable::InitScriptsTable(void)
{
	// Create storage
	mTableGeometry = new CTableRowGeometry(this, 128, 18);
	mTableSelector = new CTableRowSelector(this);
	mTableStorage = new LTableArrayStorage(this, sizeof(void*));

	mType = CFilterItem::eLocal;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CScriptsTable::OnCreate()
{
	// Find window in super view chain
	mWindow = dynamic_cast<CRulesWindow*>(GetWindow()->GetDirector());

	CHierarchyTableDrag::OnCreate();

	// Get keys but not Tab which is used to shift focus
	WantInput(kTrue);

	AddDragFlavor(CMulberryApp::sFlavorRuleType);
	AddDropFlavor(CMulberryApp::sFlavorRuleType);

	// Set read only status of Drag and Drop
	SetReadOnly(false);
	SetDropCell(true);
	SetDropCursor(false);
	SetAllowDrag(false);
	SetAllowMove(false);
	SetSelfDrag(false);

	// Create columns
	InsertCols(1, 1);
	SetColWidth(GetApertureWidth(), 1, 1);

	// Context menu
	//CreateContextMenu(CMainMenu::eContextAdbkTable);
}

void CScriptsTable::SetFilterType(CFilterItem::EType type)
{
	mType = type;

	if (mType == CFilterItem::eLocal)
		mWindow->mScriptsTitleTable->SetTitleInfo(false, false, "UI::Titles::Targets", 1, 0);
	else
		mWindow->mScriptsTitleTable->SetTitleInfo(false, false, "UI::Titles::Scripts", 1, 0);

	ResetTable();
}

//	Respond to commands
bool CScriptsTable::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eRulesNew:
		OnNewScript();
		return true;

	case CCommand::eRulesEdit:
		OnEditScript();
		return true;

	case CCommand::eRulesDelete:
		OnDeleteScript();
		return true;

	case CCommand::eRulesWriteScript:
		OnGenerateScript();
		return true;

	default:;
	}

	return CHierarchyTableDrag::ObeyCommand(cmd, menu);
}

//	Pass back status of a (menu) command
void CScriptsTable::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eRulesNew:
		OnUpdateAlways(cmdui);
		return;

	// These ones must have a selection
	case CCommand::eRulesEdit:
	case CCommand::eRulesDelete:
	case CCommand::eRulesWriteScript:
		OnUpdateSelection(cmdui);
		return;

	default:;
	}

	CHierarchyTableDrag::UpdateCommand(cmd, cmdui);
}

// Handle key presses
bool CScriptsTable::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	switch (key)
	{
	// Edit the address
	case kJReturnKey:
		OnEditScript();
		return true;

	// Delete
	case kJDeleteKey:
	case kJForwardDeleteKey:
		OnDeleteScript();
		return true;

	default:
		return CHierarchyTableDrag::HandleChar(key, modifiers);
	}
}

// Click in the cell
void CScriptsTable::LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	OnEditScript();
}

// Draw the titles
void CScriptsTable::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
	unsigned long bkgnd = GetCellBackground(inCell);

	TableIndexT woRow = GetWideOpenIndex(inCell.row);
	UInt32 nestingLevel = GetNestingLevel(woRow);
	bool is_script = nestingLevel == 0;
	CFilterScript* script = NULL;
	CFilterItem* filter = NULL;

	STableCell	woCell(woRow, inCell.col);
	UInt32 dataSize = sizeof(void*);
	GetCellData(woCell, is_script ? (void*) &script : (void*) &filter, dataSize);

	ResIDT iconID;
	if (mType == CFilterItem::eLocal)
		iconID = is_script ? IDI_TARGETSICON : IDI_RULESICON;
	else
		iconID = is_script ? IDI_SCRIPTSICON : IDI_RULESICON;
	JXImage* icon = CIconLoader::GetIcon(iconID, this, 16, bkgnd);
	cdstring content = is_script ? script->GetName() : filter->GetName();

	// Use italic for disabled scripts
	if (script && !script->IsEnabled())
	{
		// Set appropriate font & color
		JFontStyle style = pDC->GetFontStyle();
		style.italic = kTrue;
		pDC->SetFontStyle(style);
	}

	DrawHierarchyRow(pDC, inCell.row, inLocalRect, content, icon);
}

// Make sure columns are set
void CScriptsTable::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	// Do inherited call
	CHierarchyTableDrag::ApertureResized(dw, dh);

	//Since only the name column has variable width, we just
	//adjust it.
	if (mCols)
	{
		JCoordinate cw = GetApertureWidth();
		if (cw < 32)
			cw = 32;
		SetColWidth(cw, 1, 1);
		
		if ((mWindow != NULL) && (mWindow->mScriptsTitleTable != NULL))
			mWindow->mScriptsTitleTable->SyncTable(this, false);
	}
}

// Update delete buttons
void CScriptsTable::UpdateButtons(void)
{
	if (IsSelectionValid())
	{
		mWindow->mEditBtn->Activate();
		mWindow->mDeleteBtn->Activate();
		
		if ((mType == CFilterItem::eSIEVE) &&
			IsSingleSelection() &&
			TestSelectionOr((TestSelectionPP) &CScriptsTable::TestSelectionScript))
			mWindow->mWriteBtn->Activate();
		else
			mWindow->mWriteBtn->Deactivate();
	}
	else
	{
		mWindow->mEditBtn->Deactivate();
		mWindow->mDeleteBtn->Deactivate();
		mWindow->mWriteBtn->Deactivate();
	}
}

// Keep cached list in sync
void CScriptsTable::ResetTable(void)
{
	// Delete all existing rows
	RemoveAllRows(false);

	// Add all scripts
	UInt32 last_grp_row = 0;
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

	Refresh();
	UpdateButtons();
}

// Determine effect
Atom CScriptsTable::GetDNDAction(const JXContainer* target,
									 const JXButtonStates& buttonStates,
									 const JXKeyModifiers& modifiers)
{
	// Always a copy action
	JXDNDManager* dndMgr = GetDNDManager();
	return dndMgr->GetDNDActionCopyXAtom();
}

// Test drag insert cursor
bool CScriptsTable::IsDropCell(JArray<Atom>& typeList, const STableCell& cell)
{
	TableIndexT woRow = GetWideOpenIndex(cell.row);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);
	return IsValidCell(cell) && (nestingLevel == 0);
}

// Test drop at cell
bool CScriptsTable::IsDropAtCell(JArray<Atom>& typeList, STableCell& cell)
{
	TableIndexT woRow = GetWideOpenIndex(cell.row);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);
	return IsValidCell(cell) && (nestingLevel == 1);
}

// Drop data into cell
bool CScriptsTable::DropDataIntoCell(Atom theFlavor, unsigned char* drag_data,
										unsigned long data_size, const STableCell& cell)
{
	bool added = false;

	// Get its info
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(cell.row);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);
	bool is_script = (nestingLevel == 0);
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

// Drop data at cell
bool CScriptsTable::DropDataAtCell(Atom theFlavor, unsigned char* drag_data,
										unsigned long data_size, const STableCell& cell)
{
	return false;
}
