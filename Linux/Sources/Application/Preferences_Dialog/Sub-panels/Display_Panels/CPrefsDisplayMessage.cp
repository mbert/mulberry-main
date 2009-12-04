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
#include "CPreferencesDialog.h"

#include "JXColorButton.h"

#include <JXEngravedRect.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsDisplayMessage::CPrefsDisplayMessage(JXContainer* enclosure,
								const HSizingOption hSizing, 
								const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) 
	: CPrefsDisplayPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsDisplayMessage::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Colour:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,10, 50,20);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Bold", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 150,10, 35,20);
    assert( obj2 != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Italic", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 190,10, 35,20);
    assert( obj3 != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Strike", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 227,10, 40,20);
    assert( obj4 != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Under", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 266,10, 45,20);
    assert( obj5 != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Unseen:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,30, 67,20);
    assert( obj6 != NULL );

    mUnseenColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,30, 40,20);
    assert( mUnseenColour != NULL );

    mUnseenBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,30, 20,20);
    assert( mUnseenBold != NULL );

    mUnseenItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,30, 20,20);
    assert( mUnseenItalic != NULL );

    mUnseenStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,30, 20,20);
    assert( mUnseenStrike != NULL );

    mUnseenUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 275,30, 20,20);
    assert( mUnseenUnderline != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Seen:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,55, 77,20);
    assert( obj7 != NULL );

    mSeenColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,55, 40,20);
    assert( mSeenColour != NULL );

    mSeenBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,55, 20,20);
    assert( mSeenBold != NULL );

    mSeenItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,55, 20,20);
    assert( mSeenItalic != NULL );

    mSeenStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,55, 20,20);
    assert( mSeenStrike != NULL );

    mSeenUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 275,55, 20,20);
    assert( mSeenUnderline != NULL );

    JXStaticText* obj8 =
        new JXStaticText("Answered:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,80, 77,20);
    assert( obj8 != NULL );

    mAnsweredColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,80, 40,20);
    assert( mAnsweredColour != NULL );

    mAnsweredBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,80, 20,20);
    assert( mAnsweredBold != NULL );

    mAnsweredItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,80, 20,20);
    assert( mAnsweredItalic != NULL );

    mAnsweredStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,80, 20,20);
    assert( mAnsweredStrike != NULL );

    mAnsweredUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 275,80, 20,20);
    assert( mAnsweredUnderline != NULL );

    JXStaticText* obj9 =
        new JXStaticText("Important:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,105, 77,20);
    assert( obj9 != NULL );

    mImportantColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,105, 40,20);
    assert( mImportantColour != NULL );

    mImportantBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,105, 20,20);
    assert( mImportantBold != NULL );

    mImportantItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,105, 20,20);
    assert( mImportantItalic != NULL );

    mImportantStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,105, 20,20);
    assert( mImportantStrike != NULL );

    mImportantUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 275,105, 20,20);
    assert( mImportantUnderline != NULL );

    JXStaticText* obj10 =
        new JXStaticText("Deleted:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,130, 77,20);
    assert( obj10 != NULL );

    mDeletedColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,130, 40,20);
    assert( mDeletedColour != NULL );

    mDeletedBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,130, 20,20);
    assert( mDeletedBold != NULL );

    mDeletedItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,130, 20,20);
    assert( mDeletedItalic != NULL );

    mDeletedStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,130, 20,20);
    assert( mDeletedStrike != NULL );

    mDeletedUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 275,130, 20,20);
    assert( mDeletedUnderline != NULL );

    JXStaticText* obj11 =
        new JXStaticText("Multiple Address:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,155, 107,20);
    assert( obj11 != NULL );

    mMultiAddressBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,155, 20,20);
    assert( mMultiAddressBold != NULL );

    mMultiAddressItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,155, 20,20);
    assert( mMultiAddressItalic != NULL );

    mMultiAddressStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,155, 20,20);
    assert( mMultiAddressStrike != NULL );

    mMultiAddressUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 275,155, 20,20);
    assert( mMultiAddressUnderline != NULL );

    JXEngravedRect* obj12 =
        new JXEngravedRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,185, 360,3);
    assert( obj12 != NULL );

    JXStaticText* obj13 =
        new JXStaticText("Matched:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,195, 67,20);
    assert( obj13 != NULL );

    mMatchColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,195, 40,20);
    assert( mMatchColour != NULL );

    mUseMatchBtn =
        new JXTextCheckbox("Colour Background", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,195, 140,20);
    assert( mUseMatchBtn != NULL );

    JXStaticText* obj14 =
        new JXStaticText("Unmatched:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,220, 72,20);
    assert( obj14 != NULL );

    mNonMatchColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,220, 40,20);
    assert( mNonMatchColour != NULL );

    mUseNonMatchBtn =
        new JXTextCheckbox("Colour Background", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,220, 140,20);
    assert( mUseNonMatchBtn != NULL );

    JXEngravedRect* obj15 =
        new JXEngravedRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,250, 360,3);
    assert( obj15 != NULL );

    mUseLocalTimezone =
        new JXTextCheckbox("Display Dates Adjusted to Local Timezone", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,260, 270,20);
    assert( mUseLocalTimezone != NULL );

