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


// CPrefsAlertsMessage.cpp : implementation file
//


#include "CPrefsAlertsMessage.h"

#include "CErrorHandler.h"
#include "CGetStringDialog.h"
#include "CMailAccountManager.h"
#include "CPreferences.h"
#include "CMulberryCommon.h"
#include "CUnicodeUtils.h"
#include "CXStringResources.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAlertsMessage property page

IMPLEMENT_DYNCREATE(CPrefsAlertsMessage, CTabPanel)

CPrefsAlertsMessage::CPrefsAlertsMessage()
	: CTabPanel(CPrefsAlertsMessage::IDD)
{
	//{{AFX_DATA_INIT(CPrefsAlertsMessage)
	//}}AFX_DATA_INIT
	mCopyPrefs = NULL;
}

void CPrefsAlertsMessage::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsAlertsMessage)
	DDV_MinMaxInt(pDX, IDC_PREFS_ALERTS1_CHECKINTERVAL, 0, 32767);
	DDX_Control(pDX, IDC_PREFS_ALERTS1_SPEAK, mNewMailSpeakCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsAlertsMessage, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsAlertsMessage)
	ON_COMMAND(IDM_ALERTSTYLE_New, OnNewStyle)
	ON_COMMAND(IDM_ALERTSTYLE_Rename, OnRenameStyle)
	ON_COMMAND(IDM_ALERTSTYLE_Delete, OnDeleteStyle)
	ON_COMMAND_RANGE(IDM_ALERTSTYLE_Start, IDM_ALERTSTYLE_End, OnStyle)
	ON_BN_CLICKED(IDC_PREFS_ALERTS1_CHECKNEVER, OnCheckNever)
	ON_BN_CLICKED(IDC_PREFS_ALERTS1_CHECKONCE, OnCheckOnce)
	ON_BN_CLICKED(IDC_PREFS_ALERTS1_CHECKEVERY, OnCheckEvery)
	ON_BN_CLICKED(IDC_PREFS_ALERTS1_PLAYSOUND, OnNewMailPlaySound)
	ON_COMMAND_RANGE(IDM_SOUND_Start, IDM_SOUND_End, OnNewMailSound)
	ON_BN_CLICKED(IDC_PREFS_ALERTS1_SPEAK, OnNewMailSpeak)
	ON_COMMAND_RANGE(IDM_ALERTCABINET_Start, IDM_ALERTCABINET_End, OnCabinet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CPrefsAlertsMessage::OnInitDialog()
{
	CTabPanel::OnInitDialog();
	
	// Subclass buttons
	mAlertStylePopup.SubclassDlgItem(IDC_PREFS_ALERTS1_STYLEPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mAlertStylePopup.SetMenu(IDR_POPUP_ALERTSTYLES);

	mNewMailSound.SubclassDlgItem(IDC_PREFS_ALERTS1_SOUNDPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mNewMailSound.SetMenu(IDR_POPUP_SOUND);
	mNewMailSound.SetCommandBase(IDM_SOUND_Start);
	mNewMailSound.SyncMenu();
	
	mApplyToCabinet.SubclassDlgItem(IDC_PREFS_ALERTS1_CABINETS, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mApplyToCabinet.SetMenu(IDR_POPUP_ALERTCABINETS);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Set up params for DDX
void CPrefsAlertsMessage::SetContent(void* data)
{
	// Save ref to prefs
	mCopyPrefs = (CPreferences*) data;

	if (!mCopyPrefs->mMailNotification.GetValue().size())
	{
		CMailNotification notify;
		notify.SetName("Default");
		mCopyPrefs->mMailNotification.Value().push_back(notify);
		mCopyPrefs->mMailNotification.SetDirty();
	}

	// Init popups
	InitStylePopup();
	InitCabinetPopup();

	// Set initial style
	mStyleValue = 0;
	mAlertStylePopup.SetValue(mStyleValue + IDM_ALERTSTYLE_Start);
	
	// Set initial data
	SetNotifaction();
}

// Get params from DDX
bool CPrefsAlertsMessage::UpdateContent(void* data)
{
	// Update current style
	if (mStyleValue >= 0)
		UpdateNotifaction();
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsAlertsMessage message handlers

// Add new account
void CPrefsAlertsMessage::OnNewStyle(void)
{
	// Update current style
	if (mStyleValue >= 0)
		UpdateNotifaction();

	// Create the dialog
	{
		while (true)
		{
			bool fail = false;
			cdstring new_name;

			if (CGetStringDialog::PoseDialog("Alerts::Preferences::NewAlertStyleName", "Alerts::Preferences::NewAlertStyleName", new_name))
			{
				// Empty name
				if (new_name.empty())
				{
					CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::EmptyAlertStyleName");
					continue;
				}

				// Check duplicate name
				for(CMailNotificationList::const_iterator iter = mCopyPrefs->mMailNotification.GetValue().begin();
						iter != mCopyPrefs->mMailNotification.GetValue().end(); iter++)
				{
					if (new_name == (*iter).GetName())
					{
						CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::DuplicateAlertStyleName");
						fail = true;
						break;
					}
				}
				if (fail)
					continue;

				// Create new alert style
				CMailNotification notify;
				notify.SetName(new_name);
				mCopyPrefs->mMailNotification.Value().push_back(notify);
				mCopyPrefs->mMailNotification.SetDirty();
				unsigned long index = mCopyPrefs->mMailNotification.GetValue().size() - 1;

				// Reset menu
				InitStylePopup();

				// Set new values
				mStyleValue = index;
				mAlertStylePopup.SetValue(mStyleValue + IDM_ALERTSTYLE_Start);
				break;
			}
			else
				mAlertStylePopup.SetValue(mStyleValue + IDM_ALERTSTYLE_Start);
				break;
		}
	}
}

// Rename account
void CPrefsAlertsMessage::OnRenameStyle(void)
{
	// Update current style
	if (mStyleValue >= 0)
		UpdateNotifaction();

	// Create the dialog
	{
		CMailNotification& notify = mCopyPrefs->mMailNotification.Value().at(mStyleValue);

		while (true)
		{
			bool fail = false;
			cdstring new_name = notify.GetName();

			if (CGetStringDialog::PoseDialog("Alerts::Preferences::RenameAlertStyle", "Alerts::Preferences::RenameAlertStyle", new_name))
			{
				// Empty name
				if (new_name.empty())
				{
					CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::EmptyAlertStyleName");
					continue;
				}

				// Check duplicate name
				for(CMailNotificationList::const_iterator iter = mCopyPrefs->mMailNotification.GetValue().begin();
						iter != mCopyPrefs->mMailNotification.GetValue().end(); iter++)
				{
					if (new_name == (*iter).GetName())
					{
						CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::DuplicateAlertStyleName");
						fail = true;
						break;
					}
				}
				if (fail)
					continue;

				// Do style rename
				notify.SetName(new_name);
				mCopyPrefs->mMailNotification.SetDirty();

				// Reset menu
				InitStylePopup();
				break;
			}
			else
				break;
		}

		// Reset to previous value - will update display
		mAlertStylePopup.SetValue(mStyleValue + IDM_ALERTSTYLE_Start);
	}
}

// Delete accoount
void CPrefsAlertsMessage::OnDeleteStyle(void)
{
	// Prevent delete of last alert style
	if (mCopyPrefs->mMailNotification.GetValue().size() == 1)
	{
		CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::NoDeleteAlertStyle");
		mAlertStylePopup.SetValue(mStyleValue + IDM_ALERTSTYLE_Start);
		return;
	}

	// Update current style
	if (mStyleValue >= 0)
		UpdateNotifaction();

	// Check that this is what we want to do
	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Preferences::DeleteAlertStyle") == CErrorHandler::Ok)
	{
		// Erase from prefs
		mCopyPrefs->mMailNotification.Value().erase(mCopyPrefs->mMailNotification.Value().begin() + mStyleValue);
		mCopyPrefs->mMailNotification.SetDirty();

		// Remove from menu
		InitStylePopup();

		// Adjust to new value
		mStyleValue--;
		if (mStyleValue < 0)
			mStyleValue = 0;

		// Update items
		if (mStyleValue >= 0)
			mAlertStylePopup.SetValue(mStyleValue + IDM_ALERTSTYLE_Start);
	}
	else
		mAlertStylePopup.SetValue(mStyleValue + IDM_ALERTSTYLE_Start);
}

// Delete accoount
void CPrefsAlertsMessage::OnStyle(UINT nID)
{
	// Update existing style
	if ((mStyleValue >= 0) && (mStyleValue != nID - IDM_ALERTSTYLE_Start))
		UpdateNotifaction();

	// Set new style
	mStyleValue = nID - IDM_ALERTSTYLE_Start;
	if (mStyleValue >= 0)
		SetNotifaction();

	mAlertStylePopup.SetValue(mStyleValue + IDM_ALERTSTYLE_Start);
}

void CPrefsAlertsMessage::OnCheckNever()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_PREFS_ALERTS1_CHECKINTERVAL)->EnableWindow(false);
}

void CPrefsAlertsMessage::OnCheckOnce()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_PREFS_ALERTS1_CHECKINTERVAL)->EnableWindow(false);
}

void CPrefsAlertsMessage::OnCheckEvery()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_PREFS_ALERTS1_CHECKINTERVAL)->EnableWindow(true);
}

