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


// Source for CPrefsDisplayQuotes class

#include "CPrefsDisplayQuotes.h"

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
CPrefsDisplayQuotes::CPrefsDisplayQuotes(JXContainer* enclosure,
								const HSizingOption hSizing, 
								const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) 
	: CPrefsDisplayPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsDisplayQuotes::OnCreate()
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
        new JXStaticText(">Level:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,33, 67,20);
    assert( obj5 != NULL );

    mQuotation1Colour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,30, 40,20);
    assert( mQuotation1Colour != NULL );

    mQuotationBold =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,30, 20,20);
    assert( mQuotationBold != NULL );

    mQuotationItalic =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,30, 20,20);
    assert( mQuotationItalic != NULL );

    mQuotationUnderline =
        new JXTextCheckbox("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,30, 20,20);
    assert( mQuotationUnderline != NULL );

    JXStaticText* obj6 =
        new JXStaticText(">>Level:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,58, 77,20);
    assert( obj6 != NULL );

    mQuotation2Colour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,55, 40,20);
    assert( mQuotation2Colour != NULL );

    JXStaticText* obj7 =
        new JXStaticText(">>>Level:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,83, 77,20);
    assert( obj7 != NULL );

    mQuotation3Colour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,80, 40,20);
    assert( mQuotation3Colour != NULL );

    JXStaticText* obj8 =
        new JXStaticText(">>>>Level:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,108, 77,20);
    assert( obj8 != NULL );

    mQuotation4Colour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,105, 40,20);
    assert( mQuotation4Colour != NULL );

    JXStaticText* obj9 =
        new JXStaticText(">>>>>Level:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,133, 77,20);
    assert( obj9 != NULL );

    mQuotation5Colour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,130, 40,20);
    assert( mQuotation5Colour != NULL );

    mUseQuotationBtn =
        new JXTextCheckbox("Use Multiple Colours", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,90, 150,20);
    assert( mUseQuotationBtn != NULL );

    JXUpRect* obj10 =
        new JXUpRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,165, 175,25);
    assert( obj10 != NULL );

    JXStaticText* obj11 =
        new JXStaticText("Recognised Quotes", obj10,
                    JXWidget::kHElastic, JXWidget::kVElastic, 25,2, 120,20);
    assert( obj11 != NULL );

    mQuotes =
        new CTextInputDisplay(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,190, 175,105);
    assert( mQuotes != NULL );

// end JXLayout1

	// Get controls
	GetControls(mQuotation, mQuotation1Colour, mQuotationBold, mQuotationItalic, mQuotationUnderline);

	mQuotation1Colour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mQuotation2Colour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mQuotation3Colour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mQuotation4Colour->SetDirector(CPreferencesDialog::sPrefsDlog);
	mQuotation5Colour->SetDirector(CPreferencesDialog::sPrefsDlog);

	// Start listening
	ListenTo(mUseQuotationBtn);
}

// Handle buttons
void CPrefsDisplayQuotes::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mUseQuotationBtn)
		{
			OnUseQuotation();
			return;
		}
	}
	
	CPrefsDisplayPanel::Receive(sender, message);
}

// Set prefs
void CPrefsDisplayQuotes::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info
	mQuotation1Colour->SetColor(copyPrefs->mQuotationStyle.GetValue().color);
	mQuotationBold->SetState(JBoolean(copyPrefs->mQuotationStyle.GetValue().style & bold));
	mQuotationItalic->SetState(JBoolean(copyPrefs->mQuotationStyle.GetValue().style & italic));
	mQuotationUnderline->SetState(JBoolean(copyPrefs->mQuotationStyle.GetValue().style & underline));

	mQuotation2Colour->SetColor(copyPrefs->mQuoteColours.GetValue().at(0));
	mQuotation3Colour->SetColor(copyPrefs->mQuoteColours.GetValue().at(1));
	mQuotation4Colour->SetColor(copyPrefs->mQuoteColours.GetValue().at(2));
	mQuotation5Colour->SetColor(copyPrefs->mQuoteColours.GetValue().at(3));

	mUseQuotationBtn->SetState(JBoolean(copyPrefs->mUseMultipleQuotes.GetValue()));
	if (!copyPrefs->mUseMultipleQuotes.GetValue())
	{
		mQuotation2Colour->Deactivate();
		mQuotation3Colour->Deactivate();
		mQuotation4Colour->Deactivate();
		mQuotation5Colour->Deactivate();
	}

	cdstring quotes;
	for(unsigned long i = 0; i < copyPrefs->mRecognizeQuotes.GetValue().size(); i++)
	{
		quotes += copyPrefs->mRecognizeQuotes.GetValue()[i];
		quotes += '\n';
	}
	mQuotes->SetText(quotes);
}

// Force update of prefs
bool CPrefsDisplayQuotes::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	SStyleTraits traits;

	// Copy info from panel into prefs
	traits.color = mQuotation1Colour->GetColor();
	traits.style = normal;
	if (mQuotationBold->IsChecked())
		traits.style |= bold;
	if (mQuotationItalic->IsChecked())
		traits.style |= italic;
	if (mQuotationUnderline->IsChecked())
		traits.style |= underline;
	copyPrefs->mQuotationStyle.SetValue(traits);

	// Check for dirty
	bool dirty_quotes = false;
	if (copyPrefs->mQuoteColours.Value().at(0) != mQuotation2Colour->GetColor())
	{
		copyPrefs->mQuoteColours.Value().at(0) = mQuotation2Colour->GetColor();
		dirty_quotes = true;
	}
	if (copyPrefs->mQuoteColours.Value().at(1) != mQuotation3Colour->GetColor())
	{
		copyPrefs->mQuoteColours.Value().at(1) = mQuotation3Colour->GetColor();
		dirty_quotes = true;
	}
	if (copyPrefs->mQuoteColours.Value().at(2) != mQuotation4Colour->GetColor())
	{
		copyPrefs->mQuoteColours.Value().at(2) = mQuotation4Colour->GetColor();
		dirty_quotes = true;
	}
	if (copyPrefs->mQuoteColours.Value().at(3) != mQuotation5Colour->GetColor())
	{
		copyPrefs->mQuoteColours.Value().at(3) = mQuotation5Colour->GetColor();
		dirty_quotes = true;
	}
	if (dirty_quotes)
		copyPrefs->mQuoteColours.SetDirty();

	copyPrefs->mUseMultipleQuotes.SetValue(mUseQuotationBtn->IsChecked());

	{
		// Tokenise text to get each line
		cdstring txt = mQuotes->GetText();

		char* s = ::strtok(txt.c_str_mod(), "\n");
		cdstrvect accumulate;
		while(s)
		{
			cdstring copyStr(s);
			accumulate.push_back(copyStr);

			s = ::strtok(nil, "\n");
		}
		copyPrefs->mRecognizeQuotes.SetValue(accumulate);
	}
	
	return true;
}

void CPrefsDisplayQuotes::OnUseQuotation()
{
	if (mUseQuotationBtn->IsChecked())
	{
		mQuotation2Colour->Activate();
		mQuotation3Colour->Activate();
		mQuotation4Colour->Activate();
		mQuotation5Colour->Activate();
	}
	else
	{
		mQuotation2Colour->Deactivate();
		mQuotation3Colour->Deactivate();
		mQuotation4Colour->Deactivate();
		mQuotation5Colour->Deactivate();
	}
}
