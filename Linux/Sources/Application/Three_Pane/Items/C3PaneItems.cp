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


// Source for C3PaneItems class

#include "C3PaneItems.h"

#include "CAddressBookView.h"
#include "CAdminLock.h"
#include "CCalendarView.h"
#include "CCommands.h"
#include "CIconLoader.h"
#include "CMailboxInfoView.h"
#include "CStaticText.h"
#include "CToolbarButton.h"
#include "C3PaneAddressBook.h"
#include "C3PaneItemsTabs.h"
#include "C3PaneCalendar.h"
#include "C3PaneMailbox.h"
#include "C3PaneWindow.h"

#include <JXColormap.h>
#include <JXImageWidget.h>
#include <JXUpRect.h>

#include <cassert>

// Static

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneItems::C3PaneItems(JXContainer* enclosure,
						 const HSizingOption hSizing, 
						 const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h) 
	: C3PaneParentPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
	mTabDummy = true;
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
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void C3PaneItems::OnCreate()
{
	// Do inherited
	C3PaneParentPanel::OnCreate();

// begin JXLayout1

    mTabs =
        new C3PaneItemsTabs(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,0, 275,35);
    assert( mTabs != NULL );

    mZoomBtn =
        new CToolbarButton("",this,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 280,7, 20,20);
    assert( mZoomBtn != NULL );

    mHeader =
        new JXWidgetSet(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,35, 300,165);
    assert( mHeader != NULL );

    JXUpRect* obj1 =
        new JXUpRect(mHeader,
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

    mCloseTabBtn =
        new CToolbarButton("",mHeader,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 280,2, 20,20);
    assert( mCloseTabBtn != NULL );

    mViewPane =
        new JXWidgetSet(mHeader,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,24, 300,141);
    assert( mViewPane != NULL );

// end JXLayout1

	mView = mViewPane;
	mTabs->SetOwner(this);

	mZoomBtn->SetImage(IDI_3PANE_ZOOM, 0);
	mZoomBtn->SetSmallIcon(true);
	mZoomBtn->SetShowIcon(true);
	mZoomBtn->SetShowCaption(false);
	
	mCloseTabBtn->SetImage(IDI_3PANE_CLOSE, 0);
	mCloseTabBtn->SetSmallIcon(true);
	mCloseTabBtn->SetShowIcon(true);
	mCloseTabBtn->SetShowCaption(false);

	ListenTo(mTabs);
	ListenTo(mZoomBtn);
	ListenTo(mCloseTabBtn);
	
	// Force tabs to be initially hidden
	ShowTabs(false);
	cdstring temp;
	temp.FromResource("UI::3Pane::NoMailboxesOpen");
	mTabs->AppendCard(NULL, temp, 0);
	mTabDummy = true;

	// Always need these
	MakeMailbox();
	MakeAddressBook();
	MakeCalendar();
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

			if (reinterpret_cast<CMailboxInfoView*>(param) == GetMailboxView())
				broadcaster = mMailbox;
			else if (reinterpret_cast<CAddressBookView*>(param) == GetAddressBookView())
				broadcaster = mAddressBook;
			else if (reinterpret_cast<CBaseView*>(param) == GetCalendarView())
				broadcaster = mCalendar;

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
					if (C3PaneWindow::s3PaneWindow->GetZoomItems())
						C3PaneWindow::s3PaneWindow->ZoomItems(false);
				}
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
		if (IsSubstituteLocked(*reinterpret_cast<unsigned long*>(param)))
			mCloseTabBtn->Deactivate();
		else
			mCloseTabBtn->Activate();
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
	
	default:;
	}
}

// Respond to clicks in the icon buttons
void C3PaneItems::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXRadioGroup::kSelectionChanged) && (sender == mTabs))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		OnTabs(index);
		return;
	}
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mZoomBtn)
		{
			OnZoom();
			return;
		}
		else if (sender == mCloseTabBtn)
		{
			OnCloseTab();
			return;
		}
	}
}

void C3PaneItems::OnTabs(unsigned long index)
{
	// Only do for mailboxes right now
	if (mMailbox && (mMailbox == mCurrent) && GetMailboxView() && mTabs->HasActiveTabs())
	{
		GetMailboxView()->SetSubstitute(index - 1);
		GetMailboxView()->Focus();
	}
}

void C3PaneItems::OnTabsMove(std::pair<unsigned long, unsigned long> indices)
{
	// Only do for mailboxes right now
	if (mMailbox && (mMailbox == mCurrent) && GetMailboxView())
		GetMailboxView()->MoveSubstitute(indices.first - 1, indices.second - 1);
}

void C3PaneItems::OnTabsClose(unsigned long index)
{
	OnCloseOne(index - 1);
}

void C3PaneItems::OnTabsCloseAll()
{
	OnCloseAll();
}

void C3PaneItems::OnTabsCloseOthers(unsigned long index)
{
	OnCloseOthers(index - 1);
}

void C3PaneItems::OnTabsRename(unsigned long index)
{
	// Only do for mailboxes right now
	if (mMailbox && (mMailbox == mCurrent) && GetMailboxView())
		GetMailboxView()->RenameSubstitute(index - 1);
}

void C3PaneItems::OnTabsLock(unsigned long index)
{
	// Only do for mailboxes right now
	if (mMailbox && (mMailbox == mCurrent) && GetMailboxView())
		GetMailboxView()->LockSubstitute(index - 1);
	
	// Now check whether this is the active one and change state
	if (index == mTabs->GetSelectedItem())
	{
		// Determine if locked
		if (IsSubstituteLocked(index - 1))
			mCloseTabBtn->Deactivate();
		else
			mCloseTabBtn->Activate();
	}
}

