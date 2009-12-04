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

#include "CIconLoader.h"
#include "CMessageWindow.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CResizeNotifier.h"
#include "CTableScrollbarSet.h"
#include "CSimpleTitleTable.h"
#include "CTextDisplay.h"

#include "StPenState.h"

#include <UNX_LTableMultiGeometry.h>

#include <JXEngravedRect.h>
#include <JXImage.h>
#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CNamespaceDialog::CNamespaceDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
	mTitles = NULL;
	mTable = NULL;
}

// Default destructor
CNamespaceDialog::~CNamespaceDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

const int cTitleHeight = 16;

// Get details of sub-panes
void CNamespaceDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 450,410, "");
    assert( window != NULL );
    SetWindow(window);

    CResizeNotifier* obj1 =
        new CResizeNotifier(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 450,410);
    assert( obj1 != NULL );

    mHelpText =
        new JXStaticText("text", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,10, 430,70);
    assert( mHelpText != NULL );
    mHelpText->SetFontSize(10);

    mServer =
        new JXEngravedRect(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,95, 430,160);
    assert( mServer != NULL );

    mServerTitle =
        new JXStaticText("Server Suggestions:", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 15,80, 120,20);
    assert( mServerTitle != NULL );

    CScrollbarSet* sbs_server =
        new CScrollbarSet(mServer,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,5, 420,100);
    assert( sbs_server != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Select:", mServer,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,115, 40,15);
    assert( obj2 != NULL );
    obj2->SetFontSize(10);

    mPersonalBtn =
        new JXTextButton("Personal", mServer,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 50,110, 70,20);
    assert( mPersonalBtn != NULL );
    mPersonalBtn->SetFontSize(10);

    mSharedBtn =
        new JXTextButton("Shared", mServer,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 125,110, 70,20);
    assert( mSharedBtn != NULL );
    mSharedBtn->SetFontSize(10);

    mPublicBtn =
        new JXTextButton("Public", mServer,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 200,110, 70,20);
    assert( mPublicBtn != NULL );
    mPublicBtn->SetFontSize(10);

    mAllBtn =
        new JXTextButton("All", mServer,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 275,110, 70,20);
    assert( mAllBtn != NULL );
    mAllBtn->SetFontSize(10);

    mNoneBtn =
        new JXTextButton("None", mServer,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 350,110, 70,20);
    assert( mNoneBtn != NULL );
    mNoneBtn->SetFontSize(10);

    mDoAuto =
        new JXTextCheckbox("Automatically Add Server Suggestions in Future", mServer,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,135, 300,20);
    assert( mDoAuto != NULL );

    mUserItemsTitle =
        new JXUpRect(obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 10,270, 430,25);
    assert( mUserItemsTitle != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Places to Search for Mailboxes on the Server", mUserItemsTitle,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 80,4, 265,17);
    assert( obj3 != NULL );

    mUserItems =
        new CTextInputDisplay(obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 10,290, 430,75);
    assert( mUserItems != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 270,375, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 360,375, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

// end JXLayout

	mTable = new CNamespaceTable(sbs_server,sbs_server->GetScrollEnclosure(),
										JXWidget::kHElastic,
										JXWidget::kVElastic,
										0,cTitleHeight, 550, 100);
	mTitles = new CSimpleTitleTable(sbs_server, sbs_server->GetScrollEnclosure(),
																	 JXWidget::kHElastic,
																	 JXWidget::kFixedTop,
																	 0, 0, 550, cTitleHeight);
	mTitles->OnCreate();
	mTable->OnCreate();
	mTitles->SyncTable(mTable, true);

	mTitles->LoadTitles("UI::Titles::Namespace", 3);

	window->SetTitle("Choose Mailbox Locations");
	SetButtons(mOKBtn, mCancelBtn);

	ListenTo(obj1);
	ListenTo(mPersonalBtn);
	ListenTo(mSharedBtn);
	ListenTo(mPublicBtn);
	ListenTo(mAllBtn);
	ListenTo(mNoneBtn);
}

void CNamespaceDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mPersonalBtn)
		{
			mTable->ChangeSelection(CNamespaceTable::eNamespace_Personal);
			return;
		}
		else if (sender == mSharedBtn)
		{
			mTable->ChangeSelection(CNamespaceTable::eNamespace_Shared);
			return;
		}
		else if (sender == mPublicBtn)
		{
			mTable->ChangeSelection(CNamespaceTable::eNamespace_Public);
			return;
		}
		else if (sender == mAllBtn)
		{
			mTable->ChangeSelection(CNamespaceTable::eNamespace_All);
			return;
		}
		else if (sender == mNoneBtn)
		{
			mTable->ChangeSelection(CNamespaceTable::eNamespace_None);
			return;
		}
	}
		
	CDialogDirector::Receive(sender, message);
	return;
}

