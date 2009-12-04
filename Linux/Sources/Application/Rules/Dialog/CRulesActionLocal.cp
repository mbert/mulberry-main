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
#include "CMailAccount.h"
#include "CMailboxPopup.h"
#include "CPreferences.h"
#include "CRejectActionDialog.h"
#include "CReplyActionDialog.h"
#include "CTextField.h"

#include "TPopupMenu.h"

#include "JXMultiImageButton.h"
#include <JXTextButton.h>
#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Menu items
enum
{
	eRules_SetFlag = 1,
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

const char* cRulesActions = 
	"Set Flag %r| Unset Flag %r| "
	"Copy Message to %r| Move Message to %r| "
	"Reply to Message %r| Forward Message %r| Bounce Message %r| Reject Message %r |Expunge Message %r| "
	"Print %r|Save %r| Play Sound %r| Display Alert with %l %r| Speak with %r";

enum
{
	eRules_Flag_Seen = 1,
	eRules_Flag_Important,
	eRules_Flag_Answered,
	eRules_Flag_Deleted,
	eRules_Flag_Draft,
	//eRules_Flag_Separator,
	eRules_Flag_Label1,
	eRules_Flag_Label2,
	eRules_Flag_Label3,
	eRules_Flag_Label4,
	eRules_Flag_Label5,
	eRules_Flag_Label6,
	eRules_Flag_Label7,
	eRules_Flag_Label8
};

const char* cRulesActionsFlags = 
	"Seen %r| Important %r| Answered %r| Deleted %r| Draft %r %l | "
	"Label1 %r | Label2 %r | Label3 %r | Label4 %r | Label5 %r | Label6 %r | Label7 %r | Label8 %r";

// Default constructor
CRulesActionLocal::CRulesActionLocal(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) :
	CRulesAction(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Default destructor
CRulesActionLocal::~CRulesActionLocal()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CRulesActionLocal::OnCreate(CRulesDialog* dlog)
{
// begin JXLayout1

    mUp =
        new JXMultiImageButton(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 3,0, 15,10);
    assert( mUp != NULL );

    mDown =
        new JXMultiImageButton(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 3,10, 15,10);
    assert( mDown != NULL );

    mPopup1 =
        new HPopupMenu("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 25,0, 160,20);
    assert( mPopup1 != NULL );

    mPopup2 =
        new HPopupMenu("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 190,0, 100,20);
    assert( mPopup2 != NULL );

    mOptionsBtn =
        new JXTextButton("Options", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 190,0, 100,20);
    assert( mOptionsBtn != NULL );
    mOptionsBtn->SetFontSize(10);

    mText =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 190,0, 262,20);
    assert( mText != NULL );

    mMailboxPopup =
        new CMailboxPopupButton(true, this,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 456,3, 30,16);
    assert( mMailboxPopup != NULL );

    mSaveSingle =
        new JXTextCheckbox("Save All to Single File", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 189,2, 159,16);
    assert( mSaveSingle != NULL );
    mSaveSingle->SetFontSize(10);

// end JXLayout1

	mUp->SetImage(IDI_MOVEUP);
	mDown->SetImage(IDI_MOVEDOWN);

	mPopup1->SetMenuItems(cRulesActions);
	mPopup1->SetValue(eRules_SetFlag);
	mPopup2->SetMenuItems(cRulesActionsFlags);
	mPopup2->SetValue(eRules_Flag_Seen);

	// Show/hide items
	mText->Hide();
	mMailboxPopup->Hide();
	mSaveSingle->Hide();
	mOptionsBtn->Hide();

	InitLabelNames();

	// Prevent reject action if locked out by admin
	if (!CAdminLock::sAdminLock.mAllowRejectCommand)
		mPopup1->DisableItem(eRules_Reject);

	// Listen to certain items
	ListenTo(mPopup1);
	ListenTo(mMailboxPopup);
	ListenTo(mOptionsBtn);

	CRulesAction::OnCreate(dlog);
	SetBtns(mUp, mDown);
}

void CRulesActionLocal::InitLabelNames()
{
	// Change name of labels
	for(short i = eRules_Flag_Label1; i < eRules_Flag_Label1 + NMessage::eMaxLabels; i++)
	{
		cdstring labelName(CPreferences::sPrefs->mLabels.GetValue()[i - eRules_Flag_Label1]->name);
		mPopup2->SetItemText(i, labelName);
	}
}

// Tell window to focus on this one
void CRulesActionLocal::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXMenu::kItemSelected))
	{
		JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
    	if (sender == mPopup1)
    	{
			OnSetAction(index);
			return;
		}
    	else if (sender == mMailboxPopup)
    	{
			OnMailboxPopup(index);
			return;
		}
	}
	else if (message.Is(JXButton::kPushed))
	{
    	if (sender == mOptionsBtn)
    	{
			OnOptionsBtn();
			return;
		}
	}

