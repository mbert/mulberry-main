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


// Source for CPrefsLetterGeneral class

#include "CPrefsLetterGeneral.h"

#include "CAdminLock.h"
#include "CMailboxPopup.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsEditCaption.h"
#include "CPrefsEditHeadFoot.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>
#include <LPushButton.h>
#include <LRadioButton.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsLetterGeneral::CPrefsLetterGeneral()
{
}

// Constructor from stream
CPrefsLetterGeneral::CPrefsLetterGeneral(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
	mSpacesPerTab = NULL;	// Need NULL because of early Activate call
}

// Default destructor
CPrefsLetterGeneral::~CPrefsLetterGeneral()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsLetterGeneral::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	mMailDomain = (CTextFieldX*) FindPaneByID(paneid_LGMailDomain);

	// Set backgrounds
	mSpacesPerTab = (CTextFieldX*) FindPaneByID(paneid_LGSpacesPerTab);
	mTabSpace = (LCheckBox*) FindPaneByID(paneid_LGTabSpace);

	mReplyQuoteChar = (CTextFieldX*) FindPaneByID(paneid_LGIncludeChar);
	mForwardQuoteChar = (CTextFieldX*) FindPaneByID(paneid_LGForwardQuoteChar);
	mForwardSubject = (CTextFieldX*) FindPaneByID(paneid_LGForwardSubject);

	mWrapLength = (CTextFieldX*) FindPaneByID(paneid_LGWrapLength);

	mSeparateBCC = (LCheckBox*) FindPaneByID(paneid_LGSeparateBCC);
	mBCCCaption = (LPushButton*) FindPaneByID(paneid_LGBCCCaption);

	mSaveToFile = (LRadioButton*) FindPaneByID(paneid_LGSaveToFile);
	mSaveToMailbox = (LRadioButton*) FindPaneByID(paneid_LGSaveToMailbox);
	mSaveChoose = (LRadioButton*) FindPaneByID(paneid_LGSaveChoose);
	mSaveToMailboxName = (CTextFieldX*) FindPaneByID(paneid_LGSaveToMailboxName);
	mSaveToMailboxPopup = (CMailboxPopup*) FindPaneByID(paneid_LGSaveToMailboxPopup);
	mSaveToMailboxPopup->SetCopyTo(false);
	if (!CMulberryApp::sApp->LoadedPrefs())
		mSaveToMailboxPopup->Disable();
	
	mAutoSaveDrafts = (LCheckBox*) FindPaneByID(paneid_LGAutoSaveDrafts);
	mAutoSaveDraftsInterval = (CTextFieldX*) FindPaneByID(paneid_LGAutoSaveDraftsInterval);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsLetterGeneralBtns);
}

// Toggle display of IC items
void CPrefsLetterGeneral::ToggleICDisplay(bool IC_on)
{
	if (IC_on)
	{
		mReplyQuoteChar->Disable();
		//mForwardQuoteChar->Disable();
	}
	else
	{
		mReplyQuoteChar->Enable();
		//mForwardQuoteChar->Enable();
	}
}

// Handle buttons
void CPrefsLetterGeneral::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_LGEditReplySet:
		EditCaption(&mCopyPrefs->mReplyStart, NULL, &mCopyPrefs->mReplyEnd, NULL, &mCopyPrefs->mReplyCursorTop, NULL, NULL, NULL);
		break;
	case msg_LGEditForwardSet:
		EditCaption(&mCopyPrefs->mForwardStart, NULL, &mCopyPrefs->mForwardEnd, NULL, &mCopyPrefs->mForwardCursorTop, NULL, NULL, NULL);
		break;
	case msg_LGEditLtrPrintCaption:
		EditCaption(&mCopyPrefs->mHeaderCaption,
					&mCopyPrefs->mLtrHeaderCaption,
					&mCopyPrefs->mFooterCaption,
					&mCopyPrefs->mLtrFooterCaption,
					NULL,
					&mCopyPrefs->mHeaderBox,
					&mCopyPrefs->mFooterBox,
					&mCopyPrefs->mPrintSummary);
		break;
	case msg_LGSeparateBCC:
		if (*(long*) ioParam)
			mBCCCaption->Enable();
		else
			mBCCCaption->Disable();
		break;
	case msg_LGEditBCCCaption:
		EditBCCCaption();
		break;

	case msg_LGSaveToFile:
	case msg_LGSaveChoose:
		if (*((long*) ioParam))
		{
			mSaveToMailboxName->Disable();
			mSaveToMailboxPopup->Disable();
		}
		break;
	case msg_LGSaveToMailbox:
		if (*((long*) ioParam))
		{
			mSaveToMailboxName->Enable();
			if (CMulberryApp::sApp->LoadedPrefs())
				mSaveToMailboxPopup->Enable();
		}
		break;
	case msg_LGSaveToMailboxPopup:
		{
			cdstring mbox_name;
			if (mSaveToMailboxPopup->GetSelectedMboxName(mbox_name))
			{
				if (mbox_name.empty())
				{
					mSaveToMailboxName->SetText(cdstring::null_str);
				}
				else if (*mbox_name.c_str() != 0x01)
				{
					mSaveToMailboxName->SetText(mbox_name);
				}
			}
		}
		break;
	case msg_LGAutoSaveDrafts:
		mAutoSaveDraftsInterval->SetEnabled(*((long*) ioParam));
		break;
	}
}

