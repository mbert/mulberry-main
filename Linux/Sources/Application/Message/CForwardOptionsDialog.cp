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


// Source for CForwardOptionsDialog class

#include "CForwardOptionsDialog.h"

#include "CMulberryApp.h"
#include "CPreferences.h"

#include <JXEngravedRect.h>
#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>
#include "CInputField.h"

#include <jXGlobals.h>

#include <cassert>

EForwardOptions CForwardOptionsDialog::sForward = eForwardQuote;
bool CForwardOptionsDialog::sForwardInit = false;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CForwardOptionsDialog::CForwardOptionsDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
	InitOptions();
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CForwardOptionsDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 380,290, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 380,290);
    assert( obj1 != NULL );

    JXEngravedRect* obj2 =
        new JXEngravedRect(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,20, 360,115);
    assert( obj2 != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Forward Options:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,7, 105,20);
    assert( obj3 != NULL );

    mForwardQuote =
        new JXTextCheckbox("Quote Original", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,10, 135,20);
    assert( mForwardQuote != NULL );

    mForwardHeaders =
        new JXTextCheckbox("Include Headers in Quote", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,30, 180,20);
    assert( mForwardHeaders != NULL );

    mForwardAttachment =
        new JXTextCheckbox("Forward as Attachment", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,60, 160,20);
    assert( mForwardAttachment != NULL );

    mForwardRFC822 =
        new JXTextCheckbox("Use Embedded Message", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,80, 180,20);
    assert( mForwardRFC822 != NULL );

    mQuote =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,165, 360,40);
    assert( mQuote != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Quote:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,150, 45,20);
    assert( obj4 != NULL );

    mQuoteSelection =
        new JXTextRadioButton(eQuoteSelection, "Selection", mQuote,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 95,20);
    assert( mQuoteSelection != NULL );

    mQuoteAll =
        new JXTextRadioButton(eQuoteAll, "Entire Message", mQuote,
                    JXWidget::kHElastic, JXWidget::kVElastic, 115,10, 120,20);
    assert( mQuoteAll != NULL );

    mQuoteNone =
        new JXTextRadioButton(eQuoteNone, "None", mQuote,
                    JXWidget::kHElastic, JXWidget::kVElastic, 255,10, 95,20);
    assert( mQuoteNone != NULL );

    mSave =
        new JXTextCheckbox("Save Options and Skip Dialog in Future", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,220, 255,20);
    assert( mSave != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 200,255, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mOKBtn =
        new JXTextButton("Forward", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 290,255, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

// end JXLayout

	window->SetTitle("Forward Options");
	SetButtons(mOKBtn, mCancelBtn);

	ListenTo(mForwardQuote);
	ListenTo(mForwardAttachment);
}

// Handle buttons
void CForwardOptionsDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mForwardQuote)
		{
			if (mForwardQuote->IsChecked())
				mForwardHeaders->Activate();
			else
				mForwardHeaders->Deactivate();
			return;
		}
		else if (sender == mForwardAttachment)
		{
			if (mForwardAttachment->IsChecked())
				mForwardRFC822->Activate();
			else
				mForwardRFC822->Deactivate();
			return;
		}
	}

	CDialogDirector::Receive(sender, message);
}

void CForwardOptionsDialog::InitOptions()
{
	if (!sForwardInit)
	{
		sForward = static_cast<EForwardOptions>(0);
		if (CPreferences::sPrefs->mForwardQuoteOriginal.GetValue())
			sForward = static_cast<EForwardOptions>(sForward | eForwardQuote);
		if (CPreferences::sPrefs->mForwardHeaders.GetValue())
			sForward = static_cast<EForwardOptions>(sForward | eForwardHeaders);
		if (CPreferences::sPrefs->mForwardAttachment.GetValue())
			sForward = static_cast<EForwardOptions>(sForward | eForwardAttachment);
		if (CPreferences::sPrefs->mForwardRFC822.GetValue())
			sForward = static_cast<EForwardOptions>(sForward | eForwardRFC822);

		sForwardInit = true;
	}
}

// Set the dialogs info
void CForwardOptionsDialog::SetDetails(EReplyQuote quote, bool has_selection)
{
	mForwardQuote->SetState(JBoolean((sForward & eForwardQuote) ? 1 : 0));
	mForwardHeaders->SetState(JBoolean((sForward & eForwardHeaders) ? 1 : 0));
	if (!(sForward & eForwardQuote))
		mForwardHeaders->Deactivate();
	mForwardAttachment->SetState(JBoolean((sForward & eForwardAttachment) ? 1 : 0));
	mForwardRFC822->SetState(JBoolean((sForward & eForwardRFC822) ? 1 : 0));
	if (!(sForward & eForwardAttachment))
		mForwardRFC822->Deactivate();

	// Set it up
	if (quote == eQuoteSelection)
		mQuote->SelectItem(has_selection ? eQuoteSelection : eQuoteAll);
	else
		mQuote->SelectItem(quote);

	// Disable selection option if none available
	if (!has_selection)
		mQuoteSelection->Deactivate();

	mSave->SetState(kFalse);
}

// Set the dialogs info
void CForwardOptionsDialog::GetDetails(EReplyQuote& quote, bool& save)
{
	sForward = static_cast<EForwardOptions>(0);
	if (mForwardQuote->IsChecked())
		sForward = static_cast<EForwardOptions>(sForward | eForwardQuote);
	if (mForwardHeaders->IsChecked())
		sForward = static_cast<EForwardOptions>(sForward | eForwardHeaders);
	if (mForwardAttachment->IsChecked())
		sForward = static_cast<EForwardOptions>(sForward | eForwardAttachment);
	if (mForwardRFC822->IsChecked())
		sForward = static_cast<EForwardOptions>(sForward | eForwardRFC822);

	// Get the proper setting
	quote = (EReplyQuote) mQuote->GetSelectedItem();

	save = mSave->IsChecked();
}

bool CForwardOptionsDialog::PoseDialog(EForwardOptions& forward, EReplyQuote& quote, bool has_selection)
{
	bool result = false;

	CForwardOptionsDialog* dlog = new CForwardOptionsDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetDetails(quote, has_selection);

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		bool save;
		dlog->GetDetails(quote, save);
		forward = sForward;

		// Set preferences if required
		if (save)
		{
			CPreferences::sPrefs->mForwardChoice.SetValue(false);
			CPreferences::sPrefs->mForwardQuoteOriginal.SetValue(sForward & eForwardQuote);
			CPreferences::sPrefs->mForwardHeaders.SetValue(sForward & eForwardHeaders);
			CPreferences::sPrefs->mForwardAttachment.SetValue(sForward & eForwardAttachment);
			CPreferences::sPrefs->mForwardRFC822.SetValue(sForward & eForwardRFC822);
		}
		result = true;
		dlog->Close();
	}

	return result;
}
