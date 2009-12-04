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


// Source for CPropMailboxServer class

#include "CPropMailboxServer.h"

#include "CConnectionManager.h"
#include "CMboxProtocol.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CResources.h"
#include "CTextDisplay.h"
#include "CTextFieldX.h"
#include "CViewCertDialog.h"
#include "CXStringResources.h"

#include <LIconControl.h>
#include <LPushButton.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPropMailboxServer::CPropMailboxServer()
{
}

// Constructor from stream
CPropMailboxServer::CPropMailboxServer(LStream *inStream)
		: CMailboxPropPanel(inStream)
{
}

// Default destructor
CPropMailboxServer::~CPropMailboxServer()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPropMailboxServer::FinishCreateSelf(void)
{
	// Do inherited
	CMailboxPropPanel::FinishCreateSelf();

	// Icon state
	mIconState = (LIconControl*) FindPaneByID(paneid_MailboxServerState);

	// Get items
	mName = (CTextFieldX*) FindPaneByID(paneid_MailboxServerName);
	mType = (CTextFieldX*) FindPaneByID(paneid_MailboxServerType);
	mGreeting = (CTextDisplay*) FindPaneByID(paneid_MailboxServerGreeting);
	mCapability = (CTextDisplay*) FindPaneByID(paneid_MailboxServerCapability);
	mConnectionTitle = (LStaticText*) FindPaneByID(paneid_MailboxServerConnectionTitle);
	mConnection = (CTextFieldX*) FindPaneByID(paneid_MailboxServerConnection);
	mViewCert = (LPushButton*) FindPaneByID(paneid_MailboxServerViewCert);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPropMailboxServerBtns);
}

// Handle buttons
void CPropMailboxServer::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_MailboxServerViewCert:
		OnViewCert();
		break;
	}
}

// Set mbox list
void CPropMailboxServer::SetMboxList(CMboxList* mbox_list)
{
	// Do nothing
}

// Set protocol
void CPropMailboxServer::SetProtocol(CMboxProtocol* server)
{
	mProto = server;

	// Do icon state
	switch(server->GetAccountType())
	{
	case CINETAccount::eIMAP:
		if (server->CanDisconnect() && server->IsDisconnected())
			mIconState->SetResourceID(icnx_MailboxStateDisconnected);
		else
			mIconState->SetResourceID(icnx_MailboxStateRemote);
		break;
	case CINETAccount::ePOP3:
		mIconState->SetResourceID(icnx_MailboxStatePOP3);
		break;
	case CINETAccount::eLocal:
		mIconState->SetResourceID(icnx_MailboxStateLocal);
		break;
	default:;
	}

	// Do server data
	mName->SetText(server->GetDescriptor());

	mType->SetText(server->GetType());

	mGreeting->SetText(server->GetGreeting());

	mCapability->SetText(server->GetCapability());
	
	// Check connection state
	if (server->IsSecure())
		mConnection->SetText(rsrc::GetString("UI::ServerProp::Secure"));
	else
	{
		mConnection->SetText(rsrc::GetString("UI::ServerProp::NotSecure"));
		mViewCert->Disable();
	}
}

// Force update of items
void CPropMailboxServer::ApplyChanges(void)
{
	// Nothing to update
}

// Force update of items
void CPropMailboxServer::OnViewCert()
{
	// get text version of cert from server
	cdstring cert = mProto->GetCertText();

	// View server certificate
	CViewCertDialog::PoseDialog(cert);
}
