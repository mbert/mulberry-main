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
#include "CBalloonDialog.h"
#include "CErrorHandler.h"
#include "CLog.h"
#include "CMulberryCommon.h"
#include "CNewACLDialog.h"
#include "CMbox.h"
#include "CMailAccountManager.h"
#include "CMailboxPropDialog.h"
#include "CMboxProtocol.h"
#include "CPreferences.h"
#include "CResources.h"

#include <LIconControl.h>
#include <LPopupButton.h>
#include <LPushButton.h>



// __________________________________________________________________________________________________
// C L A S S __ C P R E F S N E T W O R K
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPropMailboxACL::CPropMailboxACL()
{
	mCanChange = false;
	mNoChange = false;
	mActionMbox = NULL;
}

// Constructor from stream
CPropMailboxACL::CPropMailboxACL(LStream *inStream)
		: CMailboxPropPanel(inStream)
{
	mCanChange = false;
	mNoChange = false;
	mActionMbox = NULL;
}

// Default destructor
CPropMailboxACL::~CPropMailboxACL()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPropMailboxACL::FinishCreateSelf(void)
{
	// Do inherited
	CMailboxPropPanel::FinishCreateSelf();

	// Get items
	mMailboxPopup = (LPopupButton*) FindPaneByID(paneid_MailboxACLMailboxPopup);

	mLookupMark = (LIconControl*) FindPaneByID(paneid_MailboxACLLookupMark);
	mReadMark = (LIconControl*) FindPaneByID(paneid_MailboxACLReadMark);
	mSeenMark = (LIconControl*) FindPaneByID(paneid_MailboxACLSeenMark);
	mWriteMark = (LIconControl*) FindPaneByID(paneid_MailboxACLWriteMark);
	mInsertMark = (LIconControl*) FindPaneByID(paneid_MailboxACLInsertMark);
	mPostMark = (LIconControl*) FindPaneByID(paneid_MailboxACLPostMark);
	mCreateMark = (LIconControl*) FindPaneByID(paneid_MailboxACLCreateMark);
	mDeleteMark = (LIconControl*) FindPaneByID(paneid_MailboxACLDeleteMark);
	mAdminMark = (LIconControl*) FindPaneByID(paneid_MailboxACLAdminMark);

	mACLTable = (CACLTable*) FindPaneByID(paneid_MailboxACLTable);
	mACLTable->Add_Listener(this);

	mNewUserBtn = (LPushButton*) FindPaneByID(paneid_MailboxACLNewUserBtn);
	mDeleteUserBtn = (LPushButton*) FindPaneByID(paneid_MailboxACLDeleteUserBtn);
	mDeleteUserBtn->Disable();
	mChangeAllBtn = (LPushButton*) FindPaneByID(paneid_MailboxACLChangeAllBtn);

	// Create popup
	mStylePopup = (CACLStylePopup*) FindPaneByID(paneid_MailboxACLStylePopup);
	mStylePopup->Disable();
	mStylePopup->Reset(true);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPropMailboxACLBtns);
}

void CPropMailboxACL::ListenTo_Message(long msg, void* param)
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
void CPropMailboxACL::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_ACLMailbox:
			long index = *(long*) ioParam;
			CMbox* mbox = static_cast<CMbox*>(mMboxList->at(index - 1));
			SetMbox(mbox);
			break;

		case msg_ACLNewUser:
			DoNewUser();
			break;

		case msg_ACLDeleteUser:
			DoDeleteUser();
			break;

		case msg_ACLStylePopup:
			DoStylePopup(*(long*) ioParam);
			break;

		case msg_ACLChangeAll:
			DoChangeAll();
			break;
	}
}

// Set mbox list
void CPropMailboxACL::SetMboxList(CMboxList* mbox_list)
{
	// Save list
	mMboxList = mbox_list;

	// For now just use first item
	SetMbox((CMbox*) mMboxList->front());
	
	// Reset menu
	// Delete previous items
	MenuHandle menuH = mMailboxPopup->GetMacMenuH();
	for(short i = ::CountMenuItems(menuH); i > 0; i--)
		::DeleteMenuItem(menuH, i);

	// Add each mailbox item
	short menu_pos = 1;
	bool multi = (CMailAccountManager::sMailAccountManager->GetProtocolList().size() > 1);
	for(CMboxList::const_iterator iter = mMboxList->begin(); iter != mMboxList->end(); iter++, menu_pos++)
		::AppendItemToMenu(menuH, menu_pos, (*iter)->GetAccountName(multi));

	// Force max/min update
	mMailboxPopup->SetMenuMinMax();
	
	// Remove apply to all if only one
	if (mMboxList->size() == 1)
		mChangeAllBtn->Hide();
}

