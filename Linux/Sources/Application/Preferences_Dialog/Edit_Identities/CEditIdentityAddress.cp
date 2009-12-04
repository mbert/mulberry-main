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


// CEditIdentityAddress.cpp : implementation file
//

#include "CEditIdentityAddress.h"

#include "CAddressList.h"
#include "CAdminLock.h"
#include "CCalendarAddress.h"
#include "CPreferences.h"
#include "CTextDisplay.h"
#include "CTextField.h"
#include "cdstring.h"

#include <JXDownRect.h>
#include <JXRadioGroup.h>
#include <JXScrollbarSet.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CEditIdentityAddress dialog

/////////////////////////////////////////////////////////////////////////////
// CEditIdentityAddress message handlers

// Get details of sub-panes
void CEditIdentityAddress::OnCreate()
{
	if (!mOutgoing)
	{
// begin JXLayout1

    mActiveGroup =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 430,175);
    assert( mActiveGroup != NULL );

    mActive =
        new JXTextCheckbox("Active", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,5, 65,20);
    assert( mActive != NULL );

    mSingleGroup =
        new JXRadioGroup(mActiveGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,30, 160,40);
    assert( mSingleGroup != NULL );
    mSingleGroup->SetBorderWidth(0);

    mSingleBtn =
        new JXTextRadioButton(1, "Single Address", mSingleGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 115,20);
    assert( mSingleBtn != NULL );

    mMultipleBtn =
        new JXTextRadioButton(2, "Multiple Addresses", mSingleGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,20, 140,20);
    assert( mMultipleBtn != NULL );

    mNameTitle =
        new JXStaticText("Name:", mActiveGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,82, 50,20);
    assert( mNameTitle != NULL );

    mName =
        new CTextInputField(mActiveGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,80, 300,20);
    assert( mName != NULL );

    mEmailTitle =
        new JXStaticText("Email:", mActiveGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,107, 50,20);
    assert( mEmailTitle != NULL );

    mEmail =
        new CTextInputField(mActiveGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,105, 300,20);
    assert( mEmail != NULL );

    mText =
        new CTextInputDisplay(mActiveGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,80, 295,65);
    assert( mText != NULL );

// end JXLayout1
	}
	else
	{
// begin JXLayout2

    mActiveGroup =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 410,130);
    assert( mActiveGroup != NULL );

    mActive =
        new JXTextCheckbox("Active", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,5, 65,20);
    assert( mActive != NULL );

    mSingleGroup =
        new JXRadioGroup(mActiveGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,10, 160,40);
    assert( mSingleGroup != NULL );
    mSingleGroup->SetBorderWidth(0);

    mSingleBtn =
        new JXTextRadioButton(1, "Single Address", mSingleGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 115,20);
    assert( mSingleBtn != NULL );

    mMultipleBtn =
        new JXTextRadioButton(2, "Multiple Addresses", mSingleGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,20, 140,20);
    assert( mMultipleBtn != NULL );

    mNameTitle =
        new JXStaticText("Name:", mActiveGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,57, 50,20);
    assert( mNameTitle != NULL );

    mName =
        new CTextInputField(mActiveGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,55, 300,20);
    assert( mName != NULL );

    mEmailTitle =
        new JXStaticText("Email:", mActiveGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,82, 50,20);
    assert( mEmailTitle != NULL );

    mEmail =
        new CTextInputField(mActiveGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,80, 300,20);
    assert( mEmail != NULL );

    mText =
        new CTextInputDisplay(mActiveGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,55, 295,65);
    assert( mText != NULL );

// end JXLayout2
	}

	// Hide items
	mTextScroller = mText->GetScroller();
	mTextScroller->Hide();
	
	// Only allow single address for sender
	if (mSender)
		mMultipleBtn->Deactivate();

	// Lock fields
	if (mFrom && CAdminLock::sAdminLock.mLockReturnAddress)
	{
		mMultipleBtn->Deactivate();
		mEmail->Deactivate();
		mTextScroller->Deactivate();
	}

	// Start listening
	ListenTo(mActive);
	ListenTo(mSingleGroup);
}

void CEditIdentityAddress::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		if (sender == mSingleGroup)
		{
			SetSingle(index == 1);
			return;
		}
	}
	else if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mActive)
		{
			OnActive(mActive->IsChecked());
			return;
		}
	}
	
	CTabPanel::Receive(sender, message);
}

void CEditIdentityAddress::OnActive(bool active)
{
	if (active)
		mActiveGroup->Activate();
	else
		mActiveGroup->Deactivate();
}

