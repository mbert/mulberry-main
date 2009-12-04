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


// Source for C3PanePreview class

#include "C3PanePreview.h"

#include "CAddressView.h"
#include "CAdminLock.h"
#include "CEventPreview.h"
#include "CMessageView.h"
#include "C3PaneAddress.h"
#include "C3PaneEvent.h"
#include "C3PaneMessage.h"
#include "C3PaneWindow.h"

#include <JXStaticText.h>

#include <cassert>

// Static

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PanePreview::C3PanePreview(JXContainer* enclosure,
						 const HSizingOption hSizing, 
						 const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h) 
	: C3PaneParentPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
	mMessage = NULL;
	mAddress = NULL;
	mEvent = NULL;
}

// Default destructor
C3PanePreview::~C3PanePreview()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void C3PanePreview::OnCreate()
{
	// Do inherited
	C3PaneParentPanel::OnCreate();

// begin JXLayout1

    mViewPane =
        new JXWidgetSet(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 300,200);
    assert( mViewPane != NULL );

// end JXLayout1

	mView = mViewPane;

	// Always need these
	MakeMessage();
	MakeAddress();
	MakeEvent();
}

void C3PanePreview::ListenTo_Message(long msg, void* param)
{
	// Look for messages
	switch(msg)
	{
	// Change in message view contents
	case CBaseView::eBroadcast_ViewChanged:
		{
			C3PanePanel* broadcaster = NULL;

			if (reinterpret_cast<CMessageView*>(param) == GetMessageView())
				broadcaster = mMessage;
			else if (reinterpret_cast<CAddressView*>(param) == GetAddressView())
				broadcaster = mAddress;
			else if (reinterpret_cast<CEventPreview*>(param) == GetEventView())
				broadcaster = mEvent;

			// Only if its one of ours
			if (broadcaster)
			{
				// Change title if view is displayed
				if (mCurrent == broadcaster)
				{
					SetTitle(broadcaster->GetTitle());
					SetIcon(broadcaster->GetIconID());
				}
				
				// Check active status
				if (broadcaster->IsSpecified())
					broadcaster->Activate();
				else
				{
					broadcaster->Deactivate();

					// Make sure we reset zoom state on deactivate
					if (C3PaneWindow::s3PaneWindow->GetZoomPreview())
						C3PaneWindow::s3PaneWindow->ZoomPreview(false);
				}
			}
		}
		break;

	default:;
	}
}

void C3PanePreview::MakeMessage()
{
	// Read the message view resource
	mMessage = new C3PaneMessage(mView, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 100, 100);
	mMessage->OnCreate();
	mMessage->FitToEnclosure(kTrue, kTrue);
	mMessage->Hide();
	
	// Setup listeners
	GetMessageView()->Add_Listener(this);
	
	// Create a toolbar for it
	MakeToolbars(GetMessageView());
}

void C3PanePreview::MakeAddress()
{
	// Read the message view resource
	mAddress = new C3PaneAddress(mView, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 100, 100);
	mAddress->OnCreate();
	mAddress->FitToEnclosure(kTrue, kTrue);
	mAddress->Hide();
	
	// Setup listeners
	GetAddressView()->Add_Listener(this);
	
	// Create a toolbar for it
	MakeToolbars(GetAddressView());
}

void C3PanePreview::MakeEvent()
{
	// Don't do if admin locks it out
	if (CAdminLock::sAdminLock.mPreventCalendars)
		return;

	// Read the message view resource
	mEvent = new C3PaneEvent(mView, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 300, 200);
	mEvent->OnCreate();
	mEvent->FitToEnclosure(kTrue, kTrue);
	mEvent->Hide();
	
	// Setup listeners
	GetEventView()->Add_Listener(this);
	
	// Create a toolbar for it
	MakeToolbars(GetEventView());
}

bool C3PanePreview::IsSpecified() const
{
	// Check that sub view is specified
	return mCurrent->IsSpecified();
}

// Set the title for the preview
void C3PanePreview::SetTitle(const cdstring& title)
{
	if (mCurrent == mEvent)
		mEvent->SetTitle(title);
}

// Set the icon for the preview
void C3PanePreview::SetIcon(unsigned int icon)
{
}

bool C3PanePreview::TestClose()
{
	// Test each pane that exists
	if (mMessage && !mMessage->TestClose())
		return false;
	if (mAddress && !mAddress->TestClose())
		return false;
	if (mEvent && !mEvent->TestClose())
		return false;
	
	return true;
}

void C3PanePreview::DoClose()
{
	// Close each pane that exists
	if (mMessage)
		mMessage->DoClose();
	if (mAddress)
		mAddress->DoClose();
	if (mEvent)
		mEvent->DoClose();
}

void C3PanePreview::SetViewType(N3Pane::EViewType view)
{
	// Only bother if different
	if (mViewType == view)
		return;

	// Hide current view
	if (mCurrent)
		mCurrent->Hide();

	// Set the pane contents
	mViewType = view;
	switch(mViewType)
	{
	case N3Pane::eView_Mailbox:
		// Show the message preview
		if (!mMessage)
			MakeMessage();
		mCurrent = mMessage;
		break;

	case N3Pane::eView_Contacts:
		// Show the address preview
		if (!mAddress)
			MakeAddress();
		mCurrent = mAddress;
		break;

	case N3Pane::eView_Calendar:
		// Show the address preview
		if (!mEvent)
			MakeEvent();
		mCurrent = mEvent;
		break;

	case N3Pane::eView_Empty:
	//case N3Pane::eView_IM:
	//case N3Pane::eView_Bookmarks:
	default:;
		// Delete the content of all views
		mCurrent = NULL;
		break;
	}

	// Show the new one
	if (mCurrent)
	{
		mCurrent->Show();
		if (mCurrent->IsSpecified())
			mCurrent->Activate();
		else
			mCurrent->Deactivate();

		// Change title and icon when view changed
		SetTitle(mCurrent->GetTitle());
		SetIcon(mCurrent->GetIconID());
	}
}

// Get message view from sub-pane
CMessageView* C3PanePreview::GetMessageView() const
{
	return mMessage ? mMessage->GetMessageView() : NULL;
}

// Get address view from sub-pane
CAddressView* C3PanePreview::GetAddressView() const
{
	return mAddress ? mAddress->GetAddressView() : NULL;
}

// Get address view from sub-pane
CEventPreview* C3PanePreview::GetEventView() const
{
	return mEvent ? mEvent->GetEventView() : NULL;
}

// Reset state from prefs
void C3PanePreview::ResetState()
{
	switch(mViewType)
	{
	case N3Pane::eView_Mailbox:
		if (GetMessageView())
			GetMessageView()->ResetState(true);
		break;

	case N3Pane::eView_Contacts:
		if (GetAddressView())
			GetAddressView()->ResetState(true);
		break;

	case N3Pane::eView_Calendar:
		if (GetEventView())
			GetEventView()->ResetState(true);
		break;

	default:;
	}
}

// Save state in prefs
void C3PanePreview::SaveDefaultState()
{
	switch(mViewType)
	{
	case N3Pane::eView_Mailbox:
		if (GetMessageView())
			GetMessageView()->SaveDefaultState();
		break;

	case N3Pane::eView_Contacts:
		if (GetAddressView())
			GetAddressView()->SaveDefaultState();
		break;

	case N3Pane::eView_Calendar:
		if (GetEventView())
			GetEventView()->SaveDefaultState();
		break;

	default:;
	}
}