// Set prefs
void CPrefsLetterGeneral::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	LStr255	copyStr;

	// Save ref to prefs
	mCopyPrefs = copyPrefs;

	// Toggle IC display
	ToggleICDisplay(mCopyPrefs->mUse_IC.GetValue());

	mMailDomain->SetText(copyPrefs->mMailDomain.GetValue());

	// Copy text to edit fields
	mSpacesPerTab->SetNumber(copyPrefs->spaces_per_tab.GetValue());
	mTabSpace->SetValue(copyPrefs->mTabSpace.GetValue());

	mReplyQuoteChar->SetText(copyPrefs->mReplyQuote.GetValue());

	mForwardQuoteChar->SetText(copyPrefs->mForwardQuote.GetValue());

	mForwardSubject->SetText(copyPrefs->mForwardSubject.GetValue());

	mWrapLength->SetNumber(copyPrefs->wrap_length.GetValue());
	
	mSeparateBCC->SetValue(copyPrefs->mSeparateBCC.GetValue());
	
	mSaveToFile->SetValue(copyPrefs->mSaveOptions.GetValue().GetValue() == eSaveDraftToFile);
	mSaveToMailbox->SetValue(copyPrefs->mSaveOptions.GetValue().GetValue() == eSaveDraftToMailbox);
	mSaveChoose->SetValue(copyPrefs->mSaveOptions.GetValue().GetValue() == eSaveDraftChoose);

	mSaveToMailboxName->SetText(copyPrefs->mSaveMailbox.GetValue());
	if (!mSaveToMailbox->GetValue())
	{
		mSaveToMailboxName->Disable();
		mSaveToMailboxPopup->Disable();
	}	

	mAutoSaveDrafts->SetValue(copyPrefs->mAutoSaveDrafts.GetValue());
	mAutoSaveDraftsInterval->SetNumber(copyPrefs->mAutoSaveDraftsInterval.GetValue());
	mAutoSaveDraftsInterval->SetEnabled(mAutoSaveDrafts->GetValue());
}

// Force update of prefs
void CPrefsLetterGeneral::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	copyPrefs->mMailDomain.SetValue(mMailDomain->GetText());

	// Copy info from panel into prefs
	copyPrefs->spaces_per_tab.SetValue(mSpacesPerTab->GetNumber());
	copyPrefs->mTabSpace.SetValue(mTabSpace->GetValue() == 1);

	copyPrefs->mReplyQuote.SetValue(mReplyQuoteChar->GetText());

	copyPrefs->mForwardQuote.SetValue(mForwardQuoteChar->GetText());

	copyPrefs->mForwardSubject.SetValue(mForwardSubject->GetText());

	copyPrefs->wrap_length.SetValue(mWrapLength->GetNumber());

	copyPrefs->mSeparateBCC.SetValue(mSeparateBCC->GetValue());

	if (mSaveToFile->GetValue())
		copyPrefs->mSaveOptions.SetValue(eSaveDraftToFile);
	else if (mSaveToMailbox->GetValue())
		copyPrefs->mSaveOptions.SetValue(eSaveDraftToMailbox);
	else if (mSaveChoose->GetValue())
		copyPrefs->mSaveOptions.SetValue(eSaveDraftChoose);
	copyPrefs->mSaveMailbox.SetValue(mSaveToMailboxName->GetText());

	copyPrefs->mAutoSaveDrafts.SetValue(mAutoSaveDrafts->GetValue());
	copyPrefs->mAutoSaveDraftsInterval.SetValue(mAutoSaveDraftsInterval->GetNumber());
}

// Edit caption
void CPrefsLetterGeneral::EditCaption(CPreferenceValueMap<cdstring>* text1,
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


// Edit bcc caption
void CPrefsLetterGeneral::EditBCCCaption()
{
	CPrefsEditHeadFoot::PoseDialog("Set BCC Caption", mCopyPrefs->mBCCCaption.Value(), false,
									mCopyPrefs->spaces_per_tab.GetValue(), mCopyPrefs->wrap_length.GetValue());
}
