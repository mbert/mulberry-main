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


// CPropMailboxACL.cp : implementation file
//

#include "CPropMailboxACL.h"

#include "CErrorHandler.h"
#include "CFontCache.h"
#include "CLog.h"
#include "CMailAccountManager.h"
#include "CMbox.h"
#include "CMboxList.h"
#include "CNewACLDialog.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPropMailboxACL property page

IMPLEMENT_DYNCREATE(CPropMailboxACL, CHelpPropertyPage)

CPropMailboxACL::CPropMailboxACL() : CHelpPropertyPage(CPropMailboxACL::IDD)
{
	mCanChange = false;
	mNoChange = false;
	mActionMbox = NULL;
}

CPropMailboxACL::~CPropMailboxACL()
{
	mACLTitleTable.Detach();
	mACLTable.Detach();

	mStylePopup.Detach();
}

void CPropMailboxACL::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropMailboxACL)
	DDX_Control(pDX, IDC_MAILBOXACL_NEWUSER, mNewUserBtn);
	DDX_Control(pDX, IDC_MAILBOXACL_DELETEUSER, mDeleteUserBtn);
	DDX_Control(pDX, IDC_MAILBOXACL_APPLYALL, mChangeAllBtn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropMailboxACL, CHelpPropertyPage)
	//{{AFX_MSG_MAP(CPropMailboxACL)
	ON_COMMAND_RANGE(IDM_MAILBOXACLStart, IDM_MAILBOXACLEnd, OnMailboxPopup)
	ON_BN_CLICKED(IDC_MAILBOXACL_NEWUSER, OnNewUser)
	ON_BN_CLICKED(IDC_MAILBOXACL_DELETEUSER, OnDeleteUser)
	ON_BN_CLICKED(IDC_MAILBOXACL_STYLEPOPUP, OnStylePopup)
	ON_BN_CLICKED(IDC_MAILBOXACL_APPLYALL, OnChangeAll)

	ON_COMMAND(IDM_ACL_SAVE_STYLE, OnSaveStyle)
	ON_COMMAND(IDM_ACL_DELETE_STYLE, OnDeleteStyle)
	ON_COMMAND_RANGE(IDM_ACLStyleStart, IDM_ACLStyleEnd, OnChangeStyle)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropMailboxACL message handlers

BOOL CPropMailboxACL::OnInitDialog()
{
	CHelpPropertyPage::OnInitDialog();

	// Subclass buttons
	mStylePopup.SubclassDlgItem(IDC_MAILBOXACL_STYLEPOPUP, this, IDI_POPUPBTN);
	mStylePopup.Reset(true);

	mACLLookup.SubclassDlgItem(IDC_ACL_LOOKUP, this, IDI_ACL_LOOKUP);
	mACLRead.SubclassDlgItem(IDC_ACL_READ, this, IDI_ACL_READ);
	mACLSeen.SubclassDlgItem(IDC_ACL_SEEN, this, IDI_ACL_SEEN);
	mACLWrite.SubclassDlgItem(IDC_ACL_WRITE, this, IDI_ACL_WRITE);
	mACLInsert.SubclassDlgItem(IDC_ACL_INSERT, this, IDI_ACL_INSERT);
	mACLPost.SubclassDlgItem(IDC_ACL_POST, this, IDI_ACL_POST);
	mACLCreate.SubclassDlgItem(IDC_ACL_CREATE, this, IDI_ACL_CREATE);
	mACLDelete.SubclassDlgItem(IDC_ACL_DELETE, this, IDI_ACL_DELETE);
	mACLAdmin.SubclassDlgItem(IDC_ACL_ADMIN, this, IDI_ACL_ADMIN);

	mACLTable.SubclassDlgItem(IDC_MAILBOXACL_TABLE, this);
	mACLTable.Add_Listener(this);

	mACLTitleTable.SubclassDlgItem(IDC_MAILBOXACL_TITLES, this);
	mACLTitleTable.SetDetails(true, false, false);

	// Do mailbox menu
	mMailboxPopup.SubclassDlgItem(IDC_MAILBOXACL_MAILBOXPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mMailboxPopup.SetMenu(IDR_POPUP_ACL_MAILBOX);

	CMenu* pPopup = mMailboxPopup.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);

	// Now add current items
	int menu_id = IDM_MAILBOXACLStart;
	bool multi = (CMailAccountManager::sMailAccountManager->GetProtocolList().size() > 1);
	for(CMboxList::const_iterator iter = mMboxList->begin(); iter != mMboxList->end(); iter++)
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id++, (*iter)->GetAccountName(multi));
	mMailboxPopup.SetValue(IDM_MAILBOXACLStart);

	mDeleteUserBtn.EnableWindow(false);
	mStylePopup.EnableWindow(false);

	// For now just use first item
	SetMbox((CMbox*) mMboxList->front());

	return true;
}

void CPropMailboxACL::ListenTo_Message(long msg, void* param)
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

void CPropMailboxACL::SetMboxList(CMboxList* mbox_list)
{
	// Save list
	mMboxList = mbox_list;
	
	if ((mMboxList->size() == 1) && (m_hWnd != NULL))
		mChangeAllBtn.ShowWindow(SW_HIDE);
}


// Set mbox list
void CPropMailboxACL::SetMbox(CMbox* mbox)
{
	// Cache actionable mailbox
	mActionMbox = mbox;

	// Set myrights
	SetMyRights(mbox->GetMyRights());
	
	// Set state of buttons - must have admin right and logged in account
	mCanChange = mbox->GetMyRights().HasRight(CMboxACL::eMboxACL_Admin);
	mNoChange = !mActionMbox->GetProtocol()->IsLoggedOn();
	SetButtons(mCanChange, mNoChange);

	// Give list to table
	mACLTable.SetList(mbox->GetACLs(), mCanChange);
	mACLTitleTable.SyncTable(&mACLTable, true);
}

