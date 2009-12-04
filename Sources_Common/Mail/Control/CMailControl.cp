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


// CMailControl.cp

/*
	Class to handle interaction between OS independent mail classes and OS/GUI dependent
	actions.
*/

#include "CMailControl.h"

#include "CActionManager.h"
#include "CAdbkProtocol.h"
#include "CAddressBookManager.h"
#include "CAddressBookView.h"
#include "CAddressView.h"
#include "CAdminLock.h"
#include "CAuthenticator.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CBalloonDialog.h"
#endif
#include "CCancelDialog.h"
#include "CErrorHandler.h"
#include "CFilterManager.h"
#include "CLetterWindow.h"
#include "CMailAccountManager.h"
#include "CMailboxInfoWindow.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMboxRef.h"
#include "CMessage.h"
#include "CMessageView.h"
#include "CMessageWindow.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CNewMailTask.h"
#include "CPasswordManager.h"
#include "CPreferences.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CResources.h"
#endif
#include "CServerView.h"
#include "CStringUtils.h"
#ifdef __use_speech
#include "CSpeechSynthesis.h"
#endif
#include "CTaskClasses.h"
#include "CXStringResources.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include <LThread.h>
#endif

#if __dest_os == __linux_os
#include <jXGlobals.h>
#endif

#include <algorithm>

#pragma mark Globals

#define WIN32_POSTTHREAD

bool CMailControl::sBlockBusy = false;
bool CMailControl::sAllowBusyKeys = false;
bool CMailControl::sWasBlocked = false;
unsigned long CMailControl::sBusyCount = 0;
bool CMailControl::sBusyDialog = false;
bool CMailControl::sCancelOthers = false;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
bool CMailControl::sPreventYield = false;		// Prevent yielding in main thread - hack for non-re-entrant WinSock during blocking call (LDAP)
#endif
unsigned long CMailControl::sBlockCount = 0;
cdstrqueue CMailControl::sAlerts;
CINETProtocolList CMailControl::sPeriodics;
bool CMailControl::sPeriodicsChanged = false;
long CMailControl::sTotalNew = 0;				// Accumalted number of new messages
CSimpleMboxList CMailControl::sMboxToAlert;		// Accumulated mailboxes for alert
CMboxRefList CMailControl::sMboxToOpen;			// Mailboxes to open

cdmutex sMboxToOpen_mutex;

#pragma mark -

// Try to recover disconnected server to previous state
void CMailControl::MboxServerReconnect(CMboxProtocol* server)
{

	// Assume server network connection is dead, but state correct
	// Assume caller will handle throws

	// Force logon to server
	server->Forceon();

	// Clean up UI items
	CleanUpServerDisconnect(server, true);

	// Recover policy:
	// Uncloned -> do nothing: each mbox has its own (still open) connection
	// Cloned -> reselect current mbox

	if (server->IsCloned())
	{
		// Must have existing mailbox
		if (server->GetCurrentMbox())
		{
			// Get mailbox to recover its state
			server->GetCurrentMbox()->Recover();
		}
		else
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}
	}
}

// Forced disconnection of server
void CMailControl::MboxServerDisconnect(CMboxProtocol* server)
{
	// Must get state of server here as it will be deleted by the end
	bool cloned = server->IsCloned();

	// Clean up UI items
	bool closed = CleanUpServerDisconnect(server, false);

	// Now clear out server (if not clone or clone not closed)
	// Clones will get deleted by mailbox window close.
	if (!cloned || !closed)
	{
		CServerForceoffTask* task = new CServerForceoffTask(server);
		task->Go();
	}
}

// Clean up after forced disconnection of protocol
bool CMailControl::CleanUpServerDisconnect(CMboxProtocol* server, bool reconnect)
{
	// Always reset all letter windows
	// if (!reconnect)
	{
		cdmutexprotect<CLetterWindow::CLetterWindowList>::lock _lock(CLetterWindow::sLetterWindows);
		for(CLetterWindow::CLetterWindowList::iterator iter = CLetterWindow::sLetterWindows->begin(); iter != CLetterWindow::sLetterWindows->end(); iter++)
			(*iter)->ServerState(server, false);
	}

	// Close all message windows belonging to server - in reverse
	{
		cdmutexprotect<CMessageWindow::CMessageWindowList>::lock _lock(CMessageWindow::sMsgWindows);
		for(CMessageWindow::CMessageWindowList::reverse_iterator riter = CMessageWindow::sMsgWindows->rbegin();
				riter != CMessageWindow::sMsgWindows->rend(); riter++)
		{
			CMessage* aMsg = (*riter)->GetMessage();
			if (aMsg && (aMsg->GetMbox()->GetMsgProtocol() == server))
			{
				// Clear message out of window to prevent stale references
				(*riter)->ClearMessage();
				
				// Close the window at idle time
				CCloseMessageWindowTask* task = new CCloseMessageWindowTask(*riter);
				task->Go();
			}
		}
	}

	// Close all message views belonging to server - in reverse
	{
		cdmutexprotect<CMessageView::CMessageViewList>::lock _lock(CMessageView::sMsgViews);
		for(CMessageView::CMessageViewList::reverse_iterator riter = CMessageView::sMsgViews->rbegin();
				riter != CMessageView::sMsgViews->rend(); riter++)
		{
			CMessage* aMsg = (*riter)->GetMessage();
			if (aMsg && aMsg->GetMbox()->TestMailer(server))
			{
				// Clear message out of wiew to prevent stale references
				(*riter)->ClearMessage();
				
				// Close the view at idle time
				CCloseMessageViewTask* task = new CCloseMessageViewTask(*riter);
				task->Go();
			}
		}
	}

	// Close all mailbox windows belonging to server if not reconnecting
	bool closed = false;
	if (!reconnect)
	{
		// Do in reverse as list changes on window delete
		{
			cdmutexprotect<CMailboxView::CMailboxViewList>::lock _lock(CMailboxView::sMailboxViews);
			for(CMailboxView::CMailboxViewList::reverse_iterator riter = CMailboxView::sMailboxViews->rbegin();
					riter != CMailboxView::sMailboxViews->rend(); riter++)
			{
				// Must make sure view is not in the process of closing
				if (!(*riter)->CWindowStatus::IsClosing() && (*riter)->ContainsProtocol(server))
				{
					// Clear mailbox out of view to prevent stale references
					(*riter)->ForceClose(server);

					// Found window so close it forcibly
					CCloseMailboxViewTask* task =  new CCloseMailboxViewTask(*riter, server, true);
					task->Go();
				}
			}
		}

		// Must check for an open mailbox
		if (server->GetCurrentMbox())
		{
			// Forcibly close it if safe to do so
			if (server->GetCurrentMbox()->SafeToClose())
			{
				server->GetCurrentMbox()->CloseSilent(true);
				closed = true;
			}
		}
	}

	return closed;
}