void CPrefsAlertsMessage::OnNewMailPlaySound()
{
	// TODO: Add your control notification handler code here
	mNewMailSound.EnableWindow(static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_PLAYSOUND))->GetCheck());
	if (static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_PLAYSOUND))->GetCheck())
		::PlayNamedSound(mNewMailSound.GetSound());
}

void CPrefsAlertsMessage::OnNewMailSound(UINT nID)
{
	mNewMailSound.SetValue(nID);
	::PlayNamedSound(mNewMailSound.GetSound());
}

void CPrefsAlertsMessage::OnNewMailSpeak()
{
	GetDlgItem(IDC_PREFS_ALERTS1_SPEAKTEXT)->EnableWindow(mNewMailSpeakCtrl.GetCheck());
}

void CPrefsAlertsMessage::OnCabinet(UINT nID)
{
	mApplyToCabinet.SetValue(nID);
}

// Set up style menu
void CPrefsAlertsMessage::InitStylePopup(void)
{
	CMenu* pPopup = mAlertStylePopup.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = 4; i < num_menu; i++)
		pPopup->RemoveMenu(4, MF_BYPOSITION);

	// Add each mail account
	int menu_id = IDM_ALERTSTYLE_Start;
	for(CMailNotificationList::const_iterator iter = mCopyPrefs->mMailNotification.GetValue().begin();
			iter != mCopyPrefs->mMailNotification.GetValue().end(); iter++, menu_id++)
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter).GetName());

	// Refresh its display
	mAlertStylePopup.RefreshValue();
}

