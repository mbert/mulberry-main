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


// Source for CEditIdentityAddress class

#include "CEditIdentityAddress.h"

#include "CAddressList.h"
#include "CAdminLock.h"
#include "CCalendarAddress.h"
#include "cdstring.h"
#include "CPreferences.h"
#include "CStringUtils.h"
#include "CTextDisplay.h"
#include "CTextFieldX.h"

#include "cdstring.h"

#include <LCheckBoxGroupBox.h>
#include <LRadioButton.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CEditIdentityAddress::CEditIdentityAddress()
{
}

// Constructor from stream
CEditIdentityAddress::CEditIdentityAddress(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CEditIdentityAddress::~CEditIdentityAddress()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CEditIdentityAddress::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mActive = (LCheckBoxGroupBox*) FindPaneByID(paneid_EditIdentityActive);
	mSingle = (LRadioButton*) FindPaneByID(paneid_EditIdentitySingle);
	mMultiple = (LRadioButton*) FindPaneByID(paneid_EditIdentityMultiple);
	mPanel = (LView*) FindPaneByID(paneid_EditIdentityPanel1);
	mName = (CTextFieldX*) FindPaneByID(paneid_EditIdentityName);
	mEmail = (CTextFieldX*) FindPaneByID(paneid_EditIdentityEmail);
	mFocus = (LView*) FindPaneByID(paneid_EditIdentityScroller);
	mText = (CTextDisplay*) FindPaneByID(paneid_EditIdentityText);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, (GetPaneID() == paneid_EditIdentityAddress) ?
														RidL_CEditIdentityAddressBtns : RidL_CEditIdentityAddressOutgoingBtns);
}

// Handle buttons
void CEditIdentityAddress::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_EditIdentityUseSingle:
		if (*((long*) ioParam))
			SetSingle(true);
		break;

	case msg_EditIdentityUseFull:
		if (*((long*) ioParam))
			SetSingle(false);
		break;

	default:
		break;
	}
}

void CEditIdentityAddress::SetAddressType(bool from, bool reply_to, bool sender, bool to, bool cc, bool bcc, bool calendar)
{
	mFrom = from;
	mReplyTo = reply_to;
	mSender = sender;
	mTo = to;
	mCC = cc;
	mBcc = bcc;
	mCalendar = calendar;

	// Only allow single address for sender and calendar
	if (mSender || mCalendar)
		mMultiple->Disable();
}

// Toggle display of IC
void CEditIdentityAddress::ToggleICDisplay(bool IC_on)
{
	if (IC_on)
	{
		mSingle->SetValue(1);
		mName->Disable();
		mEmail->Disable();
		mText->Disable();
		mSingle->Disable();
		mMultiple->Disable();
	}
	else
	{
		mName->Enable();
		if (!CAdminLock::sAdminLock.mLockReturnAddress)
			mEmail->Enable();
		if (!CAdminLock::sAdminLock.mLockReturnAddress)
			mText->Enable();
		if (!CAdminLock::sAdminLock.mLockReturnAddress)
			mSingle->Enable();
		if (!CAdminLock::sAdminLock.mLockReturnAddress)
			mMultiple->Enable();
	}
}

// Set prefs
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
}

// Force update of prefs
void CEditIdentityAddress::UpdateData(void* data)
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
}

void CEditIdentityAddress::SetSingle(bool single)
{
	if (single)
	{
		// Copy handle to text with null terminator
		cdstring address;
		mText->GetText(address);

		if (!mCalendar)
		{
			CAddressList addr_list(address.c_str(), address.length());
			
			cdstring temp;
			if (addr_list.size())
				temp = addr_list.front()->GetName();
			mName->SetText(temp);
			if (addr_list.size())
				temp = addr_list.front()->GetMailAddress();
			mEmail->SetText(temp);
		}
		else
		{
			CCalendarAddress addr(address);
			mName->SetText(addr.GetName());
			mEmail->SetText(addr.GetAddress());
		}

		mFocus->Hide();
		mPanel->Show();
	}
	else
	{
		cdstring name = mName->GetText();
		cdstring email = mEmail->GetText();
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

		mPanel->Hide();
		mFocus->Show();
	}
}

void CEditIdentityAddress::SetItemData(bool active, const cdstring& address)
{
	mActive->SetValue(active);

	if (!mCalendar)
	{
		// Determine address list
		CAddressList addr_list(address.c_str(), address.length());
		
		// Truncate to single address for sender or locked from
		if ((addr_list.size() > 1) &&
			(mSender || (mFrom && CAdminLock::sAdminLock.mLockReturnAddress)))
			addr_list.erase(addr_list.begin() + 1, addr_list.end());
		
		if (addr_list.size() > 1)
		{
			// Hide single items
			mPanel->Hide();
			
			// Insert text
			cdstring addrs;
			for(CAddressList::const_iterator iter = addr_list.begin(); iter != addr_list.end(); iter++)
			{
				addrs += (*iter)->GetFullAddress();
				addrs += os_endl;
			}
			mText->SetText(addrs);
			
			StopListening();
			mMultiple->SetValue(1);
			StartListening();
		}
		else
		{
			// Hide multiple items
			mFocus->Hide();
			
			// Insert text
			if (addr_list.size() == 1)
			{
				mName->SetText(addr_list.front()->GetName());
				mEmail->SetText(addr_list.front()->GetMailAddress());
			}
			mSingle->SetValue(1);
		}
	}
	else
	{
		CCalendarAddressList addrs;
		CCalendarAddress::FromIdentityText(address, addrs);
		if (addrs.size() == 1)
		{
			// Hide multiple items
			mFocus->Hide();
			
			// Insert text
			mName->SetText(addrs.front()->GetName());
			mEmail->SetText(addrs.front()->GetCalendarAddress());
			mSingle->SetValue(1);
		}
		else
		{
			// Hide single items
			mPanel->Hide();
			
			// Insert text
			cdstring txt;
			for(CCalendarAddressList::const_iterator iter = addrs.begin(); iter != addrs.end(); iter++)
			{
				txt += (*iter)->GetFullAddress();
				txt += os_endl;
			}
			mText->SetText(txt);
			
			StopListening();
			mMultiple->SetValue(1);
			StartListening();
		}

	}

	// Lock fields
	if (mFrom && CAdminLock::sAdminLock.mLockReturnAddress)
	{
		mSingle->Disable();
		mMultiple->Disable();
		mEmail->Disable();
		mText->Disable();
	}
}

void CEditIdentityAddress::GetItemData(bool& active, cdstring& address)
{
	active = (mActive->GetValue() == 1);

	bool single = (mSingle->GetValue() == 1);

	if (single)
	{
		cdstring name = mName->GetText();
		cdstring email = mEmail->GetText();
		if (!mCalendar)
		{
			CAddress addr(email, name);
			address = addr.GetFullAddress();
		}
		else
		{
			CCalendarAddress addr(email, name);
			address = addr.GetFullAddress();
		}
	}
	else
	{
		// Copy handle to text with null terminator
		mText->GetText(address);
	}
}
