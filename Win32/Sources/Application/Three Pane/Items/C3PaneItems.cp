/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// Source for C3PaneItems class

#include "C3PaneItems.h"

#include "CAddressBookView.h"
#include "CAdminLock.h"
#include "CCalendarView.h"
#include "CMailboxInfoView.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CUnicodeUtils.h"
#include "C3PaneAddressBook.h"
#include "C3PaneCalendar.h"
#include "C3PaneMailbox.h"
#include "C3PaneWindow.h"

#include "cdustring.h"

BEGIN_MESSAGE_MAP(C3PaneItems, C3PaneParentPanel)
	ON_WM_CREATE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_3PANEITEMS_TABS, OnSelChangeTabs)
	ON_COMMAND(IDC_3PANEITEMS_TABS, OnShiftClickTab)
	ON_COMMAND_RANGE(IDM_3PANEITEMSTAB_MOVELEFT, IDM_3PANEITEMSTAB_RESTORE, OnTabContext)
	ON_COMMAND(IDC_3PANETOOLBAR_ZOOMITEMS, OnZoomPane)
	ON_COMMAND(IDC_3PANEITEMS_CLOSEONE, OnCloseTab)
END_MESSAGE_MAP()

// Static

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneItems::C3PaneItems()
{
	mShowTabs = true;
	mMailbox = NULL;
	mAddressBook = NULL;
	mCalendar = NULL;
}

// Default destructor
C3PaneItems::~C3PaneItems()
{
	// Turn previews off as we delete
	if (GetMailboxView())
		GetMailboxView()->SetUsePreview(false);
	if (GetAddressBookView())
		GetAddressBookView()->SetUsePreview(false);
	if (GetCalendarView())
		GetCalendarView()->SetUsePreview(false);
	
	delete mMailbox;
	delete mAddressBook;
	delete mCalendar;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int C3PaneItems::OnCreate(LPCREATESTRUCT lpCreateStruct)
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
	mTabs.Create(WS_CHILD | WS_VISIBLE | TCS_FOCUSNEVER, CRect(3, 0, width - cSmallIconBtnSize - 8, cTabsHeight), this, IDC_3PANEITEMS_TABS);
	mTabs.SetFont(CMulberryApp::sAppFont);
	mTabs.SetOwner(this);
	AddAlignment(new CWndAlignment(&mTabs, CWndAlignment::eAlign_TopWidth));

	cdstring temp;
	temp.FromResource("UI::3Pane::NoMailboxesOpen");
	CString _temp = temp.win_str();
	TC_ITEM tabs;
	tabs.mask = TCIF_TEXT | TCIF_IMAGE;
	tabs.pszText = const_cast<TCHAR*>((const TCHAR*)_temp);
	tabs.iImage = -1;
	mTabs.InsertItem(0, &tabs);
	mTabDummy = true;

	// Close/zoom buttons at top next to tabs
	CString s;

	mZoom.Create(s, CRect(width - cSmallIconBtnSize, 0, width, cSmallIconBtnSize), this, IDC_3PANETOOLBAR_ZOOMITEMS, 0, IDI_3PANE_ZOOM);
	AddAlignment(new CWndAlignment(&mZoom, CWndAlignment::eAlign_TopRight));
	mZoom.SetSmallIcon(true);
	mZoom.SetShowIcon(true);
	mZoom.SetShowCaption(false);
	
	mCloseOne.Create(s, CRect(width - cSmallIconBtnSize, cTabsHeight, width, cTabsHeight + cSmallIconBtnSize), this, IDC_3PANEITEMS_CLOSEONE, 0, IDI_3PANE_CLOSE);
	AddAlignment(new CWndAlignment(&mCloseOne, CWndAlignment::eAlign_TopRight));
	mCloseOne.SetSmallIcon(true);
	mCloseOne.SetShowIcon(true);
	mCloseOne.SetShowCaption(false);
	
	// Header with alignment
	const int cCaptionHeight = 16;
	const int cHeaderHeight = cCaptionHeight + 8;
	const int cBtnVertOffset = (cHeaderHeight - cSmallIconBtnSize) / 2;
	mHeader.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(3, cTabsHeight, width - cSmallIconBtnSize - 8, cTabsHeight + cHeaderHeight), this, IDC_STATIC);
	mHeader.ModifyStyleEx(0, WS_EX_DLGMODALFRAME);
	AddAlignment(new CWndAlignment(&mHeader, CWndAlignment::eAlign_TopWidth));

	// Icon and descriptor
	mIcon.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 16, 16), &mHeader, IDC_STATIC);
	CRect rect;
	mHeader.GetClientRect(rect);
	rect.left += 24;
	mDescriptor.Create(NULL, WS_CHILD | WS_VISIBLE, rect, &mHeader, IDC_STATIC);
	mHeader.AddAlignment(new CWndAlignment(&mDescriptor, CWndAlignment::eAlign_WidthHeight));

	// Adjust view to account for header
	GetClientRect(rect);
	rect.top += cTabsHeight + cHeaderHeight;
	mView.MoveWindow(rect);
	//mView.SetFocusBorder();

	// Force tabs to be initially hidden
	ShowTabs(false);

	// Always need these
	MakeMailbox();
	MakeAddressBook();
	MakeCalendar();

	return 0;
}

