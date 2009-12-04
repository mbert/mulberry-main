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


// Source for CPrefsLetter class

#include "CPrefsAttachmentsSend.h"

#include "CPreferences.h"
#include "CStringUtils.h"
#include "CTextDisplay.h"

#include <LCheckBox.h>
#include <LCheckBoxGroupBox.h>
#include <LPopupButton.h>
#include <LRadioButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S A T T A C H M E N T S
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAttachmentsSend::CPrefsAttachmentsSend()
{
}

// Constructor from stream
CPrefsAttachmentsSend::CPrefsAttachmentsSend(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsAttachmentsSend::~CPrefsAttachmentsSend()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAttachmentsSend::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get menu and radios
	mDefaultEncoding = (LPopupButton*) FindPaneByID(paneid_EncodingMenu);
	mEncodingAlways = (LRadioButton*) FindPaneByID(paneid_EncodingAlways);
	mEncodingWhenNeeded = (LRadioButton*) FindPaneByID(paneid_EncodingWhenNeeded);
	mCheckDefaultMailClient = (LCheckBox*) FindPaneByID(paneid_CheckDefaultMailClient);
	mWarnMailtoFiles = (LCheckBox*) FindPaneByID(paneid_WarnMailtoFiles);
	mCheckDefaultWebcalClient = (LCheckBox*) FindPaneByID(paneid_CheckDefaultWebcalClient);
	mWarnMissingAttachments = (LCheckBoxGroupBox*) FindPaneByID(paneid_WarnMissingAttachments);
	mMissingAttachmentSubject = (LCheckBox*) FindPaneByID(paneid_MissingAttachmentSubject);
	mMissingAttachmentWords = (CTextDisplay*) FindPaneByID(paneid_MissingAttachmentWords);
}

// Set prefs
void CPrefsAttachmentsSend::SetData(void* data)
{
	// Save ref to prefs
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info
	switch(copyPrefs->mDefault_mode.GetValue())
	{
	case eUUMode:
		mDefaultEncoding->SetValue(menu_PrefsAttachmentsUU);
		break;
	case eBinHex4Mode:
		mDefaultEncoding->SetValue(menu_PrefsAttachmentsBinHex);
		break;
	case eAppleSingleMode:
		mDefaultEncoding->SetValue(menu_PrefsAttachmentsAS);
		break;
	case eAppleDoubleMode:
	default:
		mDefaultEncoding->SetValue(menu_PrefsAttachmentsAD);
		break;
	}

	mEncodingAlways->SetValue(copyPrefs->mDefault_Always.GetValue());
	mEncodingWhenNeeded->SetValue(!copyPrefs->mDefault_Always.GetValue());
	mCheckDefaultMailClient->SetValue(copyPrefs->mCheckDefaultMailClient.GetValue());
	mWarnMailtoFiles->SetValue(copyPrefs->mWarnMailtoFiles.GetValue());
	mCheckDefaultWebcalClient->SetValue(copyPrefs->mCheckDefaultWebcalClient.GetValue());
	mWarnMissingAttachments->SetValue(copyPrefs->mWarnMissingAttachments.GetValue());
	mMissingAttachmentSubject->SetValue(copyPrefs->mMissingAttachmentSubject.GetValue());

	cdstring words;
	for(cdstrvect::const_iterator iter = copyPrefs->mMissingAttachmentWords.GetValue().begin();
					iter != copyPrefs->mMissingAttachmentWords.GetValue().end(); iter++)
	{
		words += *iter;
		words += os_endl;
	}
	mMissingAttachmentWords->SetText(words);
}

// Force update of prefs
void CPrefsAttachmentsSend::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	switch(mDefaultEncoding->GetValue())
	{
	case menu_PrefsAttachmentsUU:
		copyPrefs->mDefault_mode.SetValue(eUUMode);
		break;
	case menu_PrefsAttachmentsBinHex:
		copyPrefs->mDefault_mode.SetValue(eBinHex4Mode);
		break;
	case menu_PrefsAttachmentsAS:
		copyPrefs->mDefault_mode.SetValue(eAppleSingleMode);
		break;
	case menu_PrefsAttachmentsAD:
	default:
		copyPrefs->mDefault_mode.SetValue(eAppleDoubleMode);
		break;
	}
	copyPrefs->mDefault_Always.SetValue((mEncodingAlways->GetValue() == 1));
	copyPrefs->mCheckDefaultMailClient.SetValue(mCheckDefaultMailClient->GetValue() == 1);
	copyPrefs->mWarnMailtoFiles.SetValue(mWarnMailtoFiles->GetValue() == 1);
	copyPrefs->mCheckDefaultWebcalClient.SetValue(mCheckDefaultWebcalClient->GetValue() == 1);
	copyPrefs->mMissingAttachmentSubject.SetValue(mMissingAttachmentSubject->GetValue() == 1);
	copyPrefs->mWarnMissingAttachments.SetValue(mWarnMissingAttachments->GetValue() == 1);

	// Only copy text if dirty
	if (mMissingAttachmentWords->IsDirty())
	{
		// Copy handle to text with null terminator
		cdstring txt;
		mMissingAttachmentWords->GetText(txt);

		char* s = ::strtok(txt.c_str_mod(), CR);
		cdstrvect accumulate;
		while(s)
		{
			cdstring copyStr(s);
			accumulate.push_back(copyStr);

			s = ::strtok(nil, CR);
		}
		copyPrefs->mMissingAttachmentWords.SetValue(accumulate);
	}
}