// Clean up as mailbox closes
void CMailControl::CleanUpMboxClose(CMbox* mbox)
{
	// Reset all letter windows if not reconnecting
	{
		cdmutexprotect<CLetterWindow::CLetterWindowList>::lock _lock(CLetterWindow::sLetterWindows);
		for(CLetterWindow::CLetterWindowList::iterator iter = CLetterWindow::sLetterWindows->begin(); iter != CLetterWindow::sLetterWindows->end(); iter++)
			(*iter)->MailboxState(mbox);
	}

	// Close all message windows belonging to server - in reverse
	{
		cdmutexprotect<CMessageWindow::CMessageWindowList>::lock _lock(CMessageWindow::sMsgWindows);
		for(CMessageWindow::CMessageWindowList::reverse_iterator riter = CMessageWindow::sMsgWindows->rbegin();
				riter != CMessageWindow::sMsgWindows->rend(); riter++)
		{
			CMessage* aMsg = (*riter)->GetMessage();
			if (aMsg && (aMsg->GetMbox() == mbox))
			{
				// Clear message out of window to prevent stale references
				(*riter)->ClearMessage();
				
				// Close the window at idle time
				CCloseMessageWindowTask* task = new CCloseMessageWindowTask(*riter);
				task->Go();
			}
		}
	}

	// Close all message views belonging to server - in reverse
	{
		cdmutexprotect<CMessageView::CMessageViewList>::lock _lock(CMessageView::sMsgViews);
		for(CMessageView::CMessageViewList::reverse_iterator riter = CMessageView::sMsgViews->rbegin();
				riter != CMessageView::sMsgViews->rend(); riter++)
		{
			CMessage* aMsg = (*riter)->GetMessage();
			if (aMsg && (aMsg->GetMbox() == mbox))
			{
				// Clear message out of wiew to prevent stale references
				(*riter)->ClearMessage();
				
				// Close the view at idle time
				CCloseMessageViewTask* task = new CCloseMessageViewTask(*riter);
				task->Go();
			}
		}
	}

	// Close all mailbox views belonging to mailbox
	// Do in reverse as list changes on window delete
	{
		cdmutexprotect<CMailboxView::CMailboxViewList>::lock _lock(CMailboxView::sMailboxViews);
		for(CMailboxView::CMailboxViewList::reverse_iterator riter = CMailboxView::sMailboxViews->rbegin();
				riter != CMailboxView::sMailboxViews->rend(); riter++)
		{
			// Must make sure view is not in the process of closing
			if (!(*riter)->IsClosing() && (*riter)->ContainsMailbox(mbox))
			{
				// Clear mailbox out of window to prevent stale references
				(*riter)->ForceClose(mbox);

				// Found view so close it forcibly
				CCloseMailboxViewTask* task =  new CCloseMailboxViewTask(*riter, mbox, true);
				task->Go();
			}
		}
	}
}

// Try to recover disconnected server to previous state
void CMailControl::AdbkServerReconnect(CAdbkProtocol* server)
{
	// Assume server network connection is dead, but state correct
	// Assume caller will handle throws

	// Force logon to server
	server->SetState(CINETProtocol::eINETLoggedOff);
	server->Logon();
}

// Forced disconnection of server
void CMailControl::AdbkServerDisconnect(CAdbkProtocol* server)
{
	// Force closed - will delete any windows
	server->Forceoff();

	// Tell manager its logged off (will update manager window)
	CAddressBookManager::sAddressBookManager->StopProtocol(server);
}

// Force user logon
void CMailControl::UserLogon()
{
#if 0
	CMulberryApp::sApp->BeginIMAP();
#endif
}

void CMailControl::MboxNewMessages(CMbox* mbox, unsigned long count)
{
	// Only for non-sync'ing
	if (mbox->IsSynchronising())
		return;
	
	// Trigger filters
	CPreferences::sPrefs->GetFilterManager()->NewMailMailbox(mbox, count);
}

void CMailControl::MboxAbouttoUpdate(CMbox* mbox)
{
	// Only for non-sync'ing
	if (mbox->IsSynchronising())
		return;

	// Force immediate preservation of selection
	{
		cdmutexprotect<CMailboxView::CMailboxViewList>::lock _lock(CMailboxView::sMailboxViews);
		for(CMailboxView::CMailboxViewList::iterator iter = CMailboxView::sMailboxViews->begin(); iter != CMailboxView::sMailboxViews->end(); iter++)
		{
			if ((*iter)->IsOpen() && ((*iter)->GetMbox() == mbox))
				(*iter)->GetTable()->PreserveSelection();
		}
	}
}

void CMailControl::MboxUpdate(CMbox* mbox)
{
	// Only for non-sync'ing
	if (mbox->IsSynchronising())
		return;

	// Make sure status of all similar mailboxes are sync'd

	// Look for mailbox in lists
	CMboxList list;
	mbox->GetProtocol()->FindMbox(mbox->GetName(), list);

	// Update status of others to this one
	for(CMboxList::iterator iter = list.begin(); iter != list.end(); iter++)
		static_cast<CMbox*>(*iter)->UpdateStatus(mbox);

	// NB Window ChangedMbox will take care of refreshing ALL similar rows
	{
		cdmutexprotect<CServerView::CServerViewList>::lock _lock(CServerView::sServerViews);
		for(CServerView::CServerViewList::iterator iter = CServerView::sServerViews->begin(); iter != CServerView::sServerViews->end(); iter++)
		{
			// Reset server table entry if status open
			if (((*iter)->IsManager() || ((*iter)->GetServer() == mbox->GetProtocol())) && (*iter)->IsOpen())
			{
				CServerViewChangedMboxTask* task = new CServerViewChangedMboxTask(*iter, mbox);
				task->Go();
			}
		}
	}

	// Do new mail cabinet processing
	mbox->ProcessCheck();
}

void CMailControl::MboxReset(CMbox* mbox)
{
	// Reset server table entry if open
	MboxUpdate(mbox);

	// Only do the rest if its open
	if (!mbox->IsOpen())
		return;

	// Renumber and resort after change
	mbox->RenumberMessages();
	mbox->SortMessages();

	// Reset window before alert so screen is up to date with alert on screen
	{
		cdmutexprotect<CMailboxView::CMailboxViewList>::lock _lock(CMailboxView::sMailboxViews);
		for(CMailboxView::CMailboxViewList::iterator iter = CMailboxView::sMailboxViews->begin(); iter != CMailboxView::sMailboxViews->end(); iter++)
		{
			if ((*iter)->IsOpen() && ((*iter)->GetMbox() == mbox))
			{
				CMailboxResetTableTask* task = new CMailboxResetTableTask(*iter);
				task->Go();
			}
		}
	}

	// Reset all message windows belonging to mailbox
	// Warning: window list may change if message is deleted - do in reverse
	{
		cdmutexprotect<CMessageWindow::CMessageWindowList>::lock _lock(CMessageWindow::sMsgWindows);
		for(CMessageWindow::CMessageWindowList::reverse_iterator riter = CMessageWindow::sMsgWindows->rbegin();
				riter != CMessageWindow::sMsgWindows->rend(); riter++)
		{
			CMessage* aMsg = (*riter)->GetMessage();
			if (aMsg && (aMsg->GetMbox() == mbox) && (*riter)->IsOpen())
			{
				CMessageWindowMessageChangedTask* task = new CMessageWindowMessageChangedTask(*riter);
				task->Go();
			}
		}
	}

	// Reset all message views belonging to mailbox
	{
		cdmutexprotect<CMessageView::CMessageViewList>::lock _lock(CMessageView::sMsgViews);
		for(CMessageView::CMessageViewList::iterator iter = CMessageView::sMsgViews->begin();
				iter != CMessageView::sMsgViews->end(); iter++)
		{
			CMessage* aMsg = (*iter)->GetMessage();
			if (aMsg && (aMsg->GetMbox() == mbox) && (*iter)->IsOpen())
			{
				CMessageViewMessageChangedTask* task = new CMessageViewMessageChangedTask(*iter);
				task->Go();
			}
		}
	}

	// Do new mail cabinet processing
	mbox->ProcessCheck();
}

void CMailControl::MboxRefresh(CMbox* mbox)
{
	// Reset server table entry if open
	MboxUpdate(mbox);

	// Only do the rest if its open
	if (!mbox->IsOpen())
		return;

	// Reset window before alert so screen is up to date with alert on screen
	{
		cdmutexprotect<CMailboxView::CMailboxViewList>::lock _lock(CMailboxView::sMailboxViews);
		for(CMailboxView::CMailboxViewList::iterator iter = CMailboxView::sMailboxViews->begin(); iter != CMailboxView::sMailboxViews->end(); iter++)
		{
			if ((*iter)->IsOpen() && ((*iter)->GetMbox() == mbox))
			{
				CMailboxRefreshTask* task = new CMailboxRefreshTask(*iter);
				task->Go();
			}
		}
	}
}

