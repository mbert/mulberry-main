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


// Source for CRulesActionLocal class

#include "CRulesActionLocal.h"

#include "CAdminLock.h"
#include "CBounceActionDialog.h"
#include "CForwardActionDialog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CRejectActionDialog.h"
#include "CReplyActionDialog.h"
#include "CUnicodeUtils.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Menu items
enum
{
	eRules_SetFlag = IDM_RULES_ACTIONS_SETFLAG,
	eRules_UnsetFlag,
	eRules_Copy,
	eRules_Move,
	eRules_Reply,
	eRules_Forward,
	eRules_Bounce,
	eRules_Reject,
	eRules_Expunge,
	eRules_Print,
	eRules_Save,
	eRules_Sound,
	eRules_Alert,
	eRules_Speak
};

enum
{
	eRules_Flag_Seen = IDM_RULES_ACTIONS_FLAGSEEN,
	eRules_Flag_Important,
	eRules_Flag_Answered,
	eRules_Flag_Deleted,
	eRules_Flag_Draft,
	eRules_Flag_Separator1,
	eRules_Flag_Label1,
	eRules_Flag_Label2,
	eRules_Flag_Label3,
	eRules_Flag_Label4,
	eRules_Flag_Label5,
	eRules_Flag_Label6,
	eRules_Flag_Label7,
	eRules_Flag_Label8
};

BEGIN_MESSAGE_MAP(CRulesActionLocal, CRulesAction)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	
	ON_COMMAND_RANGE(IDM_RULES_ACTIONS_SETFLAG, IDM_RULES_ACTIONS_SPEAK, OnSetAction)
	ON_COMMAND_RANGE(IDM_RULES_ACTIONS_FLAGSEEN, IDM_RULES_ACTIONS_LABEL8, OnSetFlag)
	ON_COMMAND_RANGE(IDM_CopyToPopupNone, IDM_CopyToPopupEnd, OnMailboxPopup)
	ON_COMMAND(IDC_RULES_ACTIONS_OPTIONS, OnSetOptions)
	ON_COMMAND_RANGE(IDM_SOUND_Start, IDM_SOUND_End, OnSoundPopup)
END_MESSAGE_MAP()

// Default constructor
CRulesActionLocal::CRulesActionLocal()
{
}

// Default destructor
CRulesActionLocal::~CRulesActionLocal()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
BOOL CRulesActionLocal::Create(const CRect& rect, CRulesDialog* dlog, CWnd* pParentWnd, CView* parentView)
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

const int cPopup2Width = 128;
const int cPopup2Height = cPopupHeight;
const int cPopup2HOffset = cPopup1HOffset + cPopup1Width + 4;
const int cPopup2VOffset = cPopup1VOffset;

const int cText1Width = 256;
const int cText1Height = cEditHeight;
const int cText1HOffset = cPopup1HOffset + cPopup1Width + 4;
const int cText1VOffset = cPopup1VOffset;

const int cMailboxPopupWidth = 16;
const int cMailboxPopupHeight = 16;
const int cMailboxPopupHOffset = cText1HOffset + cText1Width + 4;
const int cMailboxPopupVOffset = cPopup1VOffset;

const int cSaveSingleWidth = 128;
const int cSaveSingleHeight = cCaptionHeight;
const int cSaveSingleHOffset = cPopup1HOffset + cPopup1Width + 4;
const int cSaveSingleVOffset = cPopup1VOffset;

const int cOptionsWidth = 64;
const int cOptionsHeight = cEditHeight;
const int cOptionsHOffset = cPopup1HOffset + cPopup1Width + 4;
const int cOptionsVOffset = cPopup1VOffset;

