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

#include <LCheckBox.h>


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsSMTPOptions::CPrefsSMTPOptions()
{
}

// Constructor from stream
CPrefsSMTPOptions::CPrefsSMTPOptions(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsSMTPOptions::~CPrefsSMTPOptions()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsSMTPOptions::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mUseQueue = (LCheckBox*) FindPaneByID(paneid_PrefsSMTPOptionsUseQueue);
	mHoldMessages = (LCheckBox*) FindPaneByID(paneid_PrefsSMTPOptionsHoldMessages);
	
	// Disable certain items
	if (CAdminLock::sAdminLock.mNoSMTPQueues)
	{
		mUseQueue->Disable();
		mHoldMessages->Disable();
	}

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsSMTPOptions);
}

// Handle buttons
void CPrefsSMTPOptions::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_PrefsSMTPOptionsUseQueue:
		if (*((long*) ioParam))
			mHoldMessages->Enable();
		else
			mHoldMessages->Disable();
		break;
	}
}

// Set prefs
void CPrefsSMTPOptions::SetData(void* data)
{
	CSMTPAccount* acct = (CSMTPAccount*) data;

	// Copy info
	mUseQueue->SetValue(acct->GetUseQueue());
	mHoldMessages->SetValue(acct->GetHoldMessages());
	if (!acct->GetUseQueue())
		mHoldMessages->Disable();
}

// Force update of prefs
void CPrefsSMTPOptions::UpdateData(void* data)
{
	CSMTPAccount* acct = (CSMTPAccount*) data;

	acct->SetUseQueue(mUseQueue->GetValue()==1);
	acct->SetHoldMessages(mHoldMessages->GetValue()==1);
}
