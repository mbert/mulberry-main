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


// Source for CMacroEditDialog class

#include "CMacroEditDialog.h"

#include "CEditMacro.h"
#include "CErrorHandler.h"
#include "CKeyChoiceDialog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTableScrollbarSet.h"
#include "CSimpleTitleTable.h"

#include "StPenState.h"

#include <JXDownRect.h>
#include <JXEngravedRect.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMacroEditDialog::CMacroEditDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
	mTable = NULL;
}

// Default destructor
CMacroEditDialog::~CMacroEditDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

const int cTitleHeight = 16;

// Get details of sub-panes
void CMacroEditDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 400,310, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 400,310);
    assert( obj1 != NULL );

    CTableScrollbarSet* sbs =
        new CTableScrollbarSet(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 380,150);
    assert( sbs != NULL );

    mNewBtn =
        new JXTextButton("New...", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 25,165, 70,20);
    assert( mNewBtn != NULL );
    mNewBtn->SetFontSize(10);

    mEditBtn =
        new JXTextButton("Edit...", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 95,165, 70,20);
    assert( mEditBtn != NULL );
    mEditBtn->SetFontSize(10);

    mDeleteBtn =
        new JXTextButton("Delete", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 165,165, 70,20);
    assert( mDeleteBtn != NULL );
    mDeleteBtn->SetFontSize(10);

    JXEngravedRect* obj2 =
        new JXEngravedRect(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,205, 380,55);
    assert( obj2 != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Trigger Macro Expansion with Key:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,195, 205,15);
    assert( obj3 != NULL );

    JXDownRect* obj4 =
        new JXDownRect(obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,13, 185,25);
    assert( obj4 != NULL );

    mKey =
        new JXStaticText("", obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,2, 175,20);
    assert( mKey != NULL );

    mChooseBtn =
        new JXTextButton("Choose...", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 205,15, 90,25);
    assert( mChooseBtn != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 220,275, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 310,275, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

// end JXLayout

	CSimpleTitleTable* titles = new CSimpleTitleTable(sbs, sbs->GetScrollEnclosure(),
																	 JXWidget::kHElastic,
																	 JXWidget::kFixedTop,
																	 0, 0, 550, cTitleHeight);
	mTable = new CMacroEditTable(sbs,sbs->GetScrollEnclosure(),
										JXWidget::kHElastic,
										JXWidget::kVElastic,
										0,cTitleHeight, 550, 100);

	mTable->OnCreate();
	titles->OnCreate();

	mTable->SetSelectionMsg(true);
	mTable->SetDoubleClickMsg(true);
	ListenTo(mTable);

	titles->LoadTitles("UI::Titles::MacroEdit", 2);
	titles->SyncTable(mTable, true);

	ListenTo(mNewBtn);
	ListenTo(mEditBtn);
	ListenTo(mDeleteBtn);
	ListenTo(mChooseBtn);

	window->SetTitle("Edit Macros");
	SetButtons(mOKBtn, mCancelBtn);
}

void CMacroEditDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mNewBtn)
		{
			OnNewMacro();
			return;
		}
		else if (sender == mEditBtn)
		{
			OnEditMacro();
			return;
		}
		else if (sender == mDeleteBtn)
		{
			OnDeleteMacro();
			return;
		}
		else if (sender == mChooseBtn)
		{
			OnGetKey();
			return;
		}
	}
	else if (sender == mTable)
	{
		if (message.Is(CTextTable::kSelectionChanged))
		{
			if (mTable->IsSelectionValid())
			{
				mEditBtn->Activate();
				mDeleteBtn->Activate();
			}
			else
			{
				mEditBtn->Deactivate();
				mDeleteBtn->Deactivate();
			}
			return;
		}
		else if (message.Is(CTextTable::kLDblClickCell))
		{
			OnEditMacro();
			return;
		}
	}

	// Pass up
	CDialogDirector::Receive(sender, message);
}

void CMacroEditDialog::SetDetails()
{
	mCopy = CPreferences::sPrefs->mTextMacros.GetValue();
	mTable->SetData(&mCopy);

	mKey->SetText(CKeyAction::GetKeyDescriptor(mCopy.GetKeyAction()));
}

void CMacroEditDialog::GetDetails()
{
	CPreferences::sPrefs->mTextMacros.SetValue(mCopy);
}