void C3PaneItems::ListenTo_Message(long msg, void* param)
{
	// Look for messages
	switch(msg)
	{
	// Change in mailbox view contents
	case CBaseView::eBroadcast_ViewChanged:
		{
			C3PanePanel* broadcaster = NULL;

			if (reinterpret_cast<CBaseView*>(param) == GetMailboxView())
				broadcaster = mMailbox;
			else if (reinterpret_cast<CBaseView*>(param) == GetAddressBookView())
				broadcaster = mAddressBook;
			else if (reinterpret_cast<CBaseView*>(param) == GetCalendarView())
				broadcaster = mCalendar;

			// Only if its one of ours
			if (broadcaster)
			{
				// Change title if view is displayed
				if (mCurrent == broadcaster)
				{
					SetIcon(broadcaster->GetIconID());
					SetTitle(broadcaster->GetTitle());
				}
				
				// Check active status
				broadcaster->EnableWindow(broadcaster->IsSpecified());
				FRAMEWORK_REFRESH_WINDOW(broadcaster);

				// Make sure we reset zoom state on deactivate
				if (!broadcaster->IsSpecified() && C3PaneWindow::s3PaneWindow->GetZoomItems())
					C3PaneWindow::s3PaneWindow->ZoomItems(false);
			}
		}
		break;

	// Mailbox added a substitute item
	case CMailboxView::eBroadcast_AddSubstitute:
		AddSubstitute(reinterpret_cast<CMailboxView::SBroadcast_AddSubstitute*>(param)->mName,
						reinterpret_cast<CMailboxView::SBroadcast_AddSubstitute*>(param)->mUnseenIcon);
		break;
	
	// Mailbox selected a substitute item
	case CMailboxView::eBroadcast_SelectSubstitute:
		SelectSubstitute(*reinterpret_cast<unsigned long*>(param));
		
		// Determine if locked
		mCloseOne.EnableWindow(!IsSubstituteLocked(*reinterpret_cast<unsigned long*>(param)));
		break;
	
	// Change a substitute item
	case CMailboxView::eBroadcast_ChangeSubstitute:
		ChangeSubstitute(reinterpret_cast<CMailboxView::SBroadcast_ChangeSubstitute*>(param)->mIndex,
							reinterpret_cast<CMailboxView::SBroadcast_ChangeSubstitute*>(param)->mName,
							reinterpret_cast<CMailboxView::SBroadcast_ChangeSubstitute*>(param)->mUnseenIcon);
		break;
	
	// Change an icon in a substitute item
	case CMailboxView::eBroadcast_ChangeIconSubstitute:
		ChangeIconSubstitute(reinterpret_cast<CMailboxView::SBroadcast_ChangeIconSubstitute*>(param)->mIndex,
							reinterpret_cast<CMailboxView::SBroadcast_ChangeIconSubstitute*>(param)->mUnseenIcon);
		break;
	
	// Move a substitute item
	case CMailboxView::eBroadcast_MoveSubstitute:
		MoveSubstitute(reinterpret_cast<CMailboxView::SBroadcast_MoveSubstitute*>(param)->mOldIndex,
						reinterpret_cast<CMailboxView::SBroadcast_MoveSubstitute*>(param)->mNewIndex);
		break;
	
	// Mailbox removed a substitute item
	case CMailboxView::eBroadcast_RemoveSubstitute:
		RemoveSubstitute(*reinterpret_cast<unsigned long*>(param));
		break;
	
	default:
		C3PaneParentPanel::ListenTo_Message(msg, param);
		break;
	}
}

