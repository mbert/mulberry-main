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


// Source for CPrefsDisplayLabel class

#include "CPrefsDisplayLabel.h"

#include "CIMAPLabelsDialog.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CTextField.h"

#include "JXColorButton.h"

#include <JXEngravedRect.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsDisplayLabel::CPrefsDisplayLabel(JXContainer* enclosure,
								const HSizingOption hSizing, 
								const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) 
	: CPrefsDisplayPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsDisplayLabel::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Label:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,10, 75,20);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Use Colour:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 98,10, 75,20);
    assert( obj2 != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Use Bkgnd:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 168,10, 70,20);
    assert( obj3 != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Bold", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 236,10, 35,20);
    assert( obj4 != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Italic", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 271,10, 35,20);
    assert( obj5 != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Strike", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 303,10, 40,20);
    assert( obj6 != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Under", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 339,10, 41,20);
    assert( obj7 != NULL );

    mLabels[0].mName =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,35, 100,20);
    assert( mLabels[0].mName != NULL );

    mLabels[0].mUseColor =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 108,35, 20,20);
    assert( mLabels[0].mUseColor != NULL );

    mLabels[0].mColor =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 128,35, 40,20);
    assert( mLabels[0].mColor != NULL );

    mLabels[0].mUseBkgColor =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 173,35, 20,20);
    assert( mLabels[0].mUseBkgColor != NULL );

    mLabels[0].mBkgColor =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 193,35, 40,20);
    assert( mLabels[0].mBkgColor != NULL );

    mLabels[0].mBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 241,35, 20,20);
    assert( mLabels[0].mBold != NULL );

    mLabels[0].mItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 276,35, 20,20);
    assert( mLabels[0].mItalic != NULL );

    mLabels[0].mStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 311,35, 20,20);
    assert( mLabels[0].mStrike != NULL );

    mLabels[0].mUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 348,35, 20,20);
    assert( mLabels[0].mUnderline != NULL );

    mLabels[1].mName =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,65, 100,20);
    assert( mLabels[1].mName != NULL );

    mLabels[1].mUseColor =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 108,65, 20,20);
    assert( mLabels[1].mUseColor != NULL );

    mLabels[1].mColor =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 128,65, 40,20);
    assert( mLabels[1].mColor != NULL );

    mLabels[1].mUseBkgColor =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 173,65, 20,20);
    assert( mLabels[1].mUseBkgColor != NULL );

    mLabels[1].mBkgColor =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 193,65, 40,20);
    assert( mLabels[1].mBkgColor != NULL );

    mLabels[1].mBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 241,65, 20,20);
    assert( mLabels[1].mBold != NULL );

    mLabels[1].mItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 276,65, 20,20);
    assert( mLabels[1].mItalic != NULL );

    mLabels[1].mStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 311,65, 20,20);
    assert( mLabels[1].mStrike != NULL );

    mLabels[1].mUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 348,65, 20,20);
    assert( mLabels[1].mUnderline != NULL );

    mLabels[2].mName =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,95, 100,20);
    assert( mLabels[2].mName != NULL );

    mLabels[2].mUseColor =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 108,95, 20,20);
    assert( mLabels[2].mUseColor != NULL );

    mLabels[2].mColor =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 128,95, 40,20);
    assert( mLabels[2].mColor != NULL );

    mLabels[2].mUseBkgColor =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 173,95, 20,20);
    assert( mLabels[2].mUseBkgColor != NULL );

    mLabels[2].mBkgColor =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 193,95, 40,20);
    assert( mLabels[2].mBkgColor != NULL );

    mLabels[2].mBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 241,95, 20,20);
    assert( mLabels[2].mBold != NULL );

    mLabels[2].mItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 276,95, 20,20);
    assert( mLabels[2].mItalic != NULL );

    mLabels[2].mStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 311,95, 20,20);
    assert( mLabels[2].mStrike != NULL );

    mLabels[2].mUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 348,95, 20,20);
    assert( mLabels[2].mUnderline != NULL );

    mLabels[3].mName =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,125, 100,20);
    assert( mLabels[3].mName != NULL );

    mLabels[3].mUseColor =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 108,125, 20,20);
    assert( mLabels[3].mUseColor != NULL );

    mLabels[3].mColor =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 128,125, 40,20);
    assert( mLabels[3].mColor != NULL );

    mLabels[3].mUseBkgColor =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 173,125, 20,20);
    assert( mLabels[3].mUseBkgColor != NULL );

    mLabels[3].mBkgColor =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 193,125, 40,20);
    assert( mLabels[3].mBkgColor != NULL );

    mLabels[3].mBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 241,125, 20,20);
    assert( mLabels[3].mBold != NULL );

    mLabels[3].mItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 276,125, 20,20);
    assert( mLabels[3].mItalic != NULL );

    mLabels[3].mStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 311,125, 20,20);
    assert( mLabels[3].mStrike != NULL );

    mLabels[3].mUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 348,125, 20,20);
    assert( mLabels[3].mUnderline != NULL );

    mLabels[4].mName =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,155, 100,20);
    assert( mLabels[4].mName != NULL );

    mLabels[4].mUseColor =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 108,155, 20,20);
    assert( mLabels[4].mUseColor != NULL );

    mLabels[4].mColor =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 128,155, 40,20);
    assert( mLabels[4].mColor != NULL );

    mLabels[4].mUseBkgColor =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 173,155, 20,20);
    assert( mLabels[4].mUseBkgColor != NULL );

    mLabels[4].mBkgColor =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 193,155, 40,20);
    assert( mLabels[4].mBkgColor != NULL );

    mLabels[4].mBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 241,155, 20,20);
    assert( mLabels[4].mBold != NULL );

    mLabels[4].mItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 276,155, 20,20);
    assert( mLabels[4].mItalic != NULL );

    mLabels[4].mStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 311,155, 20,20);
    assert( mLabels[4].mStrike != NULL );

    mLabels[4].mUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 348,155, 20,20);
    assert( mLabels[4].mUnderline != NULL );

    mLabels[5].mName =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,185, 100,20);
    assert( mLabels[5].mName != NULL );

    mLabels[5].mUseColor =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 108,185, 20,20);
    assert( mLabels[5].mUseColor != NULL );

    mLabels[5].mColor =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 128,185, 40,20);
    assert( mLabels[5].mColor != NULL );

    mLabels[5].mUseBkgColor =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 173,185, 20,20);
    assert( mLabels[5].mUseBkgColor != NULL );

    mLabels[5].mBkgColor =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 193,185, 40,20);
    assert( mLabels[5].mBkgColor != NULL );

    mLabels[5].mBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 241,185, 20,20);
    assert( mLabels[5].mBold != NULL );

    mLabels[5].mItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 276,185, 20,20);
    assert( mLabels[5].mItalic != NULL );

    mLabels[5].mStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 311,185, 20,20);
    assert( mLabels[5].mStrike != NULL );

    mLabels[5].mUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 348,185, 20,20);
    assert( mLabels[5].mUnderline != NULL );

    mLabels[6].mName =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,215, 100,20);
    assert( mLabels[6].mName != NULL );

    mLabels[6].mUseColor =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 108,215, 20,20);
    assert( mLabels[6].mUseColor != NULL );

    mLabels[6].mColor =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 128,215, 40,20);
    assert( mLabels[6].mColor != NULL );

    mLabels[6].mUseBkgColor =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 173,215, 20,20);
    assert( mLabels[6].mUseBkgColor != NULL );

    mLabels[6].mBkgColor =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 193,215, 40,20);
    assert( mLabels[6].mBkgColor != NULL );

    mLabels[6].mBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 241,215, 20,20);
    assert( mLabels[6].mBold != NULL );

    mLabels[6].mItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 276,215, 20,20);
    assert( mLabels[6].mItalic != NULL );

    mLabels[6].mStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 311,215, 20,20);
    assert( mLabels[6].mStrike != NULL );

    mLabels[6].mUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 348,215, 20,20);
    assert( mLabels[6].mUnderline != NULL );

    mLabels[7].mName =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,245, 100,20);
    assert( mLabels[7].mName != NULL );

    mLabels[7].mUseColor =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 108,245, 20,20);
    assert( mLabels[7].mUseColor != NULL );

    mLabels[7].mColor =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 128,245, 40,20);
    assert( mLabels[7].mColor != NULL );

    mLabels[7].mUseBkgColor =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 173,245, 20,20);
    assert( mLabels[7].mUseBkgColor != NULL );

    mLabels[7].mBkgColor =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 193,245, 40,20);
    assert( mLabels[7].mBkgColor != NULL );

    mLabels[7].mBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 241,245, 20,20);
    assert( mLabels[7].mBold != NULL );

    mLabels[7].mItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 276,245, 20,20);
    assert( mLabels[7].mItalic != NULL );

    mLabels[7].mStrike =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 311,245, 20,20);
    assert( mLabels[7].mStrike != NULL );

    mLabels[7].mUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 348,245, 20,20);
    assert( mLabels[7].mUnderline != NULL );

