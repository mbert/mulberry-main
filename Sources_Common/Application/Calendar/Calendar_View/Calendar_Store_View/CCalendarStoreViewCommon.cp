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


// Source for CCalendarStoreView class

#include "CCalendarStoreView.h"

#include "CCalendarStoreTable.h"
#include "CCalendarStoreWindow.h"
#include "CCalendarView.h"
#include "CGetStringDialog.h"
#include "CPreferences.h"
#include "CXStringResources.h"

#include "CCalendarStoreManager.h"

#include <algorithm>

// Check for window
bool CCalendarStoreView::ViewExists(const CCalendarStoreView* wnd)
{
	cdmutexprotect<CCalendarStoreViewList>::lock _lock(sCalendarStoreViews);
	CCalendarStoreViewList::iterator found = std::find(sCalendarStoreViews->begin(), sCalendarStoreViews->end(), wnd);
	return found != sCalendarStoreViews->end();
}

void CCalendarStoreView::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	default:
		CTableView::ListenTo_Message(msg, param);
		break;
	}
}

void CCalendarStoreView::SetCalendarView(CCalendarView* view)
{
	mCalendarView = view;
	GetTable()->ResetTable();
}

CCalendarStoreWindow* CCalendarStoreView::GetCalendarStoreWindow(void) const
{
	return !Is3Pane() ? static_cast<CCalendarStoreWindow*>(GetOwningWindow()) : NULL;
}

// Return user action data
const CUserAction& CCalendarStoreView::GetPreviewAction() const
{
	return CPreferences::sPrefs->Get3PaneOptions().GetValue().GetViewOptions(N3Pane::eView_Calendar).GetUserAction(C3PaneOptions::C3PaneViewOptions::eListPreview);
}

// Return user action data
const CUserAction& CCalendarStoreView::GetFullViewAction() const
{
	return CPreferences::sPrefs->Get3PaneOptions().GetValue().GetViewOptions(N3Pane::eView_Calendar).GetUserAction(C3PaneOptions::C3PaneViewOptions::eListFullView);
}

CCalendarStoreTable* CCalendarStoreView::GetTable() const
{
	return static_cast<CCalendarStoreTable*>(mTable);
}

// Reset the table
void CCalendarStoreView::ResetTable()
{
	// reset table contents
	GetTable()->ResetTable();
	if (mCalendarView != NULL)
		mCalendarView->FullReset();
	
	// Reset zoom size if 1-pane
#ifdef _TODO
	if (!Is3Pane())
		GetServerWindow()->ResetStandardSize();
#endif
}

// Clear all contents out of the view prior to closing
void CCalendarStoreView::ClearTable()
{
	GetTable()->ClearTable();
}

// Check that close is allowed
bool CCalendarStoreView::TestClose()
{
	// Always allowed to close
	return true;
}

// Close it
void CCalendarStoreView::DoClose()
{
	// Set status
	SetClosing();

#ifdef _TODO
	// Delete all mailbox windows - in reverse
	{
		cdmutexprotect<CMailboxView::CMailboxViewList>::lock _lock(CMailboxView::sMailboxViews);
		for(CMailboxView::CMailboxViewList::reverse_iterator riter = CMailboxView::sMailboxViews->rbegin();
				riter != CMailboxView::sMailboxViews->rend(); riter++)
		{
			// Always delete if manager or mbox' protocol is logged off
			if (IsManager() ||
				(!GetServer()->IsLoggedOn() && (*riter)->GetMbox() && (GetServer() == (*riter)->GetMbox()->GetProtocol())))
				(*riter)->DoClose();
		}
	}
#endif

	// Write out any changed calendars
	OnFileSave();

	// Save state only if still visible
	if (IsVisible())
		SaveState();

	// Clear out data and stop listening
	ClearTable();

	// Save state
	SaveDefaultState();

	// Set status
	SetClosed();
	
#ifdef _TODO
	// Now close its parent 1-pane window at idle time
	if (!Is3Pane())
	{
		CCloseServerWindowTask* task = new CCloseServerWindowTask(GetServerWindow());
		task->Go();
	}
#endif
}

void CCalendarStoreView::OnFileSave()
{
	// Write out modified calendars
	calstore::CCalendarStoreManager::sCalendarStoreManager->SaveAllCalendars();
}