// Set mbox list
void CPropMailboxACL::SetMbox(CMbox* mbox)
{
	// Cache actionable mailbox
	mActionMbox = mbox;

	// Set myrights
	SetMyRights(mActionMbox->GetMyRights());

	// Set state of buttons - must have admin right and logged in account
	mCanChange = mActionMbox->GetMyRights().HasRight(CMboxACL::eMboxACL_Admin);
	mNoChange = !mActionMbox->GetProtocol()->IsLoggedOn();
	SetButtons(mCanChange, mNoChange);

	// Give list to table
	mACLTable->SetList(mActionMbox->GetACLs(), mCanChange);
}

// Set indicators from rights
void CPropMailboxACL::SetMyRights(SACLRight rights)
{
	mLookupMark->SetResourceID(rights.HasRight(CMboxACL::eMboxACL_Lookup) ? ICNx_ACLSet : ICNx_ACLUnset);
	mLookupMark->Refresh();
	mReadMark->SetResourceID(rights.HasRight(CMboxACL::eMboxACL_Read) ? ICNx_ACLSet : ICNx_ACLUnset);
	mReadMark->Refresh();
	mSeenMark->SetResourceID(rights.HasRight(CMboxACL::eMboxACL_Seen) ? ICNx_ACLSet : ICNx_ACLUnset);
	mSeenMark->Refresh();
	mWriteMark->SetResourceID(rights.HasRight(CMboxACL::eMboxACL_Write) ? ICNx_ACLSet : ICNx_ACLUnset);
	mWriteMark->Refresh();
	mInsertMark->SetResourceID(rights.HasRight(CMboxACL::eMboxACL_Insert) ? ICNx_ACLSet : ICNx_ACLUnset);
	mInsertMark->Refresh();
	mPostMark->SetResourceID(rights.HasRight(CMboxACL::eMboxACL_Post) ? ICNx_ACLSet : ICNx_ACLUnset);
	mPostMark->Refresh();
	mCreateMark->SetResourceID(rights.HasRight(CMboxACL::eMboxACL_Create) ? ICNx_ACLSet : ICNx_ACLUnset);
	mCreateMark->Refresh();
	mDeleteMark->SetResourceID(rights.HasRight(CMboxACL::eMboxACL_Delete) ? ICNx_ACLSet : ICNx_ACLUnset);
	mDeleteMark->Refresh();
	mAdminMark->SetResourceID(rights.HasRight(CMboxACL::eMboxACL_Admin) ? ICNx_ACLSet : ICNx_ACLUnset);
	mAdminMark->Refresh();
}

// Enable/disable buttons
void CPropMailboxACL::SetButtons(bool enable, bool alloff)
{
	mNewUserBtn->SetEnabled(enable && !alloff);
	mDeleteUserBtn->SetEnabled(enable && !alloff && mACLTable->IsSelectionValid());
	mStylePopup->SetEnabled(enable && !alloff && mACLTable->IsSelectionValid());
	mChangeAllBtn->SetEnabled(!alloff);
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
			// Copy existing ACL
			CMboxACL old_acl = mActionMbox->GetACLs()->at(aCell.row - 1);

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
			// Copy existing ACL
			CMboxACL old_acl = mActionMbox->GetACLs()->at(aCell.row - 1);

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
	CMailboxPropDialog* cmdr = (CMailboxPropDialog*) GetSuperView()->GetSuperView();
	CBalloonDialog	theHandler(paneid_NewACLDialog, cmdr);
	((CNewACLDialog*) theHandler.GetDialog())->SetDetails(true, false, false);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true) {
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			CMboxACLList* acl_list = ((CNewACLDialog*) theHandler.GetDialog())->GetDetailsMbox();

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
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}

	// Force reset of popup
	mStylePopup->Reset(true);
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
			// Copy existing ACL
			CMboxACL old_acl = mActionMbox->GetACLs()->at(aCell.row - 1);

			// Get mbox to do change
			// mbox will delete if successful
			mActionMbox->DeleteACL(&old_acl);
		}
	}

	// Force complete reset of table
	mACLTable->SetList(mActionMbox->GetACLs(), mCanChange);

	// Force reset of popup
	mStylePopup->Reset(true);
}

// Handle popup command
void CPropMailboxACL::DoStylePopup(long index)
{
	switch(index)
	{
	// New ACL wanted
	case eACLStyleNewUser:
	{
		// Get first selected rights
		SACLRight rights;
		STableCell aCell(0, 0);
		while(mACLTable->GetNextSelectedCell(aCell))
		{
			if (aCell.col == 1)
			{
				// Copy existing ACL
				rights = mActionMbox->GetACLs()->at(aCell.row - 1).GetRights();
				CMailboxPropDialog* cmdr = (CMailboxPropDialog*) GetSuperView()->GetSuperView();
				mStylePopup->DoNewStyle(rights);
				break;
			}
		}
		break;
	}

	// Delete existing ACL
	case eACLStyleDeleteUser:
		CMailboxPropDialog* cmdr = (CMailboxPropDialog*) GetSuperView()->GetSuperView();
		mStylePopup->DoDeleteStyle();
		break;

	// Select a style
	default:
		{
			// Get rights for style and set buttons
			SACLRight rights = CPreferences::sPrefs->mMboxACLStyles.GetValue()[index - 4].second;

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