void C3PaneItems::MakeMailbox()
{
	// Read the mailbox view resource
	mMailbox = new C3PaneMailbox;
	mMailbox->Create(_T(""), WS_CHILD, CRect(0, 0, 128, 128), &mView, IDC_STATIC);
	::ExpandChildToFit(&mView, mMailbox, true, true);
	mView.AddAlignment(new CWndAlignment(mMailbox, CWndAlignment::eAlign_WidthHeight));
	mMailbox->ShowWindow(SW_HIDE);
	
	// Setup listeners
	mMailbox->GetMailboxView()->Add_Listener(this);
	
	// Create a toolbar for it
	MakeToolbars(mMailbox->GetMailboxView());
}

void C3PaneItems::MakeAddressBook()
{
	// Read the mailbox view resource
	mAddressBook = new C3PaneAddressBook;
	mAddressBook->Create(_T(""), WS_CHILD, CRect(0, 0, 128, 128), &mView, IDC_STATIC);
	::ExpandChildToFit(&mView, mAddressBook, true, true);
	mView.AddAlignment(new CWndAlignment(mAddressBook, CWndAlignment::eAlign_WidthHeight));
	mAddressBook->ShowWindow(SW_HIDE);
	
	mAddressBook->GetAddressBookView()->Add_Listener(this);
	
	// Create a toolbar for it
	MakeToolbars(mAddressBook->GetAddressBookView());
}

void C3PaneItems::MakeCalendar()
{
	// Don't do if admin locks it out
	if (CAdminLock::sAdminLock.mPreventCalendars)
		return;

	// Read the mailbox view resource
	mCalendar = new C3PaneCalendar;
	mCalendar->Create(_T(""), WS_CHILD, CRect(0, 0, 128, 128), &mView, IDC_STATIC);
	::ExpandChildToFit(&mView, mCalendar, true, true);
	mView.AddAlignment(new CWndAlignment(mCalendar, CWndAlignment::eAlign_WidthHeight));
	mCalendar->ShowWindow(SW_HIDE);
	
	mCalendar->GetCalendarView()->Add_Listener(this);
	
	// Create a toolbar for it
	MakeToolbars(mCalendar->GetCalendarView());
}

void C3PaneItems::OnSelChangeTabs(NMHDR* pNMHDR, LRESULT* pResult)
{
	// Only do for maiklboxes right now
	if (mMailbox && (mMailbox == mCurrent) && GetMailboxView() && mTabs.IsActive())
	{
		GetMailboxView()->SetSubstitute(mTabs.GetCurSel());
		GetMailboxView()->Focus();
	}
}

void C3PaneItems::OnShiftClickTab()
{
	// Only do for mailboxes right now
	OnCloseOne(mTabs.GetLastClicked());
}