void CMailControl::MessageChanged(CMessage* msg, bool replaced)
{
	// Find mbox window for message
	{
		cdmutexprotect<CMailboxView::CMailboxViewList>::lock _lock(CMailboxView::sMailboxViews);
		for(CMailboxView::CMailboxViewList::iterator iter = CMailboxView::sMailboxViews->begin(); iter != CMailboxView::sMailboxViews->end(); iter++)
		{
			if ((*iter)->IsOpen() && ((*iter)->GetMbox() == msg->GetMbox()))
			{
				CMailboxViewMessageChangedTask* task = new CMailboxViewMessageChangedTask(*iter, msg);
				task->Go();
			}
		}
	}

	// If the message has only changed, rather than been replaced, reset any message windows
	if (!replaced)
	{
		// Reset all message windows belonging to mailbox
		// Warning: window list may change if message is deleted - do in reverse
		{
			cdmutexprotect<CMessageWindow::CMessageWindowList>::lock _lock(CMessageWindow::sMsgWindows);
			for(CMessageWindow::CMessageWindowList::reverse_iterator riter = CMessageWindow::sMsgWindows->rbegin();
					riter != CMessageWindow::sMsgWindows->rend(); riter++)
			{
				if (((*riter)->GetMessage() == msg) && (*riter)->IsOpen())
				{
					CMessageWindowMessageChangedTask* task = new CMessageWindowMessageChangedTask(*riter);
					task->Go();
				}
			}
		}

		// Reset all message views belonging to mailbox
		{
			cdmutexprotect<CMessageView::CMessageViewList>::lock _lock(CMessageView::sMsgViews);
			for(CMessageView::CMessageViewList::iterator iter = CMessageView::sMsgViews->begin();
					iter != CMessageView::sMsgViews->end(); iter++)
			{
				if (((*iter)->GetMessage() == msg) && (*iter)->IsOpen())
				{
					CMessageViewMessageChangedTask* task = new CMessageViewMessageChangedTask(*iter);
					task->Go();
				}
			}
		}
	}
}

void CMailControl::MessageRemoved(CMessage* msg)
{
	// Close all message windows belonging to mailbox - in reverse
	// Warning: window list may change if message is deleted - do in reverse
	{
		cdmutexprotect<CMessageWindow::CMessageWindowList>::lock _lock(CMessageWindow::sMsgWindows);
		for(CMessageWindow::CMessageWindowList::reverse_iterator riter = CMessageWindow::sMsgWindows->rbegin();
				riter != CMessageWindow::sMsgWindows->rend(); riter++)
		{
			// Compare with the one we want
			CMessage* test = (*riter)->GetMessage();

			// Iterate to find root owner
			while(test && test->GetOwner())
				test = test->GetOwner();

			// Do test
			if ((test == msg) && (*riter)->IsOpen())
			{
				// Clear message out of window to prevent stale references
				(*riter)->ClearMessage();
				
				// Close the window at idle time
				CCloseMessageWindowTask* task = new CCloseMessageWindowTask(*riter);
				task->Go();
			}
		}
	}

	// Reset all message views containing message
	{
		cdmutexprotect<CMessageView::CMessageViewList>::lock _lock(CMessageView::sMsgViews);
		for(CMessageView::CMessageViewList::iterator iter = CMessageView::sMsgViews->begin(); iter != CMessageView::sMsgViews->end(); iter++)
		{
			if (msg == (*iter)->GetMessage())
			{
				// Clear message out of wiew to prevent stale references
				(*iter)->ClearMessage();
				
				// Reset the view at idle time
				CMessagePreviewTask* task = new CMessagePreviewTask(*iter, NULL);
				task->Go();
			}
		}
	}

	// Make sure all draft windows remove references to this message
	{
		cdmutexprotect<CLetterWindow::CLetterWindowList>::lock _lock(CLetterWindow::sLetterWindows);
		for(CLetterWindow::CLetterWindowList::iterator iter = CLetterWindow::sLetterWindows->begin(); iter != CLetterWindow::sLetterWindows->end(); iter++)
		{
			CLetterWindowMessageRemovedTask* task = new CLetterWindowMessageRemovedTask(*iter, msg);
			task->Go();
		}
	}
}

#pragma mark ____________________________Address changes

void CMailControl::AddressBookClosed(CAddressBook* adbk)
{
	// Close all address book views belonging to address book
	// Do in reverse as list changes on window delete
	cdmutexprotect<CAddressBookView::CAddressBookViewList>::lock _lock(CAddressBookView::sAddressBookViews);
	for(CAddressBookView::CAddressBookViewList::reverse_iterator riter = CAddressBookView::sAddressBookViews->rbegin();
			riter != CAddressBookView::sAddressBookViews->rend(); riter++)
	{
		if (((*riter)->GetAddressBook() == adbk) && (*riter)->IsOpen())
		{
			// Clear address out of preview to prevent stale references
			(*riter)->GetPreview()->ClearAddress();
			(*riter)->GetPreview()->ClearGroup();
				
			CAddressBookViewClosedTask* task = new CAddressBookViewClosedTask(*riter);
			task->Go();
		}
	}
}

void CMailControl::AddressAdded(CAddressBook* adbk, CAddressList* addrs)
{
	// Reset all address book views containing address book
	cdmutexprotect<CAddressBookView::CAddressBookViewList>::lock _lock(CAddressBookView::sAddressBookViews);
	for(CAddressBookView::CAddressBookViewList::iterator iter = CAddressBookView::sAddressBookViews->begin();
			iter != CAddressBookView::sAddressBookViews->end(); iter++)
	{
		if (((*iter)->GetAddressBook() == adbk) && (*iter)->IsOpen())
		{
			CAddressBookViewChangedTask* task = new CAddressBookViewChangedTask(*iter, true);
			task->Go();
		}
	}

}

void CMailControl::AddressChanged(CAddressBook* adbk, CAddressList* addrs)
{
	// Reset address previews
	for(CAddressList::const_iterator iter1 = addrs->begin(); iter1 != addrs->end(); iter1++)
	{
		cdmutexprotect<CAddressView::CAddressViewList>::lock _lock(CAddressView::sAddressViews);
		for(CAddressView::CAddressViewList::iterator iter2 = CAddressView::sAddressViews->begin(); iter2 != CAddressView::sAddressViews->end(); iter2++)
		{
			if ((*iter2)->IsOpen() && ((*iter2)->GetAddress() == *iter1))
			{
				CAddressViewChangedTask* task = new CAddressViewChangedTask(*iter2, adbk, static_cast<CAdbkAddress*>(*iter1));
				task->Go();
			}
		}
	}

	// Reset all address book views containing address book
	{
		cdmutexprotect<CAddressBookView::CAddressBookViewList>::lock _lock(CAddressBookView::sAddressBookViews);
		for(CAddressBookView::CAddressBookViewList::iterator iter = CAddressBookView::sAddressBookViews->begin();
				iter != CAddressBookView::sAddressBookViews->end(); iter++)
		{
			if (((*iter)->GetAddressBook() == adbk) && (*iter)->IsOpen())
			{
				CAddressBookViewChangedTask* task = new CAddressBookViewChangedTask(*iter);
				task->Go();
			}
		}
	}
}

