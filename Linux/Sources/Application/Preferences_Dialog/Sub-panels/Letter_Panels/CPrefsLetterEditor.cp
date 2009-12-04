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


// Source for CPrefsLetterEditor class

#include "CPrefsLetterEditor.h"

#include "CPreferences.h"
#include "CTextField.h"
#include <JXDownRect.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsLetterEditor::OnCreate()
{
// begin JXLayout1

    JXDownRect* obj1 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,30, 360,55);
    assert( obj1 != NULL );

    mUseExternalEditor =
        new JXTextCheckbox("Enable Use of External Editor", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,20, 210,20);
    assert( mUseExternalEditor != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Editor Command:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,11, 105,20);
    assert( obj2 != NULL );

    mExternalEditor =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 115,9, 230,20);
    assert( mExternalEditor != NULL );

    mExternalEditAutoStart =
        new JXTextCheckbox("Use External Editor when Draft is Created", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,30, 270,20);
    assert( mExternalEditAutoStart != NULL );

// end JXLayout1

	// Start listening
	ListenTo(mUseExternalEditor);
}

void CPrefsLetterEditor::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mUseExternalEditor)
		{
			if (mUseExternalEditor->IsChecked())
			{
				mExternalEditor->Activate();
				mExternalEditAutoStart->Activate();
			}
			else
			{
				mExternalEditor->Deactivate();
				mExternalEditAutoStart->Deactivate();
			}
		}
	}
	
	CTabPanel::Receive(sender, message);
}

// Set prefs
void CPrefsLetterEditor::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Save ref to prefs
	mCopyPrefs = copyPrefs;

	mUseExternalEditor->SetState(JBoolean(copyPrefs->mUseExternalEditor.GetValue()));
	mExternalEditor->SetText(copyPrefs->mExternalEditor.GetValue());
	mExternalEditAutoStart->SetState(JBoolean(copyPrefs->mExternalEditAutoStart.GetValue()));
	if (!mUseExternalEditor->IsChecked())
	{
		mExternalEditor->Deactivate();
		mExternalEditAutoStart->Deactivate();
	}
}

// Force update of prefs
bool CPrefsLetterEditor::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	copyPrefs->mUseExternalEditor.SetValue(mUseExternalEditor->IsChecked());
	copyPrefs->mExternalEditor.SetValue(cdstring(mExternalEditor->GetText()));
	copyPrefs->mExternalEditAutoStart.SetValue(mExternalEditAutoStart->IsChecked());
	
	return true;
}