void C3PaneItems::OnTabsMove(std::pair<unsigned long, unsigned long> indices)
{
	// Only do for mailboxes right now
	if (mMailbox && (mMailbox == mCurrent) && GetMailboxView())
		GetMailboxView()->MoveSubstitute(indices.first, indices.second);
}

void C3PaneItems::OnTabsRename(unsigned long index)
{
	// Only do for mailboxes right now
	if (mMailbox && (mMailbox == mCurrent) && GetMailboxView())
		GetMailboxView()->RenameSubstitute(index);
}

void C3PaneItems::OnTabsLock(unsigned long index)
{
	// Only do for mailboxes right now
	if (mMailbox && (mMailbox == mCurrent) && GetMailboxView())
		GetMailboxView()->LockSubstitute(index);
	
	// Now check whether this is the active one and change state
	if (index == mTabs.GetCurSel())
		mCloseOne.EnableWindow(!IsSubstituteLocked(index));
}

void C3PaneItems::OnTabsLockAll()
{
	// Only do for mailboxes right now
	if (mMailbox && (mMailbox == mCurrent) && GetMailboxView())
		GetMailboxView()->LockAllSubstitutes();
	
	// Always disable close button
	mCloseOne.EnableWindow(false);
}

void C3PaneItems::OnTabsDynamic(unsigned long index)
{
	// Only do for mailboxes right now
	if (mMailbox && (mMailbox == mCurrent) && GetMailboxView())
		GetMailboxView()->DynamicSubstitute(index);
}

void C3PaneItems::OnTabsSave()
{
	// Only do for mailboxes right now
	if (mMailbox && (mMailbox == mCurrent) && GetMailboxView())
		GetMailboxView()->SaveSubstitutes();
}

void C3PaneItems::OnTabsRestore()
{
	// Only do for mailboxes right now
	if (mMailbox && (mMailbox == mCurrent) && GetMailboxView())
		GetMailboxView()->RestoreSubstitutes();
}

void C3PaneItems::OnTabContext(UINT item)
{
	switch(item)
	{
	case IDM_3PANEITEMSTAB_MOVELEFT:
		{
			std::pair<unsigned long, unsigned long> p(mTabs.GetLastClicked(), mTabs.GetLastClicked() - 1);
			OnTabsMove(p);
		}
		break;
	case IDM_3PANEITEMSTAB_MOVERIGHT:
		{
			std::pair<unsigned long, unsigned long> p(mTabs.GetLastClicked(), mTabs.GetLastClicked() + 1);
			OnTabsMove(p);
		}
		break;
	case IDM_3PANEITEMSTAB_MOVESTART:
		{
			std::pair<unsigned long, unsigned long> p(mTabs.GetLastClicked(), 0);
			OnTabsMove(p);
		}
		break;
	case IDM_3PANEITEMSTAB_MOVEEND:
		{
			std::pair<unsigned long, unsigned long> p(mTabs.GetLastClicked(), mTabs.GetItemCount() - 1);
			OnTabsMove(p);
		}
		break;
	case IDM_3PANEITEMSTAB_CLOSE:
		OnCloseOne(mTabs.GetLastClicked());
		break;
	case IDM_3PANEITEMSTAB_CLOSEALL:
		OnCloseAll();
		break;
	case IDM_3PANEITEMSTAB_CLOSEOTHERS:
		OnCloseOthers(mTabs.GetLastClicked());
		break;
	case IDM_3PANEITEMSTAB_RENAME:
		OnTabsRename(mTabs.GetLastClicked());
		break;
	case IDM_3PANEITEMSTAB_LOCK:
		OnTabsLock(mTabs.GetLastClicked());
		break;
	case IDM_3PANEITEMSTAB_LOCKALL:
		OnTabsLockAll();
		break;
	case IDM_3PANEITEMSTAB_DYNAMIC:
		OnTabsDynamic(mTabs.GetLastClicked());
		break;
	case IDM_3PANEITEMSTAB_SAVE:
		OnTabsSave();
		break;
	case IDM_3PANEITEMSTAB_RESTORE:
		OnTabsRestore();
		break;
	}
}

