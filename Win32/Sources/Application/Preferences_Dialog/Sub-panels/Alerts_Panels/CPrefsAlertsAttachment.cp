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


// CPrefsAlertsAttachment.cpp : implementation file
//


#include "CPrefsAlertsAttachment.h"

#include "CPreferences.h"
#include "CMulberryCommon.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAlertsAttachment property page

IMPLEMENT_DYNCREATE(CPrefsAlertsAttachment, CTabPanel)

CPrefsAlertsAttachment::CPrefsAlertsAttachment()
	: CTabPanel(CPrefsAlertsAttachment::IDD)
{
	//{{AFX_DATA_INIT(CPrefsAlertsAttachment)
	mAttachmentAlert = FALSE;
	mAttachmentPlaySound = FALSE;
	mAttachmentSpeak = FALSE;
	//}}AFX_DATA_INIT
}

void CPrefsAlertsAttachment::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsAlertsAttachment)
	DDX_Check(pDX, IDC_PREFS_ALERTS2_USEALERT, mAttachmentAlert);
	DDX_Check(pDX, IDC_PREFS_ALERTS2_PLAYSOUND, mAttachmentPlaySound);
	DDX_Control(pDX, IDC_PREFS_ALERTS2_PLAYSOUND, mAttachmentPlaySoundCtrl);
	DDX_Check(pDX, IDC_PREFS_ALERTS2_SPEAK, mAttachmentSpeak);
	DDX_Control(pDX, IDC_PREFS_ALERTS2_SPEAK, mAttachmentSpeakCtrl);
	DDX_UTF8Text(pDX, IDC_PREFS_ALERTS2_SPEAKTEXT, mAttachmentSpeakText);
	DDX_Control(pDX, IDC_PREFS_ALERTS2_SPEAKTEXT, mAttachmentSpeakTextCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsAlertsAttachment, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsAlertsAttachment)
	ON_BN_CLICKED(IDC_PREFS_ALERTS2_PLAYSOUND, OnAttachmentPlaySound)
	ON_COMMAND_RANGE(IDM_SOUND_Start, IDM_SOUND_End, OnAttachmentSound)
	ON_BN_CLICKED(IDC_PREFS_ALERTS2_SPEAK, OnAttachmentSpeak)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CPrefsAlertsAttachment::OnInitDialog()
{
	CTabPanel::OnInitDialog();
	
	// Subclass buttons
	mAttachmentSound.SubclassDlgItem(IDC_PREFS_ALERTS2_SOUNDPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mAttachmentSound.SetMenu(IDR_POPUP_SOUND);
	mAttachmentSound.SetCommandBase(IDM_SOUND_Start);	
	mAttachmentSound.SyncMenu();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Set up params for DDX
void CPrefsAlertsAttachment::SetContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	const CNotification& attachment_notify = copyPrefs->mAttachmentNotification.GetValue();
	mAttachmentAlert = attachment_notify.DoShowAlert();
	mAttachmentPlaySound = attachment_notify.DoPlaySound();
	mAttachmentSoundStr = attachment_notify.GetSoundID();
	mAttachmentSound.SetSound(mAttachmentSoundStr);
	mAttachmentSound.EnableWindow(mAttachmentPlaySound);
	mAttachmentSpeak = attachment_notify.DoSpeakText();
	mAttachmentSpeakText = attachment_notify.GetTextToSpeak();
	mAttachmentSpeakTextCtrl.EnableWindow(mAttachmentSpeak);
}

// Get params from DDX
bool CPrefsAlertsAttachment::UpdateContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Mopy to look for changes
	CNotification& attachment_notify = copyPrefs->mAttachmentNotification.Value();
	CNotification attach_copy(attachment_notify);
	
	// Get values
	attachment_notify.SetShowAlert(mAttachmentAlert);
	attachment_notify.SetPlaySound(mAttachmentPlaySound);
	attachment_notify.SetSoundID(mAttachmentSoundStr);
	attachment_notify.SetSpeakText(mAttachmentSpeak);
	attachment_notify.SetTextToSpeak(mAttachmentSpeakText);

	// Set dirty if required
	if (!(attach_copy == attachment_notify))
		copyPrefs->mAttachmentNotification.SetDirty();
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsAlertsAttachment message handlers

void CPrefsAlertsAttachment::OnAttachmentPlaySound()
{
	// TODO: Add your control notification handler code here
	mAttachmentSound.EnableWindow(mAttachmentPlaySoundCtrl.GetCheck());
	if (mAttachmentPlaySoundCtrl.GetCheck())
		::PlayNamedSound(mAttachmentSoundStr);
}

void CPrefsAlertsAttachment::OnAttachmentSound(UINT nID)
{
	mAttachmentSound.SetValue(nID);
	mAttachmentSoundStr = mAttachmentSound.GetSound();
	::PlayNamedSound(mAttachmentSoundStr);
}

void CPrefsAlertsAttachment::OnAttachmentSpeak()
{
	mAttachmentSpeakTextCtrl.EnableWindow(mAttachmentSpeakCtrl.GetCheck());
}