	CRulesAction::Receive(sender, message);
}

void CRulesActionLocal::OnSetAction(JIndex nID)
{
	// Set popup menu for method and show/hide text field as approriate
	switch(nID)
	{
	case eRules_SetFlag:
	case eRules_UnsetFlag:
	default:
		mText->Hide();
		mMailboxPopup->Hide();
		mSaveSingle->Hide();
		mOptionsBtn->Hide();
		mPopup2->Show();
		break;
	case eRules_Copy:
	case eRules_Move:
		mText->Show();
		mMailboxPopup->Show();
		mSaveSingle->Hide();
		mOptionsBtn->Hide();
		mPopup2->Hide();
		break;
	case eRules_Reply:
	case eRules_Forward:
	case eRules_Bounce:
	case eRules_Reject:
		mText->Hide();
		mMailboxPopup->Hide();
		mSaveSingle->Hide();
		mOptionsBtn->Show();
		mPopup2->Hide();
		break;
	case eRules_Sound:
	case eRules_Alert:
	case eRules_Speak:
		mText->Show();
		mMailboxPopup->Hide();
		mSaveSingle->Hide();
		mOptionsBtn->Hide();
		mPopup2->Hide();
		break;
	case eRules_Expunge:
	case eRules_Print:
		mText->Hide();
		mMailboxPopup->Hide();
		mSaveSingle->Hide();
		mOptionsBtn->Hide();
		mPopup2->Hide();
		break;
	case eRules_Save:
		mText->Hide();
		mMailboxPopup->Hide();
		mSaveSingle->Show();
		mOptionsBtn->Hide();
		mPopup2->Hide();
		break;
	}
}

// Change copy to
void CRulesActionLocal::OnMailboxPopup(JIndex nID)
{
	cdstring mbox_name;
	if (mMailboxPopup->GetSelectedMboxName(mbox_name))
		mText->SetText(mbox_name);
}

void CRulesActionLocal::OnOptionsBtn()
{
	switch(mPopup1->GetValue())
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

NMessage::EFlags cMenuToFlagMap[] = {NMessage::eSeen,
										NMessage::eFlagged, 
										NMessage::eAnswered,
										NMessage::eDeleted,
										NMessage::eDraft, 
										//NMessage::eNone,
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
	CActionItem::EActionItem type = static_cast<CActionItem::EActionItem>(mPopup1->GetValue() - eRules_SetFlag + CActionItem::eFlagMessage);

	// Adjust for flag set/unset offset
	if (type > CActionItem::eFlagMessage)
		type = static_cast<CActionItem::EActionItem>(type - 1);

	switch(type)
	{
	case CActionItem::eFlagMessage:
	{
		NMessage::EFlags flag = cMenuToFlagMap[mPopup2->GetValue() - eRules_Flag_Seen];

		return new CActionItem(type, flag, mPopup1->GetValue() == eRules_SetFlag);
	}
	case CActionItem::eCopyMessage:
	case CActionItem::eMoveMessage:
	case CActionItem::eAlert:
	case CActionItem::eSpeak:
	{
		cdstring text = mText->GetText();
		return new CActionItem(type, text);
	}
	case CActionItem::eReplyMessage:
		return new CActionItem(type, mActionReply);
	case CActionItem::eForwardMessage:
		return new CActionItem(type, mActionForward);
	case CActionItem::eBounceMessage:
		return new CActionItem(type, mActionBounce);
	case CActionItem::eRejectMessage:
		return new CActionItem(type, mActionReject);
	case CActionItem::eExpunge:
	case CActionItem::ePrint:
		return new CActionItem(type);
	case CActionItem::eSave:
		return new CActionItem(type, mSaveSingle->IsChecked());
	case CActionItem::eSound:
	{
		cdstring text = mText->GetText();
		mActionSound.SetData(text);
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

	if (spec)
	{
		popup1 = spec->GetActionItem() - CActionItem::eFlagMessage + eRules_SetFlag;
		// Adjust for flag set/unset offset
		if (popup1 > eRules_SetFlag)
			popup1++;

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
			default:;
			}
			if (!spec->GetFlagData()->GetData().IsSet())
				popup1++;
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
			text = mActionSound.GetData();
			break;

		case CActionItem::eSave:
			mSaveSingle->SetState(JBoolean(spec->GetBoolData()->GetData()));
			break;

		case CActionItem::eExpunge:
		case CActionItem::ePrint:
		default:;
		}
	}

	mPopup1->SetValue(popup1);
	OnSetAction(popup1);
	mPopup2->SetValue(popup2);
	mText->SetText(text);
}
