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


// CPreferencesDialog.cpp : implementation file
//


#include "CPreferencesDialog.h"

#include "CAdbkManagerView.h"
#include "CAdbkManagerWindow.h"
#include "CAdbkSearchWindow.h"
#include "CAddressBookDoc.h"
#include "CAddressBookView.h"
#include "CAddressBookWindow.h"
#include "CCalendarStoreWindow.h"
#include "CCalendarWindow.h"
#include "CAdminLock.h"
#include "CCopyToMenu.h"
#include "CErrorHandler.h"
#include "CFontCache.h"
#include "CIdentityPopup.h"
#include "CIMAPClient.h"
#include "CLetterWindow.h"
#include "CMailboxWindow.h"
#include "CMboxList.h"
#include "CMessageView.h"
#include "CMulberryApp.h"
#include "COptionsProtocol.h"
#include "CPrefsSimple.h"
#include "CPrefsAccounts.h"
#include "CPrefsAlerts.h"
#include "CPrefsDisplay.h"
#include "CPrefsFormatting.h"
#include "CPrefsMailbox.h"
#include "CPrefsMessage.h"
#include "CPrefsLetter.h"
#include "CPrefsSecurity.h"
#include "CPrefsIdentity.h"
#include "CPrefsAddress.h"
#include "CPrefsCalendar.h"
#include "CPrefsAttachments.h"
#include "CPrefsSpelling.h"
#include "CPluginManager.h"
#include "CPreferencesFile.h"
#include "CPreferenceKeys.h"
#include "CRemotePrefsSets.h"
#include "CSDIFrame.h"
#include "CServerView.h"
#include "CRulesWindow.h"
#include "CServerWindow.h"
#include "CTextListChoice.h"
#include "CUnicodeUtils.h"

const char* cPanelNames[] = {"", "Account", "Alerts", "Display", "Styled", "Mailbox",
							"Message", "Letter", "Security", "Identities", "Address",
							"Calendar", "Attachments", "Spelling", "Speech", NULL};
const int cSecurityIndex = 8;

/////////////////////////////////////////////////////////////////////////////
// CPreferencesDialog dialog

IMPLEMENT_DYNAMIC(CPreferencesDialog, CHelpPropertySheet)

CPreferencesDialog::CPreferencesDialog(CWnd* pParent /*=NULL*/)
	: CHelpPropertySheet(_T("Preferences"), pParent),
	  mCopyPrefs(*CPreferences::sPrefs)
{
	mCurrentPrefsFile = NULL;
	mLoadedNewPrefs = false;
	mWindowsReset = false;
	mAccountNew = false;
	mAccountRename = false;

	mPrefsSimple = NULL;
	mPrefsAccounts = NULL;
	mPrefsAlerts = NULL;
	mPrefsDisplay = NULL;
	mPrefsFormatting = NULL;
	mPrefsMailbox = NULL;
	mPrefsMessage = NULL;
	mPrefsLetter = NULL;
	mPrefsSecurity = NULL;
	mPrefsIdentity = NULL;
	mPrefsAddress = NULL;
	mPrefsCalendar = NULL;
	mPrefsAttachments = NULL;
	mPrefsSpelling = NULL;

	// Force to advanced to get dialog height correctly set up
	bool old_value = mCopyPrefs.mSimple.GetValue();
	SetSimple(false, true);
	mCopyPrefs.mSimple.SetValue(old_value, false);

}

CPreferencesDialog::~CPreferencesDialog()
{
	delete mPrefsSimple;
	delete mPrefsAccounts;
	delete mPrefsAlerts;
	delete mPrefsDisplay;
	delete mPrefsFormatting;
	delete mPrefsMailbox;
	delete mPrefsMessage;
	delete mPrefsLetter;
	delete mPrefsSecurity;
	delete mPrefsIdentity;
	delete mPrefsAddress;
	delete mPrefsCalendar;
	delete mPrefsAttachments;
	delete mPrefsSpelling;
}

