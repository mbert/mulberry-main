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

#include "CCommands.h"
#include "CFilterManager.h"
#include "CIconLoader.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CMulSelectionData.h"
#include "CPreferences.h"
#include "CRulesWindow.h"
#include "CSimpleTitleTable.h"
#include "CTableRowSelector.h"
#include "CTableRowGeometry.h"

#include "HButtonText.h"

#include <JXColormap.h>
#include <JXImage.h>
#include "JXMultiImageButton.h"
#include <JXTextMenu.h>
#include <JPainter.h>
#include <JTableSelection.h>
#include <jASCIIConstants.h>
#include <jXKeysym.h>

#include <stdio.h>
#include <string.h>

const short cTextIndent = 18;

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CRulesTable::CRulesTable(JXScrollbarSet* scrollbarSet, 
							 JXContainer* enclosure,
							 const HSizingOption hSizing, 
							 const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h)
	: CTableDragAndDrop(scrollbarSet, enclosure, hSizing, vSizing, x,y,w,h)
{
	SetBorderWidth(0);

	InitRulesTable();
}

// Default destructor
CRulesTable::~CRulesTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CRulesTable::InitRulesTable(void)
{
	mTableGeometry = new CTableRowGeometry(this, 128, 18);
	mTableSelector = new CTableRowSelector(this);

	SetRowSelect(true);
	
	mType = CFilterItem::eLocal;
}

// Get details of sub-panes
void CRulesTable::OnCreate()
{
	// Find window in super view chain
	mWindow = dynamic_cast<CRulesWindow*>(GetWindow()->GetDirector());

	CTableDragAndDrop::OnCreate();

	// Get keys but not Tab which is used to shift focus
	WantInput(kTrue);

	AddDropFlavor(CMulberryApp::sFlavorRulePosType);
	AddDragFlavor(CMulberryApp::sFlavorRulePosType);
	AddDragFlavor(CMulberryApp::sFlavorRuleType);

	SetReadOnly(false);
	SetDropCell(false);
	SetDropCursor(true);
	SetAllowDrag(true);
	SetSelfDrag(true);

	// Create columns
	InsertCols(1, 1);
	SetColWidth(GetApertureWidth(), 1, 1);

	// Context menu
	//CreateContextMenu(CMainMenu::eContextAdbkTable);
}

// Keep titles in sync
void CRulesTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh)
{
	// Find titles in owner chain
	mWindow->mRulesTitleTable->ScrollImageBy(inLeftDelta, 0, inRefresh);

	CTableDragAndDrop::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
}

void CRulesTable::SetFilterType(CFilterItem::EType type)
{
	mType = type;
	ResetTable();
}

// Handle key presses
bool CRulesTable::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	switch(key)
	{
	// Edit the rule
	case kJReturnKey:
		OnEditRules();
		return true;

	// Delete
	case kJDeleteKey:
	case kJForwardDeleteKey:
		OnDeleteRules();
		return true;

	default:
		return CTableDragAndDrop::HandleChar(key, modifiers);
	}
}

//	Respond to commands
bool CRulesTable::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eRulesNew:
		OnNewRules();
		return true;

	case CCommand::eRulesEdit:
		OnEditRules();
		return true;

	case CCommand::eRulesDelete:
		OnDeleteRules();
		return true;

	default:;
	}

	return CTableDragAndDrop::ObeyCommand(cmd, menu);
}

//	Pass back status of a (menu) command
void CRulesTable::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eRulesNew:
		OnUpdateAlways(cmdui);
		return;

	// These ones must have a selection
	case CCommand::eRulesEdit:
	case CCommand::eRulesDelete:
		OnUpdateSelection(cmdui);
		return;

	default:;
	}

	CTableDragAndDrop::UpdateCommand(cmd, cmdui);
}

// Click in the cell
void CRulesTable::LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	OnEditRules();
}

