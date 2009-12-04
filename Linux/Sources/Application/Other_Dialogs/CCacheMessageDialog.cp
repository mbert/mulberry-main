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


// CCacheMessageDialog.cpp : implementation file
//

#include "CCacheMessageDialog.h"

#include <JXWindow.h>
#include <JXUpRect.h>
#include <JXTextButton.h>
#include <JXIntegerInput.h>
#include <JXStaticText.h>
#include "CInputField.h"

#include <jXGlobals.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CCacheMessageDialog dialog


CCacheMessageDialog::CCacheMessageDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

void CCacheMessageDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 280,110, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 280,110);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Goto Message Number:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 25,25, 150,20);
    assert( obj2 != NULL );

    mEnterCtrl =
        new CInputField<JXIntegerInput>(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 170,25, 75,20);
    assert( mEnterCtrl != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 190,75, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 100,75, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

// end JXLayout

	window->SetTitle("Messages");
	SetButtons(mOKBtn, mCancelBtn);
	mEnterCtrl->SetValue(1);
}

bool CCacheMessageDialog::PoseDialog(unsigned long& goto_num)
{
	bool result = false;

	CCacheMessageDialog* dlog = new CCacheMessageDialog(JXGetApplication());

	// Test for OK
	if (dlog->DoModal() == kDialogClosed_OK)
	{
		// Get the number
		JInteger temp;
		dlog->mEnterCtrl->GetValue(&temp);
		goto_num = temp;

		result = true;
		dlog->Close();
	}

	return result;
}
