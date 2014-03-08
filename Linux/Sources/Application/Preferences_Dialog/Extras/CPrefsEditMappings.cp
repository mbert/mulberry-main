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


// CPrefsEditMappings.cpp : implementation file
//


#include "CPrefsEditMappings.h"

#include "CEditMapDialog.h"
#include "CMulberryCommon.h"
#include "CTableScrollbarSet.h"
#include "CSimpleTitleTable.h"
#include "CTableRowSelector.h"
#include "CXStringResources.h"

#include "StPenState.h"

#include <UNX_LTableMultiGeometry.h>

#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <algorithm>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CPrefsEditMappings dialog


CPrefsEditMappings::CPrefsEditMappings(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

const int cTitleHeight = 16;

void CPrefsEditMappings::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 350,240, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 350,240);
    assert( obj1 != NULL );

    mOkBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 260,205, 70,25);
    assert( mOkBtn != NULL );
    mOkBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 170,205, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    CTableScrollbarSet* sbs =
        new CTableScrollbarSet(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 330,150);
    assert( sbs != NULL );

    mAddBtn =
        new JXTextButton("Add...", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,165, 60,20);
    assert( mAddBtn != NULL );

    mChangeBtn =
        new JXTextButton("Change...", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,165, 60,20);
    assert( mChangeBtn != NULL );

    mDeleteBtn =
        new JXTextButton("Delete", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,165, 60,20);
    assert( mDeleteBtn != NULL );

// end JXLayout

	CSimpleTitleTable* titles = new CSimpleTitleTable(sbs, sbs->GetScrollEnclosure(),
																	 JXWidget::kHElastic,
																	 JXWidget::kFixedTop,
																	 0, 0, 330, cTitleHeight);

	mItsTable = new CEditMappingsTable(sbs,sbs->GetScrollEnclosure(),
										JXWidget::kHElastic,
										JXWidget::kVElastic,
										0,cTitleHeight, 330, 150);
	mItsTable->OnCreate();
	titles->OnCreate();

	titles->SyncTable(mItsTable, true);
	titles->LoadTitles("UI::Titles::MIMETypeMap", 3);

	window->SetTitle("Explicit MIME Mappings");
	SetButtons(mOkBtn, mCancelBtn);

	ListenTo(mAddBtn);
	ListenTo(mChangeBtn);
	ListenTo(mDeleteBtn);
}

void CPrefsEditMappings::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mAddBtn)
		{
			OnAddMap();
			return;
		}
		else if (sender == mChangeBtn)
		{
			OnChangeMap();
			return;
		}
		else if (sender == mDeleteBtn)
		{
			OnDeleteMap();
			return;
		}
	}

	CDialogDirector::Receive(sender, message);
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsEditMappings message handlers

void CPrefsEditMappings::OnAddMap() 
{
	mItsTable->NewMap();
}

void CPrefsEditMappings::OnChangeMap() 
{
	mItsTable->EditMap();
}

void CPrefsEditMappings::OnDeleteMap() 
{
	mItsTable->DeleteMap();
}

bool CPrefsEditMappings::PoseDialog(CMIMEMapVector* theMap)
{
	bool result = false;

	CPrefsEditMappings* dlog = new CPrefsEditMappings(JXGetApplication());
	dlog->OnCreate();
	dlog->mItsTable->SetMap(theMap);

	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		result = true;
		dlog->Close();
	}

	return result;
}

// __________________________________________________________________________________________________
// C L A S S __ C R E P L Y C H O O S E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CEditMappingsTable::CEditMappingsTable(
								 JXScrollbarSet* scrollbarSet,
								 JXContainer* enclosure,
								 const HSizingOption hSizing,
								 const VSizingOption vSizing,
								 const JCoordinate x, const JCoordinate y,
								 const JCoordinate w, const JCoordinate h)
	: CTable(scrollbarSet, enclosure, hSizing, vSizing, x,y,w,h)
{
	mTableGeometry = new LTableMultiGeometry(this, 128, 20);
	mTableSelector = new CTableSingleRowSelector(this);
	mItsMap = NULL;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CEditMappingsTable::OnCreate()
{
	// Create columns
	InsertCols(3, 1);
	SetColWidth(80, 2, 2);
	SetColWidth(64, 3, 3);
	SetColWidth(GetApertureWidth() - 160, 1, 1);
}

void CEditMappingsTable::SetMap(CMIMEMapVector* theMap)
{
	// Local copy
	mItsMap = theMap;
	
	// Insert rows
	InsertRows(mItsMap->size(), 1);
}

void CEditMappingsTable::NewMap(void)
{
	// Create new map
	CMIMEMap new_map;
	if (CEditMapDialog::PoseDialog(new_map))
	{
		CMIMEMapVector::iterator iter = lower_bound(mItsMap->begin(), mItsMap->end(), new_map);
		int pos = (iter - mItsMap->begin())/sizeof(CMIMEMap);
		mItsMap->insert(iter, new_map);
		InsertRows(1, 1);
	}
}

void CEditMappingsTable::EditMap(void)
{
	STableCell selCell;
	if (GetNextSelectedCell(selCell))
	{
		CMIMEMap change_map = (*mItsMap)[selCell.row - 1];

		if (CEditMapDialog::PoseDialog(change_map))
		{
			(*mItsMap)[selCell.row - 1] = change_map;
			sort(mItsMap->begin(), mItsMap->end());
			Refresh();
		}
	}
}

void CEditMappingsTable::DeleteMap(void)
{
	DeleteSelection();
}

// Remove rows
void CEditMappingsTable::RemoveRows(UInt32 inHowMany, TableIndexT inFromRow, bool inRefresh)
{
	CMIMEMapVector::iterator start_erase = (*mItsMap).begin() + (inFromRow - 1);
	CMIMEMapVector::iterator stop_erase = start_erase + inHowMany;
	(*mItsMap).erase(start_erase, stop_erase);
    CTable::RemoveRows(inHowMany, inFromRow, inRefresh);
}

// Draw a cell
void CEditMappingsTable::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
	StPenState save(pDC);

	// Draw selection
	DrawCellSelection(pDC, inCell);

	// Get data
	const CMIMEMap& item = (*mItsMap)[inCell.row - 1];
	cdstring theTxt;

	switch(inCell.col)
	{

	case 1:
		// Write MIME type/subtype
		theTxt = item.GetMIMEType();
		theTxt += "/";
		theTxt += item.GetMIMESubtype();
		::DrawClippedStringUTF8(pDC, theTxt, JPoint(inLocalRect.left + 4, inLocalRect.top), inLocalRect, eDrawString_Left);
		break;

	case 2:
		// Write address
		::DrawClippedStringUTF8(pDC, item.GetFileSuffix(), JPoint(inLocalRect.left + 4, inLocalRect.top), inLocalRect, eDrawString_Left);
		break;

	case 3:
		// Write address
		theTxt = rsrc::GetIndexedString("UI::Preferences::MIMETypeMap", item.GetAppLaunch());
		::DrawClippedStringUTF8(pDC, theTxt, JPoint(inLocalRect.left + 4, inLocalRect.top), inLocalRect, eDrawString_Left);
		break;


	default:
		break;
	}

}

// Click in the cell
void CEditMappingsTable::LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	EditMap();
}
