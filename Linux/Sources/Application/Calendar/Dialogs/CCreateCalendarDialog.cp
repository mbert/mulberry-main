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


// Source for CCreateCalendarDialog class

#include "CCreateCalendarDialog.h"

#include "CAdminLock.h"
#include "CINETCommon.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTextField.h"

#include "CCalendarAccount.h"
#include "CCalendarProtocol.h"
#include "CCalendarStoreManager.h"

#include <JXWindow.h>
#include <JXDownRect.h>
#include <JXUpRect.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextRadioButton.h>
#include <JXRadioGroup.h>
#include <JXInputField.h>
#include <JXTextCheckbox.h>
#include "TPopupMenu.h"

#include <jXGlobals.h>

#include <string.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CCreateCalendarDialog::CCreateCalendarDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

// Default destructor
CCreateCalendarDialog::~CCreateCalendarDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CCreateCalendarDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 435,305, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 435,305);
    assert( obj1 != NULL );

    mDirectoryGroup =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 415,50);
    assert( mDirectoryGroup != NULL );

    mCalendar =
        new JXTextRadioButton(1, "Create a Calendar", mDirectoryGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 250,20);
    assert( mCalendar != NULL );

    mDirectory =
        new JXTextRadioButton(2, "Create a Calendar Hierarchy", mDirectoryGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,25, 250,20);
    assert( mDirectory != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Name:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,65, 350,20);
    assert( obj2 != NULL );

    mCalendarName =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,85, 390,20);
    assert( mCalendarName != NULL );

    mWhichPath =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,115, 415,80);
    assert( mWhichPath != NULL );

    mFullPath =
        new JXTextRadioButton(eFullPathname, "Full Pathname", mWhichPath,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 140,20);
    assert( mFullPath != NULL );

    mUseDirectory =
        new JXTextRadioButton(eInHierarchy, "Create In Hierarchy:", mWhichPath,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,25, 140,20);
    assert( mUseDirectory != NULL );

    mInHierarchy =
        new CTextInputField(mWhichPath,
                    JXWidget::kHElastic, JXWidget::kVElastic, 30,50, 370,20);
    assert( mInHierarchy != NULL );

    JXStaticText* obj3 =
        new JXStaticText("In Account:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,212, 80,20);
    assert( obj3 != NULL );

    mAccount =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 100,210, 310,20);
    assert( mAccount != NULL );

    mAccountPopup =
        new HPopupMenu( "", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 100,210, 250,25);
    assert( mAccountPopup != NULL );

    mSubscribe =
        new JXTextCheckbox("Subscribe when Created", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,245, 190,20);
    assert( mSubscribe != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 255,270, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 345,270, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

// end JXLayout
	window->SetTitle("Create Calendar");
	SetButtons(mOKBtn, mCancelBtn);

	mUseSubscribe = true;

	// The only way to make it read only
	mInHierarchy->Deactivate();
	mAccount->Deactivate();
	
	InitAccountMenu();
	ListenTo(mAccountPopup);
}

void CCreateCalendarDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		const JXRadioGroup::SelectionChanged* info =
			dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message);
		assert(info != NULL);
		if (sender == mWhichPath)
		{
			mInHierarchy->SetActive(info->GetID() == eInHierarchy);
		}
		else if (sender == mDirectoryGroup)
		{
			mSubscribe->SetActive(info->GetID() == 1);
			mUseSubscribe = (info->GetID() == 1);
		}
		return;
	}

	CDialogDirector::Receive(sender, message);
	return;
}

// Set the details
void CCreateCalendarDialog::SetDetails(SCreateCalendar* create)
{
	// If no account use the popup
	if (create->account.empty())
	{
		mAccount->Hide();
	}
	else
	{
		mAccountPopup->Hide();
		mAccount->SetText(create->account);
	}

	mInHierarchy->SetText(create->parent);

	mWhichPath->SelectItem((create->use_wd && !create->parent.empty()) ? eInHierarchy : eFullPathname);

	if (create->parent.empty())
	{
		mUseDirectory->Deactivate();
		mInHierarchy->Deactivate();
	}
}

// Get the details
void CCreateCalendarDialog::GetDetails(SCreateCalendar* result)
{
	result->directory = (mDirectoryGroup->GetSelectedItem() == 2);
	result->new_name = mCalendarName->GetText();

	result->use_wd = (mWhichPath->GetSelectedItem() != eFullPathname);

	result->subscribe = mUseSubscribe && (mSubscribe->IsChecked() == 1);

	// Get account if not specified
	if (result->account.empty())
	{
		JIndex acct_num = mAccountPopup->GetValue() - 1;
		result->account = CPreferences::sPrefs->mCalendarAccounts.GetValue().at(acct_num)->GetName();
	}
}

// Called during idle
void CCreateCalendarDialog::InitAccountMenu(void)
{
	// Delete previous items
	mAccountPopup->RemoveAllItems();

	// Add each mail account
	for(CCalendarAccountList::const_iterator iter = CPreferences::sPrefs->mCalendarAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mCalendarAccounts.GetValue().end(); iter++)
		//Append it as a radio button
		mAccountPopup->AppendItem((*iter)->GetName(), kFalse, kTrue);

	mAccountPopup->SetToPopupChoice(kTrue, 1);
}

bool CCreateCalendarDialog::PoseDialog(SCreateCalendar& create)
{
	bool result = false;

	// Create the dialog
	CCreateCalendarDialog* dlog = new CCreateCalendarDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetDetails(&create);

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetDetails(&create);
		result = !create.new_name.empty();
		dlog->Close();
	}

	return result;
}
