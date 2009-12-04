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


// CPrefsAddressCapture.cpp : implementation file
//

#include "CPrefsAddressCapture.h"

#include "CAddressBookManager.h"
#include "CPreferences.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddressCapture dialog

IMPLEMENT_DYNAMIC(CPrefsAddressCapture, CTabPanel)

CPrefsAddressCapture::CPrefsAddressCapture()
	: CTabPanel(CPrefsAddressCapture::IDD)
{
	//{{AFX_DATA_INIT(CPrefsAddressCapture)
	mCaptureAllowEdit = FALSE;
	mCaptureAllowChoice = FALSE;
	mCaptureRead = FALSE;
	mCaptureRespond = FALSE;
	mCaptureFrom = FALSE;
	mCaptureCc = FALSE;
	mCaptureReplyTo = FALSE;
	mCaptureTo = FALSE;
	//}}AFX_DATA_INIT
}


void CPrefsAddressCapture::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsAddressCapture)
	DDX_UTF8Text(pDX, IDC_PREFS_ADDRESS_CAPTUREADBK, mCaptureAddressBook);
	DDX_Control(pDX, IDC_PREFS_ADDRESS_CAPTUREADBK, mCaptureAddressBookCtrl);
	//DDX_Control(pDX, IDC_PREFS_ADDRESS_CAPTUREADBKPOPUP, mCapturePopup);
	DDX_Check(pDX, IDC_PREFS_ADDRESS_CAPTUREDISPLAYEDIT, mCaptureAllowEdit);
	DDX_Check(pDX, IDC_PREFS_ADDRESS_CAPTUREALLOWCHOICE, mCaptureAllowChoice);
	DDX_Check(pDX, IDC_PREFS_ADDRESS_CAPTUREREAD, mCaptureRead);
	DDX_Check(pDX, IDC_PREFS_ADDRESS_CAPTURERESPONSE, mCaptureRespond);
	DDX_Check(pDX, IDC_PREFS_ADDRESS_CAPTUREFROM, mCaptureFrom);
	DDX_Check(pDX, IDC_PREFS_ADDRESS_CAPTURECC, mCaptureCc);
	DDX_Check(pDX, IDC_PREFS_ADDRESS_CATPUREREPLYTO, mCaptureReplyTo);
	DDX_Check(pDX, IDC_PREFS_ADDRESS_CAPTURETO, mCaptureTo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsAddressCapture, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsAddressCapture)
	ON_COMMAND_RANGE(IDM_AddressCopyStart, IDM_AddressCopyEnd, OnChangeAdbk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddressCapture message handlers

BOOL CPrefsAddressCapture::OnInitDialog() 
{
	CTabPanel::OnInitDialog();
	
	// Subclass buttons
	mCapturePopup.SubclassDlgItem(IDC_PREFS_ADDRESS_CAPTUREADBKPOPUP, this, IDI_POPUPBTN);
	mCapturePopup.SetMenu(IDR_POPUP_ADBK_SERVERS);
	mCapturePopup.SetButtonText(false);
	InitAddressBookPopup();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Set data
void CPrefsAddressCapture::SetContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info
	mCaptureAddressBook = copyPrefs->mCaptureAddressBook.GetValue();
	mCaptureAllowEdit = copyPrefs->mCaptureAllowEdit.GetValue();
	mCaptureAllowChoice = copyPrefs->mCaptureAllowChoice.GetValue();
	mCaptureRead = copyPrefs->mCaptureRead.GetValue();
	mCaptureRespond = copyPrefs->mCaptureRespond.GetValue();
	mCaptureFrom = copyPrefs->mCaptureFrom.GetValue();
	mCaptureCc = copyPrefs->mCaptureCc.GetValue();
	mCaptureReplyTo = copyPrefs->mCaptureReplyTo.GetValue();
	mCaptureTo = copyPrefs->mCaptureTo.GetValue();
}

// Force update of data
bool CPrefsAddressCapture::UpdateContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	copyPrefs->mCaptureAddressBook.SetValue(mCaptureAddressBook);
	copyPrefs->mCaptureAllowEdit.SetValue(mCaptureAllowEdit);
	copyPrefs->mCaptureAllowChoice.SetValue(mCaptureAllowChoice);
	copyPrefs->mCaptureRead.SetValue(mCaptureRead);
	copyPrefs->mCaptureRespond.SetValue(mCaptureRespond);
	copyPrefs->mCaptureFrom.SetValue(mCaptureFrom);
	copyPrefs->mCaptureCc.SetValue(mCaptureCc);
	copyPrefs->mCaptureReplyTo.SetValue(mCaptureReplyTo);
	copyPrefs->mCaptureTo.SetValue(mCaptureTo);
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddressCapture message handlers

// Init the popup
void CPrefsAddressCapture::InitAddressBookPopup()
{
	CMenu* pPopup = mCapturePopup.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);

	// Must have address book manager at this point
	if (CAddressBookManager::sAddressBookManager)
	{
		// Get address book list using account names as opposed to urls
		cdstrvect names;
		CAddressBookManager::sAddressBookManager->GetCurrentAddressBookList(names, false);

		int menu_id = IDM_AddressCopyStart;
		for(cdstrvect::const_iterator iter = names.begin(); iter != names.end(); iter++, menu_id++)
		{
			// Empty name => separator for accounts
			if ((*iter).length() != 0)
				CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter).c_str());
			else
				CUnicodeUtils::AppendMenuUTF8(pPopup, MF_SEPARATOR);
		}
	}
}


// Change move to
void CPrefsAddressCapture::OnChangeAdbk(UINT nID)
{
	// Must set/reset control value to ensure selected mailbox is returned but
	// popup does not display check mark
	mCapturePopup.SetValue(nID);
	cdstring adbk_name = CUnicodeUtils::GetMenuStringUTF8(mCapturePopup.GetPopupMenu(), nID, MF_BYCOMMAND);
	CUnicodeUtils::SetWindowTextUTF8(&mCaptureAddressBookCtrl, adbk_name);
	mCapturePopup.SetValue(-1);
}