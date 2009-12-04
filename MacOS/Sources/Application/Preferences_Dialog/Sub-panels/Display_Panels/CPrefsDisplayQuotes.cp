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
#include "CStringUtils.h"
#include "CTextDisplay.h"

#include <LCheckBox.h>
#include <LGAColorSwatchControl.h>


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsDisplayQuotes::CPrefsDisplayQuotes()
{
}

// Constructor from stream
CPrefsDisplayQuotes::CPrefsDisplayQuotes(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsDisplayQuotes::~CPrefsDisplayQuotes()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsDisplayQuotes::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mQuotation1Color = (LGAColorSwatchControl*) FindPaneByID(paneid_DQQuotation1Color);
	mQuotationBold = (LCheckBox*) FindPaneByID(paneid_DQQuotationBold);
	mQuotationItalic = (LCheckBox*) FindPaneByID(paneid_DQQuotationItalic);
	mQuotationUnderline = (LCheckBox*) FindPaneByID(paneid_DQQuotationUnderline);

	mQuotation2Color = (LGAColorSwatchControl*) FindPaneByID(paneid_DQQuotation2Color);
	mQuotation3Color = (LGAColorSwatchControl*) FindPaneByID(paneid_DQQuotation3Color);
	mQuotation4Color = (LGAColorSwatchControl*) FindPaneByID(paneid_DQQuotation4Color);
	mQuotation5Color = (LGAColorSwatchControl*) FindPaneByID(paneid_DQQuotation5Color);

	mUseQuotation = (LCheckBox*) FindPaneByID(paneid_DQUseQuotation);

	mQuotes = (CTextDisplay*) FindPaneByID(paneid_DQQuotes);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CPrefsDisplayQuotesBtns);
}

// Handle buttons
void CPrefsDisplayQuotes::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_DQUseQuotation:
		if (*(long*) ioParam)
		{
			mQuotation2Color->Enable();
			mQuotation3Color->Enable();
			mQuotation4Color->Enable();
			mQuotation5Color->Enable();
		}
		else
		{
			mQuotation2Color->Disable();
			mQuotation3Color->Disable();
			mQuotation4Color->Disable();
			mQuotation5Color->Disable();
		}
		break;
	}
}

// Set prefs
void CPrefsDisplayQuotes::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info
	mQuotation1Color->SetSwatchColor((RGBColor&) copyPrefs->mQuotationStyle.GetValue().color);
	mQuotationBold->SetValue((copyPrefs->mQuotationStyle.GetValue().style & bold) != 0);
	mQuotationItalic->SetValue((copyPrefs->mQuotationStyle.GetValue().style & italic) != 0);
	mQuotationUnderline->SetValue((copyPrefs->mQuotationStyle.GetValue().style & underline) != 0);

	mQuotation2Color->SetSwatchColor(const_cast<RGBColor&>(copyPrefs->mQuoteColours.GetValue().at(0)));
	mQuotation3Color->SetSwatchColor(const_cast<RGBColor&>(copyPrefs->mQuoteColours.GetValue().at(1)));
	mQuotation4Color->SetSwatchColor(const_cast<RGBColor&>(copyPrefs->mQuoteColours.GetValue().at(2)));
	mQuotation5Color->SetSwatchColor(const_cast<RGBColor&>(copyPrefs->mQuoteColours.GetValue().at(3)));

	mUseQuotation->SetValue(copyPrefs->mUseMultipleQuotes.GetValue());
	if (!copyPrefs->mUseMultipleQuotes.GetValue())
	{
		mQuotation2Color->Disable();
		mQuotation3Color->Disable();
		mQuotation4Color->Disable();
		mQuotation5Color->Disable();
	}

	cdstring quotes;
	for(cdstrvect::const_iterator iter = copyPrefs->mRecognizeQuotes.GetValue().begin(); iter != copyPrefs->mRecognizeQuotes.GetValue().end(); iter++)
	{
		quotes += *iter;
		quotes += os_endl;
	}
	mQuotes->SetText(quotes);
}

bool TestRGB(RGBColor rgb1, RGBColor rgb2);
bool TestRGB(RGBColor rgb1, RGBColor rgb2)
{
	return (rgb1.red == rgb2.red) && (rgb1.green == rgb2.green) && (rgb1.blue == rgb2.blue);
}

// Force update of prefs
void CPrefsDisplayQuotes::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	SStyleTraits traits;

	// Copy info from panel into prefs
	mQuotation1Color->GetSwatchColor(traits.color);
	traits.style = normal;
	if (mQuotationBold->GetValue())
		traits.style |= bold;
	if (mQuotationItalic->GetValue())
		traits.style |= italic;
	if (mQuotationUnderline->GetValue())
		traits.style |= underline;
	copyPrefs->mQuotationStyle.SetValue(traits);

	bool dirty_quotes = false;
	RGBColor test;
	mQuotation2Color->GetSwatchColor(test);
	if (!TestRGB(copyPrefs->mQuoteColours.Value().at(0), test))
	{
		copyPrefs->mQuoteColours.Value().at(0) = test;
		dirty_quotes = true;
	}
	mQuotation3Color->GetSwatchColor(test);
	if (!TestRGB(copyPrefs->mQuoteColours.Value().at(1), test))
	{
		copyPrefs->mQuoteColours.Value().at(1) = test;
		dirty_quotes = true;
	}
	mQuotation4Color->GetSwatchColor(test);
	if (!TestRGB(copyPrefs->mQuoteColours.Value().at(2), test))
	{
		copyPrefs->mQuoteColours.Value().at(2) = test;
		dirty_quotes = true;
	}
	mQuotation5Color->GetSwatchColor(test);
	if (!TestRGB(copyPrefs->mQuoteColours.Value().at(3), test))
	{
		copyPrefs->mQuoteColours.Value().at(3) = test;
		dirty_quotes = true;
	}
	copyPrefs->mQuoteColours.SetDirty();

	copyPrefs->mUseMultipleQuotes.SetValue(mUseQuotation->GetValue());

	// Only copy text if dirty
	if (mQuotes->IsDirty())
	{
		// Copy handle to text with null terminator
		cdstring txt;
		mQuotes->GetText(txt);

		char* s = ::strtok(txt.c_str_mod(), CR);
		cdstrvect accumulate;
		while(s)
		{
			cdstring copyStr(s);
			accumulate.push_back(copyStr);

			s = ::strtok(nil, CR);
		}
		copyPrefs->mRecognizeQuotes.SetValue(accumulate);
	}
}