// Set namespaces
void CNamespaceDialog::SetItems(const cdstrvect& items, CMboxProtocol::SNamespace& servs, boolvector& servitems, bool do_auto)
{
	cdstring user_items;
	for(cdstrvect::const_iterator iter = items.begin(); iter != items.end(); iter++)
	{
		user_items += *iter;
		user_items += os_endl;
	}
	mUserItems->SetText(user_items);

	// Hide server items if none available (i.e. no NAMESPACE support)
	mTable->SetServerList(servs, servitems);
	mDoAuto->SetState(JBoolean(do_auto));
	if (!servitems.size())
	{
		JPoint pt1 = mServer->GetBoundsGlobal().topLeft();
		JPoint pt2 = mUserItemsTitle->GetBoundsGlobal().topLeft();
		JCoordinate resizeby = pt1.y - pt2.y;

		mServer->Hide();
		mServerTitle->Hide();
		
		AdjustSize(0, resizeby);
	}

	// Reset help text
	cdstring temp;
	temp.FromResource(servitems.size() ? "UI::Namespace::Help1" : "UI::Namespace::Help2");
	mHelpText->SetText(temp);
}

// Get selected items
void CNamespaceDialog::GetItems(cdstrvect& items, bool& do_auto)
{
	// Copy handle to text with null terminator
	cdstring txt(mUserItems->GetText());

	char* s = ::strtok(txt.c_str_mod(), "\n");
	items.clear();
	while(s)
	{
		cdstring copyStr(s);
		items.push_back(copyStr);

		s = ::strtok(NULL, "\n");
	}

	do_auto = mDoAuto->IsChecked();
}

#pragma mark ____________________________Static Processing

bool CNamespaceDialog::PoseDialog(CMboxProtocol::SNamespace& server,
								boolvector& servitems,
								cdstrvect& items, bool& do_auto)
{
	// Create the dialog
	CNamespaceDialog* dlog = new CNamespaceDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->mTable->InitTable();
	dlog->SetItems(items, server, servitems, do_auto);

	// Let DialogHandler process events
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetItems(items, do_auto);
		dlog->Close();
		return true;
	}

	return false;
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C R E P L Y C H O O S E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CNamespaceTable::CNamespaceTable(JXScrollbarSet* scrollbarSet,
								 JXContainer* enclosure,
								 const HSizingOption hSizing,
								 const VSizingOption vSizing,
								 const JCoordinate x, const JCoordinate y,
								 const JCoordinate w, const JCoordinate h)
	: CTable(scrollbarSet, enclosure, hSizing, vSizing, x,y,w,h)
{
	mData = NULL;
	mDataOn = NULL;
	mTableGeometry = new LTableMultiGeometry(this, 256, 16);
}

// Default destructor
CNamespaceTable::~CNamespaceTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CNamespaceTable::InitTable()
{
	// Create columns
	InsertCols(3, 1);

	SetColWidth(30, 1, 1);
	SetColWidth(270, 2, 2);
	SetColWidth(GetApertureWidth() - 300, 3, 3);

	// Load icons
	mIcons.push_back(CIconLoader::GetIcon(IDI_DIAMOND, this, 16, 0x00FFFFFF));
	mIcons.push_back(CIconLoader::GetIcon(IDI_DIAMONDTICKED, this, 16, 0x00FFFFFF));

	// Load strings
	cdstring s;
	mTypeItems.push_back(s.FromResource("UI::Namespace::Personal"));
	mTypeItems.push_back(s.FromResource("UI::Namespace::Shared"));
	mTypeItems.push_back(s.FromResource("UI::Namespace::Public"));
	mTypeItems.push_back(s.FromResource("UI::Namespace::Entire"));
}

// Draw a cell
void CNamespaceTable::DrawCell(JPainter* pDC, const STableCell&	inCell, const JRect& inLocalRect)
{
	StPenState save(pDC);

	// Draw selection
	DrawCellSelection(pDC, inCell);

	// Get data
	unsigned long descriptor = 0;
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
		// Check for tick
		unsigned long icon_id = mDataOn->at(inCell.row - 1) ? 1 : 0;
		pDC->Image(*mIcons[icon_id], mIcons[icon_id]->GetBounds(), inLocalRect.left + 6, inLocalRect.top);
		break;
	}

	case 2:
		::DrawClippedStringUTF8(pDC, name, JPoint(inLocalRect.left + 4, inLocalRect.top), inLocalRect, eDrawString_Left);
		break;

	case 3:
		{
			JFontStyle style = pDC->GetFontStyle();
			style.bold = kTrue;
			pDC->SetFontStyle(style);
			::DrawClippedStringUTF8(pDC, mTypeItems[descriptor], JPoint(inLocalRect.left + 4, inLocalRect.top), inLocalRect, eDrawString_Left);
		}
		break;
	}
}

// Click in the cell
void CNamespaceTable::LClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	switch(inCell.col)
	{

	case 1:
		mDataOn->at(inCell.row - 1) = !mDataOn->at(inCell.row - 1);
		RefreshRow(inCell.row);
		break;

	default:;
	}
}

// Set namespaces
void CNamespaceTable::SetServerList(CMboxProtocol::SNamespace& servs, boolvector& servitems)
{
	// Insert rows
	mDataOn = &servitems;
	mData = &servs;
	InsertRows(mDataOn->size(), 1, NULL, 0, false);
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
	Refresh();
}