// Set data
void CEditIdentityAddress::SetData(void* data)
{
	CIdentity* id = (CIdentity*) data;

	// Copy info
	if (mFrom)
		SetItemData(id->UseFrom(), id->GetFrom());
	else if (mReplyTo)
		SetItemData(id->UseReplyTo(), id->GetReplyTo());
	else if (mSender)
		SetItemData(id->UseSender(), id->GetSender());
	else if (mTo)
		SetItemData(id->UseAddTo(), id->GetAddTo());
	else if (mCC)
		SetItemData(id->UseAddCC(), id->GetAddCC());
	else if (mBcc)
		SetItemData(id->UseAddBcc(), id->GetAddBcc());
	else if (mCalendar)
		SetItemData(id->UseCalendar(), id->GetCalendar());
	
	// Must update the display if changed to multiple
	if (!mSingle)
	{
		mNameTitle->Hide();
		mEmailTitle->Hide();
		mName->Hide();
		mEmail->Hide();
		mTextScroller->Show();

		mSingleGroup->SelectItem(2);
	}
}

// Force update of data
bool CEditIdentityAddress::UpdateData(void* data)
{
	CIdentity* id = (CIdentity*) data;

	bool active;
	cdstring address;
	GetItemData(active, address);

	if (mFrom)
		id->SetFrom(address, active);
	else if (mReplyTo)
		id->SetReplyTo(address, active);
	else if (mSender)
		id->SetSender(address, active);
	else if (mTo)
		id->SetAddTo(address, active);
	else if (mCC)
		id->SetAddCC(address, active);
	else if (mBcc)
		id->SetAddBcc(address, active);
	else if (mCalendar)
		id->SetCalendar(address, active);
	
	return true;
}

void CEditIdentityAddress::SetSingle(bool single)
{
	if (single && !mSingle)
	{
		cdstring text(mText->GetText());
		
		if (!mCalendar)
		{
			CAddressList addr_list(text, text.length());

			if (addr_list.size())
				mName->SetText(addr_list.front()->GetName());
			else
				mName->SetText(cdstring::null_str);
			if (addr_list.size())
				mEmail->SetText(addr_list.front()->GetMailAddress());
			else
				mEmail->SetText(cdstring::null_str);
		}
		else
		{
			CCalendarAddress addr(text);

			mName->SetText(addr.GetName());
			mEmail->SetText(addr.GetAddress());
		}

		mTextScroller->Hide();
		mNameTitle->Show();
		mEmailTitle->Show();
		mName->Show();
		mEmail->Show();
		
		mSingle = true;
	}
	else if (!single && mSingle)
	{
		cdstring name(mName->GetText());
		cdstring email(mEmail->GetText());
		
		if (!mCalendar)
		{
			CAddress addr(email, name);
			cdstring address = addr.GetFullAddress();

			mText->SetText(address);
		}
		else
		{
			CCalendarAddress addr(email, name);
			cdstring address = addr.GetFullAddress();

			mText->SetText(address);
		}
		
		mNameTitle->Hide();
		mEmailTitle->Hide();
		mName->Hide();
		mEmail->Hide();
		mTextScroller->Show();
		
		mSingle = false;
	}
}

void CEditIdentityAddress::SetItemData(bool active, const cdstring& address)
{
	mActive->SetState(JBoolean(active));
	OnActive(active);

	if (!mCalendar)
	{
		// Determine address list
		CAddressList addr_list(address.c_str(), address.length());

		// Truncate to single address for sender
		if (mSender && (addr_list.size() > 1) ||
			mFrom && CAdminLock::sAdminLock.mLockReturnAddress)
			addr_list.erase(addr_list.begin() + 1, addr_list.end());

		if (addr_list.size() > 1)
		{
			// Select single items
			mSingle = false;

			// Set multi text
			cdstring text;
			for(CAddressList::const_iterator iter = addr_list.begin(); iter != addr_list.end(); iter++)
				text += (*iter)->GetFullAddress() + "\n";
			mText->SetText(text);
		}
		else
		{
			// Select single items
			mSingle = true;

			// Insert text
			if (addr_list.size() == 1)
			{
				mName->SetText(addr_list.front()->GetName());
				mEmail->SetText(addr_list.front()->GetMailAddress());
			}
		}
	}
	else
	{
		// Determine address list
		CCalendarAddressList addrs;
		CCalendarAddress::FromIdentityText(address, addrs);

		if (addrs.size() > 1)
		{
			// Select single items
			mSingle = false;

			// Set multi text
			cdstring text;
			for(CCalendarAddressList::const_iterator iter = addrs.begin(); iter != addrs.end(); iter++)
				text += (*iter)->GetFullAddress() + "\n";
			mText->SetText(text);
		}
		else
		{
			// Select single items
			mSingle = true;

			// Insert text
			mName->SetText(addrs.front()->GetName());
			mEmail->SetText(addrs.front()->GetCalendarAddress());
		}
	}
}

void CEditIdentityAddress::GetItemData(bool& active, cdstring& address)
{
	active = mActive->IsChecked();
	
	bool single = (mSingleGroup->GetSelectedItem() == 1);
	
	if (single)
	{
		if (!mCalendar)
		{
			CAddress addr(cdstring(mEmail->GetText()), cdstring(mName->GetText()));
			address = addr.GetFullAddress();
		}
		else
		{
			CCalendarAddress addr(cdstring(mEmail->GetText()), cdstring(mName->GetText()));
			address = addr.GetFullAddress();
		}
	}
	else
		// Copy handle to text with null terminator
		address = cdstring(mText->GetText());
}
