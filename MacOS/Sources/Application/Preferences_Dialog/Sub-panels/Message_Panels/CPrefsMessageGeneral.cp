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


// Source for CPrefsMessageGeneral class

#include "CPrefsMessageGeneral.h"

#include "CPrefsEditCaption.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>
#include <LPushButton.h>
#include <LRadioButton.h>

#include <UStandardDialogs.h>

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S M E S S A G E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsMessageGeneral::CPrefsMessageGeneral()
{
}

// Constructor from stream
CPrefsMessageGeneral::CPrefsMessageGeneral(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsMessageGeneral::~CPrefsMessageGeneral()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsMessageGeneral::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Set backgrounds
	mSaveCreator = (CTextFieldX*) FindPaneByID(paneid_SaveCreator);

	mChooseCreator = (LPushButton*) FindPaneByID(paneid_ChooseCreator);

	mNoLimit = (LRadioButton*) FindPaneByID(paneid_NoLimit);
	mSizeLimit = (LRadioButton*) FindPaneByID(paneid_SizeLimit);

	mWarnMessageSize = (CTextFieldX*) FindPaneByID(paneid_WarnMessageSize);

	mOptionKeyDown = (LRadioButton*) FindPaneByID(paneid_OptionKeyDown);
	mOptionKeyUp = (LRadioButton*) FindPaneByID(paneid_OptionKeyUp);

	mForwardChoice = (LRadioButton*) FindPaneByID(paneid_ForwardChoice);
	mForwardOptions = (LRadioButton*) FindPaneByID(paneid_ForwardOptions);

	mForwardQuoteOriginal = (LCheckBox*) FindPaneByID(paneid_ForwardQuoteOriginal);
	mForwardHeaders = (LCheckBox*) FindPaneByID(paneid_ForwardHeaders);
	mForwardAttachment = (LCheckBox*) FindPaneByID(paneid_ForwardAttachment);
	mForwardRFC822 = (LCheckBox*) FindPaneByID(paneid_ForwardRFC822);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsMessageGeneralBtns);
}

// Toggle display of IC items
void CPrefsMessageGeneral::ToggleICDisplay(bool IC_on)
{
#if 0
	if (mCopyPrefs->mUse_IC)
	{
		mSaveCreator->Disable();
		mChooseCreator->Disable();
	}
	else
	{
		mSaveCreator->Enable();
		mChooseCreator->Enable();
	}
#endif
}

// Handle buttons
void CPrefsMessageGeneral::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_ChooseCreator:
		DoChooseCreator();
		break;

	case msg_NoLimit:
		if (*((long*) ioParam))
		{
			mWarnMessageSize->Disable();
			Refresh();
		}
		break;

	case msg_SizeLimit:
		if (*((long*) ioParam))
		{
			mWarnMessageSize->Enable();
			LCommander::SwitchTarget(mWarnMessageSize);
			mWarnMessageSize->SelectAll();
			Refresh();
		}
		break;

	case msg_EditPrintCaption:
		EditCaption(&mCopyPrefs->mHeaderCaption,
					&mCopyPrefs->mLtrHeaderCaption,
					&mCopyPrefs->mFooterCaption,
					&mCopyPrefs->mLtrFooterCaption,
					NULL,
					&mCopyPrefs->mHeaderBox,
					&mCopyPrefs->mFooterBox,
					&mCopyPrefs->mPrintSummary);
		break;

	case msg_ForwardChoice:
		if (*((long*) ioParam))
		{
			mForwardQuoteOriginal->Disable();
			mForwardHeaders->Disable();
			mForwardAttachment->Disable();
			mForwardRFC822->Disable();
		}
		break;

	case msg_ForwardOptions:
		if (*((long*) ioParam))
		{
			mForwardQuoteOriginal->Enable();
			if (mForwardQuoteOriginal->GetValue())
				mForwardHeaders->Enable();
			mForwardAttachment->Enable();
			if (mForwardAttachment->GetValue())
				mForwardRFC822->Enable();
		}
		break;

	case msg_ForwardQuoteOriginal:
		if (*((long*) ioParam))
			mForwardHeaders->Enable();
		else
			mForwardHeaders->Disable();
		break;

	case msg_ForwardAttachment:
		if (*((long*) ioParam))
			mForwardRFC822->Enable();
		else
			mForwardRFC822->Disable();
		break;

	}
}