void C3PaneItems::OnZoomPane()
{
	m3PaneWindow->SendMessage(WM_COMMAND, IDC_3PANETOOLBAR_ZOOMITEMS);
}

void C3PaneItems::OnCloseTab()
{
	OnCloseOne(mTabs.GetCurSel());
}

void C3PaneItems::OnCloseOne(unsigned long index)
{
	if (TestCloseOne(index))
		DoCloseOne(index);
}

void C3PaneItems::OnCloseAll()
{
	if (TestCloseAll())
		DoCloseAll();
}

void C3PaneItems::OnCloseOthers(unsigned long index)
{
	if (TestCloseOthers(index))
		DoCloseOthers(index);
}

void C3PaneItems::DoneInitMailAccounts()
{
	// Initialise it
	if (mMailbox)
	{
		mMailbox->Init();
		UpdateTabs();
	}
}

void C3PaneItems::DoneInitAdbkAccounts()
{
	// Initialise it
	if (mAddressBook)
	{
		mAddressBook->Init();
		UpdateTabs();
	}
}

void C3PaneItems::DoneInitCalendarAccounts()
{
	// Initialise it
	if (mCalendar)
	{
		mCalendar->Init();
		UpdateTabs();
	}
}

void C3PaneItems::GetOpenItems(cdstrvect& items) const
{
	// Only do mailbox for now
	if (mMailbox)
		mMailbox->GetOpenItems(items);
}

void C3PaneItems::CloseOpenItems()
{
	if (mMailbox)
		mMailbox->CloseOpenItems();
	if (mAddressBook)
		mAddressBook->CloseOpenItems();
	if (mCalendar)
		mCalendar->CloseOpenItems();
}

void C3PaneItems::SetOpenItems(const cdstrvect& items)
{
	// Only do mailbox for now
	if (mMailbox)
		mMailbox->SetOpenItems(items);
}

bool C3PaneItems::IsSpecified() const
{
	// Check that sub view is specified
	return mCurrent->IsSpecified();
}

// Set the title for the items
void C3PaneItems::SetTitle(const cdstring& title)
{
	CUnicodeUtils::SetWindowTextUTF8(&mDescriptor, title);
}

// Set the icon for the preview
void C3PaneItems::SetIcon(UINT icon)
{
	mIcon.SetIconID(icon);
}

// Tell panes to use preview or not
void C3PaneItems::UsePreview(bool preview)
{
	// Test each pane that exists
	if (GetMailboxView())
		GetMailboxView()->SetUsePreview(preview);
	if (GetAddressBookView())
		GetAddressBookView()->SetUsePreview(preview);
	if (GetCalendarView())
		GetCalendarView()->SetUsePreview(preview);
}

// Show the tab switcher
void C3PaneItems::ShowTabs(bool show)
{
	if (mShowTabs ^ show)
	{
		mShowTabs = show;

		CRect move_size;
		mTabs.GetWindowRect(move_size);
		int moveby = move_size.Height();

		if (mShowTabs)
		{
			::ResizeWindowBy(&mView, 0, -moveby, false);
			::MoveWindowBy(&mView, 0, moveby, false);
			::MoveWindowBy(&mHeader, 0, moveby, false);
			mTabs.ShowWindow(SW_SHOW);
			mCloseOne.ShowWindow(SW_SHOW);
		}
		else
		{
			mTabs.ShowWindow(SW_HIDE);
			mCloseOne.ShowWindow(SW_HIDE);
			::MoveWindowBy(&mHeader, 0, -moveby, false);
			::MoveWindowBy(&mView, 0, -moveby, false);
			::ResizeWindowBy(&mView, 0, moveby, false);
		}
		
		RedrawWindow();
	}
}

