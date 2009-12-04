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


// Source for CSearchOptionsDialog class

#include "CSearchOptionsDialog.h"

#include "CPreferences.h"

#include <JXEngravedRect.h>
#include <JXIntegerInput.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXUpRect.h>
#include <JXWindow.h>
#include "CInputField.h"

#include <jXGlobals.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSearchOptionsDialog::CSearchOptionsDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

// Default destructor
CSearchOptionsDialog::~CSearchOptionsDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSearchOptionsDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 270,160, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 270,160);
    assert( obj1 != NULL );

    JXEngravedRect* obj2 =
        new JXEngravedRect(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 250,65);
    assert( obj2 != NULL );

    mMultiple =
        new JXTextCheckbox("Simultaneous Searching", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,5, 200,20);
    assert( mMultiple != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Maximum Number:", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,12, 115,20);
    assert( obj3 != NULL );

    mMaximum =
        new CInputField<JXIntegerInput>(obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 130,10, 50,20);
    assert( mMaximum != NULL );

    mLoadBalance =
        new JXTextCheckbox("Load Balance Multiple Accounts", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,35, 215,20);
    assert( mLoadBalance != NULL );

    mOpenFirst =
        new JXTextCheckbox("Open First Matching Mailbox", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,90, 195,20);
    assert( mOpenFirst != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 180,125, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,125, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

// end JXLayout
	window->SetTitle("Search Options");
	SetButtons(mOKBtn, mCancelBtn);

	ListenTo(mMultiple);

	SetItems();
}

// Handle buttons
void CSearchOptionsDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mMultiple)
		{
			if (mMultiple->IsChecked())
			{
				mMaximum->Activate();
				mLoadBalance->Activate();
			}
			else
			{
				mMaximum->Deactivate();
				mLoadBalance->Deactivate();
			}
		}
	}

	CDialogDirector::Receive(sender, message);
}

// Set items from prefs
void CSearchOptionsDialog::SetItems()
{
	mMultiple->SetState(JBoolean(CPreferences::sPrefs->mMultipleSearch.GetValue()));
	mMaximum->SetValue(CPreferences::sPrefs->mNumberMultipleSearch.GetValue());
	mLoadBalance->SetState(JBoolean(CPreferences::sPrefs->mLoadBalanceSearch.GetValue()));
	mOpenFirst->SetState(JBoolean(CPreferences::sPrefs->mOpenFirstSearchResult.GetValue()));
}

// Set prefs from items
void CSearchOptionsDialog::GetItems()
{
	CPreferences::sPrefs->mMultipleSearch.SetValue(mMultiple->IsChecked());
	JInteger temp;
	mMaximum->GetValue(&temp);
	CPreferences::sPrefs->mNumberMultipleSearch.SetValue(temp);
	CPreferences::sPrefs->mLoadBalanceSearch.SetValue(mLoadBalance->IsChecked());
	CPreferences::sPrefs->mOpenFirstSearchResult.SetValue(mOpenFirst->IsChecked());
}

// Do dialog
bool CSearchOptionsDialog::PoseDialog()
{
	bool result = false;

	CSearchOptionsDialog* dlog = new CSearchOptionsDialog(JXGetApplication());

	// Test for OK
	if (dlog->DoModal() == kDialogClosed_OK)
	{
		dlog->GetItems();
		result = true;
		dlog->Close();
	}

	return result;
}
