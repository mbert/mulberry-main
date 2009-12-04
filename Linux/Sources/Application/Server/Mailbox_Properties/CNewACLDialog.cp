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
//#include "CMailboxPropDialog.h"
#include "CPreferences.h"
//#include "CPropMailboxACL.h"
#include "CTextDisplay.h"

#include <JXColormap.h>
#include "JXMultiImageCheckbox.h"
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

const short cAdbkStyleBtnMove = 24;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CNewACLDialog::CNewACLDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
	mMbox = false;
	mAdbk = false;
	mCalendar = false;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CNewACLDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 300,210, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 300,210);
    assert( obj1 != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 210,175, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 120,175, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    JXStaticText* obj2 =
        new JXStaticText("User IDs:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 55,15);
    assert( obj2 != NULL );
    obj2->SetFontSize(10);
    const JFontStyle obj2_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj2->SetFontStyle(obj2_style);

    mUIDList =
        new CTextInputDisplay(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,25, 280,75);
    assert( mUIDList != NULL );

    mLookupBtn =
        new JXMultiImageCheckbox(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 50,110, 24,24);
    assert( mLookupBtn != NULL );

    mReadBtn =
        new JXMultiImageCheckbox(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 74,110, 24,24);
    assert( mReadBtn != NULL );

    mSeenBtn =
        new JXMultiImageCheckbox(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 98,110, 24,24);
    assert( mSeenBtn != NULL );

    mWriteBtn =
        new JXMultiImageCheckbox(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 122,110, 24,24);
    assert( mWriteBtn != NULL );

    mInsertBtn =
        new JXMultiImageCheckbox(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 146,110, 24,24);
    assert( mInsertBtn != NULL );

    mScheduleBtn =
        new JXMultiImageCheckbox(mInsertBtn,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 24,24);
    assert( mScheduleBtn != NULL );

    mPostBtn =
        new JXMultiImageCheckbox(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 170,110, 24,24);
    assert( mPostBtn != NULL );

    mCreateBtn =
        new JXMultiImageCheckbox(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 194,110, 24,24);
    assert( mCreateBtn != NULL );

    mDeleteBtn =
        new JXMultiImageCheckbox(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 218,110, 24,24);
    assert( mDeleteBtn != NULL );

    mAdminBtn =
        new JXMultiImageCheckbox(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 242,110, 24,24);
    assert( mAdminBtn != NULL );

    mStylePopup =
        new CACLStylePopup("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 50,145, 35,20);
    assert( mStylePopup != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Set:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,115, 30,15);
    assert( obj3 != NULL );
    obj3->SetFontSize(10);
    const JFontStyle obj3_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj3->SetFontStyle(obj3_style);

    JXStaticText* obj4 =
        new JXStaticText("Style:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,150, 40,15);
    assert( obj4 != NULL );
    obj4->SetFontSize(10);
    const JFontStyle obj4_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj4->SetFontStyle(obj4_style);

// end JXLayout
	window->SetTitle("New User ACL");
	SetButtons(mOKBtn, mCancelBtn);

	// Set icons
	mLookupBtn->SetImage(IDI_ACL_LOOKUP);
	mReadBtn->SetImage(IDI_ACL_READ);
	mSeenBtn->SetImage(IDI_ACL_SEEN);
	mWriteBtn->SetImage(IDI_ACL_WRITE);
	mInsertBtn->SetImage(IDI_ACL_INSERT);
	mScheduleBtn->SetImage(IDI_ACL_INSERT);
	mPostBtn->SetImage(IDI_ACL_POST);
	mCreateBtn->SetImage(IDI_ACL_CREATE);
	mDeleteBtn->SetImage(IDI_ACL_DELETE);
	mAdminBtn->SetImage(IDI_ACL_ADMIN);
	
	// Create popup
	ListenTo(mStylePopup);
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
		mWriteBtn->Move(-cAdbkStyleBtnMove, 0);
		mCreateBtn->Move(-3*cAdbkStyleBtnMove, 0);
		mDeleteBtn->Move(-3*cAdbkStyleBtnMove, 0);
		mAdminBtn->Move(-3*cAdbkStyleBtnMove, 0);
	}
	if (mCalendar)
	{
		mSeenBtn->Hide();
		mInsertBtn->Hide();
		mPostBtn->Hide();

		// Move others
		mWriteBtn->Move(-cAdbkStyleBtnMove, 0);
		mScheduleBtn->Move(-cAdbkStyleBtnMove, 0);
		mCreateBtn->Move(-2*cAdbkStyleBtnMove, 0);
		mDeleteBtn->Move(-2*cAdbkStyleBtnMove, 0);
		mAdminBtn->Move(-2*cAdbkStyleBtnMove, 0);
	}

	// Force style popup to proper type
	mStylePopup->Reset(mbox);
}

// Handle OK button
void CNewACLDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXMenu::kItemSelected))
	{
		if (sender == mStylePopup)
		{
			JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
			DoStylePopup(index);
			return;
		}
	}

	CDialogDirector::Receive(sender, message);
}

