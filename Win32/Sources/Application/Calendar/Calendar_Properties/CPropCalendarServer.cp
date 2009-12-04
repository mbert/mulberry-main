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


// CPropCalendarServer.cp : implementation file
//

#include "CPropCalendarServer.h"

#include "CCalendarProtocol.h"
#include "CUnicodeUtils.h"
#include "CViewCertDialog.h"
#include "CXStringResources.h"

/////////////////////////////////////////////////////////////////////////////
// CPropCalendarServer property page

IMPLEMENT_DYNCREATE(CPropCalendarServer, CHelpPropertyPage)

CPropCalendarServer::CPropCalendarServer() : CHelpPropertyPage(CPropCalendarServer::IDD)
{
	//{{AFX_DATA_INIT(CPropCalendarServer)
	mIconState = 0;
	//}}AFX_DATA_INIT
	mViewCert = true;
}

CPropCalendarServer::~CPropCalendarServer()
{
}

void CPropCalendarServer::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropCalendarServer)
	DDX_UTF8Text(pDX, IDC_CALENDARSERVER_NAME, mName);
	DDX_UTF8Text(pDX, IDC_CALENDARSERVER_TYPE, mType);
	DDX_UTF8Text(pDX, IDC_CALENDARSERVER_GREETING, mGreeting);
	DDX_UTF8Text(pDX, IDC_CALENDARSERVER_CAPABILITY, mCapability);
	DDX_UTF8Text(pDX, IDC_CALENDARSERVER_CONNECTION, mConnection);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropCalendarServer, CHelpPropertyPage)
	//{{AFX_MSG_MAP(CPropCalendarServer)
	ON_BN_CLICKED(IDC_CALENDARSERVER_VIEWCERT, OnViewCert)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropCalendarServer message handlers

BOOL CPropCalendarServer::OnInitDialog()
{
	CHelpPropertyPage::OnInitDialog();

	// Set state icon
	mIconStateCtrl.SubclassDlgItem(IDC_CALENDARSERVER_STATE, this, mIconState);
	
	if (!mViewCert)
		GetDlgItem(IDC_CALENDARSERVER_VIEWCERT)->EnableWindow(false);

	return true;
}

// Set fields in dialog
void CPropCalendarServer::SetProtocol(calstore::CCalendarProtocol* proto)
{
	mProto = proto;

	// Do icon state
	if (mProto->IsLocalCalendar())
	{
		mIconState = IDI_MAILBOX_LOCAL;
	}
	else if (mProto->IsWebCalendar())
	{
		mIconState = IDI_MAILBOX_DISCONNECTED;
	}
	else if (mProto->CanDisconnect())
	{
		mIconState = mProto->IsDisconnected() ? IDI_MAILBOX_DISCONNECTED : IDI_MAILBOX_REMOTE;
	}
	else
	{
		mIconState = IDI_MAILBOX_REMOTE;
	}

	mName = mProto->GetDescriptor();
	mType = mProto->GetType();
	mGreeting = mProto->GetGreeting();
	mCapability = mProto->GetCapability();

	// Check connection state
	if (mProto->IsSecure())
		mConnection = rsrc::GetString("UI::ServerProp::Secure");
	else
	{
		mConnection = rsrc::GetString("UI::ServerProp::NotSecure");
		mViewCert = false;
	}
}

// Force update of items
void CPropCalendarServer::OnViewCert()
{
	// get text version of cert from server
	cdstring cert = mProto->GetCertText();

	// View server certificate
	CViewCertDialog::PoseDialog(cert);
}
