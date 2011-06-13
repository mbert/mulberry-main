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

#include "CPrefsAttachmentsSave.h"

#include "CLocalCommon.h"
#include "CMIMEMap.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsEditMappings.h"
#include "CTextFieldX.h"

#include <LPushButton.h>
#include <LRadioButton.h>

#include <UStandardDialogs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S A T T A C H M E N T S
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAttachmentsSave::CPrefsAttachmentsSave()
{
}

// Constructor from stream
CPrefsAttachmentsSave::CPrefsAttachmentsSave(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsAttachmentsSave::~CPrefsAttachmentsSave()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAttachmentsSave::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get menu and radios
	mViewDoubleClick = (LRadioButton*) FindPaneByID(paneid_ViewDoubleClick);
	mSaveDoubleClick = (LRadioButton*) FindPaneByID(paneid_SaveDoubleClick);

	mAskUserSave = (LRadioButton*) FindPaneByID(paneid_AskUserSave);
	mDefaultFolderSave = (LRadioButton*) FindPaneByID(paneid_DefaultFolderSave);
	mDefaultFolder = (CTextFieldX*) FindPaneByID(paneid_DefaultFolder);
	mChooseDefaultFolder = (LPushButton*) FindPaneByID(paneid_ChooseDefaultFolder);

	mAlwaysOpen = (LRadioButton*) FindPaneByID(paneid_AlwaysOpen);
	mAskOpen = (LRadioButton*) FindPaneByID(paneid_AskOpen);
	mNeverOpen = (LRadioButton*) FindPaneByID(paneid_NeverOpen);
	mIncludeText = (LRadioButton*) FindPaneByID(paneid_IncludeText);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsAttachmentsSaveBtns);
}

// Handle buttons
void CPrefsAttachmentsSave::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_AskUserSave:
			if (*((long*) ioParam))
			{
				mDefaultFolder->Disable();
				mChooseDefaultFolder->Disable();
			}
			Refresh();
			break;

		case msg_DefaultFolderSave:
			if (*((long*) ioParam))
			{
				mDefaultFolder->Enable();
				mChooseDefaultFolder->Enable();
				LCommander::SwitchTarget(mDefaultFolder);
				mDefaultFolder->SelectAll();
			}
			Refresh();
			break;

		case msg_ChooseDefaultFolder:
			DoChooseDefaultFolder();
			break;
		case msg_EditMappings:
			EditMappings();
			break;
	}
}

// Set prefs
void CPrefsAttachmentsSave::SetData(void* data)
{
	// Save ref to prefs
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info
	mViewDoubleClick->SetValue(copyPrefs->mViewDoubleClick.GetValue());
	mSaveDoubleClick->SetValue(!copyPrefs->mViewDoubleClick.GetValue());

	mAskUserSave->SetValue(copyPrefs->mAskDownload.GetValue());
	mDefaultFolderSave->SetValue(!copyPrefs->mAskDownload.GetValue());
	mDefaultFolder->SetText(copyPrefs->mDefaultDownload.GetValue());
	if (copyPrefs->mAskDownload.GetValue())
	{
		mDefaultFolder->Disable();
		mChooseDefaultFolder->Disable();
	}
	else
	{
		mDefaultFolder->Enable();
		mChooseDefaultFolder->Enable();
	}
	mAlwaysOpen->SetValue(copyPrefs->mAppLaunch.GetValue() == eAppLaunchAlways);
	mAskOpen->SetValue(copyPrefs->mAppLaunch.GetValue() == eAppLaunchAsk);
	mNeverOpen->SetValue(copyPrefs->mAppLaunch.GetValue() == eAppLaunchNever);
	mIncludeText->SetValue(copyPrefs->mLaunchText.GetValue());

	mMappings = copyPrefs->mMIMEMappings.GetValue();
	mMapChange = false;
}

// Force update of prefs
void CPrefsAttachmentsSave::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	copyPrefs->mViewDoubleClick.SetValue((mViewDoubleClick->GetValue() == Button_On));

	copyPrefs->mAskDownload.SetValue((mAskUserSave->GetValue() == Button_On));
	copyPrefs->mDefaultDownload.SetValue(mDefaultFolder->GetText());
	if (mAlwaysOpen->GetValue() == Button_On)
		copyPrefs->mAppLaunch.SetValue(eAppLaunchAlways);
	else if (mAskOpen->GetValue() == Button_On)
		copyPrefs->mAppLaunch.SetValue(eAppLaunchAsk);
	else
		copyPrefs->mAppLaunch.SetValue(eAppLaunchNever);
	copyPrefs->mLaunchText.SetValue((mIncludeText->GetValue() == Button_On));

	if (mMapChange)
		copyPrefs->mMIMEMappings.SetValue(mMappings);
}

// Choose default folder
void CPrefsAttachmentsSave::DoChooseDefaultFolder(void)
{
	// Get folder from user
	PPx::FSObject fspec;
	SInt32 dirID;
	if (PP_StandardDialogs::AskChooseFolder(fspec, dirID))
	{
		cdstring temp(fspec.GetPath());
		mDefaultFolder->SetText(temp);
	}
}

// Edit mappings
void CPrefsAttachmentsSave::EditMappings(void)
{
	CMIMEMapVector copy = mMappings;
	if (CPrefsEditMappings::PoseDialog(&copy))
	{
		// Copy changed map
		mMapChange = true;
		mMappings = copy;
	}
}
