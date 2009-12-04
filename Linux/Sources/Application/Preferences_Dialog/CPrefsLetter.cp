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


// CPrefsLetter.cpp : implementation file
//


#include "CPrefsLetter.h"

#include "CTabController.h"

#include "CPrefsLetterGeneral.h"
#include "CPrefsLetterOptions.h"
#include "CPrefsLetterStyled.h"
#include "CPrefsLetterEditor.h"

#include <JXColormap.h>
#include <JXStaticText.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CPrefsLetter dialog

CPrefsLetter::CPrefsLetter(JXContainer* enclosure,
								const HSizingOption hSizing, 
								const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) 
	:CPrefsPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Set up params for DDX
void CPrefsLetter::SetPrefs(CPreferences* prefs)
{
	mCopyPrefs = prefs;

	mTabs->SetData(prefs);
}

// Get params from DDX
void CPrefsLetter::UpdatePrefs(CPreferences* prefs)
{
	// Get values
	mTabs->UpdateData(prefs);
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplay message handlers

void CPrefsLetter::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Outgoing Preferences", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 140,20);
    assert( obj1 != NULL );
    const JFontStyle obj1_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj1->SetFontStyle(obj1_style);

    mTabs =
        new CTabController(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,25, 380,365);
    assert( mTabs != NULL );

// end JXLayout1

	// Create tab panels
	CTabPanel* card = new CPrefsLetterGeneral(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 380, 340);
	mTabs->AppendCard(card, "General");
	card = new CPrefsLetterOptions(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 380, 340);
	mTabs->AppendCard(card, "Options");
	card = new CPrefsLetterStyled(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 380, 340);
	mTabs->AppendCard(card, "Styled");
	card = new CPrefsLetterEditor(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 380, 340);
	mTabs->AppendCard(card, "Editor");
}
