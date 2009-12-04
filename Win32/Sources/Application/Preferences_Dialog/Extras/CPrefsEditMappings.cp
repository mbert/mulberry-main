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

#include "CDrawUtils.h"
#include "CEditMapDialog.h"
#include "CMIMEMap.h"
#include "CMulberryCommon.h"
#include "CSDIFrame.h"
#include "CTableRowSelector.h"
#include "CXStringResources.h"

#include <WIN_LTableMultiGeometry.h>

/////////////////////////////////////////////////////////////////////////////
// CPrefsEditMappings dialog


CPrefsEditMappings::CPrefsEditMappings(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CPrefsEditMappings::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPrefsEditMappings)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPrefsEditMappings::~CPrefsEditMappings()
{
	// Must unsubclass table
	mTable.Detach();
	mTitles.Detach();
}


void CPrefsEditMappings::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsEditMappings)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsEditMappings, CHelpDialog)
	//{{AFX_MSG_MAP(CPrefsEditMappings)
	ON_BN_CLICKED(IDC_ADDMAP, OnAddMap)
	ON_BN_CLICKED(IDC_CHANGEMAP, OnChangeMap)
	ON_BN_CLICKED(IDC_DELETEMAP, OnDeleteMap)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsEditMappings message handlers

BOOL CPrefsEditMappings::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass table
	mTable.SubclassDlgItem(IDC_MAPTABLE, this);
	mTable.InitTable();
	mTable.SetMap(mItsMap);

	// Subclass titles
	mTitles.SubclassDlgItem(IDC_MAPTITLES, this);
	mTitles.SyncTable(&mTable, true);

	mTitles.LoadTitles("UI::Titles::MIMETypeMap", 3);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPrefsEditMappings::OnAddMap() 
{
	mTable.NewMap();
}

void CPrefsEditMappings::OnChangeMap() 
{
	mTable.EditMap();
}

void CPrefsEditMappings::OnDeleteMap() 
{
	// TODO: Add your control notification handler code here
	mTable.DeleteSelection();
}

// __________________________________________________________________________________________________
// C L A S S __ C R E P L Y C H O O S E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CEditMappingsTable::CEditMappingsTable()
{
	mTableGeometry = new LTableMultiGeometry(this, 128, 20);
	mTableSelector = new CTableSingleRowSelector(this);
	mItsMap = nil;
}

// Default destructor
CEditMappingsTable::~CEditMappingsTable()
{
}

BEGIN_MESSAGE_MAP(CEditMappingsTable, CTable)
	//{{AFX_MSG_MAP(CReplyChooseTable)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CEditMappingsTable::InitTable()
{
	// Create columns
	CRect frame;
	GetClientRect(frame);

	InsertCols(3, 1);
	SetColWidth(80, 2, 2);
	SetColWidth(64, 3, 3);
	SetColWidth(frame.Width() - 160, 1, 1);
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
	CEditMapDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetMap(new_map);
	
	if (dlog.DoModal() == IDOK)
	{
		dlog.GetMap(new_map);
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

		CEditMapDialog dlog(CSDIFrame::GetAppTopWindow());
		dlog.SetMap(change_map);

		if (dlog.DoModal() == IDOK)
		{
			dlog.GetMap(change_map);
			(*mItsMap)[selCell.row - 1] = change_map;
			sort(mItsMap->begin(), mItsMap->end());
			RedrawWindow();
		}
	}
}

// Handle key down
bool CEditMappingsTable::HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_RETURN:
		EditMap();
		break;

	case VK_BACK:
	case VK_DELETE:
		DeleteSelection();
		break;

	default:
		// Did not handle key
		return CTable::HandleKeyDown(nChar, nRepCnt, nFlags);
	}

	// Handled key
	return true;
}

// Remove rows
void CEditMappingsTable::RemoveRows(UInt32 inHowMany, TableIndexT inFromRow, bool refresh)
{
	// Prevent drawing to stop row delete causing immediate redraw and crash when scroll bar dissappears
	StDeferTableAdjustment changing(this);

	CTable::RemoveRows(inHowMany, inFromRow, refresh);
	(*mItsMap).erase((*mItsMap).begin() + inFromRow - 1, (*mItsMap).begin() + inFromRow - 1 + inHowMany);
}

// Draw a cell
void CEditMappingsTable::DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect)
{
	StDCState save(pDC);

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
		::DrawClippedStringUTF8(pDC, theTxt, CPoint(inLocalRect.left + 4, inLocalRect.top), inLocalRect, eDrawString_Left);
		break;

	case 2:
		// Write address
		::DrawClippedStringUTF8(pDC, item.GetFileSuffix(), CPoint(inLocalRect.left + 4, inLocalRect.top), inLocalRect, eDrawString_Left);
		break;

	case 3:
		// Write address
		theTxt = rsrc::GetIndexedString("UI::Preferences::MIMETypeMap", item.GetAppLaunch());
		::DrawClippedStringUTF8(pDC, theTxt, CPoint(inLocalRect.left + 4, inLocalRect.top), inLocalRect, eDrawString_Left);
		break;


	default:
		break;
	}

}

// Click in the cell
void CEditMappingsTable::LDblClickCell(const STableCell& inCell, UINT nFlags)
{
	EditMap();
}
