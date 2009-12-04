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
#include "CBalloonDialog.h"
#include "CErrorHandler.h"
#include "CLog.h"
#include "CMulberryCommon.h"
#include "CNewACLDialog.h"
#include "CCalendarPropDialog.h"
#include "CPreferences.h"
#include "CResources.h"

#include <LIconControl.h>
#include <LPopupButton.h>
#include <LPushButton.h>

#include "CCalendarStoreManager.h"
#include "CCalendarProtocol.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CPropCalendarACL::CPropCalendarACL(LStream *inStream)
		: CCalendarPropPanel(inStream)
{
	mCanChange = false;
	mNoChange = false;
	mActionCal = NULL;
}

// Default destructor
CPropCalendarACL::~CPropCalendarACL()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPropCalendarACL::FinishCreateSelf(void)
{
	// Do inherited
	CCalendarPropPanel::FinishCreateSelf();

	// Get items
	mCalendarPopup = (LPopupButton*) FindPaneByID(paneid_CalendarACLCalendarPopup);

	mReadFreeBusyMark = (LIconControl*) FindPaneByID(paneid_CalendarACLReadFreeBusyMark);
	mReadMark = (LIconControl*) FindPaneByID(paneid_CalendarACLReadMark);
	mWriteMark = (LIconControl*) FindPaneByID(paneid_CalendarACLWriteMark);
	mScheduleMark = (LIconControl*) FindPaneByID(paneid_CalendarACLScheduleMark);
	mCreateMark = (LIconControl*) FindPaneByID(paneid_CalendarACLCreateMark);
	mDeleteMark = (LIconControl*) FindPaneByID(paneid_CalendarACLDeleteMark);
	mAdminMark = (LIconControl*) FindPaneByID(paneid_CalendarACLAdminMark);

	mACLTable = (CACLTable*) FindPaneByID(paneid_CalendarACLTable);
	mACLTable->Add_Listener(this);

	mNewUserBtn = (LPushButton*) FindPaneByID(paneid_CalendarACLNewUserBtn);
	mDeleteUserBtn = (LPushButton*) FindPaneByID(paneid_CalendarACLDeleteUserBtn);
	mDeleteUserBtn->Disable();
	mChangeAllBtn = (LPushButton*) FindPaneByID(paneid_CalendarACLChangeAllBtn);

	// Create popup
	mStylePopup = (CACLStylePopup*) FindPaneByID(paneid_CalendarACLStylePopup);
	mStylePopup->Disable();
	mStylePopup->Reset(true);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPropCalendarACLBtns);
}

void CPropCalendarACL::ListenTo_Message(long msg, void* param)
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
void CPropCalendarACL::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_CalACLCalendar:
		long index = *(long*) ioParam;
		calstore::CCalendarStoreNode* cal = static_cast<calstore::CCalendarStoreNode*>(mCalList->at(index - 1));
		SetCal(cal);
		break;

	case msg_CalACLNewUser:
		DoNewUser();
		break;

	case msg_CalACLDeleteUser:
		DoDeleteUser();
		break;

	case msg_CalACLStylePopup:
		DoStylePopup(*(long*) ioParam);
		break;

	case msg_CalACLChangeAll:
		DoChangeAll();
		break;
	}
}

// Set mbox list
void CPropCalendarACL::SetCalList(calstore::CCalendarStoreNodeList* cal_list)
{
	// Save list
	mCalList = cal_list;

	// For now just use first item
	SetCal((calstore::CCalendarStoreNode*) mCalList->front());
	
	// Reset menu
	// Delete previous items
	MenuHandle menuH = mCalendarPopup->GetMacMenuH();
	for(short i = ::CountMenuItems(menuH); i > 0; i--)
		::DeleteMenuItem(menuH, i);

	// Add each Calendar item
	short menu_pos = 1;
	bool multi = calstore::CCalendarStoreManager::sCalendarStoreManager->HasMultipleProtocols();
	for(calstore::CCalendarStoreNodeList::const_iterator iter = mCalList->begin(); iter != mCalList->end(); iter++, menu_pos++)
		::AppendItemToMenu(menuH, menu_pos, (*iter)->GetAccountName(multi));

	// Force max/min update
	mCalendarPopup->SetMenuMinMax();
	
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
}

