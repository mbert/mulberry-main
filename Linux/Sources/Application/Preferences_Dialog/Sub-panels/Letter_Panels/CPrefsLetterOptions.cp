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

#include <JXStaticText.h>
#include <JXTextCheckbox.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsLetterOptions::OnCreate()
{
// begin JXLayout1

    mAutoInsertSignature =
        new JXTextCheckbox("Insert Signature Automatically", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,0, 195,20);
    assert( mAutoInsertSignature != NULL );

    mSigEmptyLine =
        new JXTextCheckbox("Add Blank Line before Signature", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,20, 210,20);
    assert( mSigEmptyLine != NULL );

    mSigDashes =
        new JXTextCheckbox("Add '--' before Signature", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,40, 210,20);
    assert( mSigDashes != NULL );

    mReplyNoSignature =
        new JXTextCheckbox("Remove Signatures from Reply Quotation", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,60, 265,20);
    assert( mReplyNoSignature != NULL );

    mNoSubjectWarn =
        new JXTextCheckbox("Warn if no Subject when Sending", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,80, 215,20);
    assert( mNoSubjectWarn != NULL );

    mWarnReplySubject =
        new JXTextCheckbox("Warn when Reply Subject is Changed", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,100, 265,20);
    assert( mWarnReplySubject != NULL );

    mDisplayIdentityFrom =
        new JXTextCheckbox("Display From Address in Identity Popup", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,120, 265,20);
    assert( mDisplayIdentityFrom != NULL );

    mShowCc =
        new JXTextCheckbox("Always Show Cc Field", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,140, 185,20);
    assert( mShowCc != NULL );

    mShowBcc =
        new JXTextCheckbox("Always Show Bcc Field", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,160, 185,20);
    assert( mShowBcc != NULL );

    mDisplayAttachments =
        new JXTextCheckbox("Only Show Attachments in Parts List", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,180, 245,20);
    assert( mDisplayAttachments != NULL );

    mAppendDraft =
        new JXTextCheckbox("Append as Draft", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,200, 120,20);
    assert( mAppendDraft != NULL );

    mINBOXAppend =
        new JXTextCheckbox("Copy INBOX Messages being Replied to", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,220, 265,20);
    assert( mINBOXAppend != NULL );

    mSmartURLPaste =
        new JXTextCheckbox("Add URL Delimiters when Pasting Text", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,240, 255,20);
    assert( mSmartURLPaste != NULL );

    mComposeWrap =
        new JXTextCheckbox("Wrap Text while Composing", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,260, 190,20);
    assert( mComposeWrap != NULL );

    mDeleteOriginalDraft =
        new JXTextCheckbox("Delete Original Draft", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,280, 145,20);
    assert( mDeleteOriginalDraft != NULL );

    mTemplateDrafts =
        new JXTextCheckbox("Don't Delete Important Drafts", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,300, 190,20);
    assert( mTemplateDrafts != NULL );

    mAlwaysUnicode =
        new JXTextCheckbox("Always use Unicode Character Set for non-ASCII Text", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,320, 340,20);
    assert( mAlwaysUnicode != NULL );

// end JXLayout1

	// Start listening
	ListenTo(mDeleteOriginalDraft);
}

void CPrefsLetterOptions::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mDeleteOriginalDraft)
		{
			if (mDeleteOriginalDraft->IsChecked())
				mTemplateDrafts->Activate();
			else
				mTemplateDrafts->Deactivate();
		}
	}

	CTabPanel::Receive(sender, message);
}

// Set prefs
void CPrefsLetterOptions::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Save ref to prefs
	mCopyPrefs = copyPrefs;

	mAutoInsertSignature->SetState(JBoolean(copyPrefs->mAutoInsertSignature.GetValue()));
	mSigEmptyLine->SetState(JBoolean(copyPrefs->mSignatureEmptyLine.GetValue()));
	mSigDashes->SetState(JBoolean(copyPrefs->mSigDashes.GetValue()));
	mReplyNoSignature->SetState(JBoolean(copyPrefs->mReplyNoSignature.GetValue()));
	mNoSubjectWarn->SetState(JBoolean(copyPrefs->mNoSubjectWarn.GetValue()));
	mWarnReplySubject->SetState(JBoolean(copyPrefs->mWarnReplySubject.GetValue()));
	mDisplayIdentityFrom->SetState(JBoolean(copyPrefs->mDisplayIdentityFrom.GetValue()));
	mShowCc->SetState(JBoolean(copyPrefs->mShowCCs.GetValue()));
	mShowBcc->SetState(JBoolean(copyPrefs->mShowBCCs.GetValue()));
	mDisplayAttachments->SetState(JBoolean(copyPrefs->mDisplayAttachments.GetValue()));
	mAppendDraft->SetState(JBoolean(copyPrefs->mAppendDraft.GetValue()));
	mINBOXAppend->SetState(JBoolean(copyPrefs->inbox_append.GetValue()));
	mSmartURLPaste->SetState(JBoolean(copyPrefs->mSmartURLPaste.GetValue()));
	mComposeWrap->SetState(JBoolean(!copyPrefs->mWindowWrap.GetValue()));
	mDeleteOriginalDraft->SetState(JBoolean(copyPrefs->mDeleteOriginalDraft.GetValue()));
	mTemplateDrafts->SetState(JBoolean(copyPrefs->mTemplateDrafts.GetValue()));
	if (!copyPrefs->mDeleteOriginalDraft.GetValue())
		mTemplateDrafts->Deactivate();
	mAlwaysUnicode->SetState(JBoolean(copyPrefs->mAlwaysUnicode.GetValue()));
}

// Force update of prefs
bool CPrefsLetterOptions::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	copyPrefs->mAutoInsertSignature.SetValue(mAutoInsertSignature->IsChecked());
	copyPrefs->mSignatureEmptyLine.SetValue(mSigEmptyLine->IsChecked());
	copyPrefs->mSigDashes.SetValue(mSigDashes->IsChecked());
	copyPrefs->mReplyNoSignature.SetValue(mReplyNoSignature->IsChecked());
	copyPrefs->mNoSubjectWarn.SetValue(mNoSubjectWarn->IsChecked());
	copyPrefs->mWarnReplySubject.SetValue(mWarnReplySubject->IsChecked());
	copyPrefs->mDisplayIdentityFrom.SetValue(mDisplayIdentityFrom->IsChecked());
	copyPrefs->mShowCCs.SetValue(mShowCc->IsChecked());
	copyPrefs->mShowBCCs.SetValue(mShowBcc->IsChecked());
	copyPrefs->mDisplayAttachments.SetValue(mDisplayAttachments->IsChecked());
	copyPrefs->mAppendDraft.SetValue(mAppendDraft->IsChecked());
	copyPrefs->inbox_append.SetValue(mINBOXAppend->IsChecked());
	copyPrefs->mSmartURLPaste.SetValue(mSmartURLPaste->IsChecked());
	copyPrefs->mWindowWrap.SetValue(!mComposeWrap->IsChecked());
	copyPrefs->mDeleteOriginalDraft.SetValue(mDeleteOriginalDraft->IsChecked());
	copyPrefs->mTemplateDrafts.SetValue(mTemplateDrafts->IsChecked());
	copyPrefs->mAlwaysUnicode.SetValue(mAlwaysUnicode->IsChecked());
	
	return true;
}
