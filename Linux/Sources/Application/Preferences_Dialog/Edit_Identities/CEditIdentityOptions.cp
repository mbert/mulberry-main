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


// CEditIdentityOptions.cpp : implementation file
//

#include "CEditIdentityOptions.h"

#include "CAdminLock.h"
#include "CIconLoader.h"
#include "CIdentity.h"
#include "CMailboxPopup.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CPrefsEditHeadFoot.h"
#include "CTextField.h"

#include "JXSecondaryRadioGroup.h"

#include <JXDownRect.h>
#include <JXImageButton.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CEditIdentityOptions dialog

// Get details of sub-panes
void CEditIdentityOptions::OnCreate()
{
// begin JXLayout1

    mCopyToGroup =
        new JXSecondaryRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 430,85);
    assert( mCopyToGroup != NULL );

    mCopyToActive =
        new JXTextCheckbox("Active", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,5, 65,20);
    assert( mCopyToActive != NULL );

    JXTextRadioButton* obj1 =
        new JXTextRadioButton(1, "Copy To:", mCopyToGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,15, 80,20);
    assert( obj1 != NULL );

    JXTextRadioButton* obj2 =
        new JXTextRadioButton(2, "No Copy To", mCopyToGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,35, 95,20);
    assert( obj2 != NULL );

    JXTextRadioButton* obj3 =
        new JXTextRadioButton(3, "Choose Copy To", mCopyToGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,55, 125,20);
    assert( obj3 != NULL );

    mCopyTo =
        new CTextInputField(mCopyToGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 100,16, 285,20);
    assert( mCopyTo != NULL );

    mAlsoCopyTo =
        new JXTextCheckbox("Also Copy Original Message", mCopyToGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 200,55, 190,20);
    assert( mAlsoCopyTo != NULL );

    JXDownRect* obj4 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,120, 205,70);
    assert( obj4 != NULL );

    mHeaderActive =
        new JXTextCheckbox("Active", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,110, 65,20);
    assert( mHeaderActive != NULL );

    mHeaderBtn =
        new JXImageButton(obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,15, 32,32);
    assert( mHeaderBtn != NULL );

    JXStaticText* obj5 =
        new JXStaticText("X-Header Lines", obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 54,47, 100,20);
    assert( obj5 != NULL );

    JXDownRect* obj6 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,120, 205,70);
    assert( obj6 != NULL );

    mFooterActive =
        new JXTextCheckbox("Active", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 240,110, 65,20);
    assert( mFooterActive != NULL );

    mSignatureBtn =
        new JXImageButton(obj6,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,15, 32,32);
    assert( mSignatureBtn != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Signature", obj6,
                    JXWidget::kHElastic, JXWidget::kVElastic, 74,47, 60,20);
    assert( obj7 != NULL );

    mCopyToPopup =
        new CMailboxPopupButton(false, mCopyToGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 390,18, 30,16);
    assert( mCopyToPopup != NULL );

// end JXLayout1

	// Init controls
	mCopyToPopup->SetDefault();
	if (!CMulberryApp::sApp->LoadedPrefs())
		mCopyToPopup->Deactivate();

	mHeaderBtn->SetImage(CIconLoader::GetIcon(IDI_XHEADERLINES, mHeaderBtn, 32, 0x00CCCCCC), kFalse);
	if (!CAdminLock::sAdminLock.mAllowXHeaders)
		mHeaderBtn->Deactivate();
	mSignatureBtn->SetImage(CIconLoader::GetIcon(IDI_SIGNATURE, mSignatureBtn, 32, 0x00CCCCCC), kFalse);

	// Start listening
	ListenTo(mCopyToActive);
	ListenTo(mCopyToGroup);
	ListenTo(mCopyToPopup);
	ListenTo(mHeaderActive);
	ListenTo(mHeaderBtn);
	ListenTo(mFooterActive);
	ListenTo(mSignatureBtn);
}

void CEditIdentityOptions::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		if (sender == mCopyToGroup)
		{
			OnCopyToGroup(index);
			return;
		}
	}
	else if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mCopyToActive)
		{
			OnActiveCopy(mCopyToActive->IsChecked());
			return;
		}
		else if (sender == mHeaderActive)
		{
			OnActiveHeader(mHeaderActive->IsChecked());
			return;
		}
		else if (sender == mFooterActive)
		{
			OnActiveSignature(mFooterActive->IsChecked());
			return;
		}
	}
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mHeaderBtn)
		{
			OnHeaderBtn();
			return;
		}
		else if (sender == mSignatureBtn)
		{
			OnSignatureBtn();
			return;
		}
	}
	else if(message.Is(JXMenu::kItemSelected))
	{
    	if (sender == mCopyToPopup)
    	{
			const JXMenu::ItemSelected* is = 
				dynamic_cast<const JXMenu::ItemSelected*>(&message);			
			OnCopyToPopup(is->GetIndex());
			return;
		}
	}

	CTabPanel::Receive(sender, message);
}

