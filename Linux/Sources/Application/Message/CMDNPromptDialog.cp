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


// Source for CMDNPromptDialog class

#include "CMDNPromptDialog.h"

#include "CPreferences.h"

#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXUpRect.h>
#include <JXWindow.h>
#include "CInputField.h"

#include <jXGlobals.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMDNPromptDialog::CMDNPromptDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

// Default destructor
CMDNPromptDialog::~CMDNPromptDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CMDNPromptDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 340,150, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 340,150);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Send Read Receipt to:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,7, 135,20);
    assert( obj2 != NULL );

    mAddress =
        new JXStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,30, 315,40);
    assert( mAddress != NULL );

    mSave =
        new JXTextCheckbox("Remember Choice and Skip Dialog in Future", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,80, 285,20);
    assert( mSave != NULL );

    mCancelBtn =
        new JXTextButton("No Receipt", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 100,115, 100,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mOKBtn =
        new JXTextButton("Send Receipt", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 220,115, 100,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

// end JXLayout

	window->SetTitle("Read Receipt Request");
	SetButtons(mOKBtn, NULL);

	ListenTo(mCancelBtn);

}

// Set the dialogs info
void CMDNPromptDialog::SetDetails(const cdstring& addr)
{
	mAddress->SetText(addr);
	mSave->SetState(kFalse);
}

// Set the dialogs info
void CMDNPromptDialog::GetDetails(bool& save)
{
	save = mSave->IsChecked();
}

void CMDNPromptDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mCancelBtn)
		{
			OnNoReceiptBtn();
			return;
		}
	}

	// Pass up
	CDialogDirector::Receive(sender, message);
}

void CMDNPromptDialog::OnNoReceiptBtn()
{
	EndDialog(kDialogClosed_Btn3);
}

bool CMDNPromptDialog::PoseDialog(const cdstring& addr)
{
	bool result = false;

	CMDNPromptDialog* dlog = new CMDNPromptDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetDetails(addr);

	// Let DialogHandler process events
	switch (dlog->DoModal(false))
	{
	case kDialogClosed_OK:
	{
		bool save;
		dlog->GetDetails(save);

		// Set preferences if required
		if (save)
			CPreferences::sPrefs->mMDNOptions.SetValue(eMDNAlwaysSend);

		result = true;
		dlog->Close();
		break;
	}
	case kDialogClosed_Btn3:
	{
		bool save;
		dlog->GetDetails(save);

		// Set preferences if required
		if (save)
			CPreferences::sPrefs->mMDNOptions.SetValue(eMDNNeverSend);

		result = false;
		dlog->Close();
		break;
	}
	default:;
	}

	return result;
}
