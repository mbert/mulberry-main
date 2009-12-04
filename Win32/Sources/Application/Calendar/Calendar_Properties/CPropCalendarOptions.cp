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


// CPropCalendarOptions.cp : implementation file
//

#include "CPropCalendarOptions.h"

#include "CCalendarProtocol.h"
#include "CCalendarStoreManager.h"
#include "CCalendarStoreNode.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"

/////////////////////////////////////////////////////////////////////////////
// CPropCalendarOptions property page

IMPLEMENT_DYNCREATE(CPropCalendarOptions, CHelpPropertyPage)

CPropCalendarOptions::CPropCalendarOptions() : CHelpPropertyPage(CPropCalendarOptions::IDD)
{
	//{{AFX_DATA_INIT(CPropCalendarOptions)
	mIconState = 0;
	mSubscribe = 0;
	mTieIdentity = 0;
	//}}AFX_DATA_INIT
	
	mCalList = NULL;
	mAllDir = false;
}

CPropCalendarOptions::~CPropCalendarOptions()
{
}

void CPropCalendarOptions::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropCalendarOptions)
	DDX_Check(pDX, IDC_CALENDAROPTIONS_SUBSCRIBE, mSubscribe);
	DDX_Control(pDX, IDC_CALENDAROPTIONS_SUBSCRIBE, mSubscribeCtrl);
	DDX_Check(pDX, IDC_CALENDAROPTIONS_TIEIDENTITY, mTieIdentity);
	DDX_Control(pDX, IDC_CALENDAROPTIONS_TIEIDENTITY, mTieIdentityCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropCalendarOptions, CHelpPropertyPage)
	//{{AFX_MSG_MAP(CPropCalendarOptions)
	ON_BN_CLICKED(IDC_CALENDAROPTIONS_SUBSCRIBE, OnSubscribe)
	ON_BN_CLICKED(IDC_CALENDAROPTIONS_TIEIDENTITY, OnTieIdentity)
	ON_COMMAND_RANGE(IDM_IDENTITY_NEW, IDM_IDENTITYEnd, OnIdentityPopup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropCalendarOptions message handlers

BOOL CPropCalendarOptions::OnInitDialog()
{
	CHelpPropertyPage::OnInitDialog();

	// Set state icon
	mIconStateCtrl.SubclassDlgItem(IDC_CALENDAROPTIONS_STATE, this, mIconState);

	mIdentityPopup.SubclassDlgItem(IDC_CALENDAROPTIONS_IDENTITYPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mIdentityPopup.SetMenu(IDR_POPUP_IDENTITY);
	mIdentityPopup.Reset(CPreferences::sPrefs->mIdentities.GetValue());

	//  the identity popup
	// Set identity items
	if (mTieIdentity == 2)
		mIdentityPopup.SetValue(IDM_IDENTITYStart);
	else if (!mCurrentIdentity.empty())
	{
		// Identity to use
		const CIdentity* id = CPreferences::sPrefs->mIdentities.GetValue().GetIdentity(mCurrentIdentity);
		mIdentityPopup.SetIdentity(CPreferences::sPrefs, id);
	}
	else
		mIdentityPopup.SetValue(IDM_IDENTITYStart);
	mIdentityPopup.EnableWindow(mTieIdentity == 1);

	if (mAllDir)
	{
		GetDlgItem(IDC_CALENDAROPTIONS_SUBSCRIBE)->EnableWindow(false);
	}

	return true;
}

// Set mbox list
void CPropCalendarOptions::SetCalList(calstore::CCalendarStoreNodeList* cal_list)
{
	// Save list
	mCalList = cal_list;

	// Do icon state
	if (cal_list->front()->GetProtocol()->IsLocalCalendar())
	{
		mIconState = IDI_MAILBOX_LOCAL;
	}
	else if (cal_list->front()->GetProtocol()->IsWebCalendar())
	{
		mIconState = IDI_MAILBOX_DISCONNECTED;
	}
	else if (cal_list->front()->GetProtocol()->CanDisconnect())
	{
		mIconState = cal_list->front()->GetProtocol()->IsDisconnected() ? IDI_MAILBOX_DISCONNECTED : IDI_MAILBOX_REMOTE;
	}
	else
	{
		mIconState = IDI_MAILBOX_REMOTE;
	}

	int first_type = -1;
	int subscribe = 0;
	int multiple_tied = 0;
	bool first = true;
	mAllDir = true;
	for(calstore::CCalendarStoreNodeList::const_iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
	{
		calstore::CCalendarStoreNode* node = *iter;
		mAllDir &= node->IsDirectory();

		// Only valid id not a directory
		if (!node->IsDirectory() && node->IsSubscribed())
			subscribe++;

		CIdentity* id = const_cast<CIdentity*>(CPreferences::sPrefs->mTiedCalendars.GetValue().GetTiedCalIdentity(*iter));
		if (first)
		{
			mCurrentIdentity = (id ? id->GetIdentity() : cdstring::null_str);
			first = false;
		}
		else if (!multiple_tied)
		{
			if (mCurrentIdentity != (id ? id->GetIdentity() : cdstring::null_str))
			{
				// Mark as multiple
				multiple_tied = true;

				// Remember the first 'real' identity found
				if (mCurrentIdentity.empty())
					mCurrentIdentity = (id ? id->GetIdentity() : cdstring::null_str);
			}
		}
	}
	
	// Now adjust totals
	if (subscribe)
		subscribe = ((subscribe == mCalList->size()) ? 1 : 2);

	if (!mAllDir)
	{
		mSubscribe = subscribe;
	}

	// Set identity items
	if (multiple_tied)
	{
		mTieIdentity = 2;
		mCurrentIdentity = CPreferences::sPrefs->mIdentities.GetValue().front().GetIdentity();
	}
	else if (!mCurrentIdentity.empty())
	{
		mTieIdentity = 1;
	}
	else
	{
		mTieIdentity = 0;
		mCurrentIdentity = CPreferences::sPrefs->mIdentities.GetValue().front().GetIdentity();
	}
}


// Handle check new
void CPropCalendarOptions::OnSubscribe()
{
	UINT result = ::CheckboxToggle(&mSubscribeCtrl);

	// Iterate over all Calendars
	for(calstore::CCalendarStoreNodeList::iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
	{
		calstore::CCalendarStoreManager::sCalendarStoreManager->SubscribeNode(*iter, !(*iter)->IsSubscribed());
	}
}

// Handle tie change
void CPropCalendarOptions::OnTieIdentity(void)
{
	::CheckboxToggle(&mTieIdentityCtrl);
	mIdentityPopup.EnableWindow(mTieIdentityCtrl.GetCheck());
	SetTiedIdentity(mTieIdentityCtrl.GetCheck());
}

void CPropCalendarOptions::OnIdentityPopup(UINT nID)
{
	switch(nID)
	{
	// New identity wanted
	case IDM_IDENTITY_NEW:
		mIdentityPopup.DoNewIdentity(CPreferences::sPrefs);
		break;
	
	// New identity wanted
	case IDM_IDENTITY_EDIT:
		mIdentityPopup.DoEditIdentity(CPreferences::sPrefs);
		break;
	
	// Delete existing identity
	case IDM_IDENTITY_DELETE:
		mIdentityPopup.DoDeleteIdentity(CPreferences::sPrefs);
		break;
	
	// Select an identity
	default:
		SetTiedIdentity(false);
		mIdentityPopup.SetValue(nID);
		mCurrentIdentity = CPreferences::sPrefs->mIdentities.Value()[nID - IDM_IDENTITYStart].GetIdentity();
		SetTiedIdentity(true);
		break;
	}
}

// Add/remove tied identity
void CPropCalendarOptions::SetTiedIdentity(bool add)
{
	for(calstore::CCalendarStoreNodeList::const_iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
	{
		calstore::CCalendarStoreNode* node = *iter;
		if (add)
		{
			// Identity to use
			CIdentity* id = CPreferences::sPrefs->mIdentities.GetValue().GetIdentity(mCurrentIdentity);

			id->AddTiedCalendar(node);
		}
		else
		{
			// With multiple Calendars might have multiple ids so resolve each one
			CIdentity* id = const_cast<CIdentity*>(CPreferences::sPrefs->mTiedCalendars.GetValue().GetTiedCalIdentity(node));
			if (id)
				id->RemoveTiedCalendar(node);
		}
	}

	// Mark it as dirty
	CPreferences::sPrefs->mTiedCalendars.SetDirty();
}
