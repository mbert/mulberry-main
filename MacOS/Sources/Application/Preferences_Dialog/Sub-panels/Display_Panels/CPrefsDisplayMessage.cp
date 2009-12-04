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


// Source for CPrefsDisplayMessage class

#include "CPrefsDisplayMessage.h"

#include "CPreferences.h"

#include <LCheckBox.h>
#include <LGAColorSwatchControl.h>


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsDisplayMessage::CPrefsDisplayMessage()
{
}

// Constructor from stream
CPrefsDisplayMessage::CPrefsDisplayMessage(LStream *inStream)
		: CPrefsDisplayPanel(inStream)
{
}

// Default destructor
CPrefsDisplayMessage::~CPrefsDisplayMessage()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsDisplayMessage::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsDisplayPanel::FinishCreateSelf();

	// Get controls
	GetControls(mUnseen, paneid_DMUnseenColour, paneid_DMUnseenBold, paneid_DMUnseenItalic, paneid_DMUnseenStrike, paneid_DMUnseenUnderline);
	GetControls(mSeen, paneid_DMSeenColour, paneid_DMSeenBold, paneid_DMSeenItalic, paneid_DMSeenStrike, paneid_DMSeenUnderline);
	GetControls(mAnswered, paneid_DMAnsweredColour, paneid_DMAnsweredBold, paneid_DMAnsweredItalic, paneid_DMAnsweredStrike, paneid_DMAnsweredUnderline);
	GetControls(mImportant, paneid_DMImportantColour, paneid_DMImportantBold, paneid_DMImportantItalic, paneid_DMImportantStrike, paneid_DMImportantUnderline);
	GetControls(mDeleted, paneid_DMDeletedColour, paneid_DMDeletedBold, paneid_DMDeletedItalic, paneid_DMDeletedStrike, paneid_DMDeletedUnderline);

	mMultiAddressBold = (LCheckBox*) FindPaneByID(paneid_DMMultiAddressBold);
	mMultiAddressItalic = (LCheckBox*) FindPaneByID(paneid_DMMultiAddressItalic);
	mMultiAddressStrike = (LCheckBox*) FindPaneByID(paneid_DMMultiAddressStrike);
	mMultiAddressUnderline = (LCheckBox*) FindPaneByID(paneid_DMMultiAddressUnderline);

	mMatchColour = (LGAColorSwatchControl*) FindPaneByID(paneid_DMMatchColour);
	mUseMatch = (LCheckBox*) FindPaneByID(paneid_DMUseMatch);
	mNonMatchColour = (LGAColorSwatchControl*) FindPaneByID(paneid_DMNonMatchColour);
	mUseNonMatch = (LCheckBox*) FindPaneByID(paneid_DMUseNonMatch);

	mUseLocalTimezone = (LCheckBox*) FindPaneByID(paneid_DMUseLocalTimezone);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CPrefsDisplayMessageBtns);
}

// Handle buttons
void CPrefsDisplayMessage::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_DMUseMatch:
		if (*(long*) ioParam)
			mMatchColour->Enable();
		else
			mMatchColour->Disable();
		break;
	case msg_DMUseNonMatch:
		if (*(long*) ioParam)
			mNonMatchColour->Enable();
		else
			mNonMatchColour->Disable();
		break;
	}
}

// Set prefs
void CPrefsDisplayMessage::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info
	SetStyle(mUnseen, copyPrefs->unseen.GetValue());
	SetStyle(mSeen, copyPrefs->seen.GetValue());
	SetStyle(mAnswered, copyPrefs->answered.GetValue());
	SetStyle(mImportant, copyPrefs->important.GetValue());
	SetStyle(mDeleted, copyPrefs->deleted.GetValue());

	mMultiAddressBold->SetValue((copyPrefs->mMultiAddress.GetValue() & bold) != 0);
	mMultiAddressItalic->SetValue((copyPrefs->mMultiAddress.GetValue() & italic) != 0);
	mMultiAddressStrike->SetValue((copyPrefs->mMultiAddress.GetValue() & strike_through) != 0);
	mMultiAddressUnderline->SetValue((copyPrefs->mMultiAddress.GetValue() & underline) != 0);

	mMatchColour->SetSwatchColor((RGBColor&) copyPrefs->mMatch.GetValue().color);
	mUseMatch->SetValue(copyPrefs->mMatch.GetValue().style);
	if (!copyPrefs->mMatch.GetValue().style)
		mMatchColour->Disable();
	mNonMatchColour->SetSwatchColor((RGBColor&) copyPrefs->mNonMatch.GetValue().color);
	mUseNonMatch->SetValue(copyPrefs->mNonMatch.GetValue().style);
	if (!copyPrefs->mNonMatch.GetValue().style)
		mNonMatchColour->Disable();

	mUseLocalTimezone->SetValue(copyPrefs->mUseLocalTimezone.GetValue());
}

// Force update of prefs
void CPrefsDisplayMessage::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	SStyleTraits traits;

	// Copy info from panel into prefs
	copyPrefs->unseen.SetValue(GetStyle(mUnseen, traits));
	copyPrefs->seen.SetValue(GetStyle(mSeen, traits));
	copyPrefs->answered.SetValue(GetStyle(mAnswered, traits));
	copyPrefs->important.SetValue(GetStyle(mImportant, traits));
	copyPrefs->deleted.SetValue(GetStyle(mDeleted, traits));

	short style = normal;
	if (mMultiAddressBold->GetValue())
		style |= bold;
	if (mMultiAddressItalic->GetValue())
		style |= italic;
	if (mMultiAddressStrike->GetValue())
		style |= strike_through;
	if (mMultiAddressUnderline->GetValue())
		style |= underline;
	copyPrefs->mMultiAddress.SetValue(style);

	mMatchColour->GetSwatchColor(traits.color);
	traits.style = mUseMatch->GetValue();
	copyPrefs->mMatch.SetValue(traits);
	mNonMatchColour->GetSwatchColor(traits.color);
	traits.style = mUseNonMatch->GetValue();
	copyPrefs->mNonMatch.SetValue(traits);

	copyPrefs->mUseLocalTimezone.SetValue(mUseLocalTimezone->GetValue());
}