void C3PaneItems::UpdateTabs()
{
	// Policy:
	//
	// 1) No tabs for address books
	// 2) Tabs for mailbox only if in use or if there currently more than one
	//    Later condition takes care of switching off tabs whilst some are still assigned - 
	//    i.e. tabs remain until all but the last one are removed
	
	if (mCurrent == mMailbox)
	{
		// On if tabs are enabled and at least one present, or more than one present
		ShowTabs(mMailbox->GetUseSubstitute() || (mTabs.GetItemCount() > 1));
		
		// Disable tabs
		mTabs.SetActive(!mTabDummy);
	}
	else if (mCurrent == mAddressBook)
		// Always off for address books
		ShowTabs(false);
	else if (mCurrent == mCalendar)
		// Always off for calendars
		ShowTabs(false);
}

void C3PaneItems::AddSubstitute(const char* name, bool icon)
{
	// Just rename if we have a dummy
	CString _temp = cdstring(name).win_str();
	if (mTabDummy)
	{
		TC_ITEM tabs;
		tabs.mask = TCIF_TEXT | TCIF_IMAGE;
		tabs.pszText = const_cast<TCHAR*>((const TCHAR*)_temp);
		tabs.iImage = icon ? 0 : -1;
		mTabs.SetItem(0, &tabs);
		mTabDummy = false;
	}
	else
	{
		// Add an item to the end of the tabs
		TC_ITEM tabs;
		tabs.mask = TCIF_TEXT | TCIF_IMAGE;
		tabs.pszText = const_cast<TCHAR*>((const TCHAR*)_temp);
		tabs.iImage = icon ? 0 : -1;
		mTabs.InsertItem(mTabs.GetItemCount(), &tabs);
	}
	
	// Update tab state after change
	UpdateTabs();
}

void C3PaneItems::SelectSubstitute(unsigned long index)
{
	// Select it but don't do broadcast
	mTabs.SetCurSel(index);
}

void C3PaneItems::ChangeSubstitute(unsigned long index, const char* name, bool icon)
{
	// Change name and icon of tab item
	CString _temp = cdstring(name).win_str();
	TC_ITEM tabs;
	tabs.mask = TCIF_TEXT | TCIF_IMAGE;
	tabs.pszText = const_cast<TCHAR*>((const TCHAR*)_temp);
	tabs.iImage = icon ? 0 : -1;
	mTabs.SetItem(index, &tabs);
}

void C3PaneItems::ChangeIconSubstitute(unsigned long index, bool icon)
{
	// Change icon of tab item
	TC_ITEM tabs;
	tabs.mask = TCIF_IMAGE;
	tabs.iImage = icon ? 0 : -1;
	mTabs.SetItem(index, &tabs);
}

void C3PaneItems::MoveSubstitute(unsigned long oldindex, unsigned long newindex)
{
	// Move tab item
	mTabs.MoveTab(oldindex, newindex);
}

void C3PaneItems::RemoveSubstitute(unsigned long index)
{
	// If none would be left use a dummy name
	if (mTabs.GetItemCount() > 1)
		// Remove it
		mTabs.DeleteItem(index);
	else
	{
		cdstring temp;
		temp.FromResource("UI::3Pane::NoMailboxesOpen");
		CString _temp(temp);
		TC_ITEM tabs;
		tabs.mask = TCIF_TEXT | TCIF_IMAGE;
		tabs.pszText = const_cast<TCHAR*>((const TCHAR*)_temp);
		tabs.iImage = -1;
		mTabs.SetItem(0, &tabs);
		mTabDummy = true;
		mCloseOne.EnableWindow(false);
	}
	
	// Update tab state after change
	UpdateTabs();
}

bool C3PaneItems::TestClose()
{
	// Test each pane that exists
	if (mMailbox && !mMailbox->TestClose())
		return false;
	if (mAddressBook && !mAddressBook->TestClose())
		return false;
	if (mCalendar && !mCalendar->TestClose())
		return false;
	
	return true;
}

