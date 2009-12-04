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


// CNewACLDialog.cpp : implementation file
//

#include "CNewACLDialog.h"

#include "CPreferences.h"

// Prototype for Win32s strtok fix
char* w32strtok(char * str, const char * set);

/////////////////////////////////////////////////////////////////////////////
// CNewACLDialog dialog


CNewACLDialog::CNewACLDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CNewACLDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewACLDialog)
	//}}AFX_DATA_INIT
	mMbox = false;
	mAdbk = false;
	mCalendar = false;
}


void CNewACLDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewACLDialog)
	DDX_Control(pDX, IDC_NEW_ACL_IDS, mUIDList);
	//}}AFX_DATA_MAP

	// Set selection if exiting
	if (pDX->m_bSaveAndValidate)
	{
		// Get ACL state from buttons
		mRights = GetRights();

		for(short i = 0; i < mUIDList.GetLineCount(); i++)
		{
			CString str;
			int length = mUIDList.LineLength(mUIDList.LineIndex(i));
			mUIDList.GetLine(i, str.GetBuffer(length + 1), length);
			str.ReleaseBuffer(length);

			// Add to list
			mResult.push_back(cdstring(str));
		}
	}
}


BEGIN_MESSAGE_MAP(CNewACLDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CNewACLDialog)
	ON_BN_CLICKED(IDC_NEWACL_STYLEPOPUP, OnStylePopup)

	ON_BN_CLICKED(IDC_NEWACL_BTN1, OnACLLookup)
	ON_BN_CLICKED(IDC_NEWACL_BTN2, OnACLRead)
	ON_BN_CLICKED(IDC_NEWACL_BTN3, OnACLSeen)
	ON_BN_CLICKED(IDC_NEWACL_BTN4, OnACLWrite)
	ON_BN_CLICKED(IDC_NEWACL_BTN5, OnACLInsert)
	ON_BN_CLICKED(IDC_NEWACL_BTN6, OnACLPost)
	ON_BN_CLICKED(IDC_NEWACL_BTN7, OnACLCreate)
	ON_BN_CLICKED(IDC_NEWACL_BTN8, OnACLDelete)
	ON_BN_CLICKED(IDC_NEWACL_BTN9, OnACLAdmin)

	ON_COMMAND(IDM_ACL_SAVE_STYLE, OnSaveStyle)
	ON_COMMAND(IDM_ACL_DELETE_STYLE, OnDeleteStyle)
	ON_COMMAND_RANGE(IDM_ACLStyleStart, IDM_ACLStyleEnd, OnChangeStyle)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewACLDialog message handlers

BOOL CNewACLDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass buttons
	mLookupBtn.SubclassDlgItem(IDC_NEWACL_BTN1, this, IDI_ACL_LOOKUP);
	mReadBtn.SubclassDlgItem(IDC_NEWACL_BTN2, this, IDI_ACL_READ);
	mSeenBtn.SubclassDlgItem(IDC_NEWACL_BTN3, this, IDI_ACL_SEEN);
	mWriteBtn.SubclassDlgItem(IDC_NEWACL_BTN4, this, IDI_ACL_WRITE);
	mInsertBtn.SubclassDlgItem(IDC_NEWACL_BTN5, this, IDI_ACL_INSERT);
	mPostBtn.SubclassDlgItem(IDC_NEWACL_BTN6, this, IDI_ACL_POST);
	mCreateBtn.SubclassDlgItem(IDC_NEWACL_BTN7, this, IDI_ACL_CREATE);
	mDeleteBtn.SubclassDlgItem(IDC_NEWACL_BTN8, this, IDI_ACL_DELETE);
	mAdminBtn.SubclassDlgItem(IDC_NEWACL_BTN9, this, IDI_ACL_ADMIN);

	mStylePopup.SubclassDlgItem(IDC_NEWACL_STYLEPOPUP, this, IDI_POPUPBTN);

	// Remove unwanted buttons
	if (!mMbox)
	{
		mSeenBtn.ShowWindow(SW_HIDE);
		mInsertBtn.ShowWindow(SW_HIDE);
		mPostBtn.ShowWindow(SW_HIDE);
		
		CRect rect;
		mWriteBtn.GetWindowRect(rect);
		ScreenToClient(rect);
		int offset = -rect.Width();

		// Move others
		mWriteBtn.GetWindowRect(rect);
		ScreenToClient(rect);
		rect.OffsetRect(offset, 0);
		mWriteBtn.MoveWindow(rect);
		mCreateBtn.GetWindowRect(rect);
		ScreenToClient(rect);
		rect.OffsetRect(3*offset, 0);
		mCreateBtn.MoveWindow(rect);
		mDeleteBtn.GetWindowRect(rect);
		ScreenToClient(rect);
		rect.OffsetRect(3*offset, 0);
		mDeleteBtn.MoveWindow(rect);
		mAdminBtn.GetWindowRect(rect);
		ScreenToClient(rect);
		rect.OffsetRect(3*offset, 0);
		mAdminBtn.MoveWindow(rect);
	}
	
	// Force style popup to proper type
	mStylePopup.Reset(mMbox);

	return true;
}

