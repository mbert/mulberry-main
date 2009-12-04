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


// Source for CServerView class

#include "CServerView.h"

#include "CLog.h"
#include "CMailboxView.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CServerTable.h"
#include "CServerToolbar.h"
#include "CServerWindow.h"
#include "CTaskClasses.h"
#include "C3PaneWindow.h"

#include <algorithm>

CServerWindow* CServerView::GetServerWindow(void) const
{
	return !Is3Pane() ? static_cast<CServerWindow*>(GetOwningWindow()) : NULL;
}

// Check for window
bool CServerView::ViewExists(const CServerView* wnd)
{
	cdmutexprotect<CServerViewList>::lock _lock(sServerViews);
	CServerViewList::iterator found = std::find(sServerViews->begin(), sServerViews->end(), wnd);
	return found != sServerViews->end();
}

void CServerView::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CServerToolbar::eBroadcast_HideCabinetItems:
		// Broadcast to our listeners
		Broadcast_Message(CServerToolbar::eBroadcast_HideCabinetItems);
		break;
	default:
		CTableView::ListenTo_Message(msg, param);
		break;
	}
}

// Return user action data
const CUserAction& CServerView::GetPreviewAction() const
{
	return CPreferences::sPrefs->Get3PaneOptions().GetValue().GetViewOptions(N3Pane::eView_Mailbox).GetUserAction(C3PaneOptions::C3PaneViewOptions::eListPreview);
}

// Return user action data
const CUserAction& CServerView::GetFullViewAction() const
{
	return CPreferences::sPrefs->Get3PaneOptions().GetValue().GetViewOptions(N3Pane::eView_Mailbox).GetUserAction(C3PaneOptions::C3PaneViewOptions::eListFullView);
}

// Set server
void CServerView::SetManager()
{
	GetTable()->SetManager();

	// Reset status changed by DoClose
	SetOpen();

	// Get window state
	ResetState();

}

// Set server
void CServerView::SetServer(CMboxProtocol* server)
{
	GetTable()->SetServer(server);

	// Reset status changed by DoClose
	SetOpen();

	// Get window state
	ResetState();

}

// Reset the table
void CServerView::ResetTable()
{
	// reset table contents
	GetTable()->ResetTable();
	
	// Reset zoom size if 1-pane
	if (!Is3Pane())
		GetServerWindow()->ResetStandardSize();
}

// Mailbox status changed
void CServerView::ChangedMbox(const CMbox* aMbox)
{
	GetTable()->ChangedMbox(aMbox);
}

// Check that close is allowed
bool CServerView::TestClose()
{
	// Always allowed to close
	return true;
}

// Close it
void CServerView::DoClose()
{
	// Set status
	SetClosing();

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

	// Save state
	SaveState();

	// Close server
	try
	{
#if 0
		bool was_logged_on = GetTable()->mServer->IsLoggedOn();

		// Close - clears cache
		if (mTable)
			GetTable()->mServer->Close();

		// Remove copy to items from all letter windows
		if (was_logged_on)
		{
			// Reset message window Copy To state
			CMessageWindow::ServerState(false);
		}
#endif
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}

	// Clear out data and stop listening
	ClearTable();

	// Set status
	SetClosed();
	
	// Now close its parent 1-pane window at idle time
	if (!Is3Pane())
	{
		CCloseServerWindowTask* task = new CCloseServerWindowTask(GetServerWindow());
		task->Go();
	}
}