void C3PaneItems::DoClose()
{
	// Close each pane that exists
	if (mMailbox)
		mMailbox->DoClose();
	if (mAddressBook)
		mAddressBook->DoClose();
	if (mCalendar)
		mCalendar->DoClose();
}

bool C3PaneItems::TestCloseAll()
{
	// Test close the active pane's contents
	if (mMailbox && (mMailbox == mCurrent) && !mMailbox->TestCloseAll())
		return false;
	else if (mAddressBook && (mAddressBook == mCurrent) && !mAddressBook->TestCloseAll())
		return false;
	else if (mCalendar && (mCalendar == mCurrent) && !mCalendar->TestCloseAll())
		return false;
	
	return true;
}

void C3PaneItems::DoCloseAll()
{
	// Close the active pane's contents
	if (mMailbox && (mMailbox == mCurrent))
		mMailbox->DoCloseAll();
	else if (mAddressBook && (mAddressBook == mCurrent))
		mAddressBook->DoCloseAll();
	else if (mCalendar && (mCalendar == mCurrent))
		mCalendar->DoCloseAll();
}

bool C3PaneItems::TestCloseOne(unsigned long index)
{
	// Test close the active pane's contents
	if (mMailbox && (mMailbox == mCurrent) && !mMailbox->TestCloseOne(index))
		return false;
	else if (mAddressBook && (mAddressBook == mCurrent) && !mAddressBook->TestCloseOne(index))
		return false;
	else if (mCalendar && (mCalendar == mCurrent) && !mCalendar->TestCloseOne(index))
		return false;
	
	return true;
}

void C3PaneItems::DoCloseOne(unsigned long index)
{
	// Close the active pane's contents
	if (mMailbox && (mMailbox == mCurrent))
		mMailbox->DoCloseOne(index);
	else if (mAddressBook && (mAddressBook == mCurrent))
		mAddressBook->DoCloseOne(index);
	else if (mCalendar && (mCalendar == mCurrent))
		mCalendar->DoCloseOne(index);
}

bool C3PaneItems::TestCloseOthers(unsigned long index)
{
	// Test close the active pane's contents
	if (mMailbox && (mMailbox == mCurrent) && !mMailbox->TestCloseOthers(index))
		return false;
	else if (mAddressBook && (mAddressBook == mCurrent) && !mAddressBook->TestCloseOthers(index))
		return false;
	else if (mCalendar && (mCalendar == mCurrent) && !mCalendar->TestCloseOthers(index))
		return false;
	
	return true;
}

void C3PaneItems::DoCloseOthers(unsigned long index)
{
	// Close the active pane's contents
	if (mMailbox && (mMailbox == mCurrent))
		mMailbox->DoCloseOthers(index);
	else if (mAddressBook && (mAddressBook == mCurrent))
		mAddressBook->DoCloseOthers(index);
	else if (mCalendar && (mCalendar == mCurrent))
		mCalendar->DoCloseOthers(index);
}

void C3PaneItems::SetUseSubstitute(bool mail_subs)
{
	// Set each pane
	if (mMailbox)
		mMailbox->SetUseSubstitute(mail_subs);
	//else if (mAddressBook)
	//	mAddressBook->SetUseSubstitute(adbk_subs);
	//else if (mCalendar)
	//	mCalendar->SetUseSubstitute(adbk_subs);
	
	// Update tab state
	UpdateTabs();
}

bool C3PaneItems::IsSubstituteLocked(unsigned long index)
{
	// Test close the active pane's contents
	if (mMailbox && (mMailbox == mCurrent))
		return mMailbox->IsSubstituteLocked(index);
	else if (mAddressBook && (mAddressBook == mCurrent))
		return mAddressBook->IsSubstituteLocked(index);
	else if (mCalendar && (mCalendar == mCurrent))
		return mCalendar->IsSubstituteLocked(index);
	
	return false;
}

