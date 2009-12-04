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


// Source for CPrefsDisplayPanel class

#include "CPrefsDisplayPanel.h"

#include "CPreferences.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>
#include <LGAColorSwatchControl.h>


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsDisplayPanel::CPrefsDisplayPanel()
{
}

// Constructor from stream
CPrefsDisplayPanel::CPrefsDisplayPanel(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsDisplayPanel::~CPrefsDisplayPanel()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CPrefsDisplayPanel::GetControls(SStyleItems& items, PaneIDT color_c, PaneIDT bold_c, PaneIDT italic_c, PaneIDT underline_c)
{
	items.mColor = (LGAColorSwatchControl*) FindPaneByID(color_c);
	items.mBold = (LCheckBox*) FindPaneByID(bold_c);
	items.mItalic = (LCheckBox*) FindPaneByID(italic_c);
	items.mUnderline = (LCheckBox*) FindPaneByID(underline_c);
}

void CPrefsDisplayPanel::SetStyle(SStyleItems& items, const SStyleTraits& traits)
{
	items.mColor->SetSwatchColor((RGBColor&) traits.color);
	items.mBold->SetValue((traits.style & bold) != 0);
	items.mItalic->SetValue((traits.style & italic) != 0);
	items.mUnderline->SetValue((traits.style & underline) != 0);
}

SStyleTraits& CPrefsDisplayPanel::GetStyle(SStyleItems& items, SStyleTraits& traits)
{
	items.mColor->GetSwatchColor(traits.color);
	traits.style = normal;
	if (items.mBold->GetValue())
		traits.style |= bold;
	if (items.mItalic->GetValue())
		traits.style |= italic;
	if (items.mUnderline->GetValue())
		traits.style |= underline;
	
	return traits;
}

void CPrefsDisplayPanel::GetControls(SFullStyleItems& items, PaneIDT color_c, PaneIDT bold_c, PaneIDT italic_c, PaneIDT strike_c, PaneIDT underline_c)
{
	items.mColor = (LGAColorSwatchControl*) FindPaneByID(color_c);
	items.mBold = (LCheckBox*) FindPaneByID(bold_c);
	items.mItalic = (LCheckBox*) FindPaneByID(italic_c);
	items.mStrike = (LCheckBox*) FindPaneByID(strike_c);
	items.mUnderline = (LCheckBox*) FindPaneByID(underline_c);
}

void CPrefsDisplayPanel::SetStyle(SFullStyleItems& items, const SStyleTraits& traits)
{
	items.mColor->SetSwatchColor((RGBColor&) traits.color);
	items.mBold->SetValue((traits.style & bold) != 0);
	items.mItalic->SetValue((traits.style & italic) != 0);
	items.mStrike->SetValue((traits.style & strike_through) != 0);
	items.mUnderline->SetValue((traits.style & underline) != 0);
}

SStyleTraits& CPrefsDisplayPanel::GetStyle(SFullStyleItems& items, SStyleTraits& traits)
{
	items.mColor->GetSwatchColor(traits.color);
	traits.style = normal;
	if (items.mBold->GetValue())
		traits.style |= bold;
	if (items.mItalic->GetValue())
		traits.style |= italic;
	if (items.mStrike->GetValue())
		traits.style |= strike_through;
	if (items.mUnderline->GetValue())
		traits.style |= underline;
	
	return traits;
}

void CPrefsDisplayPanel::GetControls(SFullStyleItems2& items, PaneIDT color_c, PaneIDT usecolor_c, PaneIDT bkgcolor_c, PaneIDT usebkgcolor_c, PaneIDT bold_c, PaneIDT italic_c, PaneIDT strike_c, PaneIDT underline_c, PaneIDT name_c)
{
	items.mColor = (LGAColorSwatchControl*) FindPaneByID(color_c);
	items.mUseColor = (LCheckBox*) FindPaneByID(usecolor_c);
	items.mBkgColor = (LGAColorSwatchControl*) FindPaneByID(bkgcolor_c);
	items.mUseBkgColor = (LCheckBox*) FindPaneByID(usebkgcolor_c);
	items.mBold = (LCheckBox*) FindPaneByID(bold_c);
	items.mItalic = (LCheckBox*) FindPaneByID(italic_c);
	items.mStrike = (LCheckBox*) FindPaneByID(strike_c);
	items.mUnderline = (LCheckBox*) FindPaneByID(underline_c);
	items.mName = (CTextFieldX*) FindPaneByID(name_c);
}

void CPrefsDisplayPanel::SetStyle(SFullStyleItems2& items, const SStyleTraits2& traits)
{
	items.mColor->SetSwatchColor((RGBColor&) traits.color);
	items.mUseColor->SetValue(traits.usecolor);
	if (!traits.usecolor)
		items.mColor->Disable();
	items.mBkgColor->SetSwatchColor((RGBColor&) traits.bkgcolor);
	items.mUseBkgColor->SetValue(traits.usebkgcolor);
	if (!traits.usebkgcolor)
		items.mBkgColor->Disable();
	items.mBold->SetValue((traits.style & bold) != 0);
	items.mItalic->SetValue((traits.style & italic) != 0);
	items.mStrike->SetValue((traits.style & strike_through) != 0);
	items.mUnderline->SetValue((traits.style & underline) != 0);
	items.mName->SetText(traits.name);
}

SStyleTraits2& CPrefsDisplayPanel::GetStyle(SFullStyleItems2& items, SStyleTraits2& traits)
{
	items.mColor->GetSwatchColor(traits.color);
	items.mBkgColor->GetSwatchColor(traits.bkgcolor);
	traits.usecolor = items.mUseColor->GetValue();
	traits.usebkgcolor = items.mUseBkgColor->GetValue();
	traits.style = normal;
	if (items.mBold->GetValue())
		traits.style |= bold;
	if (items.mItalic->GetValue())
		traits.style |= italic;
	if (items.mStrike->GetValue())
		traits.style |= strike_through;
	if (items.mUnderline->GetValue())
		traits.style |= underline;
	traits.name = items.mName->GetText();
	
	return traits;
}