// Specify mbox or adbk version
void CNewACLDialog::SetDetails(bool mbox, bool adbk, bool cal)
{
	mMbox = mbox;
	mAdbk = adbk;
	mCalendar = cal;
}

// Set buttons from rights
void CNewACLDialog::SetACL(SACLRight rights)
{
	if (mMbox)
	{
		mLookupBtn.SetPushed(rights.HasRight(CMboxACL::eMboxACL_Lookup));
		mReadBtn.SetPushed(rights.HasRight(CMboxACL::eMboxACL_Read));
		mSeenBtn.SetPushed(rights.HasRight(CMboxACL::eMboxACL_Seen));
		mWriteBtn.SetPushed(rights.HasRight(CMboxACL::eMboxACL_Write));
		mInsertBtn.SetPushed(rights.HasRight(CMboxACL::eMboxACL_Insert));
		mPostBtn.SetPushed(rights.HasRight(CMboxACL::eMboxACL_Post));
		mCreateBtn.SetPushed(rights.HasRight(CMboxACL::eMboxACL_Create));
		mDeleteBtn.SetPushed(rights.HasRight(CMboxACL::eMboxACL_Delete));
		mAdminBtn.SetPushed(rights.HasRight(CMboxACL::eMboxACL_Admin));
	}
	else if (mAdbk)
	{
		mLookupBtn.SetPushed(rights.HasRight(CAdbkACL::eAdbkACL_Lookup));
		mReadBtn.SetPushed(rights.HasRight(CAdbkACL::eAdbkACL_Read));
		mWriteBtn.SetPushed(rights.HasRight(CAdbkACL::eAdbkACL_Write));
		mCreateBtn.SetPushed(rights.HasRight(CAdbkACL::eAdbkACL_Create));
		mDeleteBtn.SetPushed(rights.HasRight(CAdbkACL::eAdbkACL_Delete));
		mAdminBtn.SetPushed(rights.HasRight(CAdbkACL::eAdbkACL_Admin));
	}
	else
	{
		mLookupBtn.SetPushed(rights.HasRight(CCalendarACL::eCalACL_Lookup));
		mReadBtn.SetPushed(rights.HasRight(CCalendarACL::eCalACL_Read));
		mWriteBtn.SetPushed(rights.HasRight(CCalendarACL::eCalACL_Write));
		mCreateBtn.SetPushed(rights.HasRight(CCalendarACL::eCalACL_Create));
		mDeleteBtn.SetPushed(rights.HasRight(CCalendarACL::eCalACL_Delete));
		mAdminBtn.SetPushed(rights.HasRight(CCalendarACL::eCalACL_Admin));
	}
}

// Get rights from buttons
SACLRight CNewACLDialog::GetRights(void)
{
	// Start with empty rights
	SACLRight rights;
	rights.SetRight(SACLRight::eACL_AllRights, false);
	
	if (mMbox)
	{
		// Get ACL state from buttons
		if (mLookupBtn.IsPushed())
			rights.SetRight(CMboxACL::eMboxACL_Lookup, true);
		if (mReadBtn.IsPushed())
			rights.SetRight(CMboxACL::eMboxACL_Read, true);
		if (mSeenBtn.IsPushed())
			rights.SetRight(CMboxACL::eMboxACL_Seen, true);
		if (mWriteBtn.IsPushed())
			rights.SetRight(CMboxACL::eMboxACL_Write, true);
		if (mInsertBtn.IsPushed())
			rights.SetRight(CMboxACL::eMboxACL_Insert, true);
		if (mPostBtn.IsPushed())
			rights.SetRight(CMboxACL::eMboxACL_Post, true);
		if (mCreateBtn.IsPushed())
			rights.SetRight(CMboxACL::eMboxACL_Create, true);
		if (mDeleteBtn.IsPushed())
			rights.SetRight(CMboxACL::eMboxACL_Delete, true);
		if (mAdminBtn.IsPushed())
			rights.SetRight(CMboxACL::eMboxACL_Admin, true);
	}
	else if (mAdbk)
	{
		// Get ACL state from buttons
		if (mLookupBtn.IsPushed())
			rights.SetRight(CAdbkACL::eAdbkACL_Lookup, true);
		if (mReadBtn.IsPushed())
			rights.SetRight(CAdbkACL::eAdbkACL_Read, true);
		if (mWriteBtn.IsPushed())
			rights.SetRight(CAdbkACL::eAdbkACL_Write, true);
		if (mCreateBtn.IsPushed())
			rights.SetRight(CAdbkACL::eAdbkACL_Create, true);
		if (mDeleteBtn.IsPushed())
			rights.SetRight(CAdbkACL::eAdbkACL_Delete, true);
		if (mAdminBtn.IsPushed())
			rights.SetRight(CAdbkACL::eAdbkACL_Admin, true);
	}
	else
	{
		// Get ACL state from buttons
		if (mLookupBtn.IsPushed())
			rights.SetRight(CCalendarACL::eCalACL_Lookup, true);
		if (mReadBtn.IsPushed())
			rights.SetRight(CCalendarACL::eCalACL_Read, true);
		if (mWriteBtn.IsPushed())
			rights.SetRight(CCalendarACL::eCalACL_Write, true);
		if (mCreateBtn.IsPushed())
			rights.SetRight(CCalendarACL::eCalACL_Create, true);
		if (mDeleteBtn.IsPushed())
			rights.SetRight(CCalendarACL::eCalACL_Delete, true);
		if (mAdminBtn.IsPushed())
			rights.SetRight(CCalendarACL::eCalACL_Admin, true);
	}
	return rights;
}

