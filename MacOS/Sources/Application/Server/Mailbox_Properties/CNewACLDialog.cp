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


// Source for CNewACLDialog class

#include "CNewACLDialog.h"

#include "CACLStylePopup.h"
#include "CMailboxPropDialog.h"
#include "CPreferences.h"
#include "CPropMailboxACL.h"
#include "CTextDisplay.h"

#include <LBevelButton.h>


const short cAdbkStyleBtnMove = 24;

// __________________________________________________________________________________________________
// C L A S S __ C O P E N M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CNewACLDialog::CNewACLDialog()
{
	mMbox = false;
	mAdbk = false;
	mCalendar = false;
}

// Constructor from stream
CNewACLDialog::CNewACLDialog(LStream *inStream)
		: LDialogBox(inStream)
{
	mMbox = false;
	mAdbk = false;
	mCalendar = false;
}

// Default destructor
CNewACLDialog::~CNewACLDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CNewACLDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	mLookupBtn = (LBevelButton*) FindPaneByID(paneid_NewACLLookupBtn);
	mReadBtn = (LBevelButton*) FindPaneByID(paneid_NewACLReadBtn);
	mSeenBtn = (LBevelButton*) FindPaneByID(paneid_NewACLSeenBtn);
	mWriteBtn = (LBevelButton*) FindPaneByID(paneid_NewACLWriteBtn);
	mInsertBtn = (LBevelButton*) FindPaneByID(paneid_NewACLInsertBtn);
	mScheduleBtn = (LBevelButton*) FindPaneByID(paneid_NewACLScheduleBtn);
	mPostBtn = (LBevelButton*) FindPaneByID(paneid_NewACLPostBtn);
	mCreateBtn = (LBevelButton*) FindPaneByID(paneid_NewACLCreateBtn);
	mDeleteBtn = (LBevelButton*) FindPaneByID(paneid_NewACLDeleteBtn);
	mAdminBtn = (LBevelButton*) FindPaneByID(paneid_NewACLAdminBtn);

	mUIDList = (CTextDisplay*) FindPaneByID(paneid_NewACLUIDs);

	mStylePopup = (CACLStylePopup*) FindPaneByID(paneid_NewACLStylePopup);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CNewACLDialogBtns);

	// Make text edit field active
	SetLatentSub(mUIDList);
	mUIDList->SelectAll();
}

// Specify mbox or adbk version
void CNewACLDialog::SetDetails(bool mbox, bool adbk, bool cal)
{
	mMbox = mbox;
	mAdbk = adbk;
	mCalendar = cal;

	// Remove unwanted buttons
	if (mMbox)
	{
		mScheduleBtn->Hide();
	}
	if (mAdbk)
	{
		mSeenBtn->Hide();
		mInsertBtn->Hide();
		mScheduleBtn->Hide();
		mPostBtn->Hide();

		// Move others
		mWriteBtn->MoveBy(-cAdbkStyleBtnMove, 0, false);
		mCreateBtn->MoveBy(-3*cAdbkStyleBtnMove, 0, false);
		mDeleteBtn->MoveBy(-3*cAdbkStyleBtnMove, 0, false);
		mAdminBtn->MoveBy(-3*cAdbkStyleBtnMove, 0, false);
	}
	if (mCalendar)
	{
		mSeenBtn->Hide();
		mInsertBtn->Hide();
		mPostBtn->Hide();
		
		// Move others
		mWriteBtn->MoveBy(-cAdbkStyleBtnMove, 0, false);
		mScheduleBtn->MoveBy(-cAdbkStyleBtnMove, 0, false);
		mCreateBtn->MoveBy(-2*cAdbkStyleBtnMove, 0, false);
		mDeleteBtn->MoveBy(-2*cAdbkStyleBtnMove, 0, false);
		mAdminBtn->MoveBy(-2*cAdbkStyleBtnMove, 0, false);
	}

	// Force style popup to proper type
	mStylePopup->Reset(mbox);
}

// Handle OK button
void CNewACLDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage) {
		case msg_ACLStylePopup:
			DoStylePopup(*(long*) ioParam);
			break;

		default:
			LDialogBox::ListenToMessage(inMessage, ioParam);
			break;
	}
}

