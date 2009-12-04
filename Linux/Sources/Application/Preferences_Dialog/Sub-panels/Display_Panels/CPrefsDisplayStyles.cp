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


// Source for CPrefsDisplayStyles class

#include "CPrefsDisplayStyles.h"

#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CTextDisplay.h"

#include "JXColorButton.h"

#include <JXEngravedRect.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>
#include <JXUpRect.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsDisplayStyles::CPrefsDisplayStyles(JXContainer* enclosure,
								const HSizingOption hSizing, 
								const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) 
	: CPrefsDisplayPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsDisplayStyles::OnCreate()
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
        new JXStaticText("Under", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 226,10, 45,20);
    assert( obj4 != NULL );

    JXStaticText* obj5 =
        new JXStaticText("URL:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,33, 67,20);
    assert( obj5 != NULL );

    mURLColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,30, 40,20);
    assert( mURLColour != NULL );

    mURLBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,30, 20,20);
    assert( mURLBold != NULL );

    mURLItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,30, 20,20);
    assert( mURLItalic != NULL );

    mURLUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,30, 20,20);
    assert( mURLUnderline != NULL );

    JXStaticText* obj6 =
        new JXStaticText("URL Seen:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,58, 77,20);
    assert( obj6 != NULL );

    mURLSeenColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,55, 40,20);
    assert( mURLSeenColour != NULL );

    mURLSeenBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,55, 20,20);
    assert( mURLSeenBold != NULL );

    mURLSeenItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,55, 20,20);
    assert( mURLSeenItalic != NULL );

    mURLSeenUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,55, 20,20);
    assert( mURLSeenUnderline != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Header:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,83, 77,20);
    assert( obj7 != NULL );

    mHeaderColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,80, 40,20);
    assert( mHeaderColour != NULL );

    mHeaderBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,80, 20,20);
    assert( mHeaderBold != NULL );

    mHeaderItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,80, 20,20);
    assert( mHeaderItalic != NULL );

    mHeaderUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,80, 20,20);
    assert( mHeaderUnderline != NULL );

    JXStaticText* obj8 =
        new JXStaticText("Styled Tags:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,108, 77,20);
    assert( obj8 != NULL );

    mTagColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,105, 40,20);
    assert( mTagColour != NULL );

    mTagBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,105, 20,20);
    assert( mTagBold != NULL );

    mTagItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,105, 20,20);
    assert( mTagItalic != NULL );

    mTagUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,105, 20,20);
    assert( mTagUnderline != NULL );

    JXStaticText* obj9 =
        new JXStaticText("Selections:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,133, 77,20);
    assert( obj9 != NULL );

    mSelectionColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,130, 40,20);
    assert( mSelectionColour != NULL );

    JXUpRect* obj10 =
        new JXUpRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,165, 175,25);
    assert( obj10 != NULL );

    JXStaticText* obj11 =
        new JXStaticText("Recognise URL Schemes", obj10,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,2, 150,20);
    assert( obj11 != NULL );

    mURLs =
        new CTextInputDisplay(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,190, 175,105);
    assert( mURLs != NULL );

// end JXLayout1

	// Get controls
	GetControls(mURL, mURLColour, mURLBold, mURLItalic, mURLUnderline);
	GetControls(mURLSeen, mURLSeenColour, mURLSeenBold, mURLSeenItalic, mURLSeenUnderline);
	GetControls(mHeader, mHeaderColour, mHeaderBold, mHeaderItalic, mHeaderUnderline);
	GetControls(mTag, mTagColour, mTagBold, mTagItalic, mTagUnderline);

	mURLColour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mURLSeenColour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mHeaderColour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mTagColour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mSelectionColour->SetDirector(CPreferencesDialog::sPrefsDlog);
}

// Set prefs
void CPrefsDisplayStyles::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info
	SetStyle(mURL, copyPrefs->mURLStyle.GetValue());
	SetStyle(mURLSeen, copyPrefs->mURLSeenStyle.GetValue());
	SetStyle(mHeader, copyPrefs->mHeaderStyle.GetValue());
	SetStyle(mTag, copyPrefs->mTagStyle.GetValue());
	mSelectionColour->SetColor(copyPrefs->mSelectionColour.GetValue());

	cdstring urls;
	for(unsigned long i = 0; i < copyPrefs->mRecognizeURLs.GetValue().size(); i++)
	{
		urls += copyPrefs->mRecognizeURLs.GetValue()[i];
		urls += '\n';
	}
	mURLs->SetText(urls);
}

// Force update of prefs
bool CPrefsDisplayStyles::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	SStyleTraits temp;
	copyPrefs->mURLStyle.SetValue(GetStyle(mURL, temp));
	copyPrefs->mURLSeenStyle.SetValue(GetStyle(mURLSeen, temp));
	copyPrefs->mHeaderStyle.SetValue(GetStyle(mHeader, temp));
	copyPrefs->mTagStyle.SetValue(GetStyle(mTag, temp));
	copyPrefs->mSelectionColour.SetValue(mSelectionColour->GetColor());

	// Only copy text if dirty
	{
		// Tokenise text to get each line
		cdstring txt = mURLs->GetText();

		char* s = ::strtok(txt.c_str_mod(), "\n");
		cdstrvect accumulate;
		while(s)
		{
			cdstring copyStr(s);
			accumulate.push_back(copyStr);

			s = ::strtok(nil, "\n");
		}
		copyPrefs->mRecognizeURLs.SetValue(accumulate);
	}
	
	return true;
}
