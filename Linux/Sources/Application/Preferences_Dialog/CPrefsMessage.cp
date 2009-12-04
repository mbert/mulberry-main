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


// CPrefsMessage.cpp : implementation file
//


#include "CPrefsMessage.h"

#include "CTabController.h"

#include "CPrefsMessageGeneral.h"
#include "CPrefsMessageOptions.h"

#include <JXColormap.h>
#include <JXStaticText.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CPrefsMessage property page

CPrefsMessage::CPrefsMessage(JXContainer* enclosure,
								const HSizingOption hSizing, 
								const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) 
	:CPrefsPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
}

void CPrefsMessage::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Message Preferences", this,
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
	CTabPanel* card = new CPrefsMessageGeneral(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 380, 340);
	mTabs->AppendCard(card, "General");
	card = new CPrefsMessageOptions(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 380, 340);
	mTabs->AppendCard(card, "Options");
}

// Set up params for DDX
void CPrefsMessage::SetPrefs(CPreferences* prefs)
{
	mCopyPrefs = prefs;

	mTabs->SetData(prefs);
}

// Get params from DDX
void CPrefsMessage::UpdatePrefs(CPreferences* prefs)
{
	// Get values
	mTabs->UpdateData(prefs);
}