void CMailControl::AddressRemoved(CAddressBook* adbk, CAddressList* addrs)
{
	// Reset address previews
	for(CAddressList::const_iterator iter1 = addrs->begin(); iter1 != addrs->end(); iter1++)
	{
		cdmutexprotect<CAddressView::CAddressViewList>::lock _lock(CAddressView::sAddressViews);
		for(CAddressView::CAddressViewList::iterator iter2 = CAddressView::sAddressViews->begin(); iter2 != CAddressView::sAddressViews->end(); iter2++)
		{
			if ((*iter2)->IsOpen() && ((*iter2)->GetAddress() == *iter1))
			{
				// Clear address out of wiew to prevent stale references
				(*iter2)->ClearAddress();
				
				// Set address views address to NULL to force display update
				CAdbkAddress* dummy = NULL;
				CAddressViewChangedTask* task = new CAddressViewChangedTask(*iter2, adbk, dummy);
				task->Go();
			}
		}
	}

	// Reset all address book views containing address book
	{
		cdmutexprotect<CAddressBookView::CAddressBookViewList>::lock _lock(CAddressBookView::sAddressBookViews);
		for(CAddressBookView::CAddressBookViewList::iterator iter = CAddressBookView::sAddressBookViews->begin();
				iter != CAddressBookView::sAddressBookViews->end(); iter++)
		{
			if (((*iter)->GetAddressBook() == adbk) && (*iter)->IsOpen())
			{
				CAddressBookViewChangedTask* task = new CAddressBookViewChangedTask(*iter, true);
				task->Go();
			}
		}
	}
}

void CMailControl::GroupAdded(CAddressBook* adbk, CGroupList* grps)
{
	// Reset all address book views containing address book
	cdmutexprotect<CAddressBookView::CAddressBookViewList>::lock _lock(CAddressBookView::sAddressBookViews);
	for(CAddressBookView::CAddressBookViewList::iterator iter = CAddressBookView::sAddressBookViews->begin();
			iter != CAddressBookView::sAddressBookViews->end(); iter++)
	{
		if (((*iter)->GetAddressBook() == adbk) && (*iter)->IsOpen())
		{
			CAddressBookViewChangedTask* task = new CAddressBookViewChangedTask(*iter, true);
			task->Go();
		}
	}

}

void CMailControl::GroupChanged(CAddressBook* adbk, CGroupList* grps)
{
	// Reset group previews
	for(CGroupList::const_iterator iter1 = grps->begin(); iter1 != grps->end(); iter1++)
	{
		cdmutexprotect<CAddressView::CAddressViewList>::lock _lock(CAddressView::sAddressViews);
		for(CAddressView::CAddressViewList::iterator iter2 = CAddressView::sAddressViews->begin(); iter2 != CAddressView::sAddressViews->end(); iter2++)
		{
			if ((*iter2)->IsOpen() && ((*iter2)->GetGroup() == *iter1))
			{
				CAddressViewChangedTask* task = new CAddressViewChangedTask(*iter2, adbk, *iter1);
				task->Go();
			}
		}
	}

	// Reset all address book views containing address book
	{
		cdmutexprotect<CAddressBookView::CAddressBookViewList>::lock _lock(CAddressBookView::sAddressBookViews);
		for(CAddressBookView::CAddressBookViewList::iterator iter = CAddressBookView::sAddressBookViews->begin();
				iter != CAddressBookView::sAddressBookViews->end(); iter++)
		{
			if (((*iter)->GetAddressBook() == adbk) && (*iter)->IsOpen())
			{
				CAddressBookViewChangedTask* task = new CAddressBookViewChangedTask(*iter, true);
				task->Go();
			}
		}
	}
}

void CMailControl::GroupRemoved(CAddressBook* adbk, CGroupList* grps)
{
	// Reset group previews
	for(CGroupList::const_iterator iter1 = grps->begin(); iter1 != grps->end(); iter1++)
	{
		cdmutexprotect<CAddressView::CAddressViewList>::lock _lock(CAddressView::sAddressViews);
		for(CAddressView::CAddressViewList::iterator iter2 = CAddressView::sAddressViews->begin(); iter2 != CAddressView::sAddressViews->end(); iter2++)
		{
			if ((*iter2)->IsOpen() && ((*iter2)->GetGroup() == *iter1))
			{
				// Clear group out of wiew to prevent stale references
				(*iter2)->ClearGroup();
				
				// Set address views group to NULL to force display update
				CGroup* dummy = NULL;
				CAddressViewChangedTask* task = new CAddressViewChangedTask(*iter2, adbk, dummy);
				task->Go();
			}
		}
	}

	// Reset all address book views containing address book
	{
		cdmutexprotect<CAddressBookView::CAddressBookViewList>::lock _lock(CAddressBookView::sAddressBookViews);
		for(CAddressBookView::CAddressBookViewList::iterator iter = CAddressBookView::sAddressBookViews->begin();
				iter != CAddressBookView::sAddressBookViews->end(); iter++)
		{
			if (((*iter)->GetAddressBook() == adbk) && (*iter)->IsOpen())
			{
				CAddressBookViewChangedTask* task = new CAddressBookViewChangedTask(*iter, true);
				task->Go();
			}
		}
	}
}

#pragma mark ____________________________Periodics

void CMailControl::SpendTime(bool force_tickle, bool do_checks)
{
	if (!force_tickle && do_checks && CMailAccountManager::sMailAccountManager)
		CMailAccountManager::sMailAccountManager->SpendTime();

	sPeriodicsChanged = false;
	CINETProtocolList copy(sPeriodics);
	for(CINETProtocolList::iterator iter = copy.begin(); iter != copy.end(); iter++)
	{
		// First make sure it is still in the list
		CINETProtocolList::iterator found = std::find(sPeriodics.begin(), sPeriodics.end(), *iter);
		if (found != sPeriodics.end())
		{
			// Don't allow throw-ups
			try
			{
				(*iter)->SpendTime(force_tickle);
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Éand do nothing
			}
		}
	}
}

void CMailControl::RegisterPeriodic(CINETProtocol* periodic, bool add)
{
	if (add)
	{
		// Only add once
		CINETProtocolList::iterator found = std::find(sPeriodics.begin(), sPeriodics.end(), periodic);
		if (found == sPeriodics.end())
			sPeriodics.push_back(periodic);
	}
	else
	{
		CINETProtocolList::iterator found = std::find(sPeriodics.begin(), sPeriodics.end(), periodic);
		if (found != sPeriodics.end())
			sPeriodics.erase(found);
	}

	sPeriodicsChanged = true;
}

#pragma mark ____________________________Alerts

// If required, prompts user for any info
bool CMailControl::PromptUser(CAuthenticator* acct_auth,
								const CINETAccount* acct,
								bool secure,
								bool rmail, bool rsmtp, bool rprefs, bool radbk, bool rfilter, bool rhttp,
								bool first)
{
	// Only authenticators that require user id/pswd do this
	if (!acct_auth->RequiresUserPswd())
		// NB Must fail after first attempt as there is no way to
		// reset parameters for subsequent attempts!
		return first;

	// Get user/pswd authenticator
	CAuthenticatorUserPswd* userpswd = static_cast<CAuthenticatorUserPswd*>(acct_auth);

	// Compare with existing details (only do pswd dialog once)
	if (first && !userpswd->GetPswd().empty())
		return true;

	// If second time through => auth failure before, therefore allow user & pswd to be edited
	bool actual_save_uid = first ? static_cast<CAuthenticatorUserPswd*>(acct_auth)->GetSaveUID() : false;
	bool actual_save_pswd = first ? static_cast<CAuthenticatorUserPswd*>(acct_auth)->GetSavePswd() : false;

	// Do dialog and return result
	return CMailControl::DoUserIDPassword(static_cast<CAuthenticatorUserPswd*>(acct_auth), acct, secure, 
											rmail, rsmtp, rprefs, radbk, rfilter, rhttp, actual_save_uid, actual_save_pswd, first);
}

