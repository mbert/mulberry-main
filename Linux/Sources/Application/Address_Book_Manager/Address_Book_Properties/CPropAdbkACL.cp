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


// Source for CPropAdbkACL class

#include "CPropAdbkACL.h"

#include "CAdbkProtocol.h"
#include "CACLStylePopup.h"
#include "CACLTable.h"
#include "CACLTitleTable.h"
#include "CAdbkPropDialog.h"
#include "CErrorHandler.h"
#include "CIconLoader.h"
#include "CLog.h"
#include "CMulberryCommon.h"
#include "CNewACLDialog.h"
#include "CPreferences.h"
#include "CPropMailboxACL.h"
#include "CTableScrollbarSet.h"

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
void CPropAdbkACL::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Access Control", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 100,20);
    assert( obj1 != NULL );
    const JFontStyle obj1_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj1->SetFontStyle(obj1_style);

    JXDownRect* obj2 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 345,50, 100,135);
    assert( obj2 != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Key", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 350,43, 30,15);
    assert( obj3 != NULL );
    obj3->SetFontSize(10);
    const JFontStyle obj3_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj3->SetFontStyle(obj3_style);

    JXImageWidget* key1 =
        new JXImageWidget(obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 16,16);
    assert( key1 != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Lookup", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,10, 55,15);
    assert( obj4 != NULL );
    obj4->SetFontSize(10);
    const JFontStyle obj4_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj4->SetFontStyle(obj4_style);

    JXImageWidget* key2 =
        new JXImageWidget(obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,30, 16,16);
    assert( key2 != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Read", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,30, 49,15);
    assert( obj5 != NULL );
    obj5->SetFontSize(10);
    const JFontStyle obj5_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj5->SetFontStyle(obj5_style);

    JXImageWidget* key3 =
        new JXImageWidget(obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,50, 16,16);
    assert( key3 != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Write", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,50, 49,15);
    assert( obj6 != NULL );
    obj6->SetFontSize(10);
    const JFontStyle obj6_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj6->SetFontStyle(obj6_style);

    JXImageWidget* key4 =
        new JXImageWidget(obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,70, 16,16);
    assert( key4 != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Create", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,70, 55,15);
    assert( obj7 != NULL );
    obj7->SetFontSize(10);
    const JFontStyle obj7_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj7->SetFontStyle(obj7_style);

    JXImageWidget* key5 =
        new JXImageWidget(obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,90, 16,16);
    assert( key5 != NULL );

    JXStaticText* obj8 =
        new JXStaticText("Delete", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,90, 49,15);
    assert( obj8 != NULL );
    obj8->SetFontSize(10);
    const JFontStyle obj8_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj8->SetFontStyle(obj8_style);

    JXImageWidget* key6 =
        new JXImageWidget(obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,110, 16,16);
    assert( key6 != NULL );

    JXStaticText* obj9 =
        new JXStaticText("Admin", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,110, 49,15);
    assert( obj9 != NULL );
    obj9->SetFontSize(10);
    const JFontStyle obj9_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj9->SetFontStyle(obj9_style);

    CTableScrollbarSet* sbs =
        new CTableScrollbarSet(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,50, 325,195);
    assert( sbs != NULL );

    mNewUserBtn =
        new JXTextButton("Add User...", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,250, 90,25);
    assert( mNewUserBtn != NULL );
    mNewUserBtn->SetFontSize(10);

    mDeleteUserBtn =
        new JXTextButton("Remove User", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 120,250, 90,25);
    assert( mDeleteUserBtn != NULL );
    mDeleteUserBtn->SetFontSize(10);

    JXStaticText* obj10 =
        new JXStaticText("Style:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 220,254, 35,15);
    assert( obj10 != NULL );
    obj10->SetFontSize(10);

    mACLStylePopup =
        new CACLStylePopup("",this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 255,252, 35,20);
    assert( mACLStylePopup != NULL );

// end JXLayout1

	// Set icons for key
	key1->SetImage(CIconLoader::GetIcon(IDI_ACL_LOOKUP, key1, 16, 0x00CCCCCC),  kFalse);
	key2->SetImage(CIconLoader::GetIcon(IDI_ACL_READ, key2, 16, 0x00CCCCCC),  kFalse);
	key3->SetImage(CIconLoader::GetIcon(IDI_ACL_WRITE, key3, 16, 0x00CCCCCC),  kFalse);
	key4->SetImage(CIconLoader::GetIcon(IDI_ACL_CREATE, key4, 16, 0x00CCCCCC),  kFalse);
	key5->SetImage(CIconLoader::GetIcon(IDI_ACL_DELETE, key5, 16, 0x00CCCCCC),  kFalse);
	key6->SetImage(CIconLoader::GetIcon(IDI_ACL_ADMIN, key6, 16, 0x00CCCCCC),  kFalse);

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

	mACLTitleTable->SetDetails(false, true, false);

	// Create popup
	mACLStylePopup->Reset(false);

	mDeleteUserBtn->SetActive(kFalse);
	mACLStylePopup->SetActive(kFalse);

	ListenTo(mNewUserBtn);
	ListenTo(mDeleteUserBtn);
	ListenTo(mACLStylePopup);
}

// Handle buttons
void CPropAdbkACL::Receive(JBroadcaster* sender, const Message& message)
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
	}
	else if (message.Is(JXMenu::kItemSelected))
	{
		if (sender == mACLStylePopup)
		{
			JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
			DoStylePopup(index);
			return;
		}
	}

	CAdbkPropPanel::Receive(sender, message);
}

void CPropAdbkACL::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CTable::eBroadcast_SelectionChanged:
		// Enable/disable Delete button
		mDeleteUserBtn->SetActive(mACLTable->IsSelectionValid() ? kTrue : kFalse);
		mACLStylePopup->SetActive(mACLTable->IsSelectionValid() ? kTrue : kFalse);
		break;
	default:
		CListener::ListenTo_Message(msg, param);
		break;
	}
}

// Set adbk list
void CPropAdbkACL::SetAdbkList(CAddressBookList* adbk_list)
{
	// Save list
	mAdbkList = adbk_list;

	// For now just use first item
	SetAddressBook(mAdbkList->front());
}

// Set adbk list
void CPropAdbkACL::SetAddressBook(CAddressBook* adbk)
{
	// Set myrights
	SetMyRights(adbk->GetMyRights());

	// Set state of buttons
	mCanChange = adbk->GetMyRights().HasRight(CAdbkACL::eAdbkACL_Admin) &&
					adbk->GetProtocol()->IsLoggedOn();
	SetButtons(mCanChange);

	// Give list to table
	mACLTable->SetList(adbk->GetACLs(), mCanChange);
	mACLTitleTable->SyncTable(mACLTable, true);
}

// Set indicators from rights
void CPropAdbkACL::SetMyRights(SACLRight rights)
{
	mACLTable->SetMyRights(rights);
}

// Enable/disable buttons
void CPropAdbkACL::SetButtons(bool enable)
{
	mNewUserBtn->SetActive(enable ? kTrue : kFalse);
	mDeleteUserBtn->SetActive((enable && mACLTable->IsSelectionValid()) ? kTrue : kFalse);
	mACLStylePopup->SetActive((enable && mACLTable->IsSelectionValid()) ? kTrue : kFalse);
}

// Force update of items
void CPropAdbkACL::ApplyChanges(void)
{
	// Nothing to update
}

// User clicked a button
void CPropAdbkACL::DoChangeACL(CAdbkACL::EAdbkACL acl, bool set)
{
	// Only operate on first adbk for now...
	CAddressBook* adbk = mAdbkList->front();

	// Set this state in each selected ACL
	STableCell aCell(0, 0);
	while(mACLTable->GetNextSelectedCell(aCell))
	{
		if (aCell.col == 1)
		{
			// Copy existing ACL (adjust for my rights row)
			CAdbkACL old_acl = adbk->GetACLs()->at(aCell.row - 2);

			// Change rights in copy
			old_acl.SetRight(acl, set);

			// Get adbk to do change
			// adbk will replace if successful
			adbk->SetACL(&old_acl);
		}
	}

	// Redraw selection
	mACLTable->RefreshSelection();
}

// Change all rights
void CPropAdbkACL::DoChangeRights(SACLRight rights)
{
	// Only operate on first adbk for now...
	CAddressBook* adbk = mAdbkList->front();

	// Set this state in each selected ACL
	// Set this state in each selected ACL (not first row)
	STableCell aCell(0, 0);
	while(mACLTable->GetNextSelectedCell(aCell))
	{
		if (aCell.col == 1)
		{
			// Copy existing ACL (adjust for my rights row)
			CAdbkACL old_acl = adbk->GetACLs()->at(aCell.row - 2);

			// Change rights in copy
			old_acl.SetRights(rights);

			// Get adbk to do change
			// adbk will replace if successful
			adbk->SetACL(&old_acl);
		}
	}

	// Redraw selection
	mACLTable->RefreshSelection();
}

// Do new user dialog
void CPropAdbkACL::DoNewUser(void)
{
	// Create the dialog
	CAdbkACLList* acl_list = NULL;
	if (CNewACLDialog::PoseDialog(acl_list))
	{
		// Only operate on first adbk for now...
		CAddressBook* adbk = mAdbkList->front();

		try
		{
			// Iterate over each new acl and add to mailbox
			for(CAdbkACLList::iterator iter = acl_list->begin(); iter != acl_list->end(); iter++)
			{
				// Get mbox to do change
				// mbox will replace if successful
				adbk->SetACL(&(*iter));
			}
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Do not allow failure
		}
		delete acl_list;

		// Force complete reset of table
		mACLTable->SetList(adbk->GetACLs(), mCanChange);
	}

	// Force reset of popup
	mACLStylePopup->Reset(false);
}

// Delete user from list
void CPropAdbkACL::DoDeleteUser(void)
{
	// Do caution as this is not undoable
	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Server::DeleteAclWarn") == CErrorHandler::Cancel)
		return;

	// Only operate on first adbk for now...
	CAddressBook* adbk = mAdbkList->front();

	// Delete ACLs in reverse
	STableCell aCell(0, 0);
	while(mACLTable->GetPreviousSelectedCell(aCell))
	{
		if (aCell.col == 1)
		{
			// Copy existing ACL (adjust for my rights row)
			CAdbkACL old_acl = adbk->GetACLs()->at(aCell.row - 2);

			// Get adbk to do change
			// adbk will delete if successful
			adbk->DeleteACL(&old_acl);
		}
	}

	// Force complete reset of table
	mACLTable->SetList(adbk->GetACLs(), mCanChange);

	// Force reset of popup
	mACLStylePopup->Reset(false);
}

// Handle popup command
void CPropAdbkACL::DoStylePopup(JIndex index)
{
	switch(index)
	{
	// New ACL wanted
	case CACLStylePopup::eNewUser:
	{
		// Only operate on first Adbk for now...
		CAddressBook* adbk = mAdbkList->front();

		// Get first selected rights
		SACLRight rights;
		STableCell aCell(0, 0);
		while(mACLTable->GetNextSelectedCell(aCell))
		{
			if (aCell.col == 1)
			{
				// Copy existing ACL (adjust for my rights row)
				rights = adbk->GetACLs()->at(aCell.row - 2).GetRights();
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
			SACLRight rights = CPreferences::sPrefs->mAdbkACLStyles.GetValue()[index - 4].second;

			// Fake button hit to force update of selection and mailbox
			DoChangeRights(rights);
		}
		break;
	}
}
