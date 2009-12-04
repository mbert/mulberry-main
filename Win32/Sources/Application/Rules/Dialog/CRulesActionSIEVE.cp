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


// Source for CRulesActionSIEVE class

#include "CRulesActionSIEVE.h"

#include "CMailAccount.h"
#include "CMulberryApp.h"
#include "CUnicodeUtils.h"
#include "CVacationActionDialog.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Menu items
enum
{
	ePopup_Keep = IDM_SIEVE_ACTIONS_KEEP,
	ePopup_Discard,
	ePopup_Reject,
	ePopup_Redirect,
	ePopup_FileInto,
	ePopup_Vacation
};

BEGIN_MESSAGE_MAP(CRulesActionSIEVE, CRulesAction)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	
	ON_COMMAND_RANGE(IDM_SIEVE_ACTIONS_KEEP, IDM_SIEVE_ACTIONS_VACATION, OnSetAction)
	ON_COMMAND(IDC_RULES_ACTIONS_OPTIONS, OnSetOptions)
	ON_COMMAND_RANGE(IDM_CopyToPopupNone, IDM_CopyToPopupEnd, OnMailboxPopup)
END_MESSAGE_MAP()

// Default constructor
CRulesActionSIEVE::CRulesActionSIEVE()
{
}

// Default destructor
CRulesActionSIEVE::~CRulesActionSIEVE()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
BOOL CRulesActionSIEVE::Create(const CRect& rect, CRulesDialog* dlog, CWnd* pParentWnd, CView* parentView)
{
	if (CRulesAction::Create(rect, dlog, pParentWnd, parentView))
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

const int cText1Width = 256;
const int cText1Height = cEditHeight;
const int cText1HOffset = cPopup1HOffset + cPopup1Width + 4;
const int cText1VOffset = cPopup1VOffset;

const int cMailboxPopupWidth = 16;
const int cMailboxPopupHeight = 16;
const int cMailboxPopupHOffset = cText1HOffset + cText1Width + 4;
const int cMailboxPopupVOffset = cPopup1VOffset;

const int cOptionsWidth = 64;
const int cOptionsHeight = cEditHeight;
const int cOptionsHOffset = cPopup1HOffset + cPopup1Width + 4;
const int cOptionsVOffset = cPopup1VOffset;

int CRulesActionSIEVE::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CRulesAction::OnCreate(lpCreateStruct) == -1)
		return -1;

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	// Popups
	CString s;
	CRect r = CRect(cPopup1HOffset, cPopup1VOffset, cPopup1HOffset + cPopup1Width, cPopup1VOffset + cPopup1Height + small_offset);
	mPopup1.Create(_T(""), r, &mHeader, IDC_RULES_ACTIONS_POPUP1, IDC_STATIC, IDI_POPUPBTN);
	mPopup1.SetMenu(IDR_POPUP_SIEVE_ACTIONS);
	mPopup1.SetValue(IDM_SIEVE_ACTIONS_KEEP);
	mPopup1.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mPopup1, CWndAlignment::eAlign_TopLeft));

	// Edit fields
	r = CRect(cText1HOffset, cText1VOffset, cText1HOffset + cText1Width, cText1VOffset + cText1Height + small_offset);
	mText.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL | ES_AUTOHSCROLL,
						r, &mHeader, IDC_RULES_ACTIONS_TEXT);
	mText.SetFont(CMulberryApp::sAppFont);
	mText.SetReturnCmd(true);
	mText.ShowWindow(SW_HIDE);
	mHeader.AddAlignment(new CWndAlignment(&mText, CWndAlignment::eAlign_TopWidth));

	r = CRect(cMailboxPopupHOffset, cMailboxPopupVOffset, cMailboxPopupHOffset + cMailboxPopupWidth, cMailboxPopupVOffset + cMailboxPopupHeight + small_offset);
	mMailboxPopup.Create(_T(""), r, &mHeader, IDC_RULES_ACTIONS_MAILBOXPOPUP, IDC_STATIC, IDI_POPUPBTN);
	mMailboxPopup.SetFont(CMulberryApp::sAppFont);
	mMailboxPopup.SetButtonText(false);
	mMailboxPopup.SetCopyTo(true);
	mMailboxPopup.ShowWindow(SW_HIDE);
	mHeader.AddAlignment(new CWndAlignment(&mMailboxPopup, CWndAlignment::eAlign_TopRight));

	s.LoadString(IDS_RULES_ACTIONS_OPTIONS);
	r = CRect(cOptionsHOffset, cOptionsVOffset, cOptionsHOffset + cOptionsWidth, cOptionsVOffset + cOptionsHeight + small_offset);
	mOptionsBtn.Create(s, WS_CHILD | WS_TABSTOP, r, &mHeader, IDC_RULES_ACTIONS_OPTIONS);
	mOptionsBtn.SetFont(CMulberryApp::sAppFont);
	mOptionsBtn.ShowWindow(SW_HIDE);
	mHeader.AddAlignment(new CWndAlignment(&mOptionsBtn, CWndAlignment::eAlign_TopLeft));

	return 0;
}