bool C3PaneItems::IsSubstituteDynamic(unsigned long index)
{
	// Test close the active pane's contents
	if (mMailbox && (mMailbox == mCurrent))
		return mMailbox->IsSubstituteDynamic(index);
	else if (mAddressBook && (mAddressBook == mCurrent))
		return mAddressBook->IsSubstituteDynamic(index);
	else if (mCalendar && (mCalendar == mCurrent))
		return mCalendar->IsSubstituteDynamic(index);
	
	return false;
}

void C3PaneItems::SetViewType(N3Pane::EViewType view)
{
	// Only bother if different
	if (mViewType == view)
		return;

	// Hide current view
	if (mCurrent)
		mCurrent->ShowWindow(SW_HIDE);

	// Set the pane contents
	mViewType = view;
	switch(mViewType)
	{
	case N3Pane::eView_Mailbox:
		// Show the mailbox
		if (!mMailbox)
			MakeMailbox();
		mCurrent = mMailbox;
		SetTitle(mMailbox->GetTitle());
	break;

	case N3Pane::eView_Contacts:
		// Show the address book
		if (!mAddressBook)
			MakeAddressBook();
		mCurrent = mAddressBook;
		SetTitle(mAddressBook->GetTitle());
		break;

	case N3Pane::eView_Calendar:
		// Show the calendar
		if (!mCalendar)
			MakeCalendar();
		mCurrent = mCalendar;
		SetTitle(mCalendar->GetTitle());
		break;

	case N3Pane::eView_Empty:
	//case N3Pane::eView_IM:
	//case N3Pane::eView_Bookmarks:
		// Delete the content of all views
		mCurrent = NULL;
		SetTitle(cdstring::null_str);
		break;
	}
		
	// Update tab state after change - do this after hiding the previous panel, but
	// before showing the next one
	UpdateTabs();

	// Show the new one
	if (mCurrent)
	{
		mCurrent->ShowWindow(SW_SHOW);
		mCurrent->EnableWindow(mCurrent->IsSpecified());

		// Change title and icon when view is changed
		SetIcon(mCurrent->GetIconID());
		SetTitle(mCurrent->GetTitle());
	}
}

// Get server view from sub-pane
CMailboxInfoView* C3PaneItems::GetMailboxView() const
{
	return mMailbox ? mMailbox->GetMailboxView() : NULL;
}

// Get server view from sub-pane
CAddressBookView* C3PaneItems::GetAddressBookView() const
{
	return mAddressBook ? mAddressBook->GetAddressBookView() : NULL;
}

// Get server view from sub-pane
CCalendarView* C3PaneItems::GetCalendarView() const
{
	return mCalendar ? mCalendar->GetCalendarView() : NULL;
}

// Reset state from prefs
void C3PaneItems::ResetState()
{
	switch(mViewType)
	{
	case N3Pane::eView_Mailbox:
		if (GetMailboxView())
			GetMailboxView()->ResetState(true);
		break;

	case N3Pane::eView_Contacts:
		if (GetAddressBookView())
			GetAddressBookView()->ResetState(true);
		break;

	case N3Pane::eView_Calendar:
		if (GetCalendarView())
			GetCalendarView()->ResetState(true);
		break;

	default:;
	}
}

// Save state in prefs
void C3PaneItems::SaveDefaultState()
{
	switch(mViewType)
	{
	case N3Pane::eView_Mailbox:
		if (GetMailboxView())
			GetMailboxView()->SaveDefaultState();
		break;

	case N3Pane::eView_Contacts:
		if (GetAddressBookView())
			GetAddressBookView()->SaveDefaultState();
		break;

	case N3Pane::eView_Calendar:
		if (GetCalendarView())
			GetCalendarView()->SaveDefaultState();
		break;

	default:;
	}
}
