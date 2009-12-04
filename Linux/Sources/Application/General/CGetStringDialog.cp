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

#include "CGetStringDialog.h"

#include "cdstring.h"
#include "CStaticText.h"
#include "CTextField.h"
#include "CXStringResources.h"

#include <JXTextButton.h>
#include <JXWindow.h>
#include <JXUpRect.h>

#include <jXGlobals.h>

#include <cassert>

CGetStringDialog::CGetStringDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor, kTrue)
{}

void CGetStringDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 390,125, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 390,125);
    assert( obj1 != NULL );

    mPromptText =
        new CStaticText("Text", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,20, 350,20);
    assert( mPromptText != NULL );

    mStringInput =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,45, 350,25);
    assert( mStringInput != NULL );

    mOkBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 300,90, 70,25);
    assert( mOkBtn != NULL );
    mOkBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 210,90, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

// end JXLayout
	SetButtons(mOkBtn, mCancelBtn);
}

bool CGetStringDialog::PoseDialog(int title, int prompt, cdstring& name)
{
	cdstring window;
	window.FromResource(title);
	cdstring titletxt;
	titletxt.FromResource(prompt);
	return _PoseDialog(window, titletxt, name);
}

bool CGetStringDialog::PoseDialog(const char* window, const char* title, cdstring& name)
{
	return _PoseDialog(rsrc::GetString(window), rsrc::GetString(title), name);
}

bool CGetStringDialog::PoseDialog(const char* title, cdstring& name)
{
	return _PoseDialog(NULL, rsrc::GetString(title), name);
}

bool CGetStringDialog::_PoseDialog(const char* window, const char* title, cdstring& name)
{
	bool result = false;

	CGetStringDialog* dlog = new CGetStringDialog(JXGetApplication());
	dlog->OnCreate();

	if (window != NULL)
		dlog->GetWindow()->SetTitle(window);

	dlog->mPromptText->SetText(title);
	dlog->mStringInput->SetText(name.c_str());
	
	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		name = dlog->mStringInput->GetText();
		result = true;
		dlog->Close();
	}

	return result;
}
