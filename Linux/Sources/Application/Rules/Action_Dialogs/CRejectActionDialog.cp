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


// Source for CRejectActionDialog class

#include "CRejectActionDialog.h"

#include "CIdentityPopup.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CPrefsEditHeadFoot.h"

#include <JXDisplay.h>
#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CRejectActionDialog::CRejectActionDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

// Default destructor
CRejectActionDialog::~CRejectActionDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CRejectActionDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 380,220, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 380,220);
    assert( obj1 != NULL );

    mReturnGroup =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 360,55);
    assert( mReturnGroup != NULL );

    JXTextRadioButton* obj2 =
        new JXTextRadioButton(1, "Return Message Headers Only", mReturnGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,5, 210,20);
    assert( obj2 != NULL );

    JXTextRadioButton* obj3 =
        new JXTextRadioButton(2, "Return Entire Message", mReturnGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,25, 215,20);
    assert( obj3 != NULL );

    mIdentityGroup =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,85, 360,55);
    assert( mIdentityGroup != NULL );

    JXTextRadioButton* obj4 =
        new JXTextRadioButton(1, "Use Standard Identity", mIdentityGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,5, 170,20);
    assert( obj4 != NULL );

    JXTextRadioButton* obj5 =
        new JXTextRadioButton(2, "Use Identity:", mIdentityGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,25, 95,20);
    assert( obj5 != NULL );

    mIdentityPopup =
        new CIdentityPopup("",mIdentityGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 105,25, 200,20);
    assert( mIdentityPopup != NULL );

    mCreateDraft =
        new JXTextCheckbox("Create Draft instead of Sending", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,150, 210,20);
    assert( mCreateDraft != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 200,185, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 290,185, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

// end JXLayout

	mIdentityPopup->Reset(CPreferences::sPrefs->mIdentities.GetValue());

	window->SetTitle("Reject Action Options");
	SetButtons(mOKBtn, mCancelBtn);

	ListenTo(mIdentityGroup);
	ListenTo(mIdentityPopup);
}

// Handle OK button
void CRejectActionDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		if (sender == mIdentityGroup)
		{
			JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
			if (index == 1)
				mIdentityPopup->Deactivate();
			else
				mIdentityPopup->Activate();
			return;
		}
	}
	else if (message.Is(JXMenu::kItemSelected))
	{
		if (sender == mIdentityPopup)
		{
			JIndex item = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
			switch(item)
			{
			// New identity wanted
			case eIdentityPopup_New:
				mIdentityPopup->DoNewIdentity(CPreferences::sPrefs);
				break;
			
			// New identity wanted
			case eIdentityPopup_Edit:
				mIdentityPopup->DoEditIdentity(CPreferences::sPrefs);
				break;
			
			// Delete existing identity
			case eIdentityPopup_Delete:
				mIdentityPopup->DoDeleteIdentity(CPreferences::sPrefs);
				break;
			
			// Select an identity
			default:
				mCurrentIdentity = CPreferences::sPrefs->mIdentities.Value()[item - mIdentityPopup->FirstIndex()].GetIdentity();
				break;
			}

			return;
		}
	}

	CDialogDirector::Receive(sender, message);
}

// Set the details
void CRejectActionDialog::SetDetails(CActionItem::CActionReject& details)
{
	mReturnGroup->SelectItem(details.ReturnMessage() ? 2 : 1);

	mIdentityGroup->SelectItem(details.UseTiedIdentity() ? 1 : 2);
	if (details.UseTiedIdentity())
		mIdentityPopup->Deactivate();
	
	mCurrentIdentity = details.GetIdentity();
	mIdentityPopup->SetIdentity(CPreferences::sPrefs, mCurrentIdentity);

	mCreateDraft->SetState(JBoolean(details.CreateDraft()));
}

// Get the details
void CRejectActionDialog::GetDetails(CActionItem::CActionReject& details)
{
	details.SetReturnMessage(mReturnGroup->GetSelectedItem() == 2);

	details.SetTiedIdentity(mIdentityGroup->GetSelectedItem() == 1);

	details.SetIdentity(mCurrentIdentity);

	details.SetCreateDraft(mCreateDraft->IsChecked());
}

bool CRejectActionDialog::PoseDialog(CActionItem::CActionReject& details)
{
	bool result = false;

	// Create the dialog
	CRejectActionDialog* dlog = new CRejectActionDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetDetails(details);

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetDetails(details);
		result = true;
		dlog->Close();
	}

	return result;
}