// Set indicators from rights
void CPropCalendarACL::SetMyRights(SACLRight rights)
{
	mReadFreeBusyMark->SetResourceID(rights.HasRight(CCalendarACL::eCalACL_ReadFreeBusy) ? ICNx_ACLSet : ICNx_ACLUnset);
	mReadFreeBusyMark->Refresh();
	mReadMark->SetResourceID(rights.HasRight(CCalendarACL::eCalACL_Read) ? ICNx_ACLSet : ICNx_ACLUnset);
	mReadMark->Refresh();
	mWriteMark->SetResourceID(rights.HasRight(CCalendarACL::eCalACL_Write) ? ICNx_ACLSet : ICNx_ACLUnset);
	mWriteMark->Refresh();
	mScheduleMark->SetResourceID(rights.HasRight(CCalendarACL::eCalACL_Schedule) ? ICNx_ACLSet : ICNx_ACLUnset);
	mScheduleMark->Refresh();
	mCreateMark->SetResourceID(rights.HasRight(CCalendarACL::eCalACL_Create) ? ICNx_ACLSet : ICNx_ACLUnset);
	mCreateMark->Refresh();
	mDeleteMark->SetResourceID(rights.HasRight(CCalendarACL::eCalACL_Delete) ? ICNx_ACLSet : ICNx_ACLUnset);
	mDeleteMark->Refresh();
	mAdminMark->SetResourceID(rights.HasRight(CCalendarACL::eCalACL_Admin) ? ICNx_ACLSet : ICNx_ACLUnset);
	mAdminMark->Refresh();
}

// Enable/disable buttons
void CPropCalendarACL::SetButtons(bool enable, bool alloff)
{
	mNewUserBtn->SetEnabled(enable && !alloff);
	mDeleteUserBtn->SetEnabled(enable && !alloff && mACLTable->IsSelectionValid());
	mStylePopup->SetEnabled(enable && !alloff && mACLTable->IsSelectionValid());
	mChangeAllBtn->SetEnabled(!alloff);
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
			CCalendarACL old_acl = mActionCal->GetACLs()->at(aCell.row - 1);

			// Change rights in copy
			old_acl.SetRight(acl, set);

			// Change ACL at specific index
			mActionCal->SetACL(&old_acl, aCell.row - 1);
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
			CCalendarACL old_acl = mActionCal->GetACLs()->at(aCell.row - 1);

			// Change rights in copy
			old_acl.SetRights(rights);

			// Change ACL at specific index
			mActionCal->SetACL(&old_acl, aCell.row - 1);
		}
	}

	// Redraw selection
	mACLTable->RefreshSelection();
}

// Do new user dialog
void CPropCalendarACL::DoNewUser(void)
{
	// Create the dialog
	CCalendarPropDialog* cmdr = (CCalendarPropDialog*) GetSuperView()->GetSuperView();
	CBalloonDialog	theHandler(paneid_NewACLDialog, cmdr);
	((CNewACLDialog*) theHandler.GetDialog())->SetDetails(false, false, true);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true) {
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			CCalendarACLList* acl_list = ((CNewACLDialog*) theHandler.GetDialog())->GetDetailsCal();

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
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}

	// Force reset of popup
	mStylePopup->Reset(true);
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
			indices.push_back(aCell.row - 1);
		}
	}

	// Delete all ACLs in one go
	mActionCal->DeleteACL(indices);

	// Force complete reset of table
	mACLTable->SetList(mActionCal->GetACLs(), mCanChange);

	// Force reset of popup
	mStylePopup->Reset(true);
}

// Handle popup command
void CPropCalendarACL::DoStylePopup(long index)
{
	switch(index)
	{
	// New ACL wanted
	case eCalACLStyleNewUser:
	{
		// Get first selected rights
		SACLRight rights;
		STableCell aCell(0, 0);
		while(mACLTable->GetNextSelectedCell(aCell))
		{
			if (aCell.col == 1)
			{
				// Copy existing ACL
				rights = mActionCal->GetACLs()->at(aCell.row - 1).GetRights();
				CCalendarPropDialog* cmdr = (CCalendarPropDialog*) GetSuperView()->GetSuperView();
				mStylePopup->DoNewStyle(rights);
				break;
			}
		}
		break;
	}

	// Delete existing ACL
	case eCalACLStyleDeleteUser:
		CCalendarPropDialog* cmdr = (CCalendarPropDialog*) GetSuperView()->GetSuperView();
		mStylePopup->DoDeleteStyle();
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
