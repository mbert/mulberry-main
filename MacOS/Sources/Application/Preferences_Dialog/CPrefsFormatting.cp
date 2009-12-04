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


#include "CPrefsFormatting.h"

#include "CPreferences.h"

#include "CFontPopup.h"
#include "CSizePopup.h"

#include <LCheckBox.h>

CPrefsFormatting::CPrefsFormatting()
{
}

CPrefsFormatting::CPrefsFormatting(LStream *inStream)
		: CPrefsPanel(inStream)
{
}

CPrefsFormatting::~CPrefsFormatting()
{
}

void CPrefsFormatting::FinishCreateSelf(void)
{
	CPrefsPanel::FinishCreateSelf();

	mListFontMenu = (CFontPopup*) FindPaneByID(paneid_ListFontMenu);
	mListSizeMenu = (CSizePopup*) FindPaneByID(paneid_ListSizeMenu);

	mDisplayFontMenu = (CFontPopup*) FindPaneByID(paneid_DisplayFontMenu);
	mDisplaySizeMenu = (CSizePopup*) FindPaneByID(paneid_DisplaySizeMenu);

	mPrintFontMenu = (CFontPopup*) FindPaneByID(paneid_PrintFontMenu);
	mPrintSizeMenu = (CSizePopup*) FindPaneByID(paneid_PrintSizeMenu);

	mCaptionFontMenu = (CFontPopup*) FindPaneByID(paneid_CaptionFontMenu);
	mCaptionSizeMenu = (CSizePopup*) FindPaneByID(paneid_CaptionSizeMenu);

	mFixedFont = (CFontPopup*) FindPaneByID(paneid_FixedFont);
	mFixedSize = (CSizePopup*) FindPaneByID(paneid_FixedSize);

	mHTMLFont = (CFontPopup*) FindPaneByID(paneid_HTMLFont);
	mHTMLSize = (CSizePopup*) FindPaneByID(paneid_HTMLSize);

	mMinimumSize = (CSizePopup*) FindPaneByID(paneid_MinimumSize);

	mUseStyles = (LCheckBox*) FindPaneByID(paneid_UseStyles);

	mAntiAliasFont = (LCheckBox*) FindPaneByID(paneid_AAText);
	mAntiAliasFont->Hide();
}


void CPrefsFormatting::ToggleICDisplay(void)
{
	if (mCopyPrefs->mUse_IC.GetValue())
	{
		mListFontMenu->Disable();
		mListSizeMenu->Disable();
		mDisplayFontMenu->Disable();
		mDisplaySizeMenu->Disable();
		mPrintFontMenu->Disable();
		mPrintSizeMenu->Disable();
	}
	else
	{
		mListFontMenu->Enable();
		mListSizeMenu->Enable();
		mDisplayFontMenu->Enable();
		mDisplaySizeMenu->Enable();
		mPrintFontMenu->Enable();
		mPrintSizeMenu->Enable();
	}
}


void CPrefsFormatting::SetPrefs(CPreferences *copyPrefs)
{
	// Save ref to prefs
	mCopyPrefs = copyPrefs;
	CPreferences* local_prefs = copyPrefs;

	// Toggle IC display
	ToggleICDisplay();

	// Copy info
	mListFontMenu->SetFontName(local_prefs->mListTextTraits.GetValue().traits.fontName);
	mListSizeMenu->SetSize(local_prefs->mListTextTraits.GetValue().traits.size);

	mDisplayFontMenu->SetFontName(local_prefs->mDisplayTextTraits.GetValue().traits.fontName);
	mDisplaySizeMenu->SetSize(local_prefs->mDisplayTextTraits.GetValue().traits.size);

	mPrintFontMenu->SetFontName(local_prefs->mPrintTextTraits.GetValue().traits.fontName);
	mPrintSizeMenu->SetSize(local_prefs->mPrintTextTraits.GetValue().traits.size);

	mCaptionFontMenu->SetFontName(local_prefs->mCaptionTextTraits.GetValue().traits.fontName);
	mCaptionSizeMenu->SetSize(local_prefs->mCaptionTextTraits.GetValue().traits.size);

	mFixedFont->SetFontName(local_prefs->mFixedTextTraits.GetValue().traits.fontName);
	mFixedSize->SetSize(local_prefs->mFixedTextTraits.GetValue().traits.size);

	mHTMLFont->SetFontName(local_prefs->mHTMLTextTraits.GetValue().traits.fontName);
	mHTMLSize->SetSize(local_prefs->mHTMLTextTraits.GetValue().traits.size);

	mUseStyles->SetValue(local_prefs->mUseStyles.GetValue());
	mMinimumSize->SetSize(local_prefs->mMinimumFont.GetValue());

#if PP_Target_Carbon
	mAntiAliasFont->SetValue(local_prefs->mAntiAliasFont.GetValue());
#endif
}



void CPrefsFormatting::UpdatePrefs(void)
{
	CPreferences *local_prefs = mCopyPrefs;

	// Copy info from panel into prefs
	STextTraitsRecord ttraits = local_prefs->mListTextTraits.GetValue();
	mListFontMenu->GetFontName(ttraits.traits.fontName);
	ttraits.traits.fontNumber = UTextTraits::fontNumber_Unknown;
	ttraits.traits.size = mListSizeMenu->GetSize();
	local_prefs->mListTextTraits.SetValue(ttraits);

	ttraits = local_prefs->mDisplayTextTraits.GetValue();
	mDisplayFontMenu->GetFontName(ttraits.traits.fontName);
	ttraits.traits.fontNumber = UTextTraits::fontNumber_Unknown;
	ttraits.traits.size = mDisplaySizeMenu->GetSize();
	local_prefs->mDisplayTextTraits.SetValue(ttraits);

	ttraits = local_prefs->mPrintTextTraits.GetValue();
	mPrintFontMenu->GetFontName(ttraits.traits.fontName);
	ttraits.traits.fontNumber = UTextTraits::fontNumber_Unknown;
	ttraits.traits.size = mPrintSizeMenu->GetSize();
	local_prefs->mPrintTextTraits.SetValue(ttraits);

	ttraits = local_prefs->mCaptionTextTraits.GetValue();
	mCaptionFontMenu->GetFontName(ttraits.traits.fontName);
	ttraits.traits.fontNumber = UTextTraits::fontNumber_Unknown;
	ttraits.traits.size = mCaptionSizeMenu->GetSize();
	local_prefs->mCaptionTextTraits.SetValue(ttraits);

	ttraits = local_prefs->mFixedTextTraits.GetValue();
	mFixedFont->GetFontName(ttraits.traits.fontName);
	ttraits.traits.fontNumber = UTextTraits::fontNumber_Unknown;
	ttraits.traits.size = mFixedSize->GetSize();
	local_prefs->mFixedTextTraits.SetValue(ttraits);

	ttraits = local_prefs->mHTMLTextTraits.GetValue();
	mHTMLFont->GetFontName(ttraits.traits.fontName);
	ttraits.traits.fontNumber = UTextTraits::fontNumber_Unknown;
	ttraits.traits.size = mHTMLSize->GetSize();
	local_prefs->mHTMLTextTraits.SetValue(ttraits);

	local_prefs->mUseStyles.SetValue(mUseStyles->GetValue());
	local_prefs->mMinimumFont.SetValue(mMinimumSize->GetSize());

#if PP_Target_Carbon
	local_prefs->mAntiAliasFont.SetValue(mAntiAliasFont->GetValue());
#endif
}


