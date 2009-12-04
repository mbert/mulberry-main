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


// Source for CPrefsLetterGeneral class

#include "CPrefsLetterGeneral.h"

#include "CMailboxPopup.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CPrefsEditCaption.h"
#include "CPrefsEditHeadFoot.h"
#include "CTextField.h"

#include <JXDownRect.h>
#include <JXIntegerInput.h>
#include <JXInputField.h>
#include "JXSecondaryRadioGroup.h"
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>
#include "CInputField.h"

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsLetterGeneral::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Default Domain for\nOutgoing Email:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,0, 115,35);
    assert( obj1 != NULL );

    mMailDomain =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,5, 210,20);
    assert( mMailDomain != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Tab Width:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,35, 70,20);
    assert( obj2 != NULL );

    mSpacesPerTab =
        new CInputField<JXIntegerInput>(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,33, 40,20);
    assert( mSpacesPerTab != NULL );

    mTabSpace =
        new JXTextCheckbox("Use Spaces for Tabs", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 175,35, 155,20);
    assert( mTabSpace != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Wrap Length:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,58, 85,20);
    assert( obj3 != NULL );

    mWrapLength =
        new CInputField<JXIntegerInput>(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,56, 40,20);
    assert( mWrapLength != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Reply Prefix:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,81, 95,20);
    assert( obj4 != NULL );

    mReplyQuote =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,79, 60,20);
    assert( mReplyQuote != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Caption:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,81, 55,20);
    assert( obj5 != NULL );

    mReplySetBtn =
        new JXTextButton("Set...", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 250,78, 50,20);
    assert( mReplySetBtn != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Forward Prefix:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,106, 95,20);
    assert( obj6 != NULL );

    mForwardQuote =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,104, 60,20);
    assert( mForwardQuote != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Caption:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,106, 55,20);
    assert( obj7 != NULL );

    mForwardSetBtn =
        new JXTextButton("Set...", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 250,103, 50,20);
    assert( mForwardSetBtn != NULL );

    JXStaticText* obj8 =
        new JXStaticText("Forward Subject:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,129, 105,20);
    assert( obj8 != NULL );

    mForwardSubject =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,127, 210,20);
    assert( mForwardSubject != NULL );

    JXStaticText* obj9 =
        new JXStaticText("Print Captions:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 160,152, 90,20);
    assert( obj9 != NULL );

    mPrintSetBtn =
        new JXTextButton("Set...", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 250,149, 50,20);
    assert( mPrintSetBtn != NULL );

    mSeparateBCC =
        new JXTextCheckbox("Separate Bcc Message with Caption:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,175, 235,20);
    assert( mSeparateBCC != NULL );

    mBCCCaption =
        new JXTextButton("Set...", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 250,172, 50,20);
    assert( mBCCCaption != NULL );

    mSaveGroup =
        new JXSecondaryRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,215, 360,95);
    assert( mSaveGroup != NULL );

    mSaveToFile =
        new JXTextRadioButton(0, "Always Save to Disk", mSaveGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,7, 150,20);
    assert( mSaveToFile != NULL );

    mSaveToMailbox =
        new JXTextRadioButton(1, "Always Save to Mailbox", mSaveGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,27, 170,20);
    assert( mSaveToMailbox != NULL );

    mSaveToMailboxName =
        new CTextInputField(mSaveGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 32,52, 260,20);
    assert( mSaveToMailboxName != NULL );

    mSaveToMailboxPopup =
        new CMailboxPopupButton(false,mSaveGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 302,54, 30,16);
    assert( mSaveToMailboxPopup != NULL );

    mSaveChoose =
        new JXTextRadioButton(2, "Allow Choice of where to Save", mSaveGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,72, 210,20);
    assert( mSaveChoose != NULL );

    JXStaticText* obj10 =
        new JXStaticText("Save Draft", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,205, 70,20);
    assert( obj10 != NULL );

    mAutoSaveDrafts =
        new JXTextCheckbox("Save Draft for Recovery Every:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,320, 205,20);
    assert( mAutoSaveDrafts != NULL );

    mAutoSaveDraftsInterval =
        new CInputField<JXIntegerInput>(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 220,318, 40,20);
    assert( mAutoSaveDraftsInterval != NULL );

    JXStaticText* obj11 =
        new JXStaticText("seconds", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 265,320, 60,20);
    assert( obj11 != NULL );

// end JXLayout1

	if (!CMulberryApp::sApp->LoadedPrefs())
		mSaveToMailboxPopup->Deactivate();

	// Start listening
	ListenTo(mReplySetBtn);
	ListenTo(mForwardSetBtn);
	ListenTo(mPrintSetBtn);
	ListenTo(mSeparateBCC);
	ListenTo(mBCCCaption);
	ListenTo(mSaveGroup);
	ListenTo(mSaveToMailboxPopup);
	ListenTo(mAutoSaveDrafts);
}

void CPrefsLetterGeneral::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mReplySetBtn)
		{
			EditCaption(&mCopyPrefs->mReplyStart,
						NULL,
						&mCopyPrefs->mReplyEnd,
						NULL,
						&mCopyPrefs->mReplyCursorTop,
						NULL,
						NULL,
						NULL);
			return;
		}
		else if (sender == mForwardSetBtn)
		{
			EditCaption(&mCopyPrefs->mForwardStart,
						NULL,
						&mCopyPrefs->mForwardEnd,
						NULL,
						&mCopyPrefs->mForwardCursorTop,
						NULL,
						NULL,
						NULL);
			return;
		}
		else if (sender == mPrintSetBtn)
		{
			EditCaption(&mCopyPrefs->mHeaderCaption,
						&mCopyPrefs->mLtrHeaderCaption,
						&mCopyPrefs->mFooterCaption,
						&mCopyPrefs->mLtrFooterCaption,
						NULL,
						&mCopyPrefs->mHeaderBox,
						&mCopyPrefs->mFooterBox,
						&mCopyPrefs->mPrintSummary);
			return;
		}
		else if (sender == mBCCCaption)
		{
			EditBCCCaption();
			return;
		}
	}
	else if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mSeparateBCC)
		{
			mBCCCaption->SetActive(mSeparateBCC->IsChecked());
		}
		else if (sender == mSeparateBCC)
		{
			mAutoSaveDraftsInterval->SetActive(mAutoSaveDrafts->IsChecked());
		}
	}
	else if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		if (sender == mSaveGroup)
		{
			if (mSaveGroup->GetSelectedItem() != eSaveDraftToMailbox)
			{
				mSaveToMailboxName->Deactivate();
				mSaveToMailboxPopup->Deactivate();
			}
			else
			{
				mSaveToMailboxName->Activate();
				mSaveToMailboxPopup->Activate();
			}
			return;
		}
	}
	else if(message.Is(JXMenu::kItemSelected))
	{
    	if (sender == mSaveToMailboxPopup)
    	{
			const JXMenu::ItemSelected* is = 
				dynamic_cast<const JXMenu::ItemSelected*>(&message);			
			OnChangeSaveTo(is->GetIndex());
			return;
		} 
		
	}
	
	CTabPanel::Receive(sender, message);
}

