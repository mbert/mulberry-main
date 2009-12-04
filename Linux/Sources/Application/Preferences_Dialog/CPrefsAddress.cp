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


// CPrefsAddress.cpp : implementation file
//


#include "CPrefsAddress.h"

#include "CPreferences.h"
#include "CPrefsAddressExpansion.h"
#include "CPrefsAddressCapture.h"
#include "CPrefsAccountLocal.h"
#include "CTabController.h"

#include "JXSecondaryRadioGroup.h"

#include <JXColormap.h>
#include <JXStaticText.h>
#include <JXStaticText.h>
#include <JXTextRadioButton.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddress property page

CPrefsAddress::CPrefsAddress(JXContainer* enclosure,
								const HSizingOption hSizing, 
								const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) 
	:CPrefsPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Set up params for DDX
void CPrefsAddress::SetPrefs(CPreferences* prefs)
{
	mCopyPrefs = prefs;

	mAddressChoiceGroup->SelectItem(prefs->mOptionKeyAddressDialog.GetValue() ? 1 : 2);
	mTabs->SetData(prefs);
}

// Get params from DDX
void CPrefsAddress::UpdatePrefs(CPreferences* prefs)
{
	// Get values
	prefs->mOptionKeyAddressDialog.SetValue(mAddressChoiceGroup->GetSelectedItem() == 1);
	mTabs->UpdateData(prefs);
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplay message handlers

void CPrefsAddress::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Address Preferences", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 140,20);
    assert( obj1 != NULL );
    const JFontStyle obj1_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj1->SetFontStyle(obj1_style);

    mAddressChoiceGroup =
        new JXSecondaryRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,40, 380,55);
    assert( mAddressChoiceGroup != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Allow Choice of Addresses when:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,30, 200,20);
    assert( obj2 != NULL );

    JXTextRadioButton* obj3 =
        new JXTextRadioButton(1, "Control key is Down", mAddressChoiceGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 150,20);
    assert( obj3 != NULL );

    JXTextRadioButton* obj4 =
        new JXTextRadioButton(2, "Control key is not Down", mAddressChoiceGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,30, 170,20);
    assert( obj4 != NULL );

    mTabs =
        new CTabController(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,125, 380,245);
    assert( mTabs != NULL );

// end JXLayout1

	// Create tab panels
	CTabPanel* card = new CPrefsAddressExpansion(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 380, 340);
	mTabs->AppendCard(card, "Expansion");

	card = new CPrefsAddressCapture(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 380, 340);
	mTabs->AppendCard(card, "Capture");

	card = new CPrefsAccountLocal(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 380, 340);
	static_cast<CPrefsAccountLocal*>(card)->SetLocalAddress();
	mTabs->AppendCard(card, "Local");
}