// Draw the titles
void CRulesTable::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
	// Draw selection
	DrawCellSelection(pDC, inCell);

	unsigned long bkgnd = GetCellBackground(inCell);

	// Get its rule
	CFilterItem* item = CPreferences::sPrefs->GetFilterManager()->GetFilters(mType).at(inCell.row - 1);

	// Determine which heading it is and draw it
	JCoordinate left = inLocalRect.left;
	JXImage* icon = CIconLoader::GetIcon(IDI_RULESICON, this, 16, bkgnd);
	pDC->Image(*icon, icon->GetBounds(), inLocalRect.left, inLocalRect.top + mIconOrigin);
	left += 18;

	// Get name of item
	cdstring theTxt = item->GetName();

	// Use italic for unused rules
	if (!CPreferences::sPrefs->GetFilterManager()->RuleUsed(item))
	{
		// Set appropriate font & color
		JFontStyle style = pDC->GetFontStyle();
		style.italic = kTrue;
		pDC->SetFontStyle(style);
	}

	// Draw the string
	::DrawClippedStringUTF8(pDC, theTxt, JPoint(left, inLocalRect.top), inLocalRect, eDrawString_Left);
}

// Resize columns
void CRulesTable::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	CTableDragAndDrop::ApertureResized(dw, dh);

	//Since only the name column has variable width, we just
	//adjust it.
	if (mCols)
	{
		JCoordinate cw = GetApertureWidth();
		if (cw < 32)
			cw = 32;
		SetColWidth(cw, 1, 1);
		
		if ((mWindow != NULL) && (mWindow->mRulesTitleTable != NULL))
			mWindow->mRulesTitleTable->SyncTable(this, false);
	}
}

// Update delete buttons
void CRulesTable::UpdateButtons(void)
{
	if (IsSelectionValid())
	{
		mWindow->mEditBtn->Activate();
		mWindow->mDeleteBtn->Activate();
	}
	else
	{
		mWindow->mEditBtn->Deactivate();
		mWindow->mDeleteBtn->Deactivate();
	}
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

	// Previous selection no longer valid
	UnselectAllCells();

	// Refresh list
	Refresh();
	UpdateButtons();

}

#pragma mark ____________________________Drag & Drop

// Add selected mboxes to list
bool CRulesTable::AddSelectionToDrag(TableIndexT row, CFilterItems* list)
{
	// Get its rule
	CFilterItem* item = CPreferences::sPrefs->GetFilterManager()->GetFilters(mType).at(row - TABLE_START_INDEX);
	list->push_back(item);
	return true;
}

bool CRulesTable::RenderSelectionData(CMulSelectionData* seldata, Atom type)
{
	bool rendered = false;

	// Check for different types
	if (type == CMulberryApp::sFlavorRulePosType)
	{
		// Just add count as the selection
		unsigned long dataLength = sizeof(int);
		unsigned char* data = new unsigned char[dataLength];
		ulvector selected;
		GetSelectedRows(selected);
		*((int*) data) = selected.size();

		seldata->SetData(type, data, dataLength);
		rendered = true;
	}
	else if (type == CMulberryApp::sFlavorRuleType)
	{
		// Make list of selected messages
		CFilterItems items;
		DoToSelection1((DoToSelection1PP) &CRulesTable::AddSelectionToDrag, &items);

		int count = items.size();

		// Allocate global memory for the count
		unsigned long dataLength = count * sizeof(CFilterItem*) + sizeof(int);
		unsigned char* data = new unsigned char[dataLength];
		if (data)
		{
			CFilterItem** i = reinterpret_cast<CFilterItem**>(data);
			*((int*) i) = count;
			i += sizeof(int);
			for(CFilterItems::iterator iter = items.begin(); iter != items.end(); iter++)
				*i++ = *iter;

			seldata->SetData(type, data, dataLength);
			rendered = true;
		}
	}

	return rendered;
}

// Drop data at cell
bool CRulesTable::DropDataAtCell(Atom theFlavor,
										unsigned char* drag_data,
										unsigned long data_size, const STableCell& cell)
{
	// Clear drag accumulation object
	ulvector dragged;
	int drop_row = cell.row - 1;

	// Just add drag items to accumulator - use zero based index
	GetSelectedRows(dragged);
	for(ulvector::iterator iter = dragged.begin(); iter != dragged.end(); iter++)
		*iter -= 1;

	// Move rules to drop location
	CPreferences::sPrefs->GetFilterManager()->MoveRules(dragged, drop_row, mType);

	// Refresh list
	Refresh();
	UpdateButtons();

	return true;
}
