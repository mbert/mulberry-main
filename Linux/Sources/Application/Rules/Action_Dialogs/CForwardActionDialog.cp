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


// Source for CForwardActionDialog class

#include "CForwardActionDialog.h"

#include "CIdentityPopup.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CPrefsEditHeadFoot.h"
#include "CTextField.h"

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
CForwardActionDialog::CForwardActionDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

// Default destructor
CForwardActionDialog::~CForwardActionDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CForwardActionDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 380,310, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 380,310);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("To:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,17, 35,20);
    assert( obj2 != NULL );

    mTo =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 55,15, 300,20);
    assert( mTo != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Cc:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,42, 35,20);
    assert( obj3 != NULL );

    mCC =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 55,40, 300,20);
    assert( mCC != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Bcc:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,67, 35,20);
    assert( obj4 != NULL );

    mBcc =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 55,65, 300,20);
    assert( mBcc != NULL );

    mQuote =
        new JXTextCheckbox("Quote original Message", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,95, 170,20);
    assert( mQuote != NULL );

    mTextBtn =
        new JXTextButton("Forward Text...", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,140, 110,25);
    assert( mTextBtn != NULL );

    mAttach =
        new JXTextCheckbox("Attach original Message", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,115, 170,20);
    assert( mAttach != NULL );

    mIdentityGroup =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,175, 360,55);
    assert( mIdentityGroup != NULL );

    JXTextRadioButton* obj5 =
        new JXTextRadioButton(1, "Use Standard Identity", mIdentityGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,5, 170,20);
    assert( obj5 != NULL );

    JXTextRadioButton* obj6 =
        new JXTextRadioButton(2, "Use Identity:", mIdentityGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,25, 95,20);
    assert( obj6 != NULL );

    mIdentityPopup =
        new CIdentityPopup("",mIdentityGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 105,25, 200,20);
    assert( mIdentityPopup != NULL );

    mCreateDraft =
        new JXTextCheckbox("Create Draft instead of Sending", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,240, 210,20);
    assert( mCreateDraft != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 200,275, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 290,275, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

// end JXLayout

	mIdentityPopup->Reset(CPreferences::sPrefs->mIdentities.GetValue());

	window->SetTitle("Forward Action Options");
	SetButtons(mOKBtn, mCancelBtn);

	ListenTo(mTextBtn);
	ListenTo(mIdentityGroup);
	ListenTo(mIdentityPopup);
}

void CForwardActionDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mTextBtn)
		{
			SetText();
			return;
		}
	}
	else if (message.Is(JXRadioGroup::kSelectionChanged))
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
void CForwardActionDialog::SetDetails(CActionItem::CActionForward& details)
{
	mTo->SetText(details.Addrs().mTo);

	mCC->SetText(details.Addrs().mCC);

	mBcc->SetText(details.Addrs().mBcc);

	mQuote->SetState(JBoolean(details.Quote()));

	mAttach->SetState(JBoolean(details.Attach()));

	mText = details.GetText();

	mIdentityGroup->SelectItem(details.UseTiedIdentity() ? 1 : 2);
	if (details.UseTiedIdentity())
		mIdentityPopup->Deactivate();
	
	mCurrentIdentity = details.GetIdentity();
	mIdentityPopup->SetIdentity(CPreferences::sPrefs, mCurrentIdentity);

	mCreateDraft->SetState(JBoolean(details.CreateDraft()));
}

// Get the details
void CForwardActionDialog::GetDetails(CActionItem::CActionForward& details)
{
	details.Addrs().mTo = mTo->GetText();

	details.Addrs().mCC = mCC->GetText();

	details.Addrs().mBcc = mBcc->GetText();

	details.SetQuote(mQuote->IsChecked());

	details.SetAttach(mAttach->IsChecked());

	details.SetText(mText);

	details.SetTiedIdentity(mIdentityGroup->GetSelectedItem() == 1);

	details.SetIdentity(mCurrentIdentity);

	details.SetCreateDraft(mCreateDraft->IsChecked());
}

// Called during idle
void CForwardActionDialog::SetText()
{
	cdstring title("Set Forward Text");
	CPrefsEditHeadFoot::PoseDialog(mText, title, CPreferences::sPrefs, false);
}

bool CForwardActionDialog::PoseDialog(CActionItem::CActionForward& details)
{
	bool result = false;

	// Create the dialog
	CForwardActionDialog* dlog = new CForwardActionDialog(JXGetApplication());
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
