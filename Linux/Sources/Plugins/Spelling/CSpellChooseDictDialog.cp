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


// Source for CSpellChooseDictDialog class

#include "CSpellChooseDictDialog.h"

#include "CMulberryCommon.h"
#include "CSpellPlugin.h"

#include <JXTextButton.h>
#include <JXStaticText.h>
#include <JXUpRect.h>
#include <JXWindow.h>
#include "TPopupMenu.h"

#include <jXGlobals.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSpellChooseDictDialog::CSpellChooseDictDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

// Default destructor
CSpellChooseDictDialog::~CSpellChooseDictDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSpellChooseDictDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 340,130, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 340,130);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Choose the Main Spell Checking Dictionary:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 260,20);
    assert( obj2 != NULL );

    mDictionaries =
        new HPopupMenu("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 115,40, 210,25);
    assert( mDictionaries != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 250,95, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 160,95, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    JXStaticText* obj3 =
        new JXStaticText("Main Dictionary:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,45, 105,20);
    assert( obj3 != NULL );

// end JXLayout
	window->SetTitle("Choose Dictionary");
	SetButtons(mOKBtn, mCancelBtn);
}

// Set the details
void CSpellChooseDictDialog::SetSpeller(CSpellPlugin* speller)
{
	InitDictionaries(speller);
}

// Get the details
void CSpellChooseDictDialog::GetOptions(CSpellPlugin* speller)
{
	speller->GetPreferences()->mDictionaryName.SetValue(cdstring(mDictionaries->GetCurrentItemText()));
}

// Called during idle
void CSpellChooseDictDialog::InitDictionaries(CSpellPlugin* speller)
{
	mDictionaries->RemoveAllItems();

	// Add each enabled password changing plugin
	const char* names;
	if (speller->GetDictionaries(&names))
	{
		const char** name = (const char**)(names);
		JIndex menu_pos = 1;
		JIndex choice_pos = 1;
		while(*name)
		{
			// Get the index of the one we want to initially select
			if (speller->GetPreferences()->mDictionaryName.GetValue() == *name)
				choice_pos = menu_pos;

			mDictionaries->AppendItem(*name++, kFalse, kTrue);
			menu_pos++;
		}

		// Set intitial item
		mDictionaries->SetToPopupChoice(kTrue, choice_pos);
	}
}

bool CSpellChooseDictDialog::PoseDialog(CSpellPlugin* speller)
{
	bool result = false;

	CSpellChooseDictDialog* dlog = new CSpellChooseDictDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetSpeller(speller);

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetOptions(speller);
		result = true;
		dlog->Close();
	}

	return result;
}