int CRulesActionLocal::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CRulesAction::OnCreate(lpCreateStruct) == -1)
		return -1;

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	// Popups
	CString s;
	CRect r = CRect(cPopup1HOffset, cPopup1VOffset, cPopup1HOffset + cPopup1Width + 2*large_offset, cPopup1VOffset + cPopup1Height + small_offset);
	mPopup1.Create(_T(""), r, &mHeader, IDC_RULES_ACTIONS_POPUP1, IDC_STATIC, IDI_POPUPBTN);
	mPopup1.SetMenu(IDR_POPUP_RULES_ACTIONS);
	mPopup1.SetValue(IDM_RULES_ACTIONS_SETFLAG);
	mPopup1.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mPopup1, CWndAlignment::eAlign_TopLeft));

	r = CRect(cPopup2HOffset + 2*large_offset, cPopup2VOffset, cPopup2HOffset + cPopup2Width + 2*large_offset, cPopup2VOffset + cPopup2Height + small_offset);
	mPopup2.Create(_T(""), r, &mHeader, IDC_RULES_ACTIONS_POPUP2, IDC_STATIC, IDI_POPUPBTN);
	mPopup2.SetMenu(IDR_POPUP_RULES_ACTIONS_FLAGS);
	mPopup2.SetValue(IDM_RULES_ACTIONS_FLAGSEEN);
	mPopup2.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mPopup2, CWndAlignment::eAlign_TopLeft));

	// Edit fields
	r = CRect(cText1HOffset + 2*large_offset, cText1VOffset, cText1HOffset + cText1Width + 2*large_offset, cText1VOffset + cText1Height + small_offset);
	mText.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL | ES_AUTOHSCROLL,
						r, &mHeader, IDC_RULES_ACTIONS_TEXT);
	mText.SetFont(CMulberryApp::sAppFont);
	mText.SetReturnCmd(true);
	mText.ShowWindow(SW_HIDE);
	mHeader.AddAlignment(new CWndAlignment(&mText, CWndAlignment::eAlign_TopWidth));

	r = CRect(cMailboxPopupHOffset + 2*large_offset, cMailboxPopupVOffset, cMailboxPopupHOffset + cMailboxPopupWidth + 2*large_offset, cMailboxPopupVOffset + cMailboxPopupHeight + small_offset);
	mMailboxPopup.Create(_T(""), r, &mHeader, IDC_RULES_ACTIONS_MAILBOXPOPUP, IDC_STATIC, IDI_POPUPBTN);
	mMailboxPopup.SetFont(CMulberryApp::sAppFont);
	mMailboxPopup.SetButtonText(false);
	mMailboxPopup.SetCopyTo(true);
	mMailboxPopup.ShowWindow(SW_HIDE);
	mHeader.AddAlignment(new CWndAlignment(&mMailboxPopup, CWndAlignment::eAlign_TopRight));

	s.LoadString(IDS_RULES_ACTIONS_SAVESINGLE);
	r = CRect(cSaveSingleHOffset, cSaveSingleVOffset, cSaveSingleHOffset + cSaveSingleWidth, cSaveSingleVOffset + cSaveSingleHeight + small_offset);
	mSaveSingle.Create(s, WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX, r, &mHeader, IDC_STATIC);
	mSaveSingle.SetFont(CMulberryApp::sAppFont);
	mHeader.AddAlignment(new CWndAlignment(&mSaveSingle, CWndAlignment::eAlign_TopLeft));

	s.LoadString(IDS_RULES_ACTIONS_OPTIONS);
	r = CRect(cOptionsHOffset + 2*large_offset, cOptionsVOffset, cOptionsHOffset + cOptionsWidth + 3*large_offset, cOptionsVOffset + cOptionsHeight + small_offset);
	mOptionsBtn.Create(s, WS_CHILD | WS_TABSTOP, r, &mHeader, IDC_RULES_ACTIONS_OPTIONS);
	mOptionsBtn.SetFont(CMulberryApp::sAppFont);
	mOptionsBtn.ShowWindow(SW_HIDE);
	mHeader.AddAlignment(new CWndAlignment(&mOptionsBtn, CWndAlignment::eAlign_TopLeft));

	r = CRect(cPopup2HOffset + 2*large_offset, cPopup2VOffset, cPopup2HOffset + cPopup2Width + 2*large_offset, cPopup2VOffset + cPopup2Height + small_offset);
	mSoundPopup.Create(_T(""), r, &mHeader, IDC_RULES_ACTIONS_SOUNDPOPUP, IDC_STATIC, IDI_POPUPBTN);
	mSoundPopup.SetMenu(IDR_POPUP_SOUND);
	mSoundPopup.SetCommandBase(IDM_SOUND_Start);
	mSoundPopup.SyncMenu();
	mSoundPopup.SetValue(IDM_SOUND_Start);
	mSoundPopup.SetFont(CMulberryApp::sAppFont);
	mSoundPopup.ShowWindow(SW_HIDE);
	mHeader.AddAlignment(new CWndAlignment(&mSoundPopup, CWndAlignment::eAlign_TopLeft));

	InitLabelNames();

	// Prevent reject action if locked out by admin
	if (!CAdminLock::sAdminLock.mAllowRejectCommand)
		mPopup1.GetPopupMenu()->EnableMenuItem(eRules_Reject, MF_GRAYED | MF_BYCOMMAND);

	return 0;
}

void CRulesActionLocal::InitLabelNames()
{
	// Change name of labels
	for(short i = eRules_Flag_Label1; i < eRules_Flag_Label1 + NMessage::eMaxLabels; i++)
		CUnicodeUtils::ModifyMenuUTF8(mPopup2.GetPopupMenu(), i, MF_BYCOMMAND | MF_STRING, i, CPreferences::sPrefs->mLabels.GetValue()[i - eRules_Flag_Label1]->name);
}

// Tell window to focus on this one
void CRulesActionLocal::OnSetFocus(CWnd* pOldWnd)
{
}

