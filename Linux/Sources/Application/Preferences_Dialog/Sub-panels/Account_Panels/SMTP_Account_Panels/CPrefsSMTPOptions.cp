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


// Source for CPrefsSMTPOptions class

#include "CPrefsSMTPOptions.h"

#include "CAdminLock.h"
#include "CSMTPAccount.h"

#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsSMTPOptions::OnCreate()
{
// begin JXLayout1

    mUseQueue =
        new JXTextCheckbox("Use Local Queues for Sending", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 205,20);
    assert( mUseQueue != NULL );

    mHoldMessages =
        new JXTextCheckbox("Hold Messages when Added to Queue", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,30, 250,20);
    assert( mHoldMessages != NULL );

// end JXLayout1

	// Disable certain items
	if (CAdminLock::sAdminLock.mNoSMTPQueues)
	{
		mUseQueue->Deactivate();
		mHoldMessages->Deactivate();
	}

	// Start listening
	ListenTo(mUseQueue);
}

// Handle buttons
void CPrefsSMTPOptions::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mUseQueue)
		{
			if (mUseQueue->IsChecked())
				mHoldMessages->Activate();
			else
				mHoldMessages->Deactivate();
			return;
		}
	}

	CTabPanel::Receive(sender, message);
}

// Set prefs
void CPrefsSMTPOptions::SetData(void* data)
{
	CSMTPAccount* acct = (CSMTPAccount*) data;

	// Copy info
	mUseQueue->SetState(JBoolean(acct->GetUseQueue()));
	mHoldMessages->SetState(JBoolean(acct->GetHoldMessages()));
	if (!acct->GetUseQueue())
		mHoldMessages->Deactivate();
}

// Force update of prefs
bool CPrefsSMTPOptions::UpdateData(void* data)
{
	CSMTPAccount* acct = (CSMTPAccount*) data;

	acct->SetUseQueue(mUseQueue->IsChecked());
	acct->SetHoldMessages(mHoldMessages->IsChecked());
	
	return true;
}