// end JXLayout1

	// Get controls
	GetControls(mUnseen, mUnseenColour, mUnseenBold, mUnseenItalic, mUnseenStrike, mUnseenUnderline);
	GetControls(mSeen, mSeenColour, mSeenBold, mSeenItalic, mSeenStrike, mSeenUnderline);
	GetControls(mAnswered, mAnsweredColour, mAnsweredBold, mAnsweredItalic, mAnsweredStrike, mAnsweredUnderline);
	GetControls(mImportant, mImportantColour, mImportantBold, mImportantItalic, mImportantStrike, mImportantUnderline);
	GetControls(mDeleted, mDeletedColour, mDeletedBold, mDeletedItalic, mDeletedStrike, mDeletedUnderline);

	mUnseenColour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mSeenColour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mAnsweredColour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mImportantColour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mDeletedColour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mMatchColour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mNonMatchColour->SetDirector(CPreferencesDialog::sPrefsDlog);

	// Start listening
	ListenTo(mUseMatchBtn);
	ListenTo(mUseNonMatchBtn);
}

// Handle buttons
void CPrefsDisplayMessage::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mUseMatchBtn)
		{
			OnUseMatch();
			return;
		}
		else if (sender == mUseNonMatchBtn)
		{
			OnUseNonMatch();
			return;
		}
	}
	
	CPrefsDisplayPanel::Receive(sender, message);
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

	mMultiAddressBold->SetState(JBoolean(copyPrefs->mMultiAddress.GetValue() & bold));
	mMultiAddressItalic->SetState(JBoolean(copyPrefs->mMultiAddress.GetValue() & italic));
	mMultiAddressStrike->SetState(JBoolean(copyPrefs->mMultiAddress.GetValue() & strike_through));
	mMultiAddressUnderline->SetState(JBoolean(copyPrefs->mMultiAddress.GetValue() & underline));

	mMatchColour->SetColor(copyPrefs->mMatch.GetValue().color);
	mUseMatchBtn->SetState(JBoolean(copyPrefs->mMatch.GetValue().style));
	if (!copyPrefs->mMatch.GetValue().style)
		mMatchColour->Deactivate();
	mNonMatchColour->SetColor(copyPrefs->mNonMatch.GetValue().color);
	mUseNonMatchBtn->SetState(JBoolean(copyPrefs->mNonMatch.GetValue().style));
	if (!copyPrefs->mNonMatch.GetValue().style)
		mNonMatchColour->Deactivate();

	mUseLocalTimezone->SetState(JBoolean(copyPrefs->mUseLocalTimezone.GetValue()));
}

// Force update of prefs
bool CPrefsDisplayMessage::UpdateData(void* data)
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
	if (mMultiAddressBold->IsChecked())
		style |= bold;
	if (mMultiAddressItalic->IsChecked())
		style |= italic;
	if (mMultiAddressStrike->IsChecked())
		style |= strike_through;
	if (mMultiAddressUnderline->IsChecked())
		style |= underline;
	copyPrefs->mMultiAddress.SetValue(style);

	traits.color = mMatchColour->GetColor();
	traits.style = mUseMatchBtn->IsChecked();
	copyPrefs->mMatch.SetValue(traits);
	traits.color = mNonMatchColour->GetColor();
	traits.style = mUseNonMatchBtn->IsChecked();
	copyPrefs->mNonMatch.SetValue(traits);

	copyPrefs->mUseLocalTimezone.SetValue(mUseLocalTimezone->IsChecked());
	
	return true;
}

void CPrefsDisplayMessage::OnUseMatch()
{
	if (mUseMatchBtn->IsChecked())
		mMatchColour->Activate();
	else
		mMatchColour->Deactivate();
}

void CPrefsDisplayMessage::OnUseNonMatch()
{
	if (mUseNonMatchBtn->IsChecked())
		mNonMatchColour->Activate();
	else
		mNonMatchColour->Deactivate();
}
