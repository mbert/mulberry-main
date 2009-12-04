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


// CPropCalendarWebcal.cp : implementation file
//

#include "CPropCalendarWebcal.h"

#include "CCalendarProtocol.h"
#include "CCalendarStoreManager.h"
#include "CCalendarStoreNode.h"
#include "CCalendarStoreWebcal.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CUnicodeUtils.h"
#include "CXStringResources.h"

/////////////////////////////////////////////////////////////////////////////
// CPropCalendarWebcal property page

IMPLEMENT_DYNCREATE(CPropCalendarWebcal, CHelpPropertyPage)

CPropCalendarWebcal::CPropCalendarWebcal() : CHelpPropertyPage(CPropCalendarWebcal::IDD)
{
	//{{AFX_DATA_INIT(CPropCalendarWebcal)
	mIconState = 0;
	mAllowChanges = 0;
	mAutoPublish = 0;
	mRefreshGroup = 0;
	//}}AFX_DATA_INIT
	mMultiple = false;
	
	mCalList = NULL;
}

CPropCalendarWebcal::~CPropCalendarWebcal()
{
}

void CPropCalendarWebcal::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropCalendarWebcal)
	DDX_UTF8Text(pDX, IDC_CALENDARWEBCAL_URL, mRemoteURL);
	DDX_Check(pDX, IDC_CALENDARWEBCAL_ALLOWCHANGES, mAllowChanges);
	DDX_Check(pDX, IDC_CALENDARWEBCAL_AUTOPUBLISH, mAllowChanges);
	DDX_Radio(pDX, IDC_CALENDARWEBCAL_REFRESHNEVER, mRefreshGroup);
	DDX_UTF8Text(pDX, IDC_CALENDARWEBCAL_REFRESHINTERVAL, mRefreshInterval);
	DDV_MinMaxInt(pDX, mRefreshInterval, 0, 32767);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropCalendarWebcal, CHelpPropertyPage)
	//{{AFX_MSG_MAP(CPropCalendarWebcal)
	ON_BN_CLICKED(IDC_CALENDARWEBCAL_REVERTURL, OnRevertURL)
	ON_BN_CLICKED(IDC_CALENDARWEBCAL_ALLOWCHANGES, OnAllowChanges)
	ON_BN_CLICKED(IDC_CALENDARWEBCAL_AUTOPUBLISH, OnAutoPublish)
	ON_BN_CLICKED(IDC_CALENDARWEBCAL_REFRESHNEVER, OnRefreshNever)
	ON_BN_CLICKED(IDC_CALENDARWEBCAL_REFRESHEVERY, OnRefreshEvery)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropCalendarWebcal message handlers

BOOL CPropCalendarWebcal::OnInitDialog()
{
	CHelpPropertyPage::OnInitDialog();

	// Set state icon
	mIconStateCtrl.SubclassDlgItem(IDC_CALENDARWEBCAL_STATE, this, mIconState);

	if (mMultiple)
	{
		GetDlgItem(IDC_CALENDARWEBCAL_URL)->EnableWindow(false);
		GetDlgItem(IDC_CALENDARWEBCAL_REVERTURL)->EnableWindow(false);
	}
	
	if (!mAllowChanges)
		GetDlgItem(IDC_CALENDARWEBCAL_AUTOPUBLISH)->EnableWindow(false);
		
	if (mRefreshGroup == 0)
		GetDlgItem(IDC_CALENDARWEBCAL_REFRESHINTERVAL)->EnableWindow(false);
		
	return true;
}

