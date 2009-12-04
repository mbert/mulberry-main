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


// Source for C3PaneAccounts class

#include "C3PaneAccounts.h"

#include "CAdbkManagerView.h"
#include "CAdminLock.h"
#include "CCalendarStoreView.h"
#include "CCommands.h"
#include "CServerView.h"
#include "CToolbarButton.h"
#include "C3PaneCalendarAccount.h"
#include "C3PaneContactsAccount.h"
#include "C3PaneMailAccount.h"
#include "C3PaneWindow.h"

#include <LTabsControl.h>

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X L I S T W I N D O W
// __________________________________________________________________________________________________

// Static

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneAccounts::C3PaneAccounts()
{
	mMailAccounts = NULL;
	mContactsAccounts = NULL;
	mCalendarAccounts = NULL;
}

// Constructor from stream
C3PaneAccounts::C3PaneAccounts(LStream *inStream)
		: C3PaneParentPanel(inStream)
{
	mMailAccounts = NULL;
	mContactsAccounts = NULL;
	mCalendarAccounts = NULL;
}

// Default destructor
C3PaneAccounts::~C3PaneAccounts()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void C3PaneAccounts::FinishCreateSelf()
{
	// Do inherited
	C3PaneParentPanel::FinishCreateSelf();

	// Get sub-panes
	mTabs = (LTabsControl*) FindPaneByID(paneid_3PaneAccountsTabs);
	mTabs->AddListener(this);
	
	// Remove calendar if disabled by admin
	if (CAdminLock::sAdminLock.mPreventCalendars)
	{
		// Reset maximum to remove button
		mTabs->SetMaxValue(2);
	}
	
	CToolbarButton* btn = (CToolbarButton*) FindPaneByID(paneid_3PaneAccountsZoom);
	btn->AddListener(this);
	btn->SetSmallIcon(true);
	btn->SetShowIcon(true);
	btn->SetShowCaption(false);

	// Always need these
	MakeMail();
	MakeContacts();
	MakeCalendar();

}

void C3PaneAccounts::MakeMail()
{
	// Find commander in super view hierarchy
	LView* super = GetSuperView();
	while(super && !dynamic_cast<LCommander*>(super))
		super = super->GetSuperView();
	LCommander* cmdr = dynamic_cast<LCommander*>(super);

	// Read the mail account view resource
	mMailAccounts = static_cast<C3PaneMailAccount*>(UReanimator::CreateView(paneid_3PaneMailAccount, mView, cmdr));
	mView->ExpandSubPane(mMailAccounts, true, true);
	mMailAccounts->Hide();
	
	// Create a toolbar for it
	MakeToolbars(GetServerView());
}

void C3PaneAccounts::MakeContacts()
{
	// Find commander in super view hierarchy
	LView* super = GetSuperView();
	while(super && !dynamic_cast<LCommander*>(super))
		super = super->GetSuperView();
	LCommander* cmdr = dynamic_cast<LCommander*>(super);

	// Read the contacts account view resource
	mContactsAccounts = static_cast<C3PaneContactsAccount*>(UReanimator::CreateView(paneid_3PaneContactsAccount, mView, cmdr));
	mView->ExpandSubPane(mContactsAccounts, true, true);
	mContactsAccounts->Hide();
	
	// Create a toolbar for it
	MakeToolbars(GetContactsView());
}

void C3PaneAccounts::MakeCalendar()
{
	// Don't do if admin locks it out
	if (CAdminLock::sAdminLock.mPreventCalendars)
		return;

	// Find commander in super view hierarchy
	LView* super = GetSuperView();
	while(super && !dynamic_cast<LCommander*>(super))
		super = super->GetSuperView();
	LCommander* cmdr = dynamic_cast<LCommander*>(super);

	// Read the contacts account view resource
	mCalendarAccounts = static_cast<C3PaneCalendarAccount*>(UReanimator::CreateView(paneid_3PaneCalendarAccount, mView, cmdr));
	mView->ExpandSubPane(mCalendarAccounts, true, true);
	mCalendarAccounts->Hide();
	
	// Create a toolbar for it
	MakeToolbars(GetCalendarStoreView());
}

// Respond to clicks in the icon buttons
void C3PaneAccounts::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage)
	{
	case msg_3PaneAccountsTabs:
		OnTabs(*(unsigned long*) ioParam);
		break;
	case msg_3PaneAccountsZoom:
		OnZoom();
		break;
	}
}