void CRulesActionLocal::OnSetAction(UINT nID)
{
	// Set popup menu for method and show/hide text field as approriate
	switch(nID)
	{
	case eRules_SetFlag:
	case eRules_UnsetFlag:
	default:
		mText.ShowWindow(SW_HIDE);
		mMailboxPopup.ShowWindow(SW_HIDE);
		mSaveSingle.ShowWindow(SW_HIDE);
		mOptionsBtn.ShowWindow(SW_HIDE);
		mPopup2.ShowWindow(SW_SHOW);
		mSoundPopup.ShowWindow(SW_HIDE);
		break;
	case eRules_Copy:
	case eRules_Move:
		mText.ShowWindow(SW_SHOW);
		mText.EnableWindow(true);
		mMailboxPopup.ShowWindow(SW_SHOW);
		mSaveSingle.ShowWindow(SW_HIDE);
		mOptionsBtn.ShowWindow(SW_HIDE);
		mPopup2.ShowWindow(SW_HIDE);
		mSoundPopup.ShowWindow(SW_HIDE);
		break;
	case eRules_Reply:
	case eRules_Forward:
	case eRules_Bounce:
	case eRules_Reject:
		mText.ShowWindow(SW_HIDE);
		mMailboxPopup.ShowWindow(SW_HIDE);
		mSaveSingle.ShowWindow(SW_HIDE);
		mOptionsBtn.ShowWindow(SW_SHOW);
		mPopup2.ShowWindow(SW_HIDE);
		mSoundPopup.ShowWindow(SW_HIDE);
		break;
	case eRules_Sound:
		mText.ShowWindow(SW_HIDE);
		mMailboxPopup.ShowWindow(SW_HIDE);
		mSaveSingle.ShowWindow(SW_HIDE);
		mOptionsBtn.ShowWindow(SW_HIDE);
		mPopup2.ShowWindow(SW_HIDE);
		mSoundPopup.ShowWindow(SW_SHOW);
		break;
	case eRules_Alert:
	case eRules_Speak:
		mText.ShowWindow(SW_SHOW);
		mText.EnableWindow(true);
		mMailboxPopup.ShowWindow(SW_HIDE);
		mSaveSingle.ShowWindow(SW_HIDE);
		mOptionsBtn.ShowWindow(SW_HIDE);
		mPopup2.ShowWindow(SW_HIDE);
		break;
	case eRules_Expunge:
	case eRules_Print:
		mText.ShowWindow(SW_HIDE);
		mMailboxPopup.ShowWindow(SW_HIDE);
		mSaveSingle.ShowWindow(SW_HIDE);
		mOptionsBtn.ShowWindow(SW_HIDE);
		mPopup2.ShowWindow(SW_HIDE);
		mSoundPopup.ShowWindow(SW_HIDE);
		break;
	case eRules_Save:
		mText.ShowWindow(SW_HIDE);
		mMailboxPopup.ShowWindow(SW_HIDE);
		mSaveSingle.ShowWindow(SW_SHOW);
		mOptionsBtn.ShowWindow(SW_HIDE);
		mPopup2.ShowWindow(SW_HIDE);
		mSoundPopup.ShowWindow(SW_HIDE);
		break;
	}
	
	// Set menu item
	mPopup1.SetValue(nID);
}

void CRulesActionLocal::OnSetFlag(UINT nID)
{
	// Set menu item
	mPopup2.SetValue(nID);
}

// Change copy to
void CRulesActionLocal::OnMailboxPopup(UINT nID)
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

void CRulesActionLocal::OnSetOptions()
{
	switch(mPopup1.GetValue())
	{
	default:
		return;
	case eRules_Reply:
		CReplyActionDialog::PoseDialog(mActionReply);
		return;
	case eRules_Forward:
		CForwardActionDialog::PoseDialog(mActionForward);
		return;
	case eRules_Bounce:
		CBounceActionDialog::PoseDialog(mActionBounce);
		return;
	case eRules_Reject:
		CRejectActionDialog::PoseDialog(mActionReject);
		return;
	}
}

void CRulesActionLocal::OnSoundPopup(UINT nID)
{
	mSoundPopup.SetValue(nID);
	::PlayNamedSound(mSoundPopup.GetSound());
}

NMessage::EFlags cMenuToFlagMap[] = {NMessage::eSeen,
										NMessage::eFlagged, 
										NMessage::eAnswered,
										NMessage::eDeleted,
										NMessage::eDraft, 
										NMessage::eNone,
										NMessage::eLabel1,
										NMessage::eLabel2,
										NMessage::eLabel3,
										NMessage::eLabel4,
										NMessage::eLabel5,
										NMessage::eLabel6,
										NMessage::eLabel7,
										NMessage::eLabel8
										};

