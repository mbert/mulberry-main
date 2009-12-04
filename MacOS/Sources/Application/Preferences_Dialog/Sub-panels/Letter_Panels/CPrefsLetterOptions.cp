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


// Source for CPrefsLetterOptions class

#include "CPrefsLetterOptions.h"

#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsLetterOptions::CPrefsLetterOptions()
{
}

// Constructor from stream
CPrefsLetterOptions::CPrefsLetterOptions(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsLetterOptions::~CPrefsLetterOptions()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsLetterOptions::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	mAutoInsertSigBtn = (LCheckBox*) FindPaneByID(paneid_LOAutoInsertSigBtn);
	mSigEmptyLineBtn = (LCheckBox*) FindPaneByID(paneid_LOSigEmptyLineBtn);
	mSigDashesBtn = (LCheckBox*) FindPaneByID(paneid_LOSigDashesBtn);
	mNoSubjectWarnBtn = (LCheckBox*) FindPaneByID(paneid_LONoSubjectWarnBtn);
	mDisplayAttachments = (LCheckBox*) FindPaneByID(paneid_LODisplayAttachments);
	mAppendDraft = (LCheckBox*) FindPaneByID(paneid_LOAppendDraft);
	mInboxAppend = (LCheckBox*) FindPaneByID(paneid_LOInboxAppend);
	mSmartURLPaste = (LCheckBox*) FindPaneByID(paneid_LOSmartURLPaste);
	mComposeWrap = (LCheckBox*) FindPaneByID(paneid_LOWindowWrap);
	mDeleteOriginalDraft = (LCheckBox*) FindPaneByID(paneid_LODeleteOriginalDraft);
	mTemplateDrafts = (LCheckBox*) FindPaneByID(paneid_LOTemplateDrafts);
	mReplyNoSignature = (LCheckBox*) FindPaneByID(paneid_LOReplyNoSignature);
	mWarnReplySubject = (LCheckBox*) FindPaneByID(paneid_LOWarnReplySubject);
	mShowCC = (LCheckBox*) FindPaneByID(paneid_LOShowCC);
	mShowBCC = (LCheckBox*) FindPaneByID(paneid_LOShowBCC);
	mAlwaysUnicode = (LCheckBox*) FindPaneByID(paneid_LOAlwaysUnicode);
	mDisplayIdentityFrom = (LCheckBox*) FindPaneByID(paneid_LODisplayIdentityFrom);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsLetterOptionsBtns);
}

// Handle buttons
void CPrefsLetterOptions::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_LODeleteOriginalDraft:
		if (*((long*) ioParam))
			mTemplateDrafts->Enable();
		else
			mTemplateDrafts->Disable();
		break;
	}
}

// Set prefs
void CPrefsLetterOptions::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	LStr255	copyStr;

	// Save ref to prefs
	mCopyPrefs = copyPrefs;

	mAutoInsertSigBtn->SetValue(copyPrefs->mAutoInsertSignature.GetValue());
	mSigEmptyLineBtn->SetValue(copyPrefs->mSignatureEmptyLine.GetValue());
	mSigDashesBtn->SetValue(copyPrefs->mSigDashes.GetValue());
	mNoSubjectWarnBtn->SetValue(copyPrefs->mNoSubjectWarn.GetValue());
	mDisplayAttachments->SetValue(copyPrefs->mDisplayAttachments.GetValue());
	mAppendDraft->SetValue(copyPrefs->mAppendDraft.GetValue());
	mInboxAppend->SetValue(copyPrefs->inbox_append.GetValue());
	mSmartURLPaste->SetValue(copyPrefs->mSmartURLPaste.GetValue());
	mComposeWrap->SetValue(!copyPrefs->mWindowWrap.GetValue());
	mDeleteOriginalDraft->SetValue(copyPrefs->mDeleteOriginalDraft.GetValue());
	mTemplateDrafts->SetValue(copyPrefs->mTemplateDrafts.GetValue());
	if (!copyPrefs->mDeleteOriginalDraft.GetValue())
		mTemplateDrafts->Disable();
	mReplyNoSignature->SetValue(copyPrefs->mReplyNoSignature.GetValue());
	mWarnReplySubject->SetValue(copyPrefs->mWarnReplySubject.GetValue());
	mShowCC->SetValue(copyPrefs->mShowCCs.GetValue());
	mShowBCC->SetValue(copyPrefs->mShowBCCs.GetValue());
	mAlwaysUnicode->SetValue(copyPrefs->mAlwaysUnicode.GetValue());
	mDisplayIdentityFrom->SetValue(copyPrefs->mDisplayIdentityFrom.GetValue());
}

// Force update of prefs
void CPrefsLetterOptions::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;
	copyPrefs->mAutoInsertSignature.SetValue(mAutoInsertSigBtn->GetValue()==1);
	copyPrefs->mSignatureEmptyLine.SetValue(mSigEmptyLineBtn->GetValue()==1);
	copyPrefs->mSigDashes.SetValue(mSigDashesBtn->GetValue()==1);
	copyPrefs->mNoSubjectWarn.SetValue(mNoSubjectWarnBtn->GetValue()==1);
	copyPrefs->mDisplayAttachments.SetValue(mDisplayAttachments->GetValue()==1);
	copyPrefs->mAppendDraft.SetValue(mAppendDraft->GetValue()==1);
	copyPrefs->inbox_append.SetValue(mInboxAppend->GetValue()==1);
	copyPrefs->mSmartURLPaste.SetValue(mSmartURLPaste->GetValue() == 1);
	copyPrefs->mWindowWrap.SetValue(mComposeWrap->GetValue()==0);
	copyPrefs->mDeleteOriginalDraft.SetValue(mDeleteOriginalDraft->GetValue()==1);
	copyPrefs->mTemplateDrafts.SetValue(mTemplateDrafts->GetValue()==1);
	copyPrefs->mReplyNoSignature.SetValue(mReplyNoSignature->GetValue());
	copyPrefs->mWarnReplySubject.SetValue(mWarnReplySubject->GetValue());
	copyPrefs->mShowCCs.SetValue(mShowCC->GetValue());
	copyPrefs->mShowBCCs.SetValue(mShowBCC->GetValue());
	copyPrefs->mAlwaysUnicode.SetValue(mAlwaysUnicode->GetValue());
	copyPrefs->mDisplayIdentityFrom.SetValue(mDisplayIdentityFrom->GetValue());
}