// Set up cabinet menu
void CPrefsAlertsMessage::InitCabinetPopup(void)
{
	CMenu* pPopup = mApplyToCabinet.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = CMailAccountManager::eFavouriteOthers; i < num_menu; i++)
		pPopup->RemoveMenu(CMailAccountManager::eFavouriteOthers, MF_BYPOSITION);

	int menu_id = CMailAccountManager::eFavouriteOthers + IDM_ALERTCABINET_Start;
	for(CFavouriteItemList::const_iterator iter = mCopyPrefs->mFavourites.GetValue().begin() + CMailAccountManager::eFavouriteOthers;
			iter != mCopyPrefs->mFavourites.GetValue().end(); iter++, menu_id++)
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter).GetName());

	// Refresh its display
	mApplyToCabinet.RefreshValue();
}

// Set alert details
void CPrefsAlertsMessage::SetNotifaction(void)
{
	const CMailNotification& notify = mCopyPrefs->mMailNotification.GetValue().at(mStyleValue);

	// Copy text to edit fields
	static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_CHECKNEVER))->SetCheck(!notify.IsEnabled());
	static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_CHECKONCE))->SetCheck(notify.IsEnabled() && notify.GetCheckOnce());
	static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_CHECKEVERY))->SetCheck(notify.IsEnabled() && !notify.GetCheckOnce());

	CUnicodeUtils::SetWindowTextUTF8(GetDlgItem(IDC_PREFS_ALERTS1_CHECKINTERVAL), cdstring(notify.GetCheckInterval()));
	GetDlgItem(IDC_PREFS_ALERTS1_CHECKINTERVAL)->EnableWindow(notify.IsEnabled());

	static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_NEWMESSAGES))->SetCheck(notify.GetCheckNew());
	static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_ALLMESSAGES))->SetCheck(!notify.GetCheckNew());

	static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_USEALERT))->SetCheck(notify.DoShowAlertForeground() ? 1 : 0);
	static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_USEBACKALERT))->SetCheck(notify.DoShowAlertBackground() ? 1 : 0);
	static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_SHOWICON))->SetCheck(notify.DoFlashIcon() ? 1 : 0);

	static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_PLAYSOUND))->SetCheck(notify.DoPlaySound());
	mNewMailSound.SetSound(notify.GetSoundID());
	mNewMailSound.EnableWindow(notify.DoPlaySound());

	static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_SPEAK))->SetCheck(notify.DoSpeakText());
	CUnicodeUtils::SetWindowTextUTF8(GetDlgItem(IDC_PREFS_ALERTS1_SPEAKTEXT), notify.GetTextToSpeak());
	GetDlgItem(IDC_PREFS_ALERTS1_SPEAKTEXT)->EnableWindow(notify.DoSpeakText());

	static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_OPEN))->SetCheck(notify.GetOpenMbox() ? 1 : 0);

	// Set the cabinet
	unsigned long index = notify.GetFavouriteIndex(mCopyPrefs);
	mApplyToCabinet.SetValue(index + IDM_ALERTCABINET_Start);
}

