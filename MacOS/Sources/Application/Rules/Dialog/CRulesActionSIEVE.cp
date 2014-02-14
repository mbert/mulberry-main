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

#include "CMailboxPopup.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CRulesDialog.h"
#include "CTextFieldX.h"
#include "CVacationActionDialog.h"

#include <LPopupButton.h>
#include <LPushButton.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

enum
{
	ePopup_Keep = 1,
	ePopup_Discard,
	ePopup_Reject,
	ePopup_Redirect,
	ePopup_FileInto,
	ePopup_SetFlag,
	ePopup_Separator1,
	ePopup_Vacation
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


// Default constructor
CRulesActionSIEVE::CRulesActionSIEVE()
{
}

// Constructor from stream
CRulesActionSIEVE::CRulesActionSIEVE(LStream *inStream)
		: CRulesAction(inStream)
{
}

// Default destructor
CRulesActionSIEVE::~CRulesActionSIEVE()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CRulesActionSIEVE::FinishCreateSelf(void)
{
	// Do inherited
	CRulesAction::FinishCreateSelf();

	// Get controls
	mPopup1 = (LPopupButton*) FindPaneByID(paneid_RulesActionSPopup1);
	mPopup2 = (LPopupButton*) FindPaneByID(paneid_RulesActionSPopup2);
	mText = (CTextFieldX*) FindPaneByID(paneid_RulesActionSText);
	mMailboxPopup = (CMailboxPopup*) FindPaneByID(paneid_RulesActionSMailboxPopup);
	mMailboxPopup->SetCopyTo(true);
	mOptionsBtn = (LPushButton*) FindPaneByID(paneid_RulesActionSTextBtn);
    
	InitLabelNames();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CRulesActionSIEVEBtns);
}

void CRulesActionSIEVE::InitLabelNames()
{
	// Change name of labels
	for(short i = eFlagMenu_Label1; i < eFlagMenu_Label1 + NMessage::eMaxLabels; i++)
	{
		::SetMenuItemTextUTF8(mPopup2->GetMacMenuH(), i, CPreferences::sPrefs->mLabels.GetValue()[i - eFlagMenu_Label1]->name);
	}
    
}

// Handle buttons
void CRulesActionSIEVE::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_RulesActionSPopup1:
			OnSetAction(*(long*) ioParam);
			break;

		case msg_RulesActionSTextBtn:
			OnSetOptions();
			break;

		case msg_RulesActionSMailboxPopup:
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
					// Don't add account prefix
					const char* p = ::strchr(mbox_name.c_str(), cMailAccountSeparator);
					if (p)
						p++;
					else
						p = mbox_name.c_str();
					mText->SetText(p);
				}
			}
			break;
		}

		default:
			CRulesAction::ListenToMessage(inMessage, ioParam);
			break;
	}
}

bool CRulesActionSIEVE::DoActivate()
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

void CRulesActionSIEVE::OnSetAction(long item1)
{
	// Set popup menu for method and show/hide text field as approriate
	switch(item1)
	{
        case ePopup_Keep:
        case ePopup_Discard:
        default:
            mPopup2->Hide();
            mText->Hide();
            mMailboxPopup->Hide();
            mOptionsBtn->Hide();
            break;
        case ePopup_Reject:
        case ePopup_Redirect:
            mPopup2->Hide();
            mText->Show();
            mMailboxPopup->Hide();
            mOptionsBtn->Hide();
            break;
        case ePopup_FileInto:
            mPopup2->Hide();
            mText->Show();
            mMailboxPopup->Show();
            mOptionsBtn->Hide();
            break;
        case ePopup_SetFlag:
            mPopup2->Show();
            mText->Hide();
            mMailboxPopup->Hide();
            mOptionsBtn->Hide();
            break;
        case ePopup_Vacation:
            mPopup2->Hide();
            mText->Hide();
            mMailboxPopup->Hide();
            mOptionsBtn->Show();
            break;
	}
}

void CRulesActionSIEVE::OnSetOptions()
{
	switch(mPopup1->GetValue())
	{
	default:
		return;
	case ePopup_Vacation:
		CVacationActionDialog::PoseDialog(mActionVacation);
		return;
	}
}

NMessage::EFlags cMenuToSetFlagMap[] = {NMessage::eNone,
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

CActionItem* CRulesActionSIEVE::GetActionItem() const
{
	switch(mPopup1->GetValue())
	{
        case ePopup_Keep:
            return new CActionItem(CActionItem::eKeep);
        case ePopup_Discard:
            return new CActionItem(CActionItem::eDiscard);
        case ePopup_Reject:
        case ePopup_Redirect:
        case ePopup_FileInto:
        {
            CActionItem::EActionItem type;
            switch(mPopup1->GetValue())
            {
                case ePopup_Reject:
                    type = CActionItem::eReject;
                    break;
                case ePopup_Redirect:
                    type = CActionItem::eRedirect;
                    break;
                case ePopup_FileInto:
                    type = CActionItem::eFileInto;
                    break;
            }
            return new CActionItem(type, mText->GetText());
        }
        case ePopup_SetFlag:
        {
            NMessage::EFlags flag = cMenuToSetFlagMap[mPopup2->GetValue()];
            return new CActionItem(CActionItem::eSetFlag, flag, true);
        }
        case ePopup_Vacation:
            return new CActionItem(CActionItem::eVacation, mActionVacation);
        default:
            return NULL;
	}
}

void CRulesActionSIEVE::SetActionItem(const CActionItem* spec)
{
	long popup1 = 1;
	long popup2 = 1;
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
                
            case CActionItem::eSetFlag:
                popup1 = ePopup_SetFlag ;
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
                break;
            case CActionItem::eVacation:
                popup1 = ePopup_Vacation;
                mActionVacation = spec->GetVacationData()->GetData();
                break;
		}
	}
    
	mPopup1->SetValue(popup1);
	mPopup2->SetValue(popup2);
	mText->SetText(text);
}
