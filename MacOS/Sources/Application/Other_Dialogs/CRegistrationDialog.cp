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

#include "CBalloonDialog.h"
#include "CErrorHandler.h"
#include "CHelpAttach.h"
#include "CMulberryApp.h"
#include "CRegistration.h"
#include "CStringUtils.h"
#include "CTextFieldX.h"

#include <LPushButton.h>

#include <stdio.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CRegistrationDialog::CRegistrationDialog()
{
}

// Constructor from stream
CRegistrationDialog::CRegistrationDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CRegistrationDialog::~CRegistrationDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CRegistrationDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get buttons
	mOKBtn = (LPushButton*) FindPaneByID(paneid_RegOKBtn);
	mOKBtn->Disable();
	mDemoBtn = (LPushButton*) FindPaneByID(paneid_DemoBtn);

	// Do licensee name
	mLicenseeName = (CTextFieldX*) FindPaneByID(paneid_RegLicenseeName);
	mLicenseeName->AddListener(this);

	// Do serial number
	mSerialNumber = (CTextFieldX*) FindPaneByID(paneid_RegSerialNumber);
	mSerialNumber->AddListener(this);

	// Do server/domain
	mOrganisation = (CTextFieldX*) FindPaneByID(paneid_RegOrganisation);
	mOrganisation->AddListener(this);

	// Do registration key
	mRegKey1 = (CTextFieldX*) FindPaneByID(paneid_RegKey1);
	if (mRegKey1)
		mRegKey1->AddListener(this);

	mRegKey2 = (CTextFieldX*) FindPaneByID(paneid_RegKey2);
	if (mRegKey2)
		mRegKey2->AddListener(this);

	mRegKey3 = (CTextFieldX*) FindPaneByID(paneid_RegKey3);
	if (mRegKey3)
		mRegKey3->AddListener(this);

	mRegKey4 = (CTextFieldX*) FindPaneByID(paneid_RegKey4);
	if (mRegKey4)
		mRegKey4->AddListener(this);
}

void CRegistrationDialog::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage) {

		case paneid_RegLicenseeName:
		case paneid_RegSerialNumber:
		case paneid_RegKey1:
		case paneid_RegKey2:
		case paneid_RegKey3:
		case paneid_RegKey4:
			{
				// Now check that there's text in required fields
				cdstring temp = mLicenseeName->GetText();
				bool valid_name = !temp.empty();

				temp = mSerialNumber->GetText();
				bool valid_serial = CRegistration::sRegistration.LegalSerial(temp);
				
				bool valid_reg = true;

				temp = mRegKey1->GetText();
				valid_reg &= (temp.length() == 4);

				temp = mRegKey2->GetText();
				valid_reg &= (temp.length() == 4);

				temp = mRegKey3->GetText();
				valid_reg &= (temp.length() == 4);

				temp = mRegKey4->GetText();
				valid_reg &= (temp.length() == 4);

				if (valid_name && valid_serial && valid_reg)
				{
					if (!mOKBtn->IsEnabled())
					{
						mOKBtn->Enable();
					}
				}
				else
				{
					if (mOKBtn->IsEnabled())
					{
						mOKBtn->Disable();
					}
				}
				
				// Now check for auto-tab in reg fields
				switch(inMessage)
				{
				case paneid_RegKey1:
					temp = mRegKey1->GetText();
					if (temp.length() == 4)
						SwitchTarget(mRegKey2);
					break;
				case paneid_RegKey2:
					temp = mRegKey2->GetText();
					if (temp.length() == 4)
						SwitchTarget(mRegKey3);
					break;
				case paneid_RegKey3:
					temp = mRegKey3->GetText();
					if (temp.length() == 4)
						SwitchTarget(mRegKey4);
					break;
				}
			}
			break;

		default:
			// Pass up
			LDialogBox::ListenToMessage(inMessage, ioParam);
	}

}

