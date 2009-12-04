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

#include "CBalloonDialog.h"
#include "CEditMacro.h"
#include "CErrorHandler.h"
#include "CKeyChoiceDialog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CStaticText.h"

#include <LPushButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C R E P L Y C H O O S E D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMacroEditDialog::CMacroEditDialog()
{
	mItsTable = NULL;
}

// Constructor from stream
CMacroEditDialog::CMacroEditDialog(LStream *inStream)
		: LDialogBox(inStream)
{
	mItsTable = NULL;
}

// Default destructor
CMacroEditDialog::~CMacroEditDialog()
{
	if (mItsTable)
		mItsTable->SetData(NULL);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CMacroEditDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Info
	mItsTable = (CMacroEditTable*) FindPaneByID(paneid_MacroEditTable);
	mItsTable->AddListener(this);

	mEditBtn = (LPushButton*) FindPaneByID(paneid_MacroEditQuoteEdit);
	mEditBtn->Disable();
	mDeleteBtn = (LPushButton*) FindPaneByID(paneid_MacroEditAddToDelete);
	mDeleteBtn->Disable();

	mKey = (CStaticText*) FindPaneByID(paneid_MacroEditActionKey);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CMacroEditDialogBtns);
}

void CMacroEditDialog::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch(inMessage)
	{
	case msg_MacroEditQuoteNew:
		NewMacro();
		break;
	case msg_MacroEditQuoteEdit:
	case msg_MacroEditDoubleClick:
		EditMacro();
		break;
	case msg_MacroEditAddToDelete:
		DeleteMacro();
		break;
	case msg_MacroEditSelect:
		if (mItsTable->IsSelectionValid())
		{
			mEditBtn->Enable();
			mDeleteBtn->Enable();
		}
		else
		{
			mEditBtn->Disable();
			mDeleteBtn->Disable();
		}
		break;
	case msg_MacroEditKeyChange:
		GetKey();
		break;
	default:;
	}
}

void CMacroEditDialog::SetDetails()
{
	mCopy = CPreferences::sPrefs->mTextMacros.GetValue();
	mItsTable->SetData(&mCopy);

	mKey->SetText(CKeyAction::GetKeyDescriptor(mCopy.GetKeyAction()));
}

void CMacroEditDialog::GetDetails()
{
	CPreferences::sPrefs->mTextMacros.SetValue(mCopy);
}

void CMacroEditDialog::NewMacro()
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
			mItsTable->InsertRows(1, mCopy.GetMacros().size(), NULL, 0, false);
			mItsTable->Refresh();

			// Break out of loop
			break;
		}
	}
}

void CMacroEditDialog::EditMacro()
{
	// Get first selected macro
	ulvector selected;
	mItsTable->GetSelection(selected);
	
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
			mItsTable->Refresh();

			// Break out of loop
			break;
		}
	}
}

void CMacroEditDialog::DeleteMacro()
{
	// Do for all selected macros
	ulvector selected;
	mItsTable->GetSelection(selected);
	
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
	mItsTable->RemoveRows(names.size(), 1, true);
}

void CMacroEditDialog::GetKey()
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
	CBalloonDialog	theHandler(paneid_MacroEditDialog, CMulberryApp::sApp);
	((CMacroEditDialog*) theHandler.GetDialog())->SetDetails();
	theHandler.StartDialog();

	// Let DialogHandler process events
	MessageT hitMessage;
	while (true)
	{					
		hitMessage = theHandler.DoDialog();
		
		if (hitMessage == msg_OK)
		{
			((CMacroEditDialog*) theHandler.GetDialog())->GetDetails();
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C R E P L Y C H O O S E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CMacroEditTable::CMacroEditTable(LStream *inStream)
		: CTextTable(inStream)
{
	mCopy = NULL;
}

// Default destructor
CMacroEditTable::~CMacroEditTable()
{
	mCopy = NULL;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CMacroEditTable::FinishCreateSelf(void)
{
	// Do inherited
	CTextTable::FinishCreateSelf();

	// Add another column (one is already present from base class)
	InsertCols(1, 1, NULL, 0, false);

	// Set column widths
	SDimension16 frame;
	GetFrameSize(frame);
	SetColWidth(100, 1, 1);
	SetColWidth(frame.width - 100, 2, 2);
}

void CMacroEditTable::SetData(CTextMacros* macros)
{
	// Cache copy of data and insert initial set of rows
	mCopy = macros;
	if (mCopy)
		InsertRows(mCopy->GetMacros().size(), 1, NULL, 0, false);
	else
		// Remove all rows
		RemoveRows(mRows, 1, true);
}

// Draw a cell
void CMacroEditTable::DrawCell(const STableCell &inCell, const Rect &inLocalRect)
{
	// Must have data
	if (mCopy == NULL)
		return;

	// Save text state in stack object
	StTextState		textState;
	StColorState	saveColors;
	StColorPenState::Normalize();

	// Set to required text
	UTextTraits::SetPortTextTraits(&mTextTraits);

#if PP_Target_Carbon
	// Draw selection
	DrawCellSelection(inCell);
#endif

	// Clip to cell frame & table frame
	Rect	clipper = mRevealedRect;
	PortToLocalPoint(topLeft(clipper));
	PortToLocalPoint(botRight(clipper));
	::SectRect(&clipper, &inLocalRect, &clipper);
	StClipRgnState	clip(clipper);

	// Get data
	cdstrmap::const_iterator iter = mCopy->GetMacros().begin();
	for(int i = 0; i < inCell.row - 1; i++)
		iter++;

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

	::MoveTo(inLocalRect.left, inLocalRect.bottom - 4);
	::DrawClippedStringUTF8(str, inLocalRect.right - inLocalRect.left, eDrawString_Left);
}