// Set buttons from rights
void CNewACLDialog::SetACL(SACLRight rights)
{
	if (mMbox)
	{
		mLookupBtn->SetValue(rights.HasRight(CMboxACL::eMboxACL_Lookup));
		mReadBtn->SetValue(rights.HasRight(CMboxACL::eMboxACL_Read));
		mSeenBtn->SetValue(rights.HasRight(CMboxACL::eMboxACL_Seen));
		mWriteBtn->SetValue(rights.HasRight(CMboxACL::eMboxACL_Write));
		mInsertBtn->SetValue(rights.HasRight(CMboxACL::eMboxACL_Insert));
		mPostBtn->SetValue(rights.HasRight(CMboxACL::eMboxACL_Post));
		mCreateBtn->SetValue(rights.HasRight(CMboxACL::eMboxACL_Create));
		mDeleteBtn->SetValue(rights.HasRight(CMboxACL::eMboxACL_Delete));
		mAdminBtn->SetValue(rights.HasRight(CMboxACL::eMboxACL_Admin));
	}
	else if (mAdbk)
	{
		mLookupBtn->SetValue(rights.HasRight(CAdbkACL::eAdbkACL_Lookup));
		mReadBtn->SetValue(rights.HasRight(CAdbkACL::eAdbkACL_Read));
		mWriteBtn->SetValue(rights.HasRight(CAdbkACL::eAdbkACL_Write));
		mCreateBtn->SetValue(rights.HasRight(CAdbkACL::eAdbkACL_Create));
		mDeleteBtn->SetValue(rights.HasRight(CAdbkACL::eAdbkACL_Delete));
		mAdminBtn->SetValue(rights.HasRight(CAdbkACL::eAdbkACL_Admin));
	}
	else
	{
		mLookupBtn->SetValue(rights.HasRight(CCalendarACL::eCalACL_ReadFreeBusy));
		mReadBtn->SetValue(rights.HasRight(CCalendarACL::eCalACL_Read));
		mWriteBtn->SetValue(rights.HasRight(CCalendarACL::eCalACL_Write));
		mScheduleBtn->SetValue(rights.HasRight(CCalendarACL::eCalACL_Schedule));
		mCreateBtn->SetValue(rights.HasRight(CCalendarACL::eCalACL_Create));
		mDeleteBtn->SetValue(rights.HasRight(CCalendarACL::eCalACL_Delete));
		mAdminBtn->SetValue(rights.HasRight(CCalendarACL::eCalACL_Admin));
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
		if (mLookupBtn->GetValue() == 1)
			rights.SetRight(CMboxACL::eMboxACL_Lookup, true);
		if (mReadBtn->GetValue() == 1)
			rights.SetRight(CMboxACL::eMboxACL_Read, true);
		if (mSeenBtn->GetValue() == 1)
			rights.SetRight(CMboxACL::eMboxACL_Seen, true);
		if (mWriteBtn->GetValue() == 1)
			rights.SetRight(CMboxACL::eMboxACL_Write, true);
		if (mInsertBtn->GetValue() == 1)
			rights.SetRight(CMboxACL::eMboxACL_Insert, true);
		if (mPostBtn->GetValue() == 1)
			rights.SetRight(CMboxACL::eMboxACL_Post, true);
		if (mCreateBtn->GetValue() == 1)
			rights.SetRight(CMboxACL::eMboxACL_Create, true);
		if (mDeleteBtn->GetValue() == 1)
			rights.SetRight(CMboxACL::eMboxACL_Delete, true);
		if (mAdminBtn->GetValue() == 1)
			rights.SetRight(CMboxACL::eMboxACL_Admin, true);
	}
	else if (mAdbk)
	{
		// Get ACL state from buttons
		if (mLookupBtn->GetValue() == 1)
			rights.SetRight(CAdbkACL::eAdbkACL_Lookup, true);
		if (mReadBtn->GetValue() == 1)
			rights.SetRight(CAdbkACL::eAdbkACL_Read, true);
		if (mWriteBtn->GetValue() == 1)
			rights.SetRight(CAdbkACL::eAdbkACL_Write, true);
		if (mCreateBtn->GetValue() == 1)
			rights.SetRight(CAdbkACL::eAdbkACL_Create, true);
		if (mDeleteBtn->GetValue() == 1)
			rights.SetRight(CAdbkACL::eAdbkACL_Delete, true);
		if (mAdminBtn->GetValue() == 1)
			rights.SetRight(CAdbkACL::eAdbkACL_Admin, true);
	}
	else if (mCalendar)
	{
		// Get ACL state from buttons
		if (mLookupBtn->GetValue() == 1)
			rights.SetRight(CCalendarACL::eCalACL_ReadFreeBusy, true);
		if (mReadBtn->GetValue() == 1)
			rights.SetRight(CCalendarACL::eCalACL_Read, true);
		if (mWriteBtn->GetValue() == 1)
			rights.SetRight(CCalendarACL::eCalACL_Write, true);
		if (mScheduleBtn->GetValue() == 1)
			rights.SetRight(CCalendarACL::eCalACL_Schedule, true);
		if (mCreateBtn->GetValue() == 1)
			rights.SetRight(CCalendarACL::eCalACL_Create, true);
		if (mDeleteBtn->GetValue() == 1)
			rights.SetRight(CCalendarACL::eCalACL_Delete, true);
		if (mAdminBtn->GetValue() == 1)
			rights.SetRight(CCalendarACL::eCalACL_Admin, true);
	}

	return rights;
}

