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


// CPropMailboxGeneral.cp : implementation file
//

#include "CPropMailboxGeneral.h"

#include "CConnectionManager.h"
#include "CMbox.h"
#include "CMboxList.h"
#include "CMboxProtocol.h"
#include "CMulberryCommon.h"
#include "CRFC822.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPropMailboxGeneral property page

IMPLEMENT_DYNCREATE(CPropMailboxGeneral, CHelpPropertyPage)

CPropMailboxGeneral::CPropMailboxGeneral() : CHelpPropertyPage(CPropMailboxGeneral::IDD)
{
	//{{AFX_DATA_INIT(CPropMailboxGeneral)
	mIconState = 0;
	//}}AFX_DATA_INIT
	
	mMboxList = NULL;
	mUseSynchronise = false;
	mIsDir = false;
}

CPropMailboxGeneral::~CPropMailboxGeneral()
{
}

void CPropMailboxGeneral::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropMailboxGeneral)
	DDX_UTF8Text(pDX, IDC_MAILBOXGENERAL_NAME, mName);
	DDX_UTF8Text(pDX, IDC_MAILBOXGENERAL_SERVER, mServer);
	DDX_UTF8Text(pDX, IDC_MAILBOXGENERAL_HIERARCHY, mHierarchy);
	DDX_UTF8Text(pDX, IDC_MAILBOXGENERAL_SEPARATOR, mSeparator);
	DDV_UTF8MaxChars(pDX, mSeparator, 1);
	DDX_UTF8Text(pDX, IDC_MAILBOXGENERAL_TOTAL, mTotal);
	DDX_UTF8Text(pDX, IDC_MAILBOXGENERAL_RECENT, mRecent);
	DDX_UTF8Text(pDX, IDC_MAILBOXGENERAL_UNSEEN, mUnseen);
	DDX_UTF8Text(pDX, IDC_MAILBOXGENERAL_DELETED, mDeleted);
	DDX_UTF8Text(pDX, IDC_MAILBOXGENERAL_FULLSYNC, mFullSync);
	DDX_UTF8Text(pDX, IDC_MAILBOXGENERAL_PARTIALSYNC, mPartialSync);
	DDX_UTF8Text(pDX, IDC_MAILBOXGENERAL_NOTSYNC, mMissingSync);
	DDX_UTF8Text(pDX, IDC_MAILBOXGENERAL_SIZE, mSize);
	DDX_UTF8Text(pDX, IDC_MAILBOXGENERAL_STATUS, mStatus);
	DDX_UTF8Text(pDX, IDC_MAILBOXGENERAL_UIDVALIDITY, mUIDValidity);
	DDX_UTF8Text(pDX, IDC_MAILBOXGENERAL_SYNCHRONISED, mSynchronise);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropMailboxGeneral, CHelpPropertyPage)
	//{{AFX_MSG_MAP(CPropMailboxGeneral)
	ON_BN_CLICKED(IDC_MAILBOXGENERAL_CALCULATE, OnCalculateSize)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropMailboxGeneral message handlers

BOOL CPropMailboxGeneral::OnInitDialog()
{
	CHelpPropertyPage::OnInitDialog();

	// Set state icon
	mIconStateCtrl.SubclassDlgItem(IDC_MAILBOXGENERAL_STATE, this, mIconState);
	if (!mUseSynchronise)
		GetDlgItem(IDC_MAILBOXGENERAL_SYNCHRONISED)->EnableWindow(false);

	// Disable size calculate if a directory
	if (mIsDir)
		GetDlgItem(IDC_MAILBOXGENERAL_CALCULATE)->EnableWindow(false);
		
	return true;
}

// Set mbox list
void CPropMailboxGeneral::SetMboxList(CMboxList* mbox_list)
{
	// Save list
	mMboxList = mbox_list;

	// For now just use first item
	SetMbox((CMbox*) mMboxList->front());
}


