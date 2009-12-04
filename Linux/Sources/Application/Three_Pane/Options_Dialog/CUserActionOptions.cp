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
#include "CUserAction.h"

#include <JXDownRect.h>
#include <JXEngravedRect.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>
#include <JXTextButton.h>

#include <cassert>

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CUserActionOptions::CUserActionOptions(JXContainer* enclosure,
						 const HSizingOption hSizing, 
						 const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h) 
	: JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Default destructor
CUserActionOptions::~CUserActionOptions()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void CUserActionOptions::OnCreate()
{
// begin JXLayout1

    mSelect =
        new JXTextCheckbox("Selection Change", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,0, 160,20);
    assert( mSelect != NULL );

    mSingleClick =
        new JXTextCheckbox("Single Click", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,20, 160,20);
    assert( mSingleClick != NULL );

    mDoubleClick =
        new JXTextCheckbox("Double Click", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,40, 160,20);
    assert( mDoubleClick != NULL );

    JXEngravedRect* obj1 =
        new JXEngravedRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,70, 190,45);
    assert( obj1 != NULL );

    mUseKey =
        new JXTextCheckbox("Type Key", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,60, 85,20);
    assert( mUseKey != NULL );

    JXDownRect* obj2 =
        new JXDownRect(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,12, 100,20);
    assert( obj2 != NULL );

    mKey =
        new JXStaticText("", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 90,18);
    assert( mKey != NULL );
    mKey->SetFontSize(10);

    mChooseBtn =
        new JXTextButton("Choose...", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 115,10, 65,20);
    assert( mChooseBtn != NULL );
    mChooseBtn->SetFontSize(10);

// end JXLayout1

	ListenTo(mChooseBtn);
}

void CUserActionOptions::Receive(JBroadcaster* sender, const Message& message)
{
	if ((sender == mChooseBtn) && message.Is(JXButton::kPushed))
	{
		GetKey();
		return;
	}
}

void CUserActionOptions::SetData(const CUserAction& action)
{
	mSelect->SetState(JBoolean(action.GetSelection()));
	mSingleClick->SetState(JBoolean(action.GetSingleClick()));
	mDoubleClick->SetState(JBoolean(action.GetDoubleClick()));
	
	mUseKey->SetState(JBoolean(action.GetKey() != 0));
	
	mActualKey = action.GetKey();
	mActualMods = action.GetKeyModifiers();
	mKey->SetText(CUserAction::GetKeyDescriptor(mActualKey, mActualMods));
}

void CUserActionOptions::GetData(CUserAction& action)
{
	action.SetSelection(mSelect->IsChecked());
	action.SetSingleClick(mSingleClick->IsChecked());
	action.SetDoubleClick(mDoubleClick->IsChecked());
	
	if (mUseKey->IsChecked())
	{
		action.SetKey(mActualKey);
		action.GetKeyModifiers() = mActualMods;
	}
	else
		action.SetKey(0);
}

void CUserActionOptions::DisableItems()
{
	mSelect->Deactivate();
	mSingleClick->Deactivate();
	mDoubleClick->Deactivate();
	mUseKey->Deactivate();
	mKey->Deactivate();
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
