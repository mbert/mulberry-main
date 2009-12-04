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
#include "CMailboxPopup.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CRejectActionDialog.h"
#include "CReplyActionDialog.h"
#include "CRulesDialog.h"
#include "CSoundPopup.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>
#include <LPopupButton.h>
#include <LPushButton.h>

enum
{
	eActionMenu_SetFlag = 1,
	eActionMenu_UnsetFlag,
	eActionMenu_CopyMessage,
	eActionMenu_MoveMessage,
	eActionMenu_ReplyMessage,
	eActionMenu_ForwardMessage,
	eActionMenu_BounceMessage,
	eActionMenu_RejectMessage,
	eActionMenu_ExpungeMessage,
	eActionMenu_Print,
	eActionMenu_Save,
	eActionMenu_PlaySound,
	eActionMenu_Alert,
	eActionMenu_Speak
};

enum
{
	eFlagMenu_Seen = 1,
	eFlagMenu_Flagged,
	eFlagMenu_Answered,
	eFlagMenu_Deleted,
	eFlagMenu_Draft,
	eFlagMenu_Separator1,
	eFlagMenu_Label1,
	eFlagMenu_Label2,
	eFlagMenu_Label3,
	eFlagMenu_Label4,
	eFlagMenu_Label5,
	eFlagMenu_Label6,
	eFlagMenu_Label7,
	eFlagMenu_Label8
};

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CRulesActionLocal::CRulesActionLocal()
{
}

// Constructor from stream
CRulesActionLocal::CRulesActionLocal(LStream *inStream)
		: CRulesAction(inStream)
{
}

// Default destructor
CRulesActionLocal::~CRulesActionLocal()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CRulesActionLocal::FinishCreateSelf(void)
{
	// Do inherited
	CRulesAction::FinishCreateSelf();

	// Get controls
	mPopup1 = (LPopupButton*) FindPaneByID(paneid_RulesActionPopup1);
	mPopup2 = (LPopupButton*) FindPaneByID(paneid_RulesActionPopup2);
	mText = (CTextFieldX*) FindPaneByID(paneid_RulesActionText);
	mMailboxPopup = (CMailboxPopup*) FindPaneByID(paneid_RulesActionMailboxPopup);
	mMailboxPopup->SetCopyTo(true);
	mSaveSingle = (LCheckBox*) FindPaneByID(paneid_RulesActionSaveSingle);
	mOptionsBtn = (LPushButton*) FindPaneByID(paneid_RulesActionTextBtn);
	mSoundPopup = (CSoundPopup*) FindPaneByID(paneid_RulesActionSoundPopup);

	InitLabelNames();

	// Prevent reject action if locked out by admin
	if (!CAdminLock::sAdminLock.mAllowRejectCommand)
		::DisableItem(mPopup1->GetMacMenuH(), eActionMenu_RejectMessage);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CRulesActionLocalBtns);
}

void CRulesActionLocal::InitLabelNames()
{
	// Change name of labels
	for(short i = eFlagMenu_Label1; i < eFlagMenu_Label1 + NMessage::eMaxLabels; i++)
	{
		::SetMenuItemTextUTF8(mPopup2->GetMacMenuH(), i, CPreferences::sPrefs->mLabels.GetValue()[i - eFlagMenu_Label1]->name);
	}

}

// Handle buttons
void CRulesActionLocal::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_RulesActionPopup1:
		OnSetAction(*(long*) ioParam);
		break;

	case msg_RulesActionTextBtn:
		OnSetOptions();
		break;

	case msg_RulesActionMailboxPopup:
	{
		cdstring mbox_name;
		if (mMailboxPopup->GetSelectedMboxName(mbox_name))
		{
			if (mbox_name.empty())
			{
				mText->SetText(cdstring::null_str);
			}
			else if (*mbox_name.c_str() != 0x01)
			{
				mText->SetText(mbox_name);
			}
		}
		break;
	}

	case msg_RulesActionSoundPopup:
	{
		cdstring title;
		mSoundPopup->GetName(title);
		::PlayNamedSound(title);
		break;
	}

	default:
		CRulesAction::ListenToMessage(inMessage, ioParam);
		break;
	}
}

bool CRulesActionLocal::DoActivate()
{
	CTextFieldX* activate = NULL;
	if (mText->IsVisible())
		activate = mText;

	if (activate)
	{
		activate->GetSuperCommander()->SetLatentSub(activate);
		LCommander::SwitchTarget(activate);
		activate->SelectAll();

		return true;
	}
	else
		return false;
}

void CRulesActionLocal::OnSetAction(long item1)
{
	// Set popup menu for method and show/hide text field as approriate
	bool method_refresh = false;
	switch(item1)
	{
	case eActionMenu_SetFlag:
	case eActionMenu_UnsetFlag:
	default:
		mText->Hide();
		mMailboxPopup->Hide();
		mSaveSingle->Hide();
		mOptionsBtn->Hide();
		mPopup2->Show();
		mSoundPopup->Hide();
		break;
	case eActionMenu_CopyMessage:
	case eActionMenu_MoveMessage:
		mText->Show();
		mMailboxPopup->Show();
		mSaveSingle->Hide();
		mOptionsBtn->Hide();
		mPopup2->Hide();
		mSoundPopup->Hide();
		break;
	case eActionMenu_ReplyMessage:
	case eActionMenu_ForwardMessage:
	case eActionMenu_BounceMessage:
	case eActionMenu_RejectMessage:
		mText->Hide();
		mMailboxPopup->Hide();
		mSaveSingle->Hide();
		mOptionsBtn->Show();
		mPopup2->Hide();
		mSoundPopup->Hide();
		break;
	case eActionMenu_PlaySound:
		mText->Hide();
		mMailboxPopup->Hide();
		mSaveSingle->Hide();
		mOptionsBtn->Hide();
		mPopup2->Hide();
		mSoundPopup->Show();
		break;
	case eActionMenu_Alert:
	case eActionMenu_Speak:
		mText->Show();
		mMailboxPopup->Hide();
		mSaveSingle->Hide();
		mOptionsBtn->Hide();
		mPopup2->Hide();
		mSoundPopup->Hide();
		break;
	case eActionMenu_ExpungeMessage:
	case eActionMenu_Print:
		mText->Hide();
		mMailboxPopup->Hide();
		mSaveSingle->Hide();
		mOptionsBtn->Hide();
		mPopup2->Hide();
		mSoundPopup->Hide();
		break;
	case eActionMenu_Save:
		mText->Hide();
		mMailboxPopup->Hide();
		mSaveSingle->Show();
		mOptionsBtn->Hide();
		mPopup2->Hide();
		mSoundPopup->Hide();
		break;
	}
}