// Update current account
void CPrefsAlertsMessage::UpdateNotifaction(void)
{
	CMailNotification& notify = mCopyPrefs->mMailNotification.Value().at(mStyleValue);

	// Make copy to look for changes
	CMailNotification copy(notify);

	// Copy info from panel into prefs
	notify.Enable(static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_CHECKNEVER))->GetCheck() == 0);
	notify.SetCheckOnce(static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_CHECKONCE))->GetCheck() == 1);
	cdstring temp = CUnicodeUtils::GetWindowTextUTF8(GetDlgItem(IDC_PREFS_ALERTS1_CHECKINTERVAL));
	notify.SetCheckInterval(::atoi(temp));

	notify.SetCheckNew(static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_NEWMESSAGES))->GetCheck() == 1);

	notify.SetShowAlertForeground(static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_USEALERT))->GetCheck()==1);
	notify.SetShowAlertBackground(static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_USEBACKALERT))->GetCheck()==1);
	notify.SetFlashIcon(static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_SHOWICON))->GetCheck()==1);

	notify.SetPlaySound(static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_PLAYSOUND))->GetCheck() == 1);
	notify.SetSoundID(mNewMailSound.GetSound());

	notify.SetSpeakText(static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_SPEAK))->GetCheck() == 1);
	temp = CUnicodeUtils::GetWindowTextUTF8(GetDlgItem(IDC_PREFS_ALERTS1_SPEAKTEXT));
	notify.SetTextToSpeak(temp);

	notify.SetOpenMbox(static_cast<CButton*>(GetDlgItem(IDC_PREFS_ALERTS1_OPEN))->GetCheck()==1);

	// Set the cabinet
	CMailAccountManager::EFavourite type = static_cast<CMailAccountManager::EFavourite>(mApplyToCabinet.GetValue() - IDM_ALERTCABINET_Start);
	if (type < CMailAccountManager::eFavouriteOthers)
	{
		cdstring temp("#");
		temp += cdstring((unsigned long) type);
		notify.SetFavouriteID(temp);
	}
	else
	{
		cdstring temp("@");
		temp += mCopyPrefs->mFavourites.GetValue().at(type).GetName();
		notify.SetFavouriteID(temp);
	}

	// Set dirty if required
	if (!(copy == notify))
		mCopyPrefs->mMailNotification.SetDirty();
}
