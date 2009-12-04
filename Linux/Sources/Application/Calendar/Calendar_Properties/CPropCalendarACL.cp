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


// Source for CPropCalendarACL class

#include "CPropCalendarACL.h"

#include "CACLStylePopup.h"
#include "CACLTable.h"
#include "CACLTitleTable.h"
#include "CErrorHandler.h"
#include "CIconLoader.h"
#include "CLog.h"
#include "CMulberryCommon.h"
#include "CNewACLDialog.h"
#include "CPreferences.h"
#include "CSimpleTitleTable.h"
#include "CTableScrollbarSet.h"

#include "CCalendarStoreManager.h"
#include "CCalendarProtocol.h"

#include "TPopupMenu.h"

#include <JXColormap.h>
#include <JXDownRect.h>
#include <JXImageWidget.h>
#include <JXStaticText.h>
#include <JXTextButton.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

const int cTitleHeight = 16;

// Get details of sub-panes
void CPropCalendarACL::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Access Control", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 100,20);
    assert( obj1 != NULL );
    const JFontStyle obj1_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj1->SetFontStyle(obj1_style);

    JXStaticText* obj2 =
        new JXStaticText("Calendar:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,25, 60,20);
    assert( obj2 != NULL );

    mCalendarPopup =
        new HPopupMenu("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,21, 250,24);
    assert( mCalendarPopup != NULL );

    JXDownRect* obj3 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 345,50, 100,160);
    assert( obj3 != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Key", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 350,43, 30,15);
    assert( obj4 != NULL );
    obj4->SetFontSize(10);
    const JFontStyle obj4_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj4->SetFontStyle(obj4_style);

    CTableScrollbarSet* sbs =
        new CTableScrollbarSet(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,50, 325,195);
    assert( sbs != NULL );

    mNewUserBtn =
        new JXTextButton("New User...", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,250, 90,24);
    assert( mNewUserBtn != NULL );
    mNewUserBtn->SetFontSize(10);

    mDeleteUserBtn =
        new JXTextButton("Delete", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 120,250, 90,24);
    assert( mDeleteUserBtn != NULL );
    mDeleteUserBtn->SetFontSize(10);

    JXStaticText* obj5 =
        new JXStaticText("Style:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 220,254, 35,15);
    assert( obj5 != NULL );
    obj5->SetFontSize(10);

    mACLStylePopup =
        new CACLStylePopup("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 255,252, 35,20);
    assert( mACLStylePopup != NULL );

    mChangeAllBtn =
        new JXTextButton("Apply to All Selected Calendars", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,279, 195,26);
    assert( mChangeAllBtn != NULL );
    mChangeAllBtn->SetFontSize(10);

    JXImageWidget* key1 =
        new JXImageWidget(obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 16,16);
    assert( key1 != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Freebusy", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,10, 55,15);
    assert( obj6 != NULL );
    obj6->SetFontSize(10);
    const JFontStyle obj6_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj6->SetFontStyle(obj6_style);

    JXImageWidget* key2 =
        new JXImageWidget(obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,30, 16,16);
    assert( key2 != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Read", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,30, 49,15);
    assert( obj7 != NULL );
    obj7->SetFontSize(10);
    const JFontStyle obj7_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj7->SetFontStyle(obj7_style);

    JXImageWidget* key4 =
        new JXImageWidget(obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,50, 16,16);
    assert( key4 != NULL );

    JXStaticText* obj8 =
        new JXStaticText("Write", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,50, 49,15);
    assert( obj8 != NULL );
    obj8->SetFontSize(10);
    const JFontStyle obj8_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj8->SetFontStyle(obj8_style);

    JXImageWidget* key5 =
        new JXImageWidget(obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,70, 16,16);
    assert( key5 != NULL );

    JXStaticText* obj9 =
        new JXStaticText("Schedule", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,70, 49,15);
    assert( obj9 != NULL );
    obj9->SetFontSize(10);
    const JFontStyle obj9_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj9->SetFontStyle(obj9_style);

    JXImageWidget* key8 =
        new JXImageWidget(obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,90, 16,16);
    assert( key8 != NULL );

    JXStaticText* obj10 =
        new JXStaticText("Create", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,90, 55,15);
    assert( obj10 != NULL );
    obj10->SetFontSize(10);
    const JFontStyle obj10_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj10->SetFontStyle(obj10_style);

    JXImageWidget* key9 =
        new JXImageWidget(obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,110, 16,16);
    assert( key9 != NULL );

    JXStaticText* obj11 =
        new JXStaticText("Delete", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,110, 49,15);
    assert( obj11 != NULL );
    obj11->SetFontSize(10);
    const JFontStyle obj11_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj11->SetFontStyle(obj11_style);

    JXImageWidget* key10 =
        new JXImageWidget(obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,130, 16,16);
    assert( key10 != NULL );

    JXStaticText* obj12 =
        new JXStaticText("Admin", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,130, 49,15);
    assert( obj12 != NULL );
    obj12->SetFontSize(10);
    const JFontStyle obj12_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj12->SetFontStyle(obj12_style);

// end JXLayout1

	// Set icons for key
	key1->SetImage(CIconLoader::GetIcon(IDI_ACL_LOOKUP, key1, 16, 0x00CCCCCC),  kFalse);
	key2->SetImage(CIconLoader::GetIcon(IDI_ACL_READ, key2, 16, 0x00CCCCCC),  kFalse);
	key4->SetImage(CIconLoader::GetIcon(IDI_ACL_WRITE, key4, 16, 0x00CCCCCC),  kFalse);
	key5->SetImage(CIconLoader::GetIcon(IDI_ACL_POST, key5, 16, 0x00CCCCCC),  kFalse);
	key8->SetImage(CIconLoader::GetIcon(IDI_ACL_CREATE, key8, 16, 0x00CCCCCC),  kFalse);
	key9->SetImage(CIconLoader::GetIcon(IDI_ACL_DELETE, key9, 16, 0x00CCCCCC),  kFalse);
	key10->SetImage(CIconLoader::GetIcon(IDI_ACL_ADMIN, key10, 16, 0x00CCCCCC),  kFalse);

    sbs->SetBorderWidth(kJXDefaultBorderWidth);

	mACLTitleTable = new CACLTitleTable(sbs, sbs->GetScrollEnclosure(),
										 JXWidget::kHElastic,
										 JXWidget::kFixedTop,
										 0, 0, 100, cTitleHeight);

	mACLTable = new CACLTable(sbs, sbs->GetScrollEnclosure(),
										JXWidget::kHElastic,
										JXWidget::kVElastic,
										0,cTitleHeight, 100, 100);

	mACLTable->OnCreate();
	mACLTitleTable->OnCreate();

	mACLTable->Add_Listener(this);
	mACLTitleTable->SetDetails(false, false, true);

	// Create popup
	mACLStylePopup->Reset(true);

	mDeleteUserBtn->SetActive(kFalse);
	mACLStylePopup->SetActive(kFalse);

	ListenTo(mNewUserBtn);
	ListenTo(mDeleteUserBtn);
	ListenTo(mCalendarPopup);
	ListenTo(mACLStylePopup);
	ListenTo(mChangeAllBtn);
}

void CPropCalendarACL::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CTableDrag::eBroadcast_SelectionChanged:
		// Enable/disable Delete button
		mDeleteUserBtn->SetActive(mACLTable->IsSelectionValid());
		mACLStylePopup->SetActive(mACLTable->IsSelectionValid());
		break;
	default:
		CListener::ListenTo_Message(msg, param);
		break;
	}
}

// Handle buttons
void CPropCalendarACL::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXTextButton::kPushed))
	{
		if (sender == mNewUserBtn)
		{
			DoNewUser();
			return;
		}
		else if (sender == mDeleteUserBtn)
		{
			DoDeleteUser();
			return;
		}
		else if (sender == mChangeAllBtn)
		{
			DoChangeAll();
			return;
		}
	}
	else if (message.Is(JXMenu::kItemSelected))
	{
		if (sender == mCalendarPopup)
		{
			JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
			calstore::CCalendarStoreNode* cal = static_cast<calstore::CCalendarStoreNode*>(mCalList->at(index - 1));
			SetCal(cal);
			return;
		}
		else if (sender == mACLStylePopup)
		{
			JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
			DoStylePopup(index);
			return;
		}
	}

	CCalendarPropPanel::Receive(sender, message);
}

