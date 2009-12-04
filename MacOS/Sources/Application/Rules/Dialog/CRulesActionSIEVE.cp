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
	ePopup_Separator1,
	ePopup_Vacation
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
	mText = (CTextFieldX*) FindPaneByID(paneid_RulesActionSText);
	mMailboxPopup = (CMailboxPopup*) FindPaneByID(paneid_RulesActionSMailboxPopup);
	mMailboxPopup->SetCopyTo(true);
	mOptionsBtn = (LPushButton*) FindPaneByID(paneid_RulesActionSTextBtn);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CRulesActionSIEVEBtns);
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
		mText->Hide();
		mMailboxPopup->Hide();
		mOptionsBtn->Hide();
		break;
	case ePopup_Reject:
	case ePopup_Redirect:
		mText->Show();
		mMailboxPopup->Hide();
		mOptionsBtn->Hide();
		break;
	case ePopup_FileInto:
		mText->Show();
		mMailboxPopup->Show();
		mOptionsBtn->Hide();
		break;
	case ePopup_Vacation:
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
	case ePopup_Vacation:
		return new CActionItem(CActionItem::eVacation, mActionVacation);
	default:
		return NULL;
	}
}

void CRulesActionSIEVE::SetActionItem(const CActionItem* spec)
{
	long popup1 = 1;
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

	mPopup1->SetValue(popup1);
	mText->SetText(text);
}