// Set mbox list
void CPropCalendarWebcal::SetCalList(calstore::CCalendarStoreNodeList* cal_list)
{
	// Save list
	mCalList = cal_list;

	// Do icon state
	mIconState = IDI_MAILBOX_DISCONNECTED;

	int webcal = 0;
	int allow_changes = 0;
	int auto_publish = 0;
	int periodic_refresh = 0;
	uint32_t common_refresh_interval = 0;
	bool refresh_intervals_equal = true;
	for(calstore::CCalendarStoreNodeList::const_iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
	{
		webcal++;
		if ((*iter)->GetWebcal() != NULL)
		{
			if (!(*iter)->GetWebcal()->GetReadOnly())
				allow_changes++;
			if ((*iter)->GetWebcal()->GetAutoPublish())
				auto_publish++;
			if ((*iter)->GetWebcal()->GetPeriodicRefresh())
				periodic_refresh++;
			
			// Check refersh intervals
			if (iter == mCalList->begin())
				common_refresh_interval = (*iter)->GetWebcal()->GetRefreshInterval();
			else if ((*iter)->GetWebcal()->GetPeriodicRefresh() != common_refresh_interval)
				refresh_intervals_equal = false;
		}
	}

	// Now adjust totals
	if (allow_changes != 0)
		allow_changes = ((allow_changes == mCalList->size()) ? 1 : 2);
	if (auto_publish != 0)
		auto_publish = ((auto_publish == mCalList->size()) ? 1 : 2);
	if (periodic_refresh != 0)
		periodic_refresh = ((periodic_refresh == mCalList->size()) ? 1 : 2);

	// Disable URL if more than one
	if (webcal > 1)
	{
		mRemoteURL = rsrc::GetString("UI::CalendarProp::MultipleWebcals");
		mMultiple = true;
	}
	else
	{
		mRemoteURL = mCalList->front()->GetRemoteURL();
		
		// Only do this once
		if (mRevertURL.empty())
			mRevertURL = mRemoteURL;
	}

	mAllowChanges = allow_changes;
	mAutoPublish = auto_publish;
	
	// Set refresh on if all are on, else off
	mRefreshGroup = (periodic_refresh != 0) ? 0 : 1;

	// Leave refresh interval empty if more than one
	if (refresh_intervals_equal)
		mRefreshInterval = common_refresh_interval;
	else
		mRefreshInterval = 0;
}

void CPropCalendarWebcal::OnRevertURL()
{
	CUnicodeUtils::SetWindowTextUTF8(GetDlgItem(IDC_CALENDARWEBCAL_URL), mRevertURL);
}

void CPropCalendarWebcal::OnAllowChanges()
{
	UINT result = ::CheckboxToggle(static_cast<CButton*>(GetDlgItem(IDC_CALENDARWEBCAL_ALLOWCHANGES)));

	// Enable/disable publish
	GetDlgItem(IDC_CALENDARWEBCAL_AUTOPUBLISH)->EnableWindow(result == 1);
	
	// Iterate over all Calendars and change webcal read-only state
	for(calstore::CCalendarStoreNodeList::iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
	{
		if ((*iter)->GetWebcal() != NULL)
		{
			(*iter)->GetWebcal()->SetReadOnly(result == 0);
		}
	}
	
	// Flush changes
	calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol()->ListChanged();
}

void CPropCalendarWebcal::OnAutoPublish()
{
	UINT result = ::CheckboxToggle(static_cast<CButton*>(GetDlgItem(IDC_CALENDARWEBCAL_AUTOPUBLISH)));

	// Iterate over all Calendars and change webcal read-only state
	for(calstore::CCalendarStoreNodeList::iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
	{
		if ((*iter)->GetWebcal() != NULL)
		{
			(*iter)->GetWebcal()->SetAutoPublish(result == 1);
		}
	}
	
	// Flush changes
	calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol()->ListChanged();
}

void CPropCalendarWebcal::OnRefreshNever()
{
	// Iterate over all Calendars and change webcal read-only state
	for(calstore::CCalendarStoreNodeList::iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
	{
		if ((*iter)->GetWebcal() != NULL)
		{
			(*iter)->GetWebcal()->SetPeriodicRefresh(false);
		}
	}
	
	// Flush changes
	calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol()->ListChanged();
	
	GetDlgItem(IDC_CALENDARWEBCAL_REFRESHINTERVAL)->EnableWindow(false);
}

void CPropCalendarWebcal::OnRefreshEvery()
{
	// Iterate over all Calendars and change webcal read-only state
	for(calstore::CCalendarStoreNodeList::iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
	{
		if ((*iter)->GetWebcal() != NULL)
		{
			(*iter)->GetWebcal()->SetPeriodicRefresh(true);
		}
	}
	
	// Flush changes
	calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol()->ListChanged();
	
	GetDlgItem(IDC_CALENDARWEBCAL_REFRESHINTERVAL)->EnableWindow(true);
}