// end JXLayout1
// begin JXLayout2

    mIMAPLabelsBtn =
        new JXTextButton("Set IMAP Label Names", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,275, 155,25);
    assert( mIMAPLabelsBtn != NULL );

// end JXLayout2

	// Get controls

	for(int i = 0; i < NMessage::eMaxLabels; i++)
	{
		mLabels[i].mColor->SetDirector(CPreferencesDialog::sPrefsDlog);
		mLabels[i].mBkgColor->SetDirector(CPreferencesDialog::sPrefsDlog);

		// Start listening
		ListenTo(mLabels[i].mUseColor);
		ListenTo(mLabels[i].mUseBkgColor);
	}
	
	ListenTo(mIMAPLabelsBtn);
}

// Handle buttons
void CPrefsDisplayLabel::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		// Enable/disable colour buttons
		for(int i = 0; i < NMessage::eMaxLabels; i++)
		{
			if (sender == mLabels[i].mUseColor)
			{
				OnUseColor(i);
				return;
			}
			else if (sender == mLabels[i].mUseBkgColor)
			{
				OnUseBkgnd(i);
				return;
			}
		}
	}
	else if (message.Is(JXTextButton::kPushed))
	{
		if (sender == mIMAPLabelsBtn)
		{
			OnIMAPLabels();
			return;
		}
	}
	
	CPrefsDisplayPanel::Receive(sender, message);
}

