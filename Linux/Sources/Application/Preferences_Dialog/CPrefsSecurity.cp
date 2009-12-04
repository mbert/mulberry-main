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


// CPrefsSecurity.cpp : implementation file
//

#include "CPrefsSecurity.h"

#include "CCertManagerDialog.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSecurityPlugin.h"
#include "CTextField.h"
#include "CXStringResources.h"

#include "TPopupMenu.h"

#include <JXColormap.h>
#include "JXSecondaryRadioGroup.h"
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>

extern const char* cGPGName;
extern const char* cPGPName;
extern const char* cSMIMEName;

/////////////////////////////////////////////////////////////////////////////
// CPrefsSecurity dialog

CPrefsSecurity::CPrefsSecurity( JXContainer* enclosure,
														const HSizingOption hSizing, 
														const VSizingOption vSizing,
														const JCoordinate x, const JCoordinate y,
														const JCoordinate w, const JCoordinate h) 
	:CPrefsPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsSecurity message handlers

void CPrefsSecurity::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Security Preferences", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 140,20);
    assert( obj1 != NULL );
    const JFontStyle obj1_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj1->SetFontStyle(obj1_style);

    JXStaticText* obj2 =
        new JXStaticText("Preferred Plugin:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,30, 105,20);
    assert( obj2 != NULL );

    mPreferredPlugin =
        new HPopupMenu("",this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,25, 195,25);
    assert( mPreferredPlugin != NULL );

    mUseMIMESecurity =
        new JXTextCheckbox("Use MIME Multipart Security with PGP", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,60, 250,20);
    assert( mUseMIMESecurity != NULL );

    mEncryptToSelf =
        new JXTextCheckbox("Always Encrypt to Self", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,80, 250,20);
    assert( mEncryptToSelf != NULL );

    mCachePassphrase =
        new JXTextCheckbox("Cache passphrase while Running", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,100, 250,20);
    assert( mCachePassphrase != NULL );

    mAutoVerify =
        new JXTextCheckbox("Verify Signed Messages when Opened", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,130, 250,20);
    assert( mAutoVerify != NULL );

    mAutoDecrypt =
        new JXTextCheckbox("Decrypt Messages when Opened", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,150, 250,20);
    assert( mAutoDecrypt != NULL );

    mWarnUnencryptedSend =
        new JXTextCheckbox("Warn when Responding without Encryption", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,180, 270,20);
    assert( mWarnUnencryptedSend != NULL );

    mUseErrorAlerts =
        new JXTextCheckbox("Notify Errors with an Alert Box", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,210, 250,20);
    assert( mUseErrorAlerts != NULL );

    JXSecondaryRadioGroup* obj3 =
        new JXSecondaryRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,250, 360,70);
    assert( obj3 != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Signature Verified OK Notification:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 25,240, 210,20);
    assert( obj4 != NULL );

    mSigAlert =
        new JXTextCheckbox("Use Alert", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 85,20);
    assert( mSigAlert != NULL );

    mSigPlaySound =
        new JXTextCheckbox("Play Sound:", obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,35, 105,20);
    assert( mSigPlaySound != NULL );

    mSigSound =
        new CTextInputField(obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 115,36, 230,20);
    assert( mSigSound != NULL );

    mCertificatesBtn =
        new JXTextButton("Manage Certificates...", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,340, 150,25);
    assert( mCertificatesBtn != NULL );

// end JXLayout1

	// Init the popup - noting missing plugins
	cdstring notloaded(rsrc::GetString("Alerts::General::PreferredPlugin_Missing"));
	cdstring str = "PGP - using PGP Plugin";
	if (CSecurityPlugin::GetRegisteredPlugin(cPGPName) == NULL)
		str += notloaded;
	mPreferredPlugin->AppendItem(str, kFalse, kTrue);
	str = "PGP - using GPG Plugin";
	if (CSecurityPlugin::GetRegisteredPlugin(cGPGName) == NULL)
		str += notloaded;
	mPreferredPlugin->AppendItem(str, kFalse, kTrue);
	str = "S/MIME";
	if (CSecurityPlugin::GetRegisteredPlugin(cSMIMEName) == NULL)
		str += notloaded;
	mPreferredPlugin->AppendItem(str, kFalse, kTrue);
	mPreferredPlugin->SetUpdateAction(JXMenu::kDisableNone);
	mPreferredPlugin->SetToPopupChoice(kTrue, 1);
	
	// Start listening
	ListenTo(mSigPlaySound);
	ListenTo(mCertificatesBtn);
}

