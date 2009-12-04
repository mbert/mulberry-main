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
#include "CSoundPopup.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAlertsAttachment::CPrefsAlertsAttachment()
{
}

// Constructor from stream
CPrefsAlertsAttachment::CPrefsAlertsAttachment(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsAlertsAttachment::~CPrefsAlertsAttachment()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAlertsAttachment::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	mAttachmentAlert = (LCheckBox*) FindPaneByID(paneid_AttachmentAlert);
	mAttachmentPlaySound = (LCheckBox*) FindPaneByID(paneid_AttachmentPlaySound);
	mAttachmentSound = (CSoundPopup*) FindPaneByID(paneid_AttachmentSound);
	mAttachmentSpeak = (LCheckBox*) FindPaneByID(paneid_AttachmentSpeak);
	mAttachmentSpeakText = (CTextFieldX*) FindPaneByID(paneid_AttachmentSpeakText);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsAlertsAttachmentBtns);
}

// Handle buttons
void CPrefsAlertsAttachment::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_AttachmentPlaySound:
			if (*((long*) ioParam))
			{
				mAttachmentSound->Enable();

				cdstring title;
				mAttachmentSound->GetName(title);
				::PlayNamedSound(title);
			}
			else
				mAttachmentSound->Disable();
			break;
		case msg_AttachmentSound:
			{
				cdstring title;
				mAttachmentSound->GetName(title);
				::PlayNamedSound(title);
			}
			break;
		case msg_AttachmentSpeak:
			if (*((long*) ioParam))
				mAttachmentSpeakText->Enable();
			else
				mAttachmentSpeakText->Disable();
			break;
	}
}

// Set prefs
void CPrefsAlertsAttachment::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	StopListening();

	const CNotification& attachment_notify = copyPrefs->mAttachmentNotification.GetValue();
	mAttachmentAlert->SetValue(attachment_notify.DoShowAlert() ? 1 : 0);
	mAttachmentPlaySound->SetValue(attachment_notify.DoPlaySound());
	mAttachmentSound->SetName(attachment_notify.GetSoundID());
	if (!attachment_notify.DoPlaySound())
		mAttachmentSound->Disable();
	mAttachmentSpeak->SetValue(attachment_notify.DoSpeakText() ? 1 : 0);
	mAttachmentSpeakText->SetText(attachment_notify.GetTextToSpeak());
	if (!attachment_notify.DoSpeakText())
		mAttachmentSpeakText->Disable();

	StartListening();
}

// Force update of prefs
void CPrefsAlertsAttachment::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Make copy to look for changes
	CNotification& attachment_notify = copyPrefs->mAttachmentNotification.Value();
	CNotification attach_copy(attachment_notify);

	attachment_notify.SetShowAlert(mAttachmentAlert->GetValue()==1);

	attachment_notify.SetPlaySound(mAttachmentPlaySound->GetValue());
	cdstring snd;
	mAttachmentSound->GetName(snd);
	attachment_notify.SetSoundID(snd);
	attachment_notify.SetSpeakText(mAttachmentSpeak->GetValue());
	attachment_notify.SetTextToSpeak(mAttachmentSpeakText->GetText());

	// Set dirty if required
	if (!(attach_copy == attachment_notify))
		copyPrefs->mAttachmentNotification.SetDirty();
}