void C3PaneItems::OnTabsLockAll()
{
	// Only do for mailboxes right now
	if (mMailbox && (mMailbox == mCurrent) && GetMailboxView())
		GetMailboxView()->LockAllSubstitutes();
	
	// Always disable close button
	mCloseTabBtn->Deactivate();
}

void C3PaneItems::OnTabsDynamic(unsigned long index)
{
	// Only do for mailboxes right now
	if (mMailbox && (mMailbox == mCurrent) && GetMailboxView())
		GetMailboxView()->DynamicSubstitute(index - 1);
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

void C3PaneItems::OnZoom()
{
	m3PaneWindow->ObeyCommand(CCommand::eToolbarZoomItems, NULL);
}

void C3PaneItems::OnCloseTab()
{
	OnCloseOne(mTabs->GetSelectedItem() - 1);
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

void C3PaneItems::MakeMailbox()
{
	// Read the mailbox view resource
	mMailbox = new C3PaneMailbox(mView, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 100, 100);
	mMailbox->OnCreate();
	mMailbox->FitToEnclosure(kTrue, kTrue);
	mMailbox->Hide();

	// Setup listeners
	GetMailboxView()->Add_Listener(this);
	
	// Create a toolbar for it
	MakeToolbars(GetMailboxView());
}

void C3PaneItems::MakeAddressBook()
{
	// Read the mailbox view resource
	mAddressBook = new C3PaneAddressBook(mView, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 100, 100);
	mAddressBook->OnCreate();
	mAddressBook->FitToEnclosure(kTrue, kTrue);
	mAddressBook->Hide();

	GetAddressBookView()->Add_Listener(this);
	
	// Create a toolbar for it
	MakeToolbars(GetAddressBookView());
}

void C3PaneItems::MakeCalendar()
{
	// Don't do if admin locks it out
	if (CAdminLock::sAdminLock.mPreventCalendars)
		return;

	// Read the mailbox view resource
	mCalendar = new C3PaneCalendar(mView, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 100, 100);
	mCalendar->OnCreate();
	mCalendar->FitToEnclosure(kTrue, kTrue);
	mCalendar->Hide();

	// Setup listeners
	GetCalendarView()->Add_Listener(this);
	
	// Create a toolbar for it
	MakeToolbars(GetCalendarView());
}

bool C3PaneItems::IsSpecified() const
{
	// Check that sub view is specified
	return mCurrent->IsSpecified();
}

// Set the title for the items
void C3PaneItems::SetTitle(const cdstring& title)
{
	mDescriptor->SetText(title);
}

// Set the icon for the preview
void C3PaneItems::SetIcon(unsigned int icon)
{
	mIcon->SetImage(CIconLoader::GetIcon(icon, mIcon, 16, 0x00CCCCCC), kFalse);
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

		JCoordinate moveby = mTabs->GetFrameHeight();

		if (mShowTabs)
		{
			mHeader->AdjustSize(0, -moveby);
			mHeader->Move(0, moveby);
			mTabs->Show();
			mCloseTabBtn->Show();
		}
		else
		{
			mTabs->Hide();
			mCloseTabBtn->Hide();
			mHeader->Move(0, -moveby);
			mHeader->AdjustSize(0, moveby);
		}
		
		Refresh();
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
		ShowTabs(mMailbox->GetUseSubstitute() || (mTabs->GetTabCount() > 1));
		
		// Disable tabs
		mTabs->SetActiveTabs(!mTabDummy);
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
	if (mTabDummy)
	{
		mTabs->RenameCard(1, name, icon ? IDI_TAB_UNSEEN : 0);
		mTabDummy = false;
	}
	else
		// Add an item to the end of the tabs
		mTabs->AppendCard(NULL, name, icon ? IDI_TAB_UNSEEN : 0);
	
	// Update tab state after change
	UpdateTabs();
}

void C3PaneItems::SelectSubstitute(unsigned long index)
{
	// Select it but don't do broadcast
	SetListening(kFalse);
	mTabs->SelectItem(index + 1);
	SetListening(kTrue);
}

void C3PaneItems::ChangeSubstitute(unsigned long index, const char* name, bool icon)
{
	// Change name of tab item
	mTabs->RenameCard(index + 1, name, icon ? IDI_TAB_UNSEEN : 0);
}

void C3PaneItems::ChangeIconSubstitute(unsigned long index, bool icon)
{
	// Add an item to the end of the tabs
	mTabs->SetIconCard(index + 1, icon ? IDI_TAB_UNSEEN : 0);
}

void C3PaneItems::MoveSubstitute(unsigned long oldindex, unsigned long newindex)
{
	// Move tab item
	SetListening(kFalse);
	mTabs->MoveCard(oldindex + 1, newindex + 1);
	SetListening(kTrue);
}

void C3PaneItems::RemoveSubstitute(unsigned long index)
{
	// If none would be left use a dummy name
	if (mTabs->GetTabCount() > 1)
		// Remove it
		mTabs->RemoveCard(index + 1);
	else
	{
		cdstring temp;
		temp.FromResource("UI::3Pane::NoMailboxesOpen");
		mTabs->RenameCard(1, temp, 0);
		mTabDummy = true;
		mCloseTabBtn->Deactivate();
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
		mCurrent->Hide();

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
	default:;
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
		mCurrent->Show();
		if (mCurrent->IsSpecified())
			mCurrent->Activate();
		else
			mCurrent->Deactivate();

		// Change title and icon when view is changed
		SetTitle(mCurrent->GetTitle());
		SetIcon(mCurrent->GetIconID());
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