// Do user id & password authentication
bool CMailControl::DoUserIDPassword(CAuthenticatorUserPswd* auth,
										const CINETAccount* acct,
										bool secure,
										bool rmail, bool rsmtp, bool rprefs, bool radbk, bool rfilter, bool rhttp,
										bool save_user, bool save_pswd, bool first)
{
	// Flag for first time through
	bool save_MUPrefsUsed = CMulberryApp::sApp->MUPrefsUsed() && first;

	// check for admin pswd
	if (!CAdminLock::sAdminLock.mInitialPswd.empty())
	{
		auth->SetPswd(CAdminLock::sAdminLock.mInitialPswd);
		CAdminLock::sAdminLock.mInitialPswd = cdstring::null_str;
	}

	// Need special case for first time through Match User ID and Password
	bool first_save_user = first || save_user;

	// check for cached pswd
	if (CPreferences::sPrefs->mRemoteCachePswd.GetValue() && first_save_user && !CINETProtocol::GetCachedPswd(auth->GetUID()).empty())
	{
		auth->SetPswd(CINETProtocol::GetCachedPswd(auth->GetUID()));
		return true;
	}

	// Try password manager
	cdstring savedpswd;
	if (first_save_user && CPasswordManager::GetManager()->GetPassword(acct, savedpswd))
	{
		auth->SetPswd(savedpswd);
		return true;
	}
	else
	{
		cdstring old_uid = auth->GetUID();
		cdstring old_pswd = auth->GetPswd();
		cdstring uid = old_uid;
		cdstring pswd = old_pswd;
		cdstring method = auth->GetDescriptor();
		
		// Server descriptor is account name followed by IP address in parenthesis
		cdstring server = acct->GetName();
		server += " (";
		server += acct->GetServerIP();
		server += ")";

		// Get password if required
		if ((!save_user && !save_MUPrefsUsed) || uid.empty() ||
			(!save_pswd && !save_MUPrefsUsed) || pswd.empty())
		{
			cdstring title;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			ResIDT icon;
			if (rmail)
			{
				icon = (acct->GetServerType() == CINETAccount::ePOP3) ? ICNx_UserPswdPOP3Mailbox : ICNx_UserPswdMailbox;
				title = rsrc::GetString(acct->GetServerType() == CINETAccount::ePOP3 ? "Alerts::UserPswd::POP3LOGIN" : "Alerts::UserPswd::IMAPLOGIN");
			}
			else if (rsmtp)
			{
				icon = ICNx_UserPswdSMTP;
				title = rsrc::GetString("Alerts::UserPswd::SMTPLOGIN");
			}
			else if (rprefs)
			{
				icon = ICNx_UserPswdPrefs;
				title = rsrc::GetString("Alerts::UserPswd::PREFSLOGIN");
			}
			else if (radbk)
			{
				switch(acct->GetServerType())
				{
				case CINETAccount::eIMSP:
				case CINETAccount::eACAP:
				case CINETAccount::eCardDAVAdbk:
					icon = ICNx_UserPswdAddressBook;
					title = rsrc::GetString("Alerts::UserPswd::ADBKLOGIN");
					break;
				case CINETAccount::eLDAP:
					icon = ICNx_UserPswdLDAP;
					title = rsrc::GetString("Alerts::UserPswd::LDAPLOGIN");
					break;
				default:;
				}
			}
			else if (rfilter)
			{
				icon = ICNx_UserPswdManageSIEVE;
				title = rsrc::GetString("Alerts::UserPswd::MANAGESIEVELOGIN");
			}
			else if (rhttp)
			{
				icon = ICNx_UserPswdRemoteCalendar;
				title = rsrc::GetString("Alerts::UserPswd::REMOTECALENDARLOGIN");
			}
#elif __dest_os == __win32_os
			int icon;
			if (rmail)
			{
				icon = (acct->GetServerType() == CINETAccount::ePOP3) ? IDI_USERPSWD_POP3 : IDI_USERPSWD_IMAP;
				title = rsrc::GetString((acct->GetServerType() == CINETAccount::ePOP3) ? "Alerts::UserPswd::POP3LOGIN" : "Alerts::UserPswd::IMAPLOGIN");
			}
			else if (rsmtp)
			{
				icon = IDI_USERPSWD_SMTP;
				title = rsrc::GetString("Alerts::UserPswd::SMTPLOGIN");
			}
			else if (rprefs)
			{
				icon = IDI_USERPSWD_PREFS;
				title = rsrc::GetString("Alerts::UserPswd::PREFSLOGIN");
			}
			else if (radbk)
			{
				switch(acct->GetServerType())
				{
				case CINETAccount::eIMSP:
				case CINETAccount::eACAP:
				case CINETAccount::eCardDAVAdbk:
					icon = IDI_USERPSWD_ADBK;
					title = rsrc::GetString("Alerts::UserPswd::ADBKLOGIN");
				break;
				case CINETAccount::eLDAP:
					icon = IDI_USERPSWD_LDAP;
					title = rsrc::GetString("Alerts::UserPswd::LDAPLOGIN");
					break;
				}
			}
			else if (rfilter)
			{
				icon = IDI_USERPSWD_MANAGESIEVE;
				title = rsrc::GetString("Alerts::UserPswd::MANAGESIEVELOGIN");
			}
			else if (rfilter)
			{
				icon = IDI_USERPSWD_REMOTECALENDAR;
				title = rsrc::GetString("Alerts::UserPswd::REMOTECALENDARLOGIN");
			}
			else if (rhttp)
			{
				icon = IDI_USERPSWD_REMOTECALENDAR;
				title = rsrc::GetString("Alerts::UserPswd::REMOTECALENDARLOGIN");
			}
#else
			title.reserve(256);
			int icon;
			if (rmail)
			{
				icon = (acct->GetServerType() == CINETAccount::ePOP3) ? IDI_USERPSWD_POP3 : IDI_USERPSWD_IMAP;
				title = rsrc::GetString((acct->GetServerType() == CINETAccount::ePOP3) ? "Alerts::UserPswd::POP3LOGIN" : "Alerts::UserPswd::IMAPLOGIN");
			}
			else if (rsmtp)
			{
				icon = IDI_USERPSWD_SMTP;
				title = rsrc::GetString("Alerts::UserPswd::SMTPLOGIN");
			}
			else if (rprefs)
			{
				icon = IDI_USERPSWD_PREFS;
				title = rsrc::GetString("Alerts::UserPswd::PREFSLOGIN");
			}
			else if (radbk)
			{
				switch(acct->GetServerType())
				{
				case CINETAccount::eIMSP:
				case CINETAccount::eACAP:
				case CINETAccount::eCardDAVAdbk:
					icon = IDI_USERPSWD_ADBK;
					title = rsrc::GetString("Alerts::UserPswd::ADBKLOGIN");
				break;
				case CINETAccount::eLDAP:
					icon = IDI_USERPSWD_LDAP;
					title = rsrc::GetString("Alerts::UserPswd::LDAPLOGIN");
					break;
				default:;
				}
			}
			else if (rfilter)
			{
				icon = IDI_USERPSWD_MANAGESIEVE;
				title = rsrc::GetString("Alerts::UserPswd::MANAGESIEVELOGIN");
			}
			else if (rhttp)
			{
				icon = IDI_USERPSWD_REMOTECALENDAR;
				title = rsrc::GetString("Alerts::UserPswd::REMOTECALENDARLOGIN");
			}
#endif

			CUserPswdTask* task = new CUserPswdTask(uid, pswd, save_user, save_pswd,
											title, server, method, secure, icon);
			if (!task->Go())
				// Break out of logon loop
				return false;

			// Look for change to uid & pswd and force change and prefs update
			if ((old_uid != uid) || (old_pswd != pswd))
			{
				if (old_uid != uid)
					auth->SetUID(uid);
				if (old_pswd != pswd)
					auth->SetPswd(pswd);
				if (rmail)
					CPreferences::sPrefs->mMailAccounts.SetDirty();
				else if (rsmtp)
					CPreferences::sPrefs->mSMTPAccounts.SetDirty();
				else if (rprefs)
					CPreferences::sPrefs->mRemoteAccounts.SetDirty();
				else if (radbk)
					CPreferences::sPrefs->mAddressAccounts.SetDirty();
				else if (rfilter)
					CPreferences::sPrefs->mSIEVEAccounts.SetDirty();
				else if (rhttp)
					CPreferences::sPrefs->mCalendarAccounts.SetDirty();
			}
		}
	}

	return true;
}

