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


// Source for CPropCalendarServer class

#include "CPropCalendarServer.h"

#include "CCalendarProtocol.h"
#include "CConnectionManager.h"
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

// Constructor from stream
CPropCalendarServer::CPropCalendarServer(LStream *inStream)
		: CCalendarPropPanel(inStream)
{
}

// Default destructor
CPropCalendarServer::~CPropCalendarServer()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPropCalendarServer::FinishCreateSelf(void)
{
	// Do inherited
	CCalendarPropPanel::FinishCreateSelf();

	// Icon state
	mIconState = (LIconControl*) FindPaneByID(paneid_CalendarServerState);

	// Get items
	mName = (CTextFieldX*) FindPaneByID(paneid_CalendarServerName);
	mType = (CTextFieldX*) FindPaneByID(paneid_CalendarServerType);
	mGreeting = (CTextDisplay*) FindPaneByID(paneid_CalendarServerGreeting);
	mCapability = (CTextDisplay*) FindPaneByID(paneid_CalendarServerCapability);
	mConnectionTitle = (LStaticText*) FindPaneByID(paneid_CalendarServerConnectionTitle);
	mConnection = (CTextFieldX*) FindPaneByID(paneid_CalendarServerConnection);
	mViewCert = (LPushButton*) FindPaneByID(paneid_CalendarServerViewCert);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPropCalendarServerBtns);
}

// Handle buttons
void CPropCalendarServer::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_CalendarServerViewCert:
		OnViewCert();
		break;
	}
}

// Set mbox list
void CPropCalendarServer::SetCalList(calstore::CCalendarStoreNodeList* cal_list)
{
	// Do nothing
}

// Set protocol
void CPropCalendarServer::SetProtocol(calstore::CCalendarProtocol* server)
{
	mProto = server;

	// Do icon state
	if (server->IsLocalCalendar())
	{
		mIconState->SetResourceID(icnx_MailboxStateLocal);
	}
	else if (server->IsWebCalendar())
	{
		mIconState->SetResourceID(icnx_MailboxStateDisconnected);
	}
	else if (server->CanDisconnect())
	{
		mIconState->SetResourceID(server->IsDisconnected() ? icnx_MailboxStateDisconnected : icnx_MailboxStateRemote);
	}
	else
	{
		mIconState->SetResourceID(icnx_MailboxStateRemote);
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
void CPropCalendarServer::ApplyChanges()
{
	// Nothing to update
}

// Force update of items
void CPropCalendarServer::OnViewCert()
{
	// get text version of cert from server
	cdstring cert = mProto->GetCertText();

	// View server certificate
	CViewCertDialog::PoseDialog(cert);
}
