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


// CPrefsFormatting.cpp : implementation file
//


#include "CPrefsFormatting.h"

#include "CFontNameMenu.h"
#include "CFontSizeMenu.h"
#include "CPreferences.h"

#include "TPopupMenu.h"

#include <JXColormap.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CPrefsFormatting dialog

CPrefsFormatting::CPrefsFormatting( JXContainer* enclosure,
														const HSizingOption hSizing, 
														const VSizingOption vSizing,
														const JCoordinate x, const JCoordinate y,
														const JCoordinate w, const JCoordinate h) 
	:CPrefsPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
	mPrefsSet = false;
}

void CPrefsFormatting::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Font Preferences", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 125,20);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Size:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 305,35, 60,15);
    assert( obj2 != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Lists:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,60, 91,17);
    assert( obj3 != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Plain Text:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,90, 91,17);
    assert( obj4 != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Printed Text:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,130, 91,17);
    assert( obj5 != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Print Captions:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,160, 91,17);
    assert( obj6 != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Styled Base:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,200, 91,17);
    assert( obj7 != NULL );

    JXStaticText* obj8 =
        new JXStaticText("Styled Fixed:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,230, 91,17);
    assert( obj8 != NULL );

    JXStaticText* obj9 =
        new JXStaticText("Font:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,35, 60,15);
    assert( obj9 != NULL );

    mListFont =
        new CFontNamePopup("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,58, 160,20);
    assert( mListFont != NULL );

    mListSize =
        new CFontSizePopup("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 305,58, 60,20);
    assert( mListSize != NULL );

    mDisplayFont =
        new CFontNamePopup("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,88, 160,20);
    assert( mDisplayFont != NULL );

    mDisplaySize =
        new CFontSizePopup("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 305,88, 60,20);
    assert( mDisplaySize != NULL );

    mPrinterFont =
        new CFontNamePopup("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,128, 160,20);
    assert( mPrinterFont != NULL );

    mPrinterSize =
        new CFontSizePopup("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 305,128, 60,20);
    assert( mPrinterSize != NULL );

    mCaptionFont =
        new CFontNamePopup("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,158, 160,20);
    assert( mCaptionFont != NULL );

    mCaptionSize =
        new CFontSizePopup("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 305,158, 60,20);
    assert( mCaptionSize != NULL );

    mHTMLFont =
        new CFontNamePopup("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,198, 160,20);
    assert( mHTMLFont != NULL );

    mHTMLSize =
        new CFontSizePopup("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 305,198, 60,20);
    assert( mHTMLSize != NULL );

    mFixedFont =
        new CFontNamePopup("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,228, 160,20);
    assert( mFixedFont != NULL );

    mFixedSize =
        new CFontSizePopup("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 305,228, 60,20);
    assert( mFixedSize != NULL );

    JXStaticText* obj10 =
        new JXStaticText("Smallest Size Allowed in Styled Text:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,272, 221,17);
    assert( obj10 != NULL );

    mMinimumSize =
        new CFontSizePopup("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 305,270, 60,20);
    assert( mMinimumSize != NULL );

    mUseStyles =
        new JXTextCheckbox("Use Styled Fonts in Formatted Messages", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 20,310, 265,20);
    assert( mUseStyles != NULL );

// end JXLayout1

    mListFont->OnCreate(true);
    mListFont->CFontNameMenu::SetToPopupChoice();
    mListSize->OnCreate(mListFont);
    mListSize->CFontSizeMenu::SetToPopupChoice();

    mDisplayFont->OnCreate(true);
    mDisplayFont->CFontNameMenu::SetToPopupChoice();
    mDisplaySize->OnCreate(mDisplayFont);
    mDisplaySize->CFontSizeMenu::SetToPopupChoice();

    mPrinterFont->OnCreate(false);
    mPrinterFont->CFontNameMenu::SetToPopupChoice();
    mPrinterSize->OnCreate(mPrinterFont);
    mPrinterSize->CFontSizeMenu::SetToPopupChoice();

    mCaptionFont->OnCreate(true);
    mCaptionFont->CFontNameMenu::SetToPopupChoice();
    mCaptionSize->OnCreate(mCaptionFont);
    mCaptionSize->CFontSizeMenu::SetToPopupChoice();

    mFixedFont->OnCreate(true, true);
    mFixedFont->CFontNameMenu::SetToPopupChoice();
    mFixedSize->OnCreate(mFixedFont);
    mFixedSize->CFontSizeMenu::SetToPopupChoice();

    mHTMLFont->OnCreate(true);
    mHTMLFont->CFontNameMenu::SetToPopupChoice();
    mHTMLSize->OnCreate(mHTMLFont);
    mHTMLSize->CFontSizeMenu::SetToPopupChoice();

    mMinimumSize->OnCreate(mHTMLFont);
    mMinimumSize->CFontSizeMenu::SetToPopupChoice();
}

// Set up params for DDX
void CPrefsFormatting::SetPrefs(CPreferences* prefs)
{
	// Save ref to prefs
	mCopyPrefs = prefs;

	mUseStyles->SetState(JBoolean(prefs->mUseStyles.GetValue()));

	// Just cache the prefs ptr for now
	// We'll actually use it the first time we are displayed
}


// Set up params for DDX
void CPrefsFormatting::AboutToShow()
{
	// Only do once
	if (mPrefsSet)
		return;

	// Must set up the font menus
	mListFont->BuildMenuNow();
	mDisplayFont->BuildMenuNow();
	mPrinterFont->BuildMenuNow();
	mCaptionFont->BuildMenuNow();
	mFixedFont->BuildMenuNow();
	mHTMLFont->BuildMenuNow();

	// Get ref to prefs
	CPreferences* local_prefs = mCopyPrefs;

	// Copy info
#ifdef USE_FONTMAPPER
	ECharset charset = eUSAscii;
	
	const SFontInfo* ttraits = &local_prefs->mListFontMap.GetValue().GetCharsetFontDescriptor(charset).mTraits;
	mListFont->SetFontName(ttraits->fontname);
	mListSize->SetFontSize(ttraits->size);

	ttraits = &local_prefs->mDisplayFontMap.GetValue().GetCharsetFontDescriptor(charset).mTraits;
	mDisplayFont->SetFontName(ttraits->fontname);
	mDisplaySize->SetFontSize(ttraits->size);

	ttraits = &local_prefs->mPrintFontMap.GetValue().GetCharsetFontDescriptor(charset).mTraits;
	mPrinterFont->SetFontName(ttraits->fontname);
	mPrinterSize->SetFontSize(ttraits->size);

	ttraits = &local_prefs->mCaptionFontMap.GetValue().GetCharsetFontDescriptor(charset).mTraits;
	mCaptionFont->SetFontName(ttraits->fontname);
	mCaptionSize->SetFontSize(ttraits->size);

	ttraits = &local_prefs->mFixedFontMap.GetValue().GetCharsetFontDescriptor(charset).mTraits;
	mFixedFont->SetFontName(ttraits->fontname);
	mFixedSize->SetFontSize(ttraits->size);

	ttraits = &local_prefs->mHTMLFontMap.GetValue().GetCharsetFontDescriptor(charset).mTraits;
	mHTMLFont->SetFontName(ttraits->fontname);
	mHTMLSize->SetFontSize(ttraits->size);
#else
	mListFont->SetFontName(local_prefs->mListTextFontInfo.GetValue().fontname);
	mListSize->SetFontSize(local_prefs->mListTextFontInfo.GetValue().size);

	mDisplayFont->SetFontName(local_prefs->mDisplayTextFontInfo.GetValue().fontname);
	mDisplaySize->SetFontSize(local_prefs->mDisplayTextFontInfo.GetValue().size);

	mPrinterFont->SetFontName(local_prefs->mPrintTextFontInfo.GetValue().fontname);
	mPrinterSize->SetFontSize(local_prefs->mPrintTextFontInfo.GetValue().size);

	mCaptionFont->SetFontName(local_prefs->mCaptionTextFontInfo.GetValue().fontname);
	mCaptionSize->SetFontSize(local_prefs->mCaptionTextFontInfo.GetValue().size);

	mFixedFont->SetFontName(local_prefs->mFixedTextFontInfo.GetValue().fontname);
	mFixedSize->SetFontSize(local_prefs->mFixedTextFontInfo.GetValue().size);

	mHTMLFont->SetFontName(local_prefs->mHTMLTextFontInfo.GetValue().fontname);
	mHTMLSize->SetFontSize(local_prefs->mHTMLTextFontInfo.GetValue().size);
#endif

	mMinimumSize->SetFontSize(local_prefs->mMinimumFont.GetValue());
}

// Get params from DDX
void CPrefsFormatting::UpdatePrefs(CPreferences* prefs)
{
	CPreferences *local_prefs = mCopyPrefs;

	// Copy info from panel into prefs
#ifdef USE_FONTMAPPER
	ECharset charset = eUSAscii;
	CFontDescriptor font;

	SFontInfo* old_traits = &local_prefs->mListFontMap.Value().GetCharsetFontDescriptor(charset).mTraits;
	SFontInfo new_traits;
	new_traits.fontname = mListFont->GetFontName();
	new_traits.size = mListSize->GetFontSize();
	if (new_traits != *old_traits)
	{
		*old_traits = new_traits;
		local_prefs->mListFontMap.SetDirty();
	}

	old_traits = &local_prefs->mDisplayFontMap.Value().GetCharsetFontDescriptor(charset).mTraits;
	new_traits.fontname = mDisplayFont->GetFontName();
	new_traits.size = mDisplaySize->GetFontSize();
	if (new_traits != *old_traits)
	{
		*old_traits = new_traits;
		local_prefs->mDisplayFontMap.SetDirty();
	}

	old_traits = &local_prefs->mPrintFontMap.Value().GetCharsetFontDescriptor(charset).mTraits;
	new_traits.fontname = mPrintFont->GetFontName();
	new_traits.size = mPrintSize->GetFontSize();
	if (new_traits != *old_traits)
	{
		*old_traits = new_traits;
		local_prefs->mPrintFontMap.SetDirty();
	}

	old_traits = &local_prefs->mCaptionFontMap.Value().GetCharsetFontDescriptor(charset).mTraits;
	new_traits.fontname = mCaptionFont->GetFontName();
	new_traits.size = mCaptionSize->GetFontSize();
	if (new_traits != *old_traits)
	{
		*old_traits = new_traits;
		local_prefs->mCaptionFontMap.SetDirty();
	}

	old_traits = &local_prefs->mFixedFontMap.Value().GetCharsetFontDescriptor(charset).mTraits;
	new_traits.fontname = mFixedFont->GetFontName();
	new_traits.size = mFixedSize->GetFontSize();
	if (new_traits != *old_traits)
	{
		*old_traits = new_traits;
		local_prefs->mFixedFontMap.SetDirty();
	}

	old_traits = &local_prefs->mHTMLFontMap.Value().GetCharsetFontDescriptor(charset).mTraits;
	new_traits.fontname = mHTMLFont->GetFontName();
	new_traits.size = mHTMLSize->GetFontSize();
	if (new_traits != *old_traits)
	{
		*old_traits = new_traits;
		local_prefs->mHTMLFontMap.SetDirty();
	}
#else
	SFontInfo ttraits;
	ttraits.fontname = mListFont->GetFontName();
	ttraits.size = mListSize->GetFontSize();
	local_prefs->mListTextFontInfo.SetValue(ttraits);

	ttraits.fontname = mDisplayFont->GetFontName();
	ttraits.size = mDisplaySize->GetFontSize();
	local_prefs->mDisplayTextFontInfo.SetValue(ttraits);

	ttraits.fontname = mPrinterFont->GetFontName();
	ttraits.size = mPrinterSize->GetFontSize();
	local_prefs->mPrintTextFontInfo.SetValue(ttraits);

	ttraits.fontname = mCaptionFont->GetFontName();
	ttraits.size = mCaptionSize->GetFontSize();
	local_prefs->mCaptionTextFontInfo.SetValue(ttraits);

	ttraits.fontname = mFixedFont->GetFontName();
	ttraits.size = mFixedSize->GetFontSize();
	local_prefs->mFixedTextFontInfo.SetValue(ttraits);

	ttraits.fontname = mHTMLFont->GetFontName();
	ttraits.size = mHTMLSize->GetFontSize();
	local_prefs->mHTMLTextFontInfo.SetValue(ttraits);
#endif

	local_prefs->mMinimumFont.SetValue(mMinimumSize->GetFontSize());

	local_prefs->mUseStyles.SetValue(mUseStyles->IsChecked());
}
