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


// Source for CRulesTarget class

#include "CRulesTarget.h"

#include "CMailAccount.h"
#include "CMailAccountManager.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CTargetsDialog.h"
#include "CUnicodeUtils.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Menu items
enum
{
	ePopup_None = IDM_TARGET_NONE,
	ePopup_Mailbox,
	ePopup_Cabinet,
	ePopup_Account,
	ePopup_All
};

BEGIN_MESSAGE_MAP(CRulesTarget, CCriteriaBase)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	
	ON_COMMAND_RANGE(IDM_TARGET_NONE, IDM_TARGET_ALL, OnSetTarget)
	ON_COMMAND_RANGE(IDM_ALERTCABINET_Start, IDM_ALERTCABINET_End, OnSetCabinet)
	ON_COMMAND_RANGE(IDM_AccountStart, IDM_AccountStop, OnSetAccount)
	ON_COMMAND_RANGE(IDM_CopyToPopupNone, IDM_CopyToPopupEnd, OnMailboxPopup)
END_MESSAGE_MAP()

// Default constructor
CRulesTarget::CRulesTarget()
{
}

// Default destructor
CRulesTarget::~CRulesTarget()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
BOOL CRulesTarget::Create(const CRect& rect, CTargetsDialog* dlog, CWnd* pParentWnd, CView* parentView)
{
	mDlog = dlog;

	if (CRulesTarget::CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD, rect, pParentWnd, IDC_STATIC))
	{
		// Make sure context menus are active
		mText.SetContextView(parentView);
		return true;
	}
	else
		return false;
}

const int cCaptionHeight = 16;
const int cEditHeight = 22;
const int cPopupHeight = 22;

const int cMoveBtnWidth = 16;
const int cMoveBtnHeight = 12;
const int cMoveBtnHOffset = 0;
const int cMoveUpBtnVOffset = 1;
const int cMoveDownBtnVOffset = cMoveUpBtnVOffset + cMoveBtnHeight;

const int cPopup1Width = 122;
const int cPopup1Height = cPopupHeight;
const int cPopup1HOffset = cMoveBtnHOffset + cMoveBtnWidth + 4;
const int cPopup1VOffset = 2;

const int cPopup2Width = 122;
const int cPopup2Height = cPopupHeight;
const int cPopup2HOffset = cPopup1HOffset + cPopup1Width + 4;
const int cPopup2VOffset = 2;

const int cPopup3Width = 122;
const int cPopup3Height = cPopupHeight;
const int cPopup3HOffset = cPopup1HOffset + cPopup1Width + 4;
const int cPopup3VOffset = 2;

const int cText1Width = 256;
const int cText1Height = cEditHeight;
const int cText1HOffset = cPopup1HOffset + cPopup1Width + 4;
const int cText1VOffset = cPopup1VOffset;

const int cMailboxPopupWidth = 16;
const int cMailboxPopupHeight = 16;
const int cMailboxPopupHOffset = cText1HOffset + cText1Width + 4;
const int cMailboxPopupVOffset = cPopup1VOffset;

