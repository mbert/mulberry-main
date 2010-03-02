/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CPropCalendarGeneral.cp : implementation file
//

#include "CPropCalendarGeneral.h"

#include "CCalendarProtocol.h"
#include "CCalendarStoreNode.h"
#include "CConnectionManager.h"
#include "CMulberryCommon.h"
#include "CRFC822.h"
#include "CUnicodeUtils.h"
#include "CXstringResources.h"

/////////////////////////////////////////////////////////////////////////////
// CPropCalendarGeneral property page

IMPLEMENT_DYNCREATE(CPropCalendarGeneral, CHelpPropertyPage)

CPropCalendarGeneral::CPropCalendarGeneral() : CHelpPropertyPage(CPropCalendarGeneral::IDD)
{
	//{{AFX_DATA_INIT(CPropCalendarGeneral)
	mIconState = 0;
	//}}AFX_DATA_INIT
	
	mCalList = NULL;
	mUseSynchronise = true;
	mIsDir = false;
}

CPropCalendarGeneral::~CPropCalendarGeneral()
{
}

void CPropCalendarGeneral::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropCalendarGeneral)
	DDX_UTF8Text(pDX, IDC_CALENDARGENERAL_NAME, mName);
	DDX_UTF8Text(pDX, IDC_CALENDARGENERAL_SERVER, mServer);
	DDX_UTF8Text(pDX, IDC_CALENDARGENERAL_HIERARCHY, mHierarchy);
	DDX_UTF8Text(pDX, IDC_CALENDARGENERAL_SEPARATOR, mSeparator);
	DDV_UTF8MaxChars(pDX, mSeparator, 1);
	DDX_UTF8Text(pDX, IDC_CALENDARGENERAL_SIZE, mSize);
	DDX_UTF8Text(pDX, IDC_CALENDARGENERAL_STATUS, mStatus);
	DDX_UTF8Text(pDX, IDC_CALENDARGENERAL_SYNCHRONISED, mSynchronise);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropCalendarGeneral, CHelpPropertyPage)
	//{{AFX_MSG_MAP(CPropCalendarGeneral)
	ON_BN_CLICKED(IDC_CALENDARGENERAL_CALCULATE, OnCalculateSize)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropCalendarGeneral message handlers

BOOL CPropCalendarGeneral::OnInitDialog()
{
	CHelpPropertyPage::OnInitDialog();

	// Set state icon
	mIconStateCtrl.SubclassDlgItem(IDC_CALENDARGENERAL_STATE, this, mIconState);
	if (!mUseSynchronise)
		GetDlgItem(IDC_CALENDARGENERAL_SYNCHRONISED)->EnableWindow(false);

	// Disable size calculate if a directory
	if (mIsDir)
		GetDlgItem(IDC_CALENDARGENERAL_CALCULATE)->EnableWindow(false);
		
	return true;
}

// Set mbox list
void CPropCalendarGeneral::SetCalList(calstore::CCalendarStoreNodeList* cal_list)
{
	// Save list
	mCalList = cal_list;

	// For now just use first item
	SetCalendar(mCalList->front());
}


// Set mbox list
void CPropCalendarGeneral::SetCalendar(calstore::CCalendarStoreNode* node)
{
	// Do icon state
	if (node->GetProtocol()->IsLocalCalendar())
	{
		mIconState = IDI_MAILBOX_LOCAL;
		mUseSynchronise = false;
	}
	else if (node->GetProtocol()->IsWebCalendar())
	{
		mIconState = IDI_MAILBOX_DISCONNECTED;
	}
	else if (node->GetProtocol()->CanDisconnect())
	{
		mIconState = node->GetProtocol()->IsDisconnected() ? IDI_MAILBOX_DISCONNECTED : IDI_MAILBOX_REMOTE;
	}
	else
	{
		mIconState = IDI_MAILBOX_REMOTE;
		mUseSynchronise = false;
	}

	// Copy text to edit fields
	mName = node->GetDisplayName();

	mServer = node->GetProtocol()->GetDescriptor();

	mHierarchy = node->GetParent()->GetName();

	cdstring temp(node->GetProtocol()->GetDirDelim());
	mSeparator = temp;

	// Only set these if no error
	if (!node->IsDirectory())
	{
		{
			unsigned long size = node->GetSize();
			if ((size == ULONG_MAX) && (node->GetProtocol()->IsOffline() || node->GetProtocol()->IsWebCalendar()))
			{
				node->CheckSize();
				size = node->GetSize();
			}
			if (size != ULONG_MAX)
			{
				mSize = ::GetNumericFormat(size);
			}
			else
				mSize = cdstring::null_str;
		}

		{
			// Depends on account type
			if (node->GetProtocol()->IsLocalCalendar())
			{
				mSynchronise = rsrc::GetString("UI::CalendarProp::LocalSync");
			}
			else if (node->GetProtocol()->IsWebCalendar() || node->GetProtocol()->CanDisconnect())
			{
				unsigned long utc_time = node->GetLastSync();
				
				if (utc_time != 0)
				{
					// Determine timezone offset
					time_t t1 = ::time(NULL);
					time_t t2 = ::mktime(::gmtime(&t1));
					long offset = ::difftime(t1, t2);
					long current_zone = ::difftime(t1, t2) / 3600.0;	// Zone in hundreds e.g. -0500
					current_zone *= 100;
					utc_time += offset;

					temp = CRFC822::GetTextDate(utc_time, current_zone, true);
				}
				else
					temp = rsrc::GetString("UI::CalendarProp::NoSync");
				mSynchronise = temp;
			}
			else
			{
				mSynchronise = rsrc::GetString("UI::CalendarProp::CannotSync");
			}
		}
	}
	else
	{
		mSize = cdstring::null_str;
		mSynchronise = cdstring::null_str;
	}
	
	// Disable size calculate if a directory
	mIsDir = node->IsDirectory();

	temp = cdstring::null_str;
	if (node->IsActive())
	{
		cdstring rsrc;
		rsrc.FromResource("UI::CalendarProp::StateOpen");
		temp += rsrc;
	}
	if (node->IsSubscribed())
	{
		cdstring rsrc;
		rsrc.FromResource("UI::CalendarProp::StateSubs");
		if (!temp.empty())
			temp += ", ";
		temp += rsrc;
	}
	mStatus = temp;
}


// Check sizes of all mailboxes
void CPropCalendarGeneral::OnCalculateSize()
{
	// Iterate over all mailboxes
	for(calstore::CCalendarStoreNodeList::iterator iter = mCalList->begin(); iter != mCalList->end(); iter++)
	{
		if (!(*iter)->IsDirectory())
			(*iter)->CheckSize();
	}

	{
		uint32_t size = mCalList->front()->GetSize();
		if (size != ULONG_MAX)
		{
			CUnicodeUtils::SetWindowTextUTF8(GetDlgItem(IDC_CALENDARGENERAL_SIZE), ::GetNumericFormat(size));
		}
		else
			CUnicodeUtils::SetWindowTextUTF8(GetDlgItem(IDC_CALENDARGENERAL_SIZE), cdstring::null_str);
	}
}
