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


// Source for CRegistrationDialog class

#include "CRegistrationDialog.h"

#include "CErrorHandler.h"
#include "CLicenseDialog.h"
#include "CMulberryApp.h"
#include "CRegistration.h"
#include "CStringUtils.h"
#include "CTextField.h"
#include "CXStringResources.h"

#include "cdstring.h"

#include "HResourceMap.h"

#include <JXColormap.h>
#include <JXWindow.h>
#include <JXUpRect.h>
#include <JXEngravedRect.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXImageWidget.h>

#include <jXGlobals.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CRegistrationDialog::CRegistrationDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
	mAllowDemo = true;
	
	// These need to be nulled out as some may not be used
	mRegKey1 = NULL;
	mRegKey2 = NULL;
	mRegKey3 = NULL;
	mRegKey4 = NULL;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CRegistrationDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 430,330, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 430,330);
    assert( obj1 != NULL );

    JXEngravedRect* frame =
        new JXEngravedRect(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,120, 400,150);
    assert( frame != NULL );

    mOKBtn =
        new JXTextButton("Register", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 340,295, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mDemoBtn =
        new JXTextButton("Run as Demo", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 110,295, 100,25);
    assert( mDemoBtn != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 250,295, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mLogo =
        new JXImageWidget(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 260,100);
    assert( mLogo != NULL );

    mLicensee =
        new CTextInputField(frame,
                    JXWidget::kHElastic, JXWidget::kVElastic, 120,20, 269,20);
    assert( mLicensee != NULL );

    mOrganisation =
        new CTextInputField(frame,
                    JXWidget::kHElastic, JXWidget::kVElastic, 120,50, 269,20);
    assert( mOrganisation != NULL );

    mSerialNumber =
        new CTextInputField(frame,
                    JXWidget::kHElastic, JXWidget::kVElastic, 120,80, 269,20);
    assert( mSerialNumber != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Registration", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 25,110, 80,20);
    assert( obj2 != NULL );
    const JFontStyle obj2_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj2->SetFontStyle(obj2_style);

    JXStaticText* obj3 =
        new JXStaticText("Licensee Name:", frame,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,22, 115,20);
    assert( obj3 != NULL );
    const JFontStyle obj3_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj3->SetFontStyle(obj3_style);

    JXStaticText* obj4 =
        new JXStaticText("Organisation:", frame,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,52, 115,20);
    assert( obj4 != NULL );
    const JFontStyle obj4_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj4->SetFontStyle(obj4_style);

    JXStaticText* obj5 =
        new JXStaticText("Serial Number:", frame,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,82, 115,20);
    assert( obj5 != NULL );
    const JFontStyle obj5_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj5->SetFontStyle(obj5_style);

    JXStaticText* obj6 =
        new JXStaticText("Registration Key:", frame,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,112, 115,20);
    assert( obj6 != NULL );
    const JFontStyle obj6_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj6->SetFontStyle(obj6_style);

// end JXLayout
	{
// begin JXLayout2
    JXStaticText* obj1 =
        new JXStaticText("-", frame,
                    JXWidget::kHElastic, JXWidget::kVElastic, 176,112, 15,20);
    assert( obj1 != NULL );
    const JFontStyle obj1_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj1->SetFontStyle(obj1_style);

    JXStaticText* obj2 =
        new JXStaticText("-", frame,
                    JXWidget::kHElastic, JXWidget::kVElastic, 247,112, 15,20);
    assert( obj2 != NULL );
    const JFontStyle obj2_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj2->SetFontStyle(obj2_style);

    JXStaticText* obj3 =
        new JXStaticText("-", frame,
                    JXWidget::kHElastic, JXWidget::kVElastic, 318,112, 15,20);
    assert( obj3 != NULL );
    const JFontStyle obj3_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj3->SetFontStyle(obj3_style);

    mRegKey1 =
        new CTextInputField(frame,
                    JXWidget::kHElastic, JXWidget::kVElastic, 120,110, 56,20);
    assert( mRegKey1 != NULL );

    mRegKey2 =
        new CTextInputField(frame,
                    JXWidget::kHElastic, JXWidget::kVElastic, 191,110, 56,20);
    assert( mRegKey2 != NULL );

    mRegKey3 =
        new CTextInputField(frame,
                    JXWidget::kHElastic, JXWidget::kVElastic, 262,110, 56,20);
    assert( mRegKey3 != NULL );

    mRegKey4 =
        new CTextInputField(frame,
                    JXWidget::kHElastic, JXWidget::kVElastic, 333,110, 56,20);
    assert( mRegKey4 != NULL );

// end JXLayout2
	}

	window->SetTitle("Register Mulberry");
	mLogo->SetImage(bmpFromResource(IDB_LOGO, mLogo), kTrue);
	SetButtons(mOKBtn, mCancelBtn);
	mOKBtn->Deactivate();

	mLicensee->ShouldBroadcastAllTextChanged(kTrue);
	ListenTo(mLicensee);
	mOrganisation->ShouldBroadcastAllTextChanged(kTrue);
	ListenTo(mOrganisation);
	mSerialNumber->ShouldBroadcastAllTextChanged(kTrue);
	ListenTo(mSerialNumber);
	mRegKey1->ShouldBroadcastAllTextChanged(kTrue);
	ListenTo(mRegKey1);
	mRegKey2->ShouldBroadcastAllTextChanged(kTrue);
	ListenTo(mRegKey2);
	mRegKey3->ShouldBroadcastAllTextChanged(kTrue);
	ListenTo(mRegKey3);
	mRegKey4->ShouldBroadcastAllTextChanged(kTrue);
	ListenTo(mRegKey4);

	// Hide registration button if already registered
	if (!mAllowDemo)
		mDemoBtn->Hide();
	else
		ListenTo(mDemoBtn);
}

void CRegistrationDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (((sender == mLicensee) || (sender == mSerialNumber) ||
		 (sender == mRegKey1) || (sender == mRegKey2) || (sender == mRegKey3) || (sender == mRegKey4)) &&
		message.Is(JTextEditor16::kTextChanged))
	{
		cdstring licensee_name(mLicensee->GetText());
		bool valid_name = !licensee_name.empty();
		cdstring serial_number(mSerialNumber->GetText());
		bool valid_serial = CRegistration::sRegistration.LegalSerial(serial_number);

		bool valid_reg = false;
		{
			cdstring reg_key(mRegKey1->GetText());
			valid_reg = (reg_key.length() == 4); 
			reg_key = mRegKey2->GetText();
			valid_reg &= (reg_key.length() == 4); 
			reg_key = mRegKey3->GetText();
			valid_reg &= (reg_key.length() == 4); 
			reg_key = mRegKey4->GetText();
			valid_reg &= (reg_key.length() == 4); 
		}

		if (valid_name && valid_serial && valid_reg)
			mOKBtn->Activate();
		else
			mOKBtn->Deactivate();
		
		// Do auto tab for V3 fields
		if (sender == mRegKey1)
		{
			cdstring reg_key(mRegKey1->GetText());
			if (reg_key.length() == 4)
				mRegKey2->Focus();
		}
		else if (sender == mRegKey2)
		{
			cdstring reg_key(mRegKey2->GetText());
			if (reg_key.length() == 4)
				mRegKey3->Focus();
		}
		else if (sender == mRegKey3)
		{
			cdstring reg_key(mRegKey3->GetText());
			if (reg_key.length() == 4)
				mRegKey4->Focus();
		}
	
		return;
	}
	else if ((sender == mDemoBtn) && message.Is(JXButton::kPushed))
	{
		EndDialog(kDialogClosed_Btn3);
		return;
	}

	// Pass up
	CDialogDirector::Receive(sender, message);
}

// Try to register
bool CRegistrationDialog::DoRegistration(bool initial)
{
	// Do multi-user prefs dialog
	bool okayed = false;

	// Loop until valid data entry
	while(true)
	{
		CRegistrationDialog* dlog = new CRegistrationDialog(JXGetApplication());
		dlog->mAllowDemo = initial;

		// Let Dialog process events
		int result = dlog->DoModal();

		if (result == kDialogClosed_OK)
		{
			// Must have valid registration input first
			cdstring licensee(dlog->mLicensee->GetText());
			cdstring organisation(dlog->mOrganisation->GetText());
			cdstring serial_number(dlog->mSerialNumber->GetText());
			cdstring reg_key;
			reg_key = dlog->mRegKey1->GetText();
			reg_key += dlog->mRegKey2->GetText();
			reg_key += dlog->mRegKey3->GetText();
			reg_key += dlog->mRegKey4->GetText();
			
			// Must be uppercase
			::strupper(reg_key.c_str_mod());

			if (CRegistration::sRegistration.ValidInfoCurrent(serial_number, reg_key))
			{
				// Remove existing resources
				// cdstr
				CRegistration::sRegistration.ClearAppInfo();
				CRegistration::sRegistration.SetLicensee(licensee);
				CRegistration::sRegistration.SetOrganisation(organisation);
				CRegistration::sRegistration.SetSerialNumber(serial_number);
				CRegistration::sRegistration.SetAppUserCode(reg_key);
				
				// Validate - should work unless resource failure
				if (CRegistration::sRegistration.WriteAppInfo())
				{
					// Mark to run as not demo
					CMulberryApp::sApp->RunDemo(false);
					dlog->Close();
					return true;
				}
			}
			dlog->Close();
		}
		else if (result == kDialogClosed_Cancel)
		{
			return false;
		}
		else if (result == kDialogClosed_Btn3)
		{
			// Look for date resource
			time_t timeout = CRegistration::sRegistration.GetAppDemoTimeout();

			// Check for existing time
			if (timeout)
			{
				// Get time difference
				double diff = ::difftime(::time(nil), timeout);

				// Check that it can run
				if (diff < 31L*24L*60L*60L)
				{
					// Put up alert warning of time left
					cdstring txt(rsrc::GetString("Alerts::General::Timeleft"));
					txt.Substitute((long) (31L*24L*60L*60L - diff)/(24L*60L*60L));
					CErrorHandler::PutNoteAlert(txt);

					// Mark to run as demo
					CMulberryApp::sApp->RunDemo(true);

					// Set registration details
					CRegistration::sRegistration.SetLicensee("Unlicensed - for evaluation only");
					CRegistration::sRegistration.SetOrganisation(cdstring::null_str);
					CRegistration::sRegistration.SetSerialNumber("Evaluation");
					
					dlog->Close();
					return true;
				}
				else
				{
					// Time expired
					CErrorHandler::PutNoteAlertRsrc("Alerts::General::NoDemoTime");
					
					// Recyle the reg dialog but disable demo
					dlog->mAllowDemo = false;
				}
				dlog->Close();
			}
			// Try to set new time in resource
			else
			{
				CLicenseDialog* dlog1 = new CLicenseDialog(JXGetApplication());
				int result1 = dlog1->DoModal();
				if (result1 == kDialogClosed_OK)
				{
					// Mark to run as demo
					CMulberryApp::sApp->RunDemo(true);
					CRegistration::sRegistration.SetAppDemoTimeout(::time(NULL));

					// Validate - should work unless resource failure
					if (CRegistration::sRegistration.WriteAppDemoInfo())
					{
						// Set registration details
						CRegistration::sRegistration.SetLicensee("Unlicensed - for evaluation only");
						CRegistration::sRegistration.SetOrganisation(cdstring::null_str);
						CRegistration::sRegistration.SetSerialNumber("Evaluation");
						dlog1->Close();
						dlog->Close();
						return true;
					}
					dlog1->Close();
				}
				dlog->Close();
			}
		}
	}
	
	return okayed;
}
