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

#include "CIconLoader.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CStaticText.h"
#include "CTextDisplay.h"
#include "CViewCertDialog.h"
#include "CXStringResources.h"

#include <JXColormap.h>
#include <JXDownRect.h>
#include <JXImageWidget.h>
#include <JXStaticText.h>
#include <JXTextButton.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPropMailboxServer::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Server", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 55,20);
    assert( obj1 != NULL );
    const JFontStyle obj1_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj1->SetFontStyle(obj1_style);

    mIconState =
        new JXImageWidget(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 414,4, 32,32);
    assert( mIconState != NULL );
    mIconState->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj2 =
        new JXStaticText("Name:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,22, 45,20);
    assert( obj2 != NULL );

    mName =
        new CStaticText("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,20, 315,20);
    assert( mName != NULL );
    mName->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj3 =
        new JXStaticText("Type:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,47, 60,20);
    assert( obj3 != NULL );

    mType =
        new CStaticText("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,45, 315,20);
    assert( mType != NULL );
    mType->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj4 =
        new JXStaticText("Greeting:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,72, 65,20);
    assert( obj4 != NULL );

    mGreeting =
        new CTextDisplay(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,70, 315,80);
    assert( mGreeting != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Capability:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,157, 70,20);
    assert( obj5 != NULL );

    mCapability =
        new CTextDisplay(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,155, 315,80);
    assert( mCapability != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Connection:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,254, 75,20);
    assert( obj6 != NULL );

    mConnection =
        new CStaticText("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,252, 205,20);
    assert( mConnection != NULL );
    mConnection->SetBorderWidth(kJXDefaultBorderWidth);

    mViewCert =
        new JXTextButton("View Certificate", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 300,250, 105,24);
    assert( mViewCert != NULL );
    mViewCert->SetFontSize(10);

// end JXLayout1

	ListenTo(mViewCert);
}

// Handle buttons
void CPropMailboxServer::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXTextButton::kPushed))
	{
		if (sender == mViewCert)
		{
			OnViewCert();
			return;
		}
	}

	CMailboxPropPanel::Receive(sender, message);
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
			mIconState->SetImage(CIconLoader::GetIcon(IDI_MAILBOX_DISCONNECTED, mIconState, 32, 0x00CCCCCC),  kFalse);
		else
			mIconState->SetImage(CIconLoader::GetIcon(IDI_MAILBOX_REMOTE, mIconState, 32, 0x00CCCCCC),  kFalse);
		break;
	case CINETAccount::ePOP3:
		mIconState->SetImage(CIconLoader::GetIcon(IDI_MAILBOX_POP3, mIconState, 32, 0x00CCCCCC),  kFalse);
		break;
	case CINETAccount::eLocal:
		mIconState->SetImage(CIconLoader::GetIcon(IDI_MAILBOX_LOCAL, mIconState, 32, 0x00CCCCCC),  kFalse);
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
		mViewCert->Deactivate();
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
