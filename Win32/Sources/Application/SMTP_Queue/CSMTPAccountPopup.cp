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


// Source for CSMTPAccountPopup class

#include "CSMTPAccountPopup.h"

#include "CMulberryApp.h"
#include "CSMTPAccountManager.h"
#include "CSMTPSender.h"
#include "CUnicodeUtils.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

BEGIN_MESSAGE_MAP(CSMTPAccountPopup, CGrayBackground)
	//{{AFX_MSG_MAP(CGrayBackground)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// Constructor from stream
CSMTPAccountPopup::CSMTPAccountPopup(UINT cmdID, const cdstring& title) :
	mAccountsPopup(true, false)
{
	mCmdID = cmdID;
	mTitleStr = title;
}

// Default destructor
CSMTPAccountPopup::~CSMTPAccountPopup()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

const int cCaptionHeight = 16;

const int cAccountsWidth = 128;
const int cAccountsHeight = 22;

const int cAccountsTitleWidth = 48;
const int cAccountsTitleVOffset = 5;

int CSMTPAccountPopup::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGrayBackground::OnCreate(lpCreateStruct) == -1)
		return -1;

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	const int cAccountsTitleWidth1 = cAccountsTitleWidth + large_offset;
	const int cAccountsTitleVOffset1 = cAccountsTitleVOffset + 2*small_offset;

	const int cAccountsHeight1 = cAccountsHeight + small_offset;

	// Title
	CRect rect(0, cAccountsTitleVOffset1, cAccountsTitleWidth1, cAccountsTitleVOffset1 + cCaptionHeight + small_offset);
	CString s;
	mTitle.Create(s, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	mTitle.SetFont(CMulberryApp::sAppFont);
	CUnicodeUtils::SetWindowTextUTF8(&mTitle, mTitleStr);

	// Popup
	rect = CRect(cAccountsTitleWidth1, 0, cAccountsTitleWidth1 + cAccountsWidth, cAccountsHeight1);
	mAccountsPopup.Create(_T(""), rect, this, mCmdID, IDC_STATIC, IDI_POPUPBTN);
	mAccountsPopup.SetMenu(IDR_POPUP_SMTP_ACCOUNTS);
	mAccountsPopup.SetFont(CMulberryApp::sAppFont);
	InitAccountPopup();

	// Icon
	rect = CRect(cAccountsTitleWidth1 + cAccountsWidth + 4, 4, cAccountsTitleWidth1 + cAccountsWidth + 16 + 4, 4 + 16);
	mSecure.Create(NULL, WS_CHILD | WS_VISIBLE | SS_ICON, rect, this, IDC_SMTPSECURE);
	mSecure.SetIconID(IDI_ENCRYPT);

	return 0;
}

void CSMTPAccountPopup::InitAccountPopup()
{
	CMenu* pPopup = mAccountsPopup.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);

	// Add each SMTP account
	int menu_id = IDM_SMTP_ACCOUNTS_Start;
	int first_enabled = 0;
	CSMTPSenderList& senders = CSMTPAccountManager::sSMTPAccountManager->GetSMTPSenders();
	for(CSMTPSenderList::const_iterator iter = senders.begin(); iter != senders.end(); iter++, menu_id++)
	{
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter)->GetAccountName());
		
		// Disable if not queued
		if (!static_cast<CSMTPAccount*>((*iter)->GetAccount())->GetUseQueue())
			pPopup->EnableMenuItem(menu_id, MF_BYCOMMAND | MF_GRAYED);
		else if (!first_enabled)
			first_enabled = menu_id;
	}

	// Set initial value and fake hit
	mAccountsPopup.SetValue(first_enabled);
}