void C3PaneAccounts::OnTabs(unsigned long index)
{
	N3Pane::EViewType view;
	switch(index)
	{
	case 1:
	default:
		view = N3Pane::eView_Mailbox;
		break;
	case 2:
		view = N3Pane::eView_Contacts;
		break;
	case 3:
		view = N3Pane::eView_Calendar;
		break;
	}

	// If we are a child of a 3-pane window then tell the 3-pane we are changing
	if (m3PaneWindow)
		// The 3 pane window will call this->SetViewType
		m3PaneWindow->SetViewType(view);
	else
		// Call this->SetViewType explicitly
		SetViewType(view);
}

void C3PaneAccounts::OnZoom()
{
	m3PaneWindow->ObeyCommand(cmd_ToolbarZoomList, NULL);
}

bool C3PaneAccounts::TestClose()
{
	// Test each pane that exists
	if (mMailAccounts && !mMailAccounts->TestClose())
		return false;
	if (mContactsAccounts && !mContactsAccounts->TestClose())
		return false;
	if (mCalendarAccounts && !mCalendarAccounts->TestClose())
		return false;
	
	return true;
}

void C3PaneAccounts::DoClose()
{
	// Close each pane that exists
	if (mMailAccounts)
		mMailAccounts->DoClose();
	if (mContactsAccounts)
		mContactsAccounts->DoClose();
	if (mCalendarAccounts)
		mCalendarAccounts->DoClose();
}

void C3PaneAccounts::SetViewType(N3Pane::EViewType view)
{
	// Only bother if different
	if (mViewType == view)
		return;

	// Hide current view
	if (mCurrent)
		mCurrent->Hide();

	// Set the pane contents
	mViewType = view;
	unsigned long tab_index = 0;
	switch(mViewType)
	{
	case N3Pane::eView_Mailbox:
		// Show the mail account
		if (!mMailAccounts)
			MakeMail();
		mCurrent = mMailAccounts;
		tab_index = 1;
		break;

	case N3Pane::eView_Contacts:
		// Show the contacts account
		if (!mContactsAccounts)
			MakeContacts();
		mCurrent = mContactsAccounts;
		tab_index = 2;
		break;

	case N3Pane::eView_Calendar:
		// Show the calendar account
		if (!mCalendarAccounts)
			MakeCalendar();
		mCurrent = mCalendarAccounts;
		tab_index = 3;
		break;

	case N3Pane::eView_Empty:
	//case N3Pane::eView_IM:
	//case N3Pane::eView_Bookmarks:
		// Delete the content of all views
		mCurrent = NULL;
		break;
	}

	// Make sure tabs are in sync
	if (mTabs->GetValue() != tab_index)
	{
		StopListening();
		mTabs->SetValue(tab_index);
		StartListening();
	}

	// Show the new one and always focus on it
	if (mCurrent)
	{
		mCurrent->Show();
		mCurrent->Focus();
	}
}

// Get server view from sub-pane
CServerView* C3PaneAccounts::GetServerView() const
{
	return mMailAccounts ? mMailAccounts->GetServerView() : NULL;
}

// Get server view from sub-pane
CAdbkManagerView* C3PaneAccounts::GetContactsView() const
{
	return mContactsAccounts ? mContactsAccounts->GetAdbkManagerView() : NULL;
}

// Get view from sub-pane
CCalendarStoreView* C3PaneAccounts::GetCalendarStoreView() const
{
	return mCalendarAccounts ? mCalendarAccounts->GetCalendarStoreView() : NULL;
}

// Reset state from prefs
void C3PaneAccounts::ResetState()
{
	switch(mViewType)
	{
	case N3Pane::eView_Mailbox:
		if (GetServerView())
			GetServerView()->ResetState(true);
		break;

	case N3Pane::eView_Contacts:
		if (GetContactsView())
			GetContactsView()->ResetState(true);
		break;

	case N3Pane::eView_Calendar:
		if (GetCalendarStoreView())
			GetCalendarStoreView()->ResetState(true);
		break;

	default:;
	}
}

// Save state in prefs
void C3PaneAccounts::SaveDefaultState()
{
	switch(mViewType)
	{
	case N3Pane::eView_Mailbox:
		if (GetServerView())
			GetServerView()->SaveDefaultState();
		break;

	case N3Pane::eView_Contacts:
		if (GetContactsView())
			GetContactsView()->SaveDefaultState();
		break;

	case N3Pane::eView_Calendar:
		if (GetCalendarStoreView())
			GetCalendarStoreView()->SaveDefaultState();
		break;

	default:;
	}
}
