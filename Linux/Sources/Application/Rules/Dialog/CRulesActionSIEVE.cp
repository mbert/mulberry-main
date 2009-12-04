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
#include "CMailboxPopup.h"
#include "CTextField.h"
#include "CVacationActionDialog.h"

#include "TPopupMenu.h"

#include "JXMultiImageButton.h"
#include <JXTextButton.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Menu items
enum
{
	ePopup_Keep = 1,
	ePopup_Discard,
	ePopup_Reject,
	ePopup_Redirect,
	ePopup_FileInto,
	//ePopup_Separator,
	ePopup_Vacation
};

const char* cRulesActionsSIEVE = 
	"Keep %r| Discard %r| Reject %r| Redirect %r| File into %r%l| Vacation %r";

// Default constructor
CRulesActionSIEVE::CRulesActionSIEVE(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) :
	CRulesAction(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Default destructor
CRulesActionSIEVE::~CRulesActionSIEVE()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CRulesActionSIEVE::OnCreate(CRulesDialog* dlog)
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
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 25,0, 120,20);
    assert( mPopup1 != NULL );

    mOptionsBtn =
        new JXTextButton("Options", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 150,0, 100,20);
    assert( mOptionsBtn != NULL );
    mOptionsBtn->SetFontSize(10);

    mText =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 150,0, 302,20);
    assert( mText != NULL );

    mMailboxPopup =
        new CMailboxPopupButton(true, this,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 456,3, 30,16);
    assert( mMailboxPopup != NULL );

// end JXLayout1

	mUp->SetImage(IDI_MOVEUP);
	mDown->SetImage(IDI_MOVEDOWN);

	mPopup1->SetMenuItems(cRulesActionsSIEVE);
	mPopup1->SetValue(ePopup_Keep);

	// Show/hide items
	mText->Hide();
	mMailboxPopup->Hide();
	mOptionsBtn->Hide();

	// Listen to certain items
	ListenTo(mPopup1);
	ListenTo(mMailboxPopup);
	ListenTo(mOptionsBtn);

	CRulesAction::OnCreate(dlog);
	SetBtns(mUp, mDown);
}

// Tell window to focus on this one
void CRulesActionSIEVE::Receive(JBroadcaster* sender, const Message& message)
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

void CRulesActionSIEVE::OnSetAction(JIndex nID)
{
	// Set popup menu for method and show/hide text field as approriate
	switch(nID)
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

void CRulesActionSIEVE::OnOptionsBtn()
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

// Change copy to
void CRulesActionSIEVE::OnMailboxPopup(JIndex nID)
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
			cdstring copyStr;
			if (::strchr(mbox_name.c_str(), cMailAccountSeparator))
				copyStr = ::strchr(mbox_name.c_str(), cMailAccountSeparator) + 1;
			else
				copyStr = mbox_name.c_str();
			mText->SetText(copyStr);
		}
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
		cdstring text = mText->GetText();

		CActionItem::EActionItem type1;
		switch(mPopup1->GetValue())
		{
		case ePopup_Reject:
			type1 = CActionItem::eReject;
			break;
		case ePopup_Redirect:
			type1 = CActionItem::eRedirect;
			break;
		case ePopup_FileInto:
			type1 = CActionItem::eFileInto;
			break;
		default:;
		}
		return new CActionItem(type1, text);
	}
	case ePopup_Vacation:
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

	mPopup1->SetValue(popup1);
	OnSetAction(popup1);
	mText->SetText(text);
}
