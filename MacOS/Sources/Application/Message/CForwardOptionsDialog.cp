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

#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CPreferences.h"

#include <LCheckBox.h>
#include <LRadioButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C O P E N M A I L B O X D I A L O G
// __________________________________________________________________________________________________

EForwardOptions CForwardOptionsDialog::sForward = eForwardQuote;
bool CForwardOptionsDialog::sForwardInit = false;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CForwardOptionsDialog::CForwardOptionsDialog()
{
	InitOptions();
}

// Constructor from stream
CForwardOptionsDialog::CForwardOptionsDialog(LStream *inStream)
		: LDialogBox(inStream)
{
	InitOptions();
}

// Default destructor
CForwardOptionsDialog::~CForwardOptionsDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CForwardOptionsDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	mForwardQuote = (LCheckBox*) FindPaneByID(paneid_ForwardOptionsQuote);
	mForwardHeaders = (LCheckBox*) FindPaneByID(paneid_ForwardOptionsHeaders);
	mForwardAttachment = (LCheckBox*) FindPaneByID(paneid_ForwardOptionsAttachment);
	mForwardRFC822 = (LCheckBox*) FindPaneByID(paneid_ForwardOptionsRFC822);
	mQuoteSelection = (LRadioButton*) FindPaneByID(paneid_ForwardOptionsQuoteSelection);
	mQuoteAll = (LRadioButton*) FindPaneByID(paneid_ForwardOptionsQuoteAll);
	mQuoteNone = (LRadioButton*) FindPaneByID(paneid_ForwardOptionsQuoteNone);
	mSave = (LCheckBox*) FindPaneByID(paneid_ForwardOptionsSave);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CForwardOptionsDialogBtns);
}

// Handle OK button
void CForwardOptionsDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_ForwardOptionsQuote:
		// Disable item
		if (*(long*)ioParam)
			mForwardHeaders->Enable();
		else
			mForwardHeaders->Disable();
		break;

	case msg_ForwardOptionsAttachment:
		// Disable item
		if (*(long*)ioParam)
			mForwardRFC822->Enable();
		else
			mForwardRFC822->Disable();
		break;

	default:
		LDialogBox::ListenToMessage(inMessage, ioParam);
		break;
	}
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
	mForwardQuote->SetValue((sForward & eForwardQuote) ? 1 : 0);
	mForwardHeaders->SetValue((sForward & eForwardHeaders) ? 1 : 0);
	if (!(sForward & eForwardQuote))
		mForwardHeaders->Disable();
	mForwardAttachment->SetValue((sForward & eForwardAttachment) ? 1 : 0);
	mForwardRFC822->SetValue((sForward & eForwardRFC822) ? 1 : 0);
	if (!(sForward & eForwardAttachment))
		mForwardRFC822->Disable();

	// Disable selection option if none available
	if (!has_selection)
		mQuoteSelection->Disable();

	// Set it up
	switch(quote)
	{
	case eQuoteSelection:
		(has_selection ? mQuoteSelection : mQuoteAll)->SetValue(1);
		break;
	case eQuoteAll:
		mQuoteAll->SetValue(1);
		break;
	case eQuoteNone:
		mQuoteNone->SetValue(1);
		break;
	}

	mSave->SetValue(0);
}

// Set the dialogs info
void CForwardOptionsDialog::GetDetails(EReplyQuote& quote, bool& save)
{
	sForward = static_cast<EForwardOptions>(0);
	if (mForwardQuote->GetValue())
		sForward = static_cast<EForwardOptions>(sForward | eForwardQuote);
	if (mForwardHeaders->GetValue())
		sForward = static_cast<EForwardOptions>(sForward | eForwardHeaders);
	if (mForwardAttachment->GetValue())
		sForward = static_cast<EForwardOptions>(sForward | eForwardAttachment);
	if (mForwardRFC822->GetValue())
		sForward = static_cast<EForwardOptions>(sForward | eForwardRFC822);

	if (mQuoteSelection->GetValue())
		quote = eQuoteSelection;
	else if (mQuoteAll->GetValue())
		quote = eQuoteAll;
	else if (mQuoteNone->GetValue())
		quote = eQuoteNone;

	save = mSave->GetValue();
}

bool CForwardOptionsDialog::PoseDialog(EForwardOptions& forward, EReplyQuote& quote, bool has_selection)
{
	bool result = false;

	CBalloonDialog dlog(paneid_ForwardOptionsDialog, CMulberryApp::sApp);
	CForwardOptionsDialog* fdlog = static_cast<CForwardOptionsDialog*>(dlog.GetDialog());
	fdlog->SetDetails(quote, has_selection);
	dlog.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = dlog.DoDialog();

		if (hitMessage == msg_OK)
		{
			bool save;
			fdlog->GetDetails(quote, save);
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
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}

	return result;
}
