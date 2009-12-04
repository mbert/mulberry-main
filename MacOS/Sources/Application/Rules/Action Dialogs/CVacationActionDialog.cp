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

#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CPrefsEditHeadFoot.h"
#include "CStringUtils.h"
#include "CTextDisplay.h"
#include "CTextFieldX.h"

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CVacationActionDialog::CVacationActionDialog()
{
}

// Constructor from stream
CVacationActionDialog::CVacationActionDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CVacationActionDialog::~CVacationActionDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CVacationActionDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	mDays = (CTextFieldX*) FindPaneByID(paneid_VacationActionDays);
	mSubject = (CTextFieldX*) FindPaneByID(paneid_VacationActionSubject);
	mAddresses = (CTextDisplay*) FindPaneByID(paneid_VacationActionAddresses);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CVacationActionDialogBtns);
}

// Handle OK button
void CVacationActionDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_VacationActionText:
		SetText();
		break;

	default:
		LDialogBox::ListenToMessage(inMessage, ioParam);
		break;
	}
}

// Set the details
void CVacationActionDialog::SetDetails(CActionItem::CActionVacation& details)
{
	mDays->SetNumber(details.GetDays());

	mSubject->SetText(details.GetSubject());

	mText = details.GetText();

	cdstring addrs;
	for(cdstrvect::const_iterator iter = details.GetAddresses().begin(); iter != details.GetAddresses().end(); iter++)
	{
		addrs += *iter;
		addrs += os_endl;
	}
	mAddresses->SetText(addrs);
}

// Get the details
void CVacationActionDialog::GetDetails(CActionItem::CActionVacation& details)
{
	details.SetDays(mDays->GetNumber());

	details.SetSubject(mSubject->GetText());

	details.SetText(mText);

	// Copy handle to text with null terminator
	cdstring text;
	mAddresses->GetText(text);

	char* s = ::strtok(text.c_str_mod(), CR);
	cdstrvect accumulate;
	while(s)
	{
		cdstring copyStr(s);
		accumulate.push_back(copyStr);

		s = ::strtok(NULL, CR);
	}
	details.SetAddresses(accumulate);
}

// Called during idle
void CVacationActionDialog::SetText()
{
	CPrefsEditHeadFoot::PoseDialog("Set Vacation Text", mText, false,
									CPreferences::sPrefs->spaces_per_tab.GetValue(),
									CPreferences::sPrefs->wrap_length.GetValue());
}

bool CVacationActionDialog::PoseDialog(CActionItem::CActionVacation& details)
{
	bool result = false;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_VacationActionDialog, CMulberryApp::sApp);
	((CVacationActionDialog*) theHandler.GetDialog())->SetDetails(details);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			((CVacationActionDialog*) theHandler.GetDialog())->GetDetails(details);
			result = true;
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}

	return result;
}
