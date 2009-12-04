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


// Source for CPrefsMessageGeneral class

#include "CPrefsMessageGeneral.h"

#include "CPreferences.h"
#include "CPrefsEditCaption.h"
#include "CTextField.h"

#include "JXSecondaryRadioGroup.h"

#include <JXColormap.h>
#include <JXDownRect.h>
#include <JXStaticText.h>
#include <JXInputField.h>
#include <JXIntegerInput.h>
#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>
#include "CInputField.h"

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsMessageGeneral::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Extension for Saved Messages:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,7, 185,20);
    assert( obj1 != NULL );

    mSaveExtension =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 205,5, 50,20);
    assert( mSaveExtension != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Print Captions:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,32, 100,20);
    assert( obj2 != NULL );

    mPrintSetBtn =
        new JXTextButton("Set...", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 110,30, 50,20);
    assert( mPrintSetBtn != NULL );

    mSizeLimitGroup =
        new JXSecondaryRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,70, 360,45);
    assert( mSizeLimitGroup != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Warn when Opening a Message bigger than:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,60, 260,20);
    assert( obj3 != NULL );

    JXTextRadioButton* obj4 =
        new JXTextRadioButton(1, "No Limit", mSizeLimitGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,15, 80,20);
    assert( obj4 != NULL );

    JXTextRadioButton* obj5 =
        new JXTextRadioButton(2, "Size:", mSizeLimitGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,15, 60,20);
    assert( obj5 != NULL );

    mWarnMessageSize =
        new CInputField<JXIntegerInput>(mSizeLimitGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 190,15, 65,20);
    assert( mWarnMessageSize != NULL );

    JXStaticText* obj6 =
        new JXStaticText("KBytes", mSizeLimitGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 262,17, 55,20);
    assert( obj6 != NULL );

    mReplyChoiceGroup =
        new JXSecondaryRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,135, 360,55);
    assert( mReplyChoiceGroup != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Allow Choice of who to Reply to when:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,125, 230,20);
    assert( obj7 != NULL );

    JXTextRadioButton* obj8 =
        new JXTextRadioButton(1, "Control key is Down", mReplyChoiceGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 150,20);
    assert( obj8 != NULL );

    JXTextRadioButton* obj9 =
        new JXTextRadioButton(2, "Control key is not Down", mReplyChoiceGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,30, 170,20);
    assert( obj9 != NULL );

    mForwardGroup =
        new JXRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,195, 360,135);
    assert( mForwardGroup != NULL );
    mForwardGroup->SetBorderWidth(0);

    JXDownRect* obj10 =
        new JXDownRect(mForwardGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,35, 360,100);
    assert( obj10 != NULL );

    JXTextRadioButton* obj11 =
        new JXTextRadioButton(1, "Allow Choice of Forward Options", mForwardGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 220,20);
    assert( obj11 != NULL );

    JXTextRadioButton* obj12 =
        new JXTextRadioButton(2, "Forward using Options:", mForwardGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,25, 160,20);
    assert( obj12 != NULL );

    mForwardQuoteOriginal =
        new JXTextCheckbox("Quote Original", obj10,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 115,20);
    assert( mForwardQuoteOriginal != NULL );

    mForwardHeaders =
        new JXTextCheckbox("Include Headers in Quote", obj10,
                    JXWidget::kHElastic, JXWidget::kVElastic, 30,30, 175,20);
    assert( mForwardHeaders != NULL );

    mForwardAttachment =
        new JXTextCheckbox("Always Forward as Attachment", obj10,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,55, 205,20);
    assert( mForwardAttachment != NULL );

    mForwardRFC822 =
        new JXTextCheckbox("Use Embedded Message", obj10,
                    JXWidget::kHElastic, JXWidget::kVElastic, 30,75, 175,20);
    assert( mForwardRFC822 != NULL );

// end JXLayout1

	// Initial state
	mWarnMessageSize->Deactivate();

	// Listen to certain items
	ListenTo(mPrintSetBtn);
	ListenTo(mSizeLimitGroup);
	ListenTo(mForwardGroup);
	ListenTo(mForwardQuoteOriginal);
	ListenTo(mForwardAttachment);
}

