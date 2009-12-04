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


// Source for CPrefsMailboxBasic class

#include "CPrefsMailboxBasic.h"

#include "CMailboxPopup.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CTextField.h"

#include "JXSecondaryRadioGroup.h"

#include <JXDownRect.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsMailboxBasic::OnCreate()
{
// begin JXLayout1

    mOpenGroup =
        new JXSecondaryRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 200,95);
    assert( mOpenGroup != NULL );

    JXStaticText* obj1 =
        new JXStaticText("Open Mailbox at:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,5, 105,20);
    assert( obj1 != NULL );

    JXTextRadioButton* obj2 =
        new JXTextRadioButton(1, "First Message", mOpenGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 115,20);
    assert( obj2 != NULL );

    JXTextRadioButton* obj3 =
        new JXTextRadioButton(2, "Last Message", mOpenGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,25, 115,20);
    assert( obj3 != NULL );

    JXTextRadioButton* obj4 =
        new JXTextRadioButton(3, "First New Message", mOpenGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,45, 140,20);
    assert( obj4 != NULL );

    mProcessOrder =
        new JXSecondaryRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 225,15, 145,95);
    assert( mProcessOrder != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Process Messages:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 230,5, 120,20);
    assert( obj5 != NULL );

    JXTextRadioButton* obj6 =
        new JXTextRadioButton(1, "Oldest to Newest", mProcessOrder,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 135,20);
    assert( obj6 != NULL );

    JXTextRadioButton* obj7 =
        new JXTextRadioButton(2, "Newest to Oldest", mProcessOrder,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,25, 135,20);
    assert( obj7 != NULL );

    mExpungeOnClose =
        new JXTextCheckbox("Expunge on Close", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,120, 135,20);
    assert( mExpungeOnClose != NULL );

    mWarnOnExpunge =
        new JXTextCheckbox("Warn on Expunge", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,140, 135,20);
    assert( mWarnOnExpunge != NULL );

    JXDownRect* obj8 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,270, 360,65);
    assert( obj8 != NULL );

    mMoveFromINBOX =
        new JXTextCheckbox("Move Read Messages in INBOX", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,260, 210,20);
    assert( mMoveFromINBOX != NULL );

    JXStaticText* obj9 =
        new JXStaticText("Move to:", obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,17, 60,20);
    assert( obj9 != NULL );

    mClearMailbox =
        new CTextInputField(obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 65,15, 230,20);
    assert( mClearMailbox != NULL );

    mWarnOnClear =
        new JXTextCheckbox("Warn when Moving", obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,40, 140,20);
    assert( mWarnOnClear != NULL );

    mClearMailboxPopup =
        new CMailboxPopupButton(true, obj8,
                    JXWidget::kHElastic, JXWidget::kVElastic, 305,17, 30,16);
    assert( mClearMailboxPopup != NULL );

    mNoOpenPreview =
        new JXTextCheckbox("Don't Preview New Message", mOpenGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,70, 187,20);
    assert( mNoOpenPreview != NULL );

    mWarnPuntUnseen =
        new JXTextCheckbox("Warn when Marking All Messages as Seen", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,165, 270,20);
    assert( mWarnPuntUnseen != NULL );

    mDoRollover =
        new JXTextCheckbox("Open Next with New Messages", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,190, 205,20);
    assert( mDoRollover != NULL );

    mRolloverWarn =
        new JXTextCheckbox("Ask before Opening Next", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,210, 170,20);
    assert( mRolloverWarn != NULL );

    mScrollForUnseen =
        new JXTextCheckbox("Scroll Mailbox View to Expose Any Unseen Messages", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,235, 350,20);
    assert( mScrollForUnseen != NULL );

// end JXLayout1

	if (!CMulberryApp::sApp->LoadedPrefs())
		mClearMailboxPopup->Deactivate();

	// Start listening
	ListenTo(mMoveFromINBOX);
	ListenTo(mClearMailboxPopup);
}

// Handle buttons
void CPrefsMailboxBasic::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mMoveFromINBOX)
		{
			OnMoveRead();
			return;
		}
	}
	else if(message.Is(JXMenu::kItemSelected))
	{
    	if (sender == mClearMailboxPopup)
    	{
			const JXMenu::ItemSelected* is = 
				dynamic_cast<const JXMenu::ItemSelected*>(&message);			
			OnChangeMoveTo(is->GetIndex());
			return;
		} 
		
	}
}