/////////////////////////////////////////////////////////////////////////////
// CEditIdentityOptions message handlers

// Set data
void CEditIdentityOptions::SetData(void* data)
{
	CIdentity* id = (CIdentity*) data;

	// Copy info
	mCopyToActive->SetState(JBoolean(id->UseCopyTo()));
	OnActiveCopy(id->UseCopyTo());
	mCopyTo->SetText(id->GetCopyTo());
	if (id->GetCopyToNone())
		mCopyToGroup->SelectItem(2);
	else if(id->GetCopyToChoose())
		mCopyToGroup->SelectItem(3);
	else
		mCopyToGroup->SelectItem(1);

	mAlsoCopyTo->SetState(JBoolean(id->GetCopyReplied()));

	mHeader = id->GetHeader();
	mHeaderActive->SetState(JBoolean(id->UseHeader() && CAdminLock::sAdminLock.mAllowXHeaders));
	OnActiveHeader(id->UseHeader() && CAdminLock::sAdminLock.mAllowXHeaders);

	mFooter = id->GetSignatureRaw();
	mFooterActive->SetState(JBoolean(id->UseSignature()));
	OnActiveSignature(id->UseSignature());
}

// Force update of data
bool CEditIdentityOptions::UpdateData(void* data)
{
	CIdentity* id = (CIdentity*) data;

	id->SetCopyTo(cdstring(mCopyTo->GetText()), mCopyToActive->IsChecked());
	id->SetCopyToNone(mCopyToGroup->GetSelectedItem() == 2);
	id->SetCopyToChoose(mCopyToGroup->GetSelectedItem() == 3);
	id->SetCopyReplied(mAlsoCopyTo->IsChecked());

	if (!CAdminLock::sAdminLock.mAllowXHeaders)
		mHeader = "";
	id->SetHeader(mHeader, mHeaderActive->IsChecked() && CAdminLock::sAdminLock.mAllowXHeaders);

	id->SetSignature(mFooter, mFooterActive->IsChecked());
	
	return true;
}

void CEditIdentityOptions::OnActiveCopy(bool active)
{
	if (active)
		mCopyToGroup->Activate();
	else
		mCopyToGroup->Deactivate();
}

void CEditIdentityOptions::OnActiveHeader(bool active)
{
	if (active)
		mHeaderBtn->Activate();
	else
		mHeaderBtn->Deactivate();
}

void CEditIdentityOptions::OnActiveSignature(bool active)
{
	if (active)
		mSignatureBtn->Activate();
	else
		mSignatureBtn->Deactivate();
}

// Change control states
void CEditIdentityOptions::OnCopyToGroup(JIndex index)
{
	switch(index)
	{
	case 1:
		mCopyTo->Activate();
		if (CMulberryApp::sApp->LoadedPrefs())
			mCopyToPopup->Activate();
		break;
	default:
		mCopyTo->Deactivate();
		mCopyToPopup->Deactivate();
	}		
}

void CEditIdentityOptions::OnCopyToPopup(JIndex nID) 
{
	if (nID == CCopyToMenu::cPopupCopyNone)
		mCopyToGroup->SelectItem(2);
	else if (nID == CCopyToMenu::cPopupCopyChoose)
		mCopyToGroup->SelectItem(3);
	else
	{
		// Must set/reset control value to ensure selected mailbox is returned but
		// popup does not display check mark
		cdstring mbox_name;
		if (mCopyToPopup->GetSelectedMboxName(mbox_name, true))
			mCopyTo->SetText(mbox_name);
	}
}

void CEditIdentityOptions::OnHeaderBtn() 
{
	cdstring title = "Set X-Headers";
	CPrefsEditHeadFoot::PoseDialog(mHeader, title, mCopyPrefs, false);
}

void CEditIdentityOptions::OnSignatureBtn() 
{
	cdstring title = "Set Signature";
	CPrefsEditHeadFoot::PoseDialog(mFooter, title, mCopyPrefs, true);
}