// Set prefs
void CPrefsDisplayLabel::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info
	for(int i = 0; i < NMessage::eMaxLabels; i++)
	{
		SetStyle(mLabels[i], *copyPrefs->mLabels.GetValue()[i]);
		mIMAPLabels.push_back(copyPrefs->mIMAPLabels.GetValue()[i]);
	}
}

// Force update of prefs
bool CPrefsDisplayLabel::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	SStyleTraits2 traits;

	// Copy info from panel into prefs
	for(int i = 0; i < NMessage::eMaxLabels; i++)
	{
		*copyPrefs->mLabels.Value()[i] = GetStyle(mLabels[i], traits);
		copyPrefs->mIMAPLabels.Value()[i] = mIMAPLabels[i];
	}

	return true;
}

void CPrefsDisplayLabel::OnUseColor(JIndex nID)
{
	if (mLabels[nID].mUseColor->IsChecked())
		mLabels[nID].mColor->Activate();
	else
		mLabels[nID].mColor->Deactivate();
}

void CPrefsDisplayLabel::OnUseBkgnd(JIndex nID)
{
	if (mLabels[nID].mUseBkgColor->IsChecked())
		mLabels[nID].mBkgColor->Activate();
	else
		mLabels[nID].mBkgColor->Deactivate();
}


// Set IMAP labels
void CPrefsDisplayLabel::OnIMAPLabels()
{
	// Get current names
	cdstrvect names;
	for(int i = 0; i < NMessage::eMaxLabels; i++)
	{
		names.push_back(mLabels[i].mName->GetText());
	}
	
	CIMAPLabelsDialog::PoseDialog(names, mIMAPLabels);
}
