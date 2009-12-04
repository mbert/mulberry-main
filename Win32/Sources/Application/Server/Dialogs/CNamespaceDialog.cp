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


// Source for CNamespaceDialog class


#include "CNamespaceDialog.h"

#include "CDrawUtils.h"
#include "CIconLoader.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

#include "resource1.h"

#include <WIN_LTableMultiGeometry.h>

/////////////////////////////////////////////////////////////////////////////
// CNamespaceDialog dialog


CNamespaceDialog::CNamespaceDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(IDD_NAMESPACE, pParent)
{
	//{{AFX_DATA_INIT(CNamespaceDialog)
	mDoAuto = FALSE;
	mUserItems = _T("");
	//}}AFX_DATA_INIT
}

CNamespaceDialog::~CNamespaceDialog()
{
	// Must unsubclass table
	mTitles.Detach();
	mTable.Detach();
}


void CNamespaceDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNamespaceDialog)
	DDX_UTF8Text(pDX, IDC_NAMESPACE_HELP, mHelpText);
	DDX_Check(pDX, IDC_NAMESPACE_AUTO, mDoAuto);
	DDX_UTF8Text(pDX, IDC_NAMESPACE_PLACES, mUserItems);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNamespaceDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CNamespaceDialog)
	ON_BN_CLICKED(IDC_NAMESPACE_PERSONAL, OnSelectPersonalBtn)
	ON_BN_CLICKED(IDC_NAMESPACE_SHARED, OnSelectSharedBtn)
	ON_BN_CLICKED(IDC_NAMESPACE_PUBLIC, OnSelectPublicBtn)
	ON_BN_CLICKED(IDC_NAMESPACE_ALL, OnSelectAllBtn)
	ON_BN_CLICKED(IDC_NAMESPACE_NONE, OnSelectNoneBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNamespaceDialog message handlers

BOOL CNamespaceDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass table
	mTable.SubclassDlgItem(IDC_NAMESPACECHOICE, this);
	mTable.InitTable();
	mTable.SetServerList(*mServs, *mServitems);

	// Subclass titles
	mTitles.SubclassDlgItem(IDC_NAMESPACECHOICETITLES, this);
	mTitles.SyncTable(&mTable, true);

	mTitles.LoadTitles("UI::Titles::Namespace", 3);

	if (!mServitems->size())
		HideServerItems();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

#pragma mark ____________________________Static Processing

bool CNamespaceDialog::PoseDialog(CMboxProtocol::SNamespace& server,
								boolvector& servitems,
								cdstrvect& items, bool& do_auto)
{
	bool result = false;

	// Create the dialog
	CNamespaceDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetItems(items, server, servitems, do_auto);

	// Let Dialog process events
	if (dlog.DoModal() == IDOK)
	{
		dlog.GetItems(items, do_auto);

		// Flag success
		result = true;
	}

	return result;
}

// Set namespaces
void CNamespaceDialog::SetItems(const cdstrvect& items, CMboxProtocol::SNamespace& servs, boolvector& servitems, bool do_auto)
{
	mUserItems.clear();
	for(cdstrvect::const_iterator iter = items.begin(); iter != items.end(); iter++)
	{
		mUserItems += (*iter).c_str();
		mUserItems += os_endl;
	}

	// Hide server items if none available (i.e. no NAMESPACE support)
	mServs = &servs;	
	mServitems = &servitems;
	mDoAuto = do_auto;

	// Reset help text
	cdstring temp;
	temp.FromResource(servitems.size() ? "UI::Namespace::Help1" : "UI::Namespace::Help2");
	mHelpText = temp;
}

// Get selected items
void CNamespaceDialog::GetItems(cdstrvect& items, bool& do_auto)
{
	// Copy handle to text with null terminator
	char* s = ::strtok(mUserItems.c_str_mod(), "\r\n");
	items.clear();
	while(s)
	{
		cdstring copyStr(s);
		items.push_back(copyStr);

		s = ::strtok(NULL, "\r\n");
	}

	do_auto = mDoAuto;
}

void CNamespaceDialog::HideServerItems()
{
	CRect rect1;
	GetDlgItem(IDC_NAMESPACE_SERVER)->GetWindowRect(rect1);
	CRect rect2;
	GetDlgItem(IDC_NAMESPACE_PLACESTITLE)->GetWindowRect(rect2);
	int resizeby = rect1.top - rect2.top;

	GetDlgItem(IDC_NAMESPACE_SERVER)->ShowWindow(SW_HIDE);
	mTable.ShowWindow(SW_HIDE);
	GetDlgItem(IDC_NAMESPACE_SELECTTITLE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_NAMESPACE_PERSONAL)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_NAMESPACE_SHARED)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_NAMESPACE_PUBLIC)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_NAMESPACE_ALL)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_NAMESPACE_NONE)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_NAMESPACE_AUTO)->ShowWindow(SW_HIDE);
	
	::MoveWindowBy(GetDlgItem(IDC_NAMESPACE_PLACESTITLE), 0, resizeby);
	::MoveWindowBy(GetDlgItem(IDC_NAMESPACE_PLACES), 0, resizeby);
	::ResizeWindowBy(this, 0, resizeby);
}

void CNamespaceDialog::OnSelectPersonalBtn()
{
	GetTable()->ChangeSelection(CNamespaceTable::eNamespace_Personal);
}

