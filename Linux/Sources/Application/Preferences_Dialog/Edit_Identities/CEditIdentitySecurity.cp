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


// CEditIdentitySecurity.cpp : implementation file
//

#include "CEditIdentitySecurity.h"

#include "CPreferences.h"
#include "CTextField.h"

#include "TPopupMenu.h"

#include <JXDownRect.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CEditIdentitySecurity dialog

void CEditIdentitySecurity::OnCreate()
{
// begin JXLayout1

    mGroup =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 430,175);
    assert( mGroup != NULL );

    mActive =
        new JXTextCheckbox("Active", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,5, 65,20);
    assert( mActive != NULL );

    mSign =
        new JXTextCheckbox("Sign Messages", mGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,25, 115,20);
    assert( mSign != NULL );

    mEncrypt =
        new JXTextCheckbox("Encrypt Message", mGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,45, 125,20);
    assert( mEncrypt != NULL );

    JXStaticText* obj1 =
        new JXStaticText("Sign with Key:", mGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 40,72, 90,20);
    assert( obj1 != NULL );

    mSignWithPopup =
        new HPopupMenu("",mGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 135,70, 200,25);
    assert( mSignWithPopup != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Key:", mGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 45,107, 35,20);
    assert( obj2 != NULL );

    mSignOther =
        new CTextInputField(mGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,105, 255,20);
    assert( mSignOther != NULL );

// end JXLayout1

	// Set menu items
	mSignWithPopup->SetMenuItems("Default Key %r |"
								"From Address Key %r |"
								"Reply-To Address Key %r |"
								"Sender Address key %r %l |"
								"Other Key %r");
	mSignWithPopup->SetUpdateAction(JXMenu::kDisableNone);
	mSignWithPopup->SetToPopupChoice(kTrue, 1);

	// Start listening
	ListenTo(mActive);
	ListenTo(mSignWithPopup);
}

/////////////////////////////////////////////////////////////////////////////
// CEditIdentitySecurity message handlers

void CEditIdentitySecurity::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mActive)
		{
			OnActive(mActive->IsChecked());
			return;
		}
	}
	else if (message.Is(JXMenu::kItemSelected))
	{
    	if (sender == mSignWithPopup)
    	{
			const JXMenu::ItemSelected* is = 
				dynamic_cast<const JXMenu::ItemSelected*>(&message);			
			OnSignWithPopup(is->GetIndex());
			return;
		}
	}

	CTabPanel::Receive(sender, message);
}

void CEditIdentitySecurity::OnActive(bool active)
{
	if (active)
		mGroup->Activate();
	else
		mGroup->Deactivate();
}

// Set data
void CEditIdentitySecurity::SetData(void* data)
{
	CIdentity* identity = (CIdentity*) data;

	// Copy info
	mActive->SetState(JBoolean(identity->GetUseSecurity()));
	if (!identity->GetUseSecurity())
		mGroup->Deactivate();
	mSign->SetState(JBoolean(identity->GetSign()));
	mEncrypt->SetState(JBoolean(identity->GetEncrypt()));
	mSignWithPopup->SetValue(identity->GetSignWith() + 1);
	if (identity->GetSignWith() != CIdentity::eSignWithOther)
		mSignOther->Deactivate();
	mSignOther->SetText(identity->GetSignOther());
}

// Force update of data
bool CEditIdentitySecurity::UpdateData(void* data)
{
	CIdentity* identity = (CIdentity*) data;

	identity->SetUseSecurity(mActive->IsChecked());
	identity->SetSign(mSign->IsChecked());
	identity->SetEncrypt(mEncrypt->IsChecked());

	identity->SetSignWith(static_cast<CIdentity::ESignWith>(mSignWithPopup->GetValue() - 1), cdstring(mSignOther->GetText()));
	
	return true;
}

void CEditIdentitySecurity::OnSignWithPopup(JIndex nID) 
{
	if (nID - 1 == CIdentity::eSignWithOther)
		mSignOther->Activate();
	else
		mSignOther->Deactivate();
}
