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

#include "CDrawUtils.h"
#include "CEditMacro.h"
#include "CErrorHandler.h"
#include "CKeyChoiceDialog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CUnicodeUtils.h"
#include "CSDIFrame.h"

/////////////////////////////////////////////////////////////////////////////
// CMacroEditDialog dialog


CMacroEditDialog::CMacroEditDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CMacroEditDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CMacroEditDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMacroEditDialog)
	DDX_Control(pDX, IDC_MACROEDIT_EDIT, mEditBtn);
	DDX_Control(pDX, IDC_MACROEDIT_DELETE, mDeleteBtn);
	DDX_UTF8Text(pDX, IDC_MACROEDIT_KEY, mKey);
	DDX_Control(pDX, IDC_MACROEDIT_KEY, mKeyCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMacroEditDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CMacroEditDialog)
	ON_COMMAND(IDC_MACROEDIT_SELCHANGE, OnSelchangeList)
	ON_COMMAND(IDC_MACROEDIT_DBLCLICK, OnEditBtn)
	ON_COMMAND(IDC_MACROEDIT_NEW, OnNewBtn)
	ON_COMMAND(IDC_MACROEDIT_EDIT, OnEditBtn)
	ON_COMMAND(IDC_MACROEDIT_DELETE, OnDeleteBtn)
	ON_COMMAND(IDC_MACROEDIT_KEYCHOOSE, OnChooseBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMacroEditDialog message handlers

BOOL CMacroEditDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass table
	mTable.SubclassDlgItem(IDC_MACROEDIT_LIST, this);
	mTable.SetMsgTarget(this);
	mTable.SetSelectionMsg(IDC_MACROEDIT_SELCHANGE);
	mTable.SetDoubleClickMsg(IDC_MACROEDIT_DBLCLICK);
	mTable.SetData(&mCopy);

	// Subclass titles
	mTitles.SubclassDlgItem(IDC_MACROEDIT_TITLES, this);
	mTitles.SyncTable(&mTable, true);

	mTitles.LoadTitles("UI::Titles::MacroEdit", 2);

	// Disable buttons when no selection
	mEditBtn.EnableWindow(false);
	mDeleteBtn.EnableWindow(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMacroEditDialog::SetDetails()
{
	mCopy = CPreferences::sPrefs->mTextMacros.GetValue();
	mKey = CKeyAction::GetKeyDescriptor(mCopy.GetKeyAction());
}

void CMacroEditDialog::GetDetails()
{
	CPreferences::sPrefs->mTextMacros.SetValue(mCopy);
}

void CMacroEditDialog::OnSelchangeList()
{
	// Disable buttons when no selection
	mEditBtn.EnableWindow(mTable.IsSelectionValid());
	mDeleteBtn.EnableWindow(mTable.IsSelectionValid());
}

void CMacroEditDialog::OnNewBtn()
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
			mTable.InsertRows(1, mTable.GetItemCount());
			mTable.RedrawWindow();

			// Break out of loop
			break;
		}
	}
}

void CMacroEditDialog::OnEditBtn()
{
	// Get first selected macro
	ulvector selected;
	mTable.GetSelection(selected);
	
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
			mTable.RedrawWindow();

			// Break out of loop
			break;
		}
	}
}

void CMacroEditDialog::OnDeleteBtn()
{
	// Do for all selected macros
	ulvector selected;
	mTable.GetSelection(selected);
	
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
	mTable.RemoveRows(names.size(), 1, true);
}

void CMacroEditDialog::OnChooseBtn()
{
	unsigned char key;
	CKeyModifiers mods;
	if (CKeyChoiceDialog::PoseDialog(key, mods))
	{
		mCopy.GetKeyAction().SetKey(key);
		mCopy.GetKeyAction().GetKeyModifiers() = mods;
		CUnicodeUtils::SetWindowTextUTF8(&mKeyCtrl, CKeyAction::GetKeyDescriptor(mCopy.GetKeyAction()));
	}
}

void CMacroEditDialog::PoseDialog()
{
	// Create dlog
	CMacroEditDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetDetails();
	
	if (dlog.DoModal() == IDOK)
		dlog.GetDetails();
}

#pragma mark -

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CMacroEditTable::CMacroEditTable()
{
	mDoubleClickAll = true;
}

// Default destructor
CMacroEditTable::~CMacroEditTable()
{
}

BEGIN_MESSAGE_MAP(CMacroEditTable, CTable)
	//{{AFX_MSG_MAP(CMacroEditTable)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CMacroEditTable::InitTable()
{
	// Create columns
	CRect frame;
	GetClientRect(frame);

	// Add another column (one is already present from base class)
	InsertCols(2, 1);
	SetColWidth(100, 1, 1);
	SetColWidth(frame.Width() - 116, 2, 2);
}

void CMacroEditTable::SetData(CTextMacros* macros)
{
	// Init the table first
	InitTable();

	// Cache copy of data and insert initial set of rows
	mCopy = macros;
	InsertRows(mCopy->GetMacros().size(), 0);
}

// Draw a cell
void CMacroEditTable::DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect)
{
	StDCState save(pDC);

	// Get data
	cdstrmap::const_iterator iter = mCopy->GetMacros().begin();
	for(int i = 0; i < inCell.row - 1; i++)
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

	// Draw string
	::DrawClippedStringUTF8(pDC, str, CPoint(inLocalRect.left + 4, inLocalRect.top), inLocalRect, eDrawString_Left);
}

