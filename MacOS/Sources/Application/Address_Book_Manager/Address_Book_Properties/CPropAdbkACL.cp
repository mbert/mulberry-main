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

#include "CACLStylePopup.h"
#include "CACLTable.h"
#include "CAdbkPropDialog.h"
#include "CAdbkProtocol.h"
#include "CBalloonDialog.h"
#include "CErrorHandler.h"
#include "CMailboxPropDialog.h"
#include "CMulberryCommon.h"
#include "CNewACLDialog.h"
#include "CPreferences.h"
#include "CPropMailboxACL.h"
#include "CResources.h"

#include <LIconControl.h>
#include <LPushButton.h>



// __________________________________________________________________________________________________
// C L A S S __ C P R E F S N E T W O R K
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPropAdbkACL::CPropAdbkACL()
{
	mCanChange = false;
}

// Constructor from stream
CPropAdbkACL::CPropAdbkACL(LStream *inStream)
		: CAdbkPropPanel(inStream)
{
	mCanChange = false;
}

// Default destructor
CPropAdbkACL::~CPropAdbkACL()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPropAdbkACL::FinishCreateSelf(void)
{
	// Do inherited
	CAdbkPropPanel::FinishCreateSelf();

	// Get items
	mLookupMark = (LIconControl*) FindPaneByID(paneid_AdbkACLLookupMark);
	mReadMark = (LIconControl*) FindPaneByID(paneid_AdbkACLReadMark);
	mWriteMark = (LIconControl*) FindPaneByID(paneid_AdbkACLWriteMark);
	mCreateMark = (LIconControl*) FindPaneByID(paneid_AdbkACLCreateMark);
	mDeleteMark = (LIconControl*) FindPaneByID(paneid_AdbkACLDeleteMark);
	mAdminMark = (LIconControl*) FindPaneByID(paneid_AdbkACLAdminMark);

	mACLTable = (CACLTable*) FindPaneByID(paneid_AdbkACLTable);
	mACLTable->Add_Listener(this);

	mNewUserBtn = (LPushButton*) FindPaneByID(paneid_AdbkACLNewUserBtn);
	mDeleteUserBtn = (LPushButton*) FindPaneByID(paneid_AdbkACLDeleteUserBtn);
	mDeleteUserBtn->Disable();

	// Create popup
	mStylePopup = (CACLStylePopup*) FindPaneByID(paneid_AdbkACLStylePopup);
	mStylePopup->Disable();
	mStylePopup->Reset(false);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPropAdbkACLBtns);
}

void CPropAdbkACL::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CTableDrag::eBroadcast_SelectionChanged:
		// Enable/disable Delete button
		mDeleteUserBtn->SetEnabled(mACLTable->IsSelectionValid());
		mStylePopup->SetEnabled(mACLTable->IsSelectionValid());
		break;
	default:
		CListener::ListenTo_Message(msg, param);
		break;
	}
}

// Handle buttons
void CPropAdbkACL::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_ACLAdbkNewUser:
			DoNewUser();
			break;

		case msg_ACLAdbkDeleteUser:
			DoDeleteUser();
			break;

		case msg_ACLAdbkStylePopup:
			DoStylePopup(*(long*) ioParam);
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
}

// Set indicators from rights
void CPropAdbkACL::SetMyRights(SACLRight rights)
{
	mLookupMark->SetResourceID(rights.HasRight(CAdbkACL::eAdbkACL_Lookup) ? ICNx_ACLSet : ICNx_ACLUnset);
	mLookupMark->Refresh();
	mReadMark->SetResourceID(rights.HasRight(CAdbkACL::eAdbkACL_Read) ? ICNx_ACLSet : ICNx_ACLUnset);
	mReadMark->Refresh();
	mWriteMark->SetResourceID(rights.HasRight(CAdbkACL::eAdbkACL_Write) ? ICNx_ACLSet : ICNx_ACLUnset);
	mWriteMark->Refresh();
	mCreateMark->SetResourceID(rights.HasRight(CAdbkACL::eAdbkACL_Create) ? ICNx_ACLSet : ICNx_ACLUnset);
	mCreateMark->Refresh();
	mDeleteMark->SetResourceID(rights.HasRight(CAdbkACL::eAdbkACL_Delete) ? ICNx_ACLSet : ICNx_ACLUnset);
	mDeleteMark->Refresh();
	mAdminMark->SetResourceID(rights.HasRight(CAdbkACL::eAdbkACL_Admin) ? ICNx_ACLSet : ICNx_ACLUnset);
	mAdminMark->Refresh();
}

// Enable/disable buttons
void CPropAdbkACL::SetButtons(bool enable)
{
	mNewUserBtn->SetEnabled(enable);
	mDeleteUserBtn->SetEnabled(enable && mACLTable->IsSelectionValid());
	mStylePopup->SetEnabled(enable && mACLTable->IsSelectionValid());
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
			// Copy existing ACL
			CAdbkACL old_acl = adbk->GetACLs()->at(aCell.row - 1);

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
	STableCell aCell(0, 0);
	while(mACLTable->GetNextSelectedCell(aCell))
	{
		if (aCell.col == 1)
		{
			// Copy existing ACL
			CAdbkACL old_acl = adbk->GetACLs()->at(aCell.row - 1);

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
	CAdbkPropDialog* cmdr = (CAdbkPropDialog*) GetSuperView()->GetSuperView();
	CBalloonDialog	theHandler(paneid_NewACLDialog, cmdr);
	((CNewACLDialog*) theHandler.GetDialog())->SetDetails(false, true, false);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true) {
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			// Only operate on first adbk for now...
			CAddressBook* adbk = mAdbkList->front();

			CAdbkACLList* acl_list = ((CNewACLDialog*) theHandler.GetDialog())->GetDetailsAdbk();

			// Iterate over each new acl and add to Adbk
			for(CAdbkACLList::iterator iter = acl_list->begin(); iter != acl_list->end(); iter++)
			{
				// Get adbk to do change
				// adbk will replace if successful
				adbk->SetACL(&(*iter));
			}

			// Force complete reset of table
			mACLTable->SetList(adbk->GetACLs(), mCanChange);
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}

	// Force reset of popup
	mStylePopup->Reset(false);
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
			// Copy existing ACL
			CAdbkACL old_acl = adbk->GetACLs()->at(aCell.row - 1);

			// Get adbk to do change
			// adbk will delete if successful
			adbk->DeleteACL(&old_acl);
		}
	}

	// Force complete reset of table
	mACLTable->SetList(adbk->GetACLs(), mCanChange);

	// Force reset of popup
	mStylePopup->Reset(false);
}

// Handle popup command
void CPropAdbkACL::DoStylePopup(long index)
{
	switch(index)
	{
	// New ACL wanted
	case eACLStyleNewUser:
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
				// Copy existing ACL
				rights = adbk->GetACLs()->at(aCell.row - 1).GetRights();
				CAdbkPropDialog* cmdr = (CAdbkPropDialog*) GetSuperView()->GetSuperView();
				mStylePopup->DoNewStyle(rights);
				break;
			}
		}
		break;
	}

	// Delete existing ACL
	case eACLStyleDeleteUser:
		CAdbkPropDialog* cmdr = (CAdbkPropDialog*) GetSuperView()->GetSuperView();
		mStylePopup->DoDeleteStyle();
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
