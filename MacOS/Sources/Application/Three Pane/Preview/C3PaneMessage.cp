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


// Source for C3PaneMessage class

#include "C3PaneMessage.h"

#include "CCommands.h"
#include "CMessage.h"
#include "CMessageView.h"
#include "CResources.h"
#include "CToolbarButton.h"
#include "C3PaneWindow.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X L I S T W I N D O W
// __________________________________________________________________________________________________

// Static

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneMessage::C3PaneMessage()
{
}

// Constructor from stream
C3PaneMessage::C3PaneMessage(LStream *inStream)
		: C3PaneViewPanel(inStream)
{
}

// Default destructor
C3PaneMessage::~C3PaneMessage()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void C3PaneMessage::FinishCreateSelf()
{
	// Do inherited
	C3PaneViewPanel::FinishCreateSelf();

	// Get sub-panes
	mMessageView = (CMessageView*) FindPaneByID(paneid_3PaneView);
	
	CToolbarButton* btn = (CToolbarButton*) FindPaneByID(paneid_3PaneMessageZoom);
	btn->AddListener(this);
	btn->SetSmallIcon(true);
	btn->SetShowIcon(true);
	btn->SetShowCaption(false);

	// Hide it until a message is set
	Hide();
}

CBaseView* C3PaneMessage::GetBaseView() const
{
	return mMessageView;
}

// Respond to clicks in the icon buttons
void C3PaneMessage::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage)
	{
	case msg_3PaneMessageZoom:
		OnZoom();
		break;
	}
}

void C3PaneMessage::OnZoom()
{
	m3PaneWindow->ObeyCommand(cmd_ToolbarZoomPreview, NULL);
}

bool C3PaneMessage::TestClose()
{
	// Can always close
	return true;
}

bool C3PaneMessage::IsSpecified() const
{
	return mMessageView->GetMessage() != NULL;
}

void C3PaneMessage::SetMessage(CMessage* msg)
{
	// Give it to message view to display it
	mMessageView->SetMessage(msg);
}

cdstring C3PaneMessage::GetTitle() const
{
	CMessage* msg = (mMessageView ? mMessageView->GetMessage() : NULL);
	return (msg && msg->GetEnvelope()) ? msg->GetEnvelope()->GetSubject() : cdstring::null_str;
}

ResIDT C3PaneMessage::GetIconID() const
{
	return ICNx_3PanePreviewMessage;
}