CActionItem* CRulesActionLocal::GetActionItem() const
{
	CActionItem::EActionItem type = static_cast<CActionItem::EActionItem>(mPopup1.GetValue() - eRules_SetFlag + CActionItem::eFlagMessage);

	// Adjust for flag offset
	if (mPopup1.GetValue() >= eRules_UnsetFlag)
		type = static_cast<CActionItem::EActionItem>(type - 1);

	switch(mPopup1.GetValue())
	{
	case eRules_SetFlag:
	case eRules_UnsetFlag:
	{
		NMessage::EFlags flag = cMenuToFlagMap[mPopup2.GetValue() - IDM_RULES_ACTIONS_FLAGSEEN];

		return new CActionItem(type, flag, mPopup1.GetValue() == eRules_SetFlag);
	}
	case eRules_Copy:
	case eRules_Move:
	case eRules_Alert:
	case eRules_Speak:
	{
		cdstring txt;
		mText.GetText(txt);
		return new CActionItem(type, txt);
	}
	case eRules_Reply:
		return new CActionItem(type, mActionReply);
	case eRules_Forward:
		return new CActionItem(type, mActionForward);
	case eRules_Bounce:
		return new CActionItem(type, mActionBounce);
	case eRules_Reject:
		return new CActionItem(type, mActionReject);
	case eRules_Expunge:
	case eRules_Print:
		return new CActionItem(type);
	case eRules_Save:
		return new CActionItem(type, mSaveSingle.GetCheck() == 1);
	case eRules_Sound:
	{
		cdstring txt = mSoundPopup.GetSound();
		mActionSound.SetData(txt);
		return new CActionItem(type, mActionSound);
	}
	default:
		return NULL;
	}
}

void CRulesActionLocal::SetActionItem(const CActionItem* spec)
{
	long popup1 = eRules_SetFlag;
	long popup2 = eRules_Flag_Seen;
	cdstring text;
	bool flag = true;
	cdstring snd_popup;

	if (spec)
	{
		// Include flag unset offset
		popup1 = spec->GetActionItem() - CActionItem::eFlagMessage + eRules_UnsetFlag;
		switch(spec->GetActionItem())
		{
		case CActionItem::eFlagMessage:
			switch(spec->GetFlagData()->GetData().GetFlags())
			{
			case NMessage::eSeen:
				popup2 = eRules_Flag_Seen;
				break;
			case NMessage::eFlagged:
				popup2 = eRules_Flag_Important;
				break;
			case NMessage::eAnswered:
				popup2 = eRules_Flag_Answered;
				break;
			case NMessage::eDeleted:
				popup2 = eRules_Flag_Deleted;
				break;
			case NMessage::eDraft:
				popup2 = eRules_Flag_Draft;
				break;
			case NMessage::eLabel1:
				popup2 = eRules_Flag_Label1;
				break;
			case NMessage::eLabel2:
				popup2 = eRules_Flag_Label2;
				break;
			case NMessage::eLabel3:
				popup2 = eRules_Flag_Label3;
				break;
			case NMessage::eLabel4:
				popup2 = eRules_Flag_Label4;
				break;
			case NMessage::eLabel5:
				popup2 = eRules_Flag_Label5;
				break;
			case NMessage::eLabel6:
				popup2 = eRules_Flag_Label6;
				break;
			case NMessage::eLabel7:
				popup2 = eRules_Flag_Label7;
				break;
			case NMessage::eLabel8:
				popup2 = eRules_Flag_Label8;
				break;
			}
			
			// Readjust for flag set offset
			if (spec->GetFlagData()->GetData().IsSet())
				popup1--;
			break;
		case CActionItem::eCopyMessage:
		case CActionItem::eMoveMessage:
		case CActionItem::eAlert:
		case CActionItem::eSpeak:
			text = spec->GetStringData()->GetData();
			break;

		case CActionItem::eReplyMessage:
			mActionReply = spec->GetReplyData()->GetData();
			break;

		case CActionItem::eForwardMessage:
			mActionForward = spec->GetForwardData()->GetData();
			break;

		case CActionItem::eBounceMessage:
			mActionBounce = spec->GetBounceData()->GetData();
			break;

		case CActionItem::eRejectMessage:
			mActionReject = spec->GetRejectData()->GetData();
			break;

		case CActionItem::eSound:
			mActionSound = spec->GetOSStringMapData()->GetData();
			snd_popup = mActionSound.GetData();
			break;

		case CActionItem::eSave:
			mSaveSingle.SetCheck(spec->GetBoolData()->GetData());
			break;

		case CActionItem::eExpunge:
		case CActionItem::ePrint:
		default:;
		}
	}

	OnSetAction(popup1);
	mPopup2.SetValue(popup2);
	CUnicodeUtils::SetWindowTextUTF8(&mText, text);
	if (!snd_popup.empty())
		mSoundPopup.SetSound(snd_popup);
}