void CRulesActionLocal::OnSetOptions()
{
	switch(mPopup1->GetValue())
	{
	default:
		return;
	case eActionMenu_ReplyMessage:
		CReplyActionDialog::PoseDialog(mActionReply);
		return;
	case eActionMenu_ForwardMessage:
		CForwardActionDialog::PoseDialog(mActionForward);
		return;
	case eActionMenu_BounceMessage:
		CBounceActionDialog::PoseDialog(mActionBounce);
		return;
	case eActionMenu_RejectMessage:
		CRejectActionDialog::PoseDialog(mActionReject);
		return;
	}
}

NMessage::EFlags cMenuToFlagMap[] = {NMessage::eNone,
										NMessage::eSeen,
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
	CActionItem::EActionItem type = static_cast<CActionItem::EActionItem>(mPopup1->GetValue());
	
	// Adjust for flag offset
	if (mPopup1->GetValue() >= eActionMenu_UnsetFlag)
		type = static_cast<CActionItem::EActionItem>(type - 1);

	switch(mPopup1->GetValue())
	{
	case eActionMenu_SetFlag:
	case eActionMenu_UnsetFlag:
	{
		NMessage::EFlags flag = cMenuToFlagMap[mPopup2->GetValue()];

		return new CActionItem(type, flag, mPopup1->GetValue() == eActionMenu_SetFlag);
	}
	case eActionMenu_CopyMessage:
	case eActionMenu_MoveMessage:
	case eActionMenu_Alert:
	case eActionMenu_Speak:
	{
		return new CActionItem(type, mText->GetText());
	}
	case eActionMenu_ReplyMessage:
		return new CActionItem(type, mActionReply);
	case eActionMenu_ForwardMessage:
		return new CActionItem(type, mActionForward);
	case eActionMenu_BounceMessage:
		return new CActionItem(type, mActionBounce);
	case eActionMenu_RejectMessage:
		return new CActionItem(type, mActionReject);
	case eActionMenu_ExpungeMessage:
	case eActionMenu_Print:
		return new CActionItem(type);
	case eActionMenu_Save:
		return new CActionItem(type, mSaveSingle->GetValue() == 1);
	case eActionMenu_PlaySound:
	{
		cdstring txt;
		mSoundPopup->GetName(txt);
		mActionSound.SetData(txt);
		return new CActionItem(type, mActionSound);
	}
	default:
		return NULL;
	}
}

void CRulesActionLocal::SetActionItem(const CActionItem* spec)
{
	long popup1 = 1;
	long popup2 = 1;
	cdstring text;
	bool flag = true;
	cdstring snd_popup;

	if (spec)
	{
		// Include flag unset offset
		popup1 = spec->GetActionItem() + 1;
		switch(spec->GetActionItem())
		{
		case CActionItem::eFlagMessage:
			popup2 = 1;
			switch(spec->GetFlagData()->GetData().GetFlags())
			{
			case NMessage::eSeen:
				popup2 = eFlagMenu_Seen;
				break;
			case NMessage::eFlagged:
				popup2 = eFlagMenu_Flagged;
				break;
			case NMessage::eAnswered:
				popup2 = eFlagMenu_Answered;
				break;
			case NMessage::eDeleted:
				popup2 = eFlagMenu_Deleted;
				break;
			case NMessage::eDraft:
				popup2 = eFlagMenu_Draft;
				break;
			case NMessage::eLabel1:
				popup2 = eFlagMenu_Label1;
				break;
			case NMessage::eLabel2:
				popup2 = eFlagMenu_Label2;
				break;
			case NMessage::eLabel3:
				popup2 = eFlagMenu_Label3;
				break;
			case NMessage::eLabel4:
				popup2 = eFlagMenu_Label4;
				break;
			case NMessage::eLabel5:
				popup2 = eFlagMenu_Label5;
				break;
			case NMessage::eLabel6:
				popup2 = eFlagMenu_Label6;
				break;
			case NMessage::eLabel7:
				popup2 = eFlagMenu_Label7;
				break;
			case NMessage::eLabel8:
				popup2 = eFlagMenu_Label8;
				break;
			default:;
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
			mSaveSingle->SetValue(spec->GetBoolData()->GetData());
			break;

		case CActionItem::eExpunge:
		case CActionItem::ePrint:
		default:;
		}
	}

	mPopup1->SetValue(popup1);
	mPopup2->SetValue(popup2);
	mText->SetText(text);
	if (!snd_popup.empty())
	{
		StopListening();
		mSoundPopup->SetName(snd_popup);
		StartListening();
	}
}