BEGIN_MESSAGE_MAP(CPreferencesDialog, CHelpPropertySheet)
	//{{AFX_MSG_MAP(CPreferencesDialog)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_PREFSSIMPLEBTN, OnPrefsSimpleBtn)
	ON_BN_CLICKED(IDC_PREFSADVANCEDBTN, OnPrefsAdvancedBtn)
	ON_BN_CLICKED(IDC_PREFSLOCALBTN, OnPrefsLocalBtn)
	ON_BN_CLICKED(IDC_PREFSREMOTEBTN, OnPrefsRemoteBtn)
	ON_BN_CLICKED(IDC_PREFSOPENBTN, OnPrefsOpenBtn)
	ON_BN_CLICKED(IDC_PREFSSAVEASBTN, OnPrefsSaveAsBtn)
	ON_BN_CLICKED(IDC_PREFSSAVEDEFAULTBTN, OnPrefsSaveDefaultBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPreferencesDialog::InitPrefs(void)
{
	// Do data exchange
	if (mCopyPrefs.mSimple.GetValue())
		mPrefsSimple->SetPrefs(&mCopyPrefs);
	else
	{
		mPrefsAccounts->SetPrefs(&mCopyPrefs);
		mPrefsAlerts->SetPrefs(&mCopyPrefs);
		mPrefsDisplay->SetPrefs(&mCopyPrefs);
		mPrefsFormatting->SetPrefs(&mCopyPrefs);
		mPrefsMailbox->SetPrefs(&mCopyPrefs);
		mPrefsMessage->SetPrefs(&mCopyPrefs);
		mPrefsLetter->SetPrefs(&mCopyPrefs);
		if (mPrefsSecurity)
			mPrefsSecurity->SetPrefs(&mCopyPrefs);
		mPrefsIdentity->SetPrefs(&mCopyPrefs);
		mPrefsAddress->SetPrefs(&mCopyPrefs);
		if (mPrefsCalendar)
			mPrefsCalendar->SetPrefs(&mCopyPrefs);
		mPrefsAttachments->SetPrefs(&mCopyPrefs);
		if (mPrefsSpelling)
			mPrefsSpelling->SetPrefs(&mCopyPrefs);
	}
}

void CPreferencesDialog::ResetPrefs(void)
{
	// Do data exchange
	if (mCopyPrefs.mSimple.GetValue())
		mPrefsSimple->ResetPrefs(&mCopyPrefs);
	else
	{
		mPrefsAccounts->ResetPrefs(&mCopyPrefs);
		mPrefsAlerts->ResetPrefs(&mCopyPrefs);
		mPrefsDisplay->ResetPrefs(&mCopyPrefs);
		mPrefsFormatting->ResetPrefs(&mCopyPrefs);
		mPrefsMailbox->ResetPrefs(&mCopyPrefs);
		mPrefsMessage->ResetPrefs(&mCopyPrefs);
		mPrefsLetter->ResetPrefs(&mCopyPrefs);
		if (mPrefsSecurity)
			mPrefsSecurity->ResetPrefs(&mCopyPrefs);
		mPrefsIdentity->ResetPrefs(&mCopyPrefs);
		mPrefsAddress->ResetPrefs(&mCopyPrefs);
		if (mPrefsCalendar)
			mPrefsCalendar->ResetPrefs(&mCopyPrefs);
		mPrefsAttachments->ResetPrefs(&mCopyPrefs);
		if (mPrefsSpelling)
			mPrefsSpelling->ResetPrefs(&mCopyPrefs);
	}
}

// Update prefs from panel
void CPreferencesDialog::UpdateCopyPrefs(void)
{
	// Force active page to update in case of existing changes
	GetActivePage()->UpdateData(true);

	// Make sheets update prefs
	if (mCopyPrefs.mSimple.GetValue())
		mPrefsSimple->UpdatePrefs(&mCopyPrefs);
	else
	{
		mPrefsAccounts->UpdatePrefs(&mCopyPrefs);
		mPrefsAlerts->UpdatePrefs(&mCopyPrefs);
		mPrefsDisplay->UpdatePrefs(&mCopyPrefs);
		mPrefsFormatting->UpdatePrefs(&mCopyPrefs);
		mPrefsMailbox->UpdatePrefs(&mCopyPrefs);
		mPrefsMessage->UpdatePrefs(&mCopyPrefs);
		mPrefsLetter->UpdatePrefs(&mCopyPrefs);
		if (mPrefsSecurity)
			mPrefsSecurity->UpdatePrefs(&mCopyPrefs);
		mPrefsIdentity->UpdatePrefs(&mCopyPrefs);
		mPrefsAddress->UpdatePrefs(&mCopyPrefs);
		if (mPrefsCalendar)
			mPrefsCalendar->UpdatePrefs(&mCopyPrefs);
		mPrefsAttachments->UpdatePrefs(&mCopyPrefs);
		if (mPrefsSpelling)
			mPrefsSpelling->UpdatePrefs(&mCopyPrefs);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPreferencesDialog message handlers

BOOL CPreferencesDialog::OnInitDialog()
{
	const short cBtnWidth = 82;
	const short cBtnAdjust = 8;
	const short cBtnHeight = 22;
	const short cBtnGap = 30;
	const short cBtnSmallGap = 20;
	const short cBtnExtraGap = 20;

	BOOL result = CHelpPropertySheet::OnInitDialog();

	// Set flags
	m_psh.dwFlags &= ~PSH_HASHELP;		// Disable help button
	m_psh.dwFlags &= ~PSH_NOAPPLYNOW;	// Disable Apply Now button
	ModifyStyleEx(0, WS_EX_CONTEXTHELP | DS_CENTER);	// Add context help and center it

	// Move and add extra buttons

	// Resize
	CRect wndRect;
	GetWindowRect(wndRect);
	MoveWindow(wndRect.left, wndRect.top, wndRect.Width() + 100, wndRect.Height() - 25, false);

	CPoint btnPos;
	btnPos.x = wndRect.right - 4 + cBtnAdjust;
	btnPos.y = wndRect.top + 40;
	ScreenToClient(&btnPos);

	// Adjust for Win32s
	//if (afxData.bWin31)
	//	btnPos.x -= 15;

	// Move OK btn
	CWnd* btn = GetDlgItem(IDOK);
	btn->SetWindowPos(NULL, btnPos.x, btnPos.y, cBtnWidth, cBtnHeight, SWP_NOZORDER);
	btn = btn->GetParent();
	btn = btn->GetParent();

	// Move Cancel btn
	btnPos.Offset(0, cBtnGap);
	btn = GetDlgItem(IDCANCEL);
	btn->SetWindowPos(NULL, btnPos.x, btnPos.y, cBtnWidth, cBtnHeight, SWP_NOZORDER);

	// Hide Apply & Help
	btn = GetDlgItem(ID_APPLY_NOW);
	btn->ShowWindow(SW_HIDE);
	btn = GetDlgItem(IDHELP);
	btn->ShowWindow(SW_HIDE);

	// Position supplementary buttons
	CRect btnRect;
	btnRect.left = btnPos.x;
	btnRect.right = btnRect.left + cBtnWidth;
	btnRect.top = btnPos.y + cBtnExtraGap;
	btnRect.bottom = btnRect.top + cBtnHeight;

	btnRect.DeflateRect(-cBtnAdjust, 0);

	// Create simple button
	btnRect.OffsetRect(0, cBtnGap);
	CString s;
	s.LoadString(IDS_PREFSSIMPLEBTN);
	mPrefsSimpleBtn.Create(s, BS_AUTORADIOBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP, btnRect, this, IDC_PREFSSIMPLEBTN);
	mPrefsSimpleBtn.SetFont(CMulberryApp::sAppFont);
	mPrefsSimpleBtn.SetCheck(mCopyPrefs.mSimple.GetValue());

	// Create advanced button
	btnRect.OffsetRect(0, cBtnSmallGap);
	s.LoadString(IDS_PREFSADVANCEDBTN);
	mPrefsAdvancedBtn.Create(s, BS_AUTORADIOBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP, btnRect, this, IDC_PREFSADVANCEDBTN);
	mPrefsAdvancedBtn.SetFont(CMulberryApp::sAppFont);
	mPrefsAdvancedBtn.SetCheck(!mCopyPrefs.mSimple.GetValue());

	// Create storage area
	btnRect.OffsetRect(0, cBtnGap);
	s.LoadString(IDS_PREFSSTORAGEAREA);
	CRect r = CRect(btnRect.left, btnRect.top + cBtnGap - cBtnAdjust, btnRect.right, btnRect.top + 5 * cBtnGap + cBtnSmallGap + cBtnSmallGap/2);
	mStorageArea.Create(s, WS_CHILD | WS_VISIBLE | BS_GROUPBOX, r, this, IDC_STATIC);
	mStorageArea.SetFont(CMulberryApp::sAppFont);

	btnRect.DeflateRect(cBtnAdjust, 0);

	// Create local button
	btnRect.OffsetRect(0, cBtnGap + cBtnSmallGap/2);
	s.LoadString(IDS_PREFSLOCALBTN);
	mPrefsLocalBtn.Create(s, BS_AUTORADIOBUTTON | WS_GROUP | WS_CHILD | WS_VISIBLE | WS_TABSTOP, btnRect, this, IDC_PREFSLOCALBTN);
	mPrefsLocalBtn.SetFont(CMulberryApp::sAppFont);
	mPrefsLocalBtn.ShowWindow(mCopyPrefs.mSimple.GetValue() ? SW_HIDE : SW_SHOW);

	// Create remote button
	btnRect.OffsetRect(0, cBtnSmallGap);
	s.LoadString(IDS_PREFSREMOTEBTN);
	mPrefsRemoteBtn.Create(s, BS_AUTORADIOBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP, btnRect, this, IDC_PREFSREMOTEBTN);
	mPrefsRemoteBtn.SetFont(CMulberryApp::sAppFont);
	mPrefsRemoteBtn.ShowWindow(mCopyPrefs.mSimple.GetValue() ? SW_HIDE : SW_SHOW);

	// Local/remote captions
	mLocal = true;
	btnRect.DeflateRect(20, 0);
	s.LoadString(IDS_PREFSLOCALBTN);
	mPrefsLocalCaption.Create(s, WS_CHILD | WS_VISIBLE, btnRect, this, IDC_STATIC);
	mPrefsLocalCaption.SetFont(CMulberryApp::sAppFont);
	mPrefsLocalCaption.ShowWindow(mCopyPrefs.mSimple.GetValue() && mLocal ? SW_SHOW : SW_HIDE);
	s.LoadString(IDS_PREFSREMOTEBTN);
	mPrefsRemoteCaption.Create(s, WS_CHILD | WS_VISIBLE, btnRect, this, IDC_STATIC);
	mPrefsRemoteCaption.SetFont(CMulberryApp::sAppFont);
	mPrefsRemoteCaption.ShowWindow(mCopyPrefs.mSimple.GetValue() && !mLocal ? SW_SHOW : SW_HIDE);
	btnRect.DeflateRect(-20, 0);

	// Create open button
	btnRect.OffsetRect(0, cBtnGap);
	s.LoadString(IDS_PREFSOPENBTN);
	mPrefsOpenBtn.Create(s, WS_GROUP | WS_CHILD | WS_VISIBLE | WS_TABSTOP, btnRect, this, IDC_PREFSOPENBTN);
	mPrefsOpenBtn.SetFont(CMulberryApp::sAppFont);

	// Create save as button
	btnRect.OffsetRect(0, cBtnGap);
	s.LoadString(IDS_PREFSSAVEASBTN);
	mPrefsSaveAsBtn.Create(s, WS_CHILD | WS_VISIBLE | WS_TABSTOP, btnRect, this, IDC_PREFSSAVEASBTN);
	mPrefsSaveAsBtn.SetFont(CMulberryApp::sAppFont);

	// Create save default button
	btnRect.OffsetRect(0, cBtnGap);
	s.LoadString(IDS_PREFSSAVEDEFAULTBTN);
	mPrefsSaveDefaultBtn.Create(s, WS_CHILD | WS_VISIBLE | WS_TABSTOP, btnRect, this, IDC_PREFSSAVEDEFAULTBTN);
	mPrefsSaveDefaultBtn.SetFont(CMulberryApp::sAppFont);

	// Set prefs file to current app file and change title
	mCurrentPrefsFile = CMulberryApp::sCurrentPrefsFile;
	SetWindowTitle();

	// Handle admin lockouts
	if (CAdminLock::sAdminLock.mNoLocalPrefs)
	{
		mPrefsLocalBtn.EnableWindow(false);
		mPrefsLocalBtn.SetCheck(0);
		mPrefsRemoteBtn.SetCheck(1);
		mLocal = false;
	}
	if (CAdminLock::sAdminLock.mNoRemotePrefs)
	{
		mPrefsRemoteBtn.EnableWindow(false);
		mPrefsLocalBtn.SetCheck(1);
		mPrefsRemoteBtn.SetCheck(0);
		mLocal = true;
	}
	if (mCurrentPrefsFile)
	{
		mPrefsLocalBtn.SetCheck(!mCurrentPrefsFile->IsRemote());
		mPrefsRemoteBtn.SetCheck(mCurrentPrefsFile->IsRemote());
		mLocal = !mCurrentPrefsFile->IsRemote();
	}


	// Set button states
	mPrefsSaveDefaultBtn.EnableWindow(CAdminLock::sAdminLock.mAllowDefault || (mPrefsRemoteBtn.GetCheck() == 1));

	// Disable Save and open buttons button if not allowed
	if (CAdminLock::sAdminLock.mNoLocalPrefs && CAdminLock::sAdminLock.mNoRemotePrefs)
	{
		mPrefsOpenBtn.EnableWindow(false);
		mPrefsSaveAsBtn.EnableWindow(false);
	}

	// Init'd to advanced in constructor - so change here if different
	if (mCopyPrefs.mSimple.GetValue())
		SetSimple(mCopyPrefs.mSimple.GetValue(), true);

	return result;
}

// Set title
void CPreferencesDialog::SetWindowTitle(void)
{
	// Load string title
	cdstring title;
	title.FromResource("UI::Preferences::Title");
	
	// If no current file say no file or default if allowed
	if (!mCurrentPrefsFile)
	{
		cdstring subtitle;
		subtitle.FromResource(CAdminLock::sAdminLock.mAllowDefault ? "UI::Preferences::DefaultTitle" : "UI::Preferences::NoTitle");
		title += subtitle;
	}
	
	// Otherwise just get its name
	else
	{
		// If remote replace name
		if (mCurrentPrefsFile->IsRemote())
			title.FromResource("UI::Preferences::RemoteTitle");
			
		title += mCurrentPrefsFile->GetSpecifier();
	}
	
	// Now set window title
	CUnicodeUtils::SetWindowTextUTF8(this, title);
}

void CPreferencesDialog::OnOK()
{

	// Force active page to update in case of existing changes
	UpdateCopyPrefs();

	// Prefs must be valid before exit
	if (!mCopyPrefs.Valid(true))
		return;

	// Update current panel name
	UpdateLastPanel();
	
	// Do default OK action
	EndDialog(IDOK);
}

void CPreferencesDialog::UpdateLastPanel()
{
	// Get name of current panel if not simple
	if (GetPageCount() > 1)
	{
		int index = GetActiveIndex();
		if (!CPluginManager::sPluginManager.HasSecurity() && (index >= cSecurityIndex - 1))
			index++;
		
		mCopyPrefs.mLastPanel.SetValue(cPanelNames[index + 1]);
		CPreferences::sPrefs->mLastPanel.SetValue(cPanelNames[index + 1]);
	}
}

void CPreferencesDialog::SetLastPanel()
{
	int panel = 0;
	if (!mCopyPrefs.mLastPanel.GetValue().empty())
	{
		const char** p = cPanelNames;
		while(*++p)
		{
			if (mCopyPrefs.mLastPanel.GetValue() == *p)
				break;
			
		}
		if (*p)
			panel = p - cPanelNames - 1;
		
		if (!CPluginManager::sPluginManager.HasSecurity())
		{
			if (panel == cSecurityIndex - 1)
				panel = 0;
			else if (panel > cSecurityIndex - 1)
				panel--;
		}
	}
	SetActivePage(panel);
}

void CPreferencesDialog::UpdatePrefs()
{
	// Make a copy of the prefs to copy
	CPreferences* newPrefs = new CPreferences(mCopyPrefs);

	// Always save existing current file
	if (mCurrentPrefsFile)
	{
		// Save new prefs
		mCurrentPrefsFile->SetPrefs(newPrefs);
		mCurrentPrefsFile->SavePrefs(true, false);
	}

	// Try to create default prefs file
	else if (CAdminLock::sAdminLock.mAllowDefault)
	{
		// Create new default prefs file
		mCurrentPrefsFile = new CPreferencesFile(true, false);

		// Save new prefs
		mCurrentPrefsFile->SetPrefs(newPrefs);
		mCurrentPrefsFile->SavePrefs(true, false);
	}

	// Try to save in new file
	else if (!CAdminLock::sAdminLock.mNoLocalPrefs || !CAdminLock::sAdminLock.mNoRemotePrefs)
	{
		// Create new file to save as
		mCurrentPrefsFile = new CPreferencesFile(false, false);
		if (mLocal ? !PrefsLocalSaveAs() : !PrefsRemoteSaveAs())
		{
			// Free unwanted objects and cancel OK action
			delete mCurrentPrefsFile;
			mCurrentPrefsFile = NULL;
			delete newPrefs;

			return;
		}
	}

	// If current file is not previous
	if (mCurrentPrefsFile != CMulberryApp::sCurrentPrefsFile)
	{
		// Delete old prefs file
		delete CMulberryApp::sCurrentPrefsFile;

		// Make apps current file the new one
		CMulberryApp::sCurrentPrefsFile = mCurrentPrefsFile;
	}

	// Make changes based on prefs - will reset default
	MakeChanges(newPrefs);

	// Reset cabinets if required
	if (mAccountNew && CMailAccountManager::sMailAccountManager)
		CMailAccountManager::sMailAccountManager->UpdateFavourites();

	// Reset windows if required
	if (mWindowsReset)
	{
		// Do each server window
		{
			cdmutexprotect<CServerWindow::CServerWindowList>::lock _lock(CServerWindow::sServerWindows);
			for(CServerWindow::CServerWindowList::iterator iter1 = CServerWindow::sServerWindows->begin(); iter1 != CServerWindow::sServerWindows->end(); iter1++)
				(*iter1)->ResetState();
		}

		// Do each mailbox window
		{
			cdmutexprotect<CMailboxWindow::CMailboxWindowList>::lock _lock(CMailboxWindow::sMboxWindows);
			for(CMailboxWindow::CMailboxWindowList::iterator iter = CMailboxWindow::sMboxWindows->begin(); iter != CMailboxWindow::sMboxWindows->end(); iter++)
				(*iter)->ResetState();
		}

		// Do each address book window
		{
			cdmutexprotect<CAddressBookWindow::CAddressBookWindowList>::lock _lock(CAddressBookWindow::sAddressBookWindows);
			for(CAddressBookWindow::CAddressBookWindowList::iterator iter = CAddressBookWindow::sAddressBookWindows->begin(); iter != CAddressBookWindow::sAddressBookWindows->end(); iter++)
				(*iter)->ResetState();
		}

		// Address book manager window
		if (CAdbkManagerWindow::sAdbkManager)
			CAdbkManagerWindow::sAdbkManager->ResetState();

		// Address book search window
		if (CAdbkSearchWindow::sAdbkSearch)
			CAdbkSearchWindow::sAdbkSearch->ResetState();
		
		// Calendar manager window
		if (CCalendarStoreWindow::sCalendarStoreManager)
			CCalendarStoreWindow::sCalendarStoreManager->ResetState();

		// Do each calendar window
		{
			cdmutexprotect<CCalendarWindow::CCalendarWindowList>::lock _lock(CCalendarWindow::sCalendarWindows);
			for(CCalendarWindow::CCalendarWindowList::iterator iter = CCalendarWindow::sCalendarWindows->begin(); iter != CCalendarWindow::sCalendarWindows->end(); iter++)
				(*iter)->ResetState();
		}

	}
}

void CPreferencesDialog::OnCancel()
{
	// Delete prefs file object if not same as existing
	if (mCurrentPrefsFile != CMulberryApp::sCurrentPrefsFile)
		delete mCurrentPrefsFile;
	mCurrentPrefsFile = NULL;

	// Do menu list reset if it changed
	if (mForceMenuListReset)
	{

		// Force mailbox menu reset
		CCopyToMenu::SetMenuOptions(CPreferences::sPrefs->mUseCopyToMboxes.GetValue(), CPreferences::sPrefs->mUseAppendToMboxes.GetValue());

		CCopyToMenu::DirtyMenuList();
	}

	// Update last panel now
	UpdateLastPanel();

	// Do default cancel action
	EndDialog(IDCANCEL);
}

// Check for and act on change
void CPreferencesDialog::MakeChanges(CPreferences* newPrefs)
{
	bool refresh_server = false;
	bool refresh_mailbox = false;
	bool refresh_message = false;
	bool refresh_letter = false;
	bool refresh_adbkmanager = false;
	bool refresh_adbksearch = false;
	bool refresh_addressbook = false;
	bool refresh_rules = false;
	bool refresh_mrus = false;

	// Check for change of style or colour in mailbox window
	if ((CPreferences::sPrefs->mServerOpenStyle.GetValue() != newPrefs->mServerOpenStyle.GetValue()) ||
		(CPreferences::sPrefs->mServerClosedStyle.GetValue() != newPrefs->mServerClosedStyle.GetValue()) ||
		(CPreferences::sPrefs->mServerBkgndStyle.GetValue() != newPrefs->mServerBkgndStyle.GetValue()) ||
		(CPreferences::sPrefs->mFavouriteBkgndStyle.GetValue() != newPrefs->mFavouriteBkgndStyle.GetValue()) ||
		(CPreferences::sPrefs->mHierarchyBkgndStyle.GetValue() != newPrefs->mHierarchyBkgndStyle.GetValue()) ||
		(CPreferences::sPrefs->mMboxRecentStyle.GetValue() != newPrefs->mMboxRecentStyle.GetValue()) ||
		(CPreferences::sPrefs->mMboxUnseenStyle.GetValue() != newPrefs->mMboxUnseenStyle.GetValue()) ||
		(CPreferences::sPrefs->mMboxOpenStyle.GetValue() != newPrefs->mMboxOpenStyle.GetValue()) ||
		(CPreferences::sPrefs->mMboxFavouriteStyle.GetValue() != newPrefs->mMboxFavouriteStyle.GetValue()) ||
		(CPreferences::sPrefs->mMboxClosedStyle.GetValue() != newPrefs->mMboxClosedStyle.GetValue()))
	{
		refresh_server = true;
	}

	// Check for change of style or colour
	if ((CPreferences::sPrefs->unseen.GetValue() != newPrefs->unseen.GetValue()) ||
		(CPreferences::sPrefs->seen.GetValue() != newPrefs->seen.GetValue()) ||
		(CPreferences::sPrefs->answered.GetValue() != newPrefs->answered.GetValue()) ||
		(CPreferences::sPrefs->important.GetValue() != newPrefs->important.GetValue()) ||
		(CPreferences::sPrefs->deleted.GetValue() != newPrefs->deleted.GetValue()) ||
		(CPreferences::sPrefs->mMultiAddress.GetValue() != newPrefs->mMultiAddress.GetValue()))
	{
		refresh_mailbox = true;
	}

	// Check for change of font or size in list display
	//if ((CPreferences::sPrefs->mListTextTraits.GetValue().size != mCopyPrefs.mListTextTraits.GetValue().size) ||
	//	::PLstrcmp(CPreferences::sPrefs->mListTextTraits.GetValue().fontName, mCopyPrefs.mListTextTraits.GetValue().fontName) != 0)
	{	// Always force change as Font object must be replaced
		refresh_server = true;
		refresh_mailbox = true;
		refresh_message = true;
		refresh_letter = true;
		refresh_adbkmanager = true;
		refresh_adbksearch = true;
		refresh_addressbook = true;
		refresh_rules = true;
		
		//CFontCache::ResetFonts(newPrefs);
	}

	// Always change font as object is static in prefs
/*
	// Check for change of font or size
	LOGFONT info1;
	LOGFONT info2;
	CPreferences::sPrefs->mTextFont.GetValue().GetLogFont(&info1);
	newPrefs->mTextFont.GetLogFont(&info2);
	if ((info1.lfHeight != info2.lfHeight) ||
		(info1.lfWidth != info2.lfWidth) ||
		(info1.lfEscapement != info2.lfEscapement) ||
		(info1.lfOrientation != info2.lfOrientation) ||
		(info1.lfItalic != info2.lfItalic) ||
		(info1.lfStrikeOut != info2.lfStrikeOut) ||
		(info1.lfCharSet != info2.lfCharSet) ||
		(info1.lfOutPrecision != info2.lfOutPrecision) ||
		(info1.lfClipPrecision != info2.lfClipPrecision) ||
		(info1.lfQuality != info2.lfQuality) ||
		(info1.lfPitchAndFamily != info2.lfPitchAndFamily) ||
		(::strcmp(info1.lfFaceName, info2.lfFaceName) != 0))
*/
	{
		refresh_message = true;
		refresh_letter = true;
	}

	// Check for change of smart address filters
	bool smart_change = (CPreferences::sPrefs->mSmartAddressList.GetValue().size() != newPrefs->mSmartAddressList.GetValue().size());
	if (!smart_change)
	{
		for(int i = 0; i < CPreferences::sPrefs->mSmartAddressList.GetValue().size(); i++)
		{
			if (CPreferences::sPrefs->mSmartAddressList.GetValue()[i] != newPrefs->mSmartAddressList.GetValue()[i])
			{
				smart_change = true;
				break;
			}
		}
	}

	// Always resync identities
	if (true)
	{
		// Iterate over all letter windows
		cdmutexprotect<CLetterWindow::CLetterWindowList>::lock _lock(CLetterWindow::sLetterWindows);
		for(CLetterWindow::CLetterWindowList::iterator iter = CLetterWindow::sLetterWindows->begin(); iter != CLetterWindow::sLetterWindows->end(); iter++)
			(*iter)->SyncIdentities(newPrefs->mIdentities.GetValue());
	}
	
	// Check for change in MRU number
	if (CMailAccountManager::sMailAccountManager &&
		(CPreferences::sPrefs->mMRUMaximum.GetValue() != newPrefs->mMRUMaximum.GetValue()))
		refresh_mrus = true;

	// Replace old prefs with copy of new
	newPrefs->SetAsDefault();

	// Now do window refreshing AFTER setting the default prefs to the new set
	// as some of the windows will use the default prefs values

	if (refresh_server)
	{
		// Iterate over all server windows
		cdmutexprotect<CServerView::CServerViewList>::lock _lock(CServerView::sServerViews);
		for(CServerView::CServerViewList::iterator iter = CServerView::sServerViews->begin(); iter != CServerView::sServerViews->end(); iter++)
		{
			(*iter)->ResetFont(CFontCache::GetListFont());
			(*iter)->Invalidate();	// Needed to get list items drawn in new colours
		}
	}

	if (refresh_mailbox)
	{
		// Iterate over all mailbox windows
		cdmutexprotect<CMailboxView::CMailboxViewList>::lock _lock(CMailboxView::sMailboxViews);
		for(CMailboxView::CMailboxViewList::iterator iter = CMailboxView::sMailboxViews->begin(); iter != CMailboxView::sMailboxViews->end(); iter++)
			(*iter)->ResetFont(CFontCache::GetListFont());
	}

	if (refresh_message)
	{
		// NB IMPORTANT: message window and message views are separate right now
		// Eventually the message window will contain the message view so only the view
		// will need to be updated

		// Iterate over all message windows
		{
			cdmutexprotect<CMessageWindow::CMessageWindowList>::lock _lock(CMessageWindow::sMsgWindows);
			for(CMessageWindow::CMessageWindowList::iterator iter = CMessageWindow::sMsgWindows->begin(); iter != CMessageWindow::sMsgWindows->end(); iter++)
				(*iter)->ResetFont(CFontCache::GetListFont(), CFontCache::GetDisplayFont());
		}

		// Iterate over all message views
		{
			cdmutexprotect<CMessageView::CMessageViewList>::lock _lock(CMessageView::sMsgViews);
			for(CMessageView::CMessageViewList::iterator iter = CMessageView::sMsgViews->begin(); iter != CMessageView::sMsgViews->end(); iter++)
				(*iter)->ResetFont(CFontCache::GetListFont(), CFontCache::GetDisplayFont());
		}
	}

	if (refresh_letter)
	{
		// Iterate over all letter windows
		cdmutexprotect<CLetterWindow::CLetterWindowList>::lock _lock(CLetterWindow::sLetterWindows);
		for(CLetterWindow::CLetterWindowList::iterator iter = CLetterWindow::sLetterWindows->begin(); iter != CLetterWindow::sLetterWindows->end(); iter++)
			(*iter)->ResetFont(CFontCache::GetListFont(), CFontCache::GetDisplayFont());
	}

	if (refresh_adbkmanager)
	{
		// Iterate over all adbk manager windows
		cdmutexprotect<CAdbkManagerView::CAdbkManagerViewList>::lock _lock(CAdbkManagerView::sAdbkManagerViews);
		for(CAdbkManagerView::CAdbkManagerViewList::iterator iter = CAdbkManagerView::sAdbkManagerViews->begin(); iter != CAdbkManagerView::sAdbkManagerViews->end(); iter++)
			(*iter)->ResetFont(CFontCache::GetListFont());
	}

	if (refresh_adbksearch && CAdbkSearchWindow::sAdbkSearch)
		CAdbkSearchWindow::sAdbkSearch->ResetFont(CFontCache::GetListFont());

	if (refresh_addressbook)
	{
		// Iterate over all address books
		cdmutexprotect<CAddressBookView::CAddressBookViewList>::lock _lock(CAddressBookView::sAddressBookViews);
		for(CAddressBookView::CAddressBookViewList::iterator iter = CAddressBookView::sAddressBookViews->begin(); iter != CAddressBookView::sAddressBookViews->end(); iter++)
			(*iter)->ResetFont(CFontCache::GetListFont());
	}

	if (refresh_rules && CRulesWindow::sRulesWindow)
	{
		// Reset the lists and then the font
		CRulesWindow::sRulesWindow->ResetTable();
		CRulesWindow::sRulesWindow->ResetFont(CFontCache::GetListFont());
	}

	// Check for change in MRU number
	if (refresh_mrus && CMailAccountManager::sMailAccountManager)
		CMailAccountManager::sMailAccountManager->CleanMRUHistory(newPrefs->mMRUMaximum.GetValue());

	if (smart_change)
	{
		// Iterate over all mailbox windows
		cdmutexprotect<CMailboxView::CMailboxViewList>::lock _lock(CMailboxView::sMailboxViews);
		for(CMailboxView::CMailboxViewList::iterator iter = CMailboxView::sMailboxViews->begin(); iter != CMailboxView::sMailboxViews->end(); iter++)
			(*iter)->RefreshSmartAddress();
	}

}

void CPreferencesDialog::OnPrefsSimpleBtn()
{
	// Only if not already
	if (!mPrefsSimple)
		SetSimple(true);
}

void CPreferencesDialog::OnPrefsAdvancedBtn()
{
	// Only if not already
	if (mPrefsSimple)
		SetSimple(false);
}

void CPreferencesDialog::SetSimple(bool simple, bool init)
{
	// Prevent flashing
	bool locked = (m_hWnd != NULL) ? LockWindowUpdate() : false;

	if (m_hWnd != NULL)
	{
		mPrefsLocalBtn.ShowWindow(simple ? SW_HIDE : SW_SHOW);
		mPrefsRemoteBtn.ShowWindow(simple ? SW_HIDE : SW_SHOW);
		mPrefsLocalCaption.ShowWindow(simple && mLocal ? SW_SHOW : SW_HIDE);
		mPrefsRemoteCaption.ShowWindow(simple && !mLocal ? SW_SHOW : SW_HIDE);
	}
	
	if (simple)
	{
		// Update current panel name before doing switch
		UpdateLastPanel();
	
		// Add simple page
		AddPage(mPrefsSimple = new CPrefsSimple);
		
		if (!init)
			// Update current preferences from panels
			UpdateCopyPrefs();

		// Now set prefs value
		mCopyPrefs.mSimple.SetValue(simple);
		
		// Force current panels to reset prefs	
		InitPrefs();

		// Remove advanced pages
		if (mPrefsAccounts)
			RemovePage(mPrefsAccounts);
		if (mPrefsAlerts)
			RemovePage(mPrefsAlerts);
		if (mPrefsDisplay)
			RemovePage(mPrefsDisplay);
		if (mPrefsFormatting)
			RemovePage(mPrefsFormatting);
		if (mPrefsMailbox)
			RemovePage(mPrefsMailbox);
		if (mPrefsMessage)
			RemovePage(mPrefsMessage);
		if (mPrefsLetter)
			RemovePage(mPrefsLetter);
		if (mPrefsSecurity)
			RemovePage(mPrefsSecurity);
		if (mPrefsIdentity)
			RemovePage(mPrefsIdentity);
		if (mPrefsAddress)
			RemovePage(mPrefsAddress);
		if (mPrefsCalendar)
			RemovePage(mPrefsCalendar);
		if (mPrefsAttachments)
			RemovePage(mPrefsAttachments);
		if (mPrefsSpelling)
			RemovePage(mPrefsSpelling);

		delete mPrefsAccounts;
		delete mPrefsAlerts;
		delete mPrefsDisplay;
		delete mPrefsFormatting;
		delete mPrefsMailbox;
		delete mPrefsMessage;
		delete mPrefsLetter;
		delete mPrefsSecurity;
		delete mPrefsIdentity;
		delete mPrefsAddress;
		delete mPrefsCalendar;
		delete mPrefsAttachments;
		delete mPrefsSpelling;

		mPrefsAccounts = NULL;
		mPrefsAlerts = NULL;
		mPrefsDisplay = NULL;
		mPrefsFormatting = NULL;
		mPrefsMailbox = NULL;
		mPrefsMessage = NULL;
		mPrefsLetter = NULL;
		mPrefsSecurity = NULL;
		mPrefsIdentity = NULL;
		mPrefsAddress = NULL;
		mPrefsCalendar = NULL;
		mPrefsAttachments = NULL;
		mPrefsSpelling = NULL;
	}
	else
	{
		// Add advanced pages
		AddPage(mPrefsAccounts = new CPrefsAccounts);
		AddPage(mPrefsAlerts = new CPrefsAlerts);
		AddPage(mPrefsDisplay = new CPrefsDisplay);
		AddPage(mPrefsFormatting = new CPrefsFormatting);
		AddPage(mPrefsMailbox = new CPrefsMailbox);
		AddPage(mPrefsMessage = new CPrefsMessage);
		AddPage(mPrefsLetter = new CPrefsLetter);
		if (CPluginManager::sPluginManager.HasSecurity())
			AddPage(mPrefsSecurity = new CPrefsSecurity);
		AddPage(mPrefsIdentity = new CPrefsIdentity);
		AddPage(mPrefsAddress = new CPrefsAddress);
		// Remove unwanted prefs panels by hiding buttons
		if (!CAdminLock::sAdminLock.mPreventCalendars)
			AddPage(mPrefsCalendar = new CPrefsCalendar);
		AddPage(mPrefsAttachments = new CPrefsAttachments);
		if (CPluginManager::sPluginManager.HasSpelling())
			AddPage(mPrefsSpelling = new CPrefsSpelling);
		
		if (!init)
			// Update current preferences from panels
			UpdateCopyPrefs();

		// Now set prefs value
		mCopyPrefs.mSimple.SetValue(simple);
		
		// Force current panels to reset prefs	
		InitPrefs();

		// Remove simple page
		if (mPrefsSimple)
			RemovePage(mPrefsSimple);
		delete mPrefsSimple;
		mPrefsSimple = NULL;

		// Set to last used page
		SetLastPanel();
	}

	if (locked)
	{
		UnlockWindowUpdate();
		RedrawWindow(NULL, NULL, RDW_INVALIDATE);
	}
}

#pragma mark ____________________________Storage Related

void CPreferencesDialog::OnPrefsLocalBtn()
{
	// Only if not local
	if (!mLocal)
	{
		mLocal = true;
		mPrefsSaveDefaultBtn.EnableWindow(CAdminLock::sAdminLock.mAllowDefault);
		
		// Change state of prefs file
		if (mCurrentPrefsFile)
			mCurrentPrefsFile->SetRemote(false);
	}
}

void CPreferencesDialog::OnPrefsRemoteBtn()
{
	// Only if not already
	if (!mLocal)
		return;

	// Force active page to update in case of existing changes
	UpdateCopyPrefs();

	if (!CAdminLock::sAdminLock.mAllowDefault)
		mPrefsSaveDefaultBtn.EnableWindow(true);

	// Must have valid remote IP
	if (!mCopyPrefs.mRemoteAccounts.GetValue().size())
	{
		CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::NoRemoteAccount");

		// Reset buttons
		mPrefsLocalBtn.SetCheck(true);
		mPrefsRemoteBtn.SetCheck(false);

		// Force to remote panel
		SetActivePage(0);
	}
	else if (mCopyPrefs.mRemoteAccounts.GetValue().front()->GetServerIP().empty())
	{
		const cdstring& name = mCopyPrefs.mRemoteAccounts.GetValue().front()->GetName();
		CErrorHandler::PutStopAlertRsrcStr("Alerts::Preferences::Invalid_RemoteServerIP", name);
		
		// Reset buttons
		mPrefsLocalBtn.SetCheck(true);
		mPrefsRemoteBtn.SetCheck(false);

		// Force to account panel
		SetActivePage(0);
	}
	else if (!CMulberryApp::sApp->BeginRemote(&mCopyPrefs))
	{
		mPrefsLocalBtn.SetCheck(true);
		mPrefsRemoteBtn.SetCheck(false);
		if (!CAdminLock::sAdminLock.mAllowDefault)
			mPrefsSaveDefaultBtn.EnableWindow(false);
	}
	else
	{
		// Do logoff to kill connection
		CMulberryApp::sOptionsProtocol->Logoff();

		mLocal = false;
		mPrefsSaveDefaultBtn.EnableWindow(true);
	}

	// Change state of prefs file
	if (mCurrentPrefsFile && !mLocal)
		mCurrentPrefsFile->SetRemote(true);
}

void CPreferencesDialog::OnPrefsOpenBtn()
{
	bool opened = false;

	if (mLocal)
		opened = PrefsLocalOpen();
	else
		opened = PrefsRemoteOpen();
	
	// Must force reset of mailbox menu lists and cabinet contents
	if (opened)
	{
		// Indicate new prefs
		SetLoadedNewPrefs(true);
		
		// Reset prefs changed flags
		SetAccountNew(false);
		SetAccountRename(false);
		GetRenames().clear();
		SetForceWindowReset(false);

		// Mark popups as dirty for new prefs set
		CCopyToMenu::DirtyMenuList();
		SetForceMenuListReset(true);
	}
}

void CPreferencesDialog::OnPrefsSaveAsBtn()
{
	// Force active page to update in case of existing changes
	UpdateCopyPrefs();

	// Allow save of invalid prefs
	if (mLocal)
		PrefsLocalSaveAs();
	else
		PrefsRemoteSaveAs();
}

void CPreferencesDialog::OnPrefsSaveDefaultBtn()
{
	// Force active page to update in case of existing changes
	UpdateCopyPrefs();

	// Prefs must be valid before save
	if (!mCopyPrefs.Valid(true))
		return;

	if (mLocal)
		PrefsLocalSaveDefault();
	else
		PrefsRemoteSaveDefault();
}

bool CPreferencesDialog::PrefsLocalOpen()
{
	// Do standard open dialog
	// prompt the user (with all document templates)
	CString filter = _T("Preferences File (*.mbp)");
	filter += '|';
	filter += '*';
	filter += cPrefFileExtension;
	filter += "||";
	CFileDialog dlgFile(true, NULL, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, filter, CSDIFrame::GetAppTopWindow());

	// Check for good reply
	if (dlgFile.DoModal() == IDOK)
	{
		// Save copy of existing preferences
		CPreferences temp(mCopyPrefs);

		// Create file object
		CPreferencesFile* new_file = new CPreferencesFile(false, false);

		// Set prefs file to new file and copy prefs
		new_file->SetSpecifier(cdstring(dlgFile.GetPathName()));
		new_file->SetSpecified(true);
		new_file->SetPrefs(&mCopyPrefs);

		// Read in new prefs (open as local do not resolve to remote)
		if (new_file->VerifyRead(false))
		{
			// Delete prefs file object if not same as existing
			if (mCurrentPrefsFile != CMulberryApp::sCurrentPrefsFile)
				delete mCurrentPrefsFile;
			mCurrentPrefsFile = new_file;

			// Need to sync new prefs simple/advanced state with old
			mCopyPrefs.mSimple.SetValue(temp.mSimple.GetValue());

			// Set new prefs values in all panels
			ResetPrefs();
			GetActivePage()->UpdateData(false);

			// Reset title
			SetWindowTitle();
		}
		else
		{
			delete new_file;

			// Restore existing prefs
			mCopyPrefs = temp;

			return false;
		}

		return true;
	}
	
	return false;
}

bool CPreferencesDialog::PrefsLocalSaveAs()
{
	CString saveAsName;

	// Use current file if any
	if (mCurrentPrefsFile)
		saveAsName = mCurrentPrefsFile->GetSpecifier();

	else
	{
		// Set default name
		cdstring temp;
		temp.FromResource("UI::Preferences::DefaultTitle");
		saveAsName = temp;
	}

	// Do standard save as dialog
	// prompt the user (with all document templates)
	cdstring filter = "Preferences File (*.mbp)";
	filter += '|';
	filter += '*';
	filter += cPrefFileExtension;
	filter += "||";
	CFileDialog dlgFile(false, cdstring(cPrefFileExtension).win_str(), saveAsName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter.win_str(), CSDIFrame::GetAppTopWindow());

	// Check for good reply
	if (dlgFile.DoModal() == IDOK)
	{

		// Reset prefs file object if same as existing
		if (mCurrentPrefsFile == CMulberryApp::sCurrentPrefsFile)
			mCurrentPrefsFile = NULL;

		// Create file object if required
		if (!mCurrentPrefsFile)
			mCurrentPrefsFile = new CPreferencesFile(false, false);

		// Save current prefs in file
		mCurrentPrefsFile->SetRemote(false);
		mCurrentPrefsFile->SetPrefs(&mCopyPrefs);
		mCurrentPrefsFile->DoSaveAs(dlgFile.GetPathName());
		mCurrentPrefsFile->SetSpecified(true);

		// Reset window title
		SetWindowTitle();

		return true;
	}
	else
		return false;
}

void CPreferencesDialog::PrefsLocalSaveDefault()
{
	// Force active page to update in case of existing changes
	UpdateCopyPrefs();

	// Prefs must be valid before save
	if (!mCopyPrefs.Valid(true))
		return;

	// Create new System Folder prefs file
	CPreferencesFile* temp = new CPreferencesFile(true, false);

	// Save current prefs in System Folder
	temp->SetPrefs(&mCopyPrefs);
	temp->SavePrefs(false, true);

	// Finished with file
	delete temp;
}

bool CPreferencesDialog::PrefsRemoteOpen()
{
	// Make sure set is read in
	// Must be logged in to server
	if (CMulberryApp::sApp->BeginRemote(&mCopyPrefs))
	{
		CMulberryApp::sRemotePrefs->ListRemoteSets();

		// Do logoff to kill connection
		CMulberryApp::sOptionsProtocol->Logoff();
	}
	else
		return false;

	// Create the dialog
	CTextListChoice dlog(CSDIFrame::GetAppTopWindow());
	dlog.mSingleSelection = true;
	dlog.mSelectFirst = true;
	dlog.mButtonTitle = "Open";
	dlog.mTextListDescription = "Remote Preferences:";

	// Add default name first
	dlog.mItems.push_back(cDefaultPrefsSetKey_2_0);

	// Add the rest
	for(cdstrvect::const_iterator iter = CMulberryApp::sRemotePrefs->GetRemoteSets().begin();
			iter != CMulberryApp::sRemotePrefs->GetRemoteSets().end(); iter++)
		dlog.mItems.push_back(*iter);

	// Let Dialog process events
	if (dlog.DoModal() == IDOK)
	{
		// Get selection from list
		cdstring remote = cDefaultPrefsSetKey_2_0;
		if (dlog.mSelection.front())
			remote = CMulberryApp::sRemotePrefs->GetRemoteSets().at(dlog.mSelection.front() - 1);

		// Save copy of existing preferences
		CPreferences temp(mCopyPrefs);

		// Create file object
		CPreferencesFile* new_file = new CPreferencesFile(false, true);

		// Set prefs file to new file and copy prefs
		new_file->SetSpecifier(remote);
		new_file->SetSpecified(false);
		new_file->SetPrefs(&mCopyPrefs);

		// Read in new prefs
		if (new_file->VerifyRead(false))
		{
			// Delete prefs file object if not same as existing
			if (mCurrentPrefsFile != CMulberryApp::sCurrentPrefsFile)
				delete mCurrentPrefsFile;
			mCurrentPrefsFile = new_file;

			// Need to sync new prefs simple/advanced state with old
			mCopyPrefs.mSimple.SetValue(temp.mSimple.GetValue());

			// Set new prefs values in all panels
			ResetPrefs();
			GetActivePage()->UpdateData(false);
			
			// Reset title
			SetWindowTitle();
		}
		else
		{
			delete new_file;

			// Restore existing prefs
			mCopyPrefs = temp;

			return false;
		}
		
		
		return true;
	}
	
	return false;
}

bool CPreferencesDialog::PrefsRemoteSaveAs()
{
	cdstring saveAsName;

	// Use current file if any
	if (mCurrentPrefsFile)
	{
		// If remote replace name
		saveAsName = mCurrentPrefsFile->GetSpecifier();
		if (!mCurrentPrefsFile->IsRemote())
		{
			cdstring temp;
			const char* p = ::strrchr(saveAsName.c_str(), '\\');
			if (p)
				temp = ++p;
			else
				temp = saveAsName;
			p = ::strrchr(temp.c_str(), '.');
			if (p)
				*const_cast<char*>(p) = 0;
			saveAsName = temp;
		}
	}

	// Make sure set is read in
	// Must be logged in to server
	if (CMulberryApp::sApp->BeginRemote(&mCopyPrefs))
	{
		CMulberryApp::sRemotePrefs->ListRemoteSets();

		// Do logoff to kill connection
		CMulberryApp::sOptionsProtocol->Logoff();
	}
	else
		return false;
	
	// Create the dialog
	CTextListChoice dlog(CSDIFrame::GetAppTopWindow());
	dlog.mSingleSelection = true;
	dlog.mUseEntry = true;
	dlog.mButtonTitle = "Save As...";
	dlog.mTextListDescription = "Remote Preferences:";
	dlog.mTextListEnter = saveAsName;
	dlog.mTextListEnterDescription = "Save Preferences As:";

	// Add default name first
	dlog.mItems.push_back(cDefaultPrefsSetKey_2_0);

	// Add the rest
	for(cdstrvect::const_iterator iter = CMulberryApp::sRemotePrefs->GetRemoteSets().begin();
			iter != CMulberryApp::sRemotePrefs->GetRemoteSets().end(); iter++)
		dlog.mItems.push_back(*iter);

	// Let Dialog process events
	while(true)
	{
		if (dlog.DoModal() == IDOK)
		{
			// Get selection from list
			saveAsName = dlog.mTextListEnter;

			// Check for duplicate
			cdstring duplicate;
			if (CMulberryApp::sRemotePrefs->IsDuplicateName(saveAsName, duplicate))
			{
				// Warn replace
				if (CErrorHandler::PutCautionAlertRsrcStr(true, "Alerts::Preferences::DuplicateSaveAsSet", duplicate) == CErrorHandler::Cancel)
					// Stop dialog OK
					continue;
			}

			// Login here so that all operations take place on a single connection
			StProtocolLogin login(CMulberryApp::sOptionsProtocol);

			// Delete any duplicate set first
			if (!duplicate.empty())
				CMulberryApp::sRemotePrefs->DeleteSet(duplicate);

			// Reset prefs file object if same as existing
			if (mCurrentPrefsFile == CMulberryApp::sCurrentPrefsFile)
				mCurrentPrefsFile = NULL;

			// Create file object if required
			if (!mCurrentPrefsFile)
				mCurrentPrefsFile = new CPreferencesFile(false, true);
			
			// Save current prefs in file
			mCurrentPrefsFile->SetRemote(true);
			mCurrentPrefsFile->SetSpecifier(saveAsName);
			mCurrentPrefsFile->SetSpecified(false);
			mCurrentPrefsFile->SetPrefs(&mCopyPrefs);
			mCurrentPrefsFile->SavePrefs(false, true);
			
			// Add to sets
			CMulberryApp::sRemotePrefs->GetRemoteSets().push_back(saveAsName);
			CMulberryApp::sRemotePrefs->UpdateRemoteSets();
			
			// Reset window title
			SetWindowTitle();
			
			return true;
		}
		else
			return false;
	}
}

void CPreferencesDialog::PrefsRemoteSaveDefault()
{
	// Delete existing set first if required
	if (::GetKeyState(VK_MENU) < 0)
	{
		// Must be logged in to server
		if (CMulberryApp::sApp->BeginRemote(GetCopyPrefs()))
		{
			// Delete default set
			cdstring set = cDefaultPrefsSetKey_2_0;
			CMulberryApp::sRemotePrefs->DeleteSet(set);

			// Do logoff to kill connection
			CMulberryApp::sOptionsProtocol->Logoff();
		}
	}

	// Create new default remote prefs file
	CPreferencesFile* temp = new CPreferencesFile(true, true);
	temp->CreateDefault();

	// Save current prefs as default remote
	temp->SetPrefs(&mCopyPrefs);
	temp->SavePrefs(false, true);
	
	// Finished with file
	delete temp;
}

