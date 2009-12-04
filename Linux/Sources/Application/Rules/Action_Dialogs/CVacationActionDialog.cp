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


// Source for CVacationActionDialog class

#include "CVacationActionDialog.h"

#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CPrefsEditHeadFoot.h"
#include "CTextDisplay.h"
#include "CTextField.h"

#include <JXDisplay.h>
#include <JXInputField.h>
#include <JXIntegerInput.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>
#include "CInputField.h"

#include <jXGlobals.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CVacationActionDialog::CVacationActionDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

// Default destructor
CVacationActionDialog::~CVacationActionDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CVacationActionDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 390,310, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 390,310);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("On Vacation for:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,17, 105,20);
    assert( obj2 != NULL );

    mDays =
        new CInputField<JXIntegerInput>(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 135,15, 40,20);
    assert( mDays != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Subject:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,52, 55,20);
    assert( obj3 != NULL );

    mSubject =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 75,50, 280,20);
    assert( mSubject != NULL );

    mTextBtn =
        new JXTextButton("Vacation Text...", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,90, 110,25);
    assert( mTextBtn != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 210,275, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 300,275, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    JXStaticText* obj4 =
        new JXStaticText("days", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 180,17, 45,20);
    assert( obj4 != NULL );

    mAddresses =
        new CTextDisplay(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,150, 370,90);
    assert( mAddresses != NULL );

    JXUpRect* obj5 =
        new JXUpRect(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,130, 370,25);
    assert( obj5 != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Additional Addresses to Count as Mine", obj5,
                    JXWidget::kHElastic, JXWidget::kVElastic, 75,5, 225,20);
    assert( obj6 != NULL );

// end JXLayout

	window->SetTitle("Vacation Action Options");
	SetButtons(mOKBtn, mCancelBtn);

	ListenTo(mTextBtn);
}

void CVacationActionDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mTextBtn)
		{
			SetText();
			return;
		}
	}

	CDialogDirector::Receive(sender, message);
}

// Set the details
void CVacationActionDialog::SetDetails(CActionItem::CActionVacation& details)
{
	mDays->SetText(cdstring(details.GetDays()));

	mSubject->SetText(details.GetSubject());

	mText = details.GetText();

	cdstring addrs;
	for(unsigned long i = 0; i < details.GetAddresses().size(); i++)
	{
		addrs += details.GetAddresses()[i];
		addrs += '\n';
	}
	mAddresses->SetText(addrs);
}

// Get the details
void CVacationActionDialog::GetDetails(CActionItem::CActionVacation& details)
{
	details.SetDays(::atoi(mDays->GetText()));

	details.SetSubject(mSubject->GetText());

	details.SetText(mText);

	// Copy handle to text with null terminator
	cdstring text(mAddresses->GetText());

	char* s = ::strtok(text.c_str_mod(), "\n");
	cdstrvect accumulate;
	while(s)
	{
		cdstring copyStr(s);
		accumulate.push_back(copyStr);

		s = ::strtok(NULL, "\n");
	}
	details.SetAddresses(accumulate);
}

// Called during idle
void CVacationActionDialog::SetText()
{
	cdstring title("Set Vacation Text");
	CPrefsEditHeadFoot::PoseDialog(mText, title, CPreferences::sPrefs, false);
}

bool CVacationActionDialog::PoseDialog(CActionItem::CActionVacation& details)
{
	bool result = false;

	// Create the dialog
	CVacationActionDialog* dlog = new CVacationActionDialog(JXGetApplication());
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