void CNamespaceDialog::OnSelectSharedBtn()
{
	GetTable()->ChangeSelection(CNamespaceTable::eNamespace_Shared);
}

void CNamespaceDialog::OnSelectPublicBtn()
{
	GetTable()->ChangeSelection(CNamespaceTable::eNamespace_Public);
}

void CNamespaceDialog::OnSelectAllBtn()
{
	GetTable()->ChangeSelection(CNamespaceTable::eNamespace_All);
}

void CNamespaceDialog::OnSelectNoneBtn()
{
	GetTable()->ChangeSelection(CNamespaceTable::eNamespace_None);
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C R E P L Y C H O O S E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CNamespaceTable::CNamespaceTable()
{
	mData = NULL;
	mDataOn = NULL;
	mTableGeometry = new LTableMultiGeometry(this, 256, 16);
	
	// Prevent selection being displayed
	//SetDrawRowSelection(false);
}

// Default destructor
CNamespaceTable::~CNamespaceTable()
{
}

BEGIN_MESSAGE_MAP(CNamespaceTable, CTable)
	//{{AFX_MSG_MAP(CNamespaceTable)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CNamespaceTable::InitTable()
{
	// Create columns
	CRect frame;
	GetClientRect(frame);

	InsertCols(3, 1);
	SetColWidth(32, 1, 1);
	SetColWidth(100, 3, 3);
	SetColWidth(frame.Width() - 16 - 132, 2, 2);

	// Load strings
	cdstring s;
	mTypeItems.push_back(s.FromResource("UI::Namespace::Personal"));
	mTypeItems.push_back(s.FromResource("UI::Namespace::Shared"));
	mTypeItems.push_back(s.FromResource("UI::Namespace::Public"));
	mTypeItems.push_back(s.FromResource("UI::Namespace::Entire"));
}

// Draw a cell
void CNamespaceTable::DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect)
{
	StDCState save(pDC);

	// Draw selection
	DrawCellSelection(pDC, inCell);

	// Get data
	int descriptor = 0;
	cdstring name;
	if (inCell.row > mData->offset(CMboxProtocol::ePublic))
	{
		descriptor = 2;
		name = mData->mItems[CMboxProtocol::ePublic].at(inCell.row - 1 - mData->offset(CMboxProtocol::ePublic)).first;
	}
	else if (inCell.row > mData->offset(CMboxProtocol::eShared))
	{
		descriptor = 1;
		name = mData->mItems[CMboxProtocol::eShared].at(inCell.row - 1 - mData->offset(CMboxProtocol::eShared)).first;
	}
	else
	{
		descriptor = 0;
		name = mData->mItems[CMboxProtocol::ePersonal].at(inCell.row - 1).first;
	}
	if (name.empty())
		name = mTypeItems[3];

	switch(inCell.col)
	{

	case 1:
	{
		CRect	iconRect;
		iconRect.left = inLocalRect.left + 3;
		iconRect.right = iconRect.left + 16;
		iconRect.bottom = inLocalRect.bottom;
		iconRect.top = iconRect.bottom - 16;
		// Check for tick
		CIconLoader::DrawIcon(pDC, inLocalRect.left + 6, inLocalRect.top, mDataOn->at(inCell.row - 1) ? IDI_DIAMONDTICKED : IDI_DIAMOND, 16);
		break;
	}

	case 2:
		// Write address
		::DrawClippedStringUTF8(pDC, name, CPoint(inLocalRect.left + 4, inLocalRect.top), inLocalRect, eDrawString_Left);
		break;

	case 3:	// Will not get this if no original column
		pDC->SelectObject(CMulberryApp::sAppFontBold);
		::DrawClippedStringUTF8(pDC, mTypeItems[descriptor], CPoint(inLocalRect.left + 4, inLocalRect.top), inLocalRect, eDrawString_Left);
		break;

	default:
		break;
	}

}

// Click in the cell
void CNamespaceTable::LClickCell(const STableCell& inCell, UINT nFlags)
{
	switch(inCell.col)
	{

	case 1:
		mDataOn->at(inCell.row - 1) = !mDataOn->at(inCell.row - 1);
		RefreshRow(inCell.row);
		break;

	default:
		// Do nothing
		return;
	}
}

// Set namespaces
void CNamespaceTable::SetServerList(CMboxProtocol::SNamespace& servs, boolvector& servitems)
{
	// Insert rows
	mDataOn = &servitems;
	mData = &servs;
	InsertRows(mDataOn->size(), 1, nil, 0, false);
}

void CNamespaceTable::ChangeSelection(ENamespaceSelect select)
{
	// Iterator over each element
	unsigned long ctr = 1;
	for(boolvector::iterator iter = mDataOn->begin(); iter != mDataOn->end(); iter++, ctr++)
	{
		switch(select)
		{
		case eNamespace_Personal:
			if (ctr <= mData->offset(CMboxProtocol::eShared))
				*iter = true;
			break;
		case eNamespace_Shared:
			if ((ctr > mData->offset(CMboxProtocol::eShared)) && (ctr <= mData->offset(CMboxProtocol::ePublic)))
				*iter = true;
			break;
		case eNamespace_Public:
			if (ctr > mData->offset(CMboxProtocol::ePublic))
				*iter = true;
			break;
		case eNamespace_All:
			*iter = true;
			break;
		case eNamespace_None:
			*iter = false;
			break;
		}
	}
	
	// Force redraw
	RedrawWindow();
}