// Try to register
bool CRegistrationDialog::DoRegistration(bool initial)
{
	// Do multi-user prefs dialog
	bool okayed = false;

	// Create the dialog
	CBalloonDialog theHandler(paneid_RegistrationV3Dialog, CMulberryApp::sApp);

	CRegistrationDialog* dlog = (CRegistrationDialog*) theHandler.GetDialog();
	theHandler.StartDialog();
	if (!initial)
		dlog->mDemoBtn->Hide();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			// Get details from fields
			cdstring licensee = dlog->mLicenseeName->GetText();

			cdstring organisation = dlog->mOrganisation->GetText();

			cdstring serial = dlog->mSerialNumber->GetText();

			cdstring regkey;
			regkey = dlog->mRegKey1->GetText();
			regkey += dlog->mRegKey2->GetText();
			regkey += dlog->mRegKey3->GetText();
			regkey += dlog->mRegKey4->GetText();
			
			// Must be uppercase
			::strupper(regkey.c_str_mod());

			// Must have valid registration input first
			if (CRegistration::sRegistration.ValidInfoCurrent(serial, regkey))
			{
				// Remove existing resources
				CRegistration::sRegistration.ClearAppInfo(0);
				CRegistration::sRegistration.SetLicensee(licensee);
				CRegistration::sRegistration.SetOrganisation(organisation);
				CRegistration::sRegistration.SetSerialNumber(serial);
				CRegistration::sRegistration.SetAppUserCode(regkey);
				if (CRegistration::sRegistration.WriteAppInfo(0))
				{
					// Mark to run as not demo
					CMulberryApp::sApp->RunDemo(false);
					return true;
				}
			}
		}
		else if (hitMessage == msg_Cancel)
		{
			return false;
		}
		else if (hitMessage == msg_RunDemo)
		{
			// Look for date resource
			time_t timeout = CRegistration::sRegistration.LoadAppTimeout(0);

			// Check for existing time
			if (timeout)
			{
				// Get time difference
				double diff = difftime(time(nil), timeout);

				// Check that it can run
				if (diff < 31L*24L*60L*60L)
				{
					// Put up alert warning of time left
					cdstring txt;
					txt.FromResource("Alerts::General::Timeleft");
					txt.Substitute((long)(31L*24L*60L*60L - diff)/(24L*60L*60L));
					CErrorHandler::PutNoteAlert(txt);

					// Mark to run as demo
					CMulberryApp::sApp->RunDemo(true);

					// Set registration details
					CRegistration::sRegistration.SetLicensee("Unlicensed - for evaluation only");
					CRegistration::sRegistration.SetOrganisation(cdstring::null_str);
					CRegistration::sRegistration.SetSerialNumber("Evaluation");

					return true;
				}
				else
				{
					// Time expired
					CErrorHandler::PutNoteAlertRsrc("Alerts::General::NoMoreTime");
					
					// Recyle the reg dialog but disable demo
					dlog->mDemoBtn->Hide();
				}
			}
			else
			{
				// Check for licence agreement
				CBalloonDialog theHandler2(paneid_DemoLicenceDialog, CMulberryApp::sApp);
				theHandler2.StartDialog();

				// Let DialogHandler process events
				while (true)
				{
					MessageT hitMessage = theHandler2.DoDialog();

					if (hitMessage == msg_OK)
					{
						// Mark to run as demo
						CMulberryApp::sApp->RunDemo(true);
						okayed = true;

						// Set new time in resource
						CRegistration::sRegistration.WriteAppTimeout(0);

						// Set registration details
						CRegistration::sRegistration.SetLicensee("Unlicensed - for evaluation only");
						CRegistration::sRegistration.SetOrganisation(cdstring::null_str);
						CRegistration::sRegistration.SetSerialNumber("Evaluation");
						
						return true;
					}
					else if (hitMessage == msg_Cancel)
					{
						break;
					}
				}
			}
		}
	}


	return okayed;
}