int CRulesTarget::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CCriteriaBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	// Popups
	CString s;
	CRect r = CRect(cPopup1HOffset, cPopup1VOffset, cPopup1HOffset + cPopup1Width, cPopup1VOffset + cPopup1Height + small_offset);
	mPopup1.Create(_T(""), r, &mHeader, IDC_RULES_ACTIONS_POPUP1, IDC_STATIC, IDI_POPUPBTN);
	mPopup1.SetMenu(IDR_POPUP_TARGETS);
	mPopup1.SetValue(IDM_TARGET_NONE);
	mPopup1.SetFont(CMulberryApp::sAppFont);

	r = CRect(cPopup2HOffset, cPopup2VOffset, cPopup2HOffset + cPopup2Width, cPopup2VOffset + cPopup2Height + small_offset);
	mPopup2.Create(_T(""), r, &mHeader, IDC_RULES_ACTIONS_POPUP1, IDC_STATIC, IDI_POPUPBTN);
	mPopup2.SetMenu(IDR_POPUP_ALERTCABINETS);
	mPopup2.SetValue(IDM_ALERTCABINET_Start);
	mPopup2.SetFont(CMulberryApp::sAppFont);
	mPopup2.ShowWindow(SW_HIDE);
	InitCabinetMenu();

	r = CRect(cPopup3HOffset, cPopup3VOffset, cPopup3HOffset + cPopup3Width, cPopup3VOffset + cPopup3Height + small_offset);
	mPopup3.Create(_T(""), r, &mHeader, IDC_RULES_ACTIONS_POPUP1, IDC_STATIC, IDI_POPUPBTN);
	mPopup3.SetMenu(IDR_POPUP_MAILBOX_SEARCH_ACCOUNTS);
	mPopup3.SetFont(CMulberryApp::sAppFont);
	mPopup3.ShowWindow(SW_HIDE);
	InitAccountMenu();
	mPopup3.SetValue(IDM_AccountStart);

	// Edit fields
	r = CRect(cText1HOffset, cText1VOffset, cText1HOffset + cText1Width, cText1VOffset + cText1Height + small_offset);
	mText.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL | ES_AUTOHSCROLL,
						r, &mHeader, IDC_RULES_ACTIONS_TEXT);
	mText.SetFont(CMulberryApp::sAppFont);
	mText.SetReturnCmd(true);
	mText.ShowWindow(SW_HIDE);

	r = CRect(cMailboxPopupHOffset, cMailboxPopupVOffset, cMailboxPopupHOffset + cMailboxPopupWidth, cMailboxPopupVOffset + cMailboxPopupHeight + small_offset);
	mMailboxPopup.Create(_T(""), r, &mHeader, IDC_RULES_ACTIONS_MAILBOXPOPUP, IDC_STATIC, IDI_POPUPBTN);
	mMailboxPopup.SetFont(CMulberryApp::sAppFont);
	mMailboxPopup.SetButtonText(false);
	mMailboxPopup.SetCopyTo(true);
	mMailboxPopup.ShowWindow(SW_HIDE);

	return 0;
}

// Tell window to focus on this one
void CRulesTarget::OnSetFocus(CWnd* pOldWnd)
{
}

void CRulesTarget::OnSetTarget(UINT nID)
{
	// Set popup menu for method and show/hide text field as approriate
	switch(nID - ePopup_None + CFilterTarget::eNone)
	{
	case CFilterTarget::eNone:
	case CFilterTarget::eAny:
	default:
		mPopup2.ShowWindow(SW_HIDE);
		mPopup3.ShowWindow(SW_HIDE);
		mText.ShowWindow(SW_HIDE);
		mMailboxPopup.ShowWindow(SW_HIDE);
		break;
	case CFilterTarget::eMailbox:
		mPopup2.ShowWindow(SW_HIDE);
		mPopup3.ShowWindow(SW_HIDE);
		mText.ShowWindow(SW_SHOW);
		mMailboxPopup.ShowWindow(SW_SHOW);
		break;
	case CFilterTarget::eCabinet:
		mPopup2.ShowWindow(SW_SHOW);
		mPopup3.ShowWindow(SW_HIDE);
		mText.ShowWindow(SW_HIDE);
		mMailboxPopup.ShowWindow(SW_HIDE);
		break;
	case CFilterTarget::eAccount:
		mPopup2.ShowWindow(SW_HIDE);
		mPopup3.ShowWindow(SW_SHOW);
		mText.ShowWindow(SW_HIDE);
		mMailboxPopup.ShowWindow(SW_HIDE);
		break;
	}
	
	// Set menu item
	mPopup1.SetValue(nID);
}

void CRulesTarget::OnSetCabinet(UINT nID)
{
	// Set menu item
	mPopup2.SetValue(nID);
}

void CRulesTarget::OnSetAccount(UINT nID)
{
	// Set menu item
	mPopup3.SetValue(nID);
}

// Change copy to
void CRulesTarget::OnMailboxPopup(UINT nID)
{
	if (nID == IDM_CopyToPopupNone)
		CUnicodeUtils::SetWindowTextUTF8(&mText, cdstring::null_str);
	else
	{
		// Must set/reset control value to ensure selected mailbox is returned but
		// popup does not display check mark
		mMailboxPopup.SetValue(nID);
		cdstring mbox_name;
		if (mMailboxPopup.GetSelectedMboxName(mbox_name, true))
			CUnicodeUtils::SetWindowTextUTF8(&mText, mbox_name);
		mMailboxPopup.SetValue(-1);
	}
}

