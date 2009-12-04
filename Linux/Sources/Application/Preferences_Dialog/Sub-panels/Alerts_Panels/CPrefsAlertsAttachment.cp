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


// Source for CPrefsAlertsAttachment class

#include "CPrefsAlertsAttachment.h"

#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTextField.h"

#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAlertsAttachment::OnCreate()
{
// begin JXLayout1

    mAttachmentAlert =
        new JXTextCheckbox("Use Alert", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,25, 85,20);
    assert( mAttachmentAlert != NULL );

    mAttachmentPlaySound =
        new JXTextCheckbox("Play Sound:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,45, 105,20);
    assert( mAttachmentPlaySound != NULL );

    mAttachmentSound =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,46, 175,20);
    assert( mAttachmentSound != NULL );

// end JXLayout1

	// Start listening
	ListenTo(mAttachmentPlaySound);
}

// Handle buttons
void CPrefsAlertsAttachment::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mAttachmentPlaySound)
		{
			if (mAttachmentPlaySound->IsChecked())
			{
				mAttachmentSound->Activate();

				cdstring txt = mAttachmentSound->GetText();
				::PlayNamedSound(txt);
			}
			else
				mAttachmentSound->Deactivate();
			return;
		}
	}
}

// Set prefs
void CPrefsAlertsAttachment::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	const CNotification& attachment_notify = copyPrefs->mAttachmentNotification.GetValue();
	mAttachmentAlert->SetState(JBoolean(attachment_notify.DoShowAlert()));

	SetListening(kFalse);
	mAttachmentPlaySound->SetState(JBoolean(attachment_notify.DoPlaySound()));
	SetListening(kTrue);
	mAttachmentSound->SetText(attachment_notify.GetSoundID());
	if (!attachment_notify.DoPlaySound())
		mAttachmentSound->Deactivate();
}

// Force update of prefs
bool CPrefsAlertsAttachment::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Make copy to look for changes
	CNotification& attachment_notify = copyPrefs->mAttachmentNotification.Value();
	CNotification attach_copy(attachment_notify);

	attachment_notify.SetShowAlert(mAttachmentAlert->IsChecked());

	attachment_notify.SetPlaySound(mAttachmentPlaySound->IsChecked());
	attachment_notify.SetSoundID(mAttachmentSound->GetText());

	// Set dirty if required
	if (!(attach_copy == attachment_notify))
		copyPrefs->mAttachmentNotification.SetDirty();
	
	return true;
}
