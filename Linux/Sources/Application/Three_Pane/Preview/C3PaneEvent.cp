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
#include "CIconLoader.h"
#include "CStaticText.h"
#include "CToolbarButton.h"
#include "C3PaneWindow.h"

#include <JXImageWidget.h>
#include <JXUpRect.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneEvent::C3PaneEvent(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h)
	: C3PaneViewPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Default destructor
C3PaneEvent::~C3PaneEvent()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void C3PaneEvent::OnCreate()
{
	// Do inherited
	C3PaneViewPanel::OnCreate();

// begin JXLayout1

    mZoomBtn =
        new CToolbarButton("",this,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 280,2, 20,20);
    assert( mZoomBtn != NULL );

    JXUpRect* obj1 =
        new JXUpRect(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,0, 275,24);
    assert( obj1 != NULL );

    mIcon =
        new JXImageWidget(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,2, 16,16);
    assert( mIcon != NULL );

    mDescriptor =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 25,2, 245,16);
    assert( mDescriptor != NULL );

    mViewPane =
        new JXWidgetSet(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,24, 300,176);
    assert( mViewPane != NULL );

// end JXLayout1

	mZoomBtn->SetImage(IDI_3PANE_ZOOM, 0);
	mZoomBtn->SetSmallIcon(true);
	mZoomBtn->SetShowIcon(true);
	mZoomBtn->SetShowCaption(false);

	mIcon->SetImage(CIconLoader::GetIcon(IDI_CALENDAR, mIcon, 16, 0x00CCCCCC), kFalse);

	ListenTo(mZoomBtn);

	// Get sub-panes
	mEventView = new CEventPreview(mViewPane, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 500, 300);
	mEventView->OnCreate();
	mEventView->FitToEnclosure(kTrue, kTrue);
	
	// Hide it until an address is set
	Hide();
}

// Respond to clicks in the icon buttons
void C3PaneEvent::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mZoomBtn)
		{
			OnZoom();
			return;
		}
	}
}

void C3PaneEvent::OnZoom()
{
	m3PaneWindow->ObeyCommand(CCommand::eToolbarZoomPreview, NULL);
}

CBaseView* C3PaneEvent::GetBaseView() const
{
	return mEventView;
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
	return mEventView->GetSummary();
}

void C3PaneEvent::SetTitle(const cdstring& title)
{
	mDescriptor->SetText(title);
}

unsigned int C3PaneEvent::GetIconID() const
{
	return IDI_CALENDAR;
}
