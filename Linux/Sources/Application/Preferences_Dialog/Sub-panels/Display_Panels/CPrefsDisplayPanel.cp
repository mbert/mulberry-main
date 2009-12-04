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

#include "CTextField.h"
#include "CWindowStates.h"

#include "JXColorButton.h"
#include "JXTextCheckbox.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S


// O T H E R  M E T H O D S ____________________________________________________________________________

void CPrefsDisplayPanel::GetControls(SStyleItems& items, JXColorButton* color_c, JXTextCheckbox* bold_c,
										JXTextCheckbox* italic_c, JXTextCheckbox* underline_c)
{
	items.mColor = color_c;
	items.mBold = bold_c;
	items.mItalic = italic_c;
	items.mUnderline = underline_c;
}

void CPrefsDisplayPanel::SetStyle(SStyleItems& items, const SStyleTraits& traits)
{
	items.mColor->SetColor(traits.color);
	items.mBold->SetState(JBoolean((traits.style & bold) != 0));
	items.mItalic->SetState(JBoolean((traits.style & italic) != 0));
	items.mUnderline->SetState(JBoolean((traits.style & underline) != 0));
}

SStyleTraits& CPrefsDisplayPanel::GetStyle(SStyleItems& items, SStyleTraits& traits)
{
	traits.color = items.mColor->GetColor();
	traits.style = normal;
	if (items.mBold->IsChecked())
		traits.style |= bold;
	if (items.mItalic->IsChecked())
		traits.style |= italic;
	if (items.mUnderline->IsChecked())
		traits.style |= underline;
	
	return traits;
}

void CPrefsDisplayPanel::GetControls(SFullStyleItems& items, JXColorButton* color_c, JXTextCheckbox* bold_c,
										JXTextCheckbox* italic_c, JXTextCheckbox* strike_c, JXTextCheckbox* underline_c)
{
	items.mColor = color_c;
	items.mBold = bold_c;
	items.mItalic = italic_c;
	items.mStrike = strike_c;
	items.mUnderline = underline_c;
}

void CPrefsDisplayPanel::SetStyle(SFullStyleItems& items, const SStyleTraits& traits)
{
	items.mColor->SetColor(traits.color);
	items.mBold->SetState(JBoolean((traits.style & bold) != 0));
	items.mItalic->SetState(JBoolean((traits.style & italic) != 0));
	items.mStrike->SetState(JBoolean((traits.style & strike_through) != 0));
	items.mUnderline->SetState(JBoolean((traits.style & underline) != 0));
}

SStyleTraits& CPrefsDisplayPanel::GetStyle(SFullStyleItems& items, SStyleTraits& traits)
{
	traits.color = items.mColor->GetColor();
	traits.style = normal;
	if (items.mBold->IsChecked())
		traits.style |= bold;
	if (items.mItalic->IsChecked())
		traits.style |= italic;
	if (items.mStrike->IsChecked())
		traits.style |= strike_through;
	if (items.mUnderline->IsChecked())
		traits.style |= underline;
	
	return traits;
}

void CPrefsDisplayPanel::GetControls(SFullStyleItems2& items, JXColorButton* color_c, JXTextCheckbox* usecolor_c, JXColorButton* bkgnd_c, JXTextCheckbox* usebkgnd_c,
										JXTextCheckbox* bold_c, JXTextCheckbox* italic_c, JXTextCheckbox* strike_c, JXTextCheckbox* underline_c, CTextInputField* name_c)
{
	items.mUseColor = usecolor_c;
	items.mColor = color_c;
	items.mUseBkgColor = usebkgnd_c;
	items.mBkgColor = bkgnd_c;
	items.mBold = bold_c;
	items.mItalic = italic_c;
	items.mStrike = strike_c;
	items.mUnderline = underline_c;
	items.mName = name_c;
}

void CPrefsDisplayPanel::SetStyle(SFullStyleItems2& items, const SStyleTraits2& traits)
{
	items.mColor->SetColor(traits.color);
	items.mUseColor->SetState(JBoolean(traits.usecolor));
	if (!traits.usecolor)
		items.mColor->Deactivate();
	items.mBkgColor->SetColor(traits.bkgcolor);
	items.mUseBkgColor->SetState(JBoolean(traits.usebkgcolor));
	if (!traits.usebkgcolor)
		items.mBkgColor->Deactivate();
	items.mBold->SetState(JBoolean((traits.style & bold) != 0));
	items.mItalic->SetState(JBoolean((traits.style & italic) != 0));
	items.mStrike->SetState(JBoolean((traits.style & strike_through) != 0));
	items.mUnderline->SetState(JBoolean((traits.style & underline) != 0));
	items.mName->SetText(traits.name);
}

SStyleTraits2& CPrefsDisplayPanel::GetStyle(SFullStyleItems2& items, SStyleTraits2& traits)
{
	traits.color = items.mColor->GetColor();
	traits.usecolor = items.mUseColor->IsChecked();
	traits.bkgcolor = items.mBkgColor->GetColor();
	traits.usebkgcolor = items.mUseBkgColor->IsChecked();
	traits.style = normal;
	if (items.mBold->IsChecked())
		traits.style |= bold;
	if (items.mItalic->IsChecked())
		traits.style |= italic;
	if (items.mStrike->IsChecked())
		traits.style |= strike_through;
	if (items.mUnderline->IsChecked())
		traits.style |= underline;
	traits.name = items.mName->GetText();
	
	return traits;
}
