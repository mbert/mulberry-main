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


// Source for CPrefsSpeech class

#include "CPrefsSpeech.h"

#include "CBalloonDialog.h"
#include "CPrefsConfigMessageSpeech.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>



// __________________________________________________________________________________________________
// C L A S S __ C P R E F S M A I L B O X
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsSpeech::CPrefsSpeech()
{
}

// Constructor from stream
CPrefsSpeech::CPrefsSpeech(LStream *inStream)
		: CPrefsPanel(inStream)
{
}

// Default destructor
CPrefsSpeech::~CPrefsSpeech()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsSpeech::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsPanel::FinishCreateSelf();

	// Get items
	mSpeechNewMessage = (LCheckBox*) FindPaneByID(paneid_SpeechNewMessage);
	mSpeechOnArrival = (LCheckBox*) FindPaneByID(paneid_SpeechOnArrival);
	mSpeechMaxChars = (CTextFieldX*) FindPaneByID(paneid_SpeechMaxChars);
	mSpeechEmptyText = (CTextFieldX*) FindPaneByID(paneid_SpeechEmptyText);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsSpeechBtns);
}

// Handle buttons
void CPrefsSpeech::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage)
	{
	case msg_ConfigureMessageSpeech:
		ConfigureMessage();
		break;

	case msg_ConfigureLetterSpeech:
		ConfigureLetter();
		break;
	}
}

// Toggle display of IC items
void CPrefsSpeech::ToggleICDisplay(void)
{
}

// Set prefs
void CPrefsSpeech::SetPrefs(CPreferences* copyPrefs)
{
	// Save ref to prefs
	mCopyPrefs = copyPrefs;
	CPreferences* local_prefs = copyPrefs;

	// Set controls
	mSpeechNewMessage->SetValue(local_prefs->mSpeakNewOpen.GetValue());
	mSpeechOnArrival->SetValue(local_prefs->mSpeakNewArrivals.GetValue());

	// Copy text to edit fields
	mSpeechMaxChars->SetNumber(local_prefs->mSpeakMessageMaxLength.GetValue());

	mSpeechEmptyText->SetText(local_prefs->mSpeakMessageEmptyItem.GetValue());
}

// Force update of prefs
void CPrefsSpeech::UpdatePrefs(void)
{
	CPreferences* local_prefs = mCopyPrefs;

	local_prefs->mSpeakNewOpen.SetValue((mSpeechNewMessage->GetValue()==1));
	local_prefs->mSpeakNewArrivals.SetValue((mSpeechOnArrival->GetValue()==1));

	// Copy info from panel into prefs
	local_prefs->mSpeakMessageMaxLength.SetValue(mSpeechMaxChars->GetNumber());

	local_prefs->mSpeakMessageEmptyItem.SetValue(mSpeechEmptyText->GetText());
}

// Configure message speech
void CPrefsSpeech::ConfigureMessage(void)
{
	CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetSuperView();
	while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
		prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();

	{
		bool canceled = false;

		// Create the dialog
		CBalloonDialog	theHandler(paneid_PrefsConfigMessageSpeech, prefs_dlog);
		CPrefsConfigMessageSpeech* config_speech = (CPrefsConfigMessageSpeech*) theHandler.GetDialog();

		// Make a copy of the list
		CMessageSpeakVector copy_list = mCopyPrefs->mSpeakMessageItems.GetValue();
		config_speech->SetList(&copy_list);
		theHandler.StartDialog();

		// Let DialogHandler process events
		while (true)
		{
			MessageT hitMessage = theHandler.DoDialog();

			if (hitMessage == msg_OK)
			{
				// Copy new list
				mCopyPrefs->mSpeakMessageItems.SetValue(copy_list);
				break;
			}
			else if (hitMessage == msg_Cancel)
			{
				break;
			}
		}

		prefs_dlog->SwitchTarget(prefs_dlog);
	}
}

// Configure letter speech
void CPrefsSpeech::ConfigureLetter(void)
{
	CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetSuperView();
	while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
		prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();

	{
		bool canceled = false;

		// Create the dialog
		CBalloonDialog	theHandler(paneid_PrefsConfigMessageSpeech, prefs_dlog);
		CPrefsConfigMessageSpeech* config_speech = (CPrefsConfigMessageSpeech*) theHandler.GetDialog();

		// Make a copy of the list
		CMessageSpeakVector copy_list = mCopyPrefs->mSpeakLetterItems.GetValue();
		config_speech->SetList(&copy_list);
		theHandler.StartDialog();

		// Let DialogHandler process events
		while (true)
		{
			MessageT hitMessage = theHandler.DoDialog();

			if (hitMessage == msg_OK)
			{
				// Copy new list
				mCopyPrefs->mSpeakLetterItems.SetValue(copy_list);
				break;
			}
			else if (hitMessage == msg_Cancel)
			{
				break;
			}
		}

		prefs_dlog->SwitchTarget(prefs_dlog);
	}
}
