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


// CEditIdentityDSN.cpp : implementation file
//

#include "CEditIdentityDSN.h"

#include "CIdentity.h"

#include "JXSecondaryRadioGroup.h"

#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CEditIdentityDSN dialog

void CEditIdentityDSN::OnCreate()
{
// begin JXLayout1

    mFullGroup =
        new JXSecondaryRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 410,125);
    assert( mFullGroup != NULL );

    mActive =
        new JXTextCheckbox("Active", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,5, 65,20);
    assert( mActive != NULL );

    mUseDSN =
        new JXTextCheckbox("Request DSN", mFullGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,15, 105,20);
    assert( mUseDSN != NULL );

    mSuccess =
        new JXTextCheckbox("Success Notification", mFullGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,45, 145,20);
    assert( mSuccess != NULL );

    mFailure =
        new JXTextCheckbox("Failure Notification", mFullGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,65, 145,20);
    assert( mFailure != NULL );

    mDelay =
        new JXTextCheckbox("Delay Notification", mFullGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,85, 145,20);
    assert( mDelay != NULL );

    JXTextRadioButton* obj1 =
        new JXTextRadioButton(1, "Return Whole Message", mFullGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 230,45, 160,20);
    assert( obj1 != NULL );

    JXTextRadioButton* obj2 =
        new JXTextRadioButton(2, "Return Headers Only", mFullGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 230,65, 150,20);
    assert( obj2 != NULL );

// end JXLayout1

	// Start listening
	ListenTo(mActive);
}

/////////////////////////////////////////////////////////////////////////////
// CEditIdentityDSN message handlers

void CEditIdentityDSN::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mActive)
		{
			// Active/deactivate panel
			EnablePanel(mActive->IsChecked());
			return;
		}
	}

	CTabPanel::Receive(sender, message);
}

// Set data
void CEditIdentityDSN::SetData(void* data)
{
	CIdentity* identity = (CIdentity*) data;

	mActive->SetState(JBoolean(identity->GetUseDSN()));
	mUseDSN->SetState(JBoolean(identity->GetDSN().GetRequest()));
	mSuccess->SetState(JBoolean(identity->GetDSN().GetSuccess()));
	mFailure->SetState(JBoolean(identity->GetDSN().GetFailure()));
	mDelay->SetState(JBoolean(identity->GetDSN().GetDelay()));
	mFullGroup->SelectItem(identity->GetDSN().GetFull() ? 1 : 2);
	
	EnablePanel(mActive->IsChecked());
}

// Force update of data
bool CEditIdentityDSN::UpdateData(void* data)
{
	CIdentity* identity = (CIdentity*) data;

	identity->SetUseDSN(mActive->IsChecked());
	identity->SetDSN().SetRequest(mUseDSN->IsChecked());
	identity->SetDSN().SetSuccess(mSuccess->IsChecked());
	identity->SetDSN().SetFailure(mFailure->IsChecked());
	identity->SetDSN().SetDelay(mDelay->IsChecked());
	identity->SetDSN().SetFull(mFullGroup->GetSelectedItem() == 1);
	
	return true;
}

void CEditIdentityDSN::EnablePanel(bool enable) 
{
	if (enable)
		mFullGroup->Activate();
	else
		mFullGroup->Deactivate();
}
