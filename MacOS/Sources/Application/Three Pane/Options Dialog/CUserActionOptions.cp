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


// Source for CUserActionOptions class

#include "CUserActionOptions.h"

#include "CKeyChoiceDialog.h"
#include "CStaticText.h"
#include "CUserAction.h"

#include <LCheckBox.h>
#include <LCheckBoxGroupBox.h>
#include <LPushButton.h>

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CUserActionOptions::CUserActionOptions()
{
}

// Constructor from stream
CUserActionOptions::CUserActionOptions(LStream *inStream)
		: LView(inStream)
{
}

// Default destructor
CUserActionOptions::~CUserActionOptions()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void CUserActionOptions::FinishCreateSelf()
{
	// Do inherited
	LView::FinishCreateSelf();

	mSelect = (LCheckBox*) FindPaneByID(paneid_UserActionSelect);
	mSingleClick = (LCheckBox*) FindPaneByID(paneid_UserActionSingleClick);
	mDoubleClick = (LCheckBox*) FindPaneByID(paneid_UserActionDoubleClick);
	mUseKey = (LCheckBoxGroupBox*) FindPaneByID(paneid_UserActionUseKey);
	mKey = (CStaticText*) FindPaneByID(paneid_UserActionKey);
	
	static_cast<LPushButton*>(FindPaneByID(paneid_UserActionKeyChange))->AddListener(this);
}

void CUserActionOptions::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch(inMessage)
	{
	case msg_UserActionKeyChange:
		GetKey();
		break;
	}
}

void CUserActionOptions::SetData(const CUserAction& action)
{
	mSelect->SetValue(action.GetSelection());
	mSingleClick->SetValue(action.GetSingleClick());
	mDoubleClick->SetValue(action.GetDoubleClick());
	
	mUseKey->SetValue(action.GetKey() != 0);
	
	mActualKey = action.GetKey();
	mActualMods = action.GetKeyModifiers();
	mKey->SetText(CUserAction::GetKeyDescriptor(mActualKey, mActualMods));
}

void CUserActionOptions::GetData(CUserAction& action)
{
	action.SetSelection(mSelect->GetValue());
	action.SetSingleClick(mSingleClick->GetValue());
	action.SetDoubleClick(mDoubleClick->GetValue());
	
	if (mUseKey->GetValue())
	{
		action.SetKey(mActualKey);
		action.GetKeyModifiers() = mActualMods;
	}
	else
		action.SetKey(0);
}

void CUserActionOptions::DisableItems()
{
	mSelect->Disable();
	mSingleClick->Disable();
	mDoubleClick->Disable();
	mUseKey->Disable();
	mKey->Disable();
}

void CUserActionOptions::GetKey()
{
	unsigned char key;
	CKeyModifiers mods;
	if (CKeyChoiceDialog::PoseDialog(key, mods))
	{
		mActualKey = key;
		mActualMods = mods;
		mKey->SetText(CUserAction::GetKeyDescriptor(mActualKey, mActualMods));
	}
}