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
#include "CTextFieldX.h"

#include <LCheckBoxGroupBox.h>
#include <LCheckBox.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAccountPOP3::CPrefsAccountPOP3()
{
}

// Constructor from stream
CPrefsAccountPOP3::CPrefsAccountPOP3(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsAccountPOP3::~CPrefsAccountPOP3()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAccountPOP3::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mLeaveOnServer = (LCheckBoxGroupBox*) FindPaneByID(paneid_AccountPOP3LeaveOnServer);
	mDeleteAfter = (LCheckBox*) FindPaneByID(paneid_AccountPOP3DeleteAfter);
	mDeleteDays = (CTextFieldX*) FindPaneByID(paneid_AccountPOP3DeleteDays);
	mDeleteExpunged = (LCheckBox*) FindPaneByID(paneid_AccountPOP3DeleteExpunged);
	mUseMaxSize = (LCheckBox*) FindPaneByID(paneid_AccountPOP3UseMaxSize);
	mMaxSize = (CTextFieldX*) FindPaneByID(paneid_AccountPOP3MaxSize);
	mUseAPOP = (LCheckBox*) FindPaneByID(paneid_AccountPOP3UseAPOP);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsAccountPOP3Btns);
}

// Handle buttons
void CPrefsAccountPOP3::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_AccountPOP3LeaveOnServer:
			SetLeaveOnServer(*((long*) ioParam));
			break;
		case msg_AccountPOP3DeleteAfter:
			if (*((long*) ioParam))
				mDeleteDays->Enable();
			else
				mDeleteDays->Disable();
			break;
		case msg_AccountPOP3UseMaxSize:
			if (*((long*) ioParam))
				mMaxSize->Enable();
			else
				mMaxSize->Disable();
			break;
	}
}

// Set prefs
void CPrefsAccountPOP3::SetData(void* data)
{
	CMailAccount* account = (CMailAccount*) data;

	mLeaveOnServer->SetValue(account->GetLeaveOnServer());
	mDeleteAfter->SetValue(account->GetDoTimedDelete());
	mDeleteDays->SetNumber(account->GetDeleteAfter());
	mDeleteExpunged->SetValue(account->GetDeleteExpunged());
	mUseMaxSize->SetValue(account->GetUseMaxSize());
	mMaxSize->SetNumber(account->GetMaxSize());
	mUseAPOP->SetValue(account->GetUseAPOP());

	if (!account->GetDoTimedDelete())
		mDeleteDays->Disable();
	if (!account->GetUseMaxSize())
		mMaxSize->Disable();
	SetLeaveOnServer(account->GetLeaveOnServer());
}

// Force update of prefs
void CPrefsAccountPOP3::UpdateData(void* data)
{
	CMailAccount* account = (CMailAccount*) data;

	// Copy info from panel into prefs
	account->SetLeaveOnServer(mLeaveOnServer->GetValue());
	account->SetDoTimedDelete(mDeleteAfter->GetValue());
	account->SetDeleteAfter(mDeleteDays->GetNumber());
	account->SetDeleteExpunged(mDeleteExpunged->GetValue());
	account->SetUseMaxSize(mUseMaxSize->GetValue());
	account->SetMaxSize(mMaxSize->GetNumber());
	account->SetUseAPOP(mUseAPOP->GetValue());
}

void CPrefsAccountPOP3::SetLeaveOnServer(bool on)
{
	if (on)
	{
		mDeleteAfter->Enable();
		if (mDeleteAfter->GetValue())
			mDeleteDays->Enable();
		else
			mDeleteDays->Disable();
		mDeleteExpunged->Enable();
	}
	else
	{
		mDeleteAfter->Disable();
		mDeleteDays->Disable();
		mDeleteExpunged->Disable();
	}
}
