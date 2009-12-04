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

#include "CBalloonDialog.h"
#include "CMessageWindow.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CResources.h"
#include "CTextDisplay.h"

#include <LCheckBox.h>
#include <LTableMultiGeometry.h>


// __________________________________________________________________________________________________
// C L A S S __ C R E P L Y C H O O S E D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CNamespaceDialog::CNamespaceDialog()
{
	mItsTable = NULL;
}

// Constructor from stream
CNamespaceDialog::CNamespaceDialog(LStream *inStream)
		: LDialogBox(inStream)
{
	mItsTable = NULL;
}

// Default destructor
CNamespaceDialog::~CNamespaceDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CNamespaceDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Info
	mHelpText = (CTextDisplay*) FindPaneByID(paneid_NamespaceHelp);
	mUserItems = (CTextDisplay*) FindPaneByID(paneid_NamespaceText);
	mUserItems->SetTabSelectAll(false);
	mServer = (LView*) FindPaneByID(paneid_NamespaceServer);
	mItsTable = (CNamespaceTable*) FindPaneByID(paneid_NamespaceTable);
	mDoAuto = (LCheckBox*) FindPaneByID(paneid_NamespaceAuto);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CNamespaceDialogBtns);
}

void CNamespaceDialog::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch(inMessage)
	{
	case msg_Namespace_Personal_Btn:
		mItsTable->ChangeSelection(CNamespaceTable::eNamespace_Personal);
		break;
	case msg_Namespace_Shared_Btn:
		mItsTable->ChangeSelection(CNamespaceTable::eNamespace_Shared);
		break;
	case msg_Namespace_Public_Btn:
		mItsTable->ChangeSelection(CNamespaceTable::eNamespace_Public);
		break;
	case msg_Namespace_All_Btn:
		mItsTable->ChangeSelection(CNamespaceTable::eNamespace_All);
		break;
	case msg_Namespace_None_Btn:
		mItsTable->ChangeSelection(CNamespaceTable::eNamespace_None);
		break;
	default:;
	}
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
	mItsTable->SetServerList(servs, servitems);
	mDoAuto->SetValue(do_auto);
	if (!servitems.size())
	{
		SDimension16 size;
		mServer->GetFrameSize(size);
		mServer->Hide();
		ResizeWindowBy(0, -size.height);
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
	cdstring txt;
	mUserItems->GetText(txt);

	char* s = ::strtok(txt.c_str_mod(), "\r");
	items.clear();
	while(s)
	{
		cdstring copyStr(s);
		items.push_back(copyStr);

		s = ::strtok(NULL, "\r");
	}

	do_auto = mDoAuto->GetValue();
}

#pragma mark ____________________________Static Processing

bool CNamespaceDialog::PoseDialog(CMboxProtocol::SNamespace& server,
								boolvector& servitems,
								cdstrvect& items, bool& do_auto)
{
	// Create the dialog
	CBalloonDialog theHandler(paneid_NamespaceDialog, CMulberryApp::sApp);
	CNamespaceDialog* dlog = (CNamespaceDialog*) theHandler.GetDialog();
	dlog->SetItems(items, server, servitems, do_auto);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		switch(hitMessage)
		{
		case msg_OK:
			dlog->GetItems(items, do_auto);
			return true;
		case msg_Cancel:
			return false;
		}
	}
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C R E P L Y C H O O S E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CNamespaceTable::CNamespaceTable(LStream *inStream)
		: CTableDrag(inStream)
{
	mData = NULL;
	mDataOn = NULL;
	mTextTraits = NULL;
	mTableGeometry = new LTableMultiGeometry(this, mFrameSize.width, 16);
}

// Default destructor
CNamespaceTable::~CNamespaceTable()
{
	// Forget traits
	DISPOSE_HANDLE(mTextTraits);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CNamespaceTable::FinishCreateSelf(void)
{
	// Do inherited
	CTableDrag::FinishCreateSelf();

	// Create columns
	SDimension16 frame;
	GetFrameSize(frame);

	InsertCols(3, 1, nil, 0, false);
	SetColWidth(20, 1, 1);
	SetColWidth(270, 2, 2);
	SetColWidth(frame.width - 290, 3, 3);

	// Get text traits resource - detatch & lock
	mTextTraits = UTextTraits::LoadTextTraits(261);
	ThrowIfResFail_(mTextTraits);
	::DetachResource((Handle) mTextTraits);
	::MoveHHi((Handle) mTextTraits);
	::HLock((Handle) mTextTraits);

	// No drag and drop unless exlicitly specified
	SetAllowDrag(false);

	// Load strings
	cdstring s;
	mTypeItems.push_back(s.FromResource("UI::Namespace::Personal"));
	mTypeItems.push_back(s.FromResource("UI::Namespace::Shared"));
	mTypeItems.push_back(s.FromResource("UI::Namespace::Public"));
	mTypeItems.push_back(s.FromResource("UI::Namespace::Entire"));
}

// Draw a cell
void CNamespaceTable::DrawCell(const STableCell &inCell, const Rect &inLocalRect)
{
	// Save text state in stack object
	StTextState		textState;
	StColorState	saveColors;
	StColorPenState::Normalize();

#if PP_Target_Carbon
	// Draw selection
	DrawCellSelection(inCell);
#endif

	// Set to required text
	UTextTraits::SetPortTextTraits(*mTextTraits);

	// Clip to cell frame & table frame
	Rect	clipper = mRevealedRect;
	PortToLocalPoint(topLeft(clipper));
	PortToLocalPoint(botRight(clipper));
	::SectRect(&clipper, &inLocalRect, &clipper);
	StClipRgnState	clip(clipper);

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
		Rect	iconRect;
		iconRect.left = inLocalRect.left + 3;
		iconRect.right = iconRect.left + 16;
		iconRect.bottom = inLocalRect.bottom;
		iconRect.top = iconRect.bottom - 16;
		// Check for tick
		::Ploticns(&iconRect, atNone, ttNone, mDataOn->at(inCell.row - 1) ? ICNx_DiamondTicked : ICNx_Diamond);
		break;

	case 2:
		{
			// Write address
			::MoveTo(inLocalRect.left, inLocalRect.bottom - 4);
			::DrawClippedStringUTF8(name, inLocalRect.right - inLocalRect.left, eDrawString_Left);
		}
		break;

	case 3:
		{
			::TextFace(bold);
			::MoveTo(inLocalRect.left, inLocalRect.bottom - 4);
			::DrawClippedStringUTF8(mTypeItems[descriptor], inLocalRect.right - inLocalRect.left, eDrawString_Left);
		}
		break;

	default:
		break;
	}

}

// Click in the cell
void CNamespaceTable::ClickCell(const STableCell &inCell, const SMouseDownEvent &inMouseDown)
{
	switch(inCell.col)
	{

	case 1:
		mDataOn->at(inCell.row - 1) = !mDataOn->at(inCell.row - 1);
		RefreshCell(inCell);
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
	Refresh();
}
