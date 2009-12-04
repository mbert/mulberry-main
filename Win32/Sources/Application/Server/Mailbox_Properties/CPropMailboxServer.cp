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


// CPropMailboxServer.cp : implementation file
//

#include "CPropMailboxServer.h"

#include "CMailAccountManager.h"
#include "CMboxProtocol.h"
#include "CUnicodeUtils.h"
#include "CViewCertDialog.h"
#include "CXStringResources.h"

/////////////////////////////////////////////////////////////////////////////
// CPropMailboxServer property page

IMPLEMENT_DYNCREATE(CPropMailboxServer, CHelpPropertyPage)

CPropMailboxServer::CPropMailboxServer() : CHelpPropertyPage(CPropMailboxServer::IDD)
{
	//{{AFX_DATA_INIT(CPropMailboxServer)
	mIconState = 0;
	//}}AFX_DATA_INIT
}

CPropMailboxServer::~CPropMailboxServer()
{
}

void CPropMailboxServer::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropMailboxServer)
	DDX_UTF8Text(pDX, IDC_MAILBOXSERVER_NAME, mMailboxServerName);
	DDX_UTF8Text(pDX, IDC_MAILBOXSERVER_TYPE, mMailboxServerType);
	DDX_UTF8Text(pDX, IDC_MAILBOXSERVER_GREETING, mMailboxServerGreeting);
	DDX_UTF8Text(pDX, IDC_MAILBOXSERVER_CAPABILITY, mMailboxServerCapability);
	DDX_UTF8Text(pDX, IDC_MAILBOXSERVER_CONNECTION, mMailboxServerConnection);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropMailboxServer, CHelpPropertyPage)
	//{{AFX_MSG_MAP(CPropMailboxServer)
	ON_BN_CLICKED(IDC_MAILBOXSERVER_VIEWCERT, OnViewCert)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropMailboxServer message handlers

BOOL CPropMailboxServer::OnInitDialog()
{
	CHelpPropertyPage::OnInitDialog();

	// Set state icon
	mIconStateCtrl.SubclassDlgItem(IDC_MAILBOXSERVER_STATE, this, mIconState);
	
	if (!mViewCert)
		GetDlgItem(IDC_MAILBOXSERVER_VIEWCERT)->EnableWindow(false);

	return true;
}

// Set fields in dialog
void CPropMailboxServer::SetProtocol(CMboxProtocol* server)
{
	mProto = server;

	// Do icon state
	switch(server->GetAccountType())
	{
	case CINETAccount::eIMAP:
		if (server->CanDisconnect() && server->IsDisconnected())
			mIconState = IDI_MAILBOX_DISCONNECTED;
		else
			mIconState = IDI_MAILBOX_REMOTE;
		break;
	case CINETAccount::ePOP3:
		mIconState = IDI_MAILBOX_POP3;
		break;
	case CINETAccount::eLocal:
		mIconState = IDI_MAILBOX_LOCAL;
		break;
	}

	mMailboxServerName = server->GetDescriptor();
	mMailboxServerType = server->GetType();
	mMailboxServerGreeting = server->GetGreeting();
	mMailboxServerCapability = server->GetCapability();

	// Check connection state
	if (mProto->IsSecure())
		mMailboxServerConnection = rsrc::GetString("UI::ServerProp::Secure");
	else
	{
		mMailboxServerConnection = rsrc::GetString("UI::ServerProp::NotSecure");
		mViewCert = false;
	}
}

// Force update of items
void CPropMailboxServer::OnViewCert()
{
	// get text version of cert from server
	cdstring cert = mProto->GetCertText();

	// View server certificate
	CViewCertDialog::PoseDialog(cert);
}