// Set prefs
void CPrefsMessageGeneral::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;
	mCopyPrefs = copyPrefs;

	// Toggle IC display
	ToggleICDisplay(false);

	// Copy info
	mSaveCreator->SetText(copyPrefs->mSaveCreator.GetValue());

	if (copyPrefs->mDoSizeWarn.GetValue())
		mSizeLimit->SetValue(1);
	else
		mNoLimit->SetValue(1);

	mWarnMessageSize->SetNumber(copyPrefs->warnMessageSize.GetValue());
	if (copyPrefs->mDoSizeWarn.GetValue())
		mWarnMessageSize->Enable();
	else
		mWarnMessageSize->Disable();

	if (copyPrefs->optionKeyReplyDialog.GetValue())
		mOptionKeyDown->SetValue(1);
	else
		mOptionKeyUp->SetValue(1);

	mForwardChoice->SetValue(copyPrefs->mForwardChoice.GetValue());
	mForwardOptions->SetValue(!copyPrefs->mForwardChoice.GetValue());
	if (copyPrefs->mForwardChoice.GetValue())
	{
		mForwardQuoteOriginal->Disable();
		mForwardHeaders->Disable();
		mForwardAttachment->Disable();
		mForwardRFC822->Disable();
	}

	mForwardQuoteOriginal->SetValue(copyPrefs->mForwardQuoteOriginal.GetValue());
	mForwardHeaders->SetValue(copyPrefs->mForwardHeaders.GetValue());
	if (!copyPrefs->mForwardQuoteOriginal.GetValue())
		mForwardHeaders->Disable();
	mForwardAttachment->SetValue(copyPrefs->mForwardAttachment.GetValue());
	mForwardRFC822->SetValue(copyPrefs->mForwardRFC822.GetValue());
	if (!copyPrefs->mForwardAttachment.GetValue())
		mForwardRFC822->Disable();

	// Make the first suitable TextEdit the Target and select all its text
	//CTextFieldX* select = (!copyPrefs->mUse_IC.GetValue() ? mSaveCreator : mWarnMessageSize);
	CTextFieldX* select = mSaveCreator;
	select->GetSuperCommander()->SetLatentSub(select);
	LCommander::SwitchTarget(select);
	select->SelectAll();
}

// Force update of prefs
void CPrefsMessageGeneral::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	copyPrefs->mSaveCreator.SetValue(mSaveCreator->GetText());

	copyPrefs->mDoSizeWarn.SetValue((mSizeLimit->GetValue() == 1));
	copyPrefs->warnMessageSize.SetValue(mWarnMessageSize->GetNumber());

	copyPrefs->optionKeyReplyDialog.SetValue((mOptionKeyDown->GetValue()==1));

	copyPrefs->mForwardChoice.SetValue(mForwardChoice->GetValue());

	copyPrefs->mForwardQuoteOriginal.SetValue(mForwardQuoteOriginal->GetValue());
	copyPrefs->mForwardHeaders.SetValue(mForwardHeaders->GetValue());
	copyPrefs->mForwardAttachment.SetValue(mForwardAttachment->GetValue());
	copyPrefs->mForwardRFC822.SetValue(mForwardRFC822->GetValue());
}

// Choose a creator
void CPrefsMessageGeneral::DoChooseCreator(void)
{
	// Set value
	PPx::FSObject fspec;
	if (PP_StandardDialogs::AskChooseOneFile('APPL', fspec, kNavDefaultNavDlogOptions | kNavSelectAllReadableItem | kNavAllowPreviews))
	{
		FinderInfo info;
		fspec.GetFinderInfo(&info, NULL, NULL);

		cdstring creator(info.file.fileCreator);
		mSaveCreator->SetText(creator);
	}
}

// Edit caption
void CPrefsMessageGeneral::EditCaption(CPreferenceValueMap<cdstring>* text1,
								CPreferenceValueMap<cdstring>* text2,
								CPreferenceValueMap<cdstring>* text3,
								CPreferenceValueMap<cdstring>* text4,
								CPreferenceValueMap<bool>* cursor_top,
								CPreferenceValueMap<bool>* box1,
								CPreferenceValueMap<bool>* box2,
								CPreferenceValueMap<bool>* summary)
{
	CPrefsEditCaption::PoseDialog(text1, text2, text3, text4, cursor_top, box1, box2, summary,
									CPreferences::sPrefs->spaces_per_tab.GetValue());
}