// Set buttons from rights
void CNewACLDialog::SetACL(SACLRight rights)
{
	if (mMbox)
	{
		mLookupBtn->SetState(JBoolean(rights.HasRight(CMboxACL::eMboxACL_Lookup)));
		mReadBtn->SetState(JBoolean(rights.HasRight(CMboxACL::eMboxACL_Read)));
		mSeenBtn->SetState(JBoolean(rights.HasRight(CMboxACL::eMboxACL_Seen)));
		mWriteBtn->SetState(JBoolean(rights.HasRight(CMboxACL::eMboxACL_Write)));
		mInsertBtn->SetState(JBoolean(rights.HasRight(CMboxACL::eMboxACL_Insert)));
		mPostBtn->SetState(JBoolean(rights.HasRight(CMboxACL::eMboxACL_Post)));
		mCreateBtn->SetState(JBoolean(rights.HasRight(CMboxACL::eMboxACL_Create)));
		mDeleteBtn->SetState(JBoolean(rights.HasRight(CMboxACL::eMboxACL_Delete)));
		mAdminBtn->SetState(JBoolean(rights.HasRight(CMboxACL::eMboxACL_Admin)));
	}
	else if (mAdbk)
	{
		mLookupBtn->SetState(JBoolean(rights.HasRight(CAdbkACL::eAdbkACL_Lookup)));
		mReadBtn->SetState(JBoolean(rights.HasRight(CAdbkACL::eAdbkACL_Read)));
		mWriteBtn->SetState(JBoolean(rights.HasRight(CAdbkACL::eAdbkACL_Write)));
		mCreateBtn->SetState(JBoolean(rights.HasRight(CAdbkACL::eAdbkACL_Create)));
		mDeleteBtn->SetState(JBoolean(rights.HasRight(CAdbkACL::eAdbkACL_Delete)));
		mAdminBtn->SetState(JBoolean(rights.HasRight(CAdbkACL::eAdbkACL_Admin)));
	}
	else
	{
		mLookupBtn->SetState(JBoolean(rights.HasRight(CCalendarACL::eCalACL_ReadFreeBusy)));
		mReadBtn->SetState(JBoolean(rights.HasRight(CCalendarACL::eCalACL_Read)));
		mWriteBtn->SetState(JBoolean(rights.HasRight(CCalendarACL::eCalACL_Write)));
		mScheduleBtn->SetState(JBoolean(rights.HasRight(CCalendarACL::eCalACL_Schedule)));
		mCreateBtn->SetState(JBoolean(rights.HasRight(CCalendarACL::eCalACL_Create)));
		mDeleteBtn->SetState(JBoolean(rights.HasRight(CCalendarACL::eCalACL_Delete)));
		mAdminBtn->SetState(JBoolean(rights.HasRight(CCalendarACL::eCalACL_Admin)));
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
		if (mLookupBtn->IsChecked())
			rights.SetRight(CMboxACL::eMboxACL_Lookup, true);
		if (mReadBtn->IsChecked())
			rights.SetRight(CMboxACL::eMboxACL_Read, true);
		if (mSeenBtn->IsChecked())
			rights.SetRight(CMboxACL::eMboxACL_Seen, true);
		if (mWriteBtn->IsChecked())
			rights.SetRight(CMboxACL::eMboxACL_Write, true);
		if (mInsertBtn->IsChecked())
			rights.SetRight(CMboxACL::eMboxACL_Insert, true);
		if (mPostBtn->IsChecked())
			rights.SetRight(CMboxACL::eMboxACL_Post, true);
		if (mCreateBtn->IsChecked())
			rights.SetRight(CMboxACL::eMboxACL_Create, true);
		if (mDeleteBtn->IsChecked())
			rights.SetRight(CMboxACL::eMboxACL_Delete, true);
		if (mAdminBtn->IsChecked())
			rights.SetRight(CMboxACL::eMboxACL_Admin, true);
	}
	else if (mAdbk)
	{
		// Get ACL state from buttons
		if (mLookupBtn->IsChecked())
			rights.SetRight(CAdbkACL::eAdbkACL_Lookup, true);
		if (mReadBtn->IsChecked())
			rights.SetRight(CAdbkACL::eAdbkACL_Read, true);
		if (mWriteBtn->IsChecked())
			rights.SetRight(CAdbkACL::eAdbkACL_Write, true);
		if (mCreateBtn->IsChecked())
			rights.SetRight(CAdbkACL::eAdbkACL_Create, true);
		if (mDeleteBtn->IsChecked())
			rights.SetRight(CAdbkACL::eAdbkACL_Delete, true);
		if (mAdminBtn->IsChecked())
			rights.SetRight(CAdbkACL::eAdbkACL_Admin, true);
	}
	else
	{
		// Get ACL state from buttons
		if (mLookupBtn->IsChecked())
			rights.SetRight(CCalendarACL::eCalACL_ReadFreeBusy, true);
		if (mReadBtn->IsChecked())
			rights.SetRight(CCalendarACL::eCalACL_Read, true);
		if (mWriteBtn->IsChecked())
			rights.SetRight(CCalendarACL::eCalACL_Write, true);
		if (mScheduleBtn->IsChecked())
			rights.SetRight(CCalendarACL::eCalACL_Schedule, true);
		if (mCreateBtn->IsChecked())
			rights.SetRight(CCalendarACL::eCalACL_Create, true);
		if (mDeleteBtn->IsChecked())
			rights.SetRight(CCalendarACL::eCalACL_Delete, true);
		if (mAdminBtn->IsChecked())
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

	// Copy text and get each line
	cdstring txt(mUIDList->GetText());
	char* s = ::strtok(txt.c_str_mod(), "\n");
	while(s)
	{
		// Make new ACL
		CMboxACL acl;
		acl.SetUID(s);
		acl.SetRights(rights);

		// Add to list
		acl_list->push_back(acl);

		s = ::strtok(nil, "\n");
	}

	return acl_list;
}

// Set the details
CAdbkACLList* CNewACLDialog::GetDetailsAdbk()
{
	// Create empty list
	CAdbkACLList* acl_list = new CAdbkACLList;

	// Get ACL state from buttons
	SACLRight rights = GetRights();

	// Copy text and get each line
	cdstring txt(mUIDList->GetText());
	char* s = ::strtok(txt.c_str_mod(), "\n");
	while(s)
	{
		// Make new ACL
		CAdbkACL acl;
		acl.SetUID(s);
		acl.SetRights(rights);

		// Add to list
		acl_list->push_back(acl);

		s = ::strtok(nil, "\n");
	}

	return acl_list;
}

// Set the details
CCalendarACLList* CNewACLDialog::GetDetailsCal()
{
	// Create empty list
	CCalendarACLList* acl_list = new CCalendarACLList;

	// Get ACL state from buttons
	SACLRight rights = GetRights();

	// Copy text and get each line
	cdstring txt(mUIDList->GetText());
	char* s = ::strtok(txt.c_str_mod(), "\n");
	while(s)
	{
		// Make new ACL
		CCalendarACL acl;
		acl.SetSmartUID(s);
		acl.SetRights(rights);

		// Add to list
		acl_list->push_back(acl);

		s = ::strtok(nil, "\n");
	}

	return acl_list;
}

// Handle popup command
void CNewACLDialog::DoStylePopup(JIndex index)
{
	switch(index)
	{
	// New ACL wanted
	case CACLStylePopup::eNewUser:
		mStylePopup->DoNewStyle(GetRights());
		break;

	// Delete existing ACL
	case CACLStylePopup::eDeleteUser:
		mStylePopup->DoDeleteStyle();
		break;

	// Select a style
	default:
		{
			// Get rights for style and set buttons
			SACLRight rights = CPreferences::sPrefs->mMboxACLStyles.GetValue()[index - CACLStylePopup::eFirstStyle].second;

			// Change the rights
			SetACL(rights);
		}
		break;
	}
}

bool CNewACLDialog::PoseDialog(CMboxACLList*& details)
{
	bool result = false;

	CNewACLDialog* dlog = new CNewACLDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetDetails(true, false, false);

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		details = dlog->GetDetailsMbox();
		result = true;
		dlog->Close();
	}

	return result;
}

bool CNewACLDialog::PoseDialog(CAdbkACLList*& details)
{
	bool result = false;

	CNewACLDialog* dlog = new CNewACLDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetDetails(false, true, false);

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		details = dlog->GetDetailsAdbk();
		result = true;
		dlog->Close();
	}

	return result;
}

bool CNewACLDialog::PoseDialog(CCalendarACLList*& details)
{
	bool result = false;

	CNewACLDialog* dlog = new CNewACLDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetDetails(false, false, true);

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		details = dlog->GetDetailsCal();
		result = true;
		dlog->Close();
	}

	return result;
}
