/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CPropCalendarACL.cpp : implementation file
//

#include "CPropCalendarACL.h"

#include "CErrorHandler.h"
#include "CLog.h"
#include "CNewACLDialog.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

#include "CCalendarStoreManager.h"
#include "CCalendarProtocol.h"

/////////////////////////////////////////////////////////////////////////////
// CPropCalendarACL dialog

IMPLEMENT_DYNCREATE(CPropCalendarACL, CHelpPropertyPage)

CPropCalendarACL::CPropCalendarACL() : CHelpPropertyPage(CPropCalendarACL::IDD)
{
	//{{AFX_DATA_INIT(CPropCalendarACL)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	mCanChange = false;
	mNoChange = false;
	mActionCal = NULL;
}


void CPropCalendarACL::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropCalendarACL)
	//DDX_Control(pDX, IDC_CALENDARACL_STYLEPOPUP, mStylePopup);
	DDX_Control(pDX, IDC_CALENDARACL_DELETEUSER, mDeleteUserBtn);
	DDX_Control(pDX, IDC_CALENDARACL_NEWUSER, mNewUserBtn);
	DDX_Control(pDX, IDC_CALENDARACL_APPLYALL, mChangeAllBtn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropCalendarACL, CHelpPropertyPage)
	//{{AFX_MSG_MAP(CPropCalendarACL)
	ON_COMMAND_RANGE(IDM_MAILBOXACLStart, IDM_MAILBOXACLEnd, OnCalendarPopup)
	ON_BN_CLICKED(IDC_CALENDARACL_NEWUSER, OnNewUser)
	ON_BN_CLICKED(IDC_CALENDARACL_DELETEUSER, OnDeleteUser)
	ON_BN_CLICKED(IDC_CALENDARACL_STYLEPOPUP, OnStylePopup)
	ON_BN_CLICKED(IDC_CALENDARACL_APPLYALL, OnChangeAll)

	ON_COMMAND(IDM_ACL_SAVE_STYLE, OnSaveStyle)
	ON_COMMAND(IDM_ACL_DELETE_STYLE, OnDeleteStyle)
	ON_COMMAND_RANGE(IDM_ACLStyleStart, IDM_ACLStyleEnd, OnChangeStyle)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropCalendarACL message handlers
BOOL CPropCalendarACL::OnInitDialog()
{
	CHelpPropertyPage::OnInitDialog();

	// Subclass buttons
	mStylePopup.SubclassDlgItem(IDC_CALENDARACL_STYLEPOPUP, this, IDI_POPUPBTN);
	mStylePopup.Reset(false);

	mACLFreebusy.SubclassDlgItem(IDC_ACL_LOOKUP, this, IDI_ACL_LOOKUP);
	mACLRead.SubclassDlgItem(IDC_ACL_READ, this, IDI_ACL_READ);
	mACLWrite.SubclassDlgItem(IDC_ACL_WRITE, this, IDI_ACL_WRITE);
	mACLSchedule.SubclassDlgItem(IDC_ACL_POST, this, IDI_ACL_POST);
	mACLCreate.SubclassDlgItem(IDC_ACL_CREATE, this, IDI_ACL_CREATE);
	mACLDelete.SubclassDlgItem(IDC_ACL_DELETE, this, IDI_ACL_DELETE);
	mACLAdmin.SubclassDlgItem(IDC_ACL_ADMIN, this, IDI_ACL_ADMIN);

	mACLTable.SubclassDlgItem(IDC_CALENDARACL_TABLE, this);
	mACLTable.Add_Listener(this);

	mACLTitleTable.SubclassDlgItem(IDC_CALENDARACL_TITLES, this);
	mACLTitleTable.SetDetails(false, false, true);

	// Do mailbox menu
	mCalendarPopup.SubclassDlgItem(IDC_CALENDARACL_CALENDARPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mCalendarPopup.SetMenu(IDR_POPUP_ACL_MAILBOX);

	CMenu* pPopup = mCalendarPopup.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);

	// Now add current items
	int menu_id = IDM_MAILBOXACLStart;
	bool multi = calstore::CCalendarStoreManager::sCalendarStoreManager->HasMultipleProtocols();
	for(calstore::CCalendarStoreNodeList::const_iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id++, (*iter)->GetAccountName(multi));
	mCalendarPopup.SetValue(IDM_MAILBOXACLStart);
	if (mCalList->size() == 1)
		mChangeAllBtn.ShowWindow(SW_HIDE);

	mDeleteUserBtn.EnableWindow(false);
	mStylePopup.EnableWindow(false);

	// For now just use first item
	SetCalendar(mCalList->front());

	return true;
}