// Set mbox list
void CPropCalendarACL::SetCalList(calstore::CCalendarStoreNodeList* cal_list)
{
	// Save list
	mCalList = cal_list;

	// For now just use first item
	SetCal((calstore::CCalendarStoreNode*) mCalList->front());
	
	// Reset menu
	InitCalendarPopup();
	
	// Remove apply to all if only one
	if (mCalList->size() == 1)
		mChangeAllBtn->Hide();
}

// Set mbox list
void CPropCalendarACL::SetCal(calstore::CCalendarStoreNode* cal)
{
	// Cache actionable Calendar
	mActionCal = cal;

	// Set myrights
	SetMyRights(mActionCal->GetMyRights());

	// Set state of buttons - must have admin right and logged in account
	mCanChange = mActionCal->GetMyRights().HasRight(CCalendarACL::eCalACL_Admin);
	mNoChange = !mActionCal->GetProtocol()->IsLoggedOn();
	SetButtons(mCanChange, mNoChange);

	// Give list to table
	mACLTable->SetList(mActionCal->GetACLs(), mCanChange);
	mACLTitleTable->SyncTable(mACLTable, true);
}

// Initialise alert styles
void CPropCalendarACL::InitCalendarPopup()
{
	// Delete previous items
	mCalendarPopup->RemoveAllItems();

	// Add each alert style
	short menu_pos = 1;
	bool multi = calstore::CCalendarStoreManager::sCalendarStoreManager->HasMultipleProtocols();
	for(calstore::CCalendarStoreNodeList::const_iterator iter = mCalList->begin(); iter != mCalList->end(); iter++, menu_pos++)
		mCalendarPopup->AppendItem((*iter)->GetAccountName(multi), kFalse, kTrue);

	mCalendarPopup->SetValue(1);
}

