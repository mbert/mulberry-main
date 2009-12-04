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
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CServerView.h"
#include "C3PaneCalendarAccount.h"
#include "C3PaneContactsAccount.h"
#include "C3PaneMailAccount.h"
#include "C3PaneWindow.h"

BEGIN_MESSAGE_MAP(C3PaneAccounts, C3PaneParentPanel)
	ON_WM_CREATE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_3PANEACCOUNTS_TABS, OnSelChangeTabs)
	ON_COMMAND(IDC_3PANETOOLBAR_ZOOMLIST, OnZoomPane)
END_MESSAGE_MAP()

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

// Default destructor
C3PaneAccounts::~C3PaneAccounts()
{
	delete mMailAccounts;
	delete mContactsAccounts;
	delete mCalendarAccounts;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int C3PaneAccounts::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (C3PaneParentPanel::OnCreate(lpCreateStruct) == -1)
		return -1;

	const int cSmallIconBtnSize = 20;
	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	// Tabs with alignment
	const int cTabsHeight = 32;
	mTabs.Create(WS_CHILD | WS_VISIBLE, CRect(3, 0, width - cSmallIconBtnSize, cTabsHeight), this, IDC_3PANEACCOUNTS_TABS);
	AddAlignment(new CWndAlignment(&mTabs, CWndAlignment::eAlign_TopWidth));

	CString s;
	mZoom.Create(s, CRect(width - cSmallIconBtnSize, 0, width, cSmallIconBtnSize), this, IDC_3PANETOOLBAR_ZOOMLIST, 0, IDI_3PANE_ZOOM);
	AddAlignment(new CWndAlignment(&mZoom, CWndAlignment::eAlign_TopRight));
	mZoom.SetSmallIcon(true);
	mZoom.SetShowIcon(true);
	mZoom.SetShowCaption(false);
	
	// Set tabs
	TC_ITEM tabs;
	tabs.mask = TCIF_TEXT;
	s.LoadString(IDS_3PANEACCOUNTS_TAB_MAIL);
	tabs.pszText = const_cast<TCHAR*>((const TCHAR*) s);
	mTabs.InsertItem(0, &tabs);
	s.LoadString(IDS_3PANEACCOUNTS_TAB_CONTACTS);
	tabs.pszText = const_cast<TCHAR*>((const TCHAR*) s);
	mTabs.InsertItem(1, &tabs);
	// Remove calendar if disabled by admin
	if (!CAdminLock::sAdminLock.mPreventCalendars)
	{
		s.LoadString(IDS_3PANEACCOUNTS_TAB_CALENDAR);
		tabs.pszText = const_cast<TCHAR*>((const TCHAR*) s);
		mTabs.InsertItem(2, &tabs);
	}
	mTabs.SetFont(CMulberryApp::sAppFont);

	// Adjust view to account for tabs
	CRect rect;
	GetClientRect(rect);
	rect.top += cTabsHeight;
	mView.MoveWindow(rect);
	//mView.SetFocusBorder();

	// Always need these
	MakeMail();
	MakeContacts();
	MakeCalendar();

	return 0;
}

void C3PaneAccounts::MakeMail()
{
	// Read the mail account view resource
	mMailAccounts = new C3PaneMailAccount;
	mMailAccounts->Create(_T(""), WS_CHILD, CRect(0, 0, 128, 128), &mView, IDC_STATIC);
	::ExpandChildToFit(&mView, mMailAccounts, true, true);
	mView.AddAlignment(new CWndAlignment(mMailAccounts, CWndAlignment::eAlign_WidthHeight));
	mMailAccounts->ShowWindow(SW_HIDE);
	
	// Create a toolbar for it
	MakeToolbars(GetServerView());
	
	// Listen to it to get focus info
	mMailAccounts->GetServerView()->Add_Listener(this);
}

void C3PaneAccounts::MakeContacts()
{
	// Read the contacts account view resource
	mContactsAccounts = new C3PaneContactsAccount;
	mContactsAccounts->Create(_T(""), WS_CHILD, CRect(0, 0, 128, 128), &mView, IDC_STATIC);
	::ExpandChildToFit(&mView, mContactsAccounts, true, true);
	mView.AddAlignment(new CWndAlignment(mContactsAccounts, CWndAlignment::eAlign_WidthHeight));
	mContactsAccounts->ShowWindow(SW_HIDE);
	
	// Create a toolbar for it
	MakeToolbars(GetContactsView());
	
	// Listen to it to get focus info
	mContactsAccounts->GetAdbkManagerView()->Add_Listener(this);
}

void C3PaneAccounts::MakeCalendar()
{
	// Don't do if admin locks it out
	if (CAdminLock::sAdminLock.mPreventCalendars)
		return;

	// Read the contacts account view resource
	mCalendarAccounts = new C3PaneCalendarAccount;
	mCalendarAccounts->Create(_T(""), WS_CHILD, CRect(0, 0, 128, 128), &mView, IDC_STATIC);
	::ExpandChildToFit(&mView, mCalendarAccounts, true, true);
	mView.AddAlignment(new CWndAlignment(mCalendarAccounts, CWndAlignment::eAlign_WidthHeight));
	mCalendarAccounts->ShowWindow(SW_HIDE);
	
	// Create a toolbar for it
	MakeToolbars(GetCalendarStoreView());
	
	// Listen to it to get focus info
	mCalendarAccounts->GetCalendarStoreView()->Add_Listener(this);
}

void C3PaneAccounts::OnSelChangeTabs(NMHDR* pNMHDR, LRESULT* pResult)
{
	N3Pane::EViewType view;
	switch(mTabs.GetCurSel())
	{
	case 0:
	default:
		view = N3Pane::eView_Mailbox;
		break;
	case 1:
		view = N3Pane::eView_Contacts;
		break;
	case 2:
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

void C3PaneAccounts::OnZoomPane()
{
	m3PaneWindow->SendMessage(WM_COMMAND, IDC_3PANETOOLBAR_ZOOMLIST);
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
		mCurrent->ShowWindow(SW_HIDE);

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
		tab_index = 0;
		break;

	case N3Pane::eView_Contacts:
		// Show the contacts account
		if (!mContactsAccounts)
			MakeContacts();
		mCurrent = mContactsAccounts;
		tab_index = 1;
		break;

	case N3Pane::eView_Calendar:
		// Show the calendar account
		if (!mCalendarAccounts)
			MakeCalendar();
		mCurrent = mCalendarAccounts;
		tab_index = 2;
		break;

	case N3Pane::eView_Empty:
	//case N3Pane::eView_IM:
	//case N3Pane::eView_Bookmarks:
		// Delete the content of all views
		mCurrent = NULL;
		break;
	}

	// Make sure tabs are in sync
	if (mTabs.GetCurSel() != tab_index)
		mTabs.SetCurSel(tab_index);

	// Show the new one and always focus on it
	if (mCurrent)
	{
		mCurrent->ShowWindow(SW_SHOW);
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