// Set the details
CMboxACLList* CNewACLDialog::GetDetailsMbox(void)
{
	// Create empty list
	CMboxACLList* acl_list = new CMboxACLList;

	// Get ACL state from buttons
	SACLRight rights = GetRights();

	// Copy handle to text with null terminator
	cdstring txt;
	mUIDList->GetText(txt);

	char* s = ::strtok(txt.c_str_mod(), "\r");
	while(s)
	{
		// Make new ACL
		CMboxACL acl;
		acl.SetUID(s);
		acl.SetRights(rights);

		// Add to list
		acl_list->push_back(acl);

		s = ::strtok(nil, "\r");
	}

	return acl_list;
}

// Set the details
CAdbkACLList* CNewACLDialog::GetDetailsAdbk(void)
{
	// Create empty list
	CAdbkACLList* acl_list = new CAdbkACLList;

	// Get ACL state from buttons
	SACLRight rights = GetRights();

	// Copy handle to text with null terminator
	cdstring txt;
	mUIDList->GetText(txt);

	char* s = ::strtok(txt.c_str_mod(), "\r");
	while(s)
	{
		// Make new ACL
		CAdbkACL acl;
		acl.SetUID(s);
		acl.SetRights(rights);

		// Add to list
		acl_list->push_back(acl);

		s = ::strtok(nil, "\r");
	}

	return acl_list;
}

// Set the details
CCalendarACLList* CNewACLDialog::GetDetailsCal(void)
{
	// Create empty list
	CCalendarACLList* acl_list = new CCalendarACLList;

	// Get ACL state from buttons
	SACLRight rights = GetRights();

	// Copy handle to text with null terminator
	cdstring txt;
	mUIDList->GetText(txt);

	char* s = ::strtok(txt.c_str_mod(), "\r");
	while(s)
	{
		// Make new ACL
		CCalendarACL acl;
		acl.SetSmartUID(s);
		acl.SetRights(rights);

		// Add to list
		acl_list->push_back(acl);

		s = ::strtok(nil, "\r");
	}

	return acl_list;
}

// Handle popup command
void CNewACLDialog::DoStylePopup(long index)
{
	switch(index)
	{
	// New ACL wanted
	case eACLStyleNewUser:
		mStylePopup->DoNewStyle(GetRights());
		break;

	// Delete existing ACL
	case eACLStyleDeleteUser:
		mStylePopup->DoDeleteStyle();
		break;

	// Select a style
	default:
		{
			// Get rights for style and set buttons
			SACLRight rights = (mMbox ? CPreferences::sPrefs->mMboxACLStyles.GetValue()[index - 4].second :
										(mAdbk ? CPreferences::sPrefs->mAdbkACLStyles.GetValue()[index - 4].second :
											CPreferences::sPrefs->mAdbkACLStyles.GetValue()[index - 4].second));
			SetACL(rights);
		}
		break;
	}
}
