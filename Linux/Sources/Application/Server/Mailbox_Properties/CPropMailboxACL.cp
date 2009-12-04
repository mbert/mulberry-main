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


// Source for CPropMailboxACL class

#include "CPropMailboxACL.h"

#include "CACLStylePopup.h"
#include "CACLTable.h"
#include "CACLTitleTable.h"
#include "CErrorHandler.h"
#include "CIconLoader.h"
#include "CLog.h"
#include "CMailAccountManager.h"
#include "CMailboxPropDialog.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMulberryCommon.h"
#include "CNewACLDialog.h"
#include "CPreferences.h"
#include "CSimpleTitleTable.h"
#include "CTableScrollbarSet.h"

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
void CPropMailboxACL::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Access Control", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 100,20);
    assert( obj1 != NULL );
    const JFontStyle obj1_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj1->SetFontStyle(obj1_style);

    JXStaticText* obj2 =
        new JXStaticText("Mailbox:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,24, 60,20);
    assert( obj2 != NULL );

    mMailboxPopup =
        new HPopupMenu("",this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 65,21, 270,25);
    assert( mMailboxPopup != NULL );

    JXDownRect* obj3 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 345,50, 100,195);
    assert( obj3 != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Key", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 350,43, 30,15);
    assert( obj4 != NULL );
    obj4->SetFontSize(10);
    const JFontStyle obj4_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj4->SetFontStyle(obj4_style);

    JXImageWidget* key1 =
        new JXImageWidget(obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 16,16);
    assert( key1 != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Lookup", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,10, 55,15);
    assert( obj5 != NULL );
    obj5->SetFontSize(10);
    const JFontStyle obj5_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj5->SetFontStyle(obj5_style);

    JXImageWidget* key2 =
        new JXImageWidget(obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,30, 16,16);
    assert( key2 != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Read", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,30, 49,15);
    assert( obj6 != NULL );
    obj6->SetFontSize(10);
    const JFontStyle obj6_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj6->SetFontStyle(obj6_style);

    JXImageWidget* key3 =
        new JXImageWidget(obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,50, 16,16);
    assert( key3 != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Seen", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,50, 49,15);
    assert( obj7 != NULL );
    obj7->SetFontSize(10);
    const JFontStyle obj7_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj7->SetFontStyle(obj7_style);

    JXImageWidget* key4 =
        new JXImageWidget(obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,70, 16,16);
    assert( key4 != NULL );

    JXStaticText* obj8 =
        new JXStaticText("Write", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,70, 49,15);
    assert( obj8 != NULL );
    obj8->SetFontSize(10);
    const JFontStyle obj8_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj8->SetFontStyle(obj8_style);

    JXImageWidget* key5 =
        new JXImageWidget(obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,90, 16,16);
    assert( key5 != NULL );

    JXStaticText* obj9 =
        new JXStaticText("Add", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,90, 49,15);
    assert( obj9 != NULL );
    obj9->SetFontSize(10);
    const JFontStyle obj9_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj9->SetFontStyle(obj9_style);

    JXImageWidget* key6 =
        new JXImageWidget(obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,110, 16,16);
    assert( key6 != NULL );

    JXStaticText* obj10 =
        new JXStaticText("Post", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,110, 49,15);
    assert( obj10 != NULL );
    obj10->SetFontSize(10);
    const JFontStyle obj10_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj10->SetFontStyle(obj10_style);

    JXImageWidget* key7 =
        new JXImageWidget(obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,130, 16,16);
    assert( key7 != NULL );

    JXStaticText* obj11 =
        new JXStaticText("Create", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,130, 55,15);
    assert( obj11 != NULL );
    obj11->SetFontSize(10);
    const JFontStyle obj11_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj11->SetFontStyle(obj11_style);

    JXImageWidget* key8 =
        new JXImageWidget(obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,150, 16,16);
    assert( key8 != NULL );

    JXStaticText* obj12 =
        new JXStaticText("Delete", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,150, 49,15);
    assert( obj12 != NULL );
    obj12->SetFontSize(10);
    const JFontStyle obj12_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj12->SetFontStyle(obj12_style);

    JXImageWidget* key9 =
        new JXImageWidget(obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,170, 16,16);
    assert( key9 != NULL );

    JXStaticText* obj13 =
        new JXStaticText("Admin", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 35,170, 49,15);
    assert( obj13 != NULL );
    obj13->SetFontSize(10);
    const JFontStyle obj13_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj13->SetFontStyle(obj13_style);

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

    JXStaticText* obj14 =
        new JXStaticText("Style:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 220,254, 35,15);
    assert( obj14 != NULL );
    obj14->SetFontSize(10);

    mACLStylePopup =
        new CACLStylePopup("",this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 255,252, 35,20);
    assert( mACLStylePopup != NULL );

    mChangeAllBtn =
        new JXTextButton("Apply to All Selected Mailboxes", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,280, 195,25);
    assert( mChangeAllBtn != NULL );
    mChangeAllBtn->SetFontSize(10);

// end JXLayout1

	// Set icons for key
	key1->SetImage(CIconLoader::GetIcon(IDI_ACL_LOOKUP, key1, 16, 0x00CCCCCC),  kFalse);
	key2->SetImage(CIconLoader::GetIcon(IDI_ACL_READ, key2, 16, 0x00CCCCCC),  kFalse);
	key3->SetImage(CIconLoader::GetIcon(IDI_ACL_SEEN, key3, 16, 0x00CCCCCC),  kFalse);
	key4->SetImage(CIconLoader::GetIcon(IDI_ACL_WRITE, key4, 16, 0x00CCCCCC),  kFalse);
	key5->SetImage(CIconLoader::GetIcon(IDI_ACL_INSERT, key5, 16, 0x00CCCCCC),  kFalse);
	key6->SetImage(CIconLoader::GetIcon(IDI_ACL_POST, key6, 16, 0x00CCCCCC),  kFalse);
	key7->SetImage(CIconLoader::GetIcon(IDI_ACL_CREATE, key7, 16, 0x00CCCCCC),  kFalse);
	key8->SetImage(CIconLoader::GetIcon(IDI_ACL_DELETE, key8, 16, 0x00CCCCCC),  kFalse);
	key9->SetImage(CIconLoader::GetIcon(IDI_ACL_ADMIN, key9, 16, 0x00CCCCCC),  kFalse);

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
	mACLTitleTable->SetDetails(true, false, false);

	// Create popup
	mACLStylePopup->Reset(true);

	mDeleteUserBtn->SetActive(kFalse);
	mACLStylePopup->SetActive(kFalse);

	ListenTo(mNewUserBtn);
	ListenTo(mDeleteUserBtn);
	ListenTo(mMailboxPopup);
	ListenTo(mACLStylePopup);
	ListenTo(mChangeAllBtn);
}

void CPropMailboxACL::ListenTo_Message(long msg, void* param)
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

// Handle buttons
void CPropMailboxACL::Receive(JBroadcaster* sender, const Message& message)
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
		if (sender == mMailboxPopup)
		{
			JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
			CMbox* mbox = static_cast<CMbox*>(mMboxList->at(index - 1));
			SetMbox(mbox);
			return;
		}
		else if (sender == mACLStylePopup)
		{
			JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
			DoStylePopup(index);
			return;
		}
	}

	CMailboxPropPanel::Receive(sender, message);
}

// Set mbox list
void CPropMailboxACL::SetMboxList(CMboxList* mbox_list)
{
	// Save list
	mMboxList = mbox_list;

	// For now just use first item
	SetMbox((CMbox*) mMboxList->front());
	
	// Reset menu
	InitMailboxPopup();
	
	if (mMboxList->size() == 1)
		mChangeAllBtn->SetVisible(kFalse);
}

// Set mbox list
void CPropMailboxACL::SetMbox(CMbox* mbox)
{
	// Cache actionable mailbox
	mActionMbox = mbox;

	// Set myrights
	SetMyRights(mActionMbox->GetMyRights());

	// Set state of buttons
	mCanChange = mbox->GetMyRights().HasRight(CMboxACL::eMboxACL_Admin);
	mNoChange = !mActionMbox->GetProtocol()->IsLoggedOn();
	SetButtons(mCanChange, mNoChange);

	// Give list to table
	mACLTable->SetList(mActionMbox->GetACLs(), mCanChange);
	mACLTitleTable->SyncTable(mACLTable, true);
}

// Initialise alert styles
void CPropMailboxACL::InitMailboxPopup()
{
	// Delete previous items
	mMailboxPopup->RemoveAllItems();

	// Add each alert style
	bool multi = (CMailAccountManager::sMailAccountManager->GetProtocolList().size() > 1);
	for(CMboxList::const_iterator iter = mMboxList->begin(); iter != mMboxList->end(); iter++)
		mMailboxPopup->AppendItem((*iter)->GetAccountName(multi), kFalse, kTrue);

	mMailboxPopup->SetValue(1);
}

// Set indicators from rights
void CPropMailboxACL::SetMyRights(SACLRight rights)
{
	mACLTable->SetMyRights(rights);
}

// Enable/disable buttons
void CPropMailboxACL::SetButtons(bool enable, bool alloff)
{
	mNewUserBtn->SetActive((enable && !alloff) ? kTrue : kFalse);
	mDeleteUserBtn->SetActive((enable && !alloff && mACLTable->IsSelectionValid()) ? kTrue : kFalse);
	mACLStylePopup->SetActive((enable && !alloff && mACLTable->IsSelectionValid()) ? kTrue : kFalse);
	
	mChangeAllBtn->SetActive(!alloff ? kTrue : kFalse);
}

// Set protocol
void CPropMailboxACL::SetProtocol(CMboxProtocol* protocol)
{
	// Does nothing
}

// Force update of items
void CPropMailboxACL::ApplyChanges(void)
{
	// Nothing to update
}

// User clicked a button
void CPropMailboxACL::DoChangeACL(CMboxACL::EMboxACL acl, bool set)
{
	// Set this state in each selected ACL
	STableCell aCell(0, 0);
	while(mACLTable->GetNextSelectedCell(aCell))
	{
		if (aCell.col == 1)
		{
			// Copy existing ACL (adjust for my rights row)
			CMboxACL old_acl = mActionMbox->GetACLs()->at(aCell.row - 2);

			// Change rights in copy
			old_acl.SetRight(acl, set);

			// Get mbox to do change
			// mbox will replace if successful
			mActionMbox->SetACL(&old_acl);
		}
	}

	// Redraw selection
	mACLTable->RefreshSelection();
}

// Change all rights
void CPropMailboxACL::DoChangeRights(SACLRight rights)
{
	// Set this state in each selected ACL
	STableCell aCell(0, 0);
	while(mACLTable->GetNextSelectedCell(aCell))
	{
		if (aCell.col == 1)
		{
			// Copy existing ACL (adjust for my rights row)
			CMboxACL old_acl = mActionMbox->GetACLs()->at(aCell.row - 2);

			// Change rights in copy
			old_acl.SetRights(rights);

			// Get mbox to do change
			// mbox will replace if successful
			mActionMbox->SetACL(&old_acl);
		}
	}

	// Redraw selection
	mACLTable->RefreshSelection();
}

// Do new user dialog
void CPropMailboxACL::DoNewUser(void)
{
	// Create the dialog
	CMboxACLList* acl_list = NULL;
	if (CNewACLDialog::PoseDialog(acl_list))
	{
		try
		{
			// Iterate over each new acl and add to mailbox
			for(CMboxACLList::iterator iter = acl_list->begin(); iter != acl_list->end(); iter++)
			{
				// Get mbox to do change
				// mbox will replace if successful
				mActionMbox->SetACL(&(*iter));
			}
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Do not allow failure
		}
		delete acl_list;

		// Force complete reset of table
		mACLTable->SetList(mActionMbox->GetACLs(), mCanChange);
	}

	// Force reset of popup
	mACLStylePopup->Reset(true);
}

// Delete user from list
void CPropMailboxACL::DoDeleteUser(void)
{
	// Do caution as this is not undoable
	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Server::DeleteAclWarn") == CErrorHandler::Cancel)
		return;

	// Delete ACLs in reverse
	STableCell aCell(0, 0);
	while(mACLTable->GetPreviousSelectedCell(aCell))
	{
		if (aCell.col == 1)
		{
			// Copy existing ACL (adjust for my rights row)
			CMboxACL old_acl = mActionMbox->GetACLs()->at(aCell.row - 2);

			// Get mbox to do change
			// mbox will delete if successful
			mActionMbox->DeleteACL(&old_acl);
		}
	}

	// Force complete reset of table
	mACLTable->SetList(mActionMbox->GetACLs(), mCanChange);

	// Force reset of popup
	mACLStylePopup->Reset(true);
}

// Handle popup command
void CPropMailboxACL::DoStylePopup(JIndex index)
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
				// Copy existing ACL (adjust for my rights row)
				rights = mActionMbox->GetACLs()->at(aCell.row - 2).GetRights();
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
			SACLRight rights = CPreferences::sPrefs->mMboxACLStyles.GetValue()[index - CACLStylePopup::eFirstStyle].second;

			// Fake button hit to force update of selection and mailbox
			DoChangeRights(rights);
		}
		break;
	}
}

// Apply to all mailboxes
void CPropMailboxACL::DoChangeAll(void)
{
	// Get entire rights for this mailbox
	CMboxACLList* aList = mActionMbox->GetACLs();
	
	// Do to each in list
	for(CMboxList::const_iterator iter = mMboxList->begin(); iter != mMboxList->end(); iter++)
	{
		CMbox* mbox = static_cast<CMbox*>(*iter);

		// Only if not same as current
		if (mbox == mActionMbox)
			continue;
		
		// Make sure have rights to change it
		if (!mbox->GetMyRights().HasRight(CMboxACL::eMboxACL_Admin))
			continue;
		
		// Reset all ACLs
		mbox->ResetACLs(aList);
	}
}