// Do new mail alert
void CMailControl::NewMailUpdate(CMbox* mbox, long num_new, const CMailNotification& notify)
{
	// Check for open window
	if (notify.GetOpenMbox())
	{
		cdmutex::lock_cdmutex _lock(sMboxToOpen_mutex);
		sMboxToOpen.push_back(new CMboxRef(mbox));
	}

	// Reset all message windows belonging to mailbox
	cdstring mbox_acct = mbox->GetAccountName();

	// Reset all message windows belonging to mailbox
	// Warning: window list may change if message is deleted - do in reverse
	{
		cdmutexprotect<CMessageWindow::CMessageWindowList>::lock _lock(CMessageWindow::sMsgWindows);
		for(CMessageWindow::CMessageWindowList::reverse_iterator riter = CMessageWindow::sMsgWindows->rbegin();
				riter != CMessageWindow::sMsgWindows->rend(); riter++)
		{
			CMessage* aMsg = (*riter)->GetMessage();
			if (aMsg && (aMsg->GetMbox()->GetAccountName() == mbox_acct) && (*riter)->IsOpen())
			{
				// Found window to refresh captions etc
				CMessageWindowMessageChangedTask* task = new CMessageWindowMessageChangedTask(*riter);
				task->Go();
			}
		}
	}

	// Reset all message views belonging to mailbox
	{
		cdmutexprotect<CMessageView::CMessageViewList>::lock _lock(CMessageView::sMsgViews);
		for(CMessageView::CMessageViewList::iterator iter = CMessageView::sMsgViews->begin();
				iter != CMessageView::sMsgViews->end(); iter++)
		{
			CMessage* aMsg = (*iter)->GetMessage();
			if (aMsg && (aMsg->GetMbox() == mbox) && (*iter)->IsOpen())
			{
				CMessageViewMessageChangedTask* task = new CMessageViewMessageChangedTask(*iter);
				task->Go();
			}
		}
	}

#ifdef __use_speech
	// If speech requested, queue before anything else
	if (CSpeechSynthesis::Available() && notify.DoSpeakText() && notify.SpeakSeparate())
	{
		cdstring str;

		const char* p = notify.GetTextToSpeak();
		while(*p)
		{
			switch(*p)
			{
			case '#':
				str += cdstring(num_new);
				break;
			case '*':
				// Add the mailbox name
				NewMailAlertSpeakMbox(mbox, str);
				break;
			default:
				str += *p;
			}
			p++;
		}

		// Speak user string as is
		CSpeechSynthesis::SpeakString(str);
	}
	else
	{
		sTotalNew += num_new;
		sMboxToAlert.push_back(mbox);
	}

#endif

	// Speak new in open mailbox? (Always do this as soon as they are known and multiple alerts requested)
#ifdef __use_speech
	if (CSpeechSynthesis::Available() && notify.SpeakSeparate() && CPreferences::sPrefs->mSpeakNewArrivals.GetValue())
	{
		// Reset window before alert so screen is up to date with alert on screen
		CMailboxView* theView = CMailboxView::FindView(mbox);

		if (theView)
			theView->SpeakRecentMessages();
	}
#endif
}

// Do new mail alert
void CMailControl::NewMailAlert(const CMailNotification& notifier)
{
#ifdef __use_speech
	// If speech requested, queue before anything else
	if (CSpeechSynthesis::Available() && notifier.DoSpeakText() && sTotalNew && sMboxToAlert.size())
	{
		cdstring str;

		// Check for number
		if (notifier.SpeakCombined())
		{
			const char* p = notifier.GetTextToSpeak();
			while(*p)
			{
				switch(*p)
				{
				case '#':
					// Always speak total in combined mode
					str += cdstring(sTotalNew);

					// Bump past double
					if (*(p+1) == '#')
						p++;
					break;
				case '*':
					{
						// Speak each mailbox in turn
						bool first = true;
						for(CSimpleMboxList::const_iterator iter = sMboxToAlert.begin(); iter != sMboxToAlert.end(); iter++)
						{
							// Space between multiple items
							if (first)
								first = false;
							else
								str += ", ";

							// Add the mailbox name
							NewMailAlertSpeakMbox(*iter, str);
						}

						// Bump past double
						if (*(p+1) == '*')
							p++;
					}
					break;
				default:
					str += *p;
				}
				p++;
			}
		}
		else
			str = notifier.GetTextToSpeak();

		// Speak user string as is
		CSpeechSynthesis::SpeakString(str);
	}

#endif

	// Now do alert based on notifier settings
	{
		CNewMailAlertTask::Make_NewMailAlertTask("Alerts::General::NewMailArrived", notifier);
	}

	// Speak new in open mailbox?
#ifdef __use_speech
	if (CSpeechSynthesis::Available() && CPreferences::sPrefs->mSpeakNewArrivals.GetValue())
	{
		for(CSimpleMboxList::const_iterator iter = sMboxToAlert.begin(); iter != sMboxToAlert.end(); iter++)
		{
			CMailboxView* theView = CMailboxView::FindView(*iter);
			if (theView)
				theView->SpeakRecentMessages();
		}
	}
#endif


	// Must clear items
	sTotalNew = 0;
	sMboxToAlert.clear();
}

// Open mailboxes for new mail
void CMailControl::NewMailOpen()
{
	// WARNING this must only be called from the main thread

	// Must prevent re-entrancy as this can lead to
	// infinite recursion if the network open takes a while
	if (sMboxToOpen_mutex.is_locked())
		return;

	// Block access to the list
	cdmutex::lock_cdmutex _lock(sMboxToOpen_mutex);
	
	// Open each mailbox
	for(CMboxRefList::reverse_iterator riter = sMboxToOpen.rbegin(); riter != sMboxToOpen.rend(); riter++)
	{
		// Resolve to mbox
		CMbox* mbox = static_cast<CMboxRef*>(*riter)->ResolveMbox();
		if (mbox)
			CActionManager::OpenMailbox(mbox, true);
	}
	
	// Clear the list
	sMboxToOpen.DeleteAll();
}

void CMailControl::NewMailAlertSpeakMbox(const CMbox* mbox, cdstring& speak)
{
	// Get full mbox name and replace special
	// chars with spoken items
	cdstring name =  mbox->GetAccountName(CMailAccountManager::sMailAccountManager->GetProtocolList().size() > 1);
	::strlower(name);
	const char* c = name.c_str();
	while(*c)
	{
		switch(*c)
		{
		case '#':
			speak += " hash ";
			break;
		case '.':
			speak += " dot ";
			break;
		case '/':
			speak += " slash ";
			break;
		case ':':	// Separates account name from mailbox name
			speak += " ";
			break;
		case '~':
			speak += " tilde ";
			break;
		default:
			speak += *c;
		}
		c++;
	}
}

// Message size alert
bool CMailControl::CheckSizeWarning(const CMessage* msg, bool all)
{
	// Check message size first
	unsigned long size = msg->CheckSizeWarning();
	if (size != 0)
	{
		cdstring size_txt = ::GetNumericFormat(size);
		bool dontshow = false;
		short answer = CErrorHandler::PutCautionAlertRsrcStr(true, "Alerts::Message::WarnSize", size_txt, NULL, &dontshow);
		if (dontshow)
			CPreferences::sPrefs->mDoSizeWarn.SetValue(false);
		if (answer == CErrorHandler::Cancel)
			return false;
	}
	
	return true;
}

