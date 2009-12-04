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


// Source for CPrefsAccountPOP3 class

#include "CPrefsAccountPOP3.h"

#include "CMailAccount.h"

#include <JXDownRect.h>
#include <JXIntegerInput.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>
#include "CInputField.h"

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAccountPOP3::OnCreate()
{
// begin JXLayout1

    JXDownRect* obj1 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,20, 340,77);
    assert( obj1 != NULL );

    mLeaveOnServer =
        new JXTextCheckbox("Leave Email on Server", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,10, 155,20);
    assert( mLeaveOnServer != NULL );

    mDeleteAfter =
        new JXTextCheckbox("Delete After:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,20, 95,20);
    assert( mDeleteAfter != NULL );

    mDeleteDays =
        new CInputField<JXIntegerInput>(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,20, 40,20);
    assert( mDeleteDays != NULL );

    JXStaticText* obj2 =
        new JXStaticText("days", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 170,22, 40,20);
    assert( obj2 != NULL );

    mDeleteExpunged =
        new JXTextCheckbox("Delete after Local Expunge", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,45, 180,20);
    assert( mDeleteExpunged != NULL );

    mUseMaxSize =
        new JXTextCheckbox("Skip Messages Bigger than:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,110, 185,20);
    assert( mUseMaxSize != NULL );

    mMaxSize =
        new CInputField<JXIntegerInput>(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 200,111, 40,20);
    assert( mMaxSize != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Kb", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 245,113, 40,20);
    assert( obj3 != NULL );

    mUseAPOP =
        new JXTextCheckbox("Use APOP when Available", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,145, 180,20);
    assert( mUseAPOP != NULL );

// end JXLayout1
	
	// Start listening
	ListenTo(mLeaveOnServer);
	ListenTo(mDeleteAfter);
	ListenTo(mUseMaxSize);
}

void CPrefsAccountPOP3::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mLeaveOnServer)
			SetLeaveOnServer(mLeaveOnServer->IsChecked());
		else if (sender == mDeleteAfter)
		{
			if (mDeleteAfter->IsChecked())
				mDeleteDays->Activate();
			else
				mDeleteDays->Deactivate();
			return;
		}
		else if (sender == mUseMaxSize)
		{
			if (mUseMaxSize->IsChecked())
				mMaxSize->Activate();
			else
				mMaxSize->Deactivate();
			return;
		}
	}

	CTabPanel::Receive(sender, message);
}

// Set prefs
void CPrefsAccountPOP3::SetData(void* data)
{
	CMailAccount* account = (CMailAccount*) data;

	mLeaveOnServer->SetState(JBoolean(account->GetLeaveOnServer()));
	mDeleteAfter->SetState(JBoolean(account->GetDoTimedDelete()));
	mDeleteDays->SetValue(account->GetDeleteAfter());
	mDeleteExpunged->SetState(JBoolean(account->GetDeleteExpunged()));
	mUseMaxSize->SetState(JBoolean(account->GetUseMaxSize()));
	mMaxSize->SetValue(account->GetMaxSize());
	mUseAPOP->SetState(JBoolean(account->GetUseAPOP()));

	if (!account->GetDoTimedDelete())
		mDeleteDays->Deactivate();
	if (!account->GetUseMaxSize())
		mMaxSize->Deactivate();
	SetLeaveOnServer(account->GetLeaveOnServer());
}

// Force update of prefs
bool CPrefsAccountPOP3::UpdateData(void* data)
{
	CMailAccount* account = (CMailAccount*) data;

	// Copy info from panel into prefs
	account->SetLeaveOnServer(mLeaveOnServer->IsChecked());
	account->SetDoTimedDelete(mDeleteAfter->IsChecked());
	JInteger value;
	mDeleteDays->GetValue(&value);
	account->SetDeleteAfter(value);
	account->SetDeleteExpunged(mDeleteExpunged->IsChecked());
	account->SetUseMaxSize(mUseMaxSize->IsChecked());
	mMaxSize->GetValue(&value);
	account->SetMaxSize(value);
	account->SetUseAPOP(mUseAPOP->IsChecked());
	
	return true;
}

void CPrefsAccountPOP3::SetLeaveOnServer(bool on)
{
	if (on)
	{
		mDeleteAfter->Activate();
		if (mDeleteAfter->IsChecked())
			mDeleteDays->Activate();
		mDeleteExpunged->Activate();
	}
	else
	{
		mDeleteAfter->Deactivate();
		mDeleteDays->Deactivate();
		mDeleteExpunged->Deactivate();
	}
}