// Set prefs
void CPrefsLetterGeneral::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Save ref to prefs
	mCopyPrefs = copyPrefs;

	mMailDomain->SetText(copyPrefs->mMailDomain.GetValue());

	// Copy text to edit fields
	mSpacesPerTab->SetValue(copyPrefs->spaces_per_tab.GetValue());
	mTabSpace->SetState(JBoolean(copyPrefs->mTabSpace.GetValue()));
	mWrapLength->SetValue(copyPrefs->wrap_length.GetValue());

	mReplyQuote->SetText(copyPrefs->mReplyQuote.GetValue());
	mForwardQuote->SetText(copyPrefs->mForwardQuote.GetValue());
	mForwardSubject->SetText(copyPrefs->mForwardSubject.GetValue());

	mSeparateBCC->SetState(JBoolean(copyPrefs->mSeparateBCC.GetValue()));
	if (!mSeparateBCC->IsChecked())
		mBCCCaption->Deactivate();

	mSaveGroup->SelectItem(copyPrefs->mSaveOptions.GetValue().GetValue());

	mSaveToMailboxName->SetText(copyPrefs->mSaveMailbox.GetValue());

	if (!copyPrefs->mSaveOptions.GetValue().GetValue() != eSaveDraftToMailbox)
	{
		mSaveToMailboxName->Deactivate();
		mSaveToMailboxPopup->Deactivate();
	}
	
	mAutoSaveDrafts->SetState(copyPrefs->mAutoSaveDrafts.GetValue());
	mAutoSaveDraftsInterval->SetValue(copyPrefs->mAutoSaveDraftsInterval.GetValue());
	mAutoSaveDraftsInterval->SetActive(copyPrefs->mAutoSaveDrafts.GetValue());
}

// Force update of prefs
bool CPrefsLetterGeneral::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	copyPrefs->mMailDomain.SetValue(cdstring(mMailDomain->GetText()));

	// Copy info from panel into prefs
	JInteger value;
	mSpacesPerTab->GetValue(&value);
	copyPrefs->spaces_per_tab.SetValue(value);
	copyPrefs->mTabSpace.SetValue(mTabSpace->IsChecked());
	mWrapLength->GetValue(&value);
	copyPrefs->wrap_length.SetValue(value);

	copyPrefs->mReplyQuote.SetValue(cdstring(mReplyQuote->GetText()));
	copyPrefs->mForwardQuote.SetValue(cdstring(mForwardQuote->GetText()));
	copyPrefs->mForwardSubject.SetValue(cdstring(mForwardSubject->GetText()));

	copyPrefs->mSeparateBCC.SetValue(mSeparateBCC->IsChecked());
	
	copyPrefs->mSaveOptions.SetValue((ESaveOptions) mSaveGroup->GetSelectedItem());

	copyPrefs->mSaveMailbox.SetValue(cdstring(mSaveToMailboxName->GetText()));

	copyPrefs->mAutoSaveDrafts.SetValue(mAutoSaveDrafts->IsChecked());
	mAutoSaveDraftsInterval->GetValue(&value);
	copyPrefs->mAutoSaveDraftsInterval.SetValue(value);
	
	return true;
}

// Edit caption
void CPrefsLetterGeneral::EditCaption(CPreferenceValueMap<cdstring>* text1,
								CPreferenceValueMap<cdstring>* text2,
								CPreferenceValueMap<cdstring>* text3,
								CPreferenceValueMap<cdstring>* text4,
								CPreferenceValueMap<bool>* cursor_top,
								CPreferenceValueMap<bool>* box1,
								CPreferenceValueMap<bool>* box2,
								CPreferenceValueMap<bool>* summary)
{
	CPrefsEditCaption::PoseDialog(mCopyPrefs, text1, text2, text3, text4, cursor_top, box1, box2, summary);
}

// Edit bcc caption
void CPrefsLetterGeneral::EditBCCCaption()
{
	cdstring title = "Set BCC Caption";
	CPrefsEditHeadFoot::PoseDialog(mCopyPrefs->mBCCCaption.Value(), title, mCopyPrefs, false);
}

// Change move to
void CPrefsLetterGeneral::OnChangeSaveTo(JIndex nID)
{
	cdstring mbox_name;
	if (mSaveToMailboxPopup->GetSelectedMboxName(mbox_name))
		mSaveToMailboxName->SetText(mbox_name);
}