// Attachment size alert
bool CMailControl::CheckSizeWarning(const CAttachment* attach)
{
	// Check attachment size first
	unsigned long size = attach->GetSize();
	if (CPreferences::sPrefs->mDoSizeWarn.GetValue() &&
		(CPreferences::sPrefs->warnMessageSize.GetValue() > 0) &&
		(size > CPreferences::sPrefs->warnMessageSize.GetValue() * 1024L))
	{
		cdstring size_txt = ::GetNumericFormat(size);
		bool dontshow = false;
		short answer = CErrorHandler::PutCautionAlertRsrcStr(true, "Alerts::Message::WarnSize", size_txt, NULL, &dontshow);
		if (dontshow)
			CPreferences::sPrefs->mDoSizeWarn.SetValue(false);
		if (answer == CErrorHandler::Cancel)
			return false;
	}
	
	return true;
}

void CMailControl::PopAlert(cdstring& txt)
{
	// Check for available string
	if (!sAlerts.empty())
	{
		// Copy and pop from queue
		txt = sAlerts.front();
		sAlerts.pop();
	}
	else
		txt = cdstring::null_str;
}

// Resolve nick-name
bool CMailControl::ResolveNickName(const char* nick_name, CAddressList* list)
{
	// May not be initialised
	return CAddressBookManager::sAddressBookManager ?
			CAddressBookManager::sAddressBookManager->ResolveNickName(nick_name, list) :
			false;
}

// Resolve group name
bool CMailControl::ResolveGroupName(const char* grp_name, CAddressList* list, short level)
{
	// May not be initialised
	return CAddressBookManager::sAddressBookManager ?
			CAddressBookManager::sAddressBookManager->ResolveGroupName(grp_name, list, level) :
			false;
}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#pragma mark ____________________________Mac OS Busy

bool CMailControl::BusyCancel(const CBusyContext* busy, bool allow_cancel)
{
	bool cancel = false;
	EventRecord macEvent;

	// Timeouts
	unsigned long busy_start = CPreferences::sPrefs ? CPreferences::sPrefs->mWaitDialogStart.GetValue() : 5;
	unsigned long busy_max = CPreferences::sPrefs ? CPreferences::sPrefs->mWaitDialogMax.GetValue() : 5 * 60;

	// Beep if busy time exceeded
	if (allow_cancel && busy && (::time(NULL) > busy->GetBusyTime() + busy_start))
	{
		// Check for max timeout
		if (::time(NULL) > busy->GetBusyTime() + busy_max)
		{
			CLOG_LOGERROR("CMailControl::BusyCancel - busy time exceeded");
			cancel = true;
		}
		else
		{
			// Display the busy dialog if not already up
			if (!sBusyDialog)
				DisplayBusyDialog(*busy);
			
			// Handle an event on the busy dialog
			cancel = HandleBusyDialog(*busy);
			if (cancel)
				CLOG_LOGERROR("CMailControl::BusyCancel - busy dialog cancel");
		}
	}
	else 
	{
		// Retrieve the next event. Context switch could happen here.
		bool gotEvent = ::WaitNextEvent(everyEvent, &macEvent, 0, nil);

		// Filter events
		switch (macEvent.what)
		{

			// Check for cmd-Period or ESC
			case keyDown:
				short theKey = macEvent.message & charCodeMask;
				if (((macEvent.modifiers & cmdKey) && (theKey==char_Period)) || (theKey==char_Escape))
				{
					CLOG_LOGERROR("CMailControl::BusyCancel - user cancel");
					cancel = true;
				}

				// Allow keystrokes which are not menus
				else if (sAllowBusyKeys && !(macEvent.modifiers & cmdKey))
					CMulberryApp::sApp->DispatchEvent(macEvent);

				break;

			// Allow suspend/resume, activate & update events
			case osEvt:
			case activateEvt:
			case updateEvt:
				CMulberryApp::sApp->DispatchEvent(macEvent);
				break;

			// Allow Apple menu items (not About) & Appl menu
			case mouseDown:
				WindowPtr	macWindowP;

				SInt16	thePart = ::FindWindow(macEvent.where, &macWindowP);

				// Only handle menu bar click
				if (thePart==inMenuBar)
				{
					// Disable all menus except Apple (no About) & Appl menu
					LMenuBar	*theMenuBar = LMenuBar::GetCurrentMenuBar();
					LMenu*		theMenu = nil;
					short		menuPos = 0;
					while (theMenuBar->FindNextMenu(theMenu))
					{
						menuPos++;
						// Check fopr Apple menu
						if (theMenu->GetMenuID()==MENU_Apple)
							// Disable About item only
							::DisableItem(theMenu->GetMacMenuH(), 1);
						else if (menuPos<=7)	// What about speech menu?
						{
							for(short i=1; i<=::CountMenuItems(theMenu->GetMacMenuH()); i++)
								::DisableItem(theMenu->GetMacMenuH(), i);
						}
					}

					// Get selected menu item
					long menuChoice;
					long menuCmd = theMenuBar->MenuCommandSelection(macEvent,menuChoice);

					// Only do negative cmds
					if (menuCmd < 0)
					{
						SignalIf_(CMulberryApp::sApp->GetTarget() == nil);
						CMulberryApp::sApp->GetTarget()->ProcessCommand(menuCmd,  nil);
					}
					::HiliteMenu(0);
				}

			default:
				if (!CMailControl::GetPreventYield())
					::YieldToAnyThread();
		}
	}
	
	return cancel;
}

static CBalloonDialog* handler = NULL;
static CCancelDialog* busyDialog = NULL;

// Display busy dialog to user
void CMailControl::DisplayBusyDialog(const CBusyContext& busy)
{
	// Create the busy dialog - make sure periodicals are turned off
	handler = new CBalloonDialog(10005, LCommander::GetTarget(), true);
	busyDialog = static_cast<CCancelDialog*>(handler->GetDialog());
	busyDialog->SetBusyDescriptor(busy.GetDescriptor());
	handler->StartDialog();
	sBusyDialog = true;
}

// Handle busy dialog event returning if cancelled
bool CMailControl::HandleBusyDialog(const CBusyContext& busy)
{
	// Set current time
	busyDialog->SetTime(::time(NULL) - busy.GetBusyTime());
	
	// Process an event
	MessageT hitMessage = handler->DoDialog();
	return (hitMessage == msg_Cancel);
}

// Remove the busy dialog
void CMailControl::RemoveBusyDialog()
{
	// Remove the busy dialog
	if (busyDialog)
	{
		busyDialog->DoClose();
		delete handler;
	}
	busyDialog = NULL;
	handler = NULL;
	sBusyDialog = false;
}

#elif __dest_os == __win32_os
#pragma mark ____________________________Win32 Busy

BOOL OnMessagePending();
BOOL OnMessagePending()
{
	MSG msg;
	if (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_REMOVE))
	{
		::DispatchMessage(&msg);
		return FALSE;   // usually return TRUE, but OnIdle usually causes WM_PAINTs
	}
	return FALSE;
}