// Set mbox list
void CPropMailboxGeneral::SetMbox(CMbox* mbox)
{
	// Do icon state
	if (mbox->GetProtocol()->CanDisconnect())
	{
		mIconState = mbox->GetProtocol()->IsDisconnected() ? IDI_MAILBOX_DISCONNECTED : IDI_MAILBOX_REMOTE;
		mUseSynchronise = true;
	}
	else if (mbox->IsLocalMbox())
	{
		mIconState = IDI_MAILBOX_LOCAL;
		mUseSynchronise = false;
	}
	else
	{
		mIconState = IDI_MAILBOX_REMOTE;
		mUseSynchronise = false;
	}

	// Copy text to edit fields
	mName = mbox->GetName();

	mServer = mbox->GetProtocol()->GetDescriptor();

	mHierarchy = mbox->GetWD();

	if (mbox->GetDirDelim())
		mSeparator = mbox->GetDirDelim();
	else
		mSeparator = cdstring::null_str;

	// Only set these if no error
	if (!mbox->Error() && !mbox->IsDirectory())
	{
		mUIDValidity = mbox->GetUIDValidity();

		{
			unsigned long utc_time = mbox->GetLastSync();
			
			if (utc_time)
			{
				// Determine timezone offset
				time_t t1 = ::time(NULL);
				time_t t2 = ::mktime(::gmtime(&t1));
				long offset = ::difftime(t1, t2);
				long current_zone = ::difftime(t1, t2) / 3600.0;	// Zone in hundreds e.g. -0500
				current_zone *= 100;
				utc_time += offset;

				mSynchronise = CRFC822::GetTextDate(utc_time, current_zone, true);
			}
			else
				mSynchronise = cdstring::null_str;
		}

		mTotal = mbox->GetNumberFound();

		mRecent = mbox->GetNumberRecent();

		mUnseen = mbox->GetNumberUnseen();

		// These are only available when open
		if (mbox->IsOpen())
			mDeleted = mbox->GetNumberDeleted();
		else
			mDeleted = cdstring::null_str;


		// These are only available when open and sync'ing is available
		if (mbox->IsOpen() && mbox->GetProtocol()->CanDisconnect() && (mbox->GetLastSync() != 0))
		{
			unsigned long fullsync = mbox->CountFlags(NMessage::eFullLocal);
			mFullSync = fullsync;

			unsigned long partialsync  = mbox->CountFlags(NMessage::ePartialLocal);
			mPartialSync = partialsync;
			
			// If disconnected we cannot know how many are missing
			if (mbox->GetProtocol()->IsDisconnected())
				mMissingSync = cdstring::null_str;
			else
				mMissingSync = mbox->GetNumberFound() - fullsync - partialsync;
		}
		else
		{
			mFullSync = cdstring::null_str;
			mPartialSync = cdstring::null_str;
			mMissingSync = cdstring::null_str;
		}

		{
			unsigned long size = mbox->GetSize();
			if ((size == ULONG_MAX) && mbox->IsLocalMbox() && mbox->IsCachedMbox())
			{
				mbox->CheckSize();
				size = mbox->GetSize();
			}
			if (size != ULONG_MAX)
			{
				mSize = ::GetNumericFormat(size);
			}
			else
				mSize = cdstring::null_str;
		}
	}
	else
	{
		mUIDValidity = cdstring::null_str;
		mSize = cdstring::null_str;
		mTotal = cdstring::null_str;
		mRecent = cdstring::null_str;
		mUnseen = cdstring::null_str;
		mDeleted = cdstring::null_str;
		mSynchronise = cdstring::null_str;
	}
	
	// Disable size calculate if a directory
	mIsDir = mbox->IsDirectory();

	cdstring copyStr;
	cdstring s;
	if (mbox->IsOpenSomewhere())
	{
		s.FromResource("UI::MailboxProp::StateOpen");
		copyStr += s;
	}
	if (mbox->IsSubscribed())
	{
		if (!copyStr.empty())
			copyStr += ", ";
		s.FromResource("UI::MailboxProp::StateSubs");
		copyStr += s;
	}
	if (mbox->NoInferiors())
	{
		if (!copyStr.empty())
			copyStr += ", ";
		s.FromResource("UI::MailboxProp::StateNoInfs");
		copyStr += s;
	}
	if (mbox->NoSelect())
	{
		if (!copyStr.empty())
			copyStr += ", ";
		s.FromResource("UI::MailboxProp::StateNoSel");
		copyStr += s;
	}
	if (mbox->Marked())
	{
		if (!copyStr.empty())
			copyStr += ", ";
		s.FromResource("UI::MailboxProp::StateMark");
		copyStr += s;
	}
	mStatus = copyStr;
}


// Check sizes of all mailboxes
void CPropMailboxGeneral::OnCalculateSize()
{
	// Iterate over all mailboxes
	for(CMboxList::iterator iter = mMboxList->begin(); iter != mMboxList->end(); iter++)
	{
		CMbox* mbox = static_cast<CMbox*>(*iter);
		if (!mbox->IsDirectory())
			mbox->CheckSize();
	}

	{
		unsigned long size = static_cast<CMbox*>(mMboxList->front())->GetSize();
		if (size != ULONG_MAX)
		{
			CUnicodeUtils::SetWindowTextUTF8(GetDlgItem(IDC_MAILBOXGENERAL_SIZE), ::GetNumericFormat(size));
		}
		else
			CUnicodeUtils::SetWindowTextUTF8(GetDlgItem(IDC_MAILBOXGENERAL_SIZE), cdstring::null_str);
	}
}