// Set the details
CMboxACLList* CNewACLDialog::GetDetailsMbox(void)
{
	// Create empty list
	CMboxACLList* acl_list = new CMboxACLList;

	for(cdstrvect::iterator iter = mResult.begin(); iter != mResult.end(); iter++)
	{
		// Make new ACL
		CMboxACL acl;
		acl.SetUID(*iter);
		acl.SetRights(mRights);
		
		// Add to list
		acl_list->push_back(acl);
	}

	return acl_list;
}

// Set the details
CAdbkACLList* CNewACLDialog::GetDetailsAdbk(void)
{
	// Create empty list
	CAdbkACLList* acl_list = new CAdbkACLList;

	for(cdstrvect::iterator iter = mResult.begin(); iter != mResult.end(); iter++)
	{
		// Make new ACL
		CAdbkACL acl;
		acl.SetUID(*iter);
		acl.SetRights(mRights);
		
		// Add to list
		acl_list->push_back(acl);
	}
	
	return acl_list;
}

// Set the details
CCalendarACLList* CNewACLDialog::GetDetailsCal(void)
{
	// Create empty list
	CCalendarACLList* acl_list = new CCalendarACLList;

	for(cdstrvect::iterator iter = mResult.begin(); iter != mResult.end(); iter++)
	{
		// Make new ACL
		CCalendarACL acl;
		acl.SetUID(*iter);
		acl.SetRights(mRights);
		
		// Add to list
		acl_list->push_back(acl);
	}
	
	return acl_list;
}

// Handle popup command
void CNewACLDialog::OnStylePopup(void)
{
}

// Handle button change
void CNewACLDialog::OnACLLookup(void)
{
	mLookupBtn.SetPushed(!mLookupBtn.IsPushed());
}

// Handle button change
void CNewACLDialog::OnACLRead(void)
{
	mReadBtn.SetPushed(!mReadBtn.IsPushed());
}

// Handle button change
void CNewACLDialog::OnACLSeen(void)
{
	mSeenBtn.SetPushed(!mSeenBtn.IsPushed());
}

// Handle button change
void CNewACLDialog::OnACLWrite(void)
{
	mWriteBtn.SetPushed(!mWriteBtn.IsPushed());
}

// Handle button change
void CNewACLDialog::OnACLInsert(void)
{
	mInsertBtn.SetPushed(!mInsertBtn.IsPushed());
}

// Handle button change
void CNewACLDialog::OnACLPost(void)
{
	mPostBtn.SetPushed(!mPostBtn.IsPushed());
}

// Handle button change
void CNewACLDialog::OnACLCreate(void)
{
	mCreateBtn.SetPushed(!mCreateBtn.IsPushed());
}

// Handle button change
void CNewACLDialog::OnACLDelete(void)
{
	mDeleteBtn.SetPushed(!mDeleteBtn.IsPushed());
}

// Handle button change
void CNewACLDialog::OnACLAdmin(void)
{
	mAdminBtn.SetPushed(!mAdminBtn.IsPushed());
}

// Do new style
void CNewACLDialog::OnSaveStyle(void)
{
	mStylePopup.DoNewStyle(GetRights());
}

// Delete style
void CNewACLDialog::OnDeleteStyle(void)
{
	mStylePopup.DoDeleteStyle();
}

// Change style
void CNewACLDialog::OnChangeStyle(UINT nID)
{
	// Get rights for style and set buttons
	SACLRight rights = (mMbox ? CPreferences::sPrefs->mMboxACLStyles.GetValue()[nID - IDM_ACLStyleStart].second :
								(mAdbk ? CPreferences::sPrefs->mAdbkACLStyles.GetValue()[nID - IDM_ACLStyleStart].second :
										CPreferences::sPrefs->mAdbkACLStyles.GetValue()[nID - IDM_ACLStyleStart].second));
	SetACL(rights);
}