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


// Source for C3PaneEvent class

#include "C3PaneEvent.h"

#include "CCommands.h"
#include "CEventPreview.h"
#include "CResources.h"
#include "CStaticText.h"
#include "CToolbarButton.h"
#include "C3PaneWindow.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X L I S T W I N D O W
// __________________________________________________________________________________________________

// Static

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneEvent::C3PaneEvent()
{
}

// Constructor from stream
C3PaneEvent::C3PaneEvent(LStream *inStream)
		: C3PaneViewPanel(inStream)
{
}

// Default destructor
C3PaneEvent::~C3PaneEvent()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void C3PaneEvent::FinishCreateSelf()
{
	// Do inherited
	C3PaneViewPanel::FinishCreateSelf();

	// Get sub-panes
	mDescriptor = (CStaticText*) FindPaneByID(paneid_3PaneEventDesc);
	mEventView = (CEventPreview*) FindPaneByID(paneid_3PaneView);
	
	CToolbarButton* btn = (CToolbarButton*) FindPaneByID(paneid_3PaneEventZoom);
	btn->AddListener(this);
	btn->SetSmallIcon(true);
	btn->SetShowIcon(true);
	btn->SetShowCaption(false);

	// Hide it until a message is set
	Hide();
}

CBaseView* C3PaneEvent::GetBaseView() const
{
	return mEventView;
}

// Respond to clicks in the icon buttons
void C3PaneEvent::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage)
	{
	case msg_3PaneEventZoom:
		OnZoom();
		break;
	}
}

void C3PaneEvent::OnZoom()
{
	m3PaneWindow->ObeyCommand(cmd_ToolbarZoomPreview, NULL);
}

bool C3PaneEvent::TestClose()
{
	// Can always close
	return true;
}

bool C3PaneEvent::IsSpecified() const
{
	return mEventView->HasData();
}

void C3PaneEvent::SetComponent(const iCal::CICalendarComponentExpandedShared& comp)
{
	// Give it to event view to display it
	mEventView->SetComponent(comp);
}

cdstring C3PaneEvent::GetTitle() const
{
	return mEventView ? mEventView->GetSummary() : cdstring::null_str;
}

void C3PaneEvent::SetTitle(const cdstring& title)
{
	mDescriptor->SetText(title);
}

ResIDT C3PaneEvent::GetIconID() const
{
	return ICNx_3PanePreviewCalendar;
}