// Tell window to focus on this one
void CRulesActionSIEVE::OnSetFocus(CWnd* pOldWnd)
{
}

void CRulesActionSIEVE::OnSetAction(UINT nID)
{
	// Set popup menu for method and show/hide text field as approriate
	switch(nID)
	{
	case ePopup_Keep:
	case ePopup_Discard:
	default:
		mText.ShowWindow(SW_HIDE);
		mMailboxPopup.ShowWindow(SW_HIDE);
		mOptionsBtn.ShowWindow(SW_HIDE);
		break;
	case ePopup_Reject:
	case ePopup_Redirect:
		mText.ShowWindow(SW_SHOW);
		mMailboxPopup.ShowWindow(SW_HIDE);
		mOptionsBtn.ShowWindow(SW_HIDE);
		break;
	case ePopup_FileInto:
		mText.ShowWindow(SW_SHOW);
		mMailboxPopup.ShowWindow(SW_SHOW);
		mOptionsBtn.ShowWindow(SW_HIDE);
		break;
	case ePopup_Vacation:
		mText.ShowWindow(SW_HIDE);
		mMailboxPopup.ShowWindow(SW_HIDE);
		mOptionsBtn.ShowWindow(SW_SHOW);
		break;
	}
	
	// Set menu item
	mPopup1.SetValue(nID);
}

void CRulesActionSIEVE::OnSetOptions()
{
	switch(mPopup1.GetValue())
	{
	default:
		return;
	case ePopup_Vacation:
		CVacationActionDialog::PoseDialog(mActionVacation);
		return;
	}
}

// Change copy to
void CRulesActionSIEVE::OnMailboxPopup(UINT nID)
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
		{
			// Don't add account prefix
			const char* p = ::strchr(mbox_name.c_str(), cMailAccountSeparator);
			if (p)
				p++;
			else
				p = mbox_name.c_str();
			CUnicodeUtils::SetWindowTextUTF8(&mText, p);
		}
		mMailboxPopup.SetValue(-1);
	}
}

CActionItem* CRulesActionSIEVE::GetActionItem() const
{
	CActionItem::EActionItem type = static_cast<CActionItem::EActionItem>(mPopup1.GetValue() - ePopup_Keep + CActionItem::eKeep);
	switch(type)
	{
	case CActionItem::eKeep:
		return new CActionItem(CActionItem::eKeep);
	case CActionItem::eDiscard:
		return new CActionItem(CActionItem::eDiscard);
	case CActionItem::eReject:
	case CActionItem::eRedirect:
	case CActionItem::eFileInto:
	{
		cdstring txt;
		mText.GetText(txt);

		CActionItem::EActionItem type1;
		switch(type)
		{
		case CActionItem::eReject:
			type1 = CActionItem::eReject;
			break;
		case CActionItem::eRedirect:
			type1 = CActionItem::eRedirect;
			break;
		case CActionItem::eFileInto:
			type1 = CActionItem::eFileInto;
			break;
		}
		return new CActionItem(type1, txt);
	}
	case CActionItem::eVacation:
		return new CActionItem(CActionItem::eVacation, mActionVacation);
	default:
		return NULL;
	}
}

void CRulesActionSIEVE::SetActionItem(const CActionItem* spec)
{
	long popup1 = ePopup_Keep;
	cdstring text;

	if (spec)
	{
		switch(spec->GetActionItem())
		{
		case CActionItem::eKeep:
		default:;
			popup1 = ePopup_Keep;
			break;

		case CActionItem::eDiscard:
			popup1 = ePopup_Discard;
			break;

		case CActionItem::eReject:
			popup1 = ePopup_Reject;
			text = spec->GetStringData()->GetData();
			break;

		case CActionItem::eRedirect:
			popup1 = ePopup_Redirect;
			text = spec->GetStringData()->GetData();
			break;

		case CActionItem::eFileInto:
			popup1 = ePopup_FileInto;
			text = spec->GetStringData()->GetData();
			break;

		case CActionItem::eVacation:
			popup1 = ePopup_Vacation;
			mActionVacation = spec->GetVacationData()->GetData();
			break;
		}
	}

	OnSetAction(popup1);
	CUnicodeUtils::SetWindowTextUTF8(&mText, text);
}