void CMacroEditDialog::OnNewMacro()
{
	cdstring name;
	cdstring macro;
	while(CEditMacro::PoseDialog(name, macro))
	{
		//  Check for duplicate
		if (mCopy.Contains(name))
		{
			// Duplicate error
			CErrorHandler::PutStopAlertRsrcStr("Alerts::MacroEdit::DuplicateName", name);
		}
		else if (mCopy.Add(name, macro))
		{
			// Add a row and refresh entire table
			mTable->InsertRows(1, mTable->GetItemCount());
			mTable->Refresh();

			// Break out of loop
			break;
		}
	}
}

void CMacroEditDialog::OnEditMacro()
{
	// Get first selected macro
	ulvector selected;
	mTable->GetSelection(selected);
	
	if (!selected.size())
		return;

	cdstrmap::iterator iter = mCopy.GetMacros().begin();
	for(unsigned long i = 0; i < selected.front(); i++)
		iter++;

	cdstring name = (*iter).first;
	cdstring macro = (*iter).second;
	while(CEditMacro::PoseDialog(name, macro))
	{
		//  Check for duplicate if name was changed
		if (((*iter).first != name) && mCopy.Contains(name))
		{
			// Duplicate error
			CErrorHandler::PutStopAlertRsrcStr("Alerts::MacroEdit::DuplicateName", name);
		}
		else if (mCopy.Change((*iter).first, name, macro))
		{
			// Force redraw after changes
			mTable->Refresh();

			// Break out of loop
			break;
		}
	}
}

void CMacroEditDialog::OnDeleteMacro()
{
	// Do for all selected macros
	ulvector selected;
	mTable->GetSelection(selected);
	
	if (!selected.size())
		return;

	// Get each selected name
	cdstrvect names;
	cdstrmap::iterator iter = mCopy.GetMacros().begin();
	ulvector::iterator pos = selected.begin();
	for(unsigned long i = 0; (i < mCopy.GetMacros().size()) && (pos != selected.end()); i++, iter++)
	{
		if (i == *pos)
		{
			names.push_back((*iter).first);
			pos++;
		}
	}

	// Now delete each name
	for(cdstrvect::const_iterator siter = names.begin(); siter != names.end(); siter++)
		mCopy.Delete(*siter);
	
	// Delete rows from table and refresh
	mTable->RemoveRows(names.size(), 1, true);
	mTable->Refresh();
}

void CMacroEditDialog::OnGetKey()
{
	unsigned char key;
	CKeyModifiers mods;
	if (CKeyChoiceDialog::PoseDialog(key, mods))
	{
		mCopy.GetKeyAction().SetKey(key);
		mCopy.GetKeyAction().GetKeyModifiers() = mods;
		mKey->SetText(CKeyAction::GetKeyDescriptor(mCopy.GetKeyAction()));
	}
}

void CMacroEditDialog::PoseDialog()
{
	// Create the dialog
	CMacroEditDialog* dlog = new CMacroEditDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetDetails();

	// Let DialogHandler process events
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetDetails();
		dlog->Close();
	}
}

#pragma mark -

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CMacroEditTable::CMacroEditTable(JXScrollbarSet* scrollbarSet,
								  JXContainer* enclosure,
								  const HSizingOption hSizing, 
								  const VSizingOption vSizing,
								  const JCoordinate x, const JCoordinate y,
								  const JCoordinate w, const JCoordinate h)
		: CTextTable(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Default destructor
CMacroEditTable::~CMacroEditTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CMacroEditTable::OnCreate()
{
	// Do inherited
	CTextTable::OnCreate();

	// Create columns and adjust flag rect
	InsertCols(2, 1);
	SetColWidth(100, 1, 1);
	SetColWidth(GetApertureWidth() - 100, 2, 2);
}

void CMacroEditTable::SetData(CTextMacros* macros)
{
	// Cache copy of data and insert initial set of rows
	mCopy = macros;
	InsertRows(mCopy->GetMacros().size(), 0);
}

// Resize columns
void CMacroEditTable::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	CTableDragAndDrop::ApertureResized(dw, dh);

	if (mCols)
	{
		SetColWidth(100, 1, 1);
		SetColWidth(GetApertureWidth() - 100, 2, 2);
	}
}

// Draw a cell
void CMacroEditTable::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
	StPenState save(pDC);

	// Get data
	cdstrmap::const_iterator iter = mCopy->GetMacros().begin();
	for(TableIndexT i = 0; i < inCell.row - 1; i++)
		iter++;

	// Draw selection
	DrawCellSelection(pDC, inCell);

	cdstring str;
	switch(inCell.col)
	{
	case 1:
		str = (*iter).first;
		break;

	case 2:
		str = (*iter).second;
		break;

	default:
		break;
	}

	::DrawClippedStringUTF8(pDC, str, JPoint(inLocalRect.left + 4, inLocalRect.top), inLocalRect, eDrawString_Left);
}
