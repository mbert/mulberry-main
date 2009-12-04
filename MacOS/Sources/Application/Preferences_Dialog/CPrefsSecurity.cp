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


// Source for CPrefsSecurity class

#include "CPrefsSecurity.h"

#include "CCertManagerDialog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSecurityPlugin.h"
#include "CSoundPopup.h"
#include "CTextFieldX.h"
#include "CXStringResources.h"

#include <LCheckBox.h>

extern const char* cGPGName;
extern const char* cPGPName;
extern const char* cSMIMEName;

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S M E S S A G E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsSecurity::CPrefsSecurity()
{
}

// Constructor from stream
CPrefsSecurity::CPrefsSecurity(LStream *inStream)
		: CPrefsPanel(inStream)
{
}

// Default destructor
CPrefsSecurity::~CPrefsSecurity()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsSecurity::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsPanel::FinishCreateSelf();

	// Set backgrounds
	mPreferredPlugin = (LPopupButton*) FindPaneByID(paneid_PreferredPlugin);
	mUseMIMESecurity = (LCheckBox*) FindPaneByID(paneid_UseMIMESecurity);
	mEncryptToSelf = (LCheckBox*) FindPaneByID(paneid_EncryptToSelf);
	mCachePassphrase = (LCheckBox*) FindPaneByID(paneid_CachePassphrase);
	mAutoVerify = (LCheckBox*) FindPaneByID(paneid_AutoVerify);
	mAutoDecrypt = (LCheckBox*) FindPaneByID(paneid_AutoDecrypt);
	mWarnUnencryptedSend = (LCheckBox*) FindPaneByID(paneid_WarnUnencryptedSend);
	mUseErrorAlerts = (LCheckBox*) FindPaneByID(paneid_UseErrorAlerts);

	mVerifyOKAlert = (LCheckBox*) FindPaneByID(paneid_VerifyOKAlert);
	mVerifyOKPlaySound = (LCheckBox*) FindPaneByID(paneid_VerifyOKPlaySound);
	mVerifyOKSound = (CSoundPopup*) FindPaneByID(paneid_VerifyOKSound);
	mVerifyOKSpeak = (LCheckBox*) FindPaneByID(paneid_VerifyOKSpeak);
	mVerifyOKSpeakText = (CTextFieldX*) FindPaneByID(paneid_VerifyOKSpeakText);

	// Check for loaded plugins
	cdstring notloaded(rsrc::GetString("Alerts::General::PreferredPlugin_Missing"));
	if (CSecurityPlugin::GetRegisteredPlugin(cPGPName) == NULL)
	{
		cdstring title = ::GetMenuItemTextUTF8(mPreferredPlugin->GetMacMenuH(), 1);
		title += notloaded;
		::SetMenuItemTextUTF8(mPreferredPlugin->GetMacMenuH(), 1, title);
	}
	if (CSecurityPlugin::GetRegisteredPlugin(cGPGName) == NULL)
	{
		cdstring title = ::GetMenuItemTextUTF8(mPreferredPlugin->GetMacMenuH(), 2);
		title += notloaded;
		::SetMenuItemTextUTF8(mPreferredPlugin->GetMacMenuH(), 2, title);
	}
	if (CSecurityPlugin::GetRegisteredPlugin(cSMIMEName) == NULL)
	{
		cdstring title = ::GetMenuItemTextUTF8(mPreferredPlugin->GetMacMenuH(), 3);
		title += notloaded;
		::SetMenuItemTextUTF8(mPreferredPlugin->GetMacMenuH(), 3, title);
	}

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsSecurityBtns);
}

// Handle buttons
void CPrefsSecurity::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_VerifyOKPlaySound:
		if (*((long*) ioParam))
		{
			mVerifyOKSound->Enable();

			cdstring title;
			mVerifyOKSound->GetName(title);
			::PlayNamedSound(title);
		}
		else
			mVerifyOKSound->Disable();
		break;

	case msg_VerifyOKSound:
		{
			cdstring title;
			mVerifyOKSound->GetName(title);
			::PlayNamedSound(title);
		}
		break;

	case msg_VerifyOKSpeak:
		if (*((long*) ioParam))
			mVerifyOKSpeakText->Enable();
		else
			mVerifyOKSpeakText->Disable();
		break;

	case msg_SecurityCertificates:
		CCertManagerDialog::PoseDialog();
		break;
	}
}

// Toggle display of IC items
void CPrefsSecurity::ToggleICDisplay(void)
{
}

