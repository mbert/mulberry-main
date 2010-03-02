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


// CPropAdbkACL.cpp : implementation file
//

#include "CPropAdbkACL.h"

#include "CAdbkProtocol.h"
#include "CErrorHandler.h"
#include "CFontCache.h"
#include "CLog.h"
#include "CNewACLDialog.h"
#include "CPreferences.h"
#include "CSDIFrame.h"

/////////////////////////////////////////////////////////////////////////////
// CPropAdbkACL dialog

IMPLEMENT_DYNCREATE(CPropAdbkACL, CHelpPropertyPage)

CPropAdbkACL::CPropAdbkACL() : CHelpPropertyPage(CPropAdbkACL::IDD)
{
	//{{AFX_DATA_INIT(CPropAdbkACL)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPropAdbkACL::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropAdbkACL)
	//DDX_Control(pDX, IDC_ADBKACL_STYLEPOPUP, mStylePopup);
	DDX_Control(pDX, IDC_ADBKACL_DELETEUSER, mDeleteUserBtn);
	DDX_Control(pDX, IDC_ADBKACL_NEWUSER, mNewUserBtn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropAdbkACL, CHelpPropertyPage)
	//{{AFX_MSG_MAP(CPropAdbkACL)
	ON_BN_CLICKED(IDC_ADBKACL_NEWUSER, OnNewUser)
	ON_BN_CLICKED(IDC_ADBKACL_DELETEUSER, OnDeleteUser)
	ON_BN_CLICKED(IDC_ADBKACL_STYLEPOPUP, OnStylePopup)

	ON_COMMAND(IDM_ACL_SAVE_STYLE, OnSaveStyle)
	ON_COMMAND(IDM_ACL_DELETE_STYLE, OnDeleteStyle)
	ON_COMMAND_RANGE(IDM_ACLStyleStart, IDM_ACLStyleEnd, OnChangeStyle)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropAdbkACL message handlers
BOOL CPropAdbkACL::OnInitDialog()
{
	CHelpPropertyPage::OnInitDialog();

	// Subclass buttons
	mStylePopup.SubclassDlgItem(IDC_ADBKACL_STYLEPOPUP, this, IDI_POPUPBTN);
	mStylePopup.Reset(false);

	mACLLookup.SubclassDlgItem(IDC_ACL_LOOKUP, this, IDI_ACL_LOOKUP);
	mACLRead.SubclassDlgItem(IDC_ACL_READ, this, IDI_ACL_READ);
	mACLWrite.SubclassDlgItem(IDC_ACL_WRITE, this, IDI_ACL_WRITE);
	mACLCreate.SubclassDlgItem(IDC_ACL_CREATE, this, IDI_ACL_CREATE);
	mACLDelete.SubclassDlgItem(IDC_ACL_DELETE, this, IDI_ACL_DELETE);
	mACLAdmin.SubclassDlgItem(IDC_ACL_ADMIN, this, IDI_ACL_ADMIN);

	mACLTable.SubclassDlgItem(IDC_ADBKACL_TABLE, this);
	mACLTable.Add_Listener(this);

	mACLTitleTable.SubclassDlgItem(IDC_ADBKACL_TITLES, this);
	mACLTitleTable.SetDetails(false, true, false);

	mDeleteUserBtn.EnableWindow(false);
	mStylePopup.EnableWindow(false);

	// For now just use first item
	SetAdbk(mAdbkList->front());

	return true;
}

void CPropAdbkACL::ListenTo_Message(long msg, void* param)
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

void CPropAdbkACL::SetAdbkList(CAddressBookList* adbk_list)
{
	// Save list
	mAdbkList = adbk_list;
}

// Set mbox list
void CPropAdbkACL::SetAdbk(CAddressBook* adbk)
{
	// Set myrights
	SetMyRights(adbk->GetMyRights());
	
	// Set state of buttons
	mCanChange = adbk->GetMyRights().HasRight(CAdbkACL::eAdbkACL_Admin) &&
					adbk->GetProtocol()->IsLoggedOn();
	SetButtons(mCanChange);

	// Give list to table
	mACLTable.SetList(adbk->GetACLs(), mCanChange);
	mACLTitleTable.SyncTable(&mACLTable, true);
}

// Set indicators from rights
void CPropAdbkACL::SetMyRights(SACLRight rights)
{
	mACLTable.SetMyRights(rights);
}

// Enable/disable buttons
void CPropAdbkACL::SetButtons(bool enable)
{
	mNewUserBtn.EnableWindow(enable);
	mDeleteUserBtn.EnableWindow(enable && mACLTable.IsSelectionValid());
	mStylePopup.EnableWindow(enable && mACLTable.IsSelectionValid());
}

// User clicked a button
void CPropAdbkACL::DoChangeACL(CAdbkACL::EAdbkACL acl, bool set)
{
	// Only operate on first adbk for now...
	CAddressBook* adbk = mAdbkList->front();

	// Set this state in each selected ACL (not first row)
	STableCell aCell(0, 0);
	while(mACLTable.GetNextSelectedCell(aCell))
	{
		if (aCell.col == 1)
		{
			// Copy existing ACL
			CAdbkACL old_acl = adbk->GetACLs()->at(aCell.row - 2);
				
			// Change rights in copy
			old_acl.SetRight(acl, set);
				
			// Get mbox to do change
			// mbox will replace if successful
			adbk->SetACL(&old_acl);
		}
	}
	
	// Redraw selection
	mACLTable.RefreshSelection();

	// Set focus back to table
	mACLTable.SetFocus();
}

// Change rights
void CPropAdbkACL::DoChangeRights(SACLRight rights)
{
	// Only operate on first adbk for now...
	CAddressBook* adbk = mAdbkList->front();

	// Set this state in each selected ACL (not first row)
	STableCell aCell(0, 0);
	while(mACLTable.GetNextSelectedCell(aCell))
	{
		if (aCell.col == 1)
		{
			// Copy existing ACL
			CAdbkACL old_acl = adbk->GetACLs()->at(aCell.row - 2);
			
			// Change rights in copy
			old_acl.SetRights(rights);
				
			// Get mbox to do change
			// mbox will replace if successful
			adbk->SetACL(&old_acl);
		}
	}
	
	// Redraw selection
	mACLTable.RefreshSelection();

	// Set focus back to table
	mACLTable.SetFocus();
}

// Do new user dialog
void CPropAdbkACL::OnNewUser(void)
{
	// Create dialog
	CNewACLDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetDetails(false, true, false);

	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{
		// Only operate on first adbk for now...
		CAddressBook* adbk = mAdbkList->front();

		CAdbkACLList* acl_list = dlog.GetDetailsAdbk();
		
		try
		{
			// Iterate over each new acl and add to adbk
			for(CAdbkACLList::iterator iter = acl_list->begin(); iter != acl_list->end(); iter++)
			{
				// Get adbk to do change
				// adbk will replace if successful
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
		mACLTable.SetList(adbk->GetACLs(), mCanChange);
	}

	// Set focus back to table
	mACLTable.SetFocus();
}

// Delete user from list
void CPropAdbkACL::OnDeleteUser(void)
{
	// Do caution as this is not undoable
	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Server::DeleteAclWarn") == CErrorHandler::Cancel)
		return;

	// Only operate on first adbk for now...
	CAddressBook* adbk = mAdbkList->front();

	// Delete ACLs in reverse (not first row)
	STableCell aCell(0, 0);
	while(mACLTable.GetPreviousSelectedCell(aCell))
	{
		if (aCell.col == 1)
		{
			// Copy existing ACL
			CAdbkACL old_acl = adbk->GetACLs()->at(aCell.row - 2);
			
			// Get adbk to do change
			// adbk will delete if successful
			adbk->DeleteACL(&old_acl);
		}
	}
	
	// Force complete reset of table
	mACLTable.SetList(adbk->GetACLs(), mCanChange);
			
	// Set focus back to table
	mACLTable.SetFocus();
}

// Handle popup command
void CPropAdbkACL::OnStylePopup(void)
{
}

// Do new style
void CPropAdbkACL::OnSaveStyle(void)
{
	// Only operate on first adbk for now...
	CAddressBook* adbk = mAdbkList->front();

	// Set this state in each selected ACL (not first row)
	STableCell aCell(0, 0);
	while(mACLTable.GetNextSelectedCell(aCell))
	{
		if (aCell.col == 1)
		{
			// Copy existing ACL
			SACLRight rights = adbk->GetACLs()->at(aCell.row - 2).GetRights();
			mStylePopup.DoNewStyle(rights);
			break;
		}
	}
}

// Delete style
void CPropAdbkACL::OnDeleteStyle(void)
{
	mStylePopup.DoDeleteStyle();
}

// Change style
void CPropAdbkACL::OnChangeStyle(UINT nID)
{
	// Get rights for style and set buttons
	SACLRight rights = CPreferences::sPrefs->mAdbkACLStyles.GetValue()[nID - IDM_ACLStyleStart].second;
	
	// Fake button hit to force update of selection and mailbox
	DoChangeRights(rights);
}