// Set indicators from rights
void CPropMailboxACL::SetMyRights(SACLRight rights)
{
	mACLTable.SetMyRights(rights);
}

// Enable/disable buttons
void CPropMailboxACL::SetButtons(bool enable, bool alloff)
{
	mNewUserBtn.EnableWindow(enable && !alloff);
	mDeleteUserBtn.EnableWindow(enable && !alloff && mACLTable.IsSelectionValid());
	mStylePopup.EnableWindow(enable && !alloff && mACLTable.IsSelectionValid());
	
	mChangeAllBtn.EnableWindow(!alloff);
}

// User clicked a button
void CPropMailboxACL::DoChangeACL(CMboxACL::EMboxACL acl, bool set)
{
	// Only operate on first Mbox for now...
	CMbox* mbox = (CMbox*) mMboxList->front();

	// Set this state in each selected ACL (not first row)
	STableCell aCell(0, 0);
	while(mACLTable.GetNextSelectedCell(aCell))
	{
		if (aCell.col == 1)
		{
			// Copy existing ACL
			CMboxACL old_acl = mbox->GetACLs()->at(aCell.row - 2);
				
			// Change rights in copy
			old_acl.SetRight(acl, set);
				
			// Get mbox to do change
			// mbox will replace if successful
			mbox->SetACL(&old_acl);
		}
	}

	// Redraw selection
	mACLTable.RefreshSelection();

	// Set focus back to table
	mACLTable.SetFocus();
}

// Change rights
void CPropMailboxACL::DoChangeRights(SACLRight rights)
{
	// Only operate on first Mbox for now...
	CMbox* mbox = (CMbox*) mMboxList->front();

	// Set this state in each selected ACL (not first row)
	STableCell aCell(0, 0);
	while(mACLTable.GetNextSelectedCell(aCell))
	{
		if (aCell.col == 1)
		{
			// Copy existing ACL
			CMboxACL old_acl = mbox->GetACLs()->at(aCell.row - 2);
				
			// Change rights in copy
			old_acl.SetRights(rights);
				
			// Get mbox to do change
			// mbox will replace if successful
			mbox->SetACL(&old_acl);
		}
	}
	
	// Redraw selection
	mACLTable.RefreshSelection();

	// Set focus back to table
	mACLTable.SetFocus();
}

// Mailbox popup change
void CPropMailboxACL::OnMailboxPopup(UINT nID)
{
	mMailboxPopup.SetValue(nID);
	CMbox* mbox = static_cast<CMbox*>(mMboxList->at(nID - IDM_MAILBOXACLStart));
	SetMbox(mbox);
}

// Do new user dialog
void CPropMailboxACL::OnNewUser(void)
{
	// Create dialog
	CNewACLDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetDetails(true, false, false);

	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{
		// Only operate on first Mbox for now...
		CMbox* mbox = (CMbox*) mMboxList->front();

		CMboxACLList* acl_list = dlog.GetDetailsMbox();
		
		try
		{
			// Iterate over each new acl and add to mailbox
			for(CMboxACLList::iterator iter = acl_list->begin(); iter != acl_list->end(); iter++)
			{
				// Get mbox to do change
				// mbox will replace if successful
				mbox->SetACL(&(*iter));
			}
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Do not allow failure
		}
		delete acl_list;
		
		// Force complete reset of table
		mACLTable.SetList(mbox->GetACLs(), mCanChange);
	}

	// Set focus back to table
	mACLTable.SetFocus();
}

// Delete user from list
void CPropMailboxACL::OnDeleteUser(void)
{
	// Do caution as this is not undoable
	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Server::DeleteAclWarn") == CErrorHandler::Cancel)
		return;

	// Only operate on first Mbox for now...
	CMbox* mbox = (CMbox*) mMboxList->front();

	// Delete ACLs in reverse (not first row)
	STableCell aCell(0, 0);
	while(mACLTable.GetPreviousSelectedCell(aCell))
	{
		if (aCell.col == 1)
		{
			// Copy existing ACL
			CMboxACL old_acl = mbox->GetACLs()->at(aCell.row - 2);
			
			// Get mbox to do change
			// mbox will delete if successful
			mbox->DeleteACL(&old_acl);
		}
	}
	
	// Force complete reset of table
	mACLTable.SetList(mbox->GetACLs(), mCanChange);
			
	// Set focus back to table
	mACLTable.SetFocus();
}

// Handle popup command
void CPropMailboxACL::OnStylePopup(void)
{
}

// Do new style
void CPropMailboxACL::OnSaveStyle(void)
{
	// Only operate on first Mbox for now...
	CMbox* mbox = (CMbox*) mMboxList->front();

	// Set this state in each selected ACL (not first row)
	STableCell aCell(0, 0);
	while(mACLTable.GetNextSelectedCell(aCell))
	{
		if (aCell.col == 1)
		{
			// Copy existing ACL
			SACLRight rights = mActionMbox->GetACLs()->at(aCell.row - 2).GetRights();
			mStylePopup.DoNewStyle(rights);
			break;
		}
	}
}


// Apply to all mailboxes
void CPropMailboxACL::OnChangeAll(void)
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

// Delete style
void CPropMailboxACL::OnDeleteStyle(void)
{
	mStylePopup.DoDeleteStyle();
}

// Change style
void CPropMailboxACL::OnChangeStyle(UINT nID)
{
	// Get rights for style and set buttons
	SACLRight rights = CPreferences::sPrefs->mMboxACLStyles.GetValue()[nID - IDM_ACLStyleStart].second;
	
	// Fake button hit to force update of selection and mailbox
	DoChangeRights(rights);
}