// Set prefs
void CPrefsSecurity::SetPrefs(CPreferences* copyPrefs)
{
	// Save ref to prefs
	mCopyPrefs = copyPrefs;
	CPreferences* local_prefs = copyPrefs;

	// Toggle IC display
	ToggleICDisplay();

	// Copy info
	if (local_prefs->mPreferredPlugin.GetValue() == cPGPName)
		mPreferredPlugin->SetValue(eUsePGP);
	else if (local_prefs->mPreferredPlugin.GetValue() == cGPGName)
		mPreferredPlugin->SetValue(eUseGPG);
	else if (local_prefs->mPreferredPlugin.GetValue() == cSMIMEName)
		mPreferredPlugin->SetValue(eUseSMIME);

	mUseMIMESecurity->SetValue(local_prefs->mUseMIMESecurity.GetValue());
	mEncryptToSelf->SetValue(local_prefs->mEncryptToSelf.GetValue());
	mCachePassphrase->SetValue(local_prefs->mCachePassphrase.GetValue());
	mAutoVerify->SetValue(local_prefs->mAutoVerify.GetValue());
	mAutoDecrypt->SetValue(local_prefs->mAutoDecrypt.GetValue());
	mWarnUnencryptedSend->SetValue(local_prefs->mWarnUnencryptedSend.GetValue());
	mUseErrorAlerts->SetValue(local_prefs->mUseErrorAlerts.GetValue());

	LStr255	copyStr;

	StopListening();

	const CNotification& verifyOK_notify = copyPrefs->mVerifyOKNotification.GetValue();
	mVerifyOKAlert->SetValue(verifyOK_notify.DoShowAlert() ? 1 : 0);
	mVerifyOKPlaySound->SetValue(verifyOK_notify.DoPlaySound());
	mVerifyOKSound->SetName(verifyOK_notify.GetSoundID());
	if (!verifyOK_notify.DoPlaySound())
		mVerifyOKSound->Disable();
	mVerifyOKSpeak->SetValue(verifyOK_notify.DoSpeakText() ? 1 : 0);
	mVerifyOKSpeakText->SetText(verifyOK_notify.GetTextToSpeak());
	if (!verifyOK_notify.DoSpeakText())
		mVerifyOKSpeakText->Disable();

	StartListening();
}

// Force update of prefs
void CPrefsSecurity::UpdatePrefs(void)
{
	CPreferences* local_prefs = mCopyPrefs;

	// Copy info from panel into prefs
	if (mPreferredPlugin->GetValue() == eUsePGP)
		local_prefs->mPreferredPlugin.SetValue(cPGPName);
	else if (mPreferredPlugin->GetValue() == eUseGPG)
		local_prefs->mPreferredPlugin.SetValue(cGPGName);
	else if (mPreferredPlugin->GetValue() == eUseSMIME)
		local_prefs->mPreferredPlugin.SetValue(cSMIMEName);
	local_prefs->mUseMIMESecurity.SetValue(mUseMIMESecurity->GetValue());
	local_prefs->mEncryptToSelf.SetValue(mEncryptToSelf->GetValue());
	local_prefs->mCachePassphrase.SetValue(mCachePassphrase->GetValue());
	local_prefs->mAutoVerify.SetValue(mAutoVerify->GetValue());
	local_prefs->mAutoDecrypt.SetValue(mAutoDecrypt->GetValue());
	local_prefs->mWarnUnencryptedSend.SetValue(mWarnUnencryptedSend->GetValue());
	local_prefs->mUseErrorAlerts.SetValue(mUseErrorAlerts->GetValue());

	// Make copy to look for changes
	CNotification& verifyOK_notify = local_prefs->mVerifyOKNotification.Value();
	CNotification attach_copy(verifyOK_notify);

	verifyOK_notify.SetShowAlert(mVerifyOKAlert->GetValue()==1);

	verifyOK_notify.SetPlaySound(mVerifyOKPlaySound->GetValue());
	cdstring snd;
	mVerifyOKSound->GetName(snd);
	verifyOK_notify.SetSoundID(snd);
	verifyOK_notify.SetSpeakText(mVerifyOKSpeak->GetValue());
	verifyOK_notify.SetTextToSpeak(mVerifyOKSpeakText->GetText());

	// Set dirty if required
	if (!(attach_copy == verifyOK_notify))
		local_prefs->mVerifyOKNotification.SetDirty();
}
