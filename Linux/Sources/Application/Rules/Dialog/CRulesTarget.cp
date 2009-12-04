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
#include "CMailboxPopup.h"
#include "CPreferences.h"
#include "CTargetsDialog.h"
#include "CTextField.h"

#include "TPopupMenu.h"

#include "JXMultiImageButton.h"
#include <JXTextButton.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Menu items
enum
{
	ePopup_None = 1,
	ePopup_Mailbox,
	ePopup_Cabinet,
	ePopup_Account,
	ePopup_All
};

const char* cTargets = 
	"None %r| Mailbox %r| Cabinet %r| Account %r| All %r";

// Default constructor
CRulesTarget::CRulesTarget(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) :
	CCriteriaBase(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Default destructor
CRulesTarget::~CRulesTarget()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CRulesTarget::OnCreate(CTargetsDialog* dlog)
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

    mPopup2 =
        new HPopupMenu("",this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 150,0, 200,20);
    assert( mPopup2 != NULL );

    mPopup3 =
        new HPopupMenu("",this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 150,0, 200,20);
    assert( mPopup3 != NULL );

    mText =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 150,0, 232,20);
    assert( mText != NULL );

    mMailboxPopup =
        new CMailboxPopupButton(true, this,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 386,3, 30,16);
    assert( mMailboxPopup != NULL );

// end JXLayout1

	mDlog = dlog;

	mUp->SetImage(IDI_MOVEUP);
	mDown->SetImage(IDI_MOVEDOWN);

	mPopup1->SetMenuItems(cTargets);
	mPopup1->SetValue(ePopup_None);
	InitCabinetMenu();
	InitAccountMenu();

	// Show/hide items
	mPopup2->Hide();
	mPopup3->Hide();
	mText->Hide();
	mMailboxPopup->Hide();

	// Listen to certain items
	ListenTo(mPopup1);
	ListenTo(mMailboxPopup);

	SetBtns(mUp, mDown);
}

// Tell window to focus on this one
void CRulesTarget::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXMenu::kItemSelected))
	{
		JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
    	if (sender == mPopup1)
    	{
			OnSetTarget(index);
			return;
		}
    	else if (sender == mMailboxPopup)
    	{
			OnMailboxPopup(index);
			return;
		}
	}

	CCriteriaBase::Receive(sender, message);
}

void CRulesTarget::OnSetTarget(JIndex nID)
{
	// Set popup menu for method and show/hide text field as approriate
	switch(nID - ePopup_None + CFilterTarget::eNone)
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

// Change copy to
void CRulesTarget::OnMailboxPopup(JIndex nID)
{
	cdstring mbox_name;
	if (mMailboxPopup->GetSelectedMboxName(mbox_name))
		mText->SetText(mbox_name);
}

CFilterTarget* CRulesTarget::GetFilterTarget() const
{
	CFilterTarget::ETarget target = static_cast<CFilterTarget::ETarget>(mPopup1->GetValue() - ePopup_None + CFilterTarget::eNone);
	switch(target)
	{
		case CFilterTarget::eNone:
		case CFilterTarget::eAny:
			return new CFilterTarget(target);
		case CFilterTarget::eMailbox:
		{
			cdstring text = mText->GetText();
			return new CFilterTarget(target, text);
		}
		case CFilterTarget::eCabinet:
		{
			CMailAccountManager::EFavourite favtype = static_cast<CMailAccountManager::EFavourite>(mPopup2->GetValue() - 1);
			cdstring txt = CMailAccountManager::sMailAccountManager->GetFavouriteID(favtype);
			return new CFilterTarget(target, txt);
		}
		case CFilterTarget::eAccount:
		{
			cdstring acct_name;
			long acct_num = mPopup3->GetValue() - 1;
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
	long popup2 = 1;
	long popup3 = 1;
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
	OnSetTarget(popup1);
	mPopup2->SetValue(popup2);
	mPopup3->SetValue(popup3);
	mText->SetText(text);
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
	// Add each cabinet item
	short num_menu = mPopup2->GetItemCount();
	for(short i = CMailAccountManager::eFavouriteOthers; i < num_menu; i++)
		mPopup2->RemoveItem(CMailAccountManager::eFavouriteOthers);

	for(CFavouriteItemList::const_iterator iter = CPreferences::sPrefs->mFavourites.GetValue().begin() + CMailAccountManager::eFavouriteOthers;
		iter != CPreferences::sPrefs->mFavourites.GetValue().end(); iter++)
		mPopup2->AppendItem((*iter).GetName(), kFalse, kTrue);

	// Force to popup
	mPopup2->SetUpdateAction(JXMenu::kDisableNone);
	mPopup2->DisableItem(CMailAccountManager::eFavouriteNew + 1);
	mPopup2->SetToPopupChoice(kTrue, 1);
}

void CRulesTarget::InitAccountMenu()
{
	// Delete previous items
	mPopup3->RemoveAllItems();

	// Add each mail account
	for(CMailAccountList::const_iterator iter = CPreferences::sPrefs->mMailAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mMailAccounts.GetValue().end(); iter++)
		//Append it as a radio button
		mPopup3->AppendItem((*iter)->GetName(), kFalse, kTrue);

	mPopup3->SetToPopupChoice(kTrue, 1);
}