bool CMailControl::BusyCancel(const CBusyContext* busy, bool allow_cancel)
{
	bool cancel = false;

	// Timeouts
	unsigned long busy_start = CPreferences::sPrefs ? CPreferences::sPrefs->mWaitDialogStart.GetValue() : 5;
	unsigned long busy_max = CPreferences::sPrefs ? CPreferences::sPrefs->mWaitDialogMax.GetValue() : 5 * 60;

	// Beep if busy time exceeded
	if (allow_cancel && busy && (::time(NULL) > busy->GetBusyTime() + busy_start))
	{
		// Check for max timeout
		if (::time(NULL) > busy->GetBusyTime() + busy_max)
		{
			CLOG_LOGERROR("CMailControl::BusyCancel - busy time exceeded");
			cancel = true;
		}
		else
		{
			// Display the busy dialog if not already up
			if (!sBusyDialog)
				DisplayBusyDialog(*busy);
			
			// Handle an event on the busy dialog
			cancel = HandleBusyDialog(*busy);
			if (cancel)
				CLOG_LOGERROR("CMailControl::BusyCancel - busy dialog cancel");
		}
	}
	else 
	{
		MSG msg;

		// Must allow socket messages through
#define	WM_TCP_ASYNCDNR	(WM_USER + 2)
		if (::PeekMessage(&msg, NULL, WM_TCP_ASYNCDNR, WM_TCP_ASYNCDNR, PM_REMOVE))
		{
			::DispatchMessage(&msg);
		}

#define	WM_TCP_ASYNCSELECT	(WM_USER + 3)
		else if (::PeekMessage(&msg, NULL, WM_TCP_ASYNCSELECT, WM_TCP_ASYNCSELECT, PM_REMOVE))
		{
			::DispatchMessage(&msg);
		}

		// Look for key downs
		else if (::PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE))
		{
			// If escape key force cancel
			cancel = (msg.wParam == VK_ESCAPE);
			if (sAllowBusyKeys && !cancel)
				::DispatchMessage(&msg);
			if (cancel)
				CLOG_LOGERROR("CMailControl::BusyCancel - user cancel");
		}

		// Paint UI on idel
		else if (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) && ::OnMessagePending())
		{
			CWinThread* pThread = AfxGetThread();
			// allow user-interface updates
			pThread->OnIdle(-1);
		}
	}
	
	return cancel;
}

static CCancelDialog* busyDialog = NULL;

// Display busy dialog to user
void CMailControl::DisplayBusyDialog(const CBusyContext& busy)
{
	// Create the busy dialog - make sure periodicals are turned off
	//::PauseSpinning(true);
	busyDialog = new CCancelDialog(NULL);
	busyDialog->SetBusyDescriptor(busy.GetDescriptor());
	busyDialog->StartModal();
	sBusyDialog = true;
}

// Handle busy dialog event returning if cancelled
bool CMailControl::HandleBusyDialog(const CBusyContext& busy)
{
	// Set current time
	busyDialog->SetTime(::time(NULL) - busy.GetBusyTime());
	
	// Process an event
	return busyDialog->ModalLoop();
}

// Remove the busy dialog
void CMailControl::RemoveBusyDialog()
{
	// Remove the busy dialog
	if (busyDialog)
	{
		busyDialog->StopModal();
		delete busyDialog;
	}
	busyDialog = NULL;
	sBusyDialog = false;
	//::PauseSpinning(false);
}

#elif __dest_os == __linux_os
#pragma mark ____________________________Linux Busy

bool CMailControl::BusyCancel(const CBusyContext* busy, bool allow_cancel)
{
	bool cancel = false;

	// Timeouts
	time_t busy_start = CPreferences::sPrefs ? CPreferences::sPrefs->mWaitDialogStart.GetValue() : 5;
	time_t busy_max = CPreferences::sPrefs ? CPreferences::sPrefs->mWaitDialogMax.GetValue() : 5 * 60;

	// Beep if busy time exceeded
	if (allow_cancel && busy && (::time(NULL) > busy->GetBusyTime() + busy_start))
	{
		// Check for max timeout
		if (::time(NULL) > busy->GetBusyTime() + busy_max)
		{
			CLOG_LOGERROR("CMailControl::BusyCancel - busy time exceeded");
			cancel = true;
		}
		else
		{
			// Display the busy dialog if not already up
			if (!sBusyDialog)
				DisplayBusyDialog(*busy);
			
			// Handle an event on the busy dialog
			cancel = HandleBusyDialog(*busy);
			if (cancel)
				CLOG_LOGERROR("CMailControl::BusyCancel - busy dialog cancel");
		}
	}
	else
	{
		cancel = CMulberryApp::sApp->HandleOneBusyEvent(kFalse);
		if (cancel)
			CLOG_LOGERROR("CMailControl::BusyCancel - user cancel");
	}

	return cancel;
}

static CCancelDialog* busyDialog = NULL;

// Display busy dialog to user
void CMailControl::DisplayBusyDialog(const CBusyContext& busy)
{
	// Create the busy dialog - make sure periodicals are turned off
	//::PauseSpinning(true);
	busyDialog = new CCancelDialog(JXGetApplication());
	busyDialog->OnCreate();
	busyDialog->SetBusyDescriptor(busy.GetDescriptor());
	busyDialog->BeginDialog();
	JXGetApplication()->DisplayInactiveCursor();
	sBusyDialog = true;
}

// Handle busy dialog event returning if cancelled
bool CMailControl::HandleBusyDialog(const CBusyContext& busy)
{
	// Set current time
	busyDialog->SetTime(::time(NULL) - busy.GetBusyTime());
	
	// Process an event
	JXWindow* window = busyDialog->GetWindow();

	// Handle an event (but without idle tasks)
	JXGetApplication()->HandleOneEventForWindow(window, kTrue, kFalse);
	return (busyDialog->GetClosedState() == CDialogDirector::kDialogClosed_OK);
}

// Remove the busy dialog
void CMailControl::RemoveBusyDialog()
{
	// Remove the busy dialog
	if (busyDialog)
	{
		busyDialog->EndDialog(CDialogDirector::kDialogClosed_Cancel);
	}
	busyDialog = NULL;
	sBusyDialog = false;
	//::PauseSpinning(false);
}

#else
#error __dest_os
#endif

#pragma mark ____________________________Common Busy

// Start busy processing
void CMailControl::StartBusy()
{
	// Check if any nested before
	if (!sBlockCount && CMulberryApp::sApp)
	{
		// None nested so get current pause state
		sWasBlocked = CMulberryApp::sApp->IsPaused();

		// If not already paused, pause it
		if (sWasBlocked)
			CMulberryApp::sApp->ErrorPause(true);
	}

	// Increment counter
	sBlockCount++;
			
	// Check for busy dialog in main thread only
	if (CTaskQueue::sTaskQueue.InMainThread())
	{	
		// Record busy time start if its the first one
		sBusyCount++;
	}
}

// Stop busy processing
void CMailControl::StopBusy()
{
	// Check for count 
	if (sBlockCount)
	{
		// Bump down count
		if (!--sBlockCount)
		{
			// If not previously paused, unpause
			if (!sWasBlocked && CMulberryApp::sApp)
				CMulberryApp::sApp->ErrorPause(false);
			sWasBlocked = false;
		}
	}

	// Remove the busy dialog if displayed and count is zero
	if (CTaskQueue::sTaskQueue.InMainThread() && !--sBusyCount && sBusyDialog)
		RemoveBusyDialog();
	
	// Main thread exit always resets the working thread cancel flag
	if (CTaskQueue::sTaskQueue.InMainThread())
		sCancelOthers = false;
}

// Give time to application while busy (test for cancel)
bool CMailControl::ProcessBusy(const CBusyContext* busy, bool allow_cancel)
{
	// No processing if blocked
	if (sBlockBusy)
		return false;

	bool cancel = false;

	// Do not allow anything in here to throw up
	try
	{
		// Only process user events if in main thread
		if (CTaskQueue::sTaskQueue.InMainThread())
		{
			cancel = BusyCancel(busy, allow_cancel);
			
			// Look for cancel of others
			if (cancel && busy && busy->CancelOthers())
			{
				cancel = false;
				sCancelOthers = true;
			}
		}

		// Force worker threads to cancel if requested and delayed more than 5 secs
		else if (sCancelOthers && busy && (::time(NULL) > busy->GetBusyTime() + 5))
			cancel = true;
		
		// Just yield to others
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		else
			::YieldToAnyThread();
#elif __dest_os == __win32_os
#elif __dest_os == __linux_os
		else
			ACE_Thread::yield();
#else
#error __dest_os
#endif
	}

	// Catch all...
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Éand do nothing
	}

	return cancel;
}
