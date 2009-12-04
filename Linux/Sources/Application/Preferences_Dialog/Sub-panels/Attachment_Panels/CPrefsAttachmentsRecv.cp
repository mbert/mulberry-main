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


// CPrefsAttachmentsRecv.cpp : implementation file
//

#include "CPrefsAttachmentsRecv.h"

#include "CIconLoader.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CPrefsEditMappings.h"
#include "CTextField.h"

#include "JXSecondaryRadioGroup.h"
#include "TPopupMenu.h"

#include <JXChooseSaveFile.h>
#include <JXDownRect.h>
#include <JXImageButton.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>
#include <jXGlobals.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CPrefsAttachmentsRecv property page

// Get details of sub-panes
void CPrefsAttachmentsRecv::OnCreate()
{
// begin JXLayout1

    mViewGroup =
        new JXSecondaryRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 360,50);
    assert( mViewGroup != NULL );

    JXStaticText* obj1 =
        new JXStaticText("Double-click Attachment to:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,0, 165,20);
    assert( obj1 != NULL );

    JXTextRadioButton* obj2 =
        new JXTextRadioButton(1, "View it", mViewGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,5, 70,20);
    assert( obj2 != NULL );

    JXTextRadioButton* obj3 =
        new JXTextRadioButton(2, "Save it to Disk", mViewGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,25, 145,20);
    assert( obj3 != NULL );

    mSaveToGroup =
        new JXSecondaryRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,80, 360,115);
    assert( mSaveToGroup != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Save to Disk:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,70, 85,20);
    assert( obj4 != NULL );

    JXTextRadioButton* obj5 =
        new JXTextRadioButton(1, "Ask for Location", mSaveToGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,5, 120,20);
    assert( obj5 != NULL );

    JXDownRect* obj6 =
        new JXDownRect(mSaveToGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,35, 340,75);
    assert( obj6 != NULL );

    JXTextRadioButton* obj7 =
        new JXTextRadioButton(2, "Use Default Location:", mSaveToGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,25, 145,20);
    assert( obj7 != NULL );

    mDefaultDownload =
        new CTextInputField(obj6,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 310,20);
    assert( mDefaultDownload != NULL );

    mChooseDefault =
        new JXTextButton("Choose...", obj6,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,40, 80,25);
    assert( mChooseDefault != NULL );

    mOpenGroup =
        new JXSecondaryRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,215, 360,120);
    assert( mOpenGroup != NULL );

    JXStaticText* obj8 =
        new JXStaticText("Open after Save:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,205, 105,20);
    assert( obj8 != NULL );

    JXTextRadioButton* obj9 =
        new JXTextRadioButton(1, "Always", mOpenGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,5, 80,20);
    assert( obj9 != NULL );

    JXTextRadioButton* obj10 =
        new JXTextRadioButton(2, "Ask", mOpenGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,25, 80,20);
    assert( obj10 != NULL );

    JXTextRadioButton* obj11 =
        new JXTextRadioButton(3, "Never", mOpenGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,45, 80,20);
    assert( obj11 != NULL );

    mLaunchText =
        new JXTextCheckbox("Include Text Attachments", mOpenGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,70, 170,20);
    assert( mLaunchText != NULL );

    mMappingsBtn =
        new JXImageButton(mOpenGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 255,10, 32,32);
    assert( mMappingsBtn != NULL );

    JXStaticText* obj12 =
        new JXStaticText("Launch:", mOpenGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,95, 55,20);
    assert( obj12 != NULL );

    mShellLaunch =
        new CTextInputField(mOpenGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 70,95, 260,20);
    assert( mShellLaunch != NULL );

    JXStaticText* obj13 =
        new JXStaticText("Mappings", mOpenGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 241,43, 65,20);
    assert( obj13 != NULL );

// end JXLayout1

	mMappingsBtn->SetImage(CIconLoader::GetIcon(IDI_EXPLICITMAPPING, mMappingsBtn, 32, 0x00CCCCCC), kFalse);

	// Start listening
	ListenTo(mSaveToGroup);
	ListenTo(mChooseDefault);
	ListenTo(mMappingsBtn);
}

void CPrefsAttachmentsRecv::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		if (sender == mSaveToGroup)
		{
			OnSaveToGroup();
			return;
		}
	}
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mChooseDefault)
		{
			OnChooseDefaultDownload();
			return;
		}
		else if (sender == mMappingsBtn)
		{
			OnExplicitMapping();
			return;
		}
	}
}

// Set up params for DDX
void CPrefsAttachmentsRecv::SetData(void* data)
{
	CPreferences* prefs = (CPreferences*) data;

	// Set values
	mViewGroup->SelectItem(prefs->mViewDoubleClick.GetValue() ? 1 : 2);
	mSaveToGroup->SelectItem(prefs->mAskDownload.GetValue() ? 1 : 2);
	OnSaveToGroup();
	mDefaultDownload->SetText(prefs->mDefaultDownload.GetValue());
	mOpenGroup->SelectItem(prefs->mAppLaunch.GetValue() + 1);
	mLaunchText->SetState(JBoolean(prefs->mLaunchText.GetValue()));
#ifdef NOTYET
	mShellLaunch->SetText(prefs->mShellLaunch.GetValue());
#endif
	
	mMappings = prefs->mMIMEMappings.GetValue();
	mMapChange = false;
}

// Get params from DDX
bool CPrefsAttachmentsRecv::UpdateData(void* data)
{
	CPreferences* prefs = (CPreferences*) data;

	// Get values
	prefs->mViewDoubleClick.SetValue(mViewGroup->GetSelectedItem() == 1);
	prefs->mAskDownload.SetValue(mSaveToGroup->GetSelectedItem() == 1);
	prefs->mDefaultDownload.SetValue(cdstring(mDefaultDownload->GetText()));
	prefs->mAppLaunch.SetValue((EAppLaunch) (mOpenGroup->GetSelectedItem() - 1));
	prefs->mLaunchText.SetValue(mLaunchText->IsChecked());
	if (mMapChange)
		prefs->mMIMEMappings.SetValue(mMappings);
#ifdef NOTYET
	prefs->mShellLaunch.SetValue(cdstring(mShellLaunch->GetText()));
#endif
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsAttachmentsRecv message handlers

void CPrefsAttachmentsRecv::OnSaveToGroup()
{
	if (mSaveToGroup->GetSelectedItem() == 1)
	{
		mDefaultDownload->Deactivate();
		mChooseDefault->Deactivate();
	}
	else
	{
		mDefaultDownload->Activate();
		mChooseDefault->Activate();
	}
}

void CPrefsAttachmentsRecv::OnChooseDefaultDownload()
{
	JString pname;
	if (JXGetChooseSaveFile()->ChooseRWPath("Choose a Download directory:", NULL, mDefaultDownload->GetText(), &pname))
		mDefaultDownload->SetText(pname.GetCString());
}

void CPrefsAttachmentsRecv::OnExplicitMapping()
{
	// Local copy for modification
	CMIMEMapVector change = mMappings;

	// Let DialogHandler process events
	if (CPrefsEditMappings::PoseDialog(&change))
	{
		mMapChange = true;
		mMappings = change;
	}
}