// Set indicators from rights
void CPropCalendarACL::SetMyRights(SACLRight rights)
{
	mACLTable->SetMyRights(rights);
}

// Enable/disable buttons
void CPropCalendarACL::SetButtons(bool enable, bool alloff)
{
	mNewUserBtn->SetActive(enable && !alloff);
	mDeleteUserBtn->SetActive(enable && !alloff && mACLTable->IsSelectionValid());
	mACLStylePopup->SetActive(enable && !alloff && mACLTable->IsSelectionValid());

	mChangeAllBtn->SetActive(!alloff);
}

// Set protocol
void CPropCalendarACL::SetProtocol(calstore::CCalendarProtocol* proto)
{
	// Does nothing
}

// Force update of items
void CPropCalendarACL::ApplyChanges(void)
{
	// Nothing to update
}

// User clicked a button
void CPropCalendarACL::DoChangeACL(CCalendarACL::ECalACL acl, bool set)
{
	// Set this state in each selected ACL
	STableCell aCell(0, 0);
	while(mACLTable->GetNextSelectedCell(aCell))
	{
		if (aCell.col == 1)
		{
			// Copy existing ACL
			CCalendarACL old_acl = mActionCal->GetACLs()->at(aCell.row - 2);

			// Change rights in copy
			old_acl.SetRight(acl, set);

			// Change ACL at specific index
			mActionCal->SetACL(&old_acl, aCell.row - 2);
		}
	}

	// Redraw selection
	mACLTable->RefreshSelection();
}

