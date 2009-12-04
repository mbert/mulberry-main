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

#include "CMailAccountManager.h"
#include "CMailboxPopup.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CRulesDialog.h"
#include "CTextFieldX.h"

#include <LPopupButton.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CRulesTarget::CRulesTarget()
{
}

// Constructor from stream
CRulesTarget::CRulesTarget(LStream *inStream)
		: CCriteriaBase(inStream)
{
}

// Default destructor
CRulesTarget::~CRulesTarget()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CRulesTarget::FinishCreateSelf(void)
{
	// Do inherited
	CCriteriaBase::FinishCreateSelf();

	// Get controls
	mPopup1 = (LPopupButton*) FindPaneByID(paneid_RulesTargetPopup1);
	mPopup2 = (LPopupButton*) FindPaneByID(paneid_RulesTargetPopup2);
	mPopup3 = (LPopupButton*) FindPaneByID(paneid_RulesTargetPopup3);
	mText = (CTextFieldX*) FindPaneByID(paneid_RulesTargetText);
	mMailboxPopup = (CMailboxPopup*) FindPaneByID(paneid_RulesTargetMailboxPopup);
	mMailboxPopup->SetCopyTo(true);

	InitCabinetMenu();
	InitAccountMenu();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CRulesTargetBtns);
}

// Handle buttons
void CRulesTarget::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_RulesTargetPopup1:
			OnSetTarget(*(long*) ioParam);
			break;

		case msg_RulesTargetMailboxPopup:
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

		default:
			CCriteriaBase::ListenToMessage(inMessage, ioParam);
			break;
	}
}

// Set up cabinet menu
void CRulesTarget::InitCabinetMenu(void)
{
	// Remove any existing items from main menu
	MenuHandle menuH = mPopup2->GetMacMenuH();
	for(short i = ::CountMenuItems(menuH); i > CMailAccountManager::eFavouriteOthers + 1; i--)
		::DeleteMenuItem(menuH, i);

	short index = CMailAccountManager::eFavouriteOthers + 1;
	for(CFavouriteItemList::const_iterator iter = CPreferences::sPrefs->mFavourites.GetValue().begin() + CMailAccountManager::eFavouriteOthers;
			iter != CPreferences::sPrefs->mFavourites.GetValue().end(); iter++, index++)
		::AppendItemToMenu(menuH, index, (*iter).GetName());

	// Force max/min update
	mPopup2->SetMenuMinMax();
}

// Set up account menu
void CRulesTarget::InitAccountMenu(void)
{
	// Delete previous items
	MenuHandle menuH = mPopup3->GetMacMenuH();
	for(short i = ::CountMenuItems(menuH); i >= 1; i--)
		::DeleteMenuItem(menuH, i);

	// Add each mail account
	short menu_pos = 1;
	for(CMailAccountList::const_iterator iter = CPreferences::sPrefs->mMailAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mMailAccounts.GetValue().end(); iter++, menu_pos++)
		::AppendItemToMenu(menuH, menu_pos, (*iter)->GetName());

	// Force max/min update
	mPopup3->SetMenuMinMax();
}

bool CRulesTarget::DoActivate()
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

void CRulesTarget::SwitchWith(CCriteriaBase* other)
{
	CFilterTarget* this_target = GetFilterTarget();
	SetFilterTarget(static_cast<CRulesTarget*>(other)->GetFilterTarget());
	static_cast<CRulesTarget*>(other)->SetFilterTarget(this_target);
}

void CRulesTarget::OnSetTarget(long item1)
{
	// Set popup menu for method and show/hide text field as approriate
	switch(item1 - 1)
	{
	case CFilterTarget::eNone:
	case CFilterTarget::eAny:
	default:
		mPopup2->Hide();
		mPopup3->Hide();
		mText->Hide();
		mMailboxPopup->Hide();
		break;
	case CFilterTarget::eMailbox:
		mPopup2->Hide();
		mPopup3->Hide();
		mText->Show();
		mMailboxPopup->Show();
		break;
	case CFilterTarget::eCabinet:
		mPopup2->Show();
		mPopup3->Hide();
		mText->Hide();
		mMailboxPopup->Hide();
		break;
	case CFilterTarget::eAccount:
		mPopup2->Hide();
		mPopup3->Show();
		mText->Hide();
		mMailboxPopup->Hide();
		break;
	}
}

CFilterTarget* CRulesTarget::GetFilterTarget() const
{
	CFilterTarget::ETarget target = static_cast<CFilterTarget::ETarget>(mPopup1->GetValue() - 1);
	switch(target)
	{
		case CFilterTarget::eNone:
		case CFilterTarget::eAny:
			return new CFilterTarget(target);
		case CFilterTarget::eMailbox:
		{
			return new CFilterTarget(target, mText->GetText());
		}
		case CFilterTarget::eCabinet:
		{
			CMailAccountManager::EFavourite favtype = static_cast<CMailAccountManager::EFavourite>(mPopup2->GetValue() - 1);
			cdstring txt = CMailAccountManager::sMailAccountManager->GetFavouriteID(favtype);
			return new CFilterTarget(target, txt);
		}
		case CFilterTarget::eAccount:
		{
			return new CFilterTarget(target, ::GetPopupMenuItemTextUTF8(mPopup3));
		}
		default:
			return NULL;
	}
}

void CRulesTarget::SetFilterTarget(const CFilterTarget* spec)
{
	long popup1 = 1;
	long popup2 = 1;
	long popup3 = 1;
	cdstring text;

	if (spec)
	{
		popup1 = spec->GetTarget() + 1;
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
			popup2 = favtype + 1;
		}
		case CFilterTarget::eAccount:
		{
			CMboxProtocol* proto = CMailAccountManager::sMailAccountManager->GetProtocol(spec->GetStringData()->GetData());
			if (proto)
				popup3 =  CMailAccountManager::sMailAccountManager->GetProtocolIndex(proto);
		}
		default:;
		}
	}

	mPopup1->SetValue(popup1);
	mPopup2->SetValue(popup2);
	mPopup3->SetValue(popup3);
	mText->SetText(text);
}