CFilterTarget* CRulesTarget::GetFilterTarget() const
{
	CFilterTarget::ETarget target = static_cast<CFilterTarget::ETarget>(mPopup1.GetValue() - ePopup_None + CFilterTarget::eNone);
	switch(target)
	{
		case CFilterTarget::eNone:
		case CFilterTarget::eAny:
			return new CFilterTarget(target);
		case CFilterTarget::eMailbox:
		{
			cdstring txt;
			mText.GetText(txt);
			return new CFilterTarget(target, txt);
		}
		case CFilterTarget::eCabinet:
		{
			CMailAccountManager::EFavourite favtype = static_cast<CMailAccountManager::EFavourite>(mPopup2.GetValue() - IDM_ALERTCABINET_Start);
			cdstring txt = CMailAccountManager::sMailAccountManager->GetFavouriteID(favtype);
			return new CFilterTarget(target, txt);
		}
		case CFilterTarget::eAccount:
		{
			cdstring acct_name;
			UINT acct_num = mPopup3.GetValue() - IDM_AccountStart;
			acct_name = CPreferences::sPrefs->mMailAccounts.GetValue().at(acct_num)->GetName();
			return new CFilterTarget(target, acct_name);
		}
		default:
			return NULL;
	}
}

void CRulesTarget::SetFilterTarget(const CFilterTarget* spec)
{
	long popup1 = ePopup_None;
	long popup2 = IDM_ALERTCABINET_Start;
	long popup3 = IDM_AccountStart;
	cdstring text;

	if (spec)
	{
		popup1 = spec->GetTarget() + ePopup_None;
		switch(spec->GetTarget())
		{
		case CFilterTarget::eNone:
		case CFilterTarget::eAny:
			break;
		case CFilterTarget::eMailbox:
			text = spec->GetStringData()->GetData();
			break;
		case CFilterTarget::eCabinet:
		{
			CMailAccountManager::EFavourite favtype = CMailAccountManager::sMailAccountManager->GetFavouriteFromID(spec->GetStringData()->GetData());
			popup2 = favtype + IDM_ALERTCABINET_Start;
		}
		case CFilterTarget::eAccount:
		{
			CMboxProtocol* proto = CMailAccountManager::sMailAccountManager->GetProtocol(spec->GetStringData()->GetData());
			if (proto)
				popup3 =  CMailAccountManager::sMailAccountManager->GetProtocolIndex(proto) + IDM_AccountStart;
		}
		default:;
		}
	}

	OnSetTarget(popup1);
	mPopup2.SetValue(popup2);
	mPopup3.SetValue(popup3);
	CUnicodeUtils::SetWindowTextUTF8(&mText, text);
}

CCriteriaBaseList& CRulesTarget::GetList()
{
	return mDlog->GetTargets();
}

void CRulesTarget::SwitchWith(CCriteriaBase* other)
{
	CFilterTarget* this_target = GetFilterTarget();
	SetFilterTarget(static_cast<CRulesTarget*>(other)->GetFilterTarget());
	static_cast<CRulesTarget*>(other)->SetFilterTarget(this_target);
}

// Set up cabinet menu
void CRulesTarget::InitCabinetMenu()
{
	CMenu* pPopup = mPopup2.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = CMailAccountManager::eFavouriteOthers; i < num_menu; i++)
		pPopup->RemoveMenu(CMailAccountManager::eFavouriteOthers, MF_BYPOSITION);

	int menu_id = CMailAccountManager::eFavouriteOthers + IDM_ALERTCABINET_Start;
	for(CFavouriteItemList::const_iterator iter = CPreferences::sPrefs->mFavourites.GetValue().begin() + CMailAccountManager::eFavouriteOthers;
			iter != CPreferences::sPrefs->mFavourites.GetValue().end(); iter++, menu_id++)
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter).GetName());
}

void CRulesTarget::InitAccountMenu()
{
	CMenu* pPopup = mPopup3.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);

	// Now add current items
	int menu_id = IDM_AccountStart;
	for(CMailAccountList::const_iterator iter = CPreferences::sPrefs->mMailAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mMailAccounts.GetValue().end(); iter++)
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id++, (*iter)->GetName());
}