// Change all rights
void CPropCalendarACL::DoChangeRights(SACLRight rights)
{
	// Set this state in each selected ACL
	STableCell aCell(0, 0);
	while(mACLTable->GetNextSelectedCell(aCell))
	{
		if (aCell.col == 1)
		{
			// Copy existing ACL
			CCalendarACL old_acl = mActionCal->GetACLs()->at(aCell.row - 2);

			// Change rights in copy
			old_acl.SetRights(rights);

			// Change ACL at specific index
			mActionCal->SetACL(&old_acl, aCell.row - 2);
		}
	}

	// Redraw selection
	mACLTable->RefreshSelection();
}

// Do new user dialog
void CPropCalendarACL::DoNewUser(void)
{
	// Create the dialog
	CCalendarACLList* acl_list = NULL;
	if (CNewACLDialog::PoseDialog(acl_list))
	{
		try
		{
			// Iterate over each new acl and add to Calendar
			for(CCalendarACLList::iterator iter = acl_list->begin(); iter != acl_list->end(); iter++)
			{
				// Add the new ACL
				mActionCal->SetACL(&(*iter), 0xFFFFFFFF);
			}
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Do not allow failure
		}
		delete acl_list;

		// Force complete reset of table
		mACLTable->SetList(mActionCal->GetACLs(), mCanChange);
	}

	// Force reset of popup
	mACLStylePopup->Reset(true);
}

// Delete user from list
void CPropCalendarACL::DoDeleteUser(void)
{
	// Do caution as this is not undoable
	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Server::DeleteAclWarn") == CErrorHandler::Cancel)
		return;

	// Get indices of ACLs to delete
	ulvector indices;
	STableCell aCell(0, 0);
	while(mACLTable->GetNextSelectedCell(aCell))
	{
		if (aCell.col == 1)
		{
			// Delete the one at this index
			indices.push_back(aCell.row - 2);
		}
	}

	// Delete all ACLs in one go
	mActionCal->DeleteACL(indices);

	// Force complete reset of table
	mACLTable->SetList(mActionCal->GetACLs(), mCanChange);

	// Force reset of popup
	mACLStylePopup->Reset(true);
}

// Handle popup command
void CPropCalendarACL::DoStylePopup(long index)
{
	switch(index)
	{
	// New ACL wanted
	case CACLStylePopup::eNewUser:
	{
		// Get first selected rights
		SACLRight rights;
		STableCell aCell(0, 0);
		while(mACLTable->GetNextSelectedCell(aCell))
		{
			if (aCell.col == 1)
			{
				// Copy existing ACL
				rights = mActionCal->GetACLs()->at(aCell.row - 2).GetRights();
				mACLStylePopup->DoNewStyle(rights);
				break;
			}
		}
		break;
	}

	// Delete existing ACL
	case CACLStylePopup::eDeleteUser:
		mACLStylePopup->DoDeleteStyle();
		break;

	// Select a style
	default:
		{
			// Get rights for style and set buttons
			SACLRight rights = CPreferences::sPrefs->mMboxACLStyles.GetValue()[index - 4].second;

			// Fake button hit to force update of selection and Calendar
			DoChangeRights(rights);
		}
		break;
	}
}

// Apply to all Calendares
void CPropCalendarACL::DoChangeAll(void)
{
	// Get entire rights for this Calendar
	CCalendarACLList* aList = mActionCal->GetACLs();
	
	// Do to each in list
	for(calstore::CCalendarStoreNodeList::const_iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
	{
		calstore::CCalendarStoreNode* cal = static_cast<calstore::CCalendarStoreNode*>(*iter);

		// Only if not same as current
		if (cal == mActionCal)
			continue;
		
		// Make sure have rights to change it
		if (!cal->GetMyRights().HasRight(CCalendarACL::eCalACL_Admin))
			continue;
		
		// Reset all ACLs
		cal->ResetACLs(aList);
	}
}
