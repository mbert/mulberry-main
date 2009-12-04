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
#include "CUnicodeUtils.h"
#include "CUserAction.h"

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CUserActionOptions::CUserActionOptions()
{
}

// Default destructor
CUserActionOptions::~CUserActionOptions()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CUserActionOptions::SetData(const CUserAction& action)
{
	mSelect = action.GetSelection();
	mSingleClick = action.GetSingleClick();
	mDoubleClick = action.GetDoubleClick();
	
	mUseKey = (action.GetKey() != 0);
	
	mActualKey = action.GetKey();
	mActualMods = action.GetKeyModifiers();
	mKey = CUserAction::GetKeyDescriptor(mActualKey, mActualMods);
}

void CUserActionOptions::GetData(CUserAction& action)
{
	action.SetSelection(mSelect);
	action.SetSingleClick(mSingleClick);
	action.SetDoubleClick(mDoubleClick);
	
	if (mUseKey)
	{
		action.SetKey(mActualKey);
		action.GetKeyModifiers() = mActualMods;
	}
	else
		action.SetKey(0);
}

void CUserActionOptions::GetKey()
{
	unsigned char key;
	CKeyModifiers mods;
	if (CKeyChoiceDialog::PoseDialog(key, mods))
	{
		mActualKey = key;
		mActualMods = mods;
		CUnicodeUtils::SetWindowTextUTF8(&mKeyCtrl, CUserAction::GetKeyDescriptor(mActualKey, mActualMods));
	}
}