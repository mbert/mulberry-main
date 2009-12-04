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


// Source for CPrefsMessageOptions class

#include "CPrefsMessageOptions.h"

#include "CAdminLock.h"
#include "CPreferences.h"

#include "JXSecondaryRadioGroup.h"
#include <JXStaticText.h>
#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsMessageOptions::OnCreate()
{
// begin JXLayout1

    mShowHeader =
        new JXTextCheckbox("Show Message Header in Window", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,5, 239,20);
    assert( mShowHeader != NULL );

    mShowStyled =
        new JXTextCheckbox("Show Styled Text Alternatives", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,25, 198,20);
    assert( mShowStyled != NULL );

    mSaveHeader =
        new JXTextCheckbox("Put Message Header in File when Saving", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,45, 265,20);
    assert( mSaveHeader != NULL );

    mDeleteAfterCopy =
        new JXTextCheckbox("Delete Message after Copy", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,65, 185,20);
    assert( mDeleteAfterCopy != NULL );

    mOpenDeleted =
        new JXTextCheckbox("Allow Deleted Messages to be Opened", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,85, 255,20);
    assert( mOpenDeleted != NULL );

    mCloseDeleted =
        new JXTextCheckbox("Close Message when Deleted", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,105, 255,20);
    assert( mCloseDeleted != NULL );

    mOpenReuse =
        new JXTextCheckbox("Reuse open Message Window", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,125, 200,20);
    assert( mOpenReuse != NULL );

    mQuoteSelection =
        new JXTextCheckbox("Quote Selection Only", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,145, 160,20);
    assert( mQuoteSelection != NULL );

    mAlwaysQuote =
        new JXTextCheckbox("Quote whole Message if no Selection", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,165, 240,20);
    assert( mAlwaysQuote != NULL );

    mAutoDigest =
        new JXTextCheckbox("Open First Digest Message", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,185, 185,20);
    assert( mAutoDigest != NULL );

    mExpandHeader =
        new JXTextCheckbox("Expand Header for Multiple Addressess", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,205, 255,20);
    assert( mExpandHeader != NULL );

    mExpandParts =
        new JXTextCheckbox("Expand Parts for Attachments", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,225, 195,20);
    assert( mExpandParts != NULL );

    mMDNGroup =
        new JXSecondaryRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,255, 360,75);
    assert( mMDNGroup != NULL );

    JXStaticText* obj1 =
        new JXStaticText("Read Receipt Requests:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,245, 145,20);
    assert( obj1 != NULL );

    JXTextRadioButton* obj2 =
        new JXTextRadioButton(1, "Always Send Read Receipt", mMDNGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 200,20);
    assert( obj2 != NULL );

    JXTextRadioButton* obj3 =
        new JXTextRadioButton(2, "Never Send Read Receipt", mMDNGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,30, 200,20);
    assert( obj3 != NULL );

    JXTextRadioButton* obj4 =
        new JXTextRadioButton(3, "Prompt to Send Read Receipt", mMDNGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,50, 200,20);
    assert( obj4 != NULL );

// end JXLayout1
}

// Set prefs
void CPrefsMessageOptions::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	mShowHeader->SetState(JBoolean(copyPrefs->showMessageHeader.GetValue()));
	mShowStyled->SetState(JBoolean(copyPrefs->showStyled.GetValue()));
	mSaveHeader->SetState(JBoolean(copyPrefs->saveMessageHeader.GetValue()));
	mDeleteAfterCopy->SetState(JBoolean(copyPrefs->deleteAfterCopy.GetValue()));
	mOpenDeleted->SetState(JBoolean(copyPrefs->mOpenDeleted.GetValue()));
	mCloseDeleted->SetState(JBoolean(copyPrefs->mCloseDeleted.GetValue()));
	mOpenReuse->SetState(JBoolean(copyPrefs->mOpenReuse.GetValue()));
	mQuoteSelection->SetState(JBoolean(copyPrefs->mQuoteSelection.GetValue()));
	mAlwaysQuote->SetState(JBoolean(copyPrefs->mAlwaysQuote.GetValue()));
	mAutoDigest->SetState(JBoolean(copyPrefs->mAutoDigest.GetValue()));
	mExpandHeader->SetState(JBoolean(copyPrefs->mExpandHeader.GetValue()));
	mExpandParts->SetState(JBoolean(copyPrefs->mExpandParts.GetValue()));
	mMDNGroup->SelectItem(copyPrefs->mMDNOptions.GetValue().GetValue() + 1);
	
	// Admin lock
	if (CAdminLock::sAdminLock.mLockMDN)
	{
		mMDNGroup->SelectItem(eMDNAlwaysSend + 1);
		
		mMDNGroup->Deactivate();
	}
	else
		mMDNGroup->SelectItem(copyPrefs->mMDNOptions.GetValue().GetValue() + 1);
}

// Force update of prefs
bool CPrefsMessageOptions::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	copyPrefs->showMessageHeader.SetValue(mShowHeader->IsChecked());
	copyPrefs->showStyled.SetValue(mShowStyled->IsChecked());
	copyPrefs->saveMessageHeader.SetValue(mSaveHeader->IsChecked());
	copyPrefs->deleteAfterCopy.SetValue(mDeleteAfterCopy->IsChecked());
	copyPrefs->mOpenDeleted.SetValue(mOpenDeleted->IsChecked());
	copyPrefs->mCloseDeleted.SetValue(mCloseDeleted->IsChecked());
	copyPrefs->mOpenReuse.SetValue(mOpenReuse->IsChecked());
	copyPrefs->mQuoteSelection.SetValue(mQuoteSelection->IsChecked());
	copyPrefs->mAlwaysQuote.SetValue(mAlwaysQuote->IsChecked());
	copyPrefs->mAutoDigest.SetValue(mAutoDigest->IsChecked());
	copyPrefs->mExpandHeader.SetValue(mExpandHeader->IsChecked());
	copyPrefs->mExpandParts.SetValue(mExpandParts->IsChecked());
	
	// Admin lock
	if (CAdminLock::sAdminLock.mLockMDN)
		copyPrefs->mMDNOptions.SetValue(eMDNAlwaysSend);
	else
		copyPrefs->mMDNOptions.SetValue(static_cast<EMDNOptions>(mMDNGroup->GetSelectedItem() - 1));
	
	return true;
}