// Set prefs
void CPrefsMailboxBasic::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info
	mOpenGroup->SelectItem(copyPrefs->openAtFirst.GetValue() ? 1 :
							(copyPrefs->openAtLast.GetValue() ? 2 : 3));
	mProcessOrder->SelectItem(copyPrefs->mNextIsNewest.GetValue() ? 1 : 2);
	mNoOpenPreview->SetState(JBoolean(copyPrefs->mNoOpenPreview.GetValue()));

	mExpungeOnClose->SetState(JBoolean(copyPrefs->expungeOnClose.GetValue()));
	mWarnOnExpunge->SetState(JBoolean(copyPrefs->warnOnExpunge.GetValue()));

	mWarnPuntUnseen->SetState(JBoolean(copyPrefs->mWarnPuntUnseen.GetValue()));

	mDoRollover->SetState(JBoolean(copyPrefs->mDoRollover.GetValue()));
	mRolloverWarn->SetState(JBoolean(copyPrefs->mRolloverWarn.GetValue()));

	mScrollForUnseen->SetState(JBoolean(copyPrefs->mScrollForUnseen.GetValue()));

	mClearMailbox->SetText(copyPrefs->clear_mailbox.GetValue());

	mWarnOnClear->SetState(JBoolean(copyPrefs->clear_warning.GetValue()));

	mMoveFromINBOX->SetState(JBoolean(copyPrefs->mDoMailboxClear.GetValue()));

	if (!copyPrefs->mDoMailboxClear.GetValue())
	{
		mClearMailbox->Deactivate();
		mClearMailboxPopup->Deactivate();
		mWarnOnClear->Deactivate();
	}
}

// Force update of prefs
bool CPrefsMailboxBasic::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	copyPrefs->openAtFirst.SetValue(mOpenGroup->GetSelectedItem() == 1);
	copyPrefs->openAtLast.SetValue(mOpenGroup->GetSelectedItem() == 2);
	copyPrefs->openAtFirstNew.SetValue(mOpenGroup->GetSelectedItem() == 3);

	copyPrefs->mNextIsNewest.SetValue(mProcessOrder->GetSelectedItem() == 1);
	copyPrefs->mNoOpenPreview.SetValue(mNoOpenPreview->IsChecked());

	copyPrefs->expungeOnClose.SetValue(mExpungeOnClose->IsChecked());
	copyPrefs->warnOnExpunge.SetValue(mWarnOnExpunge->IsChecked());

	copyPrefs->mWarnPuntUnseen.SetValue(mWarnPuntUnseen->IsChecked());

	copyPrefs->mDoRollover.SetValue(mDoRollover->IsChecked());
	copyPrefs->mRolloverWarn.SetValue(mRolloverWarn->IsChecked());

	copyPrefs->mScrollForUnseen.SetValue(mScrollForUnseen->IsChecked());

	copyPrefs->mDoMailboxClear.SetValue(mMoveFromINBOX->IsChecked());

	copyPrefs->clear_mailbox.SetValue(cdstring(mClearMailbox->GetText()));

	copyPrefs->clear_warning.SetValue(mWarnOnClear->IsChecked());
	
	return true;
}

void CPrefsMailboxBasic::OnMoveRead()
{
	// TODO: Add your control notification handler code here
	if (mMoveFromINBOX->IsChecked())
	{
		mClearMailbox->Activate();
		mClearMailbox->Focus();
		mClearMailbox->SelectAll();
		if (CMulberryApp::sApp->LoadedPrefs())
			mClearMailboxPopup->Activate();
		mWarnOnClear->Activate();
	}
	else
	{
		mClearMailbox->Deactivate();
		mClearMailboxPopup->Deactivate();
		mWarnOnClear->Deactivate();
	}
}

// Change move to
void CPrefsMailboxBasic::OnChangeMoveTo(JIndex nID)
{
	cdstring mbox_name;
	if (mClearMailboxPopup->GetSelectedMboxName(mbox_name))
		mClearMailbox->SetText(mbox_name);
}
