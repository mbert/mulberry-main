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


// Source for CPrefsLetterStyled class

#include "CPrefsLetterStyled.h"

#include "CAdminLock.h"
#include "CPreferences.h"

#include "TPopupMenu.h"

#include <JXDownRect.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsLetterStyled::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Compose Text as:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 110,20);
    assert( obj1 != NULL );

    mComposeAsPopup =
        new HPopupMenu("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,10, 155,25);
    assert( mComposeAsPopup != NULL );

    JXDownRect* obj2 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,50, 360,75);
    assert( obj2 != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Generate Alternatives:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,40, 135,20);
    assert( obj3 != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Generate:", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 120,8, 65,20);
    assert( obj4 != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Generate:", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 190,8, 65,20);
    assert( obj5 != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Compose Enriched:", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,28, 120,20);
    assert( obj6 != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Compose HTML:", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,48, 120,20);
    assert( obj7 != NULL );

    mEnrichedPlain =
        new JXTextCheckbox("Plain", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 120,25, 60,20);
    assert( mEnrichedPlain != NULL );

    mEnrichedHTML =
        new JXTextCheckbox("HTML", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 190,25, 65,20);
    assert( mEnrichedHTML != NULL );

    mHTMLPlain =
        new JXTextCheckbox("Plain", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 120,45, 60,20);
    assert( mHTMLPlain != NULL );

    mHTMLEnriched =
        new JXTextCheckbox("Enriched", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 190,45, 80,20);
    assert( mHTMLEnriched != NULL );

    mFormatFlowed =
        new JXTextCheckbox("Send Plain Text Flowed", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,140, 165,20);
    assert( mFormatFlowed != NULL );

// end JXLayout1

	mComposeAsPopup->SetMenuItems("Plain %r| Enriched %r| HTML %r");
	mComposeAsPopup->SetUpdateAction(JXMenu::kDisableNone);
	mComposeAsPopup->SetToPopupChoice(kTrue, 1);

	if (!CAdminLock::sAdminLock.mAllowStyledComposition)
	{
		mComposeAsPopup->Deactivate();
		mEnrichedPlain->Deactivate();
		mEnrichedHTML->Deactivate();
		mHTMLPlain->Deactivate();
		mHTMLEnriched->Deactivate();
	}
}

// Set prefs
void CPrefsLetterStyled::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info to into panel
	if (CAdminLock::sAdminLock.mAllowStyledComposition)
	{
		mComposeAsPopup->SetValue(copyPrefs->compose_as.GetValue());

		mEnrichedPlain->SetState(JBoolean(copyPrefs->enrMultiAltPlain.GetValue()));
		mEnrichedHTML->SetState(JBoolean(copyPrefs->enrMultiAltHTML.GetValue()));
		mHTMLPlain->SetState(JBoolean(copyPrefs->htmlMultiAltPlain.GetValue()));
		mHTMLEnriched->SetState(JBoolean(copyPrefs->htmlMultiAltEnriched.GetValue()));
	}
	mFormatFlowed->SetState(JBoolean(copyPrefs->mFormatFlowed.GetValue()));
}

// Force update of prefs
bool CPrefsLetterStyled::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	if (CAdminLock::sAdminLock.mAllowStyledComposition)
	{
		copyPrefs->compose_as.SetValue((EContentSubType) mComposeAsPopup->GetValue());

		copyPrefs->enrMultiAltPlain.SetValue(mEnrichedPlain->IsChecked());
		copyPrefs->htmlMultiAltEnriched.SetValue(mHTMLEnriched->IsChecked());
		copyPrefs->enrMultiAltHTML.SetValue(mEnrichedHTML->IsChecked());
		copyPrefs->htmlMultiAltPlain.SetValue(mHTMLPlain->IsChecked());
	}
	copyPrefs->mFormatFlowed.SetValue(mFormatFlowed->IsChecked());
	
	return true;
}