void CPropCalendarACL::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CTable::eBroadcast_SelectionChanged:
		// Enable/disable Delete button
		mDeleteUserBtn.EnableWindow(mACLTable.IsSelectionValid());
		mStylePopup.EnableWindow(mACLTable.IsSelectionValid());
		break;
	default:
		CListener::ListenTo_Message(msg, param);
		break;
	}
}

void CPropCalendarACL::SetCalList(calstore::CCalendarStoreNodeList* cal_list)
{
	// Save list
	mCalList = cal_list;
}

// Set mbox list
void CPropCalendarACL::SetCalendar(calstore::CCalendarStoreNode* cal)
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
	mACLTable.SetList(mActionCal->GetACLs(), mCanChange);
	mACLTitleTable.SyncTable(&mACLTable, true);
}

// Set indicators from rights
void CPropCalendarACL::SetMyRights(SACLRight rights)
{
	mACLTable.SetMyRights(rights);
}

// Enable/disable buttons
void CPropCalendarACL::SetButtons(bool enable, bool alloff)
{
	mNewUserBtn.EnableWindow(enable);
	mDeleteUserBtn.EnableWindow(enable && mACLTable.IsSelectionValid());
	mStylePopup.EnableWindow(enable && mACLTable.IsSelectionValid());

	mChangeAllBtn.EnableWindow(!alloff);
}

// User clicked a button
void CPropCalendarACL::DoChangeACL(CCalendarACL::ECalACL acl, bool set)
{
	// Set this state in each selected ACL
	STableCell aCell(0, 0);
	while(mACLTable.GetNextSelectedCell(aCell))
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
	mACLTable.RefreshSelection();

	// Set focus back to table
	mACLTable.SetFocus();
}

// Change rights
void CPropCalendarACL::DoChangeRights(SACLRight rights)
{
	// Set this state in each selected ACL
	STableCell aCell(0, 0);
	while(mACLTable.GetNextSelectedCell(aCell))
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
	mACLTable.RefreshSelection();

	// Set focus back to table
	mACLTable.SetFocus();
}

// Mailbox popup change
void CPropCalendarACL::OnCalendarPopup(UINT nID)
{
	mCalendarPopup.SetValue(nID);
	calstore::CCalendarStoreNode* cal = static_cast<calstore::CCalendarStoreNode*>(mCalList->at(nID - IDM_MAILBOXACLStart));
	SetCalendar(cal);
}

// Do new user dialog
void CPropCalendarACL::OnNewUser(void)
{
	// Create dialog
	CNewACLDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetDetails(false, false, true);

	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{
		CCalendarACLList* acl_list = dlog.GetDetailsCal();

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
		mACLTable.SetList(mActionCal->GetACLs(), mCanChange);
	}

	// Set focus back to table
	mACLTable.SetFocus();
}

// Delete user from list
void CPropCalendarACL::OnDeleteUser(void)
{
	// Do caution as this is not undoable
	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Server::DeleteAclWarn") == CErrorHandler::Cancel)
		return;

	// Delete ACLs in reverse (not first row)
	ulvector indices;
	STableCell aCell(0, 0);
	while(mACLTable.GetNextSelectedCell(aCell))
	{
		if ((aCell.col == 1) && (aCell.row > 1))
		{
			// Delete the one at this index
			indices.push_back(aCell.row - 2);
		}
	}

	// Delete all ACLs in one go
	mActionCal->DeleteACL(indices);
	
	// Force complete reset of table
	mACLTable.SetList(mActionCal->GetACLs(), mCanChange);
			
	// Set focus back to table
	mACLTable.SetFocus();
}

// Handle popup command
void CPropCalendarACL::OnStylePopup(void)
{
}

// Do new style
void CPropCalendarACL::OnSaveStyle(void)
{
	// Set this state in each selected ACL (not first row)
	STableCell aCell(0, 0);
	while(mACLTable.GetNextSelectedCell(aCell))
	{
		if (aCell.col == 1)
		{
			// Copy existing ACL
			SACLRight rights = mActionCal->GetACLs()->at(aCell.row - 2).GetRights();
			mStylePopup.DoNewStyle(rights);
			break;
		}
	}
}

// Delete style
void CPropCalendarACL::OnDeleteStyle(void)
{
	mStylePopup.DoDeleteStyle();
}


// Apply to all mailboxes
void CPropCalendarACL::OnChangeAll(void)
{
	// Get entire rights for this calendar
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

// Change style
void CPropCalendarACL::OnChangeStyle(UINT nID)
{
	// Get rights for style and set buttons
	SACLRight rights = CPreferences::sPrefs->mAdbkACLStyles.GetValue()[nID - IDM_ACLStyleStart].second;
	
	// Fake button hit to force update of selection and mailbox
	DoChangeRights(rights);
}
