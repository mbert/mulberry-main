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
#include "CPreferences.h"
#include "CMulberryCommon.h"
#include "CSecurityPlugin.h"
#include "CUnicodeUtils.h"
#include "CXStringResources.h"

extern const char* cGPGName;
extern const char* cPGPName;
extern const char* cSMIMEName;

/////////////////////////////////////////////////////////////////////////////
// CPrefsSecurity dialog

IMPLEMENT_DYNCREATE(CPrefsSecurity, CPrefsPanel)

CPrefsSecurity::CPrefsSecurity() : CPrefsPanel(CPrefsSecurity::IDD)
{
	//{{AFX_DATA_INIT(CPrefsSecurity)
	mUseMIMESecurity = FALSE;
	mEncryptToSelf = FALSE;
	mCachePassphrase = FALSE;
	mAutoVerify = FALSE;
	mAutoDecrypt = FALSE;
	mWarnUnencryptedSend = FALSE;
	mUseErrorAlerts = FALSE;
	mVerifyOKAlert = FALSE;
	mVerifyOKPlaySound = FALSE;
	mVerifyOKSpeak = FALSE;
	//}}AFX_DATA_INIT
}

CPrefsSecurity::~CPrefsSecurity()
{
}

void CPrefsSecurity::DoDataExchange(CDataExchange* pDX)
{
	CPrefsPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsSecurity)
	DDX_Check(pDX, IDC_USEMIMESECURITY, mUseMIMESecurity);
	DDX_Check(pDX, IDC_ENCRYPTSELF, mEncryptToSelf);
	DDX_Check(pDX, IDC_CACHEPASSPHRASE, mCachePassphrase);
	DDX_Check(pDX, IDC_AUTOVERIFY, mAutoVerify);
	DDX_Check(pDX, IDC_AUTODECRYPT, mAutoDecrypt);
	DDX_Check(pDX, IDC_WARNUNENCRYPTEDSEND, mWarnUnencryptedSend);
	DDX_Check(pDX, IDC_USEERRORALERTS, mUseErrorAlerts);
	DDX_Check(pDX, IDC_VERIFYOK_USEALERT, mVerifyOKAlert);
	DDX_Check(pDX, IDC_VERIFYOK_PLAYSOUND, mVerifyOKPlaySound);
	DDX_Control(pDX, IDC_VERIFYOK_PLAYSOUND, mVerifyOKPlaySoundCtrl);
	DDX_Check(pDX, IDC_VERIFYOK_SPEAK, mVerifyOKSpeak);
	DDX_Control(pDX, IDC_VERIFYOK_SPEAK, mVerifyOKSpeakCtrl);
	DDX_UTF8Text(pDX, IDC_VERIFYOK_SPEAKTEXT, mVerifyOKSpeakText);
	DDX_Control(pDX, IDC_VERIFYOK_SPEAKTEXT, mVerifyOKSpeakTextCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsSecurity, CPrefsPanel)
	//{{AFX_MSG_MAP(CPrefsSecurity)
	ON_COMMAND_RANGE(IDM_PREFERREDPLUGIN_PGP, IDM_PREFERREDPLUGIN_SMIME, OnPreferredPlugin)
	ON_BN_CLICKED(IDC_VERIFYOK_PLAYSOUND, OnVerifyOKPlaySound)
	ON_COMMAND_RANGE(IDM_SOUND_Start, IDM_SOUND_End, OnVerifyOKSound)
	ON_BN_CLICKED(IDC_VERIFYOK_SPEAK, OnVerifyOKSpeak)
	ON_BN_CLICKED(IDC_CERTIFICATES, OnCertificates)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsSecurity message handlers

