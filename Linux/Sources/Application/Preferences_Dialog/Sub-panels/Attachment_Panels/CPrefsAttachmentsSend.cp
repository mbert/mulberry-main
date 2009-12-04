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


// CPrefsAttachmentsSend.cpp : implementation file
//

#include "CPrefsAttachmentsSend.h"

#include "CPreferences.h"
#include "CTextDisplay.h"

#include "JXSecondaryRadioGroup.h"
#include "TPopupMenu.h"

#include <JXDownRect.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>
#include <JXUpRect.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CPrefsAttachmentsSend property page

// Get details of sub-panes
void CPrefsAttachmentsSend::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Override MIME with:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 125,20);
    assert( obj1 != NULL );

    mEncodingPopup =
        new HPopupMenu("",this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 135,5, 155,25);
    assert( mEncodingPopup != NULL );

    mOverrideGroup =
        new JXSecondaryRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,45, 195,50);
    assert( mOverrideGroup != NULL );

    JXTextRadioButton* obj2 =
        new JXTextRadioButton(1, "Always override MIME", mOverrideGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,3, 160,20);
    assert( obj2 != NULL );

    JXTextRadioButton* obj3 =
        new JXTextRadioButton(2, "Only override when needed", mOverrideGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,23, 185,20);
    assert( obj3 != NULL );

    mCheckDefaultMailClient =
        new JXTextCheckbox("Check for mailto URL Handling on Startup", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,100, 270,20);
    assert( mCheckDefaultMailClient != NULL );

    mWarnMailtoFiles =
        new JXTextCheckbox("Warn when mailto URL Attaches Files", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,120, 270,20);
    assert( mWarnMailtoFiles != NULL );

    mCheckDefaultWebcalClient =
        new JXTextCheckbox("Check for webcal URL Handling on Startup", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,140, 270,20);
    assert( mCheckDefaultWebcalClient != NULL );

    JXDownRect* obj4 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,175, 350,162);
    assert( obj4 != NULL );

    mWarnMissingAttachments =
        new JXTextCheckbox("Warn if Attachments are not Present in a Draft", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,165, 295,20);
    assert( mWarnMissingAttachments != NULL );

    mMissingAttachmentSubject =
        new JXTextCheckbox("Check Subject as well as Text", obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,15, 205,20);
    assert( mMissingAttachmentSubject != NULL );

    JXUpRect* obj5 =
        new JXUpRect(obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,40, 320,25);
    assert( obj5 != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Recognised Words", obj5,
                    JXWidget::kHElastic, JXWidget::kVElastic, 100,2, 115,20);
    assert( obj6 != NULL );

    mMissingAttachmentWords =
        new CTextInputDisplay(obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,65, 320,85);
    assert( mMissingAttachmentWords != NULL );

// end JXLayout1

	cdstring str;
	str.FromResource(IDS_ENCODING_UU);
	mEncodingPopup->AppendItem(str, kFalse, kTrue);
	str.FromResource(IDS_ENCODING_BINHEX4);
	mEncodingPopup->AppendItem(str, kFalse, kTrue);
	str.FromResource(IDS_ENCODING_AS);
	mEncodingPopup->AppendItem(str, kFalse, kTrue);
	str.FromResource(IDS_ENCODING_AD);
	mEncodingPopup->AppendItem(str, kFalse, kTrue);
	mEncodingPopup->SetUpdateAction(JXMenu::kDisableNone);
	mEncodingPopup->SetToPopupChoice(kTrue, 1);
}

// Set up params for DDX
void CPrefsAttachmentsSend::SetData(void* data)
{
	CPreferences* prefs = (CPreferences*) data;

	// Set values
	JIndex index;
	switch(prefs->mDefault_mode.GetValue())
	{
	case eUUMode:
		index = 1;
		break;
	case eBinHex4Mode:
		index = 2;
		break;
	case eAppleSingleMode:
		index = 3;
		break;
	case eAppleDoubleMode:
	default:
		index = 4;
		break;
	}
	mEncodingPopup->SetValue(index);

	mOverrideGroup->SelectItem(prefs->mDefault_Always.GetValue() ? 1 : 2);
	
	mCheckDefaultMailClient->SetState(JBoolean(prefs->mCheckDefaultMailClient.GetValue()));
	mWarnMailtoFiles->SetState(JBoolean(prefs->mWarnMailtoFiles.GetValue()));
	mCheckDefaultWebcalClient->SetState(JBoolean(prefs->mCheckDefaultWebcalClient.GetValue()));
	mWarnMissingAttachments->SetState(JBoolean(prefs->mWarnMissingAttachments.GetValue()));
	mMissingAttachmentSubject->SetState(JBoolean(prefs->mMissingAttachmentSubject.GetValue()));

	cdstring words;
	for(cdstrvect::const_iterator iter = prefs->mMissingAttachmentWords.GetValue().begin();
		iter != prefs->mMissingAttachmentWords.GetValue().end(); iter++)
	{
		words += *iter;
		words += '\n';
	}
	mMissingAttachmentWords->SetText(words);
}

// Get params from DDX
bool CPrefsAttachmentsSend::UpdateData(void* data)
{
	CPreferences* prefs = (CPreferences*) data;

	// Get values
	JIndex index = mEncodingPopup->GetValue();
	switch(index)
	{
	case 1:
		prefs->mDefault_mode.SetValue(eUUMode);
		break;
	case 2:
		prefs->mDefault_mode.SetValue(eBinHex4Mode);
		break;
	case 3:
		prefs->mDefault_mode.SetValue(eAppleSingleMode);
		break;
	case 4:
		prefs->mDefault_mode.SetValue(eAppleDoubleMode);
		break;
	}
	prefs->mDefault_Always.SetValue(mOverrideGroup->GetSelectedItem() == 1);
	
	prefs->mCheckDefaultMailClient.SetValue(mCheckDefaultMailClient->IsChecked());
	prefs->mWarnMailtoFiles.SetValue(mWarnMailtoFiles->IsChecked());
	prefs->mCheckDefaultWebcalClient.SetValue(mCheckDefaultWebcalClient->IsChecked());
	prefs->mWarnMissingAttachments.SetValue(mWarnMissingAttachments->IsChecked());
	prefs->mMissingAttachmentSubject.SetValue(mMissingAttachmentSubject->IsChecked());

	{
		// Tokenise text to get each line
		cdstring txt = mMissingAttachmentWords->GetText();

		char* s = ::strtok(txt.c_str_mod(), "\n");
		cdstrvect accumulate;
		while(s)
		{
			cdstring copyStr(s);
			accumulate.push_back(copyStr);

			s = ::strtok(nil, "\n");
		}
		prefs->mMissingAttachmentWords.SetValue(accumulate);
	}

	return true;
}