void CPrefsMessageGeneral::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		if (sender == mSizeLimitGroup)
		{
			OnSizeLimitGroup(index == 1);
			return;
		}
		else if (sender == mForwardGroup)
		{
			OnForwardGroup(index == 1);
			return;
		}
	}
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mPrintSetBtn)
		{
			OnPrintSetBtn();
			return;
		}
	}
	else if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mForwardQuoteOriginal)
		{
			if (mForwardQuoteOriginal->IsChecked())
				mForwardHeaders->Activate();
			else
				mForwardHeaders->Deactivate();
		}
		else if (sender == mForwardAttachment)
		{
			if (mForwardAttachment->IsChecked())
				mForwardRFC822->Activate();
			else
				mForwardRFC822->Deactivate();
		}
	}
	
	CTabPanel::Receive(sender, message);
}

// Set prefs
void CPrefsMessageGeneral::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Save ref to prefs
	mCopyPrefs = copyPrefs;

	// Set values
	mSaveExtension->SetText(copyPrefs->mSaveCreator.GetValue());
	mSizeLimitGroup->SelectItem(copyPrefs->mDoSizeWarn.GetValue() ? 2 : 1);
	mWarnMessageSize->SetText(cdstring(copyPrefs->warnMessageSize.GetValue()));
	mReplyChoiceGroup->SelectItem(copyPrefs->optionKeyReplyDialog.GetValue() ? 1 : 2);

	mForwardGroup->SelectItem(copyPrefs->mForwardChoice.GetValue() ? 1 : 2);
	OnForwardGroup(copyPrefs->mForwardChoice.GetValue());

	mForwardQuoteOriginal->SetState(JBoolean(copyPrefs->mForwardQuoteOriginal.GetValue()));
	mForwardHeaders->SetState(JBoolean(copyPrefs->mForwardHeaders.GetValue()));
	if (!copyPrefs->mForwardQuoteOriginal.GetValue())
		mForwardHeaders->Deactivate();
	mForwardAttachment->SetState(JBoolean(copyPrefs->mForwardAttachment.GetValue()));
	mForwardRFC822->SetState(JBoolean(copyPrefs->mForwardRFC822.GetValue()));
	if (!copyPrefs->mForwardAttachment.GetValue())
		mForwardRFC822->Deactivate();
}

// Force update of prefs
bool CPrefsMessageGeneral::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	cdstring extension = mSaveExtension->GetText();
	if (extension[0UL] == '.')
		copyPrefs->mSaveCreator.SetValue(((const char*) extension) + 1);
	else
		copyPrefs->mSaveCreator.SetValue(extension);
	copyPrefs->mDoSizeWarn.SetValue(mSizeLimitGroup->GetSelectedItem() == 2);
	cdstring warning = mWarnMessageSize->GetText().GetCString();
	copyPrefs->warnMessageSize.SetValue(::atoi(warning));
	copyPrefs->optionKeyReplyDialog.SetValue(mReplyChoiceGroup->GetSelectedItem() == 1);

	copyPrefs->mForwardChoice.SetValue(mForwardGroup->GetSelectedItem() == 1);

	copyPrefs->mForwardQuoteOriginal.SetValue(mForwardQuoteOriginal->IsChecked());
	copyPrefs->mForwardHeaders.SetValue(mForwardHeaders->IsChecked());
	copyPrefs->mForwardAttachment.SetValue(mForwardAttachment->IsChecked());
	copyPrefs->mForwardRFC822.SetValue(mForwardRFC822->IsChecked());
	
	return true;
}

void CPrefsMessageGeneral::OnSizeLimitGroup(bool no_limit)
{
	if (no_limit)
		mWarnMessageSize->Deactivate();
	else
		mWarnMessageSize->Activate();
}

void CPrefsMessageGeneral::OnPrintSetBtn()
{
	CPrefsEditCaption::PoseDialog(mCopyPrefs,
									&mCopyPrefs->mHeaderCaption,
									&mCopyPrefs->mLtrHeaderCaption,
									&mCopyPrefs->mFooterCaption,
									&mCopyPrefs->mLtrFooterCaption,
									NULL,
									&mCopyPrefs->mHeaderBox,
									&mCopyPrefs->mFooterBox,
									&mCopyPrefs->mPrintSummary);
}

void CPrefsMessageGeneral::OnForwardGroup(bool choice)
{
	if (choice)
	{
		mForwardQuoteOriginal->Deactivate();
		mForwardHeaders->Deactivate();
		mForwardAttachment->Deactivate();
		mForwardRFC822->Deactivate();
	}
	else
	{
		mForwardQuoteOriginal->Activate();
		if (mForwardQuoteOriginal->IsChecked())
			mForwardHeaders->Activate();
		mForwardAttachment->Activate();
		if (mForwardAttachment->IsChecked())
			mForwardRFC822->Activate();
	}
}