void CPrefsSecurity::InitControls(void)
{
	// Subclass buttons
	mPreferredPlugin.SubclassDlgItem(IDC_PREFERREDPLUGIN, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mPreferredPlugin.SetMenu(IDR_POPUP_PREFERREDPLUGIN);
	
	// Check for loaded plugins
	cdstring notloaded(rsrc::GetString("Alerts::General::PreferredPlugin_Missing"));
	if (CSecurityPlugin::GetRegisteredPlugin(cPGPName) == NULL)
	{
		CString title;
		mPreferredPlugin.GetPopupMenu()->GetMenuString(IDM_PREFERREDPLUGIN_PGP, title, MF_BYCOMMAND);
		title += notloaded.win_str();
		mPreferredPlugin.GetPopupMenu()->ModifyMenu(IDM_PREFERREDPLUGIN_PGP, MF_BYCOMMAND | MF_STRING, IDM_PREFERREDPLUGIN_PGP, title);
	}
	if (CSecurityPlugin::GetRegisteredPlugin(cGPGName) == NULL)
	{
		CString title;
		mPreferredPlugin.GetPopupMenu()->GetMenuString(IDM_PREFERREDPLUGIN_GPG, title, MF_BYCOMMAND);
		title += notloaded.win_str();
		mPreferredPlugin.GetPopupMenu()->ModifyMenu(IDM_PREFERREDPLUGIN_GPG, MF_BYCOMMAND | MF_STRING, IDM_PREFERREDPLUGIN_GPG, title);
	}
	if (CSecurityPlugin::GetRegisteredPlugin(cSMIMEName) == NULL)
	{
		CString title;
		mPreferredPlugin.GetPopupMenu()->GetMenuString(IDM_PREFERREDPLUGIN_SMIME, title, MF_BYCOMMAND);
		title += notloaded.win_str();
		mPreferredPlugin.GetPopupMenu()->ModifyMenu(IDM_PREFERREDPLUGIN_SMIME, MF_BYCOMMAND | MF_STRING, IDM_PREFERREDPLUGIN_SMIME, title);
	}

	mVerifyOKSound.SubclassDlgItem(IDC_VERIFYOK_SOUNDPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mVerifyOKSound.SetMenu(IDR_POPUP_SOUND);
	mVerifyOKSound.SetCommandBase(IDM_SOUND_Start);	
	mVerifyOKSound.SyncMenu();
}

void CPrefsSecurity::SetControls(void)
{
	// set initial control states
	mPreferredPlugin.SetValue(mPreferredPluginValue);

	mVerifyOKSound.SetSound(mVerifyOKSoundStr);
	mVerifyOKSound.EnableWindow(mVerifyOKPlaySound);
	mVerifyOKSpeakTextCtrl.EnableWindow(mVerifyOKSpeak);
}

// Set up params for DDX
void CPrefsSecurity::SetPrefs(CPreferences* prefs)
{
	mCopyPrefs = prefs;

	if (mCopyPrefs->mPreferredPlugin.GetValue() == cPGPName)
		mPreferredPluginValue = eUsePGP;
	else if (mCopyPrefs->mPreferredPlugin.GetValue() == cGPGName)
		mPreferredPluginValue = eUseGPG;
	else if (mCopyPrefs->mPreferredPlugin.GetValue() == cSMIMEName)
		mPreferredPluginValue = eUseSMIME;

	mUseMIMESecurity = prefs->mUseMIMESecurity.GetValue();
	mEncryptToSelf = prefs->mEncryptToSelf.GetValue();
	mCachePassphrase = prefs->mCachePassphrase.GetValue();
	mAutoVerify = prefs->mAutoVerify.GetValue();
	mAutoDecrypt = prefs->mAutoDecrypt.GetValue();
	mWarnUnencryptedSend = prefs->mWarnUnencryptedSend.GetValue();
	mUseErrorAlerts = prefs->mUseErrorAlerts.GetValue();

	const CNotification& verifyOK_notify = prefs->mVerifyOKNotification.GetValue();
	mVerifyOKAlert = verifyOK_notify.DoShowAlert();
	mVerifyOKPlaySound = verifyOK_notify.DoPlaySound();
	mVerifyOKSoundStr = verifyOK_notify.GetSoundID();
	mVerifyOKSpeak = verifyOK_notify.DoSpeakText();
	mVerifyOKSpeakText = verifyOK_notify.GetTextToSpeak();
}

// Get params from DDX
void CPrefsSecurity::UpdatePrefs(CPreferences* prefs)
{
	if (mPreferredPluginValue == eUsePGP)
		prefs->mPreferredPlugin.SetValue(cPGPName);
	else if (mPreferredPluginValue == eUseGPG)
		prefs->mPreferredPlugin.SetValue(cGPGName);
	else if (mPreferredPluginValue == eUseSMIME)
		prefs->mPreferredPlugin.SetValue(cSMIMEName);

	prefs->mUseMIMESecurity.SetValue(mUseMIMESecurity);
	prefs->mEncryptToSelf.SetValue(mEncryptToSelf);
	prefs->mCachePassphrase.SetValue(mCachePassphrase);
	prefs->mAutoVerify.SetValue(mAutoVerify);
	prefs->mAutoDecrypt.SetValue(mAutoDecrypt);
	prefs->mWarnUnencryptedSend.SetValue(mWarnUnencryptedSend);
	prefs->mUseErrorAlerts.SetValue(mUseErrorAlerts);

	// Mopy to look for changes
	CNotification& verifyOK_notify = prefs->mVerifyOKNotification.Value();
	CNotification attach_copy(verifyOK_notify);
	
	// Get values
	verifyOK_notify.SetShowAlert(mVerifyOKAlert);
	verifyOK_notify.SetPlaySound(mVerifyOKPlaySound);
	verifyOK_notify.SetSoundID(mVerifyOKSoundStr);
	verifyOK_notify.SetSpeakText(mVerifyOKSpeak);
	verifyOK_notify.SetTextToSpeak(mVerifyOKSpeakText);

	// Set dirty if required
	if (!(attach_copy == verifyOK_notify))
		prefs->mVerifyOKNotification.SetDirty();
}

void CPrefsSecurity::OnPreferredPlugin(UINT nID)
{
	mPreferredPlugin.SetValue(nID);
	mPreferredPluginValue = nID;
}

void CPrefsSecurity::OnVerifyOKPlaySound()
{
	// TODO: Add your control notification handler code here
	mVerifyOKSound.EnableWindow(mVerifyOKPlaySoundCtrl.GetCheck());
	if (mVerifyOKPlaySoundCtrl.GetCheck())
		::PlayNamedSound(mVerifyOKSoundStr);
}

void CPrefsSecurity::OnVerifyOKSound(UINT nID)
{
	mVerifyOKSound.SetValue(nID);
	mVerifyOKSoundStr = mVerifyOKSound.GetSound();
	::PlayNamedSound(mVerifyOKSoundStr);
}

void CPrefsSecurity::OnVerifyOKSpeak()
{
	mVerifyOKSpeakTextCtrl.EnableWindow(mVerifyOKSpeakCtrl.GetCheck());
}

void CPrefsSecurity::OnCertificates()
{
	CCertManagerDialog::PoseDialog();
}