// Handle buttons
void CPrefsSecurity::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mSigPlaySound)
		{
			if (mSigPlaySound->IsChecked())
			{
				mSigSound->Activate();

				cdstring txt = mSigSound->GetText();
				::PlayNamedSound(txt);
			}
			else
				mSigSound->Deactivate();
			return;
		}
	}
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mCertificatesBtn)
		{
			OnCertificates();
			return;
		}
	}
}

// Set up params for DDX
void CPrefsSecurity::SetPrefs(CPreferences* prefs)
{
	mCopyPrefs = prefs;

	if (mCopyPrefs->mPreferredPlugin.GetValue() == cPGPName)
		mPreferredPlugin->SetValue(eUsePGP);
	else if (mCopyPrefs->mPreferredPlugin.GetValue() == cGPGName)
		mPreferredPlugin->SetValue(eUseGPG);
	else if (mCopyPrefs->mPreferredPlugin.GetValue() == cSMIMEName)
		mPreferredPlugin->SetValue(eUseSMIME);

	mUseMIMESecurity->SetState(JBoolean(prefs->mUseMIMESecurity.GetValue()));
	mEncryptToSelf->SetState(JBoolean(prefs->mEncryptToSelf.GetValue()));
	mCachePassphrase->SetState(JBoolean(prefs->mCachePassphrase.GetValue()));
	mAutoVerify->SetState(JBoolean(prefs->mAutoVerify.GetValue()));
	mAutoDecrypt->SetState(JBoolean(prefs->mAutoDecrypt.GetValue()));
	mWarnUnencryptedSend->SetState(JBoolean(prefs->mWarnUnencryptedSend.GetValue()));
	mUseErrorAlerts->SetState(JBoolean(prefs->mUseErrorAlerts.GetValue()));

	const CNotification& verify_notify = mCopyPrefs->mVerifyOKNotification.GetValue();
	mSigAlert->SetState(JBoolean(verify_notify.DoShowAlert()));

	SetListening(kFalse);
	mSigPlaySound->SetState(JBoolean(verify_notify.DoPlaySound()));
	SetListening(kTrue);
	mSigSound->SetText(verify_notify.GetSoundID());
	if (!verify_notify.DoPlaySound())
		mSigSound->Deactivate();
}

// Get params from DDX
void CPrefsSecurity::UpdatePrefs(CPreferences* prefs)
{
	if (mPreferredPlugin->GetValue() == eUsePGP)
		prefs->mPreferredPlugin.SetValue(cPGPName);
	else if (mPreferredPlugin->GetValue() == eUseGPG)
		prefs->mPreferredPlugin.SetValue(cGPGName);
	else if (mPreferredPlugin->GetValue() == eUseSMIME)
		prefs->mPreferredPlugin.SetValue(cSMIMEName);

	prefs->mUseMIMESecurity.SetValue(mUseMIMESecurity->IsChecked());
	prefs->mEncryptToSelf.SetValue(mEncryptToSelf->IsChecked());
	prefs->mCachePassphrase.SetValue(mCachePassphrase->IsChecked());
	prefs->mAutoVerify.SetValue(mAutoVerify->IsChecked());
	prefs->mAutoDecrypt.SetValue(mAutoDecrypt->IsChecked());
	prefs->mWarnUnencryptedSend.SetValue(mWarnUnencryptedSend->IsChecked());
	prefs->mUseErrorAlerts.SetValue(mUseErrorAlerts->IsChecked());

	// Make copy to look for changes
	CNotification& verify_notify = prefs->mVerifyOKNotification.Value();
	CNotification verify_copy(verify_notify);

	verify_notify.SetShowAlert(mSigAlert->IsChecked());

	verify_notify.SetPlaySound(mSigPlaySound->IsChecked());
	verify_notify.SetSoundID(cdstring(mSigSound->GetText()));

	// Set dirty if required
	if (!(verify_copy == verify_notify))
		prefs->mVerifyOKNotification.SetDirty();
}

void CPrefsSecurity::OnCertificates()
{
	CCertManagerDialog::PoseDialog();
}